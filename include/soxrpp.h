#pragma once

#include "soxr.h"

#include <cstddef>
#include <optional>
#include <string>

namespace soxrpp {

class SoxrError : std::exception {
  private:
    std::string message;

  public:
    SoxrError(const std::string& message)
        : message(message) {}

    const char* what() const noexcept override {
        return message.c_str();
    }
};

enum class SoxrDataType {
    /* Internal; do not use: */
    Float32,
    Float64,
    Int32,
    Int16,
    Split = 4,

    /* Use for interleaved channels: */
    Float32_I = Float32,
    Float64_I,
    Int32_I,
    Int16_I,

    /* Use for split channels: */
    Float32_S = Split,
    Float64_S,
    Int32_S,
    Int16_S
};

enum class SoxrQualityRecipe {
    Quick = 0,    /* 'Quick' cubic interpolation. */
    Low = 1,      /* 'Low' 16-bit with larger rolloff. */
    Medium = 2,   /* 'Medium' 16-bit with medium rolloff. */
    High = 4,     /* 'High quality' 20-bit. */
    VeryHigh = 6, /* 'Very high quality' 28-bit. */

    B16 = 3,
    B20 = 4,
    B24 = 5,
    B28 = 6,
    B32 = 7,
    /* Reserved for internal use (to be removed): */
    LSR0 = 8,  /* 'Best sinc'. */
    LSR1 = 9,  /* 'Medium sinc'. */
    LSR2 = 10, /* 'Fast sinc'. */
    LinearPhase = 0x00,
    IntermediatePhase = 0x10,
    MinimumPhase = 0x30,
    SteepFilter = 0x40,
};

namespace SoxrIoFlags {
constexpr unsigned long TPDF = 0;
constexpr unsigned long NoDither = 8u;
} // namespace SoxrIoFlags
template <SoxrDataType itype, SoxrDataType otype>
struct SoxrIoSpec {
    double scale;
    unsigned long flags;

    inline SoxrIoSpec() {
        // Use soxr's error checking
        soxr_io_spec_t io_spec = soxr_io_spec(static_cast<soxr_datatype_t>(itype), static_cast<soxr_datatype_t>(otype));
        if (io_spec.e) {
            throw SoxrError(static_cast<const char*>(io_spec.e));
        }
        this->scale = 1;
        this->flags = 0;
    }
};

namespace SoxrQualityFlags {
constexpr unsigned long RolloffSmall = 0u;
constexpr unsigned long RolloffMedium = 1u;
constexpr unsigned long RolloffNone = 2u;
constexpr unsigned long HiPrecisionClock = 8u;
constexpr unsigned long DoublePrecisionClock = 16u;
constexpr unsigned long VariableRate = 32u;
}; // namespace SoxrQualityFlags
struct SoxrQualitySpec {
    double precision;
    double phase_response;
    double passband_end;
    double stopband_begin;
    unsigned long flags;

    inline SoxrQualitySpec() noexcept {
        this->precision = 20;
        this->phase_response = 50;
        this->passband_end = 0.913;
        this->stopband_begin = 1;
        this->flags = 0;
    }

    inline SoxrQualitySpec(SoxrQualityRecipe recipe, unsigned long flags) {
        soxr_quality_spec_t quality_spec = soxr_quality_spec(static_cast<unsigned long>(recipe), flags);
        if (quality_spec.e) {
            throw SoxrError(static_cast<const char*>(quality_spec.e));
        }
        this->precision = quality_spec.precision;
        this->phase_response = quality_spec.phase_response;
        this->passband_end = quality_spec.passband_end;
        this->stopband_begin = quality_spec.stopband_begin;
        this->flags = quality_spec.flags;
    }
};

namespace SoxrRuntimeFlags {
constexpr unsigned long CoeffInterpAuto = 0u;
constexpr unsigned long CoeffInterpLow = 2u;
constexpr unsigned long CoeffInterpHigh = 3u;
} // namespace SoxrRuntimeFlags
struct SoxrRuntimeSpec {
    unsigned int log2_min_dft_size;
    unsigned int log2_large_dft_size;
    unsigned int coef_size_kbytes;
    unsigned int num_threads;
    unsigned long flags;

    inline SoxrRuntimeSpec() noexcept {
        this->log2_min_dft_size = 10;
        this->log2_large_dft_size = 17;
        this->coef_size_kbytes = 400;
        this->num_threads = 1;
        this->flags = 0;
    }

    inline SoxrRuntimeSpec(unsigned int num_threads) noexcept {
        soxr_runtime_spec_t runtime_spec = soxr_runtime_spec(num_threads);
        this->log2_min_dft_size = runtime_spec.log2_min_dft_size;
        this->log2_large_dft_size = runtime_spec.log2_large_dft_size;
        this->coef_size_kbytes = runtime_spec.coef_size_kbytes;
        this->num_threads = runtime_spec.num_threads;
        this->flags = runtime_spec.flags;
    }
};

template <SoxrDataType itype = SoxrDataType::Float32_I, SoxrDataType otype = SoxrDataType::Float32_I>
class SoxResampler {
  private:
    soxr_t m_soxr{nullptr};

  public:
    inline SoxResampler(double input_rate, double output_rate, unsigned int num_channels,
                        const SoxrIoSpec<itype, otype>& io_spec = SoxrIoSpec<SoxrDataType::Float32_I, SoxrDataType::Float32_I>(),
                        const SoxrQualitySpec& quality_spec = SoxrQualitySpec(SoxrQualityRecipe::High, 0),
                        const SoxrRuntimeSpec& runtime_spec = SoxrRuntimeSpec(1)) {
        soxr_error_t err;
        soxr_io_spec_t io_spec_raw = (soxr_io_spec_t){
            .itype = static_cast<int>(itype),
            .otype = static_cast<int>(otype),
            .scale = io_spec.scale,
            .flags = io_spec.flags,
        };
        soxr_quality_spec_t quality_spec_raw = (soxr_quality_spec_t){
            .precision = quality_spec.precision,
            .phase_response = quality_spec.phase_response,
            .passband_end = quality_spec.passband_end,
            .stopband_begin = quality_spec.stopband_begin,
            .flags = quality_spec.flags,
        };
        soxr_runtime_spec_t runtime_spec_raw = (soxr_runtime_spec_t){
            .log2_min_dft_size = runtime_spec.log2_min_dft_size,
            .log2_large_dft_size = runtime_spec.log2_large_dft_size,
            .coef_size_kbytes = runtime_spec.coef_size_kbytes,
            .num_threads = runtime_spec.num_threads,
            .flags = runtime_spec.flags,
        };
        m_soxr = soxr_create(input_rate, output_rate, num_channels, &err, &io_spec_raw, &quality_spec_raw, &runtime_spec_raw);
        if (err != 0) {
            throw soxrpp::SoxrError(err);
        }
    }

    inline ~SoxResampler() {
        soxr_delete(m_soxr);
    }

    inline void process(soxr_in_t in, size_t ilen, size_t* idone, soxr_out_t out, size_t olen, size_t* odone) {
        soxr_error_t err = soxr_process(m_soxr, in, ilen, idone, out, olen, odone);
        if (err != 0) {
            throw soxrpp::SoxrError(err);
        }
    }

    inline void set_input_fn(soxr_input_fn_t input_fn, void* input_fn_state, size_t max_ilen) {
        soxr_error_t err = soxr_set_input_fn(m_soxr, input_fn, input_fn_state, max_ilen);
        if (err != 0) {
            throw soxrpp::SoxrError(err);
        }
    }

    inline size_t output(soxr_out_t data, size_t olen) {
        size_t odone = soxr_output(m_soxr, data, olen);
        soxr_error_t err = soxr_error(m_soxr);
        if (err != 0) {
            throw SoxrError(err);
        }
        return odone;
    }

    inline std::optional<std::string> error() noexcept {
        soxr_error_t err = soxr_error(m_soxr);
        return err == 0 ? std::nullopt : std::make_optional(err);
    }

    inline size_t* num_clips() noexcept {
        return soxr_num_clips(m_soxr);
    }

    inline double delay() noexcept {
        return soxr_delay(m_soxr);
    }

    inline char const* engine() noexcept {
        return soxr_engine(m_soxr);
    }

    inline void clear() {
        soxr_error_t err = soxr_clear(m_soxr);
        if (err != 0) {
            throw soxrpp::SoxrError(err);
        }
    }
};

template <SoxrDataType itype = SoxrDataType::Float32_I, SoxrDataType otype = SoxrDataType::Float32_I>
inline void oneshot(double input_rate, double output_rate, unsigned num_channels, soxr_in_t in, size_t ilen, size_t* idone,
                    soxr_out_t out, size_t olen, size_t* odone,
                    const SoxrIoSpec<itype, otype>& io_spec = SoxrIoSpec<SoxrDataType::Float32_I, SoxrDataType::Float32_I>(),
                    const SoxrQualitySpec& quality_spec = SoxrQualitySpec(SoxrQualityRecipe::Low, 0),
                    const SoxrRuntimeSpec& runtime_spec = SoxrRuntimeSpec(1)) {
    soxr_io_spec_t io_spec_raw = (soxr_io_spec_t){
        .itype = static_cast<soxr_datatype_t>(itype),
        .otype = static_cast<soxr_datatype_t>(otype),
        .scale = io_spec.scale,
        .flags = io_spec.flags,
    };
    soxr_quality_spec_t quality_spec_raw = (soxr_quality_spec_t){
        .precision = quality_spec.precision,
        .phase_response = quality_spec.phase_response,
        .passband_end = quality_spec.passband_end,
        .stopband_begin = quality_spec.stopband_begin,
        .flags = quality_spec.flags,
    };
    soxr_runtime_spec_t runtime_spec_raw = (soxr_runtime_spec_t){
        .log2_min_dft_size = runtime_spec.log2_min_dft_size,
        .log2_large_dft_size = runtime_spec.log2_large_dft_size,
        .coef_size_kbytes = runtime_spec.coef_size_kbytes,
        .num_threads = runtime_spec.num_threads,
        .flags = runtime_spec.flags,
    };
    soxr_error_t err = soxr_oneshot(input_rate, output_rate, num_channels, in, ilen, idone, out, olen, odone, &io_spec_raw,
                                    &quality_spec_raw, &runtime_spec_raw);
    if (err != 0) {
        throw soxrpp::SoxrError(err);
    }
}

} // namespace soxrpp