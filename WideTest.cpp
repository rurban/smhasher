#include "Types.h"
#include <stdio.h>
#include <stdlib.h>

bool WideTest(pfHash hash, int hashbits)
{
  const unsigned pairs = 65536;
  const unsigned maxbits = 512;
  if (hashbits <= 0 || hashbits > (int)maxbits || hashbits % 8 != 0) {
    fprintf(stderr, "Wide test requires a whole-byte hash of at most 512 bits\n");
    return false;
  }

  alignas(16) uint8_t key[64] = {};
  alignas(16) uint8_t output[2][64] = {};
  uint8_t *left = output[0], *right = output[1];
  unsigned flips[maxbits] = {};
  hash(key, sizeof(key), 0, left);
  for (unsigned pair = 0; pair < pairs; ++pair) {
    const unsigned value = pair + 1;
    for (unsigned i = 0; i < 4; ++i) key[i] = (uint8_t)(value >> (8 * i));
    hash(key, sizeof(key), 0, right);
    for (int bit = 0; bit < hashbits; ++bit)
      flips[bit] += ((left[bit / 8] ^ right[bit / 8]) >> (bit % 8)) & 1;
    uint8_t *tmp = left; left = right; right = tmp;
  }

  unsigned worst = 0;
  int worstbit = 0;
  for (int bit = 0; bit < hashbits; ++bit) {
    const unsigned deviation = (unsigned)abs((int)(2 * flips[bit]) - (int)pairs);
    if (deviation > worst) { worst = deviation; worstbit = bit; }
  }
  // Under independent unbiased output bits, z = (2*flips - pairs)/sqrt(pairs).
  const double z = worst / 256.0;
  printf("Adjacent keys: %u pairs, worst bit %d: %.3f standard deviations\n",
         pairs, worstbit, z);
  return z <= 7.0;
}
