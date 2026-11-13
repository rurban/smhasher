#include "KeysetTest.h"

static void linear_hash(const void *key, int, uint32_t seed, void *out)
{
  uint32_t value;
  memcpy(&value, key, sizeof(value));
  value += seed;
  memcpy(out, &value, sizeof(value));
}

static void mixed_hash(const void *key, int, uint32_t seed, void *out)
{
  uint32_t value;
  memcpy(&value, key, sizeof(value));
  value += seed;
  value ^= value >> 16;
  value *= 0x7feb352dU;
  value ^= value >> 15;
  value *= 0x846ca68bU;
  value ^= value >> 16;
  memcpy(out, &value, sizeof(value));
}

int main()
{
  return ArithmeticCollisionTest(linear_hash, 1) ||
         !ArithmeticCollisionTest(mixed_hash, 1);
}
