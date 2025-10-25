#pragma once

#include "soxr.h"
#include "soxrpp/export.h"

#include <cstddef>
#include <optional>
#include <string>

namespace soxrpp {

class SOXRPP_EXPORT SoxrError : std::exception {
  private:
    std::string message;

  public:
    SoxrError(const std::string& message)
        : message(message) {}

    const char* what() const noexcept override {
        return message.c_str();
    }
};

enum class SOXRPP_EXPORT SoxrDataType {
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

enum class SOXRPP_EXPORT SoxrQualityRecipe {
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
struct SOXRPP_EXPORT SoxrIoSpec {
    SoxrDataType itype, otype;
    double scale;
    unsigned long flags;

    SoxrIoSpec() noexcept;
    SoxrIoSpec(SoxrDataType itype, SoxrDataType otype);
};

namespace SoxrQualityFlags {
constexpr unsigned long RolloffSmall = 0u;
constexpr unsigned long RolloffMedium = 1u;
constexpr unsigned long RolloffNone = 2u;
constexpr unsigned long HiPrecisionClock = 8u;
constexpr unsigned long DoublePrecisionClock = 16u;
constexpr unsigned long VariableRate = 32u;
}; // namespace SoxrQualityFlags
struct SOXRPP_EXPORT SoxrQualitySpec {
    double precision;
    double phase_response;
    double passband_end;
    double stopband_begin;
    unsigned long flags;

    SoxrQualitySpec() noexcept;
    SoxrQualitySpec(SoxrQualityRecipe recipe, unsigned long flags);
};

namespace SoxrRuntimeFlags {
constexpr unsigned long CoeffInterpAuto = 0u;
constexpr unsigned long CoeffInterpLow = 2u;
constexpr unsigned long CoeffInterpHigh = 3u;
} // namespace SoxrRuntimeFlags
struct SOXRPP_EXPORT SoxrRuntimeSpec {
    unsigned int log2_min_dft_size;
    unsigned int log2_large_dft_size;
    unsigned int coef_size_kbytes;
    unsigned int num_threads;
    unsigned long flags;

    SoxrRuntimeSpec() noexcept;
    SoxrRuntimeSpec(unsigned int num_threads) noexcept;
};

class SOXRPP_EXPORT SoxResampler {
  private:
    soxr_t m_soxr{nullptr};

  public:
    SoxResampler(double input_rate, double output_rate, unsigned int num_channels,
                 const SoxrIoSpec& io_spec = SoxrIoSpec(SoxrDataType::Float32_I, SoxrDataType::Float32_I),
                 const SoxrQualitySpec& quality_spec = SoxrQualitySpec(SoxrQualityRecipe::High, 0),
                 const SoxrRuntimeSpec& runtime_spec = SoxrRuntimeSpec(1));
    ~SoxResampler();

    void process(soxr_in_t in, size_t ilen, size_t* idone, soxr_out_t out, size_t olen, size_t* odone);
    void set_input_fn(soxr_input_fn_t, void* input_fn_state, size_t max_ilen);
    size_t output(soxr_out_t data, size_t olen);

    std::optional<std::string> error() noexcept;
    size_t* num_clips() noexcept;
    double delay() noexcept;
    char const* engine() noexcept;
    void clear();

    // Advanced
    void set_io_ratio(double io_ratio, size_t slew_len);
    void set_num_channels(unsigned int num_channels);
};

SOXRPP_EXPORT void oneshot(double input_rate, double output_rate, unsigned num_channels, soxr_in_t in, size_t ilen, size_t* idone,
                           soxr_out_t out, size_t olen, size_t* odone,
                           const SoxrIoSpec& io_spec = SoxrIoSpec(SoxrDataType::Float32_I, SoxrDataType::Float32_I),
                           const SoxrQualitySpec& quality_spec = SoxrQualitySpec(SoxrQualityRecipe::Low, 0),
                           const SoxrRuntimeSpec& runtime_spec = SoxrRuntimeSpec(1));

} // namespace soxrpp