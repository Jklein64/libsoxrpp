#include "soxrpp.h"

#include <cstring>
#include <vector>

int main(int n, char const* arg[]) {
    const char *const arg0 = n ? --n, *arg++ : "";
    const double irate = n ? --n, atof(*arg++) : 96000.;
    const double orate = n ? --n, atof(*arg++) : 44100.;
    const unsigned int num_channels = n ? --n, (unsigned)atoi(*arg++) : 1;
    // Samples per channel
    const size_t buf_total_len = 15000;
    // Assume default IO layout (interleaved float in/out) for simplicity
    size_t const olen = std::clamp((size_t)(orate * buf_total_len / (irate + orate) + .5), 1UL, buf_total_len - 1);
    size_t const ilen = buf_total_len - olen;
    std::vector<float> obuf(olen, 0.0);
    std::vector<float> ibuf(ilen, 0.0);

    size_t odone, clips = 0;

    auto soxr = soxrpp::SoxResampler(irate, orate, num_channels);
    // Register a provider function that reads from stdin when it needs to
    soxr.set_input_fn(
        [&ibuf](size_t requested_len) {
            size_t ilen = fread(ibuf.data(), sizeof(float), requested_len, stdin);
            if (ilen == 0 && ferror(stdin)) {
                throw soxrpp::SoxrError("Error when reading from stdin");
            } else {
                return soxrpp::SoxrBuffer(ibuf.data(), ilen);
            }
        },
        ilen);

    try {
        do {
            odone = soxr.output(obuf.data(), olen);
        } while (fwrite(obuf.data(), sizeof(float), odone, stdout));
    } catch (soxrpp::SoxrError& err) {
        fprintf(stderr, "Error! %s\n", err.what());
    }

    clips = *soxr.num_clips();

    fprintf(stderr,
            "%-26s %s; %lu clips; I/O: %s (%s)\n",
            arg0,
            "no error",
            (long unsigned)clips,
            ferror(stdin) || ferror(stdout) ? strerror(errno) : "no error",
            soxr.engine());

    return 0;
}