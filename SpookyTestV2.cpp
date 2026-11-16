#include "SpookyV2.h"

static void store_le(void *out, uint64_t value, unsigned bytes) {
  unsigned char *p = static_cast<unsigned char *>(out);
  for (unsigned i = 0; i < bytes; ++i) p[i] = static_cast<unsigned char>(value >> (8 * i));
}

void SpookyV2_32_test(const void *key, int len, uint32 seed, void *out) {
  store_le(out, SpookyHash::Hash32(key, len, seed), 4);
}

void SpookyV2_64_test(const void *key, int len, uint32 seed, void *out) {
  store_le(out, SpookyHash::Hash64(key, len, seed), 8);
}

void SpookyV2_128_test(const void *key, int len, uint32 seed, void *out) {
  uint64 h1 = seed, h2 = seed;
  SpookyHash::Hash128(key, len, &h1, &h2);
  store_le(out, h1, 8);
  store_le(static_cast<unsigned char *>(out) + 8, h2, 8);
}

