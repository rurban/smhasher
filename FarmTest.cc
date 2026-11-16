/* We have conflicting uint128_t types in Types.h (Blob<128>) and farmhash.h */

#undef FARMHASH_UINT128_T_DEFINED
#ifdef HAVE_SSE42
# define FARMHASH_ASSUME_SSE42
#endif
#ifdef HAVE_AESNI
# define FARMHASH_ASSUME_AESNI
#endif
#include "farmhash.h"

static void farm_store_le(void *out, uint64_t value, unsigned bytes) {
  uint8_t *p = (uint8_t *)out;
  for (unsigned i = 0; i < bytes; ++i) p[i] = (uint8_t)(value >> (8 * i));
}

void FarmHash32_test ( const void * key, int len, uint32_t seed, void * out ) {
  using namespace NAMESPACE_FOR_HASH_FUNCTIONS;
  farm_store_le(out, Hash32WithSeed((const char *)key,(size_t)len,seed), 4);
}
void FarmHash64_test ( const void * key, int len, uint32_t seed, void * out ) {
  using namespace NAMESPACE_FOR_HASH_FUNCTIONS;
  farm_store_le(out, Hash64WithSeed((const char *)key,(size_t)len,(uint64_t)seed), 8);
}
void FarmHash64noSeed_test ( const void * key, int len, uint32_t seed, void * out ) {
  using namespace NAMESPACE_FOR_HASH_FUNCTIONS;
  farm_store_le(out, Hash64((const char *)key,(size_t)len), 8); (void)seed;
}
void FarmHash128_test ( const void * key, int len, uint32_t seed, void * out ) {
  using namespace NAMESPACE_FOR_HASH_FUNCTIONS;
  uint128_t s((uint64_t)seed, (uint64_t)0UL);
  uint128_t result = Hash128WithSeed((const char *)key, (size_t)len, s);
  farm_store_le(out, Uint128Low64(result), 8);
  farm_store_le((uint8_t *)out + 8, Uint128High64(result), 8);
}
