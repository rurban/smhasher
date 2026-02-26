// Tachyon 
// Copyright (c) byt3forg3
// Licensed under the MIT or Apache 2.0 License
// -------------------------------------------------------------------------

#include <stdint.h>
#include <string.h>
#include "tachyon_impl.h"

#if defined(__x86_64__) || defined(__i386__) || defined(_M_X64) || defined(_M_IX86)

#include <wmmintrin.h>
#include <emmintrin.h>
#include <smmintrin.h>
// =============================================================================
// AES-NI KERNEL
// =============================================================================

#if defined(__GNUC__) || defined(__clang__)
  #define TARGET_AESNI __attribute__((target("aes,sse4.1,pclmul")))
#else
  #define TARGET_AESNI
#endif

typedef struct {
    __m128i acc[32];
    uint64_t block_count;
} tachyon_aesni_state_t;

TARGET_AESNI void tachyon_aesni_init(tachyon_aesni_state_t *state, const uint8_t *key, uint64_t seed);

static inline __m128i init_reg(uint64_t base) {
    return _mm_set_epi64x(base + 1, base);
}

static inline void rotate_lanes(__m128i *acc, int base) {
    __m128i tmp = acc[base];
    acc[base] = acc[base + 1];
    acc[base + 1] = acc[base + 2];
    acc[base + 2] = acc[base + 3];
    acc[base + 3] = tmp;
}

// =============================================================================
// COMPRESSION HELPERS
// =============================================================================

#define AES_MIX(acc, data, rk, lo, blk) \
    _mm_aesenc_si128(acc, _mm_add_epi64(data, _mm_add_epi64(rk, _mm_add_epi64(lo, blk))))

TARGET_AESNI
static void aesni_compress_phase1_roundrobin(tachyon_aesni_state_t *state, __m128i d[NUM_LANES][LANE_STRIDE],
                                             const __m128i rk_base[10], const __m128i lo_all[32], __m128i blk) {
    for (int r = 0; r < 5; r++) {
        __m128i rk = rk_base[r];
        
        for (int i = 0; i < 32; i++) {
            state->acc[i] = AES_MIX(state->acc[i], d[i / 4][i % 4], rk, lo_all[i], blk);
        }
        
        for (int i = 0; i < NUM_LANES; i++) {
            int src = (i + 3) % NUM_LANES;
            for (int j = 0; j < LANE_STRIDE; j++) {
                d[i][j] = _mm_xor_si128(d[i][j], state->acc[ACC_INDEX(src, j)]);
            }
        }
        
        __m128i old[32]; 
        memcpy(old, state->acc, BLOCK_SIZE);
        
        for (int i = 0; i < NUM_LANES; i++) {
            memcpy(&state->acc[ACC_INDEX(i, 0)], &old[ACC_INDEX((i + 1) % NUM_LANES, 0)], LANE_STRIDE * VEC_SIZE);
        }
    }
}

TARGET_AESNI
static void aesni_compress_midblock_mixing(tachyon_aesni_state_t *state) {
    __m128i old_m[32]; 
    memcpy(old_m, state->acc, BLOCK_SIZE);
    
    for (int i = 0; i < NUM_LANES; i++) {
        for (int j = 0; j < 4; j++) {
            state->acc[ACC_INDEX(i, j)] = old_m[ACC_INDEX(i, (j + 1) % 4)];
        }
    }

    /* Cross-Accumulator Diffusion Stage 1 */
    for (int l = 0; l < 4; l++) {
        for (int i = 0; i < 4; i++) {
            __m128i t_lo = state->acc[ACC_INDEX(i, l)];
            __m128i t_hi = state->acc[ACC_INDEX(i + 4, l)];
            
            state->acc[ACC_INDEX(i, l)]     = _mm_xor_si128(t_lo, t_hi);
            state->acc[ACC_INDEX(i + 4, l)] = _mm_add_epi64(t_hi, t_lo);
        }
    }

    /* Cross-Accumulator Diffusion Stage 2 */
    for (int l = 0; l < 4; l++) {
        __m128i a0 = state->acc[ACC_INDEX(0, l)], a2 = state->acc[ACC_INDEX(2, l)];
        state->acc[ACC_INDEX(0, l)] = _mm_xor_si128(a0, a2); 
        state->acc[ACC_INDEX(2, l)] = _mm_add_epi64(a2, a0);
        
        __m128i a1 = state->acc[ACC_INDEX(1, l)], a3 = state->acc[ACC_INDEX(3, l)];
        state->acc[ACC_INDEX(1, l)] = _mm_xor_si128(a1, a3); 
        state->acc[ACC_INDEX(3, l)] = _mm_add_epi64(a3, a1);
        
        __m128i a4 = state->acc[ACC_INDEX(4, l)], a6 = state->acc[ACC_INDEX(6, l)];
        state->acc[ACC_INDEX(4, l)] = _mm_xor_si128(a4, a6); 
        state->acc[ACC_INDEX(6, l)] = _mm_add_epi64(a6, a4);
        
        __m128i a5 = state->acc[ACC_INDEX(5, l)], a7 = state->acc[ACC_INDEX(7, l)];
        state->acc[ACC_INDEX(5, l)] = _mm_xor_si128(a5, a7); 
        state->acc[ACC_INDEX(7, l)] = _mm_add_epi64(a7, a5);
    }
}

TARGET_AESNI
static void aesni_compress_phase2_and_feedforward(tachyon_aesni_state_t *state, __m128i d[NUM_LANES][LANE_STRIDE],
                                                  const __m128i rk_base[10], const __m128i lo_all[32], 
                                                  __m128i blk, const __m128i saves[32]) {
    for (int r = 5; r < 10; r++) {
        __m128i rk = rk_base[r];
        
        for (int i = 0; i < 32; i++) {
            state->acc[i] = AES_MIX(state->acc[i], d[((i / 4) + 4) % 8][i % 4], rk, lo_all[i], blk);
        }
        
        for (int i = 0; i < NUM_LANES; i++) {
            int src = (i + 3) % NUM_LANES;
            for (int j = 0; j < LANE_STRIDE; j++) {
                d[i][j] = _mm_xor_si128(d[i][j], state->acc[ACC_INDEX(src, j)]);
            }
        }
        
        __m128i old[32]; 
        memcpy(old, state->acc, BLOCK_SIZE);
        for (int i = 0; i < NUM_LANES; i++) {
            memcpy(&state->acc[ACC_INDEX(i, 0)], &old[ACC_INDEX((i + 1) % NUM_LANES, 0)], LANE_STRIDE * VEC_SIZE);
        }
    }

    /* Davies-Meyer Feed-Forward */
    __m128i old_f[32]; 
    memcpy(old_f, state->acc, BLOCK_SIZE);
    
    for (int i = 0; i < NUM_LANES; i++) {
        for (int j = 0; j < LANE_STRIDE; j++) {
            state->acc[ACC_INDEX(i, j)] = old_f[ACC_INDEX(i, (j + 1) % LANE_STRIDE)];
        }
    }
    
    for (int i = 0; i < 32; i++) {
        state->acc[i] = _mm_xor_si128(state->acc[i], saves[i]);
    }
}

// =============================================================================
// FINALIZATION HELPERS
// =============================================================================

TARGET_AESNI
static size_t aesni_finalize_remainder_chunks(tachyon_aesni_state_t *state, const uint8_t *remainder, size_t rem_len,
                                              __m128i wk, const __m128i rk_chain[10]) {
    size_t chunk_idx = 0;
    while ((chunk_idx + 1) * REMAINDER_CHUNK_SIZE <= rem_len) {
        const uint8_t *ptr = remainder + chunk_idx * REMAINDER_CHUNK_SIZE;
        __m128i d_rem[LANE_STRIDE];
        
        for (int j = 0; j < LANE_STRIDE; j++) {
            d_rem[j] = _mm_aesenc_si128(_mm_loadu_si128((__m128i*)(ptr + j * VEC_SIZE)), wk);
        }
        
        int base = chunk_idx * LANE_STRIDE;
        __m128i saves[LANE_STRIDE]; 
        memcpy(saves, &state->acc[base], REMAINDER_CHUNK_SIZE);
        
        for (int r = 0; r < 10; r++) {
            __m128i rk = rk_chain[r];
            for (int j = 0; j < LANE_STRIDE; j++) {
                __m128i lo = _mm_set1_epi64x(LANE_OFFSETS[base + j]);
                state->acc[base + j] = _mm_aesenc_si128(state->acc[base + j], _mm_add_epi64(d_rem[j], _mm_add_epi64(rk, lo)));
            }
            
            __m128i t0 = state->acc[base + 0];
            __m128i t1 = state->acc[base + 1];
            __m128i t2 = state->acc[base + 2];
            __m128i t3 = state->acc[base + 3];
            
            d_rem[0] = _mm_xor_si128(d_rem[0], t1);
            d_rem[1] = _mm_xor_si128(d_rem[1], t2);
            d_rem[2] = _mm_xor_si128(d_rem[2], t3);
            d_rem[3] = _mm_xor_si128(d_rem[3], t0);
            
            rotate_lanes(state->acc, base);
        }
        
        for (int j = 0; j < LANE_STRIDE; j++) {
            state->acc[base + j] = _mm_xor_si128(state->acc[base + j], saves[j]);
        }
        
        chunk_idx++;
    }
    
    return chunk_idx * REMAINDER_CHUNK_SIZE;
}

TARGET_AESNI
static void aesni_finalize_tree_merge(tachyon_aesni_state_t *state) {
    __m128i mrk0 = _mm_set1_epi64x(C5);
    __m128i mrk1 = _mm_set1_epi64x(C6);
    __m128i mrk2 = _mm_set1_epi64x(C7);
    
    // Level 0: 32 -> 16
    for (int i = 0; i < 16; i++) {
        state->acc[i] = _mm_aesenc_si128(state->acc[i], _mm_xor_si128(state->acc[i + 16], mrk0));
        state->acc[i] = _mm_aesenc_si128(state->acc[i], _mm_xor_si128(state->acc[i], mrk0));
    }
    
    // Level 1: 16 -> 8
    for (int i = 0; i < 8; i++) {
        state->acc[i] = _mm_aesenc_si128(state->acc[i], _mm_xor_si128(state->acc[i + 8], mrk1));
        state->acc[i] = _mm_aesenc_si128(state->acc[i], _mm_xor_si128(state->acc[i], mrk1));
    }
    
    // Level 2: 8 -> 4
    for (int i = 0; i < LANE_STRIDE; i++) {
        state->acc[i] = _mm_aesenc_si128(state->acc[i], _mm_xor_si128(state->acc[i + LANE_STRIDE], mrk2));
        state->acc[i] = _mm_aesenc_si128(state->acc[i], _mm_xor_si128(state->acc[i], mrk2));
    }
}

TARGET_AESNI
static void aesni_finalize_clmul_hardening(tachyon_aesni_state_t *state) {
    __m128i clmul_k = _mm_set_epi64x(CLMUL_CONSTANT2, CLMUL_CONSTANT);
    
    for (int i = 0; i < LANE_STRIDE; i++) {
        __m128i cl_lo = _mm_clmulepi64_si128(state->acc[i], clmul_k, 0x00);
        __m128i cl_hi = _mm_clmulepi64_si128(state->acc[i], clmul_k, 0x11);
        __m128i cl1   = _mm_xor_si128(cl_lo, cl_hi);
        __m128i mid   = _mm_aesenc_si128(state->acc[i], cl1);
        __m128i cl2   = _mm_clmulepi64_si128(mid, mid, 0x01);
        
        state->acc[i] = _mm_aesenc_si128(state->acc[i], _mm_xor_si128(cl1, cl2));
    }
}

TARGET_AESNI
static void aesni_finalize_block_process(tachyon_aesni_state_t *state, __m128i d_pad[LANE_STRIDE],
                                         uint64_t total_len, uint64_t domain, const __m128i rk_chain[10]) {
    __m128i saves_final[LANE_STRIDE]; 
    memcpy(saves_final, state->acc, REMAINDER_CHUNK_SIZE);
    
    __m128i meta[LANE_STRIDE] = {
        _mm_set_epi64x(CHAOS_BASE, domain ^ total_len),
        _mm_set_epi64x(domain,     total_len),
        _mm_set_epi64x(total_len,  CHAOS_BASE),
        _mm_set_epi64x(CHAOS_BASE, domain)
    };
    
    for (int i = 0; i < LANE_STRIDE; i++) {
        state->acc[i] = _mm_xor_si128(state->acc[i], _mm_xor_si128(d_pad[i], meta[i]));
    }
    
    for (int r = 0; r < 10; r++) {
        __m128i rk = rk_chain[r];
        
        for (int i = 0; i < LANE_STRIDE; i++) {
            state->acc[i] = _mm_aesenc_si128(state->acc[i], _mm_add_epi64(d_pad[i], rk));
        }
        
        if (r % 2 == 1) {
            __m128i t0 = state->acc[0];
            __m128i t1 = state->acc[1];
            __m128i t2 = state->acc[2];
            __m128i t3 = state->acc[3];
            
            d_pad[0] = _mm_xor_si128(d_pad[0], t1);
            d_pad[1] = _mm_xor_si128(d_pad[1], t2);
            d_pad[2] = _mm_xor_si128(d_pad[2], t3);
            d_pad[3] = _mm_xor_si128(d_pad[3], t0);
        }
        
        rotate_lanes(state->acc, 0);
    }
    
    for (int i = 0; i < LANE_STRIDE; i++) {
        state->acc[i] = _mm_xor_si128(state->acc[i], saves_final[i]);
    }
}

TARGET_AESNI
static void aesni_finalize_key_reabsorption(tachyon_aesni_state_t *state, const uint8_t *key) {
    if (!key) return;
    
    __m128i k0 = _mm_loadu_si128((__m128i*)key);
    __m128i k1 = _mm_loadu_si128((__m128i*)(key + VEC_SIZE));
    
    // Round 1
    state->acc[0] = _mm_aesenc_si128(state->acc[0], k0); 
    state->acc[1] = _mm_aesenc_si128(state->acc[1], k1);
    state->acc[2] = _mm_aesenc_si128(state->acc[2], k1); 
    state->acc[3] = _mm_aesenc_si128(state->acc[3], k0);
    
    // Round 2
    state->acc[0] = _mm_aesenc_si128(state->acc[0], k1); 
    state->acc[1] = _mm_aesenc_si128(state->acc[1], k0);
    state->acc[2] = _mm_aesenc_si128(state->acc[2], k0); 
    state->acc[3] = _mm_aesenc_si128(state->acc[3], k1);
    
    // Round 3
    state->acc[0] = _mm_aesenc_si128(state->acc[0], k0); 
    state->acc[1] = _mm_aesenc_si128(state->acc[1], k1);
    state->acc[2] = _mm_aesenc_si128(state->acc[2], k0); 
    state->acc[3] = _mm_aesenc_si128(state->acc[3], k1);
    
    // Round 4
    state->acc[0] = _mm_aesenc_si128(state->acc[0], k0); 
    state->acc[1] = _mm_aesenc_si128(state->acc[1], k0);
    state->acc[2] = _mm_aesenc_si128(state->acc[2], k1); 
    state->acc[3] = _mm_aesenc_si128(state->acc[3], k1);
}

TARGET_AESNI
static void aesni_lane_reduction_4to256(__m128i acc[LANE_STRIDE], uint8_t *out) {
    __m128i mrk0 = _mm_set1_epi64x(C5);
    __m128i mrk1 = _mm_set1_epi64x(C6);
    __m128i mrk2 = _mm_set1_epi64x(C7);
    
    __m128i a[LANE_STRIDE]; 
    for (int i = 0; i < LANE_STRIDE; i++) {
        a[i] = _mm_aesenc_si128(acc[i], acc[i]);
    }
    
    __m128i b0 = _mm_aesenc_si128(a[0], a[2]);
    __m128i b1 = _mm_aesenc_si128(a[1], a[3]);
    __m128i b2 = _mm_aesenc_si128(a[2], a[0]);
    __m128i b3 = _mm_aesenc_si128(a[3], a[1]);
    
    __m128i c0 = _mm_aesenc_si128(b0, b1);
    __m128i c1 = _mm_aesenc_si128(b1, _mm_xor_si128(b0, mrk2));
    __m128i c2 = _mm_aesenc_si128(b2, _mm_xor_si128(b3, mrk1));
    __m128i c3 = _mm_aesenc_si128(b3, _mm_xor_si128(b2, mrk0));
    
    __m128i out_l = _mm_aesenc_si128(c0, c2);
    __m128i out_h = _mm_aesenc_si128(c1, c3);
    
    _mm_storeu_si128((__m128i*)out, _mm_aesenc_si128(out_l, out_h));
    _mm_storeu_si128((__m128i*)(out + VEC_SIZE), _mm_aesenc_si128(out_h, _mm_xor_si128(out_l, mrk2)));
}

TARGET_AESNI
void tachyon_aesni_finalize(tachyon_aesni_state_t *state, const uint8_t *remainder, size_t rem_len, 
                            uint64_t total_len, uint64_t domain, const uint8_t *key, uint8_t *out) {
    __m128i rk_chain[10];
    for (int r = 0; r < 10; r++) {
        rk_chain[r] = _mm_set_epi64x(RK_CHAIN[r][1], RK_CHAIN[r][0]);
    }
    
    __m128i wk = _mm_set_epi64x((int64_t)WHITENING1, (int64_t)WHITENING0);
    
    /* 1. Remainder Chunks */
    size_t processed = aesni_finalize_remainder_chunks(state, remainder, rem_len, wk, rk_chain);

    /* 2. Final Padding Block */
    uint8_t block[REMAINDER_CHUNK_SIZE] = {0};
    size_t left = rem_len - processed;
    
    if (left > 0) {
        memcpy(block, remainder + processed, left);
    }
    block[left] = 0x80;
    
    __m128i d_pad[LANE_STRIDE];
    for (int j = 0; j < LANE_STRIDE; j++) {
        d_pad[j] = _mm_aesenc_si128(_mm_loadu_si128((__m128i*)(block + j * VEC_SIZE)), wk);
    }
    
    /* 3. Tree Merge (32 -> 16 -> 8 -> 4) */
    aesni_finalize_tree_merge(state);

    /* 4. Quadratic CLMUL Hardening */
    aesni_finalize_clmul_hardening(state);
    
    /* 5. Final Block Processing */
    aesni_finalize_block_process(state, d_pad, total_len, domain, rk_chain);
    
    /* 6. Key Re-absorption */
    aesni_finalize_key_reabsorption(state, key);
    
    /* 7. Final Lane Reduction */
    aesni_lane_reduction_4to256(state->acc, out);
}

// =============================================================================
// SHORT PATH (0...63 bytes)
// =============================================================================

TARGET_AESNI
static void aesni_short_initialize_state(__m128i acc[LANE_STRIDE]) {
    acc[0] = _mm_set_epi64x(SHORT_INIT[0][1], SHORT_INIT[0][0]);
    acc[1] = _mm_set_epi64x(SHORT_INIT[1][1], SHORT_INIT[1][0]);
    acc[2] = _mm_set_epi64x(SHORT_INIT[2][1], SHORT_INIT[2][0]);
    acc[3] = _mm_set_epi64x(SHORT_INIT[3][1], SHORT_INIT[3][0]);
}

TARGET_AESNI
static void aesni_short_process_block(__m128i acc[LANE_STRIDE], const uint8_t *input, size_t len, uint64_t domain) {
    __m128i rk_chain[10];
    for (int r = 0; r < 10; r++) {
        rk_chain[r] = _mm_set_epi64x(RK_CHAIN[r][1], RK_CHAIN[r][0]);
    }
    
    __m128i wk = _mm_set_epi64x((int64_t)WHITENING1, (int64_t)WHITENING0);
    
    uint8_t block[REMAINDER_CHUNK_SIZE] = {0};
    if (len > 0) {
        memcpy(block, input, len);
    }
    block[len] = 0x80;
    
    __m128i d0 = _mm_aesenc_si128(_mm_loadu_si128((__m128i*)block), wk);
    __m128i d1 = _mm_aesenc_si128(_mm_loadu_si128((__m128i*)(block + VEC_SIZE)), wk);
    __m128i d2 = _mm_aesenc_si128(_mm_loadu_si128((__m128i*)(block + 32)), wk);
    __m128i d3 = _mm_aesenc_si128(_mm_loadu_si128((__m128i*)(block + 48)), wk);
    
    __m128i saves[4] = {acc[0], acc[1], acc[2], acc[3]};

    __m128i meta[LANE_STRIDE] = {
        _mm_set_epi64x(CHAOS_BASE, domain ^ (uint64_t)len), 
        _mm_set_epi64x(domain, (uint64_t)len),
        _mm_set_epi64x((uint64_t)len, CHAOS_BASE), 
        _mm_set_epi64x(CHAOS_BASE, domain)
    };
    
    acc[0] = _mm_xor_si128(acc[0], _mm_xor_si128(d0, meta[0])); 
    acc[1] = _mm_xor_si128(acc[1], _mm_xor_si128(d1, meta[1]));
    acc[2] = _mm_xor_si128(acc[2], _mm_xor_si128(d2, meta[2])); 
    acc[3] = _mm_xor_si128(acc[3], _mm_xor_si128(d3, meta[3]));
    
    for (int r = 0; r < 10; r++) {
        __m128i rk = rk_chain[r];
        
        acc[0] = _mm_aesenc_si128(acc[0], _mm_add_epi64(d0, _mm_add_epi64(rk, _mm_set1_epi64x(LANE_OFFSETS[0]))));
        acc[1] = _mm_aesenc_si128(acc[1], _mm_add_epi64(d1, _mm_add_epi64(rk, _mm_set1_epi64x(LANE_OFFSETS[1]))));
        acc[2] = _mm_aesenc_si128(acc[2], _mm_add_epi64(d2, _mm_add_epi64(rk, _mm_set1_epi64x(LANE_OFFSETS[2]))));
        acc[3] = _mm_aesenc_si128(acc[3], _mm_add_epi64(d3, _mm_add_epi64(rk, _mm_set1_epi64x(LANE_OFFSETS[3]))));
        
        if (r % 2 == 1) {
            __m128i t0 = acc[0];
            __m128i t1 = acc[1];
            __m128i t2 = acc[2];
            __m128i t3 = acc[3];
            
            d0 = _mm_xor_si128(d0, t1); 
            d1 = _mm_xor_si128(d1, t2);
            d2 = _mm_xor_si128(d2, t3); 
            d3 = _mm_xor_si128(d3, t0);
        }
        rotate_lanes(acc, 0);
    }
    
    for (int i = 0; i < 4; i++) {
        acc[i] = _mm_xor_si128(acc[i], saves[i]);
    }
}

TARGET_AESNI
void tachyon_aesni_oneshot_short(const uint8_t *input, size_t len, uint64_t domain, uint64_t seed, const uint8_t *key, uint8_t *out) {
    if (seed == 0 && !key) {
        __m128i acc[4];
        aesni_short_initialize_state(acc);
        aesni_short_process_block(acc, input, len, domain);
        aesni_lane_reduction_4to256(acc, out);
    } else {
        tachyon_aesni_state_t state;
        tachyon_aesni_init(&state, key, seed);
        tachyon_aesni_finalize(&state, input, len, len, domain, key, out);
    }
}

// =============================================================================
// INITIALIZATION
// =============================================================================

TARGET_AESNI
void tachyon_aesni_init(tachyon_aesni_state_t *state, const uint8_t *key, uint64_t seed) {
    uint64_t C_VALS[NUM_LANES] = {C0, C1, C2, C3, C4, C5, C6, C7};
    for (int i = 0; i < NUM_LANES; i++) {
        uint64_t base = C_VALS[i];
        state->acc[ACC_INDEX(i, 0)] = init_reg(base);
        state->acc[ACC_INDEX(i, 1)] = init_reg(base + 2);
        state->acc[ACC_INDEX(i, 2)] = init_reg(base + 4);
        state->acc[ACC_INDEX(i, 3)] = init_reg(base + 6);
    }
    
    __m128i s_vec = (seed != 0) ? _mm_set1_epi64x(seed) : _mm_set1_epi64x(C5);
    for (int i = 0; i < 32; i++) {
        state->acc[i] = _mm_aesenc_si128(state->acc[i], s_vec);
    }
    
    if (key) {
        __m128i k0 = _mm_loadu_si128((__m128i*)key);
        __m128i k1 = _mm_loadu_si128((__m128i*)(key + VEC_SIZE));
        __m128i gr = _mm_set1_epi64x(GOLDEN_RATIO);
        __m128i k2 = _mm_xor_si128(k0, gr);
        __m128i k3 = _mm_xor_si128(k1, gr);

        for (int i = 0; i < NUM_LANES; i++) {
            __m128i lo = _mm_set1_epi64x(LANE_OFFSETS[i]);
            for (int j = 0; j < LANE_STRIDE; j++) {
                __m128i k = (j == 0) ? k0 : (j == 1) ? k1 : (j == 2) ? k2 : k3;
                state->acc[ACC_INDEX(i, j)] = _mm_aesenc_si128(state->acc[ACC_INDEX(i, j)], _mm_add_epi64(k, lo));
                state->acc[ACC_INDEX(i, j)] = _mm_aesenc_si128(state->acc[ACC_INDEX(i, j)], k);
            }
        }
    }
    state->block_count = 0;
}

// =============================================================================
// COMPRESSION
// =============================================================================

TARGET_AESNI
void tachyon_aesni_update(tachyon_aesni_state_t *state, const uint8_t *input, size_t len) {
    __m128i rk_base[10];
    for (int r = 0; r < 10; r++) {
        rk_base[r] = _mm_set_epi64x(RK_CHAIN[r][1], RK_CHAIN[r][0]);
    }
    
    __m128i wk = _mm_set_epi64x((int64_t)WHITENING1, (int64_t)WHITENING0);
    
    __m128i lo_all[32];
    for (int i = 0; i < 32; i++) {
        lo_all[i] = _mm_set1_epi64x(LANE_OFFSETS[i]);
    }

    size_t processed = 0;
    while (processed + BLOCK_SIZE <= len) {
        __m128i saves[32];
        memcpy(saves, state->acc, BLOCK_SIZE);
        
        const uint8_t *b_ptr = input + processed;
        __m128i blk = _mm_set1_epi64x(state->block_count);

        __m128i d[NUM_LANES][LANE_STRIDE];
        for (int i = 0; i < NUM_LANES; i++) {
            for (int j = 0; j < LANE_STRIDE; j++) {
                d[i][j] = _mm_aesenc_si128(_mm_loadu_si128((__m128i*)(b_ptr + i * (LANE_STRIDE * VEC_SIZE) + j * VEC_SIZE)), wk);
            }
        }

        /* Phase 1: Round-Robin Mix (Direct Mapping) */
        aesni_compress_phase1_roundrobin(state, d, rk_base, lo_all, blk);

        /* Mid-block mixing: Intra-register lane rotation */
        aesni_compress_midblock_mixing(state);

        /* Phase 2: Completion (Offset Mapping) */
        aesni_compress_phase2_and_feedforward(state, d, rk_base, lo_all, blk, saves);

        state->block_count++;
        processed += BLOCK_SIZE;
    }
}

// =============================================================================
// PUBLIC API
// =============================================================================

TARGET_AESNI
void tachyon_aesni_oneshot(const uint8_t *input, size_t len, uint64_t domain, uint64_t seed, const uint8_t *key, uint8_t *out) {
    if (len < REMAINDER_CHUNK_SIZE) {
        tachyon_aesni_oneshot_short(input, len, domain, seed, key, out);
        return;
    }
    
    tachyon_aesni_state_t state;
    tachyon_aesni_init(&state, key, seed);
    
    size_t chunk_len = (len / BLOCK_SIZE) * BLOCK_SIZE;
    if (chunk_len > 0) {
        tachyon_aesni_update(&state, input, chunk_len);
    }
    
    tachyon_aesni_finalize(&state, input + chunk_len, len - chunk_len, len, domain, key, out);
}

#endif // x86_64 or i386
