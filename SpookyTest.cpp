#include "Spooky.h"

void SpookyHash32_test(const void *key, int len, uint32_t seed, void *out) {
  *(uint32_t*)out = SpookyHash::Hash32(key, len, seed);
}

void SpookyHash64_test(const void *key, int len, uint32_t seed, void *out) {
  *(uint64_t*)out = SpookyHash::Hash64(key, len, seed);
}

void SpookyHash128_test(const void *key, int len, uint32_t seed, void *out) {
  uint64_t h1 = seed, h2 = seed;
  SpookyHash::Hash128(key, len, &h1, &h2);
  ((uint64_t*)out)[0] = h1;
  ((uint64_t*)out)[1] = h2;
}

void SpookyHash64_with_state_test(const void *key, int len, const void *seed, void *out) {
  *(uint64_t*)out = SpookyHash::Hash64(key, len, *((uint64_t *)seed));
}

void SpookyHash128_with_state_test(const void *key, int len, const void *seed, void *out) {
  uint64_t h1 = ((uint64_t *)seed)[0];
  uint64_t h2 = ((uint64_t *)seed)[1];
  SpookyHash::Hash128(key, len, &h1, &h2);
  ((uint64_t*)out)[0] = h1;
  ((uint64_t*)out)[1] = h2;
}
