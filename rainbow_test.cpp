#include <string.h>
#include <stdint.h>
typedef uint32_t seed_t;

static inline uint64_t GET_U64(const uint8_t* data, size_t index) {
  uint64_t result;
  memcpy(&result, data + index, sizeof(result));
#ifdef BIG_ENDIAN
  result = ((result & 0x00000000000000FF) << 56) |
           ((result & 0x000000000000FF00) << 40) |
           ((result & 0x0000000000FF0000) << 24) |
           ((result & 0x00000000FF000000) << 8) |
           ((result & 0x000000FF00000000) >> 8) |
           ((result & 0x0000FF0000000000) >> 24) |
           ((result & 0x00FF000000000000) >> 40) |
           ((result & 0xFF00000000000000) >> 56);
#endif
  return result;
}

static inline void PUT_U64(uint64_t value, uint8_t* data, size_t index) {
#ifdef BIG_ENDIAN
  value = ((value & 0x00000000000000FF) << 56) |
          ((value & 0x000000000000FF00) << 40) |
          ((value & 0x0000000000FF0000) << 24) |
          ((value & 0x00000000FF000000) << 8) |
          ((value & 0x000000FF00000000) >> 8) |
          ((value & 0x0000FF0000000000) >> 24) |
          ((value & 0x00FF000000000000) >> 40) |
          ((value & 0xFF00000000000000) >> 56);
#endif
  memcpy(data + index, &value, sizeof(value));
}

#include "rainbow.hpp"

//objsize: 4f7da0 - 4f811c: 892
void rainbow64_test ( const void *key, int len, uint32_t seed, void *out) {
  rainbow<64>(key, (size_t)len, seed, out);
}
//objsize: 4f8120 - 4f84c9: 937
void rainbow128_test ( const void *key, int len, uint32_t seed, void *out) {
  rainbow<128>(key, (size_t)len, seed, out);
}
//objsize: 4f84d0 - 4f89af: 1247
void rainbow256_test ( const void *key, int len, uint32_t seed, void *out) {
    rainbow<256>(key, (size_t)len, seed, out);
}
