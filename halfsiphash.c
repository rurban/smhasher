#include "siphash.h"
#include "siphash_impl.h"

#define TRACE
#define cROUNDS 2
#define dROUNDS 4
#define ROTL(x, b) (uint32_t)(((x) << (b)) | ((x) >> (32 - (b))))
#define U8TO32_LE(p)                                                    \
    (((uint32_t)((p)[0])) | ((uint32_t)((p)[1]) << 8) |                 \
     ((uint32_t)((p)[2]) << 16) | ((uint32_t)((p)[3]) << 24))
#define SIPROUND                                       \
    do {                                               \
        v0 += v1;                                      \
        v1 = ROTL(v1, 5);                              \
        v1 ^= v0;                                      \
        v0 = ROTL(v0, 16);                             \
        v2 += v3;                                      \
        v3 = ROTL(v3, 8);                              \
        v3 ^= v2;                                      \
        v0 += v3;                                      \
        v3 = ROTL(v3, 7);                              \
        v3 ^= v0;                                      \
        v2 += v1;                                      \
        v1 = ROTL(v1, 13);                             \
        v1 ^= v2;                                      \
        v2 = ROTL(v2, 16);                             \
    } while (0)

/* the faster half 32bit variant for the linux kernel */
uint32_t
halfsiphash(const unsigned char key[16], const unsigned char *m, size_t len) {
    uint32_t v0 = 0;
    uint32_t v1 = 0;
    uint32_t v2 = 0x6c796765;
    uint32_t v3 = 0x74656462;
    uint32_t k0 = U8TO32_LE(key);
    uint32_t k1 = U8TO32_LE(key + 8);
    uint32_t mi;
    const uint8_t *end = m + len - (len % sizeof(uint32_t));
    const int left = len & 3;
    uint32_t b = ((uint32_t)len) << 24;
    v3 ^= k1;
    v2 ^= k0;
    v1 ^= k1;
    v0 ^= k0;

    for (; m != end; m += 4) {
        mi = U8TO32_LE(m);
        v3 ^= mi;
        SIPROUND;
        SIPROUND;
        v0 ^= mi;
    }

    switch (left) {
    case 3:
        b |= ((uint32_t)m[2]) << 16;
    case 2:
        b |= ((uint32_t)m[1]) << 8;
    case 1:
        b |= ((uint32_t)m[0]);
        break;
    case 0:
        break;
    }

    v3 ^= b;
    SIPROUND;
    SIPROUND;
    v0 ^= b;
    v2 ^= 0xff;
    SIPROUND;
    SIPROUND;
    SIPROUND;
    SIPROUND;
    return v1 ^ v3;
}

