/* We have conflicting uint128_t types in Types.h (Blob<128>) and farmhash.h */

#undef FARMHASH_UINT128_T_DEFINED
#include "farmhash.h"

void FarmHash32_test ( const void * key, int len, uint32_t seed, void * out ) {
  using namespace NAMESPACE_FOR_HASH_FUNCTIONS;
  *(uint32_t*)out = Hash32WithSeed((const char *)key,(size_t)len,seed);
}
void FarmHash64_test ( const void * key, int len, uint32_t seed, void * out ) {
  using namespace NAMESPACE_FOR_HASH_FUNCTIONS;
  *(uint64_t*)out = Hash64WithSeed((const char *)key,(size_t)len,(uint64_t)seed);
}
void FarmHash128_test ( const void * key, int len, uint32_t seed, void * out ) {
  using namespace NAMESPACE_FOR_HASH_FUNCTIONS;
  uint128_t s((uint64_t)seed, (uint64_t)0UL);
  uint128_t result = Hash128WithSeed((const char *)key, (size_t)len, s);
  memcpy(out, &result, 128/8);
}
