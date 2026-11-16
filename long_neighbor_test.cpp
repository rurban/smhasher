#include "LongNeighborTest.h"
extern "C" {
#include "blake3/blake3.h"
}
#include <string.h>
void SpookyV2_128_test(const void *key, int len, uint32_t seed, void *out);

static void strong_hash(const void *key, int len, uint32_t, void *out) {
  blake3_hasher h;
  blake3_hasher_init(&h);
  blake3_hasher_update(&h, key, (size_t)len);
  blake3_hasher_finalize(&h, (uint8_t *)out, 16);
}

static void ignored_bit(const void *key, int len, uint32_t seed, void *out) {
  std::vector<uint8_t> copy((const uint8_t *)key, (const uint8_t *)key + len);
  if (len) copy[0] &= 0xfe;
  strong_hash(copy.data(), len, seed, out);
}

static void ignored_zero_suffix(const void *key, int len, uint32_t seed, void *out) {
  const uint8_t *bytes = (const uint8_t *)key;
  while (len && bytes[len - 1] == 0) --len;
  strong_hash(key, len, seed, out);
}

static void forced_two_bit_collision(const void *key, int len, uint32_t, void *out) {
  std::vector<uint8_t> copy((const uint8_t *)key, (const uint8_t *)key + len);
  if (len == 4 && copy[0] == 0 && copy[1] == 0 && copy[2] == 0 && copy[3] == 3)
    copy[3] = 0;
  blake3_hasher h;
  blake3_hasher_init(&h);
  blake3_hasher_update(&h, copy.data(), copy.size());
  blake3_hasher_finalize(&h, (uint8_t *)out, 8);
}

int main() {
  const LongNeighborConfig cfg = {4, 4, 1, 16, 8};
  if (!LongNeighborTest<Blob<128> >(strong_hash, cfg, false)) return 1;
  if (LongNeighborTest<Blob<128> >(ignored_bit, cfg, false)) return 2;
  if (LongNeighborTest<Blob<128> >(ignored_zero_suffix, cfg, false)) return 3;
  if (LongNeighborTest<uint64_t>(forced_two_bit_collision, cfg, false)) return 5;
  const LongNeighborConfig spooky = {281, 281, 1, 2048, 160};
  if (LongNeighborTest<Blob<128> >(SpookyV2_128_test, spooky, false)) return 4;
  return 0;
}
