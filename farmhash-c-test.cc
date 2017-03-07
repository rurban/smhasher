#include "farmhash-c.h"

#include <stdint.h>
#include <string.h>

void farmhash32_c_with_state_test ( const void * key, int len, const void *state, void * out ) {
  *(uint32_t*)out = farmhash64_with_seed((const char *)key,(size_t)len,*((uint64_t*)state));
}

void farmhash64_c_with_state_test ( const void * key, int len, const void *state, void * out ) {
  *(uint64_t*)out = farmhash64_with_seed((const char *)key,(size_t)len,*((uint64_t*)state));
}

void farmhash128_c_with_state_test ( const void * key, int len, const void *state, void * out ) {
  uint128_c_t result = farmhash128_with_seed((const char *)key, (size_t)len, *((uint128_c_t *)state));
  memcpy(out, &result, 128/8);
}
