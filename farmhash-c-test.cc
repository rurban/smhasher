#include "farmhash-c.h"

#include <stdint.h>
#include <string.h>

void farmhash32_c_test ( const void * key, int len, uint32_t seed, void * out ) {
  *(uint32_t*)out = farmhash32_with_seed((const char *)key,(size_t)len,seed);
}
void farmhash64_c_test ( const void * key, int len, uint32_t seed, void * out ) {
  *(uint64_t*)out = farmhash64_with_seed((const char *)key,(size_t)len,(uint64_t)seed);
}
void farmhash128_c_test ( const void * key, int len, uint32_t seed, void * out ) {
  uint128_c_t s = make_uint128_c_t(seed, 0UL);
  uint128_c_t result = farmhash128_with_seed((const char *)key, (size_t)len, s);
  memcpy(out, &result, 128/8);
}
