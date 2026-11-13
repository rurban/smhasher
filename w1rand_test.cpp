#include "wyhash.h"
#include <stdint.h>
#include <stdio.h>

static int check(uint64_t state, const uint64_t expected[3])
{
  for (int i = 0; i < 3; ++i) {
    uint64_t result = w1rand(&state);
    if (result != expected[i]) {
      fprintf(stderr, "w1rand sequence mismatch at index %d\n", i);
      return 0;
    }
  }
  return 1;
}

int main(void)
{
  static const uint64_t zero[] = {
    UINT64_C(0x0000000000000000), UINT64_C(0x07b8db09d78faa8e), UINT64_C(0xe5fcb3a47a3fa26c)
  };
  static const uint64_t wrap[] = {
    UINT64_C(0xd07ebc63274654c6), UINT64_C(0x37330f68f04d5e32), UINT64_C(0x347607855df90e2a)
  };
  return !(check(0, zero) && check(UINT64_MAX, wrap));
}
