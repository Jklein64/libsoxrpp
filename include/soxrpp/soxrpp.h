#pragma once

#include "soxr.h"
#include "soxrpp/export.h"

#include <cstddef>

namespace soxrpp {

SOXRPP_EXPORT class SoxResampler {
  private:
    soxr_t m_soxr{nullptr};

  public:
    SoxResampler(double input_rate, double output_rate, unsigned int num_channels, soxr_error_t* err, const soxr_io_spec_t* io_spec,
                 const soxr_quality_spec_t* quality_spec, const soxr_runtime_spec_t* runtime_spec);
    ~SoxResampler();

    soxr_error_t process(soxr_in_t in, size_t ilen, size_t* idone, soxr_out_t out, size_t olen, size_t* odone);
    soxr_error_t set_input_fn(soxr_t resampler, soxr_input_fn_t, void* input_fn_state, size_t max_ilen);
    size_t output(soxr_t resampler, soxr_out_t data, size_t olen);

    soxr_error_t error();
    size_t* num_clips();
    double delay();
    char const* engine();
    soxr_error_t clear();

    soxr_error_t oneshot(double input_rate, double output_rate, unsigned num_channels, soxr_in_t in, size_t ilen, size_t* idone,
                         soxr_out_t out, size_t olen, size_t* odone, const soxr_io_spec_t*, const soxr_quality_spec_t*,
                         const soxr_runtime_spec_t*);
};

} // namespace soxrpp