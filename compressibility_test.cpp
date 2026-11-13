#include "CompressibilityTest.h"
#include <string.h>

static void constant_hash(const void *, int, uint32_t, void *out)
{
  uint64_t value = 0;
  memcpy(out, &value, sizeof(value));
}

static void mixed_hash(const void *key, int len, uint32_t seed, void *out)
{
  const uint8_t *bytes = static_cast<const uint8_t *>(key);
  uint64_t value = UINT64_C(14695981039346656037) ^ seed;
  for (int i = 0; i < len; ++i) value = (value ^ bytes[i]) * UINT64_C(1099511628211);
  value ^= value >> 30;
  value *= UINT64_C(0xbf58476d1ce4e5b9);
  value ^= value >> 27;
  value *= UINT64_C(0x94d049bb133111eb);
  value ^= value >> 31;
  memcpy(out, &value, sizeof(value));
}

int main()
{
  return CompressibilityTest(constant_hash, 32) ||
         CompressibilityTest(constant_hash, 64) ||
         !CompressibilityTest(mixed_hash, 32) ||
         !CompressibilityTest(mixed_hash, 64);
}
