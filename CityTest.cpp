#include "City.h"
#include "CityCrc.h"

static void store_le(void *out, uint64_t value, unsigned bytes) {
  unsigned char *p = static_cast<unsigned char *>(out);
  for (unsigned i = 0; i < bytes; ++i) p[i] = static_cast<unsigned char>(value >> (8 * i));
}

void CityHash32_test(const void *key, int len, uint32_t seed, void *out) {
  // objsize 0-527: 1319
  store_le(out, CityHash32WithSeed((const char *)key, len, seed), 4);
}

void CityHash64_test(const void *key, int len, uint32_t seed, void *out) {
  // objsize 1038 + a20-a72: 82 = 1120
  store_le(out, CityHash64WithSeed((const char *)key, len, seed), 8);
}

void CityHash64noSeed_test(const void *key, int len, uint32_t seed, void *out) {
  // objsize 530-93e: 1038
  store_le(out, CityHash64((const char *)key, len), 8);
  (void)seed;
}

#if defined(HAVE_SSE42) && defined(__x86_64__)
void CityHash128_test(const void *key, int len, uint32_t seed, void *out) {
  uint128 s(0, 0);
  s.first = seed;

  // objsize ad0-1201: 1841
  uint128 hash = CityHash128WithSeed((const char *)key, len, s);
  store_le(out, hash.first, 8);
  store_le(static_cast<unsigned char *>(out) + 8, hash.second, 8);
}

void CityHashCrc128_test(const void *key, int len, uint32_t seed, void *out) {
  uint128 s(0, 0);
  s.first = seed;

  // objsize 1940-1a67: 295
  uint128 hash = CityHashCrc128WithSeed((const char *)key, len, s);
  store_le(out, hash.first, 8);
  store_le(static_cast<unsigned char *>(out) + 8, hash.second, 8);
}

void CityHashCrc256_test(const void *key, int len, uint32_t seed, void *out) {
  uint64_t hash[4];
  CityHashCrc256((const char *)key, len, hash);
  for (unsigned i = 0; i < 4; ++i)
    store_le(static_cast<unsigned char *>(out) + 8 * i, hash[i], 8);
  (void)seed;
}
#endif
