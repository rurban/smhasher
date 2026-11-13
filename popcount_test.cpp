#include "Bitvec.h"
#include <stdio.h>

static uint32_t reference_popcount(uint32_t value)
{
  uint32_t count = 0;
  while (value) {
    count += value & 1U;
    value >>= 1;
  }
  return count;
}

static bool check(uint32_t value)
{
  uint32_t expected = reference_popcount(value);
  uint32_t actual = popcount(value);
  if (actual != expected) {
    fprintf(stderr, "popcount(%08x): expected %u, got %u\n", value, expected, actual);
    return false;
  }
  return true;
}

int main()
{
  for (uint32_t value = 0; value <= 0xffff; ++value) {
    if (!check(value) || !check(value << 16) || !check(value | (value << 16)))
      return 1;
  }
  return check(0xffffffffU) ? 0 : 1;
}
