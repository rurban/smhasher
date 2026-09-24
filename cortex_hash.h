/* ==============================================================================
 * CORTEX HASH — Ultra-High-Throughput Vectorized & SWAR Hashing Runtime
 * Patent Basis: CIPO CA 3,322,620 (Floria Laboratoire Inc.)
 * License: Floria Community Source License (FCSL 1.0) / Open Source Compatible
 * Authors: The Floria Authors (@cortexLab011)
 * ============================================================================== */

#ifndef CORTEX_HASH_H
#define CORTEX_HASH_H

#include <stdint.h>
#include <stddef.h>
#include <string.h>

#if defined(__x86_64__) || defined(_M_X64)
#if defined(__AVX2__)
#include <immintrin.h>
#endif
#endif

#ifdef __cplusplus
extern "C" {
#endif

static const uint64_t CORTEX_PRIME64_1 = 0x9E3779B185EBCA87ULL;
static const uint64_t CORTEX_PRIME64_2 = 0xC2B2AE3D27D4EB4FULL;
static const uint64_t CORTEX_PRIME64_3 = 0x165667B19E3779F9ULL;
static const uint64_t CORTEX_PRIME64_4 = 0x85EBCA77C2B2AE63ULL;
static const uint64_t CORTEX_PRIME64_5 = 0x27D4EB2F165667C5ULL;

static const uint64_t CORTEX_DEFAULT_SECRET[24] = {
    0xb8fe6c3923a44bbeULL, 0x7c01812cfa21ca11ULL, 0xc836146a61d44514ULL, 0xe110aa70b516460bULL,
    0x44e106a40ed54483ULL, 0x2cacb883cdb57827ULL, 0x14a502904e999f4fULL, 0x9e3779b185ebca87ULL,
    0xc2b2ae3d27d4eb4fULL, 0x165667b19e3779f9ULL, 0x85ebca77c2b2ae63ULL, 0x27d4eb2f165667c5ULL,
    0x3c79ac4f47b2c019ULL, 0x6fb300e176b15d6dULL, 0x18a241074bf7134bULL, 0xeb651e272104d0b9ULL,
    0xcd7618ef4c49a7f9ULL, 0x16f63d2422b449f6ULL, 0x68c208b73fa3b42dULL, 0x321a22b740306186ULL,
    0x9338808fb1647ff5ULL, 0x868c222f51459495ULL, 0x5e72f88ccf6bb02aULL, 0xbe92c449f6ff204eULL,
};

static inline uint64_t cortex_rot64(uint64_t x, int r) {
    return (x << r) | (x >> (64 - r));
}

static inline uint64_t cortex_rrmxmx(uint64_t h) {
    h ^= cortex_rot64(h, 49) ^ cortex_rot64(h, 24);
    h *= CORTEX_PRIME64_1;
    h ^= (h >> 35) + cortex_rot64(h, 28);
    h *= CORTEX_PRIME64_2;
    h ^= h >> 32;
    return h;
}

static inline uint64_t cortex_read_u64_le(const uint8_t *ptr) {
    uint64_t v;
    memcpy(&v, ptr, sizeof(v));
    return v;
}

static inline uint32_t cortex_read_u32_le(const uint8_t *ptr) {
    uint32_t v;
    memcpy(&v, ptr, sizeof(v));
    return v;
}

static inline uint64_t cortex_mul128_fold(uint64_t a, uint64_t b) {
#if defined(__SIZEOF_INT128__)
    unsigned __int128 p = (unsigned __int128)a * (unsigned __int128)b;
    return (uint64_t)p ^ (uint64_t)(p >> 64);
#else
    uint64_t a_lo = (uint32_t)a, a_hi = a >> 32;
    uint64_t b_lo = (uint32_t)b, b_hi = b >> 32;
    uint64_t p0 = a_lo * b_lo;
    uint64_t p1 = a_lo * b_hi;
    uint64_t p2 = a_hi * b_lo;
    uint64_t p3 = a_hi * b_hi;
    uint64_t cy = (p0 >> 32) + (uint32_t)p1 + (uint32_t)p2;
    uint64_t hi = p3 + (p1 >> 32) + (p2 >> 32) + (cy >> 32);
    uint64_t lo = (cy << 32) | (uint32_t)p0;
    return lo ^ hi;
#endif
}

static inline uint64_t cortex_hash_len_0_to_3(const uint8_t *data, size_t len, uint64_t seed) {
    if (len == 0) {
        return cortex_rrmxmx(seed ^ CORTEX_DEFAULT_SECRET[7]);
    }
    uint64_t c1 = data[0];
    uint64_t c2 = data[len >> 1];
    uint64_t c3 = data[len - 1];
    uint64_t combined = (c1 << 16) | (c2 << 8) | c3 | ((uint64_t)len << 24);
    uint64_t keyed = combined ^ (seed + CORTEX_DEFAULT_SECRET[0]);
    return cortex_rrmxmx(keyed * CORTEX_PRIME64_1);
}

static inline uint64_t cortex_hash_len_4_to_8(const uint8_t *data, size_t len, uint64_t seed) {
    uint64_t lo = cortex_read_u32_le(data);
    uint64_t hi = cortex_read_u32_le(data + len - 4);
    uint64_t in_val = lo | (hi << 32);
    uint64_t keyed = in_val ^ (CORTEX_DEFAULT_SECRET[1] + seed);
    uint64_t h = ((uint64_t)len * CORTEX_PRIME64_1) ^ keyed;
    return cortex_rrmxmx(h);
}

static inline uint64_t cortex_hash_len_9_to_16(const uint8_t *data, size_t len, uint64_t seed) {
    uint64_t lo = cortex_read_u64_le(data);
    uint64_t hi = cortex_read_u64_le(data + len - 8);
    uint64_t k1 = lo ^ CORTEX_DEFAULT_SECRET[2];
    uint64_t k2 = hi ^ (CORTEX_DEFAULT_SECRET[3] + seed);
    uint64_t folded = cortex_mul128_fold(k1, k2);
    uint64_t acc = (uint64_t)len + cortex_rot64(k1, 56) + k2 + folded;
    return cortex_rrmxmx(acc);
}

static inline uint64_t cortex_hash_len_17_to_64(const uint8_t *data, size_t len, uint64_t seed) {
    if (len >= 32) {
        uint64_t v1 = cortex_read_u64_le(data) ^ CORTEX_DEFAULT_SECRET[0];
        uint64_t v2 = cortex_read_u64_le(data + 8) ^ CORTEX_DEFAULT_SECRET[1];
        uint64_t v3 = cortex_read_u64_le(data + 16) ^ CORTEX_DEFAULT_SECRET[2];
        uint64_t v4 = cortex_read_u64_le(data + 24) ^ CORTEX_DEFAULT_SECRET[3];

        uint64_t v_end1 = cortex_read_u64_le(data + len - 32) ^ CORTEX_DEFAULT_SECRET[4];
        uint64_t v_end2 = cortex_read_u64_le(data + len - 24) ^ CORTEX_DEFAULT_SECRET[5];
        uint64_t v_end3 = cortex_read_u64_le(data + len - 16) ^ CORTEX_DEFAULT_SECRET[6];
        uint64_t v_end4 = cortex_read_u64_le(data + len - 8) ^ (CORTEX_DEFAULT_SECRET[7] + seed);

        uint64_t f1 = cortex_mul128_fold(v1, v2);
        uint64_t f2 = cortex_mul128_fold(v3, v4);
        uint64_t f3 = cortex_mul128_fold(v_end1, v_end2);
        uint64_t f4 = cortex_mul128_fold(v_end3, v_end4);

        uint64_t acc = ((uint64_t)len * CORTEX_PRIME64_1) + f1 + cortex_rot64(f2, 29) + cortex_rot64(f3, 37) + f4;
        return cortex_rrmxmx(acc);
    } else {
        uint64_t v1 = cortex_read_u64_le(data) ^ CORTEX_DEFAULT_SECRET[0];
        uint64_t v2 = cortex_read_u64_le(data + 8) ^ CORTEX_DEFAULT_SECRET[1];
        uint64_t v_end1 = cortex_read_u64_le(data + len - 16) ^ CORTEX_DEFAULT_SECRET[2];
        uint64_t v_end2 = cortex_read_u64_le(data + len - 8) ^ (CORTEX_DEFAULT_SECRET[3] + seed);

        uint64_t f1 = cortex_mul128_fold(v1, v2);
        uint64_t f2 = cortex_mul128_fold(v_end1, v_end2);

        uint64_t acc = ((uint64_t)len * CORTEX_PRIME64_2) + f1 + cortex_rot64(f2, 33) + v_end2;
        return cortex_rrmxmx(acc);
    }
}

static inline uint64_t cortex_hash64(const void *key, size_t len, uint64_t seed) {
    const uint8_t *data = (const uint8_t *)key;
    if (len <= 3) return cortex_hash_len_0_to_3(data, len, seed);
    if (len <= 8) return cortex_hash_len_4_to_8(data, len, seed);
    if (len <= 16) return cortex_hash_len_9_to_16(data, len, seed);
    if (len <= 64) return cortex_hash_len_17_to_64(data, len, seed);

#if defined(__AVX2__)
    size_t stripes = len / 64;
    size_t processed = stripes * 64;

    __m256i s_lo = _mm256_set1_epi64x((int64_t)(CORTEX_PRIME64_1 ^ seed));
    __m256i s_hi = _mm256_set1_epi64x((int64_t)(CORTEX_PRIME64_2 ^ seed));
    __m256i p1_v = _mm256_set1_epi64x((int64_t)CORTEX_PRIME64_1);
    __m256i p2_v = _mm256_set1_epi64x((int64_t)CORTEX_PRIME64_2);

    __m256i acc0 = _mm256_set1_epi64x((int64_t)(seed + CORTEX_PRIME64_1 + CORTEX_PRIME64_2));
    __m256i acc1 = _mm256_set1_epi64x((int64_t)(seed + CORTEX_PRIME64_2));

    const uint8_t *ptr = data;
    for (size_t s = 0; s < stripes; s++) {
        __m256i v0 = _mm256_loadu_si256((const __m256i*)(ptr));
        __m256i v1 = _mm256_loadu_si256((const __m256i*)(ptr + 32));

        __m256i v0_hi = _mm256_srli_epi64(v0, 32);
        __m256i v1_hi = _mm256_srli_epi64(v1, 32);
        __m256i v0_folded = _mm256_add_epi64(v0, v0_hi);
        __m256i v1_folded = _mm256_add_epi64(v1, v1_hi);

        __m256i p0_lo = _mm256_mul_epu32(v0_folded, p1_v);
        __m256i p1_lo = _mm256_mul_epu32(v1_folded, p2_v);
        __m256i p0_hi_mul = _mm256_mul_epu32(v0_hi, p2_v);
        __m256i p1_hi_mul = _mm256_mul_epu32(v1_hi, p1_v);

        acc0 = _mm256_add_epi64(acc0, _mm256_xor_si256(p0_lo, _mm256_add_epi64(s_lo, p0_hi_mul)));
        acc1 = _mm256_add_epi64(acc1, _mm256_xor_si256(p1_lo, _mm256_add_epi64(s_hi, p1_hi_mul)));

        ptr += 64;
    }

    __m256i total = _mm256_add_epi64(acc0, acc1);
    uint64_t lanes[4];
    _mm256_storeu_si256((__m256i*)lanes, total);

    uint64_t h = cortex_rot64(lanes[0], 1) + cortex_rot64(lanes[1], 7) +
                 cortex_rot64(lanes[2], 12) + cortex_rot64(lanes[3], 18);

    size_t rem = len - processed;
    if (rem > 0) {
        uint64_t tail_h = cortex_hash_len_17_to_64(data + len - 64, 64, seed);
        h = cortex_rot64(h, 31) ^ tail_h;
    }
    h ^= len * CORTEX_PRIME64_5;
    return cortex_rrmxmx(h);
#else
    size_t stripes = len / 64;
    size_t processed = stripes * 64;
    uint64_t acc1 = (uint64_t)len * CORTEX_PRIME64_1 + seed;
    uint64_t acc2 = (uint64_t)len * CORTEX_PRIME64_2 + seed;

    const uint8_t *ptr = data;
    for (size_t s = 0; s < stripes; s++) {
        uint64_t w0 = cortex_read_u64_le(ptr) ^ CORTEX_DEFAULT_SECRET[0];
        uint64_t w1 = cortex_read_u64_le(ptr + 8) ^ CORTEX_DEFAULT_SECRET[1];
        uint64_t w2 = cortex_read_u64_le(ptr + 16) ^ CORTEX_DEFAULT_SECRET[2];
        uint64_t w3 = cortex_read_u64_le(ptr + 24) ^ CORTEX_DEFAULT_SECRET[3];
        uint64_t w4 = cortex_read_u64_le(ptr + 32) ^ CORTEX_DEFAULT_SECRET[4];
        uint64_t w5 = cortex_read_u64_le(ptr + 40) ^ CORTEX_DEFAULT_SECRET[5];
        uint64_t w6 = cortex_read_u64_le(ptr + 48) ^ CORTEX_DEFAULT_SECRET[6];
        uint64_t w7 = cortex_read_u64_le(ptr + 56) ^ CORTEX_DEFAULT_SECRET[7];

        acc1 += cortex_mul128_fold(w0 + acc2, w1);
        acc2 += cortex_mul128_fold(w2 + acc1, w3);
        acc1 += cortex_mul128_fold(w4 + acc2, w5);
        acc2 += cortex_mul128_fold(w6 + acc1, w7);

        ptr += 64;
    }

    size_t rem = len - processed;
    if (rem > 0) {
        uint64_t tail_h = cortex_hash_len_17_to_64(data + len - 64, 64, seed);
        acc1 = cortex_rot64(acc1, 31) ^ tail_h;
    }
    return cortex_rrmxmx(acc1 ^ acc2);
#endif
}

static inline void cortex_hash128(const void *key, size_t len, uint64_t seed, uint64_t *out) {
    out[0] = cortex_hash64(key, len, seed);
    out[1] = cortex_hash64(key, len, ~seed + CORTEX_PRIME64_1);
}

#ifdef __cplusplus
}
#endif

#endif
