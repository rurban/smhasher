// Tachyon
// Copyright (c) byt3forg3
// Licensed under the MIT or Apache 2.0 License
// -------------------------------------------------------------------------

/**
 * @file tachyon_impl.h
 * @brief Tachyon — Internal Implementation Details
 *
 * This header is NOT part of the public API. It is included only by the
 * Tachyon C source files. Callers should include tachyon.h instead.
 *
 * All numeric constants (except Golden Ratio) are derived from a single rule:
 *
 *   constant = floor(frac(ln(p)) * 2^64)
 *
 * where p is a prime and frac(x) = x - floor(x).
 *
 * Verify (no external tools required):
 *   python3 -c "import math; p=2; print(hex(int((math.log(p)%1)*2**64)))"
 *   Replace p with the prime listed next to each constant.
 *
 * Prime assignment (consecutive, partitioned by purpose):
 *   C0-C3, C5-C7   : ln(2, 3, 5, 7, 11, 13, 17)
 *   WHITENING0/1   : ln(19), ln(23)
 *   KEY_SCHEDULE_MULT: ln(29)
 *   CLMUL_CONSTANT : ln(31)
 *   LANE_OFFSETS   : ln(37..191) — 32 consecutive primes
 *   C4, KEY_SCHEDULE_BASE, CHAOS_BASE: Golden Ratio (φ)
 */

#ifndef TACHYON_IMPL_H
#define TACHYON_IMPL_H

#include "tachyon.h"
#include <stdint.h>
#include <stddef.h>

// =============================================================================
// ROUNDS
// =============================================================================

/// 10 AES rounds for complete diffusion (standard AES-128 round count).
#define ROUNDS 10

// =============================================================================
// STRUCTURAL CONSTANTS
// =============================================================================

/// Main block size for full compression (in bytes).
#define BLOCK_SIZE 512

/// Remainder chunk size for finalization (in bytes).
#define REMAINDER_CHUNK_SIZE 64

/// Number of parallel lanes in the state.
#define NUM_LANES 8

/// Elements per lane (128-bit vectors).
#define LANE_STRIDE 4

/// Total state size in bytes (32 × 16-byte vectors).
#define STATE_SIZE 512

/// Size of a single 128-bit vector in bytes.
#define VEC_SIZE 16

/// AES GF(2^8) reduction polynomial: x^8 + x^4 + x^3 + x + 1
#define GF_POLY 0x1b

/// Hash output size in bytes (256-bit digest).
#define HASH_SIZE 32

// =============================================================================
// INDEX ARITHMETIC MACROS
// =============================================================================

/// Compute flat index into acc[32] from lane and element indices.
/// Treats acc[32] as an 8-lane × 4-element matrix: acc[lane*4 + elem].
#define ACC_INDEX(lane, elem) ((lane) * LANE_STRIDE + (elem))

/// Compute data array index from lane and element (8×4 layout).
#define DATA_INDEX(lane, elem) ((lane) * LANE_STRIDE + (elem))

// =============================================================================
// GOLDEN RATIO
// =============================================================================

/// φ (Golden Ratio) in 64-bit fixed-point: floor(2^64 / φ).
/// Used wherever a canonical nothing-up-my-sleeve constant is needed
#define GOLDEN_RATIO  0x9E3779B97F4A7C15ULL

// =============================================================================
// INITIALIZATION CONSTANTS — frac(ln(p)) for consecutive primes
// =============================================================================

#define C0 0xB17217F7D1CF79ABULL // ln(2)
#define C1 0x193EA7AAD030A976ULL // ln(3)
#define C2 0x9C041F7ED8D336AFULL // ln(5)
#define C3 0xF2272AE325A57546ULL // ln(7)
#define C4 GOLDEN_RATIO          // φ — Golden Ratio (no prime equivalent)
#define C5 0x65DC76EFE6E976F7ULL // ln(11)
#define C6 0x90A08566318A1FD0ULL // ln(13)
#define C7 0xD54D783F4FEF39DFULL // ln(17)

// =============================================================================
// KEY SCHEDULE
// =============================================================================

/// Starting value for the AESENC-derived round key chain.
#define KEY_SCHEDULE_BASE GOLDEN_RATIO
/// Per-round diversification multiplier: frac(ln(29)).
#define KEY_SCHEDULE_MULT 0x5E071979BFC3D7ACULL // ln(29)

// =============================================================================
// LANE OFFSETS — frac(ln(p)) for primes 37..191
// =============================================================================
//
// Per-lane tweaks that break symmetry across the 8 parallel AES lanes.
// 32 unique offsets for full track diversification.

static const uint64_t LANE_OFFSETS[32] = {
    0x9C651DC758F7A6F2ULL, // ln(37)
    0xB6ACA8B1D589B575ULL, // ln(41)
    0xC2DE02C29D8222CBULL, // ln(43)
    0xD9A345F21E16CB31ULL, // ln(47)
    0xF8650D044795568FULL, // ln(53)
    0x13D97E71CA5E2DA9ULL, // ln(59)
    0x1C623AC49B03386CULL, // ln(61)
    0x3466BC4A044B5829ULL, // ln(67)
    0x433EFD0935B23D6BULL, // ln(71)
    0x4A5B8CC88BF98CD3ULL, // ln(73)
    0x5E94226BEC5CBFB8ULL, // ln(79)
    0x6B392358B9206784ULL, // ln(83)
    0x7D1745EBA2BD8E2DULL, // ln(89)
    0x9320423952FE003BULL, // ln(97)
    0x9D7889C6EE8C2F8EULL, // ln(101)
    0xA27D995644FAF994ULL, // ln(103)
    0xAC3E82AFD1D6DC79ULL, // ln(107)
    0xB0FC2CC0554191F5ULL, // ln(109)
    0xBA36168CE0D6EE1DULL, // ln(113)
    0xD81CA5180B90858DULL, // ln(127)
    0xE00CEE88B2189A5CULL, // ln(131)
    0xEB83DEB56027349AULL, // ln(137)
    0xEF39AF05C2C4931BULL, // ln(139)
    0x0102A006F9CB3C2AULL, // ln(149)
    0x046C738E0014C2F8ULL, // ln(151)
    0x0E662006821719E4ULL, // ln(157)
    0x1800035E755EC056ULL, // ln(163)
    0x1E34D7AD75D7A815ULL, // ln(167)
    0x273E1E311EA1A70BULL, // ln(173)
    0x2FF88423D2160504ULL, // ln(179)
    0x32D0B391A3CAA870ULL, // ln(181)
    0x4094FDCB1C2E7EE1ULL  // ln(191)
};

// =============================================================================
// FINALIZATION
// =============================================================================

/// Chaos injection constant for entropy in sparse inputs.
#define CHAOS_BASE GOLDEN_RATIO

/// Carry-less multiplication constant: frac(ln(31)).
/// Polynomial coefficient in GF(2^128).
#define CLMUL_CONSTANT  0x6F19C912256B3E22ULL // ln(31)

/// Second CLMUL constant for polynomial differentiation: frac(ln(193)).
#define CLMUL_CONSTANT2 0x433FAA0A53988000ULL  // ln(193)

/// Pre-whitening constants: frac(ln(19)) and frac(ln(23)).
#define WHITENING0 0xF1C6C0C096658E40ULL // ln(19)
#define WHITENING1 0x22AFBFBA367E0122ULL // ln(23)

// Merkle tree node type tags (XORed into domain field to distinguish leaf/node)
#define DOMAIN_LEAF 0xFFFFFFFF00000000ULL
#define DOMAIN_NODE 0xFFFFFFFF00000001ULL

// =============================================================================
// SHORT PATH PRECOMPUTED STATE
// =============================================================================
//
// Precomputed post-merge state for seed=0, key=None.
// Recompute: run tachyon_portable.c with seed=0, key=None and print acc[] after linear_init().
// Values are stable across all conforming implementations.

static const uint64_t SHORT_INIT[4][2] = {
    {0x8572268C3E8B949AULL, 0x55260EB0F6D08B28ULL},
    {0x7B6B869404C510F3ULL, 0x58153672FF7257BBULL},
    {0x23AE5234151A861EULL, 0x436D91128FA3A475ULL},
    {0x2D3EA94F6D07F7BCULL, 0x31C028B304D23746ULL}
};

// =============================================================================
// PRECOMPUTED ROUND KEY CHAIN
// =============================================================================
//
// AESENC-derived round key schedule for the 10-round key expansion.
// Recompute: initialize acc[0] = {GOLDEN_RATIO, GOLDEN_RATIO} and apply
// aesenc_s() ten times, reading out the 128-bit result after each round.
// See tachyon_portable.c for the portable aesenc_s() implementation.

static const uint64_t RK_CHAIN[10][2] = {
    {0x9E3779B97F4A7C15ULL, 0xFBEB0F5699A30AE2ULL},
    {0xE0772D418B604247ULL, 0xCB99FBAD212715AAULL},
    {0x9943E41C900EA2BDULL, 0x3391839B4E1DB7D2ULL},
    {0x3FDD17D01F01E973ULL, 0x4FE62D4E63CB7DB7ULL},
    {0x7C5B681836BF20E5ULL, 0x20EA7205089674B4ULL},
    {0x57E52B0B6FD122C4ULL, 0x92E23D97BDB01EABULL},
    {0x9E667CEF92177102ULL, 0x1A1761F6D1C3AAA5ULL},
    {0x5976F92D468FE2FDULL, 0xAE3623405BAFD085ULL},
    {0xCD2AF6F6F29BF341ULL, 0xD310BEDDA16B12D4ULL},
    {0xD11A12CCD34BBD1BULL, 0xAC09BEFD5925A5FEULL}
};

// =============================================================================
// BACKEND KERNEL PROTOTYPES
// =============================================================================
//
// Three hardware-specific implementations selected at runtime by the
// dispatcher in tachyon_dispatcher.c via CPUID.

void tachyon_avx512_oneshot (const uint8_t *input, size_t len, uint64_t domain, uint64_t seed, const uint8_t *key, uint8_t *out);
void tachyon_aesni_oneshot  (const uint8_t *input, size_t len, uint64_t domain, uint64_t seed, const uint8_t *key, uint8_t *out);
void tachyon_portable_oneshot(const uint8_t *input, size_t len, uint64_t domain, uint64_t seed, const uint8_t *key, uint8_t *out);

#endif // TACHYON_IMPL_H
