#include <stdint.h>

static uint64_t inline U8TO64_LE(const unsigned char *p) {
  return *(const uint64_t *)p;
}

static uint64_t rotl64(uint64_t x, int k) {
  return (((x) << (k)) | ((x) >> (64 - k)));
}

uint64_t tsip(const unsigned char *seed, const unsigned char *m, uint64_t len) {

  uint64_t v0, v1;
  uint64_t mi, k0, k1;
  uint64_t last7;

  k0 = U8TO64_LE(seed);
  k1 = U8TO64_LE(seed + 8);

  v0 = k0 ^ 0x736f6d6570736575ull;
  v1 = k1 ^ 0x646f72616e646f6dull;

  last7 = (uint64_t)(len & 0xff) << 56;

#define sipcompress()                                                          \
  do {                                                                         \
    v0 += v1;                                                                  \
    v1 = rotl64(v1, 13) ^ v0;                                                  \
    v0 = rotl64(v0, 35) + v1;                                                  \
    v1 = rotl64(v1, 17) ^ v0;                                                  \
    v0 = rotl64(v0, 21);                                                       \
  } while (0)

  const unsigned char *end = m + (len & ~7);

  while (m < end) {
    mi = U8TO64_LE(m);
    v1 ^= mi;
    sipcompress();
    v0 ^= mi;
    m += 8;
  }

  switch (len & 7) {
  case 7:
    last7 |= (uint64_t)m[6] << 48;
  case 6:
    last7 |= (uint64_t)m[5] << 40;
  case 5:
    last7 |= (uint64_t)m[4] << 32;
  case 4:
    last7 |= (uint64_t)m[3] << 24;
  case 3:
    last7 |= (uint64_t)m[2] << 16;
  case 2:
    last7 |= (uint64_t)m[1] << 8;
  case 1:
    last7 |= (uint64_t)m[0];
  case 0:
  default:;
  };

  v1 ^= last7;
  sipcompress();
  v0 ^= last7;

  // finalization
  v1 ^= 0xff;
  sipcompress();
  v1 = rotl64(v1, 32);
  sipcompress();
  v1 = rotl64(v1, 32);

  return v0 ^ v1;
}
