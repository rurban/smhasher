// Tachyon
// Copyright (c) byt3forg3
// Licensed under the MIT or Apache 2.0 License
// -------------------------------------------------------------------------

#include <stdint.h>
#include <string.h>
#include "tachyon_impl.h"

#if defined(__x86_64__) || defined(__i386__) || defined(_M_X64) || defined(_M_IX86)

#include <immintrin.h>
// =============================================================================
// AVX-512 KERNEL
// =============================================================================

#if defined(__GNUC__) || defined(__clang__)
  #define TARGET_AVX512 __attribute__((target("avx512f,avx512bw,vaes,vpclmulqdq")))
#else
  #define TARGET_AVX512
#endif

typedef struct {
    __m512i acc[NUM_LANES];
    uint64_t block_count;
} tachyon_avx512_state_t;

TARGET_AVX512
static inline __m512i init_reg(uint64_t base) {
    return _mm512_set_epi64(
        base + 7, base + 6, base + 5, base + 4,
        base + 3, base + 2, base + 1, base
    );
}

// =============================================================================
// COMPRESSION HELPERS
// =============================================================================

#define AES_MIX(acc, data, rk, lo, blk) \
    _mm512_aesenc_epi128(acc, _mm512_add_epi64(data, _mm512_add_epi64(rk, _mm512_add_epi64(lo, blk))))

TARGET_AVX512
static void avx512_compress_phase1_roundrobin(
    tachyon_avx512_state_t *state,
    __m512i *d0, __m512i *d1, __m512i *d2, __m512i *d3,
    __m512i *d4, __m512i *d5, __m512i *d6, __m512i *d7,
    const __m512i rk_base[10], const __m512i lo[NUM_LANES], __m512i blk) 
{
    for (int r = 0; r < 5; r++) {
        __m512i rk = rk_base[r];
        state->acc[0] = AES_MIX(state->acc[0], *d0, rk, lo[0], blk);
        state->acc[1] = AES_MIX(state->acc[1], *d1, rk, lo[1], blk);
        state->acc[2] = AES_MIX(state->acc[2], *d2, rk, lo[2], blk);
        state->acc[3] = AES_MIX(state->acc[3], *d3, rk, lo[3], blk);
        state->acc[4] = AES_MIX(state->acc[4], *d4, rk, lo[4], blk);
        state->acc[5] = AES_MIX(state->acc[5], *d5, rk, lo[5], blk);
        state->acc[6] = AES_MIX(state->acc[6], *d6, rk, lo[6], blk);
        state->acc[7] = AES_MIX(state->acc[7], *d7, rk, lo[7], blk);

        *d0 = _mm512_xor_si512(*d0, state->acc[3]);
        *d1 = _mm512_xor_si512(*d1, state->acc[4]);
        *d2 = _mm512_xor_si512(*d2, state->acc[5]);
        *d3 = _mm512_xor_si512(*d3, state->acc[6]);
        *d4 = _mm512_xor_si512(*d4, state->acc[7]);
        *d5 = _mm512_xor_si512(*d5, state->acc[0]);
        *d6 = _mm512_xor_si512(*d6, state->acc[1]);
        *d7 = _mm512_xor_si512(*d7, state->acc[2]);

        __m512i tmp = state->acc[0];
        state->acc[0] = state->acc[1]; 
        state->acc[1] = state->acc[2]; 
        state->acc[2] = state->acc[3]; 
        state->acc[3] = state->acc[4];
        state->acc[4] = state->acc[5]; 
        state->acc[5] = state->acc[6]; 
        state->acc[6] = state->acc[7]; 
        state->acc[7] = tmp;
    }
}

TARGET_AVX512
static void avx512_compress_midblock_mixing(tachyon_avx512_state_t *state) {
    for (int i = 0; i < NUM_LANES; i++) {
        state->acc[i] = _mm512_alignr_epi64(state->acc[i], state->acc[i], 2);
    }

    /* Cross-Accumulator Diffusion Stage 1 */
    __m512i tmp_lo[4] = {
        state->acc[0], state->acc[1], state->acc[2], state->acc[3]
    };
    
    state->acc[0] = _mm512_xor_si512(state->acc[0], state->acc[4]);
    state->acc[1] = _mm512_xor_si512(state->acc[1], state->acc[5]);
    state->acc[2] = _mm512_xor_si512(state->acc[2], state->acc[6]);
    state->acc[3] = _mm512_xor_si512(state->acc[3], state->acc[7]);
    
    state->acc[4] = _mm512_add_epi64(state->acc[4], tmp_lo[0]);
    state->acc[5] = _mm512_add_epi64(state->acc[5], tmp_lo[1]);
    state->acc[6] = _mm512_add_epi64(state->acc[6], tmp_lo[2]);
    state->acc[7] = _mm512_add_epi64(state->acc[7], tmp_lo[3]);

    /* Cross-Accumulator Diffusion Stage 2 */
    __m512i bf0 = state->acc[0];
    __m512i bf1 = state->acc[1];
    __m512i bf4 = state->acc[4];
    __m512i bf5 = state->acc[5];
    
    state->acc[0] = _mm512_xor_si512(state->acc[0], state->acc[2]);
    state->acc[2] = _mm512_add_epi64(state->acc[2], bf0);
    
    state->acc[1] = _mm512_xor_si512(state->acc[1], state->acc[3]);
    state->acc[3] = _mm512_add_epi64(state->acc[3], bf1);
    
    state->acc[4] = _mm512_xor_si512(state->acc[4], state->acc[6]);
    state->acc[6] = _mm512_add_epi64(state->acc[6], bf4);
    
    state->acc[5] = _mm512_xor_si512(state->acc[5], state->acc[7]);
    state->acc[7] = _mm512_add_epi64(state->acc[7], bf5);
}

TARGET_AVX512
static void avx512_compress_phase2_and_feedforward(
    tachyon_avx512_state_t *state,
    __m512i *d0, __m512i *d1, __m512i *d2, __m512i *d3,
    __m512i *d4, __m512i *d5, __m512i *d6, __m512i *d7,
    const __m512i rk_base[10], const __m512i lo[NUM_LANES], 
    __m512i blk, const __m512i saves[NUM_LANES]) 
{
    for (int r = 5; r < 10; r++) {
        __m512i rk = rk_base[r];
        state->acc[0] = AES_MIX(state->acc[0], *d4, rk, lo[0], blk);
        state->acc[1] = AES_MIX(state->acc[1], *d5, rk, lo[1], blk);
        state->acc[2] = AES_MIX(state->acc[2], *d6, rk, lo[2], blk);
        state->acc[3] = AES_MIX(state->acc[3], *d7, rk, lo[3], blk);
        state->acc[4] = AES_MIX(state->acc[4], *d0, rk, lo[4], blk);
        state->acc[5] = AES_MIX(state->acc[5], *d1, rk, lo[5], blk);
        state->acc[6] = AES_MIX(state->acc[6], *d2, rk, lo[6], blk);
        state->acc[7] = AES_MIX(state->acc[7], *d3, rk, lo[7], blk);
        
        *d0 = _mm512_xor_si512(*d0, state->acc[3]);
        *d1 = _mm512_xor_si512(*d1, state->acc[4]);
        *d2 = _mm512_xor_si512(*d2, state->acc[5]);
        *d3 = _mm512_xor_si512(*d3, state->acc[6]);
        *d4 = _mm512_xor_si512(*d4, state->acc[7]);
        *d5 = _mm512_xor_si512(*d5, state->acc[0]);
        *d6 = _mm512_xor_si512(*d6, state->acc[1]);
        *d7 = _mm512_xor_si512(*d7, state->acc[2]);
        
        __m512i tmp = state->acc[0];
        state->acc[0] = state->acc[1]; 
        state->acc[1] = state->acc[2]; 
        state->acc[2] = state->acc[3]; 
        state->acc[3] = state->acc[4];
        state->acc[4] = state->acc[5]; 
        state->acc[5] = state->acc[6]; 
        state->acc[6] = state->acc[7]; 
        state->acc[7] = tmp;
    }

    /* Davies-Meyer Feed-Forward */
    for (int i = 0; i < NUM_LANES; i++) {
        state->acc[i] = _mm512_alignr_epi64(state->acc[i], state->acc[i], 2);
    }
    for (int i = 0; i < NUM_LANES; i++) {
        state->acc[i] = _mm512_xor_si512(state->acc[i], saves[i]);
    }
}

// =============================================================================
// FINALIZATION HELPERS
// =============================================================================

TARGET_AVX512
static size_t avx512_finalize_remainder_chunks(
    tachyon_avx512_state_t *state, const uint8_t *remainder, size_t rem_len, 
    __m512i wk, const __m512i rk_chain[10]) 
{
    const uint8_t *ptr = remainder;
    size_t offset = 0;
    
    for (int i = 0; i < NUM_LANES; i++) {
        if (offset + REMAINDER_CHUNK_SIZE <= rem_len) {
            __m512i d = _mm512_aesenc_epi128(_mm512_loadu_si512(ptr + offset), wk);
            int base = i * 4;
            
            __m512i lo_f = _mm512_set_epi64(
                LANE_OFFSETS[base+3], LANE_OFFSETS[base+3],
                LANE_OFFSETS[base+2], LANE_OFFSETS[base+2],
                LANE_OFFSETS[base+1], LANE_OFFSETS[base+1],
                LANE_OFFSETS[base],   LANE_OFFSETS[base]
            );
            
            __m512i save = state->acc[i];
            
            for (int r = 0; r < 10; r++) {
                state->acc[i] = _mm512_aesenc_epi128(state->acc[i], _mm512_add_epi64(d, _mm512_add_epi64(rk_chain[r], lo_f)));
                state->acc[i] = _mm512_alignr_epi64(state->acc[i], state->acc[i], 2);
                d = _mm512_xor_si512(d, state->acc[i]);
            }
            
            state->acc[i] = _mm512_xor_si512(state->acc[i], save);
            offset += REMAINDER_CHUNK_SIZE;
        }
    }
    
    return offset;
}

TARGET_AVX512
static void avx512_finalize_tree_merge(tachyon_avx512_state_t *state) {
    __m512i mrk0 = _mm512_set1_epi64(C5);
    __m512i mrk1 = _mm512_set1_epi64(C6);
    __m512i mrk2 = _mm512_set1_epi64(C7);

    for (int i = 0; i < 4; i++) {
        state->acc[i] = _mm512_aesenc_epi128(state->acc[i], _mm512_xor_si512(state->acc[i+4], mrk0));
        state->acc[i] = _mm512_aesenc_epi128(state->acc[i], _mm512_xor_si512(state->acc[i],   mrk0));
    }
    
    state->acc[0] = _mm512_aesenc_epi128(state->acc[0], _mm512_xor_si512(state->acc[2], mrk1));
    state->acc[0] = _mm512_aesenc_epi128(state->acc[0], _mm512_xor_si512(state->acc[0], mrk1));
    
    state->acc[1] = _mm512_aesenc_epi128(state->acc[1], _mm512_xor_si512(state->acc[3], mrk1));
    state->acc[1] = _mm512_aesenc_epi128(state->acc[1], _mm512_xor_si512(state->acc[1], mrk1));
    
    state->acc[0] = _mm512_aesenc_epi128(state->acc[0], _mm512_xor_si512(state->acc[1], mrk2));
    state->acc[0] = _mm512_aesenc_epi128(state->acc[0], _mm512_xor_si512(state->acc[0], mrk2));
}

TARGET_AVX512
static void avx512_finalize_clmul_hardening(tachyon_avx512_state_t *state) {
    __m512i clmul_k = _mm512_set_epi64(
         CLMUL_CONSTANT2, CLMUL_CONSTANT, 
         CLMUL_CONSTANT2, CLMUL_CONSTANT, 
         CLMUL_CONSTANT2, CLMUL_CONSTANT, 
         CLMUL_CONSTANT2, CLMUL_CONSTANT
    );
    
    __m512i cl_lo = _mm512_clmulepi64_epi128(state->acc[0], clmul_k, 0x00);
    __m512i cl_hi = _mm512_clmulepi64_epi128(state->acc[0], clmul_k, 0x11);
    __m512i cl1   = _mm512_xor_si512(cl_lo, cl_hi);
    __m512i mid   = _mm512_aesenc_epi128(state->acc[0], cl1);
    __m512i cl2   = _mm512_clmulepi64_epi128(mid, mid, 0x01);
    
    state->acc[0] = _mm512_aesenc_epi128(state->acc[0], _mm512_xor_si512(cl1, cl2));
}

TARGET_AVX512
static void avx512_finalize_block_process(
    tachyon_avx512_state_t *state, __m512i d_pad, 
    uint64_t total_len, uint64_t domain, 
    const __m512i rk_chain[10], __m512i save_final) 
{
    __m512i meta_vec = _mm512_set_epi64(
        CHAOS_BASE, domain, total_len, CHAOS_BASE,
        domain, total_len, CHAOS_BASE, domain ^ total_len
    );
    
    state->acc[0] = _mm512_ternarylogic_epi64(state->acc[0], d_pad, meta_vec, 0x96); // XOR 3 vectors
    
    for (int r = 0; r < 10; r++) {
        state->acc[0] = _mm512_aesenc_epi128(state->acc[0], _mm512_add_epi64(d_pad, rk_chain[r]));
        state->acc[0] = _mm512_alignr_epi64(state->acc[0], state->acc[0], 2);
        
        if (r % 2 == 1) {
            d_pad = _mm512_xor_si512(d_pad, state->acc[0]);
        }
    }
    
    state->acc[0] = _mm512_xor_si512(state->acc[0], save_final);
}

TARGET_AVX512
static void avx512_finalize_key_reabsorption(tachyon_avx512_state_t *state, const uint8_t *key) {
    if (!key) return;
    
    uint8_t key_padding[REMAINDER_CHUNK_SIZE];
    memcpy(key_padding,             key, HASH_SIZE);
    memcpy(key_padding + HASH_SIZE, key, HASH_SIZE);
    
    __m512i k0_f = _mm512_loadu_si512(key_padding);
    __m512i gr_f = _mm512_set_epi64(GOLDEN_RATIO, GOLDEN_RATIO, GOLDEN_RATIO, GOLDEN_RATIO, 0, 0, 0, 0);
    k0_f = _mm512_xor_si512(k0_f, gr_f);
    
    state->acc[0] = _mm512_aesenc_epi128(state->acc[0], _mm512_shuffle_i32x4(k0_f, k0_f, 0x14));
    state->acc[0] = _mm512_aesenc_epi128(state->acc[0], _mm512_shuffle_i32x4(k0_f, k0_f, 0x41));
    state->acc[0] = _mm512_aesenc_epi128(state->acc[0], _mm512_shuffle_i32x4(k0_f, k0_f, 0x44));
    state->acc[0] = _mm512_aesenc_epi128(state->acc[0], _mm512_shuffle_i32x4(k0_f, k0_f, 0x50));
}

TARGET_AVX512
static void avx512_lane_reduction_4to256(__m512i acc_zero, uint8_t *out) {
    __m512i asymmetry = _mm512_set_epi64(C5, C5, C6, C6, C7, C7, 0, 0); 
    
    __m512i mix = _mm512_aesenc_epi128(acc_zero, acc_zero);
    mix = _mm512_aesenc_epi128(mix, _mm512_shuffle_i32x4(mix, mix, 0x4E));
    
    __m512i mix_s = _mm512_shuffle_i32x4(mix, mix, 0xB1);
    mix = _mm512_aesenc_epi128(mix, _mm512_xor_si512(mix_s, asymmetry));
    
    mix = _mm512_aesenc_epi128(mix, _mm512_shuffle_i32x4(mix, mix, 0x4E));
    mix_s = _mm512_shuffle_i32x4(mix, mix, 0xB1);
    mix = _mm512_aesenc_epi128(mix, _mm512_xor_si512(mix_s, asymmetry));
    
    _mm_storeu_si128((__m128i*)out, _mm512_castsi512_si128(mix));
    _mm_storeu_si128((__m128i*)(out + VEC_SIZE), _mm512_castsi512_si128(_mm512_shuffle_i32x4(mix, mix, 0x01)));
}

// External short-path fallback.
extern void tachyon_aesni_oneshot_short(const uint8_t *input, size_t len, uint64_t domain, uint64_t seed, const uint8_t *key, uint8_t *out);

// =============================================================================
// INITIALIZATION
// =============================================================================

TARGET_AVX512
void tachyon_avx512_init(tachyon_avx512_state_t *state, const uint8_t *key, uint64_t seed) {
    state->acc[0] = init_reg(C0);
    state->acc[1] = init_reg(C1);
    state->acc[2] = init_reg(C2);
    state->acc[3] = init_reg(C3);
    state->acc[4] = init_reg(C4);
    state->acc[5] = init_reg(C5);
    state->acc[6] = init_reg(C6);
    state->acc[7] = init_reg(C7);

    __m512i s_vec = (seed != 0) ? _mm512_set1_epi64(seed) : _mm512_set1_epi64(C5);
    for (int i = 0; i < NUM_LANES; i++) {
        state->acc[i] = _mm512_aesenc_epi128(state->acc[i], s_vec);
    }

    if (key) {
        uint8_t key_block[REMAINDER_CHUNK_SIZE];
        memcpy(key_block,             key, HASH_SIZE);
        memcpy(key_block + HASH_SIZE, key, HASH_SIZE);

        __m512i k_vec = _mm512_loadu_si512(key_block);

        __m512i gr_mask = _mm512_set_epi64(
            GOLDEN_RATIO, GOLDEN_RATIO, GOLDEN_RATIO, GOLDEN_RATIO,
            0, 0, 0, 0
        );
        k_vec = _mm512_xor_si512(k_vec, gr_mask);
        
        for (int i = 0; i < NUM_LANES; i++) {
            __m512i lo_k  = _mm512_add_epi64(k_vec, _mm512_set1_epi64(LANE_OFFSETS[i]));
            state->acc[i] = _mm512_aesenc_epi128(state->acc[i], lo_k);
            state->acc[i] = _mm512_aesenc_epi128(state->acc[i], k_vec);
        }
    }
    state->block_count = 0;
}

// =============================================================================
// COMPRESSION
// =============================================================================

TARGET_AVX512
void tachyon_avx512_update(tachyon_avx512_state_t *state, const uint8_t *input, size_t len) {
    __m512i rk_base[10];
    for (int r = 0; r < 10; r++) {
        rk_base[r] = _mm512_set_epi64(
            RK_CHAIN[r][1], RK_CHAIN[r][0], RK_CHAIN[r][1], RK_CHAIN[r][0],
            RK_CHAIN[r][1], RK_CHAIN[r][0], RK_CHAIN[r][1], RK_CHAIN[r][0]
        );
    }

    __m512i lo[NUM_LANES];
    for (int i = 0; i < NUM_LANES; i++) {
        int base = i * 4;
        lo[i] = _mm512_set_epi64(
            LANE_OFFSETS[base+3], LANE_OFFSETS[base+3],
            LANE_OFFSETS[base+2], LANE_OFFSETS[base+2],
            LANE_OFFSETS[base+1], LANE_OFFSETS[base+1],
            LANE_OFFSETS[base],   LANE_OFFSETS[base]
        );
    }
    
    __m512i wk = _mm512_set_epi64(
        WHITENING1, WHITENING0, WHITENING1, WHITENING0, 
        WHITENING1, WHITENING0, WHITENING1, WHITENING0
    );
    
    size_t processed = 0;
    while (processed + BLOCK_SIZE <= len) {
        const uint8_t *ptr = input + processed;
        __m512i blk = _mm512_set1_epi64(state->block_count);

        __m512i d0 = _mm512_aesenc_epi128(_mm512_loadu_si512(ptr),       wk);
        __m512i d1 = _mm512_aesenc_epi128(_mm512_loadu_si512(ptr + 64),  wk);
        __m512i d2 = _mm512_aesenc_epi128(_mm512_loadu_si512(ptr + 128), wk);
        __m512i d3 = _mm512_aesenc_epi128(_mm512_loadu_si512(ptr + 192), wk);
        __m512i d4 = _mm512_aesenc_epi128(_mm512_loadu_si512(ptr + 256), wk);
        __m512i d5 = _mm512_aesenc_epi128(_mm512_loadu_si512(ptr + 320), wk);
        __m512i d6 = _mm512_aesenc_epi128(_mm512_loadu_si512(ptr + 384), wk);
        __m512i d7 = _mm512_aesenc_epi128(_mm512_loadu_si512(ptr + 448), wk);
        
        __m512i saves[NUM_LANES];
        for (int i = 0; i < NUM_LANES; i++) saves[i] = state->acc[i];

        /* Phase 1: Round-Robin Mix (Direct Mapping) */
        avx512_compress_phase1_roundrobin(state, &d0, &d1, &d2, &d3, &d4, &d5, &d6, &d7, rk_base, lo, blk);

        /* Mid-block mixing: Intra-register lane rotation */
        avx512_compress_midblock_mixing(state);

        /* Phase 2: Completion (Offset Mapping) */
        avx512_compress_phase2_and_feedforward(state, &d0, &d1, &d2, &d3, &d4, &d5, &d6, &d7, rk_base, lo, blk, saves);

        processed += BLOCK_SIZE;
        state->block_count++;
    }
}

// =============================================================================
// FINALIZATION
// =============================================================================

TARGET_AVX512
void tachyon_avx512_finalize(
    tachyon_avx512_state_t *state, const uint8_t *remainder, size_t rem_len, 
    uint64_t total_len, uint64_t domain, const uint8_t *key, uint8_t *out) 
{
    __m512i rk_chain[10];
    for (int r = 0; r < 10; r++) {
        rk_chain[r] = _mm512_set_epi64(
            RK_CHAIN[r][1], RK_CHAIN[r][0], RK_CHAIN[r][1], RK_CHAIN[r][0],
            RK_CHAIN[r][1], RK_CHAIN[r][0], RK_CHAIN[r][1], RK_CHAIN[r][0]
        );
    }
    __m512i wk = _mm512_set_epi64(
        WHITENING1, WHITENING0, WHITENING1, WHITENING0, 
        WHITENING1, WHITENING0, WHITENING1, WHITENING0
    );

    /* 1. Remainder Chunks */
    size_t processed = avx512_finalize_remainder_chunks(state, remainder, rem_len, wk, rk_chain);

    /* 2. Final Padding Block */
    uint8_t block[REMAINDER_CHUNK_SIZE] = {0};
    size_t left = rem_len - processed;
    
    if (left > 0) {
        memcpy(block, remainder + processed, left);
    }
    block[left] = 0x80;
    __m512i d_pad = _mm512_aesenc_epi128(_mm512_loadu_si512(block), wk);

    /* 3. Tree Merge (8 -> 4 -> 2 -> 1) */
    avx512_finalize_tree_merge(state);

    /* 4. Quadratic CLMUL Hardening */
    avx512_finalize_clmul_hardening(state);
    
    __m512i save_final = state->acc[0];

    /* 5. Final Block Processing */
    avx512_finalize_block_process(state, d_pad, total_len, domain, rk_chain, save_final);

    /* 6. Key Re-absorption */
    avx512_finalize_key_reabsorption(state, key);

    /* 7. Final Lane Reduction */
    avx512_lane_reduction_4to256(state->acc[0], out);
}

// =============================================================================
// PUBLIC API
// =============================================================================

TARGET_AVX512
void tachyon_avx512_oneshot(const uint8_t *input, size_t len, uint64_t domain, uint64_t seed, const uint8_t *key, uint8_t *out) {
    if (len < REMAINDER_CHUNK_SIZE) {
        tachyon_aesni_oneshot_short(input, len, domain, seed, key, out);
        return;
    }
    
    tachyon_avx512_state_t state;
    tachyon_avx512_init(&state, key, seed);
    
    size_t chunk_len = (len / BLOCK_SIZE) * BLOCK_SIZE;
    if (chunk_len > 0) {
        tachyon_avx512_update(&state, input, chunk_len);
    }
    
    tachyon_avx512_finalize(&state, input + chunk_len, len - chunk_len, len, domain, key, out);
}

#endif // x86_64 or i386
