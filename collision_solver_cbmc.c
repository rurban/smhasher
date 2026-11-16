#include <stdint.h>
#include <assert.h>
#include "PMurHash.h"

static uint32_t hash_word(uint32_t word) {
  union { uint32_t alignment; unsigned char bytes[4]; } key;
  key.bytes[0] = (unsigned char)word;
  key.bytes[1] = (unsigned char)(word >> 8);
  key.bytes[2] = (unsigned char)(word >> 16);
  key.bytes[3] = (unsigned char)(word >> 24);
  return PMurHash32(0, key.bytes, 4);
}

#ifdef __CPROVER__
extern uint32_t nondet_uint(void);
int main(void) {
  uint32_t a = nondet_uint(), b = nondet_uint();
  __CPROVER_assume(a < b);
  uint32_t ha = hash_word(a), hb = hash_word(b);
  assert((ha & 1023U) != (hb & 1023U));
  return 0;
}
#else
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
int main(int argc, char **argv) {
  if (argc != 3) return 2;
  char *end;
  errno = 0;
  unsigned long a = strtoul(argv[1], &end, 10);
  if (errno || end == argv[1] || *end || a > UINT32_MAX) return 2;
  unsigned long b = strtoul(argv[2], &end, 10);
  if (errno || end == argv[2] || *end || b > UINT32_MAX) return 2;
  uint32_t ha = hash_word((uint32_t)a), hb = hash_word((uint32_t)b);
  printf("hashes %08x %08x, common low 10 bits %03x\n", ha, hb, ha & 1023U);
  return a == b || (ha & 1023U) != (hb & 1023U);
}
#endif
