/* lanehash - C port of the Rust definition (lanehash/src/{spec,short}.rs).
 *
 *   len <= 64: one to four protected 64x64->128 multiplies over the input words with
 *              fixed secrets; the mixed seed ks and the length enter the last multiply;
 *              one unprotected fold per output half.
 *   len >  64: eight chains of eight 64-bit lanes, chain c starting at CT[8c+i] + ks,
 *              ks = fold(seed ^ SEED0, SEED1); 64-byte stripe s goes to chain s mod 8,
 *              per lane x = A ^ w, A = x + lo32(x) * hi32(x); the last 64 bytes close
 *              chain m mod 8 with w ^ len, m = (len - 1) / 64; each chain that saw data
 *              takes one more step with Q; lane-wise XOR; four folds; a final fold with
 *              the length and ks.
 *
 * Backends, chosen at compile time, all bit-identical: scalar (LANEHASH_PORTABLE or
 * non-x86), SSE2, AVX2 (-mavx2), AVX-512F (-mavx512f).
 * Verification values (SMHasher): 0xD048C22B (64-bit), 0xC2F39939 (128-bit).
 *
 * SPDX-License-Identifier: MIT OR Apache-2.0
 */
#include "lanehash.h"
#include <string.h>

#include "lanehash_constants.h"

#if !defined(LANEHASH_PORTABLE) && (defined(__x86_64__) || defined(_M_X64))
#  if defined(__AVX512F__)
#    define LANEHASH_AVX512 1
#  elif defined(__AVX2__)
#    define LANEHASH_AVX2 1
#  else
#    define LANEHASH_SSE2 1
#  endif
#  include <immintrin.h>
#endif

#if defined(__GNUC__)
#  define LANEHASH_NOINLINE __attribute__((noinline))
#  define LANEHASH_INLINE static inline __attribute__((always_inline))
#elif defined(_MSC_VER)
#  define LANEHASH_NOINLINE __declspec(noinline)
#  define LANEHASH_INLINE static __forceinline
#else
#  define LANEHASH_NOINLINE
#  define LANEHASH_INLINE static inline
#endif
#if defined(__clang__)
#  define LANEHASH_UNROLL _Pragma("clang loop unroll(full)")
#elif defined(__GNUC__) && __GNUC__ >= 8
#  define LANEHASH_UNROLL _Pragma("GCC unroll 8")
#else
#  define LANEHASH_UNROLL
#endif

/* ------------------------------------------------------------------------- */
/* scalar helpers                                                             */

#if defined(__SIZEOF_INT128__) && !defined(LANEHASH_NO_INT128)
__extension__ typedef unsigned __int128 lanehash_u128;
LANEHASH_INLINE void lanehash_mul128(uint64_t a, uint64_t b, uint64_t *lo, uint64_t *hi) {
    lanehash_u128 r = (lanehash_u128)a * b;
    *lo = (uint64_t)r;
    *hi = (uint64_t)(r >> 64);
}
#elif defined(_MSC_VER) && (defined(_M_X64) || defined(_M_ARM64))
#  include <intrin.h>
LANEHASH_INLINE void lanehash_mul128(uint64_t a, uint64_t b, uint64_t *lo, uint64_t *hi) {
    *lo = _umul128(a, b, hi);
}
#else
LANEHASH_INLINE void lanehash_mul128(uint64_t a, uint64_t b, uint64_t *lo, uint64_t *hi) {
    uint64_t a0 = (uint32_t)a, a1 = a >> 32, b0 = (uint32_t)b, b1 = b >> 32;
    uint64_t p00 = a0 * b0, p01 = a0 * b1, p10 = a1 * b0, p11 = a1 * b1;
    uint64_t mid = (p00 >> 32) + (uint32_t)p01 + (uint32_t)p10;
    *lo = (mid << 32) | (uint32_t)p00;
    *hi = p11 + (p01 >> 32) + (p10 >> 32) + (mid >> 32);
}
#endif

LANEHASH_INLINE uint64_t lanehash_fold(uint64_t a, uint64_t b) {
    uint64_t lo, hi;
    lanehash_mul128(a, b, &lo, &hi);
    return lo ^ hi;
}
/* protected mum: (a ^ lo, b ^ hi) */
LANEHASH_INLINE void lanehash_mum(uint64_t *a, uint64_t *b) {
    uint64_t lo, hi;
    lanehash_mul128(*a, *b, &lo, &hi);
    *a ^= lo;
    *b ^= hi;
}
LANEHASH_INLINE uint64_t lanehash_le64(const uint8_t *p) {
    uint64_t v;
    memcpy(&v, p, 8);
#if defined(__BYTE_ORDER__) && (__BYTE_ORDER__ == __ORDER_BIG_ENDIAN__)
    v = __builtin_bswap64(v);
#endif
    return v;
}
LANEHASH_INLINE uint64_t lanehash_le32(const uint8_t *p) {
    uint32_t v;
    memcpy(&v, p, 4);
#if defined(__BYTE_ORDER__) && (__BYTE_ORDER__ == __ORDER_BIG_ENDIAN__)
    v = __builtin_bswap32(v);
#endif
    return (uint64_t)v;
}
LANEHASH_INLINE void lanehash_put64(uint8_t *p, uint64_t v) {
#if defined(__BYTE_ORDER__) && (__BYTE_ORDER__ == __ORDER_BIG_ENDIAN__)
    v = __builtin_bswap64(v);
#endif
    memcpy(p, &v, 8);
}
/* the lane step: x = a ^ w; x + lo32(x) * hi32(x) */
LANEHASH_INLINE uint64_t lanehash_step(uint64_t a, uint64_t w) {
    uint64_t x = a ^ w;
    return x + (uint64_t)(uint32_t)x * (x >> 32);
}
LANEHASH_INLINE uint64_t lanehash_ks(uint64_t seed) {
    return lanehash_fold(seed ^ LANEHASH_SEED0, LANEHASH_SEED1);
}

/* ------------------------------------------------------------------------- */
/* short path, len <= 64: returns the low half, writes the high half when `hi` */

LANEHASH_INLINE uint64_t lanehash_finish_short(uint64_t x, uint64_t y, uint64_t ln, uint64_t *hi) {
    if (hi)
        *hi = lanehash_fold(x ^ LANEHASH_KF[2], y ^ (LANEHASH_KF[3] ^ ln));
    return lanehash_fold(x ^ (LANEHASH_KF[0] ^ ln), y ^ LANEHASH_KF[1]);
}
LANEHASH_INLINE uint64_t lanehash_short_le16(const uint8_t *p, size_t len, uint64_t seed, uint64_t *hi) {
    uint64_t ln = (uint64_t)len, w0, w1 = 0;
    if (len == 0)
        w0 = 0;
    else if (len <= 3)
        w0 = (uint64_t)p[0] | ((uint64_t)p[len / 2] << 8) | ((uint64_t)p[len - 1] << 16);
    else if (len <= 8)
        w0 = lanehash_le32(p) | (lanehash_le32(p + len - 4) << 32);
    else {
        w0 = lanehash_le64(p);
        w1 = lanehash_le64(p + len - 8);
    }
    w0 ^= LANEHASH_KS[0];
    w1 ^= LANEHASH_KS[1] ^ ln ^ lanehash_ks(seed);
    lanehash_mum(&w0, &w1);
    return lanehash_finish_short(w0, w1, ln, hi);
}
LANEHASH_INLINE uint64_t lanehash_short_le32(const uint8_t *p, size_t len, uint64_t seed, uint64_t *hi) {
    uint64_t ln = (uint64_t)len;
    uint64_t a0 = lanehash_le64(p) ^ LANEHASH_KS[0], b0 = lanehash_le64(p + 8) ^ LANEHASH_KS[1];
    uint64_t a1 = lanehash_le64(p + len - 16) ^ LANEHASH_KS[2], b1 = lanehash_le64(p + len - 8) ^ (LANEHASH_KS[3] ^ ln ^ lanehash_ks(seed));
    lanehash_mum(&a0, &b0);
    lanehash_mum(&a1, &b1);
    return lanehash_finish_short(a0 ^ b1, b0 ^ a1, ln, hi);
}
LANEHASH_INLINE uint64_t lanehash_short_le64(const uint8_t *p, size_t len, uint64_t seed, uint64_t *hi) {
    uint64_t ln = (uint64_t)len;
    uint64_t a0 = lanehash_le64(p) ^ LANEHASH_KS[0], b0 = lanehash_le64(p + 8) ^ LANEHASH_KS[1];
    uint64_t a1 = lanehash_le64(p + 16) ^ LANEHASH_KS[2], b1 = lanehash_le64(p + 24) ^ LANEHASH_KS[3];
    uint64_t a2 = lanehash_le64(p + len - 32) ^ LANEHASH_KS[4], b2 = lanehash_le64(p + len - 24) ^ LANEHASH_KS[5];
    uint64_t a3 = lanehash_le64(p + len - 16) ^ LANEHASH_KS[6], b3 = lanehash_le64(p + len - 8) ^ (LANEHASH_KS[7] ^ ln ^ lanehash_ks(seed));
    lanehash_mum(&a0, &b0);
    lanehash_mum(&a1, &b1);
    lanehash_mum(&a2, &b2);
    lanehash_mum(&a3, &b3);
    return lanehash_finish_short(a0 ^ b1 ^ a2 ^ b3, b0 ^ a1 ^ b2 ^ a3, ln, hi);
}

/* ------------------------------------------------------------------------- */
/* long path, len >= 65: the merged lanes, then the folds                     */

LANEHASH_INLINE uint64_t lanehash_finish_lanes(const uint64_t m[8], uint64_t ks, uint64_t n, uint64_t *hi) {
    uint64_t h = lanehash_fold(m[0] ^ LANEHASH_S[0], m[1] ^ LANEHASH_S[1]) ^ lanehash_fold(m[2] ^ LANEHASH_S[2], m[3] ^ LANEHASH_S[3]) ^
                 lanehash_fold(m[4] ^ LANEHASH_S[4], m[5] ^ LANEHASH_S[5]) ^ lanehash_fold(m[6] ^ LANEHASH_S[6], m[7] ^ LANEHASH_S[7]);
    if (hi) {
        uint64_t h2 = lanehash_fold(m[0] ^ LANEHASH_S[10], m[2] ^ LANEHASH_S[11]) ^ lanehash_fold(m[1] ^ LANEHASH_S[12], m[3] ^ LANEHASH_S[13]) ^
                      lanehash_fold(m[4] ^ LANEHASH_S[14], m[6] ^ LANEHASH_S[15]) ^ lanehash_fold(m[5] ^ LANEHASH_S[16], m[7] ^ LANEHASH_S[17]);
        *hi = lanehash_fold(h2 ^ n, ks ^ LANEHASH_S[18]);
    }
    return lanehash_fold(h ^ n, ks ^ LANEHASH_S[8]);
}

#if !defined(LANEHASH_SSE2) && !defined(LANEHASH_AVX2) && !defined(LANEHASH_AVX512)
/* scalar: no state array up to eight stripes, a 64-word state beyond */
static uint64_t lanehash_long(const uint8_t *p, size_t n, uint64_t seed, uint64_t *hi) {
    uint64_t ks = lanehash_ks(seed), m = (uint64_t)(n - 1) / 64, mv[8], nn = (uint64_t)n;
    unsigned i;
    memset(mv, 0, sizeof mv);
    if (m < 8) {
        uint64_t s;
        for (s = 0; s < m; s++)
            for (i = 0; i < 8; i++)
                mv[i] ^= lanehash_step(lanehash_step(LANEHASH_CT[8 * s + i] + ks, lanehash_le64(p + s * 64 + 8 * i)), LANEHASH_Q[i]);
        for (i = 0; i < 8; i++)
            mv[i] ^= lanehash_step(lanehash_step(LANEHASH_CT[8 * m + i] + ks, lanehash_le64(p + n - 64 + 8 * i) ^ nn), LANEHASH_Q[i]);
        return lanehash_finish_lanes(mv, ks, nn, hi);
    }
    {
        uint64_t st[64], s, c;
        for (c = 0; c < 8; c++)
            for (i = 0; i < 8; i++)
                st[8 * c + i] = lanehash_step(LANEHASH_CT[8 * c + i] + ks, lanehash_le64(p + c * 64 + 8 * i));
        for (s = 8; s < m; s++) {
            uint64_t *ch = st + 8 * (s & 7);
            const uint8_t *q = p + s * 64;
            for (i = 0; i < 8; i++)
                ch[i] = lanehash_step(ch[i], lanehash_le64(q + 8 * i));
        }
        {
            uint64_t *ch = st + 8 * (m & 7);
            for (i = 0; i < 8; i++)
                ch[i] = lanehash_step(ch[i], lanehash_le64(p + n - 64 + 8 * i) ^ nn);
        }
        for (c = 0; c < 8; c++)
            for (i = 0; i < 8; i++)
                mv[i] ^= lanehash_step(st[8 * c + i], LANEHASH_Q[i]);
        return lanehash_finish_lanes(mv, ks, nn, hi);
    }
}
#endif

#if defined(LANEHASH_SSE2) || defined(LANEHASH_AVX2) || defined(LANEHASH_AVX512)
/* one register type per width; R registers per 64-byte stripe */
#  if defined(LANEHASH_AVX512)
typedef __m512i lanehash_v;
#    define LANEHASH_R 1
#    define lanehash_loadu(p) _mm512_loadu_si512((const void *)(p))
#    define lanehash_xor _mm512_xor_si512
#    define lanehash_add _mm512_add_epi64
#    define lanehash_mul _mm512_mul_epu32
#    define lanehash_srl32(x) _mm512_srli_epi64((x), 32)
#    define lanehash_set1(x) _mm512_set1_epi64((long long)(x))
#    define lanehash_zero() _mm512_setzero_si512()
#    define lanehash_storeu(p, v) _mm512_storeu_si512((void *)(p), (v))
#  elif defined(LANEHASH_AVX2)
typedef __m256i lanehash_v;
#    define LANEHASH_R 2
#    define lanehash_loadu(p) _mm256_loadu_si256((const __m256i *)(p))
#    define lanehash_xor _mm256_xor_si256
#    define lanehash_add _mm256_add_epi64
#    define lanehash_mul _mm256_mul_epu32
#    define lanehash_srl32(x) _mm256_srli_epi64((x), 32)
#    define lanehash_set1(x) _mm256_set1_epi64x((long long)(x))
#    define lanehash_zero() _mm256_setzero_si256()
#    define lanehash_storeu(p, v) _mm256_storeu_si256((__m256i *)(p), (v))
#  else
typedef __m128i lanehash_v;
#    define LANEHASH_R 4
#    define lanehash_loadu(p) _mm_loadu_si128((const __m128i *)(p))
#    define lanehash_xor _mm_xor_si128
#    define lanehash_add _mm_add_epi64
#    define lanehash_mul _mm_mul_epu32
#    define lanehash_srl32(x) _mm_srli_epi64((x), 32)
#    define lanehash_set1(x) _mm_set1_epi64x((long long)(x))
#    define lanehash_zero() _mm_setzero_si128()
#    define lanehash_storeu(p, v) _mm_storeu_si128((__m128i *)(p), (v))
#  endif
#  define LANEHASH_W (64 / LANEHASH_R)        /* bytes per register */
#  define LANEHASH_WPR (LANEHASH_W / 8)       /* words per register */

LANEHASH_INLINE lanehash_v lanehash_vstep(lanehash_v a, lanehash_v w) {
    lanehash_v x = lanehash_xor(a, w);
    return lanehash_add(x, lanehash_mul(x, lanehash_srl32(x)));
}
/* merged lanes to memory in 128-bit pieces: Zen 4 does not forward a wider store to the
 * 64-bit loads of the folds */
LANEHASH_INLINE void lanehash_store_words(uint64_t *out, lanehash_v v) {
#  if defined(LANEHASH_AVX512)
    _mm_storeu_si128((__m128i *)out, _mm512_castsi512_si128(v));
    _mm_storeu_si128((__m128i *)(out + 2), _mm512_extracti32x4_epi32(v, 1));
    _mm_storeu_si128((__m128i *)(out + 4), _mm512_extracti32x4_epi32(v, 2));
    _mm_storeu_si128((__m128i *)(out + 6), _mm512_extracti32x4_epi32(v, 3));
#  elif defined(LANEHASH_AVX2)
    _mm_storeu_si128((__m128i *)out, _mm256_castsi256_si128(v));
    _mm_storeu_si128((__m128i *)(out + 2), _mm256_extracti128_si256(v, 1));
#  else
    _mm_storeu_si128((__m128i *)out, v);
#  endif
}

/* m < 8: every chain sees one stripe, stripe step and merge step back to back in
 * registers; AVX-512 uses two ymm per stripe (more parallelism than one zmm) */
#  if defined(LANEHASH_AVX512)
LANEHASH_INLINE __m256i LANEHASH_STEP256(__m256i a, __m256i w) {
    __m256i x = _mm256_xor_si256(a, w);
    return _mm256_add_epi64(x, _mm256_mul_epu32(x, _mm256_srli_epi64(x, 32)));
}
LANEHASH_INLINE void lanehash_small(const uint8_t *p, size_t n, uint64_t m, uint64_t ks, uint64_t mv[8]) {
    __m256i ksv = _mm256_set1_epi64x((long long)ks), nv = _mm256_set1_epi64x((long long)n);
    __m256i q0 = _mm256_loadu_si256((const __m256i *)LANEHASH_Q), q1 = _mm256_loadu_si256((const __m256i *)(LANEHASH_Q + 4));
    __m256i m0 = _mm256_setzero_si256(), m1 = _mm256_setzero_si256();
    uint64_t s;
    for (s = 0; s < m; s++) {
        __m256i a0 = _mm256_add_epi64(_mm256_loadu_si256((const __m256i *)(LANEHASH_CT + 8 * s)), ksv);
        __m256i a1 = _mm256_add_epi64(_mm256_loadu_si256((const __m256i *)(LANEHASH_CT + 8 * s + 4)), ksv);
        m0 = _mm256_xor_si256(m0, LANEHASH_STEP256(LANEHASH_STEP256(a0, _mm256_loadu_si256((const __m256i *)(p + s * 64))), q0));
        m1 = _mm256_xor_si256(m1, LANEHASH_STEP256(LANEHASH_STEP256(a1, _mm256_loadu_si256((const __m256i *)(p + s * 64 + 32))), q1));
    }
    {
        __m256i a0 = _mm256_add_epi64(_mm256_loadu_si256((const __m256i *)(LANEHASH_CT + 8 * m)), ksv);
        __m256i a1 = _mm256_add_epi64(_mm256_loadu_si256((const __m256i *)(LANEHASH_CT + 8 * m + 4)), ksv);
        m0 = _mm256_xor_si256(m0, LANEHASH_STEP256(LANEHASH_STEP256(a0, _mm256_xor_si256(_mm256_loadu_si256((const __m256i *)(p + n - 64)), nv)), q0));
        m1 = _mm256_xor_si256(m1, LANEHASH_STEP256(LANEHASH_STEP256(a1, _mm256_xor_si256(_mm256_loadu_si256((const __m256i *)(p + n - 32)), nv)), q1));
    }
    _mm_storeu_si128((__m128i *)mv, _mm256_castsi256_si128(m0));
    _mm_storeu_si128((__m128i *)(mv + 2), _mm256_extracti128_si256(m0, 1));
    _mm_storeu_si128((__m128i *)(mv + 4), _mm256_castsi256_si128(m1));
    _mm_storeu_si128((__m128i *)(mv + 6), _mm256_extracti128_si256(m1, 1));
}
#  else
LANEHASH_INLINE void lanehash_small(const uint8_t *p, size_t n, uint64_t m, uint64_t ks, uint64_t mv[8]) {
    lanehash_v ksv = lanehash_set1(ks), nv = lanehash_set1(n), q[LANEHASH_R], acc[LANEHASH_R];
    uint64_t s;
    unsigned r;
    LANEHASH_UNROLL
    for (r = 0; r < LANEHASH_R; r++) {
        q[r] = lanehash_loadu(LANEHASH_Q + r * LANEHASH_WPR);
        acc[r] = lanehash_zero();
    }
    for (s = 0; s < m; s++) {
        LANEHASH_UNROLL
        for (r = 0; r < LANEHASH_R; r++) {
            lanehash_v a = lanehash_add(lanehash_loadu(LANEHASH_CT + 8 * s + r * LANEHASH_WPR), ksv);
            acc[r] = lanehash_xor(acc[r], lanehash_vstep(lanehash_vstep(a, lanehash_loadu(p + s * 64 + r * LANEHASH_W)), q[r]));
        }
    }
    LANEHASH_UNROLL
    for (r = 0; r < LANEHASH_R; r++) {
        lanehash_v a = lanehash_add(lanehash_loadu(LANEHASH_CT + 8 * m + r * LANEHASH_WPR), ksv);
        acc[r] = lanehash_xor(acc[r], lanehash_vstep(lanehash_vstep(a, lanehash_xor(lanehash_loadu(p + n - 64 + r * LANEHASH_W), nv)), q[r]));
    }
    LANEHASH_UNROLL
    for (r = 0; r < LANEHASH_R; r++)
        lanehash_store_words(mv + r * LANEHASH_WPR, acc[r]);
}
#  endif

/* m >= 8: 64-word state in memory (SSE2) or in registers (AVX2 16 ymm, AVX-512 8 zmm) */
static uint64_t lanehash_long(const uint8_t *p, size_t n, uint64_t seed, uint64_t *hi) {
    uint64_t ks = lanehash_ks(seed), m = (uint64_t)(n - 1) / 64, nn = (uint64_t)n;
    uint64_t mv[8];
    if (m < 8) {
        lanehash_small(p, n, m, ks, mv);
        return lanehash_finish_lanes(mv, ks, nn, hi);
    }
    {
        lanehash_v ksv = lanehash_set1(ks), nv = lanehash_set1(nn), q[LANEHASH_R], acc[LANEHASH_R];
        unsigned r;
        uint64_t s;
#  if LANEHASH_R == 4
        unsigned c;
        /* SSE2: the 64-word state in memory (32 xmm registers) */
        union { uint64_t w[64]; lanehash_v v[32]; } st;
        for (c = 0; c < 8; c++) {
            LANEHASH_UNROLL
            for (r = 0; r < LANEHASH_R; r++)
                st.v[4 * c + r] = lanehash_vstep(lanehash_add(lanehash_loadu(LANEHASH_CT + 8 * c + r * LANEHASH_WPR), ksv), lanehash_loadu(p + c * 64 + r * LANEHASH_W));
        }
        for (s = 8; s < m; s++) {
            const uint8_t *src = p + s * 64;
            lanehash_v *ch = st.v + 4 * (s & 7);
            LANEHASH_UNROLL
            for (r = 0; r < LANEHASH_R; r++)
                ch[r] = lanehash_vstep(ch[r], lanehash_loadu(src + r * LANEHASH_W));
        }
        {
            lanehash_v *ch = st.v + 4 * (m & 7);
            LANEHASH_UNROLL
            for (r = 0; r < LANEHASH_R; r++)
                ch[r] = lanehash_vstep(ch[r], lanehash_xor(lanehash_loadu(p + n - 64 + r * LANEHASH_W), nv));
        }
        LANEHASH_UNROLL
        for (r = 0; r < LANEHASH_R; r++) {
            q[r] = lanehash_loadu(LANEHASH_Q + r * LANEHASH_WPR);
            acc[r] = lanehash_zero();
        }
        for (c = 0; c < 8; c++) {
            LANEHASH_UNROLL
            for (r = 0; r < LANEHASH_R; r++)
                acc[r] = lanehash_xor(acc[r], lanehash_vstep(st.v[4 * c + r], q[r]));
        }
#  else
        /* eight named chains (an array of chains stays on the stack with gcc: 61 spills
         * in the loop, 25 % at 1-4 KiB) */
        lanehash_v a0[LANEHASH_R], a1[LANEHASH_R], a2[LANEHASH_R], a3[LANEHASH_R], a4[LANEHASH_R], a5[LANEHASH_R], a6[LANEHASH_R], a7[LANEHASH_R];
#    define LANEHASH_FOR8(X) X(0) X(1) X(2) X(3) X(4) X(5) X(6) X(7)
#    define LANEHASH_INIT(c) LANEHASH_UNROLL for (r = 0; r < LANEHASH_R; r++) a##c[r] = lanehash_add(lanehash_loadu(LANEHASH_CT + 8 * c + r * LANEHASH_WPR), ksv);
        LANEHASH_FOR8(LANEHASH_INIT)
        for (s = 0; s + 8 <= m; s += 8) {
            const uint8_t *src = p + s * 64;
#    define LANEHASH_ABSORB(c) LANEHASH_UNROLL for (r = 0; r < LANEHASH_R; r++) a##c[r] = lanehash_vstep(a##c[r], lanehash_loadu(src + c * 64 + r * LANEHASH_W));
            LANEHASH_FOR8(LANEHASH_ABSORB)
        }
        {
            uint64_t rem = m - s;
            const uint8_t *src = p + s * 64;
            lanehash_v cl[LANEHASH_R];
            LANEHASH_UNROLL
            for (r = 0; r < LANEHASH_R; r++)
                cl[r] = lanehash_xor(lanehash_loadu(p + n - 64 + r * LANEHASH_W), nv);
#    define LANEHASH_TAIL(c) if (c < rem) { LANEHASH_ABSORB(c) } if (c == (m & 7)) { LANEHASH_UNROLL for (r = 0; r < LANEHASH_R; r++) a##c[r] = lanehash_vstep(a##c[r], cl[r]); }
            LANEHASH_FOR8(LANEHASH_TAIL)
        }
        LANEHASH_UNROLL
        for (r = 0; r < LANEHASH_R; r++) {
            q[r] = lanehash_loadu(LANEHASH_Q + r * LANEHASH_WPR);
            acc[r] = lanehash_zero();
        }
#    define LANEHASH_MERGE(c) LANEHASH_UNROLL for (r = 0; r < LANEHASH_R; r++) acc[r] = lanehash_xor(acc[r], lanehash_vstep(a##c[r], q[r]));
        LANEHASH_FOR8(LANEHASH_MERGE)
#    undef LANEHASH_FOR8
#    undef LANEHASH_INIT
#    undef LANEHASH_ABSORB
#    undef LANEHASH_TAIL
#    undef LANEHASH_MERGE
#  endif
        LANEHASH_UNROLL
        for (r = 0; r < LANEHASH_R; r++)
            lanehash_store_words(mv + r * LANEHASH_WPR, acc[r]);
        return lanehash_finish_lanes(mv, ks, nn, hi);
    }
}
#endif /* SIMD */

/* ------------------------------------------------------------------------- */
/* public API: one out-of-line function per case                              */

static LANEHASH_NOINLINE uint64_t lanehash_64_le16(const uint8_t *p, size_t len, uint64_t seed) { return lanehash_short_le16(p, len, seed, NULL); }
static LANEHASH_NOINLINE uint64_t lanehash_64_le32(const uint8_t *p, size_t len, uint64_t seed) { return lanehash_short_le32(p, len, seed, NULL); }
static LANEHASH_NOINLINE uint64_t lanehash_64_le64(const uint8_t *p, size_t len, uint64_t seed) { return lanehash_short_le64(p, len, seed, NULL); }
static LANEHASH_NOINLINE uint64_t lanehash_64_long(const uint8_t *p, size_t len, uint64_t seed) { return lanehash_long(p, len, seed, NULL); }

uint64_t lanehash64(const void *key, size_t len, uint64_t seed) {
    const uint8_t *p = (const uint8_t *)key;
    if (len > 64)
        return lanehash_64_long(p, len, seed);
    if (len <= 16)
        return lanehash_64_le16(p, len, seed);
    if (len <= 32)
        return lanehash_64_le32(p, len, seed);
    return lanehash_64_le64(p, len, seed);
}

static LANEHASH_NOINLINE void lanehash_128_short(const uint8_t *p, size_t len, uint64_t seed, uint8_t *o) {
    uint64_t hi, lo;
    if (len <= 16)
        lo = lanehash_short_le16(p, len, seed, &hi);
    else if (len <= 32)
        lo = lanehash_short_le32(p, len, seed, &hi);
    else
        lo = lanehash_short_le64(p, len, seed, &hi);
    lanehash_put64(o, lo);
    lanehash_put64(o + 8, hi);
}
static LANEHASH_NOINLINE void lanehash_128_long(const uint8_t *p, size_t len, uint64_t seed, uint8_t *o) {
    uint64_t hi, lo = lanehash_long(p, len, seed, &hi);
    lanehash_put64(o, lo);
    lanehash_put64(o + 8, hi);
}

void lanehash128(const void *key, size_t len, uint64_t seed, void *out) {
    const uint8_t *p = (const uint8_t *)key;
    if (len <= 64)
        lanehash_128_short(p, len, seed, (uint8_t *)out);
    else
        lanehash_128_long(p, len, seed, (uint8_t *)out);
}
