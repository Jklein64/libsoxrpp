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

unsigned long convert_quality_recipe(const soxrpp::SoxrQualityRecipe& recipe) {
    // Assumes that the enum values align with the internal soxr #defines
    return static_cast<unsigned long>(recipe);
}

template <soxrpp::SoxrDataType itype, soxrpp::SoxrDataType otype>
soxr_io_spec_t convert_io_spec(const soxrpp::SoxrIoSpec<itype, otype>& io_spec) {
    return (soxr_io_spec_t){
        .itype = convert_datatype(itype),
        .otype = convert_datatype(otype),
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

soxr_runtime_spec_t convert_runtime_spec(const soxrpp::SoxrRuntimeSpec& runtime_spec) {
    return (soxr_runtime_spec_t){
        .log2_min_dft_size = runtime_spec.log2_min_dft_size,
        .log2_large_dft_size = runtime_spec.log2_large_dft_size,
        .coef_size_kbytes = runtime_spec.coef_size_kbytes,
        .num_threads = runtime_spec.num_threads,
        .flags = runtime_spec.flags,
    };
}

namespace soxrpp {

template <SoxrDataType itype, SoxrDataType otype>
SoxrIoSpec<itype, otype>::SoxrIoSpec() {
    // Use soxr's error checking
    soxr_io_spec_t io_spec = soxr_io_spec(static_cast<soxr_datatype_t>(itype), static_cast<soxr_datatype_t>(otype));
    if (io_spec.e) {
        throw SoxrError(static_cast<const char*>(io_spec.e));
    }
    this->scale = 1;
    this->flags = 0;
}

SoxrQualitySpec::SoxrQualitySpec() noexcept {
    this->precision = 20;
    this->phase_response = 50;
    this->passband_end = 0.913;
    this->stopband_begin = 1;
    this->flags = 0;
}

SoxrQualitySpec::SoxrQualitySpec(SoxrQualityRecipe recipe, unsigned long flags) {
    soxr_quality_spec_t quality_spec = soxr_quality_spec(convert_quality_recipe(recipe), flags);
    if (quality_spec.e) {
        throw SoxrError(static_cast<const char*>(quality_spec.e));
    }
    this->precision = quality_spec.precision;
    this->phase_response = quality_spec.phase_response;
    this->passband_end = quality_spec.passband_end;
    this->stopband_begin = quality_spec.stopband_begin;
    this->flags = quality_spec.flags;
}

SoxrRuntimeSpec::SoxrRuntimeSpec() noexcept {
    this->log2_min_dft_size = 10;
    this->log2_large_dft_size = 17;
    this->coef_size_kbytes = 400;
    this->num_threads = 1;
    this->flags = 0;
}

SoxrRuntimeSpec::SoxrRuntimeSpec(unsigned int num_threads) noexcept {
    soxr_runtime_spec_t runtime_spec = soxr_runtime_spec(num_threads);
    this->log2_min_dft_size = runtime_spec.log2_min_dft_size;
    this->log2_large_dft_size = runtime_spec.log2_large_dft_size;
    this->coef_size_kbytes = runtime_spec.coef_size_kbytes;
    this->num_threads = runtime_spec.num_threads;
    this->flags = runtime_spec.flags;
}

template <SoxrDataType itype, SoxrDataType otype>
SoxResampler<itype, otype>::SoxResampler(double input_rate, double output_rate, unsigned int num_channels,
                                         const SoxrIoSpec<itype, otype>& io_spec, const SoxrQualitySpec& quality_spec,
                                         const SoxrRuntimeSpec& runtime_spec) {
    soxr_error_t err;
    soxr_io_spec_t io_spec_raw = convert_io_spec(io_spec);
    soxr_quality_spec_t quality_spec_raw = convert_quality_spec(quality_spec);
    soxr_runtime_spec_t runtime_spec_raw = convert_runtime_spec(runtime_spec);
    m_soxr = soxr_create(input_rate, output_rate, num_channels, &err, &io_spec_raw, &quality_spec_raw, &runtime_spec_raw);
    throw_if_soxr_error(err);
}

template <SoxrDataType itype, SoxrDataType otype>
SoxResampler<itype, otype>::~SoxResampler() {
    soxr_delete(m_soxr);
}

template <SoxrDataType itype, SoxrDataType otype>
void SoxResampler<itype, otype>::process(soxr_in_t in, size_t ilen, size_t* idone, soxr_out_t out, size_t olen, size_t* odone) {
    throw_if_soxr_error(soxr_process(m_soxr, in, ilen, idone, out, olen, odone));
}

template <SoxrDataType itype, SoxrDataType otype>
void SoxResampler<itype, otype>::set_input_fn(soxr_input_fn_t input_fn, void* input_fn_state, size_t max_ilen) {
    throw_if_soxr_error(soxr_set_input_fn(m_soxr, input_fn, input_fn_state, max_ilen));
}

template <SoxrDataType itype, SoxrDataType otype>
size_t SoxResampler<itype, otype>::output(soxr_out_t data, size_t olen) {
    size_t odone = soxr_output(m_soxr, data, olen);
    soxr_error_t err = soxr_error(m_soxr);
    if (err != 0) {
        throw SoxrError(err);
    }
    return odone;
}

template <SoxrDataType itype, SoxrDataType otype>
std::optional<std::string> SoxResampler<itype, otype>::error() noexcept {
    soxr_error_t err = soxr_error(m_soxr);
    return err == 0 ? std::nullopt : std::make_optional(err);
}

template <SoxrDataType itype, SoxrDataType otype>
size_t* SoxResampler<itype, otype>::num_clips() noexcept {
    return soxr_num_clips(m_soxr);
}

template <SoxrDataType itype, SoxrDataType otype>
double SoxResampler<itype, otype>::delay() noexcept {
    return soxr_delay(m_soxr);
}

template <SoxrDataType itype, SoxrDataType otype>
char const* SoxResampler<itype, otype>::engine() noexcept {
    return soxr_engine(m_soxr);
}

template <SoxrDataType itype, SoxrDataType otype>
void SoxResampler<itype, otype>::clear() {
    throw_if_soxr_error(soxr_clear(m_soxr));
}

template <SoxrDataType itype, SoxrDataType otype>
void oneshot(double input_rate, double output_rate, unsigned num_channels, soxr_in_t in, size_t ilen, size_t* idone, soxr_out_t out,
             size_t olen, size_t* odone, const SoxrIoSpec<itype, otype>& io_spec, const SoxrQualitySpec& quality_spec,
             const SoxrRuntimeSpec& runtime_spec) {
    soxr_io_spec_t io_spec_raw = convert_io_spec(io_spec);
    soxr_quality_spec_t quality_spec_raw = convert_quality_spec(quality_spec);
    soxr_runtime_spec_t runtime_spec_raw = convert_runtime_spec(runtime_spec);
    throw_if_soxr_error(soxr_oneshot(input_rate, output_rate, num_channels, in, ilen, idone, out, olen, odone, &io_spec_raw,
                                     &quality_spec_raw, &runtime_spec_raw));
}

} // namespace soxrpp