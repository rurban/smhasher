// Tachyon
// Copyright (c) byt3forg3
// Licensed under the MIT or Apache 2.0 License
// -------------------------------------------------------------------------

#include "tachyon.h"
#include "tachyon_impl.h"
#include <string.h>

// =============================================================================
// PORTABLE KERNEL (PURE C)
// =============================================================================


typedef struct { uint8_t b[VEC_SIZE]; } tachyon_vec128_t;

struct portable_state_t {
    tachyon_vec128_t acc[32];
    uint64_t total_len;
    uint64_t domain;
    uint64_t seed;
    uint8_t key[HASH_SIZE];
    int has_key;
};

// AES S-Box
static const uint8_t SBOX[256] = {
    0x63, 0x7c, 0x77, 0x7b, 0xf2, 0x6b, 0x6f, 0xc5, 0x30, 0x01, 0x67, 0x2b, 0xfe, 0xd7, 0xab, 0x76,
    0xca, 0x82, 0xc9, 0x7d, 0xfa, 0x59, 0x47, 0xf0, 0xad, 0xd4, 0xa2, 0xaf, 0x9c, 0xa4, 0x72, 0xc0,
    0xb7, 0xfd, 0x93, 0x26, 0x36, 0x3f, 0xf7, 0xcc, 0x34, 0xa5, 0xe5, 0xf1, 0x71, 0xd8, 0x31, 0x15,
    0x04, 0xc7, 0x23, 0xc3, 0x18, 0x96, 0x05, 0x9a, 0x07, 0x12, 0x80, 0xe2, 0xeb, 0x27, 0xb2, 0x75,
    0x09, 0x83, 0x2c, 0x1a, 0x1b, 0x6e, 0x5a, 0xa0, 0x52, 0x3b, 0xd6, 0xb3, 0x29, 0xe3, 0x2f, 0x84,
    0x53, 0xd1, 0x00, 0xed, 0x20, 0xfc, 0xb1, 0x5b, 0x6a, 0xcb, 0xbe, 0x39, 0x4a, 0x4c, 0x58, 0xcf,
    0xd0, 0xef, 0xaa, 0xfb, 0x43, 0x4d, 0x33, 0x85, 0x45, 0xf9, 0x02, 0x7f, 0x50, 0x3c, 0x9f, 0xa8,
    0x51, 0xa3, 0x40, 0x8f, 0x92, 0x9d, 0x38, 0xf5, 0xbc, 0xb6, 0xda, 0x21, 0x10, 0xff, 0xf3, 0xd2,
    0xcd, 0x0c, 0x13, 0xec, 0x5f, 0x97, 0x44, 0x17, 0xc4, 0xa7, 0x7e, 0x3d, 0x64, 0x5d, 0x19, 0x73,
    0x60, 0x81, 0x4f, 0xdc, 0x22, 0x2a, 0x90, 0x88, 0x46, 0xee, 0xb8, 0x14, 0xde, 0x5e, 0x0b, 0xdb,
    0xe0, 0x32, 0x3a, 0x0a, 0x49, 0x06, 0x24, 0x5c, 0xc2, 0xd3, 0xac, 0x62, 0x91, 0x95, 0xe4, 0x79,
    0xe7, 0xc8, 0x37, 0x6d, 0x8d, 0xd5, 0x4e, 0xa9, 0x6c, 0x56, 0xf4, 0xea, 0x65, 0x7a, 0xae, 0x08,
    0xba, 0x78, 0x25, 0x2e, 0x1c, 0xa6, 0xb4, 0xc6, 0xe8, 0xdd, 0x74, 0x1f, 0x4b, 0xbd, 0x8b, 0x8a,
    0x70, 0x3e, 0xb5, 0x66, 0x48, 0x03, 0xf6, 0x0e, 0x61, 0x35, 0x57, 0xb9, 0x86, 0xc1, 0x1d, 0x9e,
    0xe1, 0xf8, 0x98, 0x11, 0x69, 0xd9, 0x8e, 0x94, 0x9b, 0x1e, 0x87, 0xe9, 0xce, 0x55, 0x28, 0xdf,
    0x8c, 0xa1, 0x89, 0x0d, 0xbf, 0xe6, 0x42, 0x68, 0x41, 0x99, 0x2d, 0x0f, 0xb0, 0x54, 0xbb, 0x16
};

// =============================================================================
// HELPERS
// =============================================================================

/* Pack two uint64 values into a 128-bit register (lo in bytes 0-7, hi in bytes 8-15). */
static inline tachyon_vec128_t to_m128i(uint64_t lo, uint64_t hi) {
    tachyon_vec128_t r; memcpy(&r.b[0], &lo, 8); memcpy(&r.b[8], &hi, 8); return r;
}

static inline void load_u64(const uint8_t *ptr, uint64_t *out) { memcpy(out, ptr, 8); }
static inline void store_u64(uint8_t *ptr, uint64_t val) { memcpy(ptr, &val, 8); }

static inline tachyon_vec128_t xor_m128i(tachyon_vec128_t a, tachyon_vec128_t b) {
    tachyon_vec128_t r; for (int i = 0; i < VEC_SIZE; i++) r.b[i] = a.b[i] ^ b.b[i]; return r;
}

static inline tachyon_vec128_t add_epi64(tachyon_vec128_t a, tachyon_vec128_t b) {
    tachyon_vec128_t r; uint64_t a0, a1, b0, b1;
    load_u64(&a.b[0], &a0); load_u64(&a.b[8], &a1);
    load_u64(&b.b[0], &b0); load_u64(&b.b[8], &b1);
    store_u64(&r.b[0], a0 + b0); store_u64(&r.b[8], a1 + b1); return r;
}

static inline uint8_t gf_double(uint8_t b) {
    return (uint8_t)((b << 1) ^ ((b >> 7) * GF_POLY)); // AES GF(2^8): x^8 + x^4 + x^3 + x + 1
}

static void mix_column(uint8_t *c) {
    uint8_t t[4] = {c[0], c[1], c[2], c[3]};
    c[0] = gf_double(t[0] ^ t[1]) ^ t[1] ^ t[2] ^ t[3];
    c[1] = gf_double(t[1] ^ t[2]) ^ t[2] ^ t[3] ^ t[0];
    c[2] = gf_double(t[2] ^ t[3]) ^ t[3] ^ t[0] ^ t[1];
    c[3] = gf_double(t[3] ^ t[0]) ^ t[0] ^ t[1] ^ t[2];
}

static tachyon_vec128_t aesenc_s(tachyon_vec128_t state, tachyon_vec128_t key) {
    uint8_t s[VEC_SIZE];
    s[0] = state.b[0];   s[4] = state.b[4];   s[8] = state.b[8];   s[12] = state.b[12];
    s[1] = state.b[5];   s[5] = state.b[9];   s[9] = state.b[13];  s[13] = state.b[1];
    s[2] = state.b[10];  s[6] = state.b[14];  s[10] = state.b[2];  s[14] = state.b[6];
    s[3] = state.b[15];  s[7] = state.b[3];   s[11] = state.b[7];  s[15] = state.b[11];
    for (int i = 0; i < VEC_SIZE; i++) s[i] = SBOX[s[i]];
    mix_column(&s[0]); mix_column(&s[4]); mix_column(&s[8]); mix_column(&s[12]);
    
    tachyon_vec128_t res; for (int i = 0; i < VEC_SIZE; i++) res.b[i] = (uint8_t)(s[i] ^ key.b[i]);
    return res;
}

static void clmul_u64_s(uint64_t a, uint64_t b, uint64_t *r_lo, uint64_t *r_hi) {
    uint64_t hi = 0, lo = 0;
    for (int i = 0; i < 64; i++) {
        /* Branchless: mask is all-ones if bit i of b is set, zero otherwise.
         * Avoids a data-dependent branch that could leak information via timing. */
        uint64_t mask = -(uint64_t)((b >> i) & 1);
        lo ^= (a << i) & mask;
        hi ^= ((i == 0) ? 0 : (a >> (64 - i))) & mask; /* i==0 guard: loop counter, not data */
    }
    *r_lo = lo; *r_hi = hi;
}

static tachyon_vec128_t clmulepi64_s(tachyon_vec128_t a, tachyon_vec128_t b, int imm) {
    uint64_t a_val, b_val, lo, hi;
    load_u64(&a.b[(imm & 0x01) ? 8 : 0], &a_val);
    load_u64(&b.b[(imm & 0x10) ? 8 : 0], &b_val);
    clmul_u64_s(a_val, b_val, &lo, &hi); return to_m128i(lo, hi);
}

// =============================================================================
// LOGIC
// =============================================================================

static void linear_init(struct portable_state_t *s) {
    const uint64_t c_vals[NUM_LANES] = {C0, C1, C2, C3, C4, C5, C6, C7};
    for (int i = 0; i < 32; i++) {
        uint64_t base = c_vals[i / LANE_STRIDE];
        uint64_t offset = (uint64_t)(i % LANE_STRIDE) * 2;
        s->acc[i] = to_m128i(base + offset, base + offset + 1);
    }
    uint64_t seed_val = s->seed ? s->seed : C5;
    tachyon_vec128_t seed_vec = to_m128i(seed_val, seed_val);
    for (int i = 0; i < 32; i++) {
        s->acc[i] = aesenc_s(s->acc[i], seed_vec);
    }

    if (s->has_key) {
        tachyon_vec128_t k0, k1;
        memcpy(k0.b, &s->key[0], VEC_SIZE);
        memcpy(k1.b, &s->key[VEC_SIZE], VEC_SIZE);
        tachyon_vec128_t gr = to_m128i(GOLDEN_RATIO, GOLDEN_RATIO);
        tachyon_vec128_t keys[LANE_STRIDE] = {k0, k1, xor_m128i(k0, gr), xor_m128i(k1, gr)};
        for (int i = 0; i < NUM_LANES; i++) {
            tachyon_vec128_t lo = to_m128i(LANE_OFFSETS[i], LANE_OFFSETS[i]);
            for (int j = 0; j < LANE_STRIDE; j++) {
                int idx = ACC_INDEX(i, j);
                s->acc[idx] = aesenc_s(s->acc[idx], add_epi64(keys[j], lo));
                s->acc[idx] = aesenc_s(s->acc[idx], keys[j]);
            }
        }
    }
}

/* Phase 1: Round-Robin Mix (first 5 rounds with lane rotation). */
static void compress_phase1_roundrobin(struct portable_state_t *s, tachyon_vec128_t d[NUM_LANES][LANE_STRIDE],
                                       const tachyon_vec128_t rk_base[ROUNDS], const tachyon_vec128_t lo_all[32],
                                       tachyon_vec128_t blk) {
    for (int r = 0; r < 5; r++) {
        tachyon_vec128_t rk = rk_base[r];
        for (int i = 0; i < 32; i++) {
            s->acc[i] = aesenc_s(s->acc[i], add_epi64(d[i / LANE_STRIDE][i % LANE_STRIDE], add_epi64(rk, add_epi64(lo_all[i], blk))));
        }
        for (int i = 0; i < NUM_LANES; i++) {
            int src = (i + 3) % NUM_LANES;  // Feedback from lane i+3 (mod 8) for diffusion
            for (int j = 0; j < LANE_STRIDE; j++) {
                d[i][j] = xor_m128i(d[i][j], s->acc[ACC_INDEX(src, j)]);
            }
        }
        tachyon_vec128_t old[32];
        memcpy(old, s->acc, STATE_SIZE);

        for (int i = 0; i < NUM_LANES; i++) {
            memcpy(&s->acc[ACC_INDEX(i, 0)], &old[ACC_INDEX((i + 1) % NUM_LANES, 0)], LANE_STRIDE * VEC_SIZE);
        }
    }
}

/* Mid-block mixing: break lane symmetry with element rotation and XOR/ADD mixing. */
static void compress_midblock_mixing(struct portable_state_t *s) {
    // Treating acc[32] as 8-lane × 4-element matrix: acc[lane*4 + elem]
    tachyon_vec128_t old_m[32];
    memcpy(old_m, s->acc, STATE_SIZE);

    for (int i = 0; i < NUM_LANES; i++) {
        for (int j = 0; j < LANE_STRIDE; j++) {
            s->acc[ACC_INDEX(i, j)] = old_m[ACC_INDEX(i, (j + 1) % LANE_STRIDE)];
        }
    }
    for (int lane_offset = 0; lane_offset < LANE_STRIDE; lane_offset++) {
        for (int i = 0; i < 4; i++) {
            tachyon_vec128_t t_lo = s->acc[ACC_INDEX(i, lane_offset)];
            tachyon_vec128_t t_hi = s->acc[ACC_INDEX(i + 4, lane_offset)];
            s->acc[ACC_INDEX(i, lane_offset)]     = xor_m128i(t_lo, t_hi);
            s->acc[ACC_INDEX(i + 4, lane_offset)] = add_epi64(t_hi, t_lo);
        }
    }
    for (int lane_offset = 0; lane_offset < LANE_STRIDE; lane_offset++) {
        tachyon_vec128_t a0 = s->acc[ACC_INDEX(0, lane_offset)], a2 = s->acc[ACC_INDEX(2, lane_offset)];
        s->acc[ACC_INDEX(0, lane_offset)] = xor_m128i(a0, a2);
        s->acc[ACC_INDEX(2, lane_offset)] = add_epi64(a2, a0);

        tachyon_vec128_t a1 = s->acc[ACC_INDEX(1, lane_offset)], a3 = s->acc[ACC_INDEX(3, lane_offset)];
        s->acc[ACC_INDEX(1, lane_offset)] = xor_m128i(a1, a3);
        s->acc[ACC_INDEX(3, lane_offset)] = add_epi64(a3, a1);

        tachyon_vec128_t a4 = s->acc[ACC_INDEX(4, lane_offset)], a6 = s->acc[ACC_INDEX(6, lane_offset)];
        s->acc[ACC_INDEX(4, lane_offset)] = xor_m128i(a4, a6);
        s->acc[ACC_INDEX(6, lane_offset)] = add_epi64(a6, a4);

        tachyon_vec128_t a5 = s->acc[ACC_INDEX(5, lane_offset)], a7 = s->acc[ACC_INDEX(7, lane_offset)];
        s->acc[ACC_INDEX(5, lane_offset)] = xor_m128i(a5, a7);
        s->acc[ACC_INDEX(7, lane_offset)] = add_epi64(a7, a5);
    }
}

/* Phase 2: Completion rounds (5-9) with shifted data index and Davies-Meyer feed-forward. */
static void compress_phase2_and_feedforward(struct portable_state_t *s, tachyon_vec128_t d[NUM_LANES][LANE_STRIDE],
                                            const tachyon_vec128_t rk_base[ROUNDS], const tachyon_vec128_t lo_all[32],
                                            tachyon_vec128_t blk, const tachyon_vec128_t saves[32]) {
    for (int r = 5; r < ROUNDS; r++) {
        tachyon_vec128_t rk = rk_base[r];
        for (int i = 0; i < 32; i++) {
            s->acc[i] = aesenc_s(s->acc[i], add_epi64(d[(i / LANE_STRIDE + 4) % NUM_LANES][i % LANE_STRIDE], add_epi64(rk, add_epi64(lo_all[i], blk))));
        }
        for (int i = 0; i < NUM_LANES; i++) {
            int src = (i + 3) % NUM_LANES;  // Feedback from lane i+3 (mod 8) for diffusion
            for (int j = 0; j < LANE_STRIDE; j++) {
                d[i][j] = xor_m128i(d[i][j], s->acc[ACC_INDEX(src, j)]);
            }
        }
        tachyon_vec128_t old[32];
        memcpy(old, s->acc, STATE_SIZE);
        for (int i = 0; i < NUM_LANES; i++) {
            memcpy(&s->acc[ACC_INDEX(i, 0)], &old[ACC_INDEX((i + 1) % NUM_LANES, 0)], LANE_STRIDE * VEC_SIZE);
        }
    }

    // Davies-Meyer Feed-Forward
    tachyon_vec128_t old_f[32];
    memcpy(old_f, s->acc, STATE_SIZE);
    for (int i = 0; i < NUM_LANES; i++) {
        for (int j = 0; j < LANE_STRIDE; j++) {
            s->acc[ACC_INDEX(i, j)] = old_f[ACC_INDEX(i, (j + 1) % LANE_STRIDE)];
        }
    }
    for (int i = 0; i < 32; i++) {
        s->acc[i] = xor_m128i(s->acc[i], saves[i]);
    }
}

static void linear_compress(struct portable_state_t *s, const uint8_t *data, uint64_t block_idx) {
    tachyon_vec128_t blk = to_m128i(block_idx, block_idx);
    tachyon_vec128_t wk  = to_m128i(WHITENING0, WHITENING1);

    tachyon_vec128_t rk_base[ROUNDS];
    for (int i = 0; i < ROUNDS; i++) {
        rk_base[i] = to_m128i(RK_CHAIN[i][0], RK_CHAIN[i][1]);
    }

    tachyon_vec128_t lo_all[32];
    for (int i = 0; i < 32; i++) {
        lo_all[i] = to_m128i(LANE_OFFSETS[i], LANE_OFFSETS[i]);
    }

    /* Davies-Meyer feed-forward: save state before compression. */
    tachyon_vec128_t saves[32];
    memcpy(saves, s->acc, STATE_SIZE);

    tachyon_vec128_t d[NUM_LANES][LANE_STRIDE];
    for (int i = 0; i < NUM_LANES; i++) {
        for (int j = 0; j < LANE_STRIDE; j++) {
            tachyon_vec128_t val;
            memcpy(val.b, data + DATA_INDEX(i, j) * VEC_SIZE, VEC_SIZE);
            d[i][j] = aesenc_s(val, wk);
        }
    }

    compress_phase1_roundrobin(s, d, rk_base, lo_all, blk);
    compress_midblock_mixing(s);
    compress_phase2_and_feedforward(s, d, rk_base, lo_all, blk, saves);
}

/* Process remainder chunks (< BLOCK_SIZE bytes, in REMAINDER_CHUNK_SIZE-byte increments). */
static size_t finalize_remainder_chunks(struct portable_state_t *s, const uint8_t *remainder, size_t rem_len,
                                        tachyon_vec128_t wk, const tachyon_vec128_t rk_chain[ROUNDS]) {
    size_t off = 0;
    int chunk_idx = 0;

    while (rem_len - off >= REMAINDER_CHUNK_SIZE) {
        const uint8_t *ptr = remainder + off;
        tachyon_vec128_t d_rem[LANE_STRIDE];
        for (int j = 0; j < LANE_STRIDE; j++) {
            tachyon_vec128_t val;
            memcpy(val.b, ptr + j * VEC_SIZE, VEC_SIZE);
            d_rem[j] = aesenc_s(val, wk);
        }
        int base = chunk_idx * LANE_STRIDE;
        tachyon_vec128_t saves[LANE_STRIDE];
        memcpy(saves, &s->acc[base], LANE_STRIDE * VEC_SIZE);
        for (int r = 0; r < ROUNDS; r++) {
            tachyon_vec128_t rk = rk_chain[r];
            for (int j = 0; j < LANE_STRIDE; j++) {
                s->acc[base + j] = aesenc_s(s->acc[base + j], add_epi64(d_rem[j], add_epi64(rk, to_m128i(LANE_OFFSETS[base + j], LANE_OFFSETS[base + j]))));
            }
            tachyon_vec128_t t0 = s->acc[base], t1 = s->acc[base + 1], t2 = s->acc[base + 2], t3 = s->acc[base + 3];
            d_rem[0] = xor_m128i(d_rem[0], t1);
            d_rem[1] = xor_m128i(d_rem[1], t2);
            d_rem[2] = xor_m128i(d_rem[2], t3);
            d_rem[3] = xor_m128i(d_rem[3], t0);
            s->acc[base] = t1;
            s->acc[base + 1] = t2;
            s->acc[base + 2] = t3;
            s->acc[base + 3] = t0;
        }
        for (int j = 0; j < LANE_STRIDE; j++) {
            s->acc[base + j] = xor_m128i(s->acc[base + j], saves[j]);
        }
        off += REMAINDER_CHUNK_SIZE;
        chunk_idx++;
    }

    return off;
}

/* Three-level tree merge: 32 → 16 → 8 → 4 lanes. */
static void finalize_tree_merge(struct portable_state_t *s) {
    tachyon_vec128_t mrk0 = to_m128i(C5, C5), mrk1 = to_m128i(C6, C6), mrk2 = to_m128i(C7, C7);

    // Level 0: 32 -> 16
    for (int i = 0; i < 16; i++) {
        s->acc[i] = aesenc_s(s->acc[i], xor_m128i(s->acc[i + 16], mrk0));
        s->acc[i] = aesenc_s(s->acc[i], xor_m128i(s->acc[i], mrk0));
    }
    // Level 1: 16 -> 8
    for (int i = 0; i < 8; i++) {
        s->acc[i] = aesenc_s(s->acc[i], xor_m128i(s->acc[i + 8], mrk1));
        s->acc[i] = aesenc_s(s->acc[i], xor_m128i(s->acc[i], mrk1));
    }
    // Level 2: 8 -> 4
    for (int i = 0; i < LANE_STRIDE; i++) {
        s->acc[i] = aesenc_s(s->acc[i], xor_m128i(s->acc[i + LANE_STRIDE], mrk2));
        s->acc[i] = aesenc_s(s->acc[i], xor_m128i(s->acc[i], mrk2));
    }
}

/* Quadratic CLMUL hardening: polynomial mixing to eliminate linear shortcuts. */
static void finalize_clmul_hardening(struct portable_state_t *s) {
    tachyon_vec128_t clmul_k = to_m128i(CLMUL_CONSTANT, CLMUL_CONSTANT2);
    for(int i = 0; i < LANE_STRIDE; i++) {
        // Round 1: polynomial mixing in GF(2)[x]
        tachyon_vec128_t cl1 = xor_m128i(clmulepi64_s(s->acc[i], clmul_k, 0x00), clmulepi64_s(s->acc[i], clmul_k, 0x11));
        // AES barrier: polynomial product as round key (degree ~254)
        tachyon_vec128_t mid = aesenc_s(s->acc[i], cl1);
        // Round 2: self-multiply lo×hi → quadratic in GF(2)[x] (degree ~254²)
        tachyon_vec128_t cl2 = clmulepi64_s(mid, mid, 0x01);
        // Nonlinear fold: aesenc eliminates linear shortcut back to original state
        s->acc[i] = aesenc_s(s->acc[i], xor_m128i(cl1, cl2));
    }
}

/* Final block processing: inject length/domain and perform AES rounds. */
static void finalize_block_process(struct portable_state_t *s, tachyon_vec128_t d_pad[LANE_STRIDE],
                                   uint64_t total_len, const tachyon_vec128_t rk_chain[ROUNDS]) {
    tachyon_vec128_t save_final[LANE_STRIDE];
    tachyon_vec128_t meta[LANE_STRIDE];
    for (int j = 0; j < LANE_STRIDE; j++) {
        save_final[j] = s->acc[j];
    }
    meta[0] = to_m128i(s->domain ^ total_len, CHAOS_BASE);
    meta[1] = to_m128i(total_len, s->domain);
    meta[2] = to_m128i(CHAOS_BASE, total_len);
    meta[3] = to_m128i(s->domain, CHAOS_BASE);

    for (int i = 0; i < LANE_STRIDE; i++) {
        s->acc[i] = xor_m128i(xor_m128i(s->acc[i], d_pad[i]), meta[i]);
    }
    for (int r = 0; r < ROUNDS; r++) {
        tachyon_vec128_t rk = rk_chain[r];
        for (int i = 0; i < LANE_STRIDE; i++) {
            s->acc[i] = aesenc_s(s->acc[i], add_epi64(d_pad[i], rk));
        }
        // Unconditional Lane Rotation (Matches valignq/alignr behavior)
        tachyon_vec128_t tmp = s->acc[0];
        s->acc[0] = s->acc[1];
        s->acc[1] = s->acc[2];
        s->acc[2] = s->acc[3];
        s->acc[3] = tmp;

        if (r % 2 == 1) {
            d_pad[0] = xor_m128i(d_pad[0], s->acc[0]);
            d_pad[1] = xor_m128i(d_pad[1], s->acc[1]);
            d_pad[2] = xor_m128i(d_pad[2], s->acc[2]);
            d_pad[3] = xor_m128i(d_pad[3], s->acc[3]);
        }
    }
    for (int i = 0; i < LANE_STRIDE; i++) {
        s->acc[i] = xor_m128i(s->acc[i], save_final[i]);
    }
}

/* Lane reduction helper (shared between short and finalize paths). */
static void lane_reduction_4to256(tachyon_vec128_t acc[LANE_STRIDE], uint8_t *out) {
    tachyon_vec128_t mrk0 = to_m128i(C5, C5), mrk1 = to_m128i(C6, C6), mrk2 = to_m128i(C7, C7);

    tachyon_vec128_t a[LANE_STRIDE];
    for (int i = 0; i < LANE_STRIDE; i++) {
        a[i] = aesenc_s(acc[i], acc[i]);
    }

    // Round 1: distant swap (0<->2, 1<->3)
    tachyon_vec128_t b[LANE_STRIDE];
    b[0] = aesenc_s(a[0], a[2]);
    b[1] = aesenc_s(a[1], a[3]);
    b[2] = aesenc_s(a[2], a[0]);
    b[3] = aesenc_s(a[3], a[1]);

    // Round 2: adjacent swap (0<->1, 2<->3) + per-lane asymmetry constants
    tachyon_vec128_t zero = to_m128i(0, 0);
    tachyon_vec128_t c[LANE_STRIDE];
    c[0] = aesenc_s(b[0], xor_m128i(b[1], zero)); /* lane 0: no constant  */
    c[1] = aesenc_s(b[1], xor_m128i(b[0], mrk2)); /* lane 1: C7           */
    c[2] = aesenc_s(b[2], xor_m128i(b[3], mrk1)); /* lane 2: C6           */
    c[3] = aesenc_s(b[3], xor_m128i(b[2], mrk0)); /* lane 3: C5           */

    // Round 3: distant swap again
    tachyon_vec128_t fd[LANE_STRIDE];
    fd[0] = aesenc_s(c[0], c[2]);
    fd[1] = aesenc_s(c[1], c[3]);
    fd[2] = aesenc_s(c[2], c[0]);
    fd[3] = aesenc_s(c[3], c[1]);

    // Round 4: adjacent swap + asymmetry again
    tachyon_vec128_t e[LANE_STRIDE];
    e[0] = aesenc_s(fd[0], xor_m128i(fd[1], zero));
    e[1] = aesenc_s(fd[1], xor_m128i(fd[0], mrk2));
    e[2] = aesenc_s(fd[2], xor_m128i(fd[3], mrk1));
    e[3] = aesenc_s(fd[3], xor_m128i(fd[2], mrk0));

    // Output: first two lanes = 256 bits
    memcpy(out,      e[0].b, VEC_SIZE);
    memcpy(out + VEC_SIZE, e[1].b, VEC_SIZE);
}

/* Re-absorb keying material if present (keyed mode only). */
static void finalize_key_reabsorption(struct portable_state_t *s) {
    if (!s->has_key) return;

    tachyon_vec128_t k0, k1;
    memcpy(k0.b, s->key, VEC_SIZE);
    memcpy(k1.b, s->key + VEC_SIZE, VEC_SIZE);

    // Round 1
    s->acc[0] = aesenc_s(s->acc[0], k0);
    s->acc[1] = aesenc_s(s->acc[1], k1);
    s->acc[2] = aesenc_s(s->acc[2], k1);
    s->acc[3] = aesenc_s(s->acc[3], k0);
    // Round 2
    s->acc[0] = aesenc_s(s->acc[0], k1);
    s->acc[1] = aesenc_s(s->acc[1], k0);
    s->acc[2] = aesenc_s(s->acc[2], k0);
    s->acc[3] = aesenc_s(s->acc[3], k1);
    // Round 3
    s->acc[0] = aesenc_s(s->acc[0], k0);
    s->acc[1] = aesenc_s(s->acc[1], k1);
    s->acc[2] = aesenc_s(s->acc[2], k0);
    s->acc[3] = aesenc_s(s->acc[3], k1);
    // Round 4
    s->acc[0] = aesenc_s(s->acc[0], k0);
    s->acc[1] = aesenc_s(s->acc[1], k0);
    s->acc[2] = aesenc_s(s->acc[2], k1);
    s->acc[3] = aesenc_s(s->acc[3], k1);
}

/* Final lane reduction: 4 lanes → 256-bit output via AES permutation cascade. */
static void finalize_lane_reduction(struct portable_state_t *s, uint8_t *out) {
    lane_reduction_4to256(s->acc, out);
}

static void linear_finalize(struct portable_state_t *s, const uint8_t *remainder, size_t rem_len, uint64_t total_len, uint8_t *out) {
    tachyon_vec128_t wk = to_m128i(WHITENING0, WHITENING1);
    tachyon_vec128_t rk_chain[ROUNDS];
    for (int i = 0; i < ROUNDS; i++) {
        rk_chain[i] = to_m128i(RK_CHAIN[i][0], RK_CHAIN[i][1]);
    }

    // 1. Process remainder chunks (< BLOCK_SIZE bytes, in REMAINDER_CHUNK_SIZE-byte increments)
    size_t off = finalize_remainder_chunks(s, remainder, rem_len, wk, rk_chain);

    // 2. Prepare final padding block
    uint8_t blk_pad[REMAINDER_CHUNK_SIZE] = {0};
    if (rem_len > off) {
        memcpy(blk_pad, remainder + off, rem_len - off);
    }
    blk_pad[rem_len - off] = 0x80;  // Merkle-Damgård padding sentinel

    tachyon_vec128_t d_pad[LANE_STRIDE];
    for (int j = 0; j < LANE_STRIDE; j++) {
        tachyon_vec128_t val;
        memcpy(val.b, blk_pad + j * VEC_SIZE, VEC_SIZE);
        d_pad[j] = aesenc_s(val, wk);
    }

    // 3. Tree merge: 32 → 16 → 8 → 4 lanes
    finalize_tree_merge(s);

    // 4. Quadratic CLMUL hardening
    finalize_clmul_hardening(s);

    // 5. Final block processing (inject length/domain)
    finalize_block_process(s, d_pad, total_len, rk_chain);

    // 6. Key re-absorption (keyed mode only)
    finalize_key_reabsorption(s);

    // 7. Final lane reduction: 4 lanes → 256-bit output
    finalize_lane_reduction(s, out);
}

/* Initialize short-path state (seed + optional key absorption). */
static void short_initialize_state(tachyon_vec128_t acc[LANE_STRIDE], uint64_t seed, const uint8_t *key) {
    if (seed == 0 && !key) {
        for (int i = 0; i < LANE_STRIDE; i++) {
            acc[i] = to_m128i(SHORT_INIT[i][0], SHORT_INIT[i][1]);
        }
    } else {
        uint64_t base = C0;

        for (int i = 0; i < LANE_STRIDE; i++) {
            acc[i] = to_m128i(base + (uint64_t)i * 2, base + (uint64_t)i * 2 + 1);
        }

        uint64_t seed_val = (seed != 0) ? seed : C5;
        tachyon_vec128_t s_vec = to_m128i(seed_val, seed_val);

        for (int i = 0; i < LANE_STRIDE; i++) {
            acc[i] = aesenc_s(acc[i], s_vec);
        }

        if (key) {
            tachyon_vec128_t k0, k1;
            memcpy(k0.b, key, VEC_SIZE);
            memcpy(k1.b, key + VEC_SIZE, VEC_SIZE);

            tachyon_vec128_t gr = to_m128i(GOLDEN_RATIO, GOLDEN_RATIO);
            tachyon_vec128_t keys[LANE_STRIDE] = {
                k0,
                k1,
                xor_m128i(k0, gr),
                xor_m128i(k1, gr)
            };

            tachyon_vec128_t lo = to_m128i(LANE_OFFSETS[0], LANE_OFFSETS[0]);

            for (int j = 0; j < LANE_STRIDE; j++) {
                 acc[j] = aesenc_s(acc[j], add_epi64(keys[j], lo));
                 acc[j] = aesenc_s(acc[j], keys[j]);
            }
        }
    }
}

/* Process single block with Davies-Meyer construction. */
static void short_process_block(tachyon_vec128_t acc[LANE_STRIDE], const uint8_t *input, size_t len, uint64_t domain) {
    /* Pre-whiten data block and set up padding. */
    tachyon_vec128_t wk = to_m128i(WHITENING0, WHITENING1);
    tachyon_vec128_t d[LANE_STRIDE];
    uint8_t blk[REMAINDER_CHUNK_SIZE] = {0};

    if (len > 0) {
        memcpy(blk, input, len);
    }
    blk[len] = 0x80; /* Merkle-Damgård padding sentinel */

    for (int i = 0; i < LANE_STRIDE; i++) {
        tachyon_vec128_t v;
        memcpy(v.b, blk + i * VEC_SIZE, VEC_SIZE);
        d[i] = aesenc_s(v, wk);
    }

    /* Davies-Meyer save + domain/length injection. */
    tachyon_vec128_t saves[LANE_STRIDE];
    for (int i = 0; i < LANE_STRIDE; i++) {
        saves[i] = acc[i];
    }

    tachyon_vec128_t meta[LANE_STRIDE] = {
        to_m128i(domain ^ (uint64_t)len, CHAOS_BASE),
        to_m128i((uint64_t)len, domain),
        to_m128i(CHAOS_BASE, (uint64_t)len),
        to_m128i(domain, CHAOS_BASE)
    };

    for (int i = 0; i < LANE_STRIDE; i++) {
        acc[i] = xor_m128i(acc[i], xor_m128i(d[i], meta[i]));
    }

    for (int r = 0; r < ROUNDS; r++) {
        tachyon_vec128_t rk = to_m128i(RK_CHAIN[r][0], RK_CHAIN[r][1]);

        for (int i = 0; i < LANE_STRIDE; i++) {
            acc[i] = aesenc_s(acc[i], add_epi64(d[i], add_epi64(rk, to_m128i(LANE_OFFSETS[i], LANE_OFFSETS[i]))));
        }

        if (r % 2 == 1) {
            tachyon_vec128_t t0 = acc[0], t1 = acc[1], t2 = acc[2], t3 = acc[3];
            d[0] = xor_m128i(d[0], t1);
            d[1] = xor_m128i(d[1], t2);
            d[2] = xor_m128i(d[2], t3);
            d[3] = xor_m128i(d[3], t0);
        }

        /* Lane rotation: cyclic shift acc[0..3] left by one. */
        tachyon_vec128_t tmp = acc[0];
        acc[0] = acc[1];
        acc[1] = acc[2];
        acc[2] = acc[3];
        acc[3] = tmp;
    }

    for (int i = 0; i < LANE_STRIDE; i++) {
        acc[i] = xor_m128i(acc[i], saves[i]);
    }
}

static void hash_short(const uint8_t *input, size_t len, uint64_t domain, uint64_t seed, const uint8_t *key, uint8_t *out) {
    tachyon_vec128_t acc[LANE_STRIDE];

    short_initialize_state(acc, seed, key);
    short_process_block(acc, input, len, domain);
    lane_reduction_4to256(acc, out);
}

// =============================================================================
// PUBLIC API
// =============================================================================

void tachyon_portable_oneshot(const uint8_t *data, size_t len, uint64_t domain, uint64_t seed, const uint8_t *key, uint8_t *out) {
    if (len < REMAINDER_CHUNK_SIZE && seed == 0 && !key) {
        hash_short(data, len, domain, seed, key, out);
        return;
    }
    struct portable_state_t s;
    memset(&s, 0, sizeof(s));
    s.domain = domain;
    s.seed = seed;
    if (key) {
        memcpy(s.key, key, HASH_SIZE);
        s.has_key = 1;
    }
    linear_init(&s);
    size_t off = 0;
    uint64_t b_idx = 0;
    while (len - off >= BLOCK_SIZE) {
        linear_compress(&s, data + off, b_idx++);
        off += BLOCK_SIZE;
    }
    linear_finalize(&s, data + off, len - off, (uint64_t)len, out);
}
