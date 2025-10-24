#pragma once

#include "soxr.h"
#include "soxrpp/export.h"

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

struct SoxrIoSpec {
    SoxrDataType itype, otype;
    double scale;
    unsigned long flags; // TODO how best to expose flags?
};

// using SoxrQualitySpec = soxr_quality_spec_t;
// using SoxrRuntimeSpec = soxr_runtime_spec_t;

SOXRPP_EXPORT class SoxResampler {
  private:
    soxr_t m_soxr{nullptr};
    std::optional<SoxrIoSpec> m_io_spec;
    // Type soxr_io_spec_t* is erased to not require soxr.h
    void* m_io_spec_internal{0};

  public:
    SoxResampler(double input_rate, double output_rate, unsigned int num_channels, const std::optional<SoxrIoSpec>& io_spec,
                 const soxr_quality_spec_t* quality_spec, const soxr_runtime_spec_t* runtime_spec);
    ~SoxResampler();

    void process(soxr_in_t in, size_t ilen, size_t* idone, soxr_out_t out, size_t olen, size_t* odone);
    void set_input_fn(soxr_input_fn_t, void* input_fn_state, size_t max_ilen);
    size_t output(soxr_out_t data, size_t olen);

    std::string error();
    size_t* num_clips();
    double delay();
    char const* engine();
    void clear();
};

SOXRPP_EXPORT void oneshot(double input_rate, double output_rate, unsigned num_channels, soxr_in_t in, size_t ilen, size_t* idone,
                           soxr_out_t out, size_t olen, size_t* odone, const SoxrIoSpec& io_spec,
                           const soxr_quality_spec_t* quality_spec, const soxr_runtime_spec_t* runtime_spec);

} // namespace soxrpp