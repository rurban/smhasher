#include "gxhash.h"

#include <stdint.h>
#include <stdio.h>
#include <string.h>

#if defined(__SSE__) && defined(__AES__)
#include <immintrin.h>

#if defined(__AVX2__) && defined(__VAES__)
typedef __m256i state;
typedef __m128i output;

static inline state create_empty() {
    return _mm256_setzero_si256();
}

static inline state load_unaligned(const state* p) {
    return _mm256_loadu_si256(p);
}

static inline int check_same_page(const state* ptr) {
    uintptr_t address = (uintptr_t)ptr;
    uintptr_t offset_within_page = address & 0xFFF;
    return offset_within_page <= (4096 - sizeof(state) - 1);
}

static inline state get_partial_safe(const uint8_t* data, size_t len) {
    uint8_t buffer[sizeof(state)] = {0};
    memcpy(buffer, data, len);
    return _mm256_loadu_si256((const state*)buffer);
}

static inline state get_partial(const state* p, intptr_t len) {
    static const uint8_t MASK[2 * sizeof(state)] = {
        0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
        0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
    };

    if (check_same_page(p)) {
        const uint8_t* mask_ptr = &MASK[32 - len];
        __m256i mask = _mm256_loadu_si256((const __m256i*)mask_ptr);
        return _mm256_and_si256(_mm256_loadu_si256(p), mask);
    } else {
        return get_partial_safe((const uint8_t*)p, (size_t)len);
    }
}

static inline state compress(state a, state b) {
    // return _mm256_aesdeclast_epi128(a, b); // GxHash0

    state salt1 = _mm256_set_epi32(4104244489u, 3710553163u, 3367764511u, 4219769173u, 3229102777u, 4201852661u, 3065017993u, 2722855403u);
    state salt2 = _mm256_set_epi32(3624366803u, 3553132711u, 2860740361u, 2722013029u, 2350914373u, 3418786373u, 3841501031u, 3172997263u);

    a = _mm256_aesdec_epi128(a, salt1);
    b = _mm256_aesdec_epi128(b, salt2);

    return _mm256_aesdeclast_epi128(a, b);
}

static inline output finalize(state hash, uint32_t seed) {
    __m128i lower = _mm256_castsi256_si128(hash);
    __m128i upper = _mm256_extracti128_si256(hash, 1);
    __m128i hash128 = _mm_xor_si128(lower, upper);

    __m128i salt1 = _mm_set_epi32(0x713B01D0, 0x8F2F35DB, 0xAF163956, 0x85459F85);
    __m128i salt2 = _mm_set_epi32(0x1DE09647, 0x92CFA39C, 0x3DD99ACA, 0xB89C054F);
    __m128i salt3 = _mm_set_epi32(0xC78B122B, 0x5544B1B7, 0x689D2B7D, 0xD0012E32);

    hash128 = _mm_aesenc_si128(hash128, _mm_set1_epi32(seed));
    hash128 = _mm_aesenc_si128(hash128, salt1);
    hash128 = _mm_aesenc_si128(hash128, salt2);
    hash128 = _mm_aesenclast_si128(hash128, salt3);

    return hash128;
}
#else
typedef __m128i state;
typedef __m128i output;

static inline state create_empty() {
    return _mm_setzero_si128();
}

static inline state load_unaligned(const state* p) {
    return _mm_loadu_si128(p);
}

static inline int check_same_page(const state* ptr) {
    uintptr_t address = (uintptr_t)ptr;
    uintptr_t offset_within_page = address & 0xFFF;
    return offset_within_page <= (4096 - sizeof(state) - 1);
}

static inline state get_partial_safe(const uint8_t* data, size_t len) {
    uint8_t buffer[sizeof(state)] = {0};
    memcpy(buffer, data, len);
    return _mm_loadu_si128((const state*)buffer);
}

static inline state get_partial(const state* p, intptr_t len) {
    static const uint8_t MASK[2 * sizeof(state)] = {
        0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
    };

    if (check_same_page(p)) {
        const uint8_t* mask_ptr = &MASK[16 - len];
        state mask = _mm_loadu_si128((const state*)mask_ptr);
        return _mm_and_si128(_mm_loadu_si128(p), mask);
    } else {
        return get_partial_safe((const uint8_t*)p, (size_t)len);
    }
}

static inline state compress(state a, state b) {

    state salt1 = _mm_set_epi32(4104244489u, 3710553163u, 3367764511u, 4219769173u);
    state salt2 = _mm_set_epi32(3624366803u, 3553132711u, 2860740361u, 2722013029u);

    a = _mm_aesdec_si128(a, salt1);
    b = _mm_aesdec_si128(b, salt2);

    return _mm_aesdeclast_si128(a, b);
}

static inline output finalize(state hash, uint32_t seed) {

    __m128i salt1 = _mm_set_epi32(0x713B01D0, 0x8F2F35DB, 0xAF163956, 0x85459F85);
    __m128i salt2 = _mm_set_epi32(0x1DE09647, 0x92CFA39C, 0x3DD99ACA, 0xB89C054F);
    __m128i salt3 = _mm_set_epi32(0xC78B122B, 0x5544B1B7, 0x689D2B7D, 0xD0012E32);

    hash = _mm_aesenc_si128(hash, _mm_set1_epi32(seed));
    hash = _mm_aesenc_si128(hash, salt1);
    hash = _mm_aesenc_si128(hash, salt2);
    hash = _mm_aesenclast_si128(hash, salt3);

    return hash;
}
#endif

#elif defined(__ARM_NEON) || defined(__ARM_NEON__) || defined(__aarch64__)
#include <arm_neon.h>

typedef int8x16_t state;
typedef int8x16_t output;

static inline state create_empty() {
    return vdupq_n_s8(0);
}

static inline state load_unaligned(const state* p) {
    return vld1q_s8((const int8_t*)p);
}

static inline int check_same_page(const state* ptr) {
    uintptr_t address = (uintptr_t)ptr;
    uintptr_t offset_within_page = address & 0xFFF;
    return offset_within_page <= (4096 - sizeof(state) - 1);
}

static inline state get_partial(const state* p, int len) {
    static const int8_t MASK[32] = {
        0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
    };

    int8x16_t partial;
    if (check_same_page(p)) {
        // Unsafe (hence the check) but much faster
        int8x16_t mask = vld1q_s8(&MASK[16 - len]);
        partial = vandq_s8(load_unaligned(p), mask);
    } else {
        // Safer but slower, using memcpy
        uint8_t buffer[sizeof(state)] = {0};
        memcpy(buffer, (const uint8_t*)p, len);
        partial = vld1q_s8((const int8_t*)buffer);
    }

    // Prevents padded zeroes to introduce bias
    return vaddq_u8(partial, vdupq_n_u8(len));
}

static inline uint8x16_t aes_encrypt(uint8x16_t data, uint8x16_t keys) {
    uint8x16_t encrypted = vaeseq_u8(data, vdupq_n_u8(0));
    uint8x16_t mixed = vaesmcq_u8(encrypted);
    return veorq_u8(mixed, keys);
}

static inline uint8x16_t aes_decrypt(uint8x16_t data, uint8x16_t keys) {
    uint8x16_t encrypted = vaesdq_u8(data, vdupq_n_u8(0));
    uint8x16_t mixed = vaesmcq_u8(encrypted);
    return veorq_u8(mixed, keys);
}

static inline uint8x16_t aes_encrypt_last(uint8x16_t data, uint8x16_t keys) {
    uint8x16_t encrypted = vaeseq_u8(data, vdupq_n_u8(0));
    return veorq_u8(encrypted, keys);
}

// Somewhat computationally expensive, but at least it passes SMHasher
static inline state compress(state a, state b) {
    
    //return aes_encrypt_last(a, b); // GxHash0

    static const uint32_t salt_a_data[4] = {4104244489u, 3710553163u, 3367764511u, 4219769173u};
    static const uint32_t salt_b_data[4] = {3624366803u, 3553132711u, 2860740361u, 2722013029u};

    a = aes_encrypt(a, vld1q_u32(salt_a_data));
    b = aes_encrypt(b, vld1q_u32(salt_b_data));

    return aes_encrypt_last(a, b);
}

static inline state finalize(state hash, uint32_t seed) {
    static const uint32_t salt1_data[4] = {0x713B01D0, 0x8F2F35DB, 0xAF163956, 0x85459F85};
    static const uint32_t salt2_data[4] = {0x1DE09647, 0x92CFA39C, 0x3DD99ACA, 0xB89C054F};
    static const uint32_t salt3_data[4] = {0xC78B122B, 0x5544B1B7, 0x689D2B7D, 0xD0012E32};

    hash = aes_encrypt(hash, vdupq_n_u32(seed));
    hash = aes_encrypt(hash, vld1q_u32(salt1_data));
    hash = aes_encrypt(hash, vld1q_u32(salt2_data));
    hash = aes_encrypt_last(hash, vld1q_u32(salt3_data));

    return hash;
}
#else
// Fallback ?
#endif

static inline output gxhash(const uint8_t* input, int len, uint32_t seed) {
    const int VECTOR_SIZE = sizeof(state);
    const state* p = (const state*)input;
    const state* v = p;
    const state* end_address;
    int remaining_blocks_count = len / VECTOR_SIZE;
    state hash_vector = create_empty();

    const int UNROLL_FACTOR = 8;
    if (len >= VECTOR_SIZE * UNROLL_FACTOR) {
        int unrollable_blocks_count = (len / (VECTOR_SIZE * UNROLL_FACTOR)) * UNROLL_FACTOR;
        end_address = v + unrollable_blocks_count;

        state s0 = load_unaligned(v++);
        state s1 = load_unaligned(v++);
        state s2 = load_unaligned(v++);
        state s3 = load_unaligned(v++);
        state s4 = load_unaligned(v++);
        state s5 = load_unaligned(v++);
        state s6 = load_unaligned(v++);
        state s7 = load_unaligned(v++);

        while (v < end_address) {
            state v0 = load_unaligned(v++);
            state v1 = load_unaligned(v++);
            state v2 = load_unaligned(v++);
            state v3 = load_unaligned(v++);
            state v4 = load_unaligned(v++);
            state v5 = load_unaligned(v++);
            state v6 = load_unaligned(v++);
            state v7 = load_unaligned(v++);

            s0 = compress(s0, v0);
            s1 = compress(s1, v1);
            s2 = compress(s2, v2);
            s3 = compress(s3, v3);
            s4 = compress(s4, v4);
            s5 = compress(s5, v5);
            s6 = compress(s6, v6);
            s7 = compress(s7, v7);
        }

        state a = compress(compress(s0, s1), compress(s2, s3));
        state b = compress(compress(s4, s5), compress(s6, s7));
        hash_vector = compress(a, b);

        remaining_blocks_count -= unrollable_blocks_count;
    }

    end_address = v + remaining_blocks_count;

    while (v < end_address) {
        state v0 = load_unaligned(v++);
        hash_vector = compress(hash_vector, v0);
    }

    int remaining_bytes = len % VECTOR_SIZE;
    if (remaining_bytes > 0) {
        state partial_vector = get_partial(v, remaining_bytes);
        hash_vector = compress(hash_vector, partial_vector);
    }

    return finalize(hash_vector, seed);
}

uint32_t gxhash32(const uint8_t* input, int len, uint32_t seed) {
    output full_hash = gxhash(input, len, seed);
    return *(uint32_t*)&full_hash;
}

uint64_t gxhash64(const uint8_t* input, int len, uint32_t seed) {
    output full_hash = gxhash(input, len, seed);
    return *(uint64_t*)&full_hash;
}