#include "siphash.h"
#include "siphash_impl.h"

#define U8TO64_LE(p) *(uint64_t *)(p)

#define SIPHASH_MIX(v0,v1,v2,v3,rounds)     \
do {                                        \
    int round = rounds;                     \
    do {                                    \
        v0 += v1;                           \
        v2 += v3;                           \
        v1 = ROTL64(v1,13) ^ v0;            \
        v3 = ROTL64(v3,16) ^ v2;            \
        v0 = ROTL64(v0,32) + v3;            \
        v2 += v1;                           \
        v1 = ROTL64(v1,17) ^ v2;            \
        v3 = ROTL64(v3,21) ^ v0;            \
        v2 = ROTL64(v2,32);                 \
    } while (--round);                      \
} while (0)

#define SIPHASH_MIX_VALUE(value,v0,v1,v2,v3,rounds) \
do {                                    \
    v3 ^= value;                        \
    SIPHASH_MIX(v0,v1,v2,v3,rounds);    \
    v0 ^= value;                        \
} while (0)

#define SIPHASH_SEED_STATE(key,v0,v1,v2,v3) \
do {                                    \
    v0 = v2 = U8TO64_LE(key + 0);       \
    v1 = v3 = U8TO64_LE(key + 8);       \
    v0 ^= 0x736f6d6570736575ull;        \
    v1 ^= 0x646f72616e646f6dull;        \
    v2 ^= 0x6c7967656e657261ull;        \
    v3 ^= 0x7465646279746573ull;        \
} while (0)

void
siphash_seed_state(const unsigned char seed[16], const unsigned char state[32]) {
    uint64_t *v= (uint64_t*)state;
    SIPHASH_SEED_STATE(seed,v[0],v[1],v[2],v[3]);
}

#define DECLARE_SIPHASH_FUNC(NAME,KEY_ROUNDS,FINAL_ROUNDS,SEED_SIZE)                \
uint64_t                                                                            \
NAME (const unsigned char key[SEED_SIZE], const unsigned char *m, size_t len) {     \
    uint64_t v0, v1, v2, v3;                                \
    uint64_t mi;                                            \
    uint64_t last7;                                         \
    size_t i, blocks;                                       \
                                                            \
    if (SEED_SIZE == 16) {                                  \
        SIPHASH_SEED_STATE(key,v0,v1,v2,v3);                \
    } else {                                                \
        v0 = U8TO64_LE(key +  0);                           \
        v1 = U8TO64_LE(key +  8);                           \
        v2 = U8TO64_LE(key + 16);                           \
        v3 = U8TO64_LE(key + 24);                           \
    }                                                       \
    last7 = (uint64_t)(len & 0xff) << 56;                   \
                                                            \
    for (i = 0, blocks = (len & ~7); i < blocks; i += 8) {  \
        mi = U8TO64_LE(m + i);                              \
        SIPHASH_MIX_VALUE(mi,v0,v1,v2,v3,KEY_ROUNDS);       \
    }                                                       \
                                                            \
    switch (len - blocks) {                                 \
        case 7: last7 |= (uint64_t)m[i + 6] << 48;          \
        case 6: last7 |= (uint64_t)m[i + 5] << 40;          \
        case 5: last7 |= (uint64_t)m[i + 4] << 32;          \
        case 4: last7 |= (uint64_t)m[i + 3] << 24;          \
        case 3: last7 |= (uint64_t)m[i + 2] << 16;          \
        case 2: last7 |= (uint64_t)m[i + 1] <<  8;          \
        case 1: last7 |= (uint64_t)m[i + 0]      ;          \
        case 0:                                             \
        default:;                                           \
    };                                                      \
                                                            \
    SIPHASH_MIX_VALUE(last7,v0,v1,v2,v3,KEY_ROUNDS);        \
                                                            \
    v2 ^= 0xff;                                             \
                                                            \
    SIPHASH_MIX(v0,v1,v2,v3,FINAL_ROUNDS);                  \
                                                            \
    return v0 ^ v1 ^ v2 ^ v3;                               \
}


DECLARE_SIPHASH_FUNC(siphash,2,4,16)
DECLARE_SIPHASH_FUNC(siphash_with_state,2,4,32)
DECLARE_SIPHASH_FUNC(siphash13,1,3,16)
DECLARE_SIPHASH_FUNC(siphash13_with_state,1,3,32)


#include "halfsiphash.c"
