#include "gxhash.h"

#include <stdint.h>
#include <stdio.h>
#include <string.h>

#if defined(__AES__)
#include <immintrin.h>

#if defined(HAVE_AVX2) && defined(__VAES__)
typedef __m256i state;

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
    state partial;
    if (check_same_page(p)) {
        // Unsafe (hence the check) but much faster
#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
        state indices = _mm256_set_epi8(31, 30, 29, 28, 27, 26, 25, 24, 23, 22, 21, 20, 19, 18, 17, 16, 15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0);
#else
        state indices = _mm256_set_epi8(0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31);
#endif
        state mask = _mm256_cmpgt_epi8(_mm256_set1_epi8(len), indices);
        partial = _mm256_and_si256(_mm256_loadu_si256(p), mask);
    } else {
        // Safer but slower, using memcpy
        partial = get_partial_safe((const uint8_t*)p, (size_t)len);
    }
    
    // Prevents padded zeroes to introduce bias
    return _mm256_add_epi32(partial, _mm256_set1_epi32(len));
}

static inline state compress(state a, state b) {
    state keys_1 = _mm256_set_epi32(0xFC3BC28E, 0x89C222E5, 0xB09D3E21, 0xF2784542, 0x4155EE07, 0xC897CCE2, 0x780AF2C3, 0x8A72B781);
    state keys_2 = _mm256_set_epi32(0x03FCE279, 0xCB6B2E9B, 0xB361DC58, 0x39136BD9, 0x7A83D76B, 0xB1E8F9F0, 0x028925A8, 0x3B9A4E71);

    b = _mm256_aesenc_epi128(b, keys_1);
    b = _mm256_aesenc_epi128(b, keys_2);

    return _mm256_aesenclast_epi128(a, b);
}

static inline state compress_fast(state a, state b) {
    return _mm256_aesenc_epi128(a, b);
}

static inline state finalize(state hash, uint32_t seed) {
    state keys_1 = _mm256_set_epi32(0x713B01D0, 0x8F2F35DB, 0xAF163956, 0x85459F85, 0xB49D3E21, 0xF2784542, 0x2155EE07, 0xC197CCE2);
    state keys_2 = _mm256_set_epi32(0x1DE09647, 0x92CFA39C, 0x3DD99ACA, 0xB89C054F, 0xCB6B2E9B, 0xC361DC58, 0x39136BD9, 0x7A83D76F);
    state keys_3 = _mm256_set_epi32(0xC78B122B, 0x5544B1B7, 0x689D2B7D, 0xD0012E32, 0xE2784542, 0x4155EE07, 0xC897CCE2, 0x780BF2C2);

    hash = _mm256_aesenc_epi128(hash, _mm256_set1_epi32(seed));
    hash = _mm256_aesenc_epi128(hash, keys_1);
    hash = _mm256_aesenc_epi128(hash, keys_2);
    hash = _mm256_aesenclast_epi128(hash, keys_3);

    return hash;
}
#else // __AVX2__ && __VAES__
typedef __m128i state;

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

static inline int are_equal_m128i(__m128i a, __m128i b) {
    __m128i result = _mm_cmpeq_epi32(a, b);

    // Extract the results. If all are -1, then all 32-bit elements were equal.
    int mask = _mm_movemask_epi8(result);
    return mask == 0xFFFF; // all 16 bytes were equal
}

static inline state get_partial(const state* p, int len) {
    state partial;
    if (check_same_page(p)) {
        // Unsafe (hence the check) but much faster
#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
        state indices = _mm_set_epi8(15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0);
#else
        state indices = _mm_set_epi8(0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15);
#endif
        state mask = _mm_cmpgt_epi8 (_mm_set1_epi8 ((size_t)len), indices);
        state d = _mm_loadu_si128(p);
        partial = _mm_and_si128(d, mask);
    } else {
        // Safer but slower, using memcpy
        partial = get_partial_safe((const uint8_t*)p, (size_t)len);
    }
    
    // Prevents padded zeroes to introduce bias
    return _mm_add_epi8(partial, _mm_set1_epi8(len));
}

static inline state compress(state a, state b) {
#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
    state keys_1 = _mm_set_epi32(0xF2784542, 0xB09D3E21, 0x89C222E5, 0xFC3BC28E);
    state keys_2 = _mm_set_epi32(0x39136BD9, 0xB361DC58, 0xCB6B2E9B, 0x03FCE279);
#else
    state keys_1 = _mm_set_epi32(0xFC3BC28E, 0x89C222E5, 0xB09D3E21, 0xF2784542);
    state keys_2 = _mm_set_epi32(0x03FCE279, 0xCB6B2E9B, 0xB361DC58, 0x39136BD9);
#endif

    b = _mm_aesenc_si128(b, keys_1);
    b = _mm_aesenc_si128(b, keys_2);

    return _mm_aesenclast_si128(a, b);
}

static inline state compress_fast(state a, state b) {
    return _mm_aesenc_si128(a, b);
}

static inline state finalize(state hash, uint32_t seed) {
#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
    state keys_1 = _mm_set_epi32(0xE37845F2, 0xB09D2F61, 0x89F216D5, 0x5A3BC47E);
    state keys_2 = _mm_set_epi32(0x3D423129, 0xDE3A74DB, 0x6EA75BBA, 0xE7554D6F);
    state keys_3 = _mm_set_epi32(0x444DF600, 0x790FC729, 0xA735B3F2, 0xC992E848);
#else
    state keys_1 = _mm_set_epi32(0x5A3BC47E, 0x89F216D5, 0xB09D2F61, 0xE37845F2);
    state keys_2 = _mm_set_epi32(0xE7554D6F, 0x6EA75BBA, 0xDE3A74DB, 0x3D423129);
    state keys_3 = _mm_set_epi32(0xC992E848, 0xA735B3F2, 0x790FC729, 0x444DF600);
#endif

    hash = _mm_aesenc_si128(hash, _mm_set1_epi32(seed + 0xC992E848));
    hash = _mm_aesenc_si128(hash, keys_1);
    hash = _mm_aesenc_si128(hash, keys_2);
    hash = _mm_aesenclast_si128(hash, keys_3);

    return hash;
}
#endif // __AVX2__ && __VAES__

#elif defined(__ARM_NEON) || defined(__ARM_NEON__) || defined(__aarch64__)
#include <arm_neon.h>

typedef int8x16_t state;

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
    int8x16_t partial;
    if (check_same_page(p)) {
        // Unsafe (hence the check) but much faster
#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
        static const int8_t indices[] = {15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0};
#else
        static const int8_t indices[] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15};
#endif
        int8x16_t mask = vcgtq_s8(vdupq_n_u8(len), vld1q_s8(indices));
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

static inline uint8x16_t aes_encrypt_last(uint8x16_t data, uint8x16_t keys) {
    uint8x16_t encrypted = vaeseq_u8(data, vdupq_n_u8(0));
    return veorq_u8(encrypted, keys);
}

static inline state compress(state a, state b) {
    static const uint32_t keys_1[4] = {0xFC3BC28E, 0x89C222E5, 0xB09D3E21, 0xF2784542};
    static const uint32_t keys_2[4] = {0x03FCE279, 0xCB6B2E9B, 0xB361DC58, 0x39136BD9};

    b = aes_encrypt(b, vld1q_u32(keys_1));
    b = aes_encrypt(b, vld1q_u32(keys_2));

    return aes_encrypt_last(a, b);
}

static inline state compress_fast(state a, state b) {
    return aes_encrypt(a, b);
}

static inline state finalize(state hash, uint32_t seed) {
    static const uint32_t keys_1[4] = {0x5A3BC47E, 0x89F216D5, 0xB09D2F61, 0xE37845F2};
    static const uint32_t keys_2[4] = {0xE7554D6F, 0x6EA75BBA, 0xDE3A74DB, 0x3D423129};
    static const uint32_t keys_3[4] = {0xC992E848, 0xA735B3F2, 0x790FC729, 0x444DF600};

    hash = aes_encrypt(hash, vdupq_n_u32(seed + 0xC992E848));
    hash = aes_encrypt(hash, vld1q_u32(keys_1));
    hash = aes_encrypt(hash, vld1q_u32(keys_2));
    hash = aes_encrypt_last(hash, vld1q_u32(keys_3));

    return hash;
}
#else
// Fallback ?
#endif

static inline state gxhash(const uint8_t* input, int len, uint32_t seed) {
    const int VECTOR_SIZE = sizeof(state);
    const state* p = (const state*)input;
    const state* v = p;
    const state* end_address;
    int remaining_blocks_count = len / VECTOR_SIZE;
    state hash_vector = create_empty();

    if (len <= VECTOR_SIZE) {
        hash_vector = get_partial(v, len);
        goto skip;
    }

    const int UNROLL_FACTOR = 8;
    if (len >= VECTOR_SIZE * UNROLL_FACTOR) {
        int unrollable_blocks_count = (len / (VECTOR_SIZE * UNROLL_FACTOR)) * UNROLL_FACTOR;
        end_address = v + unrollable_blocks_count;

        while (v < end_address) {
            state v0 = load_unaligned(v++);
            state v1 = load_unaligned(v++);
            state v2 = load_unaligned(v++);
            state v3 = load_unaligned(v++);
            state v4 = load_unaligned(v++);
            state v5 = load_unaligned(v++);
            state v6 = load_unaligned(v++);
            state v7 = load_unaligned(v++);

            v0 = compress_fast(v0, v1);
            v0 = compress_fast(v0, v2);
            v0 = compress_fast(v0, v3);
            v0 = compress_fast(v0, v4);
            v0 = compress_fast(v0, v5);
            v0 = compress_fast(v0, v6);
            v0 = compress_fast(v0, v7);

            hash_vector = compress(hash_vector, v0);
        }

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

 skip:
    return finalize(hash_vector, seed);
}

uint32_t gxhash32(const uint8_t* input, int len, uint32_t seed) {
    state full_hash = gxhash(input, len, seed);
    return *(uint32_t*)&full_hash;
}

uint64_t gxhash64(const uint8_t* input, int len, uint32_t seed) {
    state full_hash = gxhash(input, len, seed);
    return *(uint64_t*)&full_hash;
}