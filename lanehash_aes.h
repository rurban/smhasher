/* lanehash_aes - the AES-lane function (C port of the Rust crate's `lanehash::aes`).
 * Inputs over 64 bytes: 4/8/16 AES lanes; shorter: 64x64->128 multiply-folds. Same bits
 * on every backend: software AES, AES-NI (-maes), VAES+AVX2 (-mvaes -mavx2).
 * Verification values (SMHasher): 0x9FF60BEF (64-bit), 0x1A79672D (128-bit).
 *
 * SPDX-License-Identifier: MIT OR Apache-2.0
 */
#ifndef LANEHASH_AES_H
#define LANEHASH_AES_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* 64-bit hash of `len` bytes at `key` under `seed`. */
uint64_t lanehash_aes64(const void *key, size_t len, uint64_t seed);

/* 128-bit hash to `out`, 16 little-endian bytes; the low 64 bits equal lanehash_aes64. */
void lanehash_aes128(const void *key, size_t len, uint64_t seed, void *out);

#ifdef __cplusplus
}
#endif

#endif /* LANEHASH_AES_H */
