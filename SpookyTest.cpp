#include "Spooky.h"

static void store_le(void *out, uint64_t value, unsigned bytes) {
  unsigned char *p = static_cast<unsigned char *>(out);
  for (unsigned i = 0; i < bytes; ++i) p[i] = static_cast<unsigned char>(value >> (8 * i));
}

void SpookyHash32_test(const void *key, int len, uint32_t seed, void *out) {
  store_le(out, SpookyHashV1::Hash32(key, len, seed), 4);
}

void SpookyHash64_test(const void *key, int len, uint32_t seed, void *out) {
  store_le(out, SpookyHashV1::Hash64(key, len, seed), 8);
}

void SpookyHash128_test(const void *key, int len, uint32_t seed, void *out) {
  uint64_t h1 = seed, h2 = seed;
  SpookyHashV1::Hash128(key, len, &h1, &h2);
  store_le(out, h1, 8);
  store_le(static_cast<unsigned char *>(out) + 8, h2, 8);
}

