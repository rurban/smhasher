/*
 * hassh64 2.0.0: a fast, seeded, non-cryptographic 64-bit hash.
 * https://github.com/zsigisti/hassh64
 *
 * AI-generated: designed and written by an AI (Anthropic's Claude, via Claude
 * Code) under human direction, and reviewed only by AI agents. MIT licensed.
 *
 *     #include "hassh.h"
 *     uint64_t h = hassh64(data, len, seed);
 *
 * Single header, C99 or C++, no dependencies. Works on any CPU and compiler:
 *   x86-64 GCC/Clang  AES-NI path. Built with -maes -msse4.1 (or -march=native)
 *                     it is used directly; built for generic x86-64 it is
 *                     picked at run time if the CPU has AES-NI (define
 *                     HASSH_NO_DISPATCH to turn that off). -mvaes -mavx2 adds
 *                     a 256-bit VAES path.
 *   x86-64 MSVC       AES-NI path with /arch:AVX2 (every AVX2 CPU has
 *                     AES-NI), or define HASSH_ASSUME_AESNI; otherwise
 *                     portable C. (clang-cl behaves like Clang: run-time
 *                     check, or -maes -msse4.1.)
 *   AArch64           ARMv8 AES path when built with the crypto extension
 *                     (-march=armv8-a+crypto; the default on Apple targets).
 *   anything else     portable C (32-bit, big-endian, RISC-V, WebAssembly...).
 * Every path gives bit-identical output; SPEC.md defines the algorithm and
 * test/vectors.h freezes it. Define HASSH_PORTABLE to force portable C.
 *
 * Design summary
 *   len  0..32   scalar: 64x64->128 multiplies folded to 64 bits (wyhash /
 *                rapidhash family), with a load pattern that keeps freshly
 *                written keys on the store-forwarding fast path.
 *   len 33..64   scalar: four independent multiplies, then one.
 *   len 65..     AES: 8 lanes (65..128) or 16 lanes of 128 bits. Every 16-byte
 *                block d entering lane i does  acc_i = P(acc_i ^ d),  P = 3
 *                keyed AES rounds, so a DATA difference passes 3 rounds
 *                (>=9 active S-boxes) before it can meet another block's
 *                difference. Lane starts are two AES rounds of K_i ^ sv, sv
 *                from the seed alone. The lanes are XORed together with a
 *                seed-and-length vector lv, then 3 more rounds remove the
 *                XOR's statistical linearity (a public bijection: no added
 *                collision resistance). Inputs <=256 bytes read the head and
 *                the tail with overlapping loads; longer ones use 256-byte
 *                stripes, the last aligned to the end.
 *   len 4096..   AES: two LeMac states (ToSC 2024, corrected 2025): 2 AES
 *                rounds per 16 bytes instead of 3, and a published bound of
 *                >=26 active S-boxes for any trail to a state collision.
 *                128-byte steps, the last aligned to the end; then 4 blank
 *                steps, 4 rounds per state word, XOR with lv, 3 rounds.
 *
 * Seed handling. Every first-level multiply has both operands keyed by the
 * seed, with keys nonlinear in each other (s, and s2 = f(s, len)); later
 * multiplies take secret intermediates XOR constants. The AES lane starts
 * depend on the seed only (so hassh64_keyed can precompute them), and the
 * length enters through lv, two AES rounds over a vector jointly nonlinear in
 * seed and length. test/seedindep.c and test/xlen.c build the seed-independent
 * attacks this rules out. Passing them rules out only those attacks: hassh64
 * has had AI review and no human cryptanalysis, so it makes NO claim of
 * HashDoS resistance.
 *
 * Known margin (2.x, 65..4095 bytes): with 3 rounds per block, the best
 * 3-round AES differential found (about 2^-51.2) gives fixed input pairs that
 * collide for about 2^-51 of seeds, against the ideal 2^-64. 1.x used 4
 * rounds (>=25 active S-boxes) and had no such pairs; 2.x trades that margin
 * for speed. From 4096 bytes the LeMac absorber has no such known pairs.
 *
 * NOT a cryptographic hash and NOT a MAC. Do not use it for passwords,
 * signatures, or anything an attacker must not be able to forge.
 */
#ifndef HASSH_H
#define HASSH_H

#define HASSH_VERSION_MAJOR 2   /* bumped whenever any output changes */
#define HASSH_VERSION_MINOR 0
#define HASSH_VERSION_PATCH 0
#define HASSH_VERSION "2.0.0"

#include <stdint.h>
#include <stddef.h>
#include <string.h>

/* ISA selection. HASSH_HAVE_SIMD: an AES path is compiled. HASSH_DISPATCH:
 * it is compiled for a target the build does not assume (x86-64 GCC/Clang
 * without -maes), and hassh64 checks the CPU at run time. The portable
 * reference is compiled when there is no SIMD path, when dispatching (as the
 * fallback), or on request with HASSH_WANT_PORTABLE (tests). */
#if (defined(__x86_64__) || defined(_M_X64)) && !defined(_M_ARM64EC)
#  define HASSH_X64 1
#endif
#if !defined(HASSH_PORTABLE)
#  if defined(HASSH_X64) && ((defined(__AES__) && defined(__SSE4_1__)) || \
        (defined(_MSC_VER) && !defined(__clang__) && \
         (defined(HASSH_ASSUME_AESNI) || defined(__AVX2__))))
#    define HASSH_HAVE_SIMD 1
#    include <immintrin.h>
#    if defined(_MSC_VER) && !defined(__clang__)
#      include <wmmintrin.h>
#    endif
#  elif defined(HASSH_X64) && !defined(HASSH_NO_DISPATCH) && \
        (defined(__clang__) || (defined(__GNUC__) && __GNUC__ >= 5))
#    define HASSH_HAVE_SIMD 1
#    define HASSH_DISPATCH 1
#    include <immintrin.h>
#    include <cpuid.h>
#  elif defined(__aarch64__) && !defined(__AARCH64EB__) && \
        defined(__ARM_FEATURE_AES) && defined(__ARM_NEON)
#    define HASSH_HAVE_SIMD 1
#    include <arm_neon.h>
#  endif
#endif
#if defined(_MSC_VER) && !defined(__clang__) && (defined(_M_X64) || defined(_M_ARM64))
#  include <intrin.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

#if defined(__GNUC__) || defined(__clang__)
#  define HASSH_INLINE static inline __attribute__((always_inline))
#  define HASSH_LIKELY(x) __builtin_expect(!!(x), 1)
#elif defined(_MSC_VER)
#  define HASSH_INLINE static __forceinline
#  define HASSH_LIKELY(x) (x)
#else
#  define HASSH_INLINE static inline
#  define HASSH_LIKELY(x) (x)
#endif
/* Fixed-count loops over vector registers must be fully unrolled, or the
 * accumulators live in a stack array (GCC -O2 does not unroll them on its own:
 * measured 15 instead of 23 bytes/cycle). */
#if defined(__clang__)
#  define HASSH_UNROLL _Pragma("unroll")
#elif defined(__GNUC__) && __GNUC__ >= 8
#  define HASSH_UNROLL _Pragma("GCC unroll 16")
#else
#  define HASSH_UNROLL
#endif
/* SIMD functions. When dispatching they carry a target attribute, so they
 * cannot be force-inlined into hassh64 (which has none); the two entry points
 * are then ordinary static functions. */
#ifdef HASSH_DISPATCH
#  define HASSH_TGT __attribute__((target("aes,sse4.1")))
#  define HASSH_SIMD_INLINE HASSH_INLINE HASSH_TGT
#  define HASSH_SIMD_ENTRY static inline HASSH_TGT
#else
#  define HASSH_TGT
#  define HASSH_SIMD_INLINE HASSH_INLINE
#  define HASSH_SIMD_ENTRY HASSH_INLINE
#endif
#define HASSH_SIMD_BULK static inline HASSH_TGT

/* Nothing-up-my-sleeve constants: fractional parts of sqrt of the first
 * primes (the SHA-512 IV) and of cube roots (the first SHA-512 round keys). */
static const uint64_t HASSH_P[8] = {
    0x6a09e667f3bcc908ull, 0xbb67ae8584caa73bull, 0x3c6ef372fe94f82bull, 0xa54ff53a5f1d36f1ull,
    0x510e527fade682d1ull, 0x9b05688c2b3e6c1full, 0x1f83d9abfb41bd6bull, 0x5be0cd19137e2179ull,
};
/* Round counts are part of the algorithm. Overriding them changes every
 * output for 65+ bytes without changing HASSH_VERSION, so it is refused unless
 * HASSH_EXPERIMENTAL is defined (research builds only). */
#if (defined(HASSH_ROUNDS) || defined(HASSH_FIN_ROUNDS)) && !defined(HASSH_EXPERIMENTAL)
#  error "HASSH_ROUNDS / HASSH_FIN_ROUNDS change hassh64's output; define HASSH_EXPERIMENTAL to override them"
#endif
#ifndef HASSH_ROUNDS
#  define HASSH_ROUNDS 3      /* AES rounds per absorbed block, keys 16.. (max 8) */
#endif
#ifndef HASSH_FIN_ROUNDS
#  define HASSH_FIN_ROUNDS 3  /* rounds after the lane XOR, keys 24.. (max 6) */
#endif
/* Inputs of at least HASSH_LONG_MIN bytes use the LeMac absorber instead of
 * the lanes (part of the algorithm, like the round counts). */
#if defined(HASSH_LONG_MIN) && !defined(HASSH_EXPERIMENTAL)
#  error "HASSH_LONG_MIN changes hassh64's output; define HASSH_EXPERIMENTAL to override it"
#endif
#ifndef HASSH_LONG_MIN
#  define HASSH_LONG_MIN 4096
#endif
#if HASSH_LONG_MIN < 257
#  error "HASSH_LONG_MIN must be at least 257"
#endif
#define HASSH_INIT_KEY0 30    /* lane-start round keys (two rounds) */
#define HASSH_INIT_KEY1 31
#if HASSH_ROUNDS < 1 || HASSH_ROUNDS > 8
#  error "HASSH_ROUNDS must be 1..8 (round keys 16..23)"
#endif
#if HASSH_FIN_ROUNDS < 1 || HASSH_FIN_ROUNDS > 6
#  error "HASSH_FIN_ROUNDS must be 1..6 (round keys 24..29)"
#endif
/* 128-bit keys as little-endian 64-bit halves (lo, hi): 0..15 lane start
 * values, 16..23 block round keys, 24..29 finalizer keys, 30..31 lane-start keys. The first 64 of the 80 SHA-512 round constants. */
static const uint64_t HASSH_K[] = {
    0x428a2f98d728ae22ull, 0x7137449123ef65cdull, 0xb5c0fbcfec4d3b2full, 0xe9b5dba58189dbbcull,
    0x3956c25bf348b538ull, 0x59f111f1b605d019ull, 0x923f82a4af194f9bull, 0xab1c5ed5da6d8118ull,
    0xd807aa98a3030242ull, 0x12835b0145706fbeull, 0x243185be4ee4b28cull, 0x550c7dc3d5ffb4e2ull,
    0x72be5d74f27b896full, 0x80deb1fe3b1696b1ull, 0x9bdc06a725c71235ull, 0xc19bf174cf692694ull,
    0xe49b69c19ef14ad2ull, 0xefbe4786384f25e3ull, 0x0fc19dc68b8cd5b5ull, 0x240ca1cc77ac9c65ull,
    0x2de92c6f592b0275ull, 0x4a7484aa6ea6e483ull, 0x5cb0a9dcbd41fbd4ull, 0x76f988da831153b5ull,
    0x983e5152ee66dfabull, 0xa831c66d2db43210ull, 0xb00327c898fb213full, 0xbf597fc7beef0ee4ull,
    0xc6e00bf33da88fc2ull, 0xd5a79147930aa725ull, 0x06ca6351e003826full, 0x142929670a0e6e70ull,
    0x27b70a8546d22ffcull, 0x2e1b21385c26c926ull, 0x4d2c6dfc5ac42aedull, 0x53380d139d95b3dfull,
    0x650a73548baf63deull, 0x766a0abb3c77b2a8ull, 0x81c2c92e47edaee6ull, 0x92722c851482353bull,
    0xa2bfe8a14cf10364ull, 0xa81a664bbc423001ull, 0xc24b8b70d0f89791ull, 0xc76c51a30654be30ull,
    0xd192e819d6ef5218ull, 0xd69906245565a910ull, 0xf40e35855771202aull, 0x106aa07032bbd1b8ull,
    0x19a4c116b8d2d0c8ull, 0x1e376c085141ab53ull, 0x2748774cdf8eeb99ull, 0x34b0bcb5e19b48a8ull,
    0x391c0cb3c5c95a63ull, 0x4ed8aa4ae3418acbull, 0x5b9cca4f7763e373ull, 0x682e6ff3d6b2b8a3ull,
    0x748f82ee5defb2fcull, 0x78a5636f43172f60ull, 0x84c87814a1f0ab72ull, 0x8cc702081a6439ecull,
    0x90befffa23631e28ull, 0xa4506cebde82bde9ull, 0xbef9a3f7b2c67915ull, 0xc67178f2e372532bull,
};
/* Fails to compile if the table above is not exactly 64 entries. */
typedef char hassh_k_size_check[(sizeof HASSH_K / sizeof HASSH_K[0] == 64) ? 1 : -1];
/* Long-input keys (36 x 128 bits, as lo/hi halves): fractional parts of the
 * cube roots of the 81st to 152nd primes (the sequence SHA-512's K continues
 * with). Keys 0..17 start the 18 LeMac state words, 18..35 key their
 * compression. */
static const uint64_t HASSH_L[] = {
    0x7ba0ea2d98160007ull, 0x7eabf2d0c21f964aull, 0x8dbe8d038b409545ull, 0x90bb1721582e8285ull,
    0x99a2ad45936d4e61ull, 0x9f86e289fe03e739ull, 0xa84c4472faa9a82full, 0xb3df34fce89e0532ull,
    0xb99bb8d7b173534full, 0xbc76cbab1aea1f9cull, 0xc226a69a780f3cc3ull, 0xd304f19aa233957dull,
    0xde1be20a212129ddull, 0xe39bb43755141950ull, 0xee84927cea48ddd2ull, 0xf3edd2773c523b67ull,
    0xfbfdfe53a8d32f2aull, 0x0bee2c7ab77e9e25ull, 0x0e90181cf1b09e56ull, 0x25f57204c725bed8ull,
    0x2da45582cd598b32ull, 0x3a52c34c203bfcf3ull, 0x41dc0172cd1991c1ull, 0x495796fcb33cc1c0ull,
    0x4bd31fc693f9f16eull, 0x533cde2115f5a9a0ull, 0x5f7abfe36e99c1d3ull, 0x66c206b310a57e6full,
    0x6dfcc6bc39603f61ull, 0x7062f20f86fd1052ull, 0x778d51277adec865ull, 0x7eaba3cc25da7048ull,
    0x8363eccc37a5be05ull, 0x85be1c253beba54eull, 0x93c04028f348bbc5ull, 0x9f4a205fd05b2148ull,
    0xa19535651ca6d2deull, 0xa627bb0fbf027bc7ull, 0xacfa80891da2f06bull, 0xb3c29b23031a7f9dull,
    0xb602f6fac7d3d74dull, 0xc36cee0a10c7ba49ull, 0xc7dc81eea9ebad4full, 0xce7b8471b0f809dfull,
    0xd740288c84df269cull, 0xe21dba7ac2290607ull, 0xeabbff66be175964ull, 0xf56a9e60f62cea92ull,
    0xfde41d729d126eabull, 0x0434d0970e42e781ull, 0x0a7cb752a3f1cd86ull, 0x0ea7d22d6bcd7382ull,
    0x16f2987f9495a5eeull, 0x1d20cdcd45b8de1eull, 0x213af85a39b0c320ull, 0x2964505c52a2f35bull,
    0x2d738e114181e082ull, 0x3b8cea0e71c58aafull, 0x4584e6ae9f54016eull, 0x515f4356903dccc2ull,
    0x5356112ddfd5a8e9ull, 0x5d1bc3edbe2c897aull, 0x5f0da9f8ed53548bull, 0x62ef0be4d5492e78ull,
    0x64de896eace0be7full, 0x6e801ba3078ae05full, 0x7bdb3595cdadf50aull, 0x7fa5377856834c98ull,
    0x818916bad3d008a8ull, 0x854e959f834021a7ull, 0x926a82c27137e2c6ull, 0x9622c7ba7d179197ull,
};
typedef char hassh_l_size_check[(sizeof HASSH_L / sizeof HASSH_L[0] == 72) ? 1 : -1];

/* ---------------------------------------------------------------- scalar */

HASSH_INLINE uint64_t hassh_r64(const uint8_t *p) {
    uint64_t v; memcpy(&v, p, 8);
#if defined(__BYTE_ORDER__) && __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
    v = __builtin_bswap64(v);
#endif
    return v;
}
HASSH_INLINE uint64_t hassh_r32(const uint8_t *p) {
    uint32_t v; memcpy(&v, p, 4);
#if defined(__BYTE_ORDER__) && __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
    v = __builtin_bswap32(v);
#endif
    return v;
}

/* 64x64 -> 128-bit product. */
HASSH_INLINE void hassh_mul(uint64_t a, uint64_t b, uint64_t *lo, uint64_t *hi) {
#if defined(__SIZEOF_INT128__)
    __uint128_t r = (__uint128_t)a * b;
    *lo = (uint64_t)r; *hi = (uint64_t)(r >> 64);
#elif defined(_MSC_VER) && !defined(__clang__) && defined(_M_X64)
    *lo = _umul128(a, b, hi);
#elif defined(_MSC_VER) && !defined(__clang__) && defined(_M_ARM64)
    *lo = a * b; *hi = __umulh(a, b);
#else
    uint64_t ha = a >> 32, hb = b >> 32, la = (uint32_t)a, lb = (uint32_t)b;
    uint64_t rh = ha * hb, rm0 = ha * lb, rm1 = hb * la, rl = la * lb;
    uint64_t t = rl + (rm0 << 32), c = t < rl;
    uint64_t l = t + (rm1 << 32); c += l < t;
    *lo = l; *hi = rh + (rm0 >> 32) + (rm1 >> 32) + c;
#endif
}
/* Product folded to 64 bits. */
HASSH_INLINE uint64_t hassh_fold(uint64_t a, uint64_t b) {
    uint64_t lo, hi; hassh_mul(a, b, &lo, &hi); return lo ^ hi;
}

/* Exact 4-byte load of bytes 0..3. The empty asm stops the compiler from
 * merging it with the adjacent load at p+4 into one 8-byte load, which would
 * defeat store forwarding (see hassh_small). */
HASSH_INLINE uint64_t hassh_r32_head(const uint8_t *p) {
    uint64_t v = hassh_r32(p);
#if defined(__GNUC__) || defined(__clang__)
    __asm__("" : "+r"(v));
#endif
    return v;
}

/* 1..3 bytes at q (n must be 1..3): every byte is read at least once. */
HASSH_INLINE uint64_t hassh_tail3(const uint8_t *q, size_t n) {
    return ((uint64_t)q[0] << 16) | ((uint64_t)q[n >> 1] << 8) | q[n - 1];
}

/* 0..32 bytes, scalar.
 *
 * Load discipline: bytes 0..3 are only ever read by loads that START at p
 * (4-byte, 2-byte or 1-byte), and no other load overlaps them. A key that was
 * just written with narrow stores (a freshly built struct or integer, the
 * common hash-table case and exactly what SMHasher3's latency test does) then
 * forwards from the store buffer instead of paying a ~12-cycle
 * store-forwarding stall. Loads that begin inside a recent store also forward
 * slowly on current Intel cores, hence the 16-bit head load for 2..3 bytes.
 *
 * Mixing: 64x64->128 multiplies folded to 64 bits (lo ^ hi). The seed
 * pre-mix does not depend on the data, so it overlaps with the loads.
 * 0..16 bytes: two dependent multiplies, the second over both halves of the
 * first product (wyhash's structure). 17..32 bytes: two in parallel, then two
 * dependent. */
/* Seed keys. Both multiply operands are keyed by the seed, and by DIFFERENT
 * keys that are nonlinear in each other (s2 is a multiply of s). With an
 * unkeyed operand, an input can force that operand to 0 (or ~0: fold(~0, b)
 * == ~0 for any b != 0) and the product ignores the rest; with the same key
 * on both sides, commutativity lets an attacker swap the operands. Either
 * gives collisions under every seed (test/seedindep.c).
 *
 * s2 also absorbs the length, so different lengths use unrelated multiply
 * keys from the first multiply on. If length entered only at the end, pairs
 * such as lengths 8/11 or 40/48 could reach equal pre-final state for every
 * seed. Data-independent, so these multiplies overlap with the loads. */
HASSH_INLINE uint64_t hassh_seed_s(uint64_t seed) {
    return seed ^ hassh_fold(seed ^ HASSH_P[0], HASSH_P[1]);
}
HASSH_INLINE uint64_t hassh_seed_s2(uint64_t s, size_t len) {
    return hassh_fold(s ^ (uint64_t)len ^ HASSH_P[6], HASSH_P[7]);
}
#define HASSH_SEEDKEYS(s, len) const uint64_t t = ((s) ^ HASSH_P[5] ^ ((uint64_t)(len) << 56)) | 1

/* 0..32 bytes with the seed keys already derived: s = hassh_seed_s(seed),
 * s2 = hassh_seed_s2(s, len). */
HASSH_INLINE uint64_t hassh_small_k(const uint8_t *p, size_t len, uint64_t s, uint64_t s2) {
    uint64_t a, b;
    HASSH_SEEDKEYS(s, len);
    if (len > 16) {
        uint64_t w0 = hassh_r32_head(p) | (hassh_r32(p + 4) << 32);
        uint64_t w1 = hassh_r64(p + 8);
        uint64_t w2 = hassh_r64(p + (len < 20 ? 4 : len - 16));
        uint64_t w3 = hassh_r64(p + len - 8);
        uint64_t x = hassh_fold(w0 ^ HASSH_P[2] ^ s, w1 ^ HASSH_P[3] ^ s2);
        uint64_t y = hassh_fold(w2 ^ HASSH_P[4] ^ s, w3 ^ HASSH_P[5] ^ s2);
        a = hassh_fold(x ^ HASSH_P[6], y ^ HASSH_P[7] ^ (uint64_t)len);
        return hassh_fold(a ^ HASSH_P[4], t);
    }
    if (len >= 8) {
        a = hassh_r32_head(p) | (hassh_r32(p + 4) << 32);
        if (len >= 12)     b = hassh_r32(p + 8) | (hassh_r32(p + len - 4) << 32);
        else if (len > 8)  b = hassh_tail3(p + 8, len - 8);
        else               b = 0;
    } else if (len >= 4) {
        a = hassh_r32(p);
        b = len > 4 ? hassh_tail3(p + 4, len - 4) : 0;
    } else if (len >= 2) {
        uint16_t h; memcpy(&h, p, 2);
#if defined(__BYTE_ORDER__) && __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
        h = __builtin_bswap16(h);
#endif
        a = h | (len == 3 ? (uint64_t)p[2] << 16 : 0);
        b = 0;
    } else {
        a = len ? p[0] : 0;
        b = 0;
    }
    /* Both product halves feed the second multiply, so it has data on both
     * sides. Folding first would leave short keys with b = 0 (lengths 1..4,
     * 8) multiplied only by seed constants, twice, which SMHasher3's Bitflip
     * test can detect on 4-byte keys. */
    uint64_t lo, hi;
    hassh_mul(a ^ HASSH_P[2] ^ s, b ^ HASSH_P[3] ^ s2 ^ (uint64_t)len, &lo, &hi);
    return hassh_fold(lo ^ HASSH_P[4], hi ^ t);
}
HASSH_INLINE uint64_t hassh_small(const uint8_t *p, size_t len, uint64_t seed) {
    const uint64_t s = hassh_seed_s(seed);
    return hassh_small_k(p, len, s, hassh_seed_s2(s, len));
}

/* 33..64 bytes, scalar: four independent multiplies over eight words, then
 * one. Two dependent multiplies, against 4+3 serial AES rounds for an AES
 * path, which measured ~1.8x slower in a latency chain.
 *
 * x0 and x2 (likewise x1 and x3) are XORed together, so their key pairs
 * must not be equal up to a constant, nor equal after commuting: pair 0/1
 * use (s, s2) and pair 2/3 use (s ^ s2, s). Every XOR difference between
 * the two key pairs, swapped or not, then contains s or s2 and depends on the
 * seed. */
HASSH_INLINE uint64_t hassh_mid_k(const uint8_t *p, size_t len, uint64_t s, uint64_t s2) {
    HASSH_SEEDKEYS(s, len);
    const uint64_t s3 = s ^ s2;
    const uint8_t *e = p + len;
    uint64_t x0 = hassh_fold(hassh_r64(p)      ^ HASSH_P[2] ^ s,  hassh_r64(p + 8)  ^ HASSH_P[3] ^ s2);
    uint64_t x1 = hassh_fold(hassh_r64(p + 16) ^ HASSH_P[4] ^ s,  hassh_r64(p + 24) ^ HASSH_P[5] ^ s2);
    uint64_t x2 = hassh_fold(hassh_r64(e - 32) ^ HASSH_P[6] ^ s3, hassh_r64(e - 24) ^ HASSH_P[7] ^ s);
    uint64_t x3 = hassh_fold(hassh_r64(e - 16) ^ HASSH_P[0] ^ s3, hassh_r64(e - 8)  ^ HASSH_P[1] ^ s);
    return hassh_fold(x0 ^ x2 ^ HASSH_P[4], x1 ^ x3 ^ t);
}
HASSH_INLINE uint64_t hassh_mid(const uint8_t *p, size_t len, uint64_t seed) {
    const uint64_t s = hassh_seed_s(seed);
    return hassh_mid_k(p, len, s, hassh_seed_s2(s, len));
}

/* AES paths (2.x). Two 128-bit vectors carry the seed:
 *
 * seed vector  sv = (s, fold(s ^ P2, P3)): the lane starts are two AES rounds
 *              of K_i ^ sv. They depend on the seed only, so hassh64_keyed
 *              precomputes all 16 of them.
 * length vec.  lv = two AES rounds (keys 30, 31) of (fold(s ^ len ^ P4, P5),
 *              fold(s ^ len ^ P6, P7)), XORed into the combined lanes before
 *              the final rounds. It runs in parallel with the lanes. Each
 *              fold alone leaves the difference between two lengths about
 *              2^-20 predictable per half (review, 2026-11-05); the two
 *              rounds spread it over all 16 bytes. Both halves are jointly
 *              nonlinear in seed and length, so the difference
 *              between two lengths' lv depends on the seed: an attacker who
 *              makes two inputs of different lengths reach the same lane
 *              state still has to cancel lv(L1) ^ lv(L2), which needs the
 *              seed. (A raw (seed, len) pair would not do: SubBytes works
 *              bytewise, so seed and length bytes would never mix; see
 *              test/xlen.c.) */
HASSH_INLINE void hassh_seedvec_s(uint64_t s, uint64_t *lo, uint64_t *hi) {
    *lo = s;
    *hi = hassh_fold(s ^ HASSH_P[2], HASSH_P[3]);
}
HASSH_INLINE void hassh_lenvec_s(uint64_t s, size_t len, uint64_t *lo, uint64_t *hi) {
    *lo = hassh_fold(s ^ (uint64_t)len ^ HASSH_P[4], HASSH_P[5]);
    *hi = hassh_fold(s ^ (uint64_t)len ^ HASSH_P[6], HASSH_P[7]);
}

/* One LeMac step (see hassh_long_portable_s) on the variables V0..V8 (state)
 * and Vq, Vr0, Vr1 (LeMac's RR, R0, R1), with Vr2 = LeMac's R1 ^ R2 (the only
 * way R2 is used). Named variables rather than arrays, so compilers keep the
 * state in registers; the new S0 is computed first so the old S8 dies early. */
#define HASSH_LM_STEP(V, M0, M1, M2, M3, ENC, XOR) do {                      \
        const hassh_lm_t S0_ = XOR(XOR(V##0, V##8), M2);                     \
        V##8 = ENC(V##7, M3); V##7 = ENC(V##6, M1); V##6 = ENC(V##5, M1);     \
        V##5 = ENC(V##4, M0); V##4 = ENC(V##3, M0);                           \
        V##3 = ENC(V##2, V##r2);                                              \
        V##2 = ENC(V##1, M3); V##1 = ENC(V##0, M3); V##0 = S0_;               \
        V##r2 = XOR(V##r0, V##r1); V##r1 = V##r0; V##r0 = XOR(V##q, M1); V##q = M2; \
    } while (0)
#define HASSH_LM_VARS(T, V) T V##0, V##1, V##2, V##3, V##4, V##5, V##6, V##7, V##8, V##q, V##r0, V##r1, V##r2

/* ------------------------------------------------------- portable AES */
#if !defined(HASSH_HAVE_SIMD) || defined(HASSH_DISPATCH) || defined(HASSH_WANT_PORTABLE)

typedef struct { uint8_t b[16]; } hassh_blk;

static const uint8_t HASSH_SBOX[256] = {
    0x63,0x7c,0x77,0x7b,0xf2,0x6b,0x6f,0xc5,0x30,0x01,0x67,0x2b,0xfe,0xd7,0xab,0x76,
    0xca,0x82,0xc9,0x7d,0xfa,0x59,0x47,0xf0,0xad,0xd4,0xa2,0xaf,0x9c,0xa4,0x72,0xc0,
    0xb7,0xfd,0x93,0x26,0x36,0x3f,0xf7,0xcc,0x34,0xa5,0xe5,0xf1,0x71,0xd8,0x31,0x15,
    0x04,0xc7,0x23,0xc3,0x18,0x96,0x05,0x9a,0x07,0x12,0x80,0xe2,0xeb,0x27,0xb2,0x75,
    0x09,0x83,0x2c,0x1a,0x1b,0x6e,0x5a,0xa0,0x52,0x3b,0xd6,0xb3,0x29,0xe3,0x2f,0x84,
    0x53,0xd1,0x00,0xed,0x20,0xfc,0xb1,0x5b,0x6a,0xcb,0xbe,0x39,0x4a,0x4c,0x58,0xcf,
    0xd0,0xef,0xaa,0xfb,0x43,0x4d,0x33,0x85,0x45,0xf9,0x02,0x7f,0x50,0x3c,0x9f,0xa8,
    0x51,0xa3,0x40,0x8f,0x92,0x9d,0x38,0xf5,0xbc,0xb6,0xda,0x21,0x10,0xff,0xf3,0xd2,
    0xcd,0x0c,0x13,0xec,0x5f,0x97,0x44,0x17,0xc4,0xa7,0x7e,0x3d,0x64,0x5d,0x19,0x73,
    0x60,0x81,0x4f,0xdc,0x22,0x2a,0x90,0x88,0x46,0xee,0xb8,0x14,0xde,0x5e,0x0b,0xdb,
    0xe0,0x32,0x3a,0x0a,0x49,0x06,0x24,0x5c,0xc2,0xd3,0xac,0x62,0x91,0x95,0xe4,0x79,
    0xe7,0xc8,0x37,0x6d,0x8d,0xd5,0x4e,0xa9,0x6c,0x56,0xf4,0xea,0x65,0x7a,0xae,0x08,
    0xba,0x78,0x25,0x2e,0x1c,0xa6,0xb4,0xc6,0xe8,0xdd,0x74,0x1f,0x4b,0xbd,0x8b,0x8a,
    0x70,0x3e,0xb5,0x66,0x48,0x03,0xf6,0x0e,0x61,0x35,0x57,0xb9,0x86,0xc1,0x1d,0x9e,
    0xe1,0xf8,0x98,0x11,0x69,0xd9,0x8e,0x94,0x9b,0x1e,0x87,0xe9,0xce,0x55,0x28,0xdf,
    0x8c,0xa1,0x89,0x0d,0xbf,0xe6,0x42,0x68,0x41,0x99,0x2d,0x0f,0xb0,0x54,0xbb,0x16,
};

HASSH_INLINE uint8_t hassh_xt(uint8_t x) { return (uint8_t)((x << 1) ^ ((x >> 7) * 0x1b)); }

/* One AES encryption round: MixColumns(ShiftRows(SubBytes(s))) ^ k. */
HASSH_INLINE hassh_blk hassh_aesenc_p(hassh_blk s, hassh_blk k) {
    uint8_t t[16];
    for (int c = 0; c < 4; c++)
        for (int r = 0; r < 4; r++)
            t[4 * c + r] = HASSH_SBOX[s.b[4 * ((c + r) & 3) + r]];
    hassh_blk o = {{0}};
    for (int c = 0; c < 4; c++) {
        uint8_t a0 = t[4*c], a1 = t[4*c+1], a2 = t[4*c+2], a3 = t[4*c+3];
        uint8_t all = a0 ^ a1 ^ a2 ^ a3;
        o.b[4*c+0] = a0 ^ all ^ hassh_xt(a0 ^ a1) ^ k.b[4*c+0];
        o.b[4*c+1] = a1 ^ all ^ hassh_xt(a1 ^ a2) ^ k.b[4*c+1];
        o.b[4*c+2] = a2 ^ all ^ hassh_xt(a2 ^ a3) ^ k.b[4*c+2];
        o.b[4*c+3] = a3 ^ all ^ hassh_xt(a3 ^ a0) ^ k.b[4*c+3];
    }
    return o;
}
HASSH_INLINE hassh_blk hassh_ld_p(const uint8_t *p) { hassh_blk b = {{0}}; memcpy(b.b, p, 16); return b; }
HASSH_INLINE hassh_blk hassh_x_p(hassh_blk a, hassh_blk b) {
    for (int i = 0; i < 16; i++) a.b[i] ^= b.b[i];
    return a;
}
HASSH_INLINE hassh_blk hassh_set_p(uint64_t lo, uint64_t hi) {
    hassh_blk b = {{0}};
    for (int i = 0; i < 8; i++) { b.b[i] = (uint8_t)(lo >> (8*i)); b.b[8+i] = (uint8_t)(hi >> (8*i)); }
    return b;
}
HASSH_INLINE hassh_blk hassh_key_p(int i) { return hassh_set_p(HASSH_K[2*i], HASSH_K[2*i+1]); }
HASSH_INLINE uint64_t hassh_out_p(hassh_blk x) {
    uint64_t lo = 0, hi = 0;
    for (int i = 0; i < 8; i++) { lo |= (uint64_t)x.b[i] << (8*i); hi |= (uint64_t)x.b[8+i] << (8*i); }
    return lo ^ hi;
}


/* Reference implementation for len > 16 (the specification the SIMD paths
 * are tested against).
 *
 * Every 16-byte block d entering lane i does  acc_i = P(acc_i ^ d), where P is
 * HASSH_ROUNDS keyed AES rounds. So any block difference passes through at
 * least HASSH_ROUNDS rounds before it can meet (and cancel) another block's
 * difference (the Pelican-MAC argument, for data differences only). P and the
 * finalizer use constant keys, so ALL seed protection rests on the lane start
 * values below. Lanes are XORed at the end. */
HASSH_INLINE hassh_blk hassh_perm_p(hassh_blk x) {
    for (int r = 0; r < HASSH_ROUNDS; r++) x = hassh_aesenc_p(x, hassh_key_p(16 + r));
    return x;
}
/* Nonlinear finalizer after the lane XOR. Without it the output is linear in
 * the lanes: h(0)^h(A)^h(B)^h(A^B) == 0 whenever A and B touch different lanes,
 * which SMHasher3's differential-distribution test catches immediately. */
HASSH_INLINE hassh_blk hassh_fin_p(hassh_blk x) {
    for (int r = 0; r < HASSH_FIN_ROUNDS; r++) x = hassh_aesenc_p(x, hassh_key_p(24 + r));
    return x;
}
/* Lane start: two keyed AES rounds of K_i ^ sv, sv from hassh_seedvec. An
 * attacker who wants to swap two lanes or move a message between lengths
 * must predict start_i ^ start_j (or start(L1) ^ start(L2)); after two rounds
 * every byte depends on every byte of the seed vector. Starting from
 * K_i ^ (seed, len) would allow a lane swap for every seed, and one round
 * over it a length change. See test/seedindep.c and test/xlen.c. */
HASSH_INLINE hassh_blk hassh_init_p(int i, hassh_blk sv) {
    return hassh_aesenc_p(hassh_aesenc_p(hassh_x_p(hassh_key_p(i), sv), hassh_key_p(HASSH_INIT_KEY0)),
                          hassh_key_p(HASSH_INIT_KEY1));
}
HASSH_INLINE hassh_blk hassh_sv_ps(uint64_t s) {
    uint64_t lo, hi; hassh_seedvec_s(s, &lo, &hi); return hassh_set_p(lo, hi);
}
HASSH_INLINE hassh_blk hassh_sv_p(uint64_t seed) { return hassh_sv_ps(hassh_seed_s(seed)); }
/* The 16 lane starts (256 bytes) for s. */
static inline void hassh_starts_p(uint8_t *out, uint64_t s) {
    hassh_blk sv = hassh_sv_ps(s);
    for (int i = 0; i < 16; i++) memcpy(out + 16 * i, hassh_init_p(i, sv).b, 16);
}
/* st: the precomputed lane starts, or NULL to compute them here. */
static inline uint64_t hassh_large_portable_s(const uint8_t *p, size_t len, uint64_t s,
                                              const uint8_t *st) {
    hassh_blk acc[16];
    int lanes = len <= 128 ? 8 : 16;     /* only called for len > 64 */
    if (st) {
        for (int i = 0; i < lanes; i++) acc[i] = hassh_ld_p(st + 16 * i);
    } else {
        hassh_blk sv = hassh_sv_ps(s);
        for (int i = 0; i < lanes; i++) acc[i] = hassh_init_p(i, sv);
    }
    if (len <= 256) {
        /* first half of the lanes read from the start, second half from the end */
        int half = lanes / 2;
        for (int i = 0; i < lanes; i++) {
            const uint8_t *src = i < half ? p + 16 * i : p + len - 16 * (lanes - i);
            acc[i] = hassh_perm_p(hassh_x_p(acc[i], hassh_ld_p(src)));
        }
    } else {
        /* 256-byte stripes, the final stripe aligned to the end of the input */
        size_t off = 0;
        for (;;) {
            if (off + 256 > len) off = len - 256;
            for (int i = 0; i < 16; i++)
                acc[i] = hassh_perm_p(hassh_x_p(acc[i], hassh_ld_p(p + off + 16 * i)));
            if (off + 256 >= len) break;
            off += 256;
        }
    }
    hassh_blk x = acc[0];
    for (int i = 1; i < lanes; i++) x = hassh_x_p(x, acc[i]);
    { uint64_t lo, hi; hassh_lenvec_s(s, len, &lo, &hi);
      x = hassh_x_p(x, hassh_aesenc_p(hassh_aesenc_p(hassh_set_p(lo, hi), hassh_key_p(HASSH_INIT_KEY0)),
                                      hassh_key_p(HASSH_INIT_KEY1))); }
    return hassh_out_p(hassh_fin_p(x));
}
/* Long inputs (len >= HASSH_LONG_MIN): two LeMac states.
 *
 * LeMac (Bariant, David, Leurent, ToSC 2024(1); message schedule corrected in
 * ToSC 2025(1)) is an AES-based universal hash: nine 128-bit state words
 * absorb 64 bytes per step with eight AES rounds, two rounds per 16 bytes
 * (the lanes need three). Its designers show (MILP) that every differential
 * trail from message differences to a state collision, through the
 * absorption and four blank steps, has at least 26 active S-boxes. (This and
 * the 4-round figure below are estimates in the usual sense: trail bounds,
 * and differential bounds averaged over random round keys, while hassh64's
 * round keys are public and the state holds only the seed's 64 bits.) hassh64
 * uses LeMac's step function unchanged, two independent states for
 * parallelism (one ymm register holds both), and its own start and
 * finalization:
 *   - word i of state j starts as two AES rounds of L(9j+i) ^ sv (seed only);
 *   - each 128-byte step gives state j the four words at offsets 32k + 16j;
 *     the final step is aligned to the end of the input, like the stripes;
 *   - after four blank steps, each of the 18 words passes four AES rounds,
 *     word w keyed by L(18+w) first, and they are XORed: two nonzero word
 *     differences then cancel with probability about the 4-round AES
 *     differential bound (about 2^-113), so a collision needs a state
 *     collision in both states;
 *   - lv is XORed in, and fin removes the linearity between the two states. */
HASSH_INLINE hassh_blk hassh_lkey_p(int w) { return hassh_set_p(HASSH_L[2*w], HASSH_L[2*w+1]); }
/* The 18 LeMac start words for s, word-major: out + 16(2i + j) = word i of state j. */
static inline void hassh_lstarts_p(uint8_t *out, uint64_t s) {
    hassh_blk sv = hassh_sv_ps(s);
    for (int j = 0; j < 2; j++)
        for (int i = 0; i < 9; i++)
            memcpy(out + 16 * (2 * i + j),
                   hassh_aesenc_p(hassh_aesenc_p(hassh_x_p(hassh_lkey_p(9 * j + i), sv), hassh_key_p(HASSH_INIT_KEY0)),
                                  hassh_key_p(HASSH_INIT_KEY1)).b, 16);
}
HASSH_INLINE hassh_blk hassh_lm_comp_p(int w, hassh_blk x) {
    x = hassh_x_p(x, hassh_lkey_p(18 + w));
    for (int r = 0; r < 4; r++) x = hassh_aesenc_p(x, hassh_key_p(16 + r));
    return x;
}
/* ls: the precomputed start words (hassh_lstarts_p layout), or NULL. */
static inline uint64_t hassh_long_portable_s(const uint8_t *p, size_t len, uint64_t s, const uint8_t *ls) {
    typedef hassh_blk hassh_lm_t;
    hassh_blk z = {{0}}, x = z, w[9];
    HASSH_LM_VARS(hassh_blk, v);
    uint8_t st[16 * 18];
    if (!ls) { hassh_lstarts_p(st, s); ls = st; }
    size_t nsteps = (len + 127) / 128;
    for (int j = 0; j < 2; j++) {
        v0 = hassh_ld_p(ls + 16 * j);       v1 = hassh_ld_p(ls + 16 * (2 + j));
        v2 = hassh_ld_p(ls + 16 * (4 + j)); v3 = hassh_ld_p(ls + 16 * (6 + j));
        v4 = hassh_ld_p(ls + 16 * (8 + j)); v5 = hassh_ld_p(ls + 16 * (10 + j));
        v6 = hassh_ld_p(ls + 16 * (12 + j)); v7 = hassh_ld_p(ls + 16 * (14 + j));
        v8 = hassh_ld_p(ls + 16 * (16 + j));
        vq = vr0 = vr1 = vr2 = z;
        for (size_t k = 0; k < nsteps; k++) {
            const uint8_t *q = p + (k + 1 < nsteps ? 128 * k : len - 128) + 16 * j;
            HASSH_LM_STEP(v, hassh_ld_p(q), hassh_ld_p(q + 32), hassh_ld_p(q + 64), hassh_ld_p(q + 96),
                          hassh_aesenc_p, hassh_x_p);
        }
        for (int b = 0; b < 4; b++) HASSH_LM_STEP(v, z, z, z, z, hassh_aesenc_p, hassh_x_p);
        w[0] = v0; w[1] = v1; w[2] = v2; w[3] = v3; w[4] = v4; w[5] = v5; w[6] = v6; w[7] = v7; w[8] = v8;
        for (int i = 0; i < 9; i++) x = hassh_x_p(x, hassh_lm_comp_p(9 * j + i, w[i]));
    }
    { uint64_t lo, hi; hassh_lenvec_s(s, len, &lo, &hi);
      x = hassh_x_p(x, hassh_aesenc_p(hassh_aesenc_p(hassh_set_p(lo, hi), hassh_key_p(HASSH_INIT_KEY0)),
                                      hassh_key_p(HASSH_INIT_KEY1))); }
    return hassh_out_p(hassh_fin_p(x));
}
static inline uint64_t hassh_large_portable(const uint8_t *p, size_t len, uint64_t seed) {
    if (len >= HASSH_LONG_MIN) return hassh_long_portable_s(p, len, hassh_seed_s(seed), NULL);
    return hassh_large_portable_s(p, len, hassh_seed_s(seed), NULL);
}
#endif /* portable */

/* ------------------------------------------------- 128-bit-lane SIMD core
 * One implementation for every ISA with a hardware AES round on 128-bit
 * vectors. Each ISA supplies:
 *   hassh_v                      vector type
 *   HV_LD(p) HV_XOR(a,b)         unaligned load, xor
 *   HV_ENC(x,k)                  one AES round = MixColumns(ShiftRows(SubBytes(x))) ^ k
 *   HV_KEY(i)                    key i from HASSH_K
 *   HV_SET(lo,hi)                vector from two little-endian u64 halves
 *   HV_FOLD(x)                   low u64 ^ high u64                         */
#if defined(HASSH_HAVE_SIMD) && defined(HASSH_X64)
#  define HASSH_X86 1
#  define HASSH_SIMD 1
#  if defined(__VAES__) && defined(__AVX2__) && !defined(HASSH_DISPATCH) && \
      (defined(__GNUC__) || defined(__clang__))   /* the long path uses GNU inline asm */
#    define HASSH_VAES 1
#  endif
typedef __m128i hassh_v;
#  define HV_LD(p)      _mm_loadu_si128((const __m128i *)(const void *)(p))
#  define HV_XOR(a, b)  _mm_xor_si128(a, b)
#  define HV_ENC(x, k)  _mm_aesenc_si128(x, k)
#  define HV_KEY(i)     _mm_loadu_si128((const __m128i *)(const void *)&HASSH_K[2 * (i)])
#  define HV_SET(lo,hi) _mm_set_epi64x((long long)(hi), (long long)(lo))
#  define HV_FOLD(x)    ((uint64_t)_mm_cvtsi128_si64(x) ^ (uint64_t)_mm_extract_epi64(x, 1))
#  define HV_ST(p, x)   _mm_storeu_si128((__m128i *)(void *)(p), x)
#elif defined(HASSH_HAVE_SIMD) && defined(__aarch64__)
#  define HASSH_ARM 1
#  define HASSH_SIMD 1
typedef uint8x16_t hassh_v;
/* AESE(x, k) = ShiftRows(SubBytes(x ^ k)); AESMC = MixColumns. With a zero
 * AESE key, AESMC(AESE(x, 0)) ^ k is exactly x86 AESENC(x, k). */
#  define HV_LD(p)      vld1q_u8((const uint8_t *)(p))
#  define HV_XOR(a, b)  veorq_u8(a, b)
#  define HV_ENC(x, k)  veorq_u8(vaesmcq_u8(vaeseq_u8(x, vdupq_n_u8(0))), k)
#  define HV_KEY(i)     vld1q_u8((const uint8_t *)&HASSH_K[2 * (i)])
#  define HV_SET(lo,hi) vreinterpretq_u8_u64(vcombine_u64(vcreate_u64(lo), vcreate_u64(hi)))
#  define HV_FOLD(x)    (vgetq_lane_u64(vreinterpretq_u64_u8(x), 0) ^ vgetq_lane_u64(vreinterpretq_u64_u8(x), 1))
#  define HV_ST(p, x)   vst1q_u8((uint8_t *)(p), x)
#endif
/* (HASSH_K is stored as native u64; HV_KEY's byte load equals the
 * little-endian byte order on both targets, which are little-endian.) */

#ifdef HASSH_SIMD
HASSH_SIMD_INLINE hassh_v hassh_perm(hassh_v x) {
    HASSH_UNROLL for (int r = 0; r < HASSH_ROUNDS; r++) x = HV_ENC(x, HV_KEY(16 + r));
    return x;
}
HASSH_SIMD_INLINE uint64_t hassh_out(hassh_v x) {
    HASSH_UNROLL for (int r = 0; r < HASSH_FIN_ROUNDS; r++) x = HV_ENC(x, HV_KEY(24 + r));
    return HV_FOLD(x);
}
#define HASSH_INIT_C(i) HV_ENC(HV_ENC(HV_XOR(HV_KEY(i), sv), HV_KEY(HASSH_INIT_KEY0)), HV_KEY(HASSH_INIT_KEY1))
#define HASSH_INIT(i) (st ? HV_LD(st + 16 * (i)) : HASSH_INIT_C(i))
HASSH_SIMD_INLINE hassh_v hassh_sv(uint64_t s) {
    uint64_t lo, hi; hassh_seedvec_s(s, &lo, &hi); return HV_SET(lo, hi);
}
HASSH_SIMD_INLINE hassh_v hassh_lv(uint64_t s, size_t len) {
    uint64_t lo, hi; hassh_lenvec_s(s, len, &lo, &hi);
    return HV_ENC(HV_ENC(HV_SET(lo, hi), HV_KEY(HASSH_INIT_KEY0)), HV_KEY(HASSH_INIT_KEY1));
}
HASSH_SIMD_ENTRY void hassh_starts(uint8_t *out, uint64_t s) {
    const hassh_v sv = hassh_sv(s);
    HASSH_UNROLL for (int i = 0; i < 16; i++) HV_ST(out + 16 * i, HASSH_INIT_C(i));
}

/* Long inputs, 128-bit vectors. With 32 registers (AArch64) both LeMac
 * states run together; with 16 (SSE) one at a time, over tiles of
 * HASSH_LM_TILE steps so the second pass re-reads the input from L1. The
 * states are independent until the end, so the order does not matter. */
#if defined(HASSH_ARM)
#  define HASSH_LM_G 2
#else
#  define HASSH_LM_G 1
#endif
#define HASSH_LM_TILE 64
#define HV_LKEY(w) HV_LD(&HASSH_L[2 * (w)])
HASSH_SIMD_ENTRY void hassh_lstarts(uint8_t *out, uint64_t s) {
    const hassh_v sv = hassh_sv(s);
    HASSH_UNROLL for (int j = 0; j < 2; j++)
        HASSH_UNROLL for (int i = 0; i < 9; i++)
            HV_ST(out + 16 * (2 * i + j),
                  HV_ENC(HV_ENC(HV_XOR(HV_LKEY(9 * j + i), sv), HV_KEY(HASSH_INIT_KEY0)), HV_KEY(HASSH_INIT_KEY1)));
}
#ifndef HASSH_VAES
/* Four blank steps on the state in a, then the 9 compressed words of state j into x. */
#define HASSH_LM_FINISH(a, j, x) do {                                           \
        HASSH_UNROLL for (int b_ = 0; b_ < 4; b_++) HASSH_LM_STEP(a, z, z, z, z, HV_ENC, HV_XOR); \
        const hassh_v w_[9] = { a##0, a##1, a##2, a##3, a##4, a##5, a##6, a##7, a##8 }; \
        HASSH_UNROLL for (int i_ = 0; i_ < 9; i_++) {                                        \
            hassh_v y_ = HV_XOR(w_[i_], HV_LKEY(18 + 9 * (j) + i_));            \
            HASSH_UNROLL for (int r_ = 0; r_ < 4; r_++) y_ = HV_ENC(y_, HV_KEY(16 + r_));    \
            x = HV_XOR(x, y_);                                                  \
        }                                                                       \
    } while (0)
#define HASSH_LM_START(a, j) do {                                               \
        a##0 = HV_LD(ls + 16 * (j));      a##1 = HV_LD(ls + 16 * (2 + (j)));    \
        a##2 = HV_LD(ls + 16 * (4 + (j)));  a##3 = HV_LD(ls + 16 * (6 + (j)));  \
        a##4 = HV_LD(ls + 16 * (8 + (j)));  a##5 = HV_LD(ls + 16 * (10 + (j))); \
        a##6 = HV_LD(ls + 16 * (12 + (j))); a##7 = HV_LD(ls + 16 * (14 + (j))); \
        a##8 = HV_LD(ls + 16 * (16 + (j))); a##q = a##r0 = a##r1 = a##r2 = z;   \
    } while (0)
HASSH_SIMD_BULK uint64_t hassh_long(const uint8_t *p, size_t len, uint64_t s, const uint8_t *ls) {
    typedef hassh_v hassh_lm_t;
    const hassh_v z = HV_XOR(HV_KEY(0), HV_KEY(0));
    uint8_t st[16 * 18];
    if (!ls) { hassh_lstarts(st, s); ls = st; }
    const size_t nsteps = (len + 127) / 128;
    const uint8_t *last = p + len - 128;
    hassh_v x = hassh_lv(s, len);
    HASSH_LM_VARS(hassh_v, a);
    HASSH_LM_START(a, 0);
#if HASSH_LM_G == 2
    HASSH_LM_VARS(hassh_v, b);
    HASSH_LM_START(b, 1);
    for (size_t k = 0; k < nsteps; k++) {
        const uint8_t *q = k + 1 < nsteps ? p + 128 * k : last;
        HASSH_LM_STEP(a, HV_LD(q), HV_LD(q + 32), HV_LD(q + 64), HV_LD(q + 96), HV_ENC, HV_XOR);
        HASSH_LM_STEP(b, HV_LD(q + 16), HV_LD(q + 48), HV_LD(q + 80), HV_LD(q + 112), HV_ENC, HV_XOR);
    }
    HASSH_LM_FINISH(a, 0, x);
    HASSH_LM_FINISH(b, 1, x);
#else
    HASSH_LM_VARS(hassh_v, b);
    HASSH_LM_START(b, 1);
    for (size_t k0 = 0; k0 < nsteps; k0 += HASSH_LM_TILE) {
        size_t k1 = nsteps - k0 < HASSH_LM_TILE ? nsteps : k0 + HASSH_LM_TILE;
        for (size_t k = k0; k < k1; k++) {
            const uint8_t *q = k + 1 < nsteps ? p + 128 * k : last;
            HASSH_LM_STEP(a, HV_LD(q), HV_LD(q + 32), HV_LD(q + 64), HV_LD(q + 96), HV_ENC, HV_XOR);
        }
        for (size_t k = k0; k < k1; k++) {
            const uint8_t *q = (k + 1 < nsteps ? p + 128 * k : last) + 16;
            HASSH_LM_STEP(b, HV_LD(q), HV_LD(q + 32), HV_LD(q + 64), HV_LD(q + 96), HV_ENC, HV_XOR);
        }
    }
    HASSH_LM_FINISH(a, 0, x);
    HASSH_LM_FINISH(b, 1, x);
#endif
    return hassh_out(x);
}
#endif /* !HASSH_VAES */

#ifndef HASSH_VAES
/* 65..128 bytes: 8 lanes, 4 from the head and 4 from the tail. */
HASSH_SIMD_ENTRY uint64_t hassh_65_128(const uint8_t *p, size_t len, uint64_t s, const uint8_t *st) {
    const hassh_v sv = hassh_sv(s);
    const uint8_t *e = p + len;
    hassh_v x = HV_XOR(HV_XOR(hassh_perm(HV_XOR(HASSH_INIT(0), HV_LD(p))),
                              hassh_perm(HV_XOR(HASSH_INIT(1), HV_LD(p + 16)))),
                       HV_XOR(hassh_perm(HV_XOR(HASSH_INIT(2), HV_LD(p + 32))),
                              hassh_perm(HV_XOR(HASSH_INIT(3), HV_LD(p + 48)))));
    hassh_v y = HV_XOR(HV_XOR(hassh_perm(HV_XOR(HASSH_INIT(4), HV_LD(e - 64))),
                              hassh_perm(HV_XOR(HASSH_INIT(5), HV_LD(e - 48)))),
                       HV_XOR(hassh_perm(HV_XOR(HASSH_INIT(6), HV_LD(e - 32))),
                              hassh_perm(HV_XOR(HASSH_INIT(7), HV_LD(e - 16)))));
    return hassh_out(HV_XOR(HV_XOR(x, y), hassh_lv(s, len)));
}
/* 129+ bytes: 16 lanes. */
HASSH_SIMD_BULK uint64_t hassh_large(const uint8_t *p, size_t len, uint64_t s, const uint8_t *st) {
    const hassh_v sv = hassh_sv(s);
    hassh_v a[16];
    HASSH_UNROLL for (int i = 0; i < 16; i++) a[i] = HASSH_INIT(i);
    if (len <= 256) {
        const uint8_t *q = p + len - 128;
        HASSH_UNROLL for (int i = 0; i < 8; i++)  a[i] = hassh_perm(HV_XOR(a[i], HV_LD(p + 16 * i)));
        HASSH_UNROLL for (int i = 8; i < 16; i++) a[i] = hassh_perm(HV_XOR(a[i], HV_LD(q + 16 * (i - 8))));
    } else {
        const uint8_t *last = p + len - 256;
        for (;;) {
            const uint8_t *q = p < last ? p : last;
            HASSH_UNROLL for (int i = 0; i < 16; i++) a[i] = hassh_perm(HV_XOR(a[i], HV_LD(q + 16 * i)));
            if (q == last) break;
            p += 256;
        }
    }
    hassh_v x = a[0];
    HASSH_UNROLL for (int i = 1; i < 16; i++) x = HV_XOR(x, a[i]);
    return hassh_out(HV_XOR(x, hassh_lv(s, len)));
}
#else /* HASSH_VAES: ymm j holds lanes (2j, 2j+1); same function, twice as wide. */
HASSH_INLINE __m256i hassh_key2(int j) { return _mm256_loadu_si256((const __m256i *)(const void *)&HASSH_K[4 * j]); }
HASSH_INLINE __m256i hassh_ld2(const uint8_t *p) { return _mm256_loadu_si256((const __m256i *)(const void *)p); }
HASSH_INLINE __m256i hassh_perm2(__m256i x) {
    HASSH_UNROLL for (int r = 0; r < HASSH_ROUNDS; r++)
        x = _mm256_aesenc_epi128(x, _mm256_broadcastsi128_si256(HV_KEY(16 + r)));
    return x;
}
HASSH_INLINE __m256i hassh_init2(int j, __m256i sv) {
    __m256i x = _mm256_aesenc_epi128(_mm256_xor_si256(hassh_key2(j), sv),
                                     _mm256_broadcastsi128_si256(HV_KEY(HASSH_INIT_KEY0)));
    return _mm256_aesenc_epi128(x, _mm256_broadcastsi128_si256(HV_KEY(HASSH_INIT_KEY1)));
}
HASSH_INLINE __m256i hassh_sv2(uint64_t s) {
    return _mm256_broadcastsi128_si256(hassh_sv(s));
}
HASSH_INLINE __m256i hassh_start2(int j, __m256i sv, const uint8_t *st) {
    return st ? hassh_ld2(st + 32 * j) : hassh_init2(j, sv);
}
HASSH_INLINE uint64_t hassh_out2(__m256i x, uint64_t s, size_t len) {
    return hassh_out(_mm_xor_si128(_mm_xor_si128(_mm256_castsi256_si128(x), _mm256_extracti128_si256(x, 1)),
                                   hassh_lv(s, len)));
}
HASSH_INLINE uint64_t hassh_65_128(const uint8_t *p, size_t len, uint64_t s, const uint8_t *st) {
    const __m256i sv = hassh_sv2(s);
    const uint8_t *e = p + len;
    __m256i a = hassh_perm2(_mm256_xor_si256(hassh_start2(0, sv, st), hassh_ld2(p)));
    __m256i b = hassh_perm2(_mm256_xor_si256(hassh_start2(1, sv, st), hassh_ld2(p + 32)));
    __m256i c = hassh_perm2(_mm256_xor_si256(hassh_start2(2, sv, st), hassh_ld2(e - 64)));
    __m256i d = hassh_perm2(_mm256_xor_si256(hassh_start2(3, sv, st), hassh_ld2(e - 32)));
    return hassh_out2(_mm256_xor_si256(_mm256_xor_si256(a, b), _mm256_xor_si256(c, d)), s, len);
}
static inline uint64_t hassh_large(const uint8_t *p, size_t len, uint64_t s, const uint8_t *st) {
    const __m256i sv = hassh_sv2(s);
    __m256i a[8];
    HASSH_UNROLL for (int j = 0; j < 8; j++) a[j] = hassh_start2(j, sv, st);
    if (len <= 256) {
        const uint8_t *q = p + len - 128;
        HASSH_UNROLL for (int j = 0; j < 4; j++) a[j] = hassh_perm2(_mm256_xor_si256(a[j], hassh_ld2(p + 32 * j)));
        HASSH_UNROLL for (int j = 4; j < 8; j++) a[j] = hassh_perm2(_mm256_xor_si256(a[j], hassh_ld2(q + 32 * (j - 4))));
    } else {
        const uint8_t *last = p + len - 256;
        for (;;) {
            const uint8_t *q = p < last ? p : last;
            HASSH_UNROLL for (int j = 0; j < 8; j++) a[j] = hassh_perm2(_mm256_xor_si256(a[j], hassh_ld2(q + 32 * j)));
            if (q == last) break;
            p += 256;
        }
    }
    __m256i x = _mm256_xor_si256(_mm256_xor_si256(_mm256_xor_si256(a[0], a[1]), _mm256_xor_si256(a[2], a[3])),
                                 _mm256_xor_si256(_mm256_xor_si256(a[4], a[5]), _mm256_xor_si256(a[6], a[7])));
    return hassh_out2(x, s, len);
}
/* Long inputs with VAES: one ymm register holds word i of both states. */
/* n LeMac steps (n >= 1) at q, q + 128, ...; A = S0..S8, R0, R1, R1^R2.
 * LeMac's RR (the previous step's M2) is re-read from *prev + 64; *prev is
 * updated to the last step's address.
 *
 * Inline assembly (GCC and Clang share the syntax) rather than intrinsics:
 * the 12 state registers plus 4 message words fill all 16 ymm registers, and
 * GCC's allocator spills inside the loop at -O3 (measured 15 bytes/cycle
 * against 35 here). Each message word is loaded exactly once, since an
 * unaligned 32-byte load that splits a cache line is expensive; the
 * prefetch keeps L2-resident inputs streaming. */
static const uint8_t hassh_zero128[128] = {0};
static inline void hassh_lm_steps2(uint8_t *A, const uint8_t *q, size_t n, const uint8_t **prev) {
    const uint8_t *pv = *prev;
    __asm__ volatile(
        "vmovdqu 0x000(%[a]), %%ymm0\n\t"  "vmovdqu 0x020(%[a]), %%ymm1\n\t"
        "vmovdqu 0x040(%[a]), %%ymm2\n\t"  "vmovdqu 0x060(%[a]), %%ymm3\n\t"
        "vmovdqu 0x080(%[a]), %%ymm4\n\t"  "vmovdqu 0x0a0(%[a]), %%ymm5\n\t"
        "vmovdqu 0x0c0(%[a]), %%ymm6\n\t"  "vmovdqu 0x0e0(%[a]), %%ymm7\n\t"
        "vmovdqu 0x100(%[a]), %%ymm8\n\t"  "vmovdqu 0x120(%[a]), %%ymm9\n\t"
        "vmovdqu 0x140(%[a]), %%ymm10\n\t" "vmovdqu 0x160(%[a]), %%ymm11\n\t"
        ".p2align 5\n"
        "1:\n\t"
        "prefetcht0 1024(%[q])\n\t"
        "prefetcht0 1088(%[q])\n\t"
        "vmovdqu 64(%[q]), %%ymm14\n\t"            /* M2 */
        "vmovdqu 96(%[q]), %%ymm15\n\t"            /* M3 */
        "vmovdqu 32(%[q]), %%ymm13\n\t"            /* M1 */
        "vmovdqu (%[q]), %%ymm12\n\t"              /* M0 */
        "vpxor %%ymm8, %%ymm14, %%ymm14\n\t"
        "vpxor %%ymm0, %%ymm14, %%ymm14\n\t"       /* new S0 = S0 ^ S8 ^ M2 */
        "vaesenc %%ymm15, %%ymm7, %%ymm8\n\t"
        "vaesenc %%ymm13, %%ymm6, %%ymm7\n\t"
        "vaesenc %%ymm13, %%ymm5, %%ymm6\n\t"
        "vaesenc %%ymm12, %%ymm4, %%ymm5\n\t"
        "vaesenc %%ymm12, %%ymm3, %%ymm4\n\t"
        "vaesenc %%ymm11, %%ymm2, %%ymm3\n\t"
        "vaesenc %%ymm15, %%ymm1, %%ymm2\n\t"
        "vaesenc %%ymm15, %%ymm0, %%ymm1\n\t"
        "vmovdqa %%ymm14, %%ymm0\n\t"
        "vpxor %%ymm9, %%ymm10, %%ymm11\n\t"       /* R1^R2 for the next step */
        "vmovdqa %%ymm9, %%ymm10\n\t"              /* R1 = R0 */
        "vpxor 64(%[pv]), %%ymm13, %%ymm9\n\t"     /* R0 = RR ^ M1 */
        "mov %[q], %[pv]\n\t"
        "add $128, %[q]\n\t"
        "dec %[n]\n\t"
        "jnz 1b\n\t"
        "vmovdqu %%ymm0, 0x000(%[a])\n\t"  "vmovdqu %%ymm1, 0x020(%[a])\n\t"
        "vmovdqu %%ymm2, 0x040(%[a])\n\t"  "vmovdqu %%ymm3, 0x060(%[a])\n\t"
        "vmovdqu %%ymm4, 0x080(%[a])\n\t"  "vmovdqu %%ymm5, 0x0a0(%[a])\n\t"
        "vmovdqu %%ymm6, 0x0c0(%[a])\n\t"  "vmovdqu %%ymm7, 0x0e0(%[a])\n\t"
        "vmovdqu %%ymm8, 0x100(%[a])\n\t"  "vmovdqu %%ymm9, 0x120(%[a])\n\t"
        "vmovdqu %%ymm10, 0x140(%[a])\n\t" "vmovdqu %%ymm11, 0x160(%[a])\n\t"
        : [q] "+r"(q), [n] "+r"(n), [pv] "+r"(pv)
        : [a] "r"(A)
        : "memory", "cc", "xmm0", "xmm1", "xmm2", "xmm3", "xmm4", "xmm5", "xmm6", "xmm7",
          "xmm8", "xmm9", "xmm10", "xmm11", "xmm12", "xmm13", "xmm14", "xmm15");
    *prev = pv;
}
static inline uint64_t hassh_long(const uint8_t *p, size_t len, uint64_t s, const uint8_t *ls) {
    typedef __m256i hassh_lm_t;
    const __m256i z = _mm256_setzero_si256();
    uint8_t st[16 * 18];
    if (!ls) { hassh_lstarts(st, s); ls = st; }
    /* A byte buffer with unaligned accesses rather than an __m256i array:
     * MinGW GCC does not realign the stack for 32-byte locals (GCC bug 54412)
     * yet emits aligned moves for them. The asm uses unaligned moves. */
    uint8_t A[12 * 32];
    memcpy(A, ls, 9 * 32);
    memset(A + 9 * 32, 0, 3 * 32);
    size_t nsteps = (len + 127) / 128;
    const uint8_t *prev = hassh_zero128;
    if (nsteps > 1) hassh_lm_steps2(A, p, nsteps - 1, &prev);
    hassh_lm_steps2(A, p + len - 128, 1, &prev);   /* the final step is aligned to the end */
    HASSH_LM_VARS(__m256i, a);
    a0 = hassh_ld2(A);       a1 = hassh_ld2(A + 32);  a2 = hassh_ld2(A + 64);  a3 = hassh_ld2(A + 96);
    a4 = hassh_ld2(A + 128); a5 = hassh_ld2(A + 160); a6 = hassh_ld2(A + 192); a7 = hassh_ld2(A + 224);
    a8 = hassh_ld2(A + 256);
    aq = hassh_ld2(prev + 64); ar0 = hassh_ld2(A + 288); ar1 = hassh_ld2(A + 320); ar2 = hassh_ld2(A + 352);
    HASSH_UNROLL for (int b = 0; b < 4; b++) HASSH_LM_STEP(a, z, z, z, z, _mm256_aesenc_epi128, _mm256_xor_si256);
    const __m256i w[9] = { a0, a1, a2, a3, a4, a5, a6, a7, a8 };
    __m256i acc = z;
    HASSH_UNROLL for (int i = 0; i < 9; i++) {
        __m256i y = _mm256_xor_si256(w[i], _mm256_set_m128i(HV_LKEY(27 + i), HV_LKEY(18 + i)));
        HASSH_UNROLL for (int r = 0; r < 4; r++) y = _mm256_aesenc_epi128(y, _mm256_broadcastsi128_si256(HV_KEY(16 + r)));
        acc = _mm256_xor_si256(acc, y);
    }
    return hassh_out(_mm_xor_si128(_mm_xor_si128(_mm256_castsi256_si128(acc), _mm256_extracti128_si256(acc, 1)),
                                   hassh_lv(s, len)));
}
#endif /* HASSH_VAES */
#endif /* HASSH_SIMD */

/* ---------------------------------------------------------------- API */

#ifdef HASSH_DISPATCH
/* 1 if the CPU has AES-NI and SSE4.1 (CPUID leaf 1, ECX bits 25 and 19).
 * Plain cpuid rather than __builtin_cpu_supports, which needs libgcc's
 * __cpu_model and fails to link with some toolchains. Cached, since cpuid is
 * slow (a VM exit under virtualization); the cache is per translation unit
 * and every thread writes the same value. */
static int hassh_cpu_state;   /* 0 unknown, 1 yes, 2 no */
static __attribute__((noinline, cold)) int hassh_cpu_probe(void) {
    unsigned a, b, c = 0, d;
    int ok = __get_cpuid(1, &a, &b, &c, &d) && (c & (1u << 25)) && (c & (1u << 19));
    __atomic_store_n(&hassh_cpu_state, ok ? 1 : 2, __ATOMIC_RELAXED);
    return ok;
}
HASSH_INLINE int hassh_cpu_aes(void) {
    int st = __atomic_load_n(&hassh_cpu_state, __ATOMIC_RELAXED);
    return HASSH_LIKELY(st != 0) ? st == 1 : hassh_cpu_probe();
}
#endif

/* 65+ bytes with s = hassh_seed_s(seed); st = precomputed lane starts or NULL. */
HASSH_INLINE uint64_t hassh_over64(const uint8_t *p, size_t len, uint64_t s, const uint8_t *st,
                                   const uint8_t *ls) {
#if defined(HASSH_DISPATCH)
    if (hassh_cpu_aes()) {
        if (len <= 128) return hassh_65_128(p, len, s, st);
        return len < HASSH_LONG_MIN ? hassh_large(p, len, s, st) : hassh_long(p, len, s, ls);
    }
    if (len >= HASSH_LONG_MIN) return hassh_long_portable_s(p, len, s, ls);
    return hassh_large_portable_s(p, len, s, st);
#elif defined(HASSH_VAES)
    if (len <= 128) return hassh_65_128(p, len, s, st);
    return len < HASSH_LONG_MIN ? hassh_large(p, len, s, st) : hassh_long(p, len, s, ls);
#elif defined(HASSH_SIMD)
    if (len <= 128) return hassh_65_128(p, len, s, st);
    return len < HASSH_LONG_MIN ? hassh_large(p, len, s, st) : hassh_long(p, len, s, ls);
#else
    if (len >= HASSH_LONG_MIN) return hassh_long_portable_s(p, len, s, ls);
    return hassh_large_portable_s(p, len, s, st);
#endif
}

HASSH_INLINE uint64_t hassh64(const void *data, size_t len, uint64_t seed) {
    const uint8_t *p = (const uint8_t *)data;
    const uint64_t s = hassh_seed_s(seed);
    if (HASSH_LIKELY(len <= 32)) return hassh_small_k(p, len, s, hassh_seed_s2(s, len));
    if (len <= 64) return hassh_mid_k(p, len, s, hassh_seed_s2(s, len));
    return hassh_over64(p, len, s, NULL, NULL);
}

/* Prepared seed. A hash table hashes millions of keys with one seed, so the
 * seed-dependent keys can be derived once instead of on every call:
 *
 *     hassh64_key k;
 *     hassh64_key_init(&k, seed);
 *     uint64_t h = hassh64_keyed(&k, data, len);   // == hassh64(data, len, seed)
 *
 * The result is always identical to hassh64(); only the work per call drops:
 * two dependent multiplies fewer up to 64 bytes, and above that the 16 AES
 * lane starts and the 18 LeMac start words (two rounds each) are
 * precomputed. The key is 1072 bytes; keep one per seed, not one per call. */
typedef struct hassh64_key {
    uint64_t s;
    uint64_t s2[65];          /* hassh_seed_s2(s, len) for len 0..64 */
    uint8_t  start[16 * 16];  /* the 16 AES lane starts */
    uint8_t  lstart[16 * 18]; /* the 18 LeMac start words (long inputs) */
} hassh64_key;

static inline void hassh64_key_init(hassh64_key *k, uint64_t seed) {
    k->s = hassh_seed_s(seed);
    for (size_t len = 0; len <= 64; len++) k->s2[len] = hassh_seed_s2(k->s, len);
#if defined(HASSH_DISPATCH)
    if (hassh_cpu_aes()) { hassh_starts(k->start, k->s); hassh_lstarts(k->lstart, k->s); }
    else { hassh_starts_p(k->start, k->s); hassh_lstarts_p(k->lstart, k->s); }
#elif defined(HASSH_SIMD)
    hassh_starts(k->start, k->s);
    hassh_lstarts(k->lstart, k->s);
#else
    hassh_starts_p(k->start, k->s);
    hassh_lstarts_p(k->lstart, k->s);
#endif
}

HASSH_INLINE uint64_t hassh64_keyed(const hassh64_key *k, const void *data, size_t len) {
    const uint8_t *p = (const uint8_t *)data;
    if (HASSH_LIKELY(len <= 32)) return hassh_small_k(p, len, k->s, k->s2[len]);
    if (len <= 64) return hassh_mid_k(p, len, k->s, k->s2[len]);
    return hassh_over64(p, len, k->s, k->start, k->lstart);
}

/* Internal macros stay internal. HASSH_P, HASSH_K and the hassh_* helpers
 * remain visible (the test suite uses them); no configuration macro does. */
#undef HASSH_INLINE
#undef HASSH_LIKELY
#undef HASSH_UNROLL
#undef HASSH_X64
#undef HASSH_HAVE_SIMD
#undef HASSH_DISPATCH
#undef HASSH_SIMD
#undef HASSH_X86
#undef HASSH_ARM
#undef HASSH_VAES
#undef HASSH_SEEDKEYS
#undef HASSH_INIT
#undef HASSH_INIT_C
#undef HV_ST
#undef HV_LD
#undef HV_XOR
#undef HV_ENC
#undef HV_KEY
#undef HV_SET
#undef HV_FOLD
#undef HV_LKEY
#ifndef HASSH_WANT_PORTABLE   /* test/lemac_ref.c checks the step function */
#  undef HASSH_LM_STEP
#  undef HASSH_LM_VARS
#endif
#undef HASSH_LM_FINISH
#undef HASSH_LM_START
#undef HASSH_LM_G
#undef HASSH_LM_TILE
#undef HASSH_TGT
#undef HASSH_SIMD_INLINE
#undef HASSH_SIMD_ENTRY
#undef HASSH_SIMD_BULK

#ifdef __cplusplus
}
#endif
#endif /* HASSH_H */
