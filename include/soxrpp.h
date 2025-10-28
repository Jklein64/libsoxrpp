#pragma once

#include <any>
#include <array>
#include <concepts>
#include <memory>
#include <optional>
#include <span>
#include <string>
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

enum class SoxrDataShape { Interleaved, Split };

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

} // namespace detail

namespace SoxrIoFlags {
constexpr unsigned long TPDF = soxr::SOXR_TPDF;
constexpr unsigned long NoDither = soxr::SOXR_NO_DITHER;
} // namespace SoxrIoFlags
template <typename InputType, SoxrDataShape InputShape, typename OutputType, SoxrDataShape OutputShape>
struct SoxrIoSpec {
    double scale;
    unsigned long flags;

    SoxrIoSpec() {
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

    soxr::soxr_io_spec_t c_struct() const noexcept {
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

    SoxrQualitySpec() noexcept {
        this->precision = 20;
        this->phase_response = 50;
        this->passband_end = 0.913;
        this->stopband_begin = 1;
        this->flags = 0;
    }

    SoxrQualitySpec(SoxrQualityRecipe recipe, unsigned long flags) {
        soxr::soxr_quality_spec_t quality_spec = soxr::soxr_quality_spec(static_cast<unsigned long>(recipe), flags);
        if (quality_spec.e) {
            throw SoxrError(static_cast<const char*>(quality_spec.e));
        }
        this->precision = quality_spec.precision;
        this->phase_response = quality_spec.phase_response;
        this->passband_end = quality_spec.passband_end;
        this->stopband_begin = quality_spec.stopband_begin;
        this->flags = quality_spec.flags;
    }

    soxr::soxr_quality_spec_t c_struct() const noexcept {
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

    SoxrRuntimeSpec() noexcept {
        this->log2_min_dft_size = 10;
        this->log2_large_dft_size = 17;
        this->coef_size_kbytes = 400;
        this->num_threads = 1;
        this->flags = 0;
    }

    SoxrRuntimeSpec(unsigned int num_threads) noexcept {
        soxr::soxr_runtime_spec_t runtime_spec = soxr::soxr_runtime_spec(num_threads);
        this->log2_min_dft_size = runtime_spec.log2_min_dft_size;
        this->log2_large_dft_size = runtime_spec.log2_large_dft_size;
        this->coef_size_kbytes = runtime_spec.coef_size_kbytes;
        this->num_threads = runtime_spec.num_threads;
        this->flags = runtime_spec.flags;
    }

    soxr::soxr_runtime_spec_t c_struct() const noexcept {
        return (soxr::soxr_runtime_spec_t){
            .log2_min_dft_size = this->log2_min_dft_size,
            .log2_large_dft_size = this->log2_large_dft_size,
            .coef_size_kbytes = this->coef_size_kbytes,
            .num_threads = this->num_threads,
            .flags = this->flags,
        };
    }
};

template <typename Type, size_t Channels = 1, size_t Extent = std::dynamic_extent>
class SoxrBuffer {
  private:
    Type* m_data[Channels];
    size_t m_size;

  public:
    static constexpr size_t channels = Channels;
    static constexpr size_t extent = Extent;

    SoxrBuffer(std::array<std::span<Type, Extent>, Channels> buffers)
        : m_size(buffers[0].size()) //
    {
        for (size_t i = 0; i < Channels; i++) {
            m_data[i] = buffers[i].data();
        }
    }

    SoxrBuffer(std::array<Type*, Channels> ptrs, size_t size)
        : m_size(size) //
    {
        for (size_t i = 0; i < Channels; i++) {
            m_data[i] = ptrs[i];
        }
    }

    // NB: don't forget to set the Channels template parameter!
    SoxrBuffer(Type** ptrs, size_t size)
        : m_size(size) //
    {
        for (size_t i = 0; i < Channels; i++) {
            m_data[i] = ptrs[i];
        }
    }

    SoxrBuffer(std::span<Type, Extent> buffer)
        : m_size(buffer.size()) //
    {
        m_data[0] = buffer.data();
    }

    SoxrBuffer(Type* ptr, size_t size)
        : m_size(size) //
    {
        m_data[0] = ptr;
    }

    void* data(bool interleaved) const {
        // Yes, this casts away const-ness. Soxr will re-apply it where appropriate. C++-style casts fail
        // to remove the const-ness because the compiler gives m_data the type "Type* const*" due to the
        // compile-time constant Channels used in the array declaration, so the underlying type is const.
        return interleaved ? (void*)(m_data[0]) : (void*)(m_data);
    }

    size_t size(bool interleaved, unsigned int num_channels) const {
        return interleaved ? m_size / num_channels : m_size;
    }
};

template <typename InputType = float,
          typename OutputType = float,
          SoxrDataShape InputShape = SoxrDataShape::Interleaved,
          SoxrDataShape OutputShape = SoxrDataShape::Interleaved>
class SoxResampler {
    // If OutputType is const, then process() will segfault when it tries to write
    static_assert(!std::is_const<OutputType>::value, "OutputType cannot be const");

  private:
    soxr::soxr_t m_soxr{nullptr};
    unsigned int m_num_channels;

    struct InputFnContext {
        // Type-erased but memory-managed pointer to a copy of the input_fn lambda
        std::unique_ptr<void, void (*)(void*)> fn{nullptr, +[](void*) {}};
        unsigned int num_channels;
    } m_input_fn_context;

  public:
    SoxResampler(double input_rate,
                 double output_rate,
                 unsigned int num_channels,
                 const SoxrIoSpec<InputType, InputShape, OutputType, OutputShape>& io_spec =
                     SoxrIoSpec<float, SoxrDataShape::Interleaved, float, SoxrDataShape::Interleaved>(),
                 const SoxrQualitySpec& quality_spec = SoxrQualitySpec(SoxrQualityRecipe::High, 0),
                 const SoxrRuntimeSpec& runtime_spec = SoxrRuntimeSpec(1))
        : m_num_channels(num_channels) //
    {
        soxr::soxr_error_t err;
        soxr::soxr_io_spec_t io_spec_raw = io_spec.c_struct();
        soxr::soxr_quality_spec_t quality_spec_raw = quality_spec.c_struct();
        soxr::soxr_runtime_spec_t runtime_spec_raw = runtime_spec.c_struct();
        m_soxr = soxr::soxr_create(input_rate, output_rate, num_channels, &err, &io_spec_raw, &quality_spec_raw, &runtime_spec_raw);
        if (err != 0) {
            throw SoxrError(err);
        }
    }

    ~SoxResampler() {
        soxr::soxr_delete(m_soxr);
    }

    template <size_t InputChannels,
              size_t OutputChannels,
              size_t InputExtent = std::dynamic_extent,
              size_t OutputExtent = std::dynamic_extent>
    std::pair<size_t, size_t> process(const SoxrBuffer<InputType, InputChannels, InputExtent>& ibuf,
                                      SoxrBuffer<OutputType, OutputChannels, OutputExtent>& obuf,
                                      bool done = false) {
        // Asserts "interleaved ==> single channel array"
        constexpr bool input_interleaved = InputShape == SoxrDataShape::Interleaved;
        constexpr bool output_interleaved = OutputShape == SoxrDataShape::Interleaved;
        static_assert(!input_interleaved || InputChannels == 1, "Input buffer has invalid shape");
        static_assert(!output_interleaved || OutputChannels == 1, "Output buffer has invalid shape");

        size_t idone, odone;
        soxr::soxr_error_t err = soxr::soxr_process(m_soxr,
                                                    done ? nullptr : ibuf.data(input_interleaved),
                                                    ibuf.size(input_interleaved, m_num_channels),
                                                    &idone,
                                                    obuf.data(output_interleaved),
                                                    obuf.size(output_interleaved, m_num_channels),
                                                    &odone);
        if (err != 0) {
            throw SoxrError(err);
        }

        return std::make_pair(idone, odone);
    }

    template <typename Lambda,
              typename Result = std::invoke_result_t<Lambda, size_t>,
              size_t Channels = Result::channels,
              size_t Extent = Result::extent>
        requires requires(Lambda&& input_fn, size_t len) {
            { input_fn(len) } -> std::convertible_to<SoxrBuffer<InputType, Channels, Extent>>;
        }
    void set_input_fn(Lambda&& input_fn, size_t max_ilen) {
        // See https://stackoverflow.com/a/20527578
        // Optimization; allows it decay to a function pointer
        using Func = typename std::decay<Lambda>::type;
        m_input_fn_context = (InputFnContext){
            .fn = std::move(std::unique_ptr<void, void (*)(void*)>(
                new Func(std::forward<Func>(input_fn)),
                +[](void* ptr) {
                    delete (Func*)ptr;
                })),
            .num_channels = m_num_channels,
        };
        soxr::soxr_set_input_fn(
            m_soxr,
            +[](void* context, soxrpp::soxr::soxr_cbuf_t* ibuf_internal, size_t len) {
                InputFnContext* input_fn_context = static_cast<InputFnContext*>(context);
                Func* func = static_cast<Func*>(input_fn_context->fn.get());
                SoxrBuffer<InputType, Channels, Extent> ibuf = (*func)(len);
                constexpr bool interleaved = InputShape == SoxrDataShape::Interleaved;
                size_t ilen = ibuf.size(interleaved, input_fn_context->num_channels);
                if (ilen > 0) {
                    *ibuf_internal = ibuf.data(interleaved);
                }
                return ilen;
            },
            &m_input_fn_context,
            max_ilen);
    }

    template <size_t Channels = 1, size_t Extent = std::dynamic_extent>
    size_t output(SoxrBuffer<OutputType, Channels, Extent> obuf) {
        constexpr bool interleaved = OutputShape == SoxrDataShape::Interleaved;
        size_t odone = soxr::soxr_output(m_soxr, obuf.data(interleaved), obuf.size(interleaved, m_num_channels));
        soxr::soxr_error_t err = soxr::soxr_error(m_soxr);
        if (err != 0) {
            throw SoxrError(err);
        }
        return odone;
    }

    std::optional<std::string> error() noexcept {
        soxr::soxr_error_t err = soxr_error(m_soxr);
        return err == 0 ? std::nullopt : std::make_optional(err);
    }

    size_t* num_clips() noexcept {
        return soxr::soxr_num_clips(m_soxr);
    }

    double delay() noexcept {
        return soxr::soxr_delay(m_soxr);
    }

    char const* engine() noexcept {
        return soxr::soxr_engine(m_soxr);
    }

    void clear() {
        soxr::soxr_error_t err = soxr::soxr_clear(m_soxr);
        if (err != 0) {
            throw SoxrError(err);
        }
    }

    void set_io_ratio(double io_ratio, size_t slew_len) {
        soxr::soxr_error_t err = soxr::soxr_set_io_ratio(m_soxr, io_ratio, slew_len);
        if (err != 0) {
            throw SoxrError(err);
        }
    }

    void set_num_channels(unsigned int num_channels) {
        soxr::soxr_error_t err = soxr::soxr_set_num_channels(m_soxr, num_channels);
        if (err != 0) {
            throw SoxrError(err);
        }

        m_num_channels = num_channels;
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
inline std::pair<size_t, size_t> oneshot(double input_rate,
                                         double output_rate,
                                         unsigned int num_channels,
                                         const SoxrBuffer<InputType, InputChannels, InputExtent>& ibuf,
                                         SoxrBuffer<OutputType, OutputChannels, OutputExtent>& obuf,
                                         const SoxrIoSpec<InputType, InputShape, OutputType, OutputShape>& io_spec =
                                             SoxrIoSpec<float, SoxrDataShape::Interleaved, float, SoxrDataShape::Interleaved>(),
                                         const SoxrQualitySpec& quality_spec = SoxrQualitySpec(SoxrQualityRecipe::Low, 0),
                                         const SoxrRuntimeSpec& runtime_spec = SoxrRuntimeSpec(1)) {
    // Asserts "interleaved ==> single channel array"
    constexpr bool input_interleaved = InputShape == SoxrDataShape::Interleaved;
    constexpr bool output_interleaved = OutputShape == SoxrDataShape::Interleaved;
    static_assert(!input_interleaved || InputChannels == 1, "Input buffer has invalid shape");
    static_assert(!output_interleaved || OutputChannels == 1, "Output buffer has invalid shape");

    size_t idone, odone;
    soxr::soxr_io_spec_t io_spec_raw = io_spec.c_struct();
    soxr::soxr_quality_spec_t quality_spec_raw = quality_spec.c_struct();
    soxr::soxr_runtime_spec_t runtime_spec_raw = runtime_spec.c_struct();
    soxr::soxr_error_t err = soxr::soxr_oneshot(input_rate,
                                                output_rate,
                                                num_channels,
                                                ibuf.data(input_interleaved),
                                                ibuf.size(input_interleaved, num_channels),
                                                &idone,
                                                obuf.data(output_interleaved),
                                                obuf.size(output_interleaved, num_channels),
                                                &odone,
                                                &io_spec_raw,
                                                &quality_spec_raw,
                                                &runtime_spec_raw);
    if (err != 0) {
        throw soxrpp::SoxrError(err);
    }

    return std::make_pair(idone, odone);
}

} // namespace soxrpp

#undef soxr_datatype_size
#undef soxr_included
#undef soxr_strerror