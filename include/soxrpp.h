#pragma once

#include <array>
#include <cstddef>
#include <optional>
#include <span>
#include <string>

namespace soxrpp {

namespace soxr {
#include "soxr.h"
} // namespace soxr

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
    /* Use for interleaved channels: */
    Float32_I = soxr::SOXR_FLOAT32_I,
    Float64_I = soxr::SOXR_FLOAT64_I,
    Int32_I = soxr::SOXR_INT32_I,
    Int16_I = soxr::SOXR_INT16_I,

    /* Use for split channels: */
    Float32_S = soxr::SOXR_FLOAT32_S,
    Float64_S = soxr::SOXR_FLOAT64_S,
    Int32_S = soxr::SOXR_INT32_S,
    Int16_S = soxr::SOXR_INT16_S
};

enum class SoxrQualityRecipe {
    Quick = soxr::SOXR_QQ,     /* 'Quick' cubic interpolation. */
    Low = soxr::SOXR_LQ,       /* 'Low' 16-bit with larger rolloff. */
    Medium = soxr::SOXR_MQ,    /* 'Medium' 16-bit with medium rolloff. */
    High = soxr::SOXR_HQ,      /* 'High quality' 20-bit. */
    VeryHigh = soxr::SOXR_VHQ, /* 'Very high quality' 28-bit. */

    B16 = soxr::SOXR_16_BITQ,
    B20 = soxr::SOXR_20_BITQ,
    B24 = soxr::SOXR_24_BITQ,
    B28 = soxr::SOXR_28_BITQ,
    B32 = soxr::SOXR_32_BITQ,
    /* Reserved for internal use (to be removed): */
    LSR0 = soxr::SOXR_LSR0Q, /* 'Best sinc'. */
    LSR1 = soxr::SOXR_LSR1Q, /* 'Medium sinc'. */
    LSR2 = soxr::SOXR_LSR2Q, /* 'Fast sinc'. */
    LinearPhase = soxr::SOXR_LINEAR_PHASE,
    IntermediatePhase = soxr::SOXR_INTERMEDIATE_PHASE,
    MinimumPhase = soxr::SOXR_MINIMUM_PHASE,
    SteepFilter = soxr::SOXR_STEEP_FILTER
};

namespace SoxrIoFlags {
constexpr unsigned long TPDF = soxr::SOXR_TPDF;
constexpr unsigned long NoDither = soxr::SOXR_NO_DITHER;
} // namespace SoxrIoFlags
template <SoxrDataType itype, SoxrDataType otype>
struct SoxrIoSpec {
    double scale;
    unsigned long flags;

    inline SoxrIoSpec() {
        // Use soxr's error checking
        soxr::soxr_io_spec_t io_spec =
            soxr::soxr_io_spec(static_cast<soxr::soxr_datatype_t>(itype), static_cast<soxr::soxr_datatype_t>(otype));
        if (io_spec.e) {
            throw SoxrError(static_cast<const char*>(io_spec.e));
        }
        this->scale = 1;
        this->flags = 0;
    }

    inline soxr::soxr_io_spec_t c_struct() const noexcept {
        return (soxr::soxr_io_spec_t){
            .itype = static_cast<soxr::soxr_datatype_t>(itype),
            .otype = static_cast<soxr::soxr_datatype_t>(otype),
            .scale = this->scale,
            .flags = this->flags,
        };
    }
};

namespace SoxrQualityFlags {
constexpr unsigned long RolloffSmall = soxr::SOXR_ROLLOFF_SMALL;
constexpr unsigned long RolloffMedium = soxr::SOXR_ROLLOFF_MEDIUM;
constexpr unsigned long RolloffNone = soxr::SOXR_ROLLOFF_NONE;
constexpr unsigned long HiPrecisionClock = soxr::SOXR_HI_PREC_CLOCK;
constexpr unsigned long DoublePrecision = soxr::SOXR_DOUBLE_PRECISION;
constexpr unsigned long VariableRate = soxr::SOXR_VR;
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
        soxr::soxr_quality_spec_t quality_spec = soxr::soxr_quality_spec(static_cast<unsigned long>(recipe), flags);
        if (quality_spec.e) {
            throw SoxrError(static_cast<const char*>(quality_spec.e));
        }
        this->precision = quality_spec.precision;
        this->phase_response = quality_spec.phase_response;
        this->passband_end = quality_spec.passband_end;
        this->stopband_begin = quality_spec.stopband_begin;
        this->flags = quality_spec.flags;
    }

    inline soxr::soxr_quality_spec_t c_struct() const noexcept {
        return (soxr::soxr_quality_spec_t){
            .precision = this->precision,
            .phase_response = this->phase_response,
            .passband_end = this->passband_end,
            .stopband_begin = this->stopband_begin,
            .flags = this->flags,
        };
    }
};

namespace SoxrRuntimeFlags {
constexpr unsigned long CoeffInterpAuto = soxr::SOXR_COEF_INTERP_AUTO;
constexpr unsigned long CoeffInterpLow = soxr::SOXR_COEF_INTERP_LOW;
constexpr unsigned long CoeffInterpHigh = soxr::SOXR_COEF_INTERP_HIGH;
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
        soxr::soxr_runtime_spec_t runtime_spec = soxr::soxr_runtime_spec(num_threads);
        this->log2_min_dft_size = runtime_spec.log2_min_dft_size;
        this->log2_large_dft_size = runtime_spec.log2_large_dft_size;
        this->coef_size_kbytes = runtime_spec.coef_size_kbytes;
        this->num_threads = runtime_spec.num_threads;
        this->flags = runtime_spec.flags;
    }

    inline soxr::soxr_runtime_spec_t c_struct() const noexcept {
        return (soxr::soxr_runtime_spec_t){
            .log2_min_dft_size = this->log2_min_dft_size,
            .log2_large_dft_size = this->log2_large_dft_size,
            .coef_size_kbytes = this->coef_size_kbytes,
            .num_threads = this->num_threads,
            .flags = this->flags,
        };
    }
};

template <SoxrDataType itype = SoxrDataType::Float32_I, SoxrDataType otype = SoxrDataType::Float32_I>
class SoxResampler {
  private:
    soxr::soxr_t m_soxr{nullptr};

  public:
    inline SoxResampler(double input_rate, double output_rate, unsigned int num_channels,
                        const SoxrIoSpec<itype, otype>& io_spec = SoxrIoSpec<SoxrDataType::Float32_I, SoxrDataType::Float32_I>(),
                        const SoxrQualitySpec& quality_spec = SoxrQualitySpec(SoxrQualityRecipe::High, 0),
                        const SoxrRuntimeSpec& runtime_spec = SoxrRuntimeSpec(1)) {
        soxr::soxr_error_t err;
        soxr::soxr_io_spec_t io_spec_raw = io_spec.c_struct();
        soxr::soxr_quality_spec_t quality_spec_raw = quality_spec.c_struct();
        soxr::soxr_runtime_spec_t runtime_spec_raw = runtime_spec.c_struct();
        m_soxr = soxr::soxr_create(input_rate, output_rate, num_channels, &err, &io_spec_raw, &quality_spec_raw, &runtime_spec_raw);
        if (err != 0) {
            throw soxrpp::SoxrError(err);
        }
    }

    inline ~SoxResampler() {
        soxr::soxr_delete(m_soxr);
    }

    inline void process(soxr::soxr_in_t in, size_t ilen, size_t* idone, soxr::soxr_out_t out, size_t olen, size_t* odone) {
        soxr::soxr_error_t err = soxr::soxr_process(m_soxr, in, ilen, idone, out, olen, odone);
        if (err != 0) {
            throw soxrpp::SoxrError(err);
        }
    }

    inline void set_input_fn(soxr::soxr_input_fn_t input_fn, void* input_fn_state, size_t max_ilen) {
        soxr::soxr_error_t err = soxr::soxr_set_input_fn(m_soxr, input_fn, input_fn_state, max_ilen);
        if (err != 0) {
            throw soxrpp::SoxrError(err);
        }
    }

    inline size_t output(soxr::soxr_out_t data, size_t olen) {
        size_t odone = soxr::soxr_output(m_soxr, data, olen);
        soxr::soxr_error_t err = soxr::soxr_error(m_soxr);
        if (err != 0) {
            throw SoxrError(err);
        }
        return odone;
    }

    inline std::optional<std::string> error() noexcept {
        soxr::soxr_error_t err = soxr_error(m_soxr);
        return err == 0 ? std::nullopt : std::make_optional(err);
    }

    inline size_t* num_clips() noexcept {
        return soxr::soxr_num_clips(m_soxr);
    }

    inline double delay() noexcept {
        return soxr::soxr_delay(m_soxr);
    }

    inline char const* engine() noexcept {
        return soxr::soxr_engine(m_soxr);
    }

    inline void clear() {
        soxr::soxr_error_t err = soxr::soxr_clear(m_soxr);
        if (err != 0) {
            throw soxrpp::SoxrError(err);
        }
    }
};

template <typename InputType, typename OutputType, size_t InputExtent, size_t OutputExtent, size_t InputChannels,
          size_t OutputChannels, SoxrDataType itype = SoxrDataType::Float32_I, SoxrDataType otype = SoxrDataType::Float32_I>
inline void oneshot(double input_rate, double output_rate, unsigned num_channels,
                    const std::array<std::span<InputType, InputExtent>, InputChannels>& ibuf, size_t* idone,
                    std::array<std::span<OutputType, OutputExtent>, OutputChannels>& obuf, size_t* odone,
                    const SoxrIoSpec<itype, otype>& io_spec = SoxrIoSpec<SoxrDataType::Float32_I, SoxrDataType::Float32_I>(),
                    const SoxrQualitySpec& quality_spec = SoxrQualitySpec(SoxrQualityRecipe::Low, 0),
                    const SoxrRuntimeSpec& runtime_spec = SoxrRuntimeSpec(1)) {
    if (!(InputChannels == 1 || InputChannels == num_channels) || !(OutputChannels == 1 || OutputChannels == num_channels)) {
        throw SoxrError("Invalid channel specification");
    }

    soxr::soxr_io_spec_t io_spec_raw = io_spec.c_struct();
    soxr::soxr_quality_spec_t quality_spec_raw = quality_spec.c_struct();
    soxr::soxr_runtime_spec_t runtime_spec_raw = runtime_spec.c_struct();
    bool input_interleaved = InputChannels != num_channels || num_channels == 1;
    bool output_interleaved = OutputChannels != num_channels || num_channels == 1;
    // Convert the wrapper structs into nested pointers
    std::array<InputType*, InputChannels> ibuf_raw;
    std::array<OutputType*, OutputChannels> obuf_raw;
    for (size_t i = 0; i < InputChannels; i++) {
        ibuf_raw[i] = ibuf[i].data();
    }
    for (size_t i = 0; i < OutputChannels; i++) {
        obuf_raw[i] = obuf[i].data();
    }
    // Note: soxr represents interleaved and single-channel split differently!
    soxr::soxr_in_t in = input_interleaved ? (void*)ibuf_raw[0] : (void*)ibuf_raw.data();
    soxr::soxr_out_t out = output_interleaved ? (void*)obuf_raw[0] : (void*)obuf_raw.data();
    // Length is the number of samples, where one "sample" counts across channels
    size_t ilen = ibuf[0].size() / (input_interleaved ? num_channels : 1);
    size_t olen = obuf[0].size() / (output_interleaved ? num_channels : 1);
    soxr::soxr_error_t err = soxr::soxr_oneshot(input_rate, output_rate, num_channels, in, ilen, idone, out, olen, odone, &io_spec_raw,
                                                &quality_spec_raw, &runtime_spec_raw);
    if (err != 0) {
        throw soxrpp::SoxrError(err);
    }
}

} // namespace soxrpp

#undef soxr_datatype_size
#undef soxr_included
#undef soxr_strerror