#ifndef _FARSH_H
#define _FARSH_H
#include <stddef.h>   /* for size_t */
#include <stdint.h>   /* for uint32_t & uint64_t */

/* Return 32-bit hash of the buffer */
uint32_t farsh (const void *data, size_t bytes, uint64_t seed);

/* Compute `n` 32-bit hashes starting with the hash number `k`, storing results to the `hash` buffer.
It's `n` times slower than computation of single 32-bit hash.
Hash computed by the `farsh` function has number 0. The function aborts if `k+n > 32`. */
void farsh_n (const void *data, size_t bytes, int k, int n, uint64_t seed, void *hash);

/* Compute 32-bit hash using `key`, that should be 1024-byte long and aligned to 16-byte boundary. */
uint32_t farsh_keyed (const void *data, size_t bytes, const void *key, uint64_t seed);

/* Compute `n` 32-bit hashes using `key`, storing results to the `hash` buffer.
`key` should be `1024+16*(n-1)` bytes long and aligned to 16-byte boundary. */
void farsh_keyed_n (const void *data, size_t bytes, const void *key, int n, uint64_t seed, void *hash);

/* Hash functions accept 64-bit `seed` that can be used to "personalize" the hash value. Use seed==0 if you don't need that feature.
Seeding may have lower quality than in xxHash&co since the seed value mixed with block hashes rather than raw data. */

/* Symbolic names for the above-mentioned constants */
#define FARSH_MAX_HASHES             32  /* number of 32-bit hashes supported by the built-in key */
#define FARSH_BASE_KEY_SIZE        1024  /* size of user-supplied key required to compute 32-bit hash with index 0 */
#define FARSH_EXTRA_KEY_SIZE         16  /* extra bytes required to compute 32-bit hash with every next index */
#define FARSH_BASE_KEY_ALIGNMENT     16  /* user-supplied key should be aligned to this size, otherwise SSE2 code may fail. For maximum speed, it's recommended to align key to 64 bytes. */

#endif
