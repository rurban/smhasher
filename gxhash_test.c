#include "gxhash.h"
#include <stdint.h>
#include <stdio.h>

int main(void)
{
    static const int lengths[] = {32, 64, 256, 513};
    uint8_t input[544] = {0};
    for (unsigned n = 0; n < sizeof(lengths) / sizeof(lengths[0]); ++n) {
        int len = lengths[n];
        uint32_t base32 = gxhash32(input, len, 123);
        uint64_t base64 = gxhash64(input, len, 123);
        for (int i = 0; i < len; ++i) {
            input[i] = 1;
            uint32_t changed32 = gxhash32(input, len, 123);
            uint64_t changed64 = gxhash64(input, len, 123);
            input[i] = 0;
            if (changed32 == base32 || changed64 == base64) {
                fprintf(stderr, "gxhash ignored byte %d of %d\n", i, len);
                return 1;
            }
        }
    }
    return 0;
}
