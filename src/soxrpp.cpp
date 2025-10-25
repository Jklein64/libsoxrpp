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

SoxrIoSpec::SoxrIoSpec(SoxrDataType itype, SoxrDataType otype) {
    soxr_io_spec_t io_spec = soxr_io_spec(convert_datatype(itype), convert_datatype(otype));
    if (io_spec.e) {
        throw SoxrError(static_cast<const char*>(io_spec.e));
    }
    this->itype = itype;
    this->otype = otype;
    this->scale = io_spec.scale;
    this->flags = io_spec.flags;
}

SoxrQualitySpec::SoxrQualitySpec(unsigned long recipe, unsigned long flags) {
    soxr_quality_spec_t quality_spec = soxr_quality_spec(recipe, flags);
    if (quality_spec.e) {
        throw SoxrError(static_cast<const char*>(quality_spec.e));
    }
    this->precision = quality_spec.precision;
    this->phase_response = quality_spec.phase_response;
    this->passband_end = quality_spec.passband_end;
    this->stopband_begin = quality_spec.stopband_begin;
    this->flags = quality_spec.flags;
}

SoxResampler::SoxResampler(double input_rate, double output_rate, unsigned int num_channels, const SoxrIoSpec& io_spec,
                           const std::optional<SoxrQualitySpec>& quality_spec, const soxr_runtime_spec_t* runtime_spec) {
    soxr_error_t err;
    soxr_io_spec_t io_spec_raw = convert_io_spec(io_spec);
    soxr_quality_spec_t quality_spec_raw;
    if (quality_spec.has_value()) {
        quality_spec_raw = convert_quality_spec(*quality_spec);
    }
    // m_soxr does not hold data from any of the pointers passed into soxr_create()
    m_soxr = soxr_create(input_rate, output_rate, num_channels, &err, &io_spec_raw,
                         quality_spec.has_value() ? &quality_spec_raw : nullptr, runtime_spec);
    throw_if_soxr_error(err);
}

SoxResampler::~SoxResampler() {
    soxr_delete(m_soxr);
}

void SoxResampler::process(soxr_in_t in, size_t ilen, size_t* idone, soxr_out_t out, size_t olen, size_t* odone) {
    throw_if_soxr_error(soxr_process(m_soxr, in, ilen, idone, out, olen, odone));
}

void SoxResampler::set_input_fn(soxr_input_fn_t input_fn, void* input_fn_state, size_t max_ilen) {
    throw_if_soxr_error(soxr_set_input_fn(m_soxr, input_fn, input_fn_state, max_ilen));
}

size_t SoxResampler::output(soxr_out_t data, size_t olen) {
    size_t odone = soxr_output(m_soxr, data, olen);
    soxr_error_t err = soxr_error(m_soxr);
    if (err != 0) {
        throw SoxrError(err);
    }
    return odone;
}

std::optional<std::string> SoxResampler::error() noexcept {
    soxr_error_t err = soxr_error(m_soxr);
    return err == 0 ? std::nullopt : std::make_optional(err);
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
             size_t olen, size_t* odone, const SoxrIoSpec& io_spec, const std::optional<SoxrQualitySpec>& quality_spec,
             const soxr_runtime_spec_t* runtime_spec) {
    soxr_io_spec_t io_spec_raw = convert_io_spec(io_spec);
    soxr_quality_spec_t quality_spec_raw;
    if (quality_spec.has_value()) {
        quality_spec_raw = convert_quality_spec(*quality_spec);
    }
    throw_if_soxr_error(soxr_oneshot(input_rate, output_rate, num_channels, in, ilen, idone, out, olen, odone, &io_spec_raw,
                                     quality_spec.has_value() ? &quality_spec_raw : nullptr, runtime_spec));
}

} // namespace soxrpp