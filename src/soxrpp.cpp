#include "soxrpp/soxrpp.h"

void throw_if_soxr_error(const soxr_error_t& err) {
    // Zero, not NULL or nullptr, is non-error in soxr
    if (err != 0) {
        throw soxrpp::SoxrError(err);
    }
}

soxr_datatype_t convert_datatype(const soxrpp::SoxrDataType& datatype) {
    // Assumes that both enums assign the same values to each datatype!
    return static_cast<soxr_datatype_t>(datatype);
}

soxr_io_spec_t convert_io_spec(const soxrpp::SoxrIoSpec& io_spec) {
    return (soxr_io_spec_t){
        .itype = convert_datatype(io_spec.itype),
        .otype = convert_datatype(io_spec.otype),
        .scale = io_spec.scale,
        .flags = io_spec.flags,
    };
}

soxr_quality_spec_t convert_quality_spec(const soxrpp::SoxrQualitySpec& quality_spec) {
    return (soxr_quality_spec_t){
        .precision = quality_spec.precision,
        .phase_response = quality_spec.phase_response,
        .passband_end = quality_spec.passband_end,
        .stopband_begin = quality_spec.stopband_begin,
        .flags = quality_spec.flags,
    };
}

namespace soxrpp {

SoxResampler::SoxResampler(double input_rate, double output_rate, unsigned int num_channels,
                           const std::optional<SoxrIoSpec>& io_spec = std::nullopt,
                           const std::optional<SoxrQualitySpec>& quality_spec = std::nullopt,
                           const soxr_runtime_spec_t* runtime_spec = 0)
    : m_io_spec(io_spec) {
    soxr_error_t err;
    m_io_spec_internal = malloc(sizeof(soxr_io_spec_t));
    m_quality_spec_internal = malloc(sizeof(soxr_quality_spec_t));
    if (io_spec.has_value()) {
        *static_cast<soxr_io_spec_t*>(m_io_spec_internal) = convert_io_spec(*io_spec);
    }
    if (quality_spec.has_value()) {
        *static_cast<soxr_quality_spec_t*>(m_quality_spec_internal) = convert_quality_spec(*quality_spec);
    }
    m_soxr = soxr_create(input_rate, output_rate, num_channels, &err, static_cast<soxr_io_spec_t*>(m_io_spec_internal),
                         static_cast<soxr_quality_spec_t*>(m_quality_spec_internal), runtime_spec);
    throw_if_soxr_error(err);
}

SoxResampler::~SoxResampler() {
    soxr_delete(m_soxr);
    free(m_quality_spec_internal);
    free(m_io_spec_internal);
}

void SoxResampler::process(soxr_in_t in, size_t ilen, size_t* idone, soxr_out_t out, size_t olen, size_t* odone) {
    throw_if_soxr_error(soxr_process(m_soxr, in, ilen, idone, out, olen, odone));
}

void SoxResampler::set_input_fn(soxr_input_fn_t input_fn, void* input_fn_state, size_t max_ilen) {
    throw_if_soxr_error(soxr_set_input_fn(m_soxr, input_fn, input_fn_state, max_ilen));
}

size_t SoxResampler::output(soxr_out_t data, size_t olen) noexcept {
    return soxr_output(m_soxr, data, olen);
}

std::string SoxResampler::error() noexcept {
    soxr_error_t err = soxr_error(m_soxr);
    return err; // converts const char* -> std::string
}

size_t* SoxResampler::num_clips() noexcept {
    return soxr_num_clips(m_soxr);
}

double SoxResampler::delay() noexcept {
    return soxr_delay(m_soxr);
}

char const* SoxResampler::engine() noexcept {
    return soxr_engine(m_soxr);
}

void SoxResampler::clear() {
    throw_if_soxr_error(soxr_clear(m_soxr));
}

void oneshot(double input_rate, double output_rate, unsigned num_channels, soxr_in_t in, size_t ilen, size_t* idone, soxr_out_t out,
             size_t olen, size_t* odone, const std::optional<SoxrIoSpec>& io_spec = std::nullopt,
             const std::optional<SoxrQualitySpec>& quality_spec = std::nullopt, const soxr_runtime_spec_t* runtime_spec = 0) {
    soxr_io_spec_t* io_spec_ptr = nullptr;
    soxr_io_spec_t io_spec_raw;
    if (io_spec.has_value()) {
        io_spec_raw = convert_io_spec(*io_spec);
        io_spec_ptr = &io_spec_raw;
    }
    soxr_quality_spec_t* quality_spec_ptr = nullptr;
    soxr_quality_spec_t quality_spec_raw;
    if (quality_spec.has_value()) {
        quality_spec_raw = convert_quality_spec(*quality_spec);
        quality_spec_ptr = &quality_spec_raw;
    }
    throw_if_soxr_error(soxr_oneshot(input_rate, output_rate, num_channels, in, ilen, idone, out, olen, odone, io_spec_ptr,
                                     quality_spec_ptr, runtime_spec));
}

} // namespace soxrpp