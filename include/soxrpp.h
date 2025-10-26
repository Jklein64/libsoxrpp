#pragma once

#include <array>
#include <concepts>
#include <cstddef>
#include <memory>
#include <optional>
#include <span>
#include <string>
#include <tuple>
#include <type_traits>

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

enum class SoxrDataShape {
    Interleaved,
    Split
};

namespace detail {

// Bridging templates and the soxr enum is a bit verbose...
template <typename Type, SoxrDataShape Shape>
struct DataTypeConverter {};
template <>
struct DataTypeConverter<float, SoxrDataShape::Interleaved> {
    static const soxr::soxr_datatype_t value = soxr::SOXR_FLOAT32_I;
};
template <>
struct DataTypeConverter<const float, SoxrDataShape::Interleaved> {
    static const soxr::soxr_datatype_t value = soxr::SOXR_FLOAT32_I;
};
template <>
struct DataTypeConverter<double, SoxrDataShape::Interleaved> {
    static const soxr::soxr_datatype_t value = soxr::SOXR_FLOAT64_I;
};
template <>
struct DataTypeConverter<const double, SoxrDataShape::Interleaved> {
    static const soxr::soxr_datatype_t value = soxr::SOXR_FLOAT64_I;
};
template <>
struct DataTypeConverter<int32_t, SoxrDataShape::Interleaved> {
    static const soxr::soxr_datatype_t value = soxr::SOXR_INT32_I;
};
template <>
struct DataTypeConverter<const int32_t, SoxrDataShape::Interleaved> {
    static const soxr::soxr_datatype_t value = soxr::SOXR_INT32_I;
};
template <>
struct DataTypeConverter<int16_t, SoxrDataShape::Interleaved> {
    static const soxr::soxr_datatype_t value = soxr::SOXR_INT16_I;
};
template <>
struct DataTypeConverter<const int16_t, SoxrDataShape::Interleaved> {
    static const soxr::soxr_datatype_t value = soxr::SOXR_INT16_I;
};
template <>
struct DataTypeConverter<float, SoxrDataShape::Split> {
    static const soxr::soxr_datatype_t value = soxr::SOXR_FLOAT32_S;
};
template <>
struct DataTypeConverter<const float, SoxrDataShape::Split> {
    static const soxr::soxr_datatype_t value = soxr::SOXR_FLOAT32_S;
};
template <>
struct DataTypeConverter<double, SoxrDataShape::Split> {
    static const soxr::soxr_datatype_t value = soxr::SOXR_FLOAT64_S;
};
template <>
struct DataTypeConverter<const double, SoxrDataShape::Split> {
    static const soxr::soxr_datatype_t value = soxr::SOXR_FLOAT64_S;
};
template <>
struct DataTypeConverter<int32_t, SoxrDataShape::Split> {
    static const soxr::soxr_datatype_t value = soxr::SOXR_INT32_S;
};
template <>
struct DataTypeConverter<const int32_t, SoxrDataShape::Split> {
    static const soxr::soxr_datatype_t value = soxr::SOXR_INT32_S;
};
template <>
struct DataTypeConverter<int16_t, SoxrDataShape::Split> {
    static const soxr::soxr_datatype_t value = soxr::SOXR_INT16_S;
};
template <>
struct DataTypeConverter<const int16_t, SoxrDataShape::Split> {
    static const soxr::soxr_datatype_t value = soxr::SOXR_INT16_S;
};

template <size_t InputChannels,
          size_t OutputChannels,
          typename InputType,
          typename OutputType>
struct ConvertArrayTypesResult {
    soxr::soxr_in_t in;
    size_t ilen;
    soxr::soxr_out_t out;
    size_t olen;
    // Extends the lifetime of the pointer arrays built internally
    std::unique_ptr<std::array<InputType*, InputChannels>> ibuf_raw_ptr;
    std::unique_ptr<std::array<OutputType*, OutputChannels>> obuf_raw_ptr;

    ConvertArrayTypesResult(soxr::soxr_in_t in,
                            size_t ilen,
                            soxr::soxr_out_t out,
                            size_t olen,
                            std::unique_ptr<std::array<InputType*, InputChannels>> ibuf_raw_ptr,
                            std::unique_ptr<std::array<OutputType*, OutputChannels>> obuf_raw_ptr)
        : in(in)
        , ilen(ilen)
        , out(out)
        , olen(olen)
        , ibuf_raw_ptr(std::move(ibuf_raw_ptr))
        , obuf_raw_ptr(std::move(obuf_raw_ptr)) {}
};

template <size_t InputChannels,
          size_t OutputChannels,
          typename InputType,
          typename OutputType,
          size_t InputExtent,
          size_t OutputExtent>
ConvertArrayTypesResult<InputChannels, OutputChannels, InputType, OutputType> convert_array_types(
    unsigned num_channels,
    const std::array<std::span<InputType, InputExtent>, InputChannels>& ibuf,
    std::array<std::span<OutputType, OutputExtent>, OutputChannels>& obuf) //
{
    bool input_interleaved = InputChannels != num_channels || num_channels == 1;
    bool output_interleaved = OutputChannels != num_channels || num_channels == 1;
    if (!(InputChannels == 1 || InputChannels == num_channels) ||
        !(OutputChannels == 1 || OutputChannels == num_channels)) {
        throw SoxrError("Invalid channel specification");
    }
    // Convert the wrapper structs into nested pointers
    auto ibuf_raw_ptr = std::make_unique<std::array<InputType*, InputChannels>>();
    auto obuf_raw_ptr = std::make_unique<std::array<OutputType*, OutputChannels>>();
    for (size_t i = 0; i < InputChannels; i++) {
        (*ibuf_raw_ptr)[i] = ibuf[i].data();
    }
    for (size_t i = 0; i < OutputChannels; i++) {
        (*obuf_raw_ptr)[i] = obuf[i].data();
    }
    // Note: soxr represents interleaved and single-channel split differently!
    soxr::soxr_in_t in = input_interleaved ? (void*)(*ibuf_raw_ptr)[0] : (void*)ibuf_raw_ptr->data();
    soxr::soxr_out_t out = output_interleaved ? (void*)(*obuf_raw_ptr)[0] : (void*)obuf_raw_ptr->data();
    // Length is the number of samples, where one "sample" counts across channels
    size_t ilen = ibuf[0].size() / (input_interleaved ? num_channels : 1);
    size_t olen = obuf[0].size() / (output_interleaved ? num_channels : 1);
    return ConvertArrayTypesResult(in, ilen, out, olen, std::move(ibuf_raw_ptr), std::move(obuf_raw_ptr));
}

} // namespace detail

namespace SoxrIoFlags {
constexpr unsigned long TPDF = soxr::SOXR_TPDF;
constexpr unsigned long NoDither = soxr::SOXR_NO_DITHER;
} // namespace SoxrIoFlags
template <typename InputType,
          SoxrDataShape InputShape,
          typename OutputType,
          SoxrDataShape OutputShape>
struct SoxrIoSpec {
    double scale;
    unsigned long flags;

    inline SoxrIoSpec() {
        auto itype = detail::DataTypeConverter<InputType, InputShape>::value;
        auto otype = detail::DataTypeConverter<OutputType, OutputShape>::value;
        // Use soxr's error checking
        soxr::soxr_io_spec_t io_spec = soxr::soxr_io_spec(itype, otype);
        if (io_spec.e) {
            throw SoxrError(static_cast<const char*>(io_spec.e));
        }
        this->scale = 1;
        this->flags = 0;
    }

    inline soxr::soxr_io_spec_t c_struct() const noexcept {
        auto itype = detail::DataTypeConverter<InputType, InputShape>::value;
        auto otype = detail::DataTypeConverter<OutputType, OutputShape>::value;
        return (soxr::soxr_io_spec_t){
            .itype = itype,
            .otype = otype,
            .scale = this->scale,
            .flags = this->flags,
        };
    }
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
        soxr::soxr_quality_spec_t quality_spec =
            soxr::soxr_quality_spec(static_cast<unsigned long>(recipe), flags);
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

template <typename InputType = float,
          typename OutputType = float,
          SoxrDataShape InputShape = SoxrDataShape::Interleaved,
          SoxrDataShape OutputShape = SoxrDataShape::Interleaved>
class SoxResampler {
  private:
    soxr::soxr_t m_soxr{nullptr};

  public:
    inline SoxResampler(
        double input_rate,
        double output_rate,
        unsigned int num_channels,
        const SoxrIoSpec<InputType, InputShape, OutputType, OutputShape>& io_spec =
            SoxrIoSpec<float, SoxrDataShape::Interleaved, float, SoxrDataShape::Interleaved>(),
        const SoxrQualitySpec& quality_spec = SoxrQualitySpec(SoxrQualityRecipe::High, 0),
        const SoxrRuntimeSpec& runtime_spec = SoxrRuntimeSpec(1)) //
    {
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

template <size_t InputChannels,
          size_t OutputChannels,
          typename InputType = float,
          typename OutputType = float,
          size_t InputExtent = std::dynamic_extent,
          size_t OutputExtent = std::dynamic_extent,
          SoxrDataShape InputShape = SoxrDataShape::Interleaved,
          SoxrDataShape OutputShape = SoxrDataShape::Interleaved>
inline void oneshot(
    double input_rate,
    double output_rate,
    unsigned num_channels,
    const std::array<std::span<InputType, InputExtent>, InputChannels>& ibuf,
    size_t* idone,
    std::array<std::span<OutputType, OutputExtent>, OutputChannels>& obuf,
    size_t* odone,
    const SoxrIoSpec<InputType, InputShape, OutputType, OutputShape>& io_spec =
        SoxrIoSpec<float, SoxrDataShape::Interleaved, float, SoxrDataShape::Interleaved>(),
    const SoxrQualitySpec& quality_spec = SoxrQualitySpec(SoxrQualityRecipe::Low, 0),
    const SoxrRuntimeSpec& runtime_spec = SoxrRuntimeSpec(1)) //
{
    bool input_interleaved = InputChannels != num_channels || num_channels == 1;
    bool output_interleaved = OutputChannels != num_channels || num_channels == 1;
    if ((input_interleaved && InputShape != SoxrDataShape::Interleaved) || (!input_interleaved && InputShape != SoxrDataShape::Split)) {
        throw SoxrError("Input buffer shape does not match the shape from the provided io_spec");
    }
    if ((output_interleaved && OutputShape != SoxrDataShape::Interleaved) || (!output_interleaved && OutputShape != SoxrDataShape::Split)) {
        throw SoxrError("Output buffer shape does not match the shape from the provided io_spec");
    }

    soxr::soxr_io_spec_t io_spec_raw = io_spec.c_struct();
    soxr::soxr_quality_spec_t quality_spec_raw = quality_spec.c_struct();
    soxr::soxr_runtime_spec_t runtime_spec_raw = runtime_spec.c_struct();
    detail::ConvertArrayTypesResult res = detail::convert_array_types(num_channels, ibuf, obuf);
    soxr::soxr_error_t err = soxr::soxr_oneshot(input_rate, output_rate, num_channels, res.in, res.ilen, idone, res.out, res.olen, odone, &io_spec_raw, &quality_spec_raw, &runtime_spec_raw);
    if (err != 0) {
        throw soxrpp::SoxrError(err);
    }
}

// Convenience signature (flat input buffer)
template <size_t OutputChannels,
          typename InputType = float,
          typename OutputType = float,
          size_t InputExtent = std::dynamic_extent,
          size_t OutputExtent = std::dynamic_extent,
          SoxrDataShape InputShape = SoxrDataShape::Interleaved,
          SoxrDataShape OutputShape = SoxrDataShape::Interleaved>
inline void oneshot(double input_rate, double output_rate, unsigned num_channels, const std::span<InputType, InputExtent>& ibuf, size_t* idone, std::array<std::span<OutputType, OutputExtent>, OutputChannels>& obuf, size_t* odone, const SoxrIoSpec<InputType, InputShape, OutputType, OutputShape>& io_spec = SoxrIoSpec<float, SoxrDataShape::Interleaved, float, SoxrDataShape::Interleaved>(), const SoxrQualitySpec& quality_spec = SoxrQualitySpec(SoxrQualityRecipe::Low, 0), const SoxrRuntimeSpec& runtime_spec = SoxrRuntimeSpec(1)) {
    return oneshot<1, OutputChannels>(input_rate, output_rate, num_channels, std::array{ibuf}, idone, obuf, odone, io_spec, quality_spec, runtime_spec);
}
// Convenience signature (flat output buffer)
template <size_t InputChannels,
          typename InputType = float,
          typename OutputType = float,
          size_t InputExtent = std::dynamic_extent,
          size_t OutputExtent = std::dynamic_extent,
          SoxrDataShape InputShape = SoxrDataShape::Interleaved,
          SoxrDataShape OutputShape = SoxrDataShape::Interleaved>
inline void oneshot(double input_rate, double output_rate, unsigned num_channels, const std::array<std::span<InputType, InputExtent>, InputChannels>& ibuf, size_t* idone, std::span<OutputType, OutputExtent>& obuf, size_t* odone, const SoxrIoSpec<InputType, InputShape, OutputType, OutputShape>& io_spec = SoxrIoSpec<float, SoxrDataShape::Interleaved, float, SoxrDataShape::Interleaved>(), const SoxrQualitySpec& quality_spec = SoxrQualitySpec(SoxrQualityRecipe::Low, 0), const SoxrRuntimeSpec& runtime_spec = SoxrRuntimeSpec(1)) {
    return oneshot<InputChannels, 1>(input_rate, output_rate, num_channels, ibuf, idone, std::array{obuf}, odone, io_spec, quality_spec, runtime_spec);
}

// Convenience signature (both flat buffers)
template <typename InputType = float,
          typename OutputType = float,
          size_t InputExtent = std::dynamic_extent,
          size_t OutputExtent = std::dynamic_extent,
          SoxrDataShape InputShape = SoxrDataShape::Interleaved,
          SoxrDataShape OutputShape = SoxrDataShape::Interleaved>
inline void oneshot(double input_rate, double output_rate, unsigned num_channels, const std::span<InputType, InputExtent>& ibuf, size_t* idone, std::span<OutputType, OutputExtent>& obuf, size_t* odone, const SoxrIoSpec<InputType, InputShape, OutputType, OutputShape>& io_spec = SoxrIoSpec<float, SoxrDataShape::Interleaved, float, SoxrDataShape::Interleaved>(), const SoxrQualitySpec& quality_spec = SoxrQualitySpec(SoxrQualityRecipe::Low, 0), const SoxrRuntimeSpec& runtime_spec = SoxrRuntimeSpec(1)) {
    return oneshot<1, 1>(input_rate, output_rate, num_channels, std::array{ibuf}, idone, std::array{obuf}, odone, io_spec, quality_spec, runtime_spec);
}

} // namespace soxrpp

#undef soxr_datatype_size
#undef soxr_included
#undef soxr_strerror