#include "soxrpp.h"

#include <iostream>
#include <span>
#include <vector>

int main(int argc, char const* arg[]) {
    const double irate = argc > 1 ? atof(arg[1]) : 96000.;
    const double orate = argc > 2 ? atof(arg[2]) : 44100.;

    const size_t buf_total_len = 15000;
    const size_t olen = (size_t)(orate * buf_total_len / (irate + orate) + .5);
    const size_t ilen = buf_total_len - olen;
    const size_t osize = sizeof(float), isize = osize;
    std::vector<float> obuf(olen);
    std::vector<float> ibuf(ilen);
    // Loop state variables
    size_t written;
    bool need_input = true;
    bool done = false;

    try {
        /* Create a stream resampler: */
        auto soxr = soxrpp::SoxResampler(irate, orate, 1);
        do {
            size_t ilen1 = 0;
            if (need_input) {
                /* Read one block into the buffer, ready to be resampled: */
                ilen1 = fread((void*)ibuf.data(), isize, ilen, stdin);
                if (!ilen1) { /* If the is no (more) input data available, */
                    done = true;
                }
            }

            /* Copy data from the input buffer into the resampler, and resample
             * to produce as much output as is possible to the given output buffer: */
            auto ibuf_soxr = soxrpp::SoxrBuffer(ibuf.data(), ilen1);
            auto obuf_soxr = soxrpp::SoxrBuffer(obuf.data(), olen);
            auto [idone, odone] = soxr.process(ibuf_soxr, obuf_soxr, done);
            written = fwrite((void*)obuf.data(), osize, odone, stdout); /* Consume output.*/
            /* If the actual amount of data output is less than that requested, and
             * we have not already reached the end of the input data, then supply some
             * more input next time round the loop: */
            need_input = odone < olen && !done;
        } while (need_input || written);
    } catch (const soxrpp::SoxrError& err) {
        printf("oh no! %s\n", err.what());
        return 1;
    }

    return 0;
}