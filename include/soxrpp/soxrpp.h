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

struct SOXRPP_EXPORT SoxrIoSpec {
    SoxrDataType itype, otype;
    double scale;
    unsigned long flags; // TODO how best to expose flags?
};

struct SOXRPP_EXPORT SoxrQualitySpec {
    double precision;
    double phase_response;
    double passband_end;
    double stopband_begin;
    unsigned long flags;
};

// using SoxrRuntimeSpec = soxr_runtime_spec_t;

class SOXRPP_EXPORT SoxResampler {
  private:
    soxr_t m_soxr{nullptr};
    std::optional<SoxrIoSpec> m_io_spec;
    std::optional<SoxrQualitySpec> m_quality_spec;
    // Types are erased to not require soxr.h
    void* m_io_spec_internal{nullptr};
    void* m_quality_spec_internal{nullptr};

  public:
    SoxResampler(double input_rate, double output_rate, unsigned int num_channels,
                 const std::optional<SoxrIoSpec>& io_spec = std::nullopt,
                 const std::optional<SoxrQualitySpec>& quality_spec = std::nullopt, const soxr_runtime_spec_t* runtime_spec = nullptr);
    ~SoxResampler();

    void process(soxr_in_t in, size_t ilen, size_t* idone, soxr_out_t out, size_t olen, size_t* odone);
    void set_input_fn(soxr_input_fn_t, void* input_fn_state, size_t max_ilen);
    size_t output(soxr_out_t data, size_t olen) noexcept;

    std::string error() noexcept;
    size_t* num_clips() noexcept;
    double delay() noexcept;
    char const* engine() noexcept;
    void clear();
};

SOXRPP_EXPORT void oneshot(double input_rate, double output_rate, unsigned num_channels, soxr_in_t in, size_t ilen, size_t* idone,
                           soxr_out_t out, size_t olen, size_t* odone, const std::optional<SoxrIoSpec>& io_spec = std::nullopt,
                           const std::optional<SoxrQualitySpec>& quality_spec = std::nullopt,
                           const soxr_runtime_spec_t* runtime_spec = nullptr);

} // namespace soxrpp