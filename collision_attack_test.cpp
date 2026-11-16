#include "CollisionAttack.h"

static void multiply_hash(const void *data, int len, uint32_t seed, void *out) {
  const uint8_t *key = (const uint8_t *)data;
  uint32_t value = 0;
  if (len != 64) value = 0xffffffffU;
  else {
    for (int i = 63; i >= 4; --i) value |= key[i];
    for (int i = 3; i >= 0; --i) value = (value << 8) | key[i];
    value = value * UINT32_C(0x9e3779b1) ^ seed;
  }
  uint8_t *bytes = (uint8_t *)out;
  for (int i = 0; i < 4; ++i) bytes[i] = (uint8_t)(value >> (8 * i));
}

int main() {
  CollisionAttackResult result = CollisionAttack(multiply_hash, 0, 16);
  for (unsigned bits = 0; bits <= 32; ++bits) {
    uint64_t expected = bits <= 4 ? 16U >> bits : 1;
    const CollisionPrefixStats &s = result.prefix[bits];
    if (s.matches != expected || s.first != 0) return 1;
    if (expected >= 2 && s.second != (UINT64_C(1) << bits)) return 2;
  }
  result = CollisionAttack(multiply_hash, 1, 1);
  if (result.prefix[0].matches != 1 || result.prefix[1].matches != 0 ||
      result.prefix[32].matches != 0) return 3;
  return 0;
}
