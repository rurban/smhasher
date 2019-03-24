#include <stdlib.h>
#include <cstring>

//#include "highwayhash/c_bindings.h"
#include "highwayhash/highwayhash_target.h"
//#include "highwayhash/instruction_sets.h"
//using highwayhash::InstructionSets;

using highwayhash::HighwayHash;
using highwayhash::HHKey;
using highwayhash::HHResult64;

static HHKey HighwayHash_key;

void HighwayHash_init()
{
  for(int i=0; i<4; i++){
    uint64_t r = random();
    memcpy(&HighwayHash_key[i], &r, 8);
  }
}

void HighwayHash64(const void *bytes, int len, uint32_t seed, void *out)
{
  memcpy(&HighwayHash_key, &seed, 4);
  HighwayHash<4>(&HighwayHash_key, (const char*)bytes, (uint64_t)len,
                 reinterpret_cast<HHResult64*>(out));
}
