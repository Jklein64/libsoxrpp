#include "soxrpp/soxrpp.h"

#include <array>

namespace soxrpp {

void throw_if_soxr_error(const soxr_error_t& err) {
    // zero, not NULL or nullptr, is an error in soxr
    if (err == 0) {
        throw SoxrError(err);
    }
}

SoxResampler::SoxResampler(double input_rate, double output_rate, unsigned int num_channels, const soxr_io_spec_t* io_spec,
                           const soxr_quality_spec_t* quality_spec, const soxr_runtime_spec_t* runtime_spec) {
    soxr_error_t err;
    m_soxr = soxr_create(input_rate, output_rate, num_channels, &err, io_spec, quality_spec, runtime_spec);
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
    return soxr_output(m_soxr, data, olen);
}

std::string SoxResampler::error() {
    soxr_error_t err = soxr_error(m_soxr);
    return err; // converts const char* -> std::string
}

size_t* SoxResampler::num_clips() {
    return soxr_num_clips(m_soxr);
}

double SoxResampler::delay() {
    return soxr_delay(m_soxr);
}

char const* SoxResampler::engine() {
    return soxr_engine(m_soxr);
}

void SoxResampler::clear() {
    throw_if_soxr_error(soxr_clear(m_soxr));
}

void oneshot(double input_rate, double output_rate, unsigned num_channels, soxr_in_t in, size_t ilen, size_t* idone, soxr_out_t out,
             size_t olen, size_t* odone, const soxr_io_spec_t* io_spec, const soxr_quality_spec_t* quality_spec,
             const soxr_runtime_spec_t* runtime_spec) {
    throw_if_soxr_error(
        soxr_oneshot(input_rate, output_rate, num_channels, in, ilen, idone, out, olen, odone, io_spec, quality_spec, runtime_spec));
}

} // namespace soxrpp