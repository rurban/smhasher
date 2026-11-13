#include "Types.h"
#include <string.h>

bool WideTest(pfHash hash, int hashbits);

static uint32_t key_value(const void *key)
{
  const uint8_t *p = (const uint8_t *)key;
  return (uint32_t)p[0] | (uint32_t)p[1] << 8 |
         (uint32_t)p[2] << 16 | (uint32_t)p[3] << 24;
}
static void linear(const void *key, int, uint32_t, void *out)
{
  uint32_t x = key_value(key);
  memcpy(out, &x, sizeof(x));
}
static void mixed(const void *key, int, uint32_t, void *out)
{
  uint32_t x = key_value(key);
  x ^= x >> 16; x *= 0x85ebca6bU;
  x ^= x >> 13; x *= 0xc2b2ae35U;
  x ^= x >> 16;
  memcpy(out, &x, sizeof(x));
}
int main()
{
  return !WideTest(linear, 32) && WideTest(mixed, 32) ? 0 : 1;
}
