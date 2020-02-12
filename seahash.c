#include <stdint.h>
#include <string.h>
#include <assert.h>

#if defined(_MSC_VER)
#  define sea_swap64 _byteswap_uint64
#elif XXH_GCC_VERSION >= 403
#  define sea_swap64 __builtin_bswap64
#else
static uint64_t sea_swap64 (uint64_t x)
{
    return  ((x << 56) & 0xff00000000000000ULL) |
            ((x << 40) & 0x00ff000000000000ULL) |
            ((x << 24) & 0x0000ff0000000000ULL) |
            ((x << 8)  & 0x000000ff00000000ULL) |
            ((x >> 8)  & 0x00000000ff000000ULL) |
            ((x >> 24) & 0x0000000000ff0000ULL) |
            ((x >> 40) & 0x000000000000ff00ULL) |
            ((x >> 56) & 0x00000000000000ffULL);
}
#endif

static inline uint64_t diffuse(uint64_t val)
{
  uint64_t a, b;
  val *= 0x6eed0e9da4d94a4fULL;
  a = val >> 32;
  b = val >> 60;
  val ^= a >> b;
  val *= 0x6eed0e9da4d94a4fULL;
  return val;
}

uint64_t seahash(const char *key, int len, uint64_t seed) {
  uint64_t a, b, c, d;
  uint64_t s = seed;
  uint64_t *p;
  uint8_t pad[8] = {0};
  const uint64_t orig_len = (uint64_t)len;

  a = 0x16f11fe89b0d677cULL ^ s;
  b = 0xb480a793d8e6c86cULL;
  c = 0x6fe2e5aaf078ebc9ULL;
  d = 0x14f994a4c5259381ULL;

  p = (uint64_t *)key;
  while (len >= 32) {
    a ^= *p++;
    b ^= *p++;
    c ^= *p++;
    d ^= *p++;
    a = diffuse(a);
    b = diffuse(b);
    c = diffuse(c);
    d = diffuse(d);
    len -= 32;
  }

  switch (len) {
    // illegal msvc 15 syntax
    case 25 ... 31:
      a ^= *p++;
      b ^= *p++;
      c ^= *p++;
      memcpy(pad, p, len - 24);
      d ^= *(uint64_t *)pad;
      a = diffuse(a);
      b = diffuse(b);
      c = diffuse(c);
      d = diffuse(d);
      break;
    case 24:
      a ^= *p++;
      b ^= *p++;
      c ^= *p++;
      a = diffuse(a);
      b = diffuse(b);
      c = diffuse(c);
      break;
    case 17 ... 23:
      a ^= *p++;
      b ^= *p++;
      memcpy(pad, p, len - 16);
      c ^= *(uint64_t *)pad;
      a = diffuse(a);
      b = diffuse(b);
      c = diffuse(c);
      break;
    case 16:
      a ^= *p++;
      b ^= *p++;
      a = diffuse(a);
      b = diffuse(b);
      break;
    case 9 ... 15:
      a ^= *p++;
      memcpy(pad, p, len - 8);
      b ^= *(uint64_t *)pad;
      a = diffuse(a);
      b = diffuse(b);
      break;
    case 8:
      a ^= *p++;
      a = diffuse(a);
      break;
    case 1 ... 7:
      memcpy(pad, p, len);
      a ^= *(uint64_t *)pad;
      a = diffuse(a);
      break;
    case 0:
      break;
    default:
      assert(0);
  }

  a ^= b;
  c ^= d;
  a ^= c;
  a ^= orig_len;
  return sea_swap64(diffuse(a));
}
