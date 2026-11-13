#include "KeysetTest.h"
#include <string.h>

bool Seed_init(HashInfo *, size_t) { return true; }

static void good(const void *, int len, uint32_t seed, void *out)
{
  uint32_t value = 0x100U + len + seed;
  memcpy(out, &value, sizeof(value));
}
static void unseeded(const void *, int len, uint32_t, void *out)
{
  uint32_t value = 0x100U + len;
  memcpy(out, &value, sizeof(value));
}
static void zero_keys(const void *, int len, uint32_t seed, void *out)
{
  uint32_t value = len ? 0 : 0x100U + seed;
  memcpy(out, &value, sizeof(value));
}

int main()
{
  HashInfo info = {good, 32, 0, "good", "", GOOD, {}};
  if (!NullKeyTest(&info)) return 1;
  info.hash = unseeded;
  if (NullKeyTest(&info)) return 1;
  info.hash = zero_keys;
  return NullKeyTest(&info) ? 0 : 1;
}
