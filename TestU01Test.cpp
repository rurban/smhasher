#include "Types.h"
#include <stdint.h>
#include <stdio.h>

extern "C" {
#include <TestU01/unif01.h>
#include <TestU01/bbattery.h>
}

extern bool Seed_init(HashInfo *info, size_t seed);

namespace {
HashInfo *current_hash;
uint64_t counter;
alignas(16) uint8_t digest[64];
unsigned position;

unsigned int next_word()
{
  const unsigned bytes = (unsigned)current_hash->hashbits / 8;
  if (position == bytes) {
    alignas(16) uint8_t key[64] = {};
    for (unsigned i = 0; i < 8; ++i)
      key[i] = (uint8_t)(counter >> (8 * i));
    ++counter;
    current_hash->hash(key, sizeof(key), 0, digest);
    position = 0;
  }
  unsigned int word = 0;
  for (unsigned i = 0; i < 4; ++i)
    word |= (unsigned int)digest[position++] << (8 * i);
  return word;
}
}

void TestU01Test(HashInfo *info, int battery)
{
  if (info->hashbits <= 0 || info->hashbits > (int)(sizeof(digest) * 8) ||
      info->hashbits % 32 != 0) {
    fprintf(stderr, "TestU01 requires a hash width divisible by 32 and at most 512 bits\n");
    return;
  }
  current_hash = info;
  counter = 0;
  position = (unsigned)info->hashbits / 8;
  Seed_init(info, 0);
  unif01_Gen *generator = unif01_CreateExternGenBits(const_cast<char *>(info->name), next_word);
  if (battery == 0) bbattery_SmallCrush(generator);
  else if (battery == 1) bbattery_Crush(generator);
  else bbattery_BigCrush(generator);
  unif01_DeleteExternGenBits(generator);
}
