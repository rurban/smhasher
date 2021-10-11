#include "SpookyV2.h"

void SpookyV2_32_test(const void *key, int len, uint32 seed, void *out) {
  *(uint32*)out = SpookyHash::Hash32(key, len, seed);
}

void SpookyV2_64_test(const void *key, int len, uint32 seed, void *out) {
  *(uint64*)out = SpookyHash::Hash64(key, len, seed);
}

void SpookyV2_128_test(const void *key, int len, uint32 seed, void *out) {
  uint64 h1 = seed, h2 = seed;
  SpookyHash::Hash128(key, len, &h1, &h2);
  ((uint64*)out)[0] = h1;
  ((uint64*)out)[1] = h2;
}

