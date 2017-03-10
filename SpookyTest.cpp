#include "Spooky.h"


void SpookyHash32_with_state_test(const void *key, int len, const void *state, void *out) {
  uint64_t *state64= (uint64_t *)state;
  uint64_t s0 = state64[0];
  uint64_t s1 = state64[1];
  SpookyHash::Hash128(key, len, &s0, &s1);
  ((uint32_t *)out)[0]= (uint32_t)s0;
}

void SpookyHash64_with_state_test(const void *key, int len, const void *state, void *out) {
  uint64_t *state64= (uint64_t *)state;
  uint64_t *out64= (uint64_t *)out;
  out64[0] = state64[0];
  uint64_t s1 = state64[1];
  SpookyHash::Hash128(key, len, out64, &s1);
}

void SpookyHash128_with_state_test(const void *key, int len, const void *state, void *out) {
  uint64_t *state64= (uint64_t *)state;
  uint64_t *out64= (uint64_t *)out;
  out64[0] = state64[0];
  out64[1] = state64[1];
  SpookyHash::Hash128(key, len, out64, out64+1);
}

void SpookyHash_seed_state_test(int in_bits, const void *seed, void *state) {
    uint64_t *state64= (uint64_t *)state;
    if (in_bits == 32) {
        state64[0]= state64[1]= ((uint32_t*)seed)[0];
    }
    else {
        uint64_t *seed64= (uint64_t *)seed;
        if (in_bits == 64) {
            state64[0]= state64[1]= seed64[0];
        }
        else
        if (in_bits == 128) {
            state64[0]= seed64[0];
            state64[1]= seed64[1];
        }
    }
}
