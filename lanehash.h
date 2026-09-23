/* lanehash - fast non-cryptographic 64/128-bit hash, no AES needed (C port of the Rust
 * crate's default function). Inputs over 64 bytes: eight chains of NH-32 stripes;
 * shorter: 64x64->128 multiply-folds. Same bits on every backend: scalar, SSE2, AVX2
 * (-mavx2), AVX-512F (-mavx512f); LANEHASH_PORTABLE forces scalar, LANEHASH_NO_INT128
 * avoids unsigned __int128.
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

/* 128-bit hash to `out`, 16 little-endian bytes; the low 64 bits equal lanehash64. */
void lanehash128(const void *key, size_t len, uint64_t seed, void *out);

#ifdef __cplusplus
}
#endif

#endif /* LANEHASH_H */
