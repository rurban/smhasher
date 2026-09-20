/* lh2 - fast non-cryptographic 64/128-bit hash for machines without AES instructions
 * (C port of the Rust crate's `lanehash::lh2`).
 *
 * Inputs over 64 bytes are absorbed by eight chains of NH-32 stripes, shorter inputs by
 * 64x64->128 multiply-folds. Output is identical on every backend: portable scalar,
 * SSE2, AVX2 (-mavx2), AVX-512F (-mavx512f).
 * Verification values (SMHasher): 0xD048C22B (64-bit), 0xC2F39939 (128-bit).
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

/* 128-bit hash, written to `out` as 16 little-endian bytes (low 64 bits first; those
 * low 64 bits equal lanehash64 of the same input). */
void lanehash128(const void *key, size_t len, uint64_t seed, void *out);

#ifdef __cplusplus
}
#endif

#endif /* LANEHASH_H */
