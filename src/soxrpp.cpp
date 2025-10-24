#include "soxrpp/soxrpp.h"

#include "soxrpp.h"
#include <array>

namespace soxrpp {

SoxResampler::SoxResampler(double input_rate, double output_rate, unsigned int num_channels, soxr_error_t* err,
                           const soxr_io_spec_t* io_spec, const soxr_quality_spec_t* quality_spec,
                           const soxr_runtime_spec_t* runtime_spec) {
    m_soxr = soxr_create(input_rate, output_rate, num_channels, err, io_spec, quality_spec, runtime_spec);
}

SoxResampler::~SoxResampler() {
    soxr_delete(m_soxr);
}

soxr_error_t SoxResampler::process(soxr_in_t in, size_t ilen, size_t* idone, soxr_out_t out, size_t olen, size_t* odone) {
    return soxr_process(m_soxr, in, ilen, idone, out, olen, odone);
}

soxr_error_t SoxResampler::set_input_fn(soxr_t resampler, soxr_input_fn_t input_fn, void* input_fn_state, size_t max_ilen) {
    return soxr_set_input_fn(m_soxr, input_fn, input_fn_state, max_ilen);
}

size_t SoxResampler::output(soxr_t resampler, soxr_out_t data, size_t olen) {
    return soxr_output(m_soxr, data, olen);
}

soxr_error_t SoxResampler::error() {
    return soxr_error(m_soxr);
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

soxr_error_t SoxResampler::clear() {
    return soxr_clear(m_soxr);
}

soxr_error_t SoxResampler::oneshot(double input_rate, double output_rate, unsigned num_channels, soxr_in_t in, size_t ilen,
                                   size_t* idone, soxr_out_t out, size_t olen, size_t* odone, const soxr_io_spec_t* io_spec,
                                   const soxr_quality_spec_t* quality_spec, const soxr_runtime_spec_t* runtime_spec) {
    return soxr_oneshot(input_rate, output_rate, num_channels, in, ilen, idone, out, olen, odone, io_spec, quality_spec, runtime_spec);
}

} // namespace soxrpp