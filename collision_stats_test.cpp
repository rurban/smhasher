#include "Stats.h"
#include <algorithm>

static bool check(unsigned duplicates, bool expected_pass)
{
  const uint32_t count = 50643;
  std::vector<uint32_t> full(count);
  std::vector<uint64_t> wide(count);
  for (uint32_t i = 0; i < count; ++i) {
    uint32_t value = i >= count - duplicates ? count - 1 - i : i;
    full[i] = value;
    wide[i] = (uint64_t(value) << 32) | i;
  }
  std::sort(wide.begin(), wide.end());
  bool full_pass = TestHashList(full, false, true, false, false, false, false);
  bool high_pass = CountHighbitsCollisions(wide, 32);
  bool low_pass = CountLowbitsCollisions(wide, 32);
  return full_pass == expected_pass && high_pass == expected_pass && low_pass == expected_pass;
}

int main()
{
  return check(2, true) && check(5, true) && check(6, false) ? 0 : 1;
}
