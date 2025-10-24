#include "libsoxrpp/libsoxrpp.h"
#include "soxr.h"

#include <array>

namespace libsoxrpp {

const std::array<float, 48> in = {/* Input: 12 cycles of a sine wave with freq. = irate/4 */
                                  0, 1, 0, -1, 0, 1, 0, -1, 0, 1, 0, -1, 0, 1, 0, -1, 0, 1, 0, -1, 0, 1, 0, -1,
                                  0, 1, 0, -1, 0, 1, 0, -1, 0, 1, 0, -1, 0, 1, 0, -1, 0, 1, 0, -1, 0, 1, 0, -1};

bool example() {
    double irate = 1; /* Default to interpolation */
    double orate = 2; /* by a factor of 2. */

    size_t olen = (size_t)(in.size() * orate / irate + .5); /* Assay output len. */
    float* out = (float*)malloc(sizeof(*out) * olen);       /* Allocate output buffer. */
    size_t odone;

    soxr_error_t error = soxr_oneshot(irate, orate, 1,            /* Rates and # of chans. */
                                      in.data(), in.size(), NULL, /* Input. */
                                      out, olen, &odone,          /* Output. */
                                      NULL, NULL, NULL);          /* Default configuration.*/

    unsigned i = 0; /* Print out the resampled data, */
    while (i++ < odone)
        printf("%5.2f%c", out[i - 1], " \n"[!(i & 7) || i == odone]);
    printf("%-26s %s\n", "prog", soxr_strerror(error)); /* and reported result. */

    /* Library version check: */
    printf("runtime=%s API=" SOXR_THIS_VERSION_STR "\n", soxr_version());

    free(out); /* Tidy up. */
    return !!error;
}
} // namespace libsoxrpp