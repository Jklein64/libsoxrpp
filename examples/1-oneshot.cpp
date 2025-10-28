#include "soxrpp.h"

#include <array>
#include <iostream>
#include <vector>

int main() {
    const std::array<float, 48> in = {0, 1, 0, -1, 0, 1, 0, -1, 0, 1, 0, -1, 0, 1, 0, -1, 0, 1, 0, -1, 0, 1, 0, -1,
                                      0, 1, 0, -1, 0, 1, 0, -1, 0, 1, 0, -1, 0, 1, 0, -1, 0, 1, 0, -1, 0, 1, 0, -1};
    // Interpolate by a factor of 2
    double irate = 1;
    double orate = 2;

    size_t olen = (size_t)((in.size() / 2) * orate / irate + .5);
    std::vector<int32_t> outl(olen);
    std::vector<int32_t> outr(olen);

    auto ibuf = soxrpp::SoxrBuffer(std::span{in});
    auto obuf = soxrpp::SoxrBuffer(std::array{std::span{outl}, std::span{outr}});

    try {
        soxrpp::SoxrIoSpec<const float, soxrpp::SoxrDataShape::Interleaved, int32_t, soxrpp::SoxrDataShape::Split> io_spec;
        auto [_, odone] = soxrpp::oneshot(irate, orate, 2, ibuf, obuf, io_spec);

        size_t i = 0;
        while (i++ < odone) {
            printf("(%5.2f, %5.2f)%c", outl[i - 1] / 2147483647.0, outr[i - 1] / 2147483647.0, " \n"[!(i & 7) || i == odone]);
        }
        puts("done!");
    } catch (const soxrpp::SoxrError& e) {
        printf("exception: %s\n", e.what());
    }

    return 0;
}