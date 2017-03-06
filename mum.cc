#include "mum.h"

void mum_hash_test(const void *key, int len, uint32_t seed, void *out) {
  *(uint64_t *)out = mum_hash(key, len, seed);
}
void mum_hash_with_state_test(const void *key, int len, const void *state, void *out) {
  *(uint64_t *)out = mum_hash(key, len, *((uint64_t*)state));
}
