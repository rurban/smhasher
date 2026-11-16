#include <stdint.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include "tabulation.h"

int main()
{
  uint8_t key[2050];
  for (size_t i = 0; i < sizeof(key); ++i) key[i] = (uint8_t)i;
  const uint32_t seeds[] = {0, 1, 0x80000000U, 0xffffffffU};
  const int sizes32[] = {0, 3, 4, 16, 1025};
  const uint32_t expected32[][5] = {
    {0x2d733238,0x11766f1e,0xfe6d9031,0x967f8d31,0xbea7dfd7},
    {0x9bece34d,0xc57d271a,0xfe51b4fb,0x41263463,0x45fdb4e5},
    {0xb6446e1a,0x7078a192,0xe8ae93fe,0xe0d7a743,0x4449513b},
    {0x2cf75f9b,0xf77b6c47,0x5bbc5713,0x0d4d81bf,0x45cbc78b}
  };
#ifdef __SIZEOF_INT128__
  const int sizes64[] = {0, 7, 8, 16, 2050};
  const uint64_t expected64[][5] = {
    {0x9565730a51b73df5ULL,0x5128d9cdd39aa7f5ULL,0xb2bbfed6c2f6cff5ULL,0x866357a6a4ffa794ULL,0x72261e06ead9940cULL},
    {0x055dd1080083cc40ULL,0xf8a48ee185613e8eULL,0x6936267e4b0ae26bULL,0x77b9b4e0780809a0ULL,0x8657e14b1b0b5df1ULL},
    {0xc45cd44b39a04664ULL,0xe7a461e7adc1f338ULL,0xa18936bd7b344585ULL,0xcfebce9a7c6ca49fULL,0xd5be4c50d0a93232ULL},
    {0x32f049b4b32537deULL,0x0705299e440f20e1ULL,0x3bdfcb4181b085ecULL,0x5e1fd874c0c8e4aaULL,0xee60f063cc252458ULL}
  };
#endif
  srand(1234);
  const int first = rand();
  for (size_t s = 0; s < sizeof(seeds) / sizeof(seeds[0]); ++s) {
    size_t seed = seeds[s];
    tabulation_32_seed_init(seed);
    for (size_t i = 0; i < 5; ++i)
      if (tabulation_32_hash(key, sizes32[i], seeds[s]) != expected32[s][i]) return 1;
#ifdef __SIZEOF_INT128__
    tabulation_seed_init(seed);
    for (size_t i = 0; i < 5; ++i)
      if (tabulation_hash(key, sizes64[i], seeds[s]) != expected64[s][i]) return 1;
#endif
  }
  const int second = rand();
  srand(1234);
  if (rand() != first || rand() != second) return 1;
  return 0;
}
