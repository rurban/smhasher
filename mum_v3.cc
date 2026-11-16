#define MUM_CANONICAL_ENDIAN
#include "mum.h"

void mum_v3_hash_test(const void *key, int len, uint32_t seed, void *out) {
  uint64_t h = mum_hash(key, len, seed);
  uint8_t *bytes = (uint8_t *)out;
  for (unsigned i = 0; i < 8; ++i) bytes[i] = (uint8_t)(h >> (i * 8));
}
