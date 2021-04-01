#include "Platform.h"

// not vulnerable to bad seeds. the strict variant.
#define WYHASH_CONDOM 2
#include "wyhash.h"
void wyhash_condom_test (const void * key, int len, uint32_t seed, void * out) {
  *(uint64_t*)out = wyhash(key, (uint64_t)len, (uint64_t)seed, _wyp);
}
