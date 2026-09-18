/* lanehash - fast non-cryptographic 64/128-bit hash (C port of the Rust crate).
 *
 * Inputs over 64 bytes are absorbed by 4/8/16 independent AES lanes, shorter
 * inputs by 64x64->128 multiply-folds. Output is identical on every backend:
 * portable software AES, AES-NI (x86 -maes), VAES+AVX2 (x86 -mvaes -mavx2).
 * Verification values (SMHasher): 0x9FF60BEF (64-bit), 0x1A79672D (128-bit).
 *
 * SPDX-License-Identifier: MIT OR Apache-2.0
 */
#ifndef LANEHASH_H
#define LANEHASH_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* 64-bit hash of `len` bytes at `key` under `seed`. */
uint64_t lanehash64(const void *key, size_t len, uint64_t seed);

/* 128-bit hash, written to `out` as 16 little-endian bytes (low 64 bits first;
 * those low 64 bits equal lanehash64 of the same input). */
void lanehash128(const void *key, size_t len, uint64_t seed, void *out);

#ifdef __cplusplus
}
#endif

#endif /* LANEHASH_H */
