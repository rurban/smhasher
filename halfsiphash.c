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


inline void
halfsiphash_seed_state(const unsigned char vseed[8], const unsigned char vstate[16]) {
    uint32_t *state= (uint32_t *)vstate;
    uint32_t *seed= (uint32_t *)vseed;

    state[0]= seed[0];
    state[1]= seed[1];
    state[2]= seed[0] ^ 0x6c796765;
    state[3]= seed[1] ^ 0x74656462;
}

/* the faster half 32bit variant for the linux kernel (with tweaks for the with state api)*/
uint32_t
halfsiphash_with_state(const unsigned char state[16], const unsigned char *m, size_t len) {
    uint32_t v0 = U8TO32_LE(state);
    uint32_t v1 = U8TO32_LE(state + 4);
    uint32_t v2 = U8TO32_LE(state + 8);
    uint32_t v3 = U8TO32_LE(state + 12);
    uint32_t mi;
    int i;
    const uint8_t *end = m + len - (len % sizeof(uint32_t));
    const int left = len & 3;
    uint32_t b = ((uint32_t)len) << 24;

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

uint32_t
halfsiphash(const unsigned char key[8], const unsigned char *m, size_t len) {
    uint32_t state[4];
    halfsiphash_seed_state(key,(unsigned char *)state);
    return halfsiphash_with_state((unsigned char *)state,m,len);
}
