#include "soxrpp.h"

#include <array>
#include <functional>
#include <iostream>
#include <optional>
#include <vector>

int main() {
    std::cout << "Hello world!" << std::endl;

    const std::array<float, 48> in = {/* Input: 12 cycles of a sine wave with freq. = irate/4 */
                                      0, 1, 0, -1, 0, 1, 0, -1, 0, 1, 0, -1, 0, 1, 0, -1, 0, 1, 0, -1, 0, 1, 0, -1,
                                      0, 1, 0, -1, 0, 1, 0, -1, 0, 1, 0, -1, 0, 1, 0, -1, 0, 1, 0, -1, 0, 1, 0, -1};
    double irate = 1; /* Default to interpolation */
    double orate = 2; /* by a factor of 2. */

    size_t olen = (size_t)((in.size() / 2) * orate / irate + .5); /* Assay output len. */
    // size_t olen = (size_t)((in.size()) * orate / irate + .5); /* Assay output len. */

    // std::vector<std::vector<int32_t>> out(2);
    // out[0].resize(olen);
    // out[1].resize(olen);
    std::vector<float> out(olen);
    std::vector<int32_t> outl(olen);
    std::vector<int32_t> outr(olen);

    // auto obuf = std::array{outl.data(), outr.data()};
    auto obuf = std::array{std::span{outl}, std::span{outr}};
    // auto obuf = std::array{std::span{out}};
    // std::span in_span(in);
    // const auto ibuf = std::vector{in_span};
    // std::array<std::span<float>, 1> arr = {std::span{out}};
    // std::span inn_span(in_span);
    // std::span<std::span<float, 48>, 1>(std::span(in));
    // auto ibuf = soxrpp::SoxrBuffer<const float, 48, std::allocator<std::span<const float, 48>>>(bb);
    // auto obuf = soxrpp::SoxrBuffer(out);
    // auto ibuf = soxrpp::SoxrBuffer(soxrpp::SoxrDataType::Float32_I, std::span{int});
    // auto ibuf = soxrpp::SoxrBuffer<soxrpp::SoxrDataType::Float32_I, const float, 48>(std::span{in});
    // auto obuf = soxrpp::SoxrBuffer<soxrpp::SoxrDataType::Float32_I, float, std::dynamic_extent>(std::span<float>(out));

    // float* out = (float*)malloc(sizeof(*out) * olen); /* Allocate output buffer. */
    size_t odone;

    try {
        soxrpp::SoxrIoSpec<soxrpp::SoxrDataType::Float32_I, soxrpp::SoxrDataType::Int32_S> io_spec;
        // soxrpp::SoxrIoSpec<soxrpp::SoxrDataType::Float32_I, soxrpp::SoxrDataType::Float32_I> io_spec;
        soxrpp::oneshot(irate, orate, 2, std::array{std::span{in}}, NULL, obuf, &odone, io_spec);
        // soxrpp::oneshot(irate, orate, 1, std::array{std::span{in}}, NULL, obuf, &odone, io_spec);
        // soxrpp::oneshot(irate, orate, 2,                /* Rates and # of chans. */
        //                 in.data(), in.size() / 2, NULL, /* Input. */
        //                 obuf.data(), olen, &odone, io_spec);

        unsigned i = 0; /* Print out the resampled data, */
        while (i++ < odone)
            // printf("%5.2f%c", out[i - 1], " \n"[!(i & 7) || i == odone]);
            // printf("(%5.2f, %5.2f)%c", outl[i - 1], outr[i - 1], " \n"[!(i & 7) || i == odone]);
            printf("(%5.2f, %5.2f)%c", outl[i - 1] / 2147483647.0, outr[i - 1] / 2147483647.0, " \n"[!(i & 7) || i == odone]);
        puts("done!");
    } catch (const soxrpp::SoxrError& e) {
        printf("exception: %s\n", e.what());
    }
    // free(out); /* Tidy up. */
    return 0;
}