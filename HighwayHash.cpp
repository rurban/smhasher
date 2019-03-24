#include <stdlib.h>
#include <string.h>
#include "highwayhash/c_bindings.h"

static uint64_t HighwayHash_key[4];

void HighwayHash_init()
{
  for(int i=0; i<4; i++){
    uint64_t r = random();
    memcpy(&HighwayHash_key[i], &r, 8);
  }
}

void HighwayHash64_test(const void *bytes, int len, uint32_t seed, void *out)
{
  memcpy(&HighwayHash_key, &seed, 4);
  *(uint64_t*)out = HighwayHash64(HighwayHash_key, (const char*)bytes, (uint64_t)len);
}
