/* We have conflicting uint128_t types in Types.h (Blob<128>) and farmhash.h */

#undef FARMHASH_UINT128_T_DEFINED
#include "farmhash.h"

void FarmHash32_with_state_test ( const void * key, int len, const void *state, void * out ) {
  using namespace NAMESPACE_FOR_HASH_FUNCTIONS;
  *(uint32_t*)out = Hash64WithSeed((const char *)key,(size_t)len,*((uint64_t*)state));
}

void FarmHash64_with_state_test ( const void * key, int len, const void *state, void * out ) {
  using namespace NAMESPACE_FOR_HASH_FUNCTIONS;
  *(uint64_t*)out = Hash64WithSeed((const char *)key,(size_t)len,*((uint64_t*)state));
}

void FarmHash128_with_state_test ( const void * key, int len, const void *state, void * out ) {
  using namespace NAMESPACE_FOR_HASH_FUNCTIONS;
  *((uint128_t*)out) = Hash128WithSeed((const char *)key, (size_t)len, *((uint128_t *)state));
}
