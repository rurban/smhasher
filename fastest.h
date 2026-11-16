/* fastest - cross-platform bit-exact SIMD hardware-accelerated hash & XOF/stream cipher.
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#ifndef FASTEST_H
#define FASTEST_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Computes a deterministic 32-bit hash equal to the low 32 bits of fastest64. */
uint32_t fastest32(const void *data, size_t len, uint64_t seed);

/* Computes a deterministic 64-bit hash equal to the low 64 bits of fastest128. */
uint64_t fastest64(const void *data, size_t len, uint64_t seed);

/* Computes a deterministic 128-bit hash written as 16 little-endian bytes to `out`. */
void fastest128(const void *data, size_t len, uint64_t seed, void *out);

/* Computes a deterministic 256-bit hash written as 32 little-endian bytes to `out`. */
void fastest256(const void *data, size_t len, uint64_t seed, void *out);

/* Fills `out[..out_len]` with a deterministic arbitrary-length XOF digest. */
void fastest_xof(const void *data, size_t len, uint64_t seed, void *out, size_t out_len);

/* XORs `buf[..buf_len]` in place with the seekable keystream derived from `(key, key_len, seed)` at `offset`. */
void fastest_xor_stream(
  const void *key,
  size_t key_len,
  uint64_t seed,
  uint64_t offset,
  void *buf,
  size_t buf_len
);

#ifdef __cplusplus
}
#endif

#endif /* FASTEST_H */
