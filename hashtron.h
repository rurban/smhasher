/*
 * Hashtron StringHash, adapted from neurlang/classifier hash/hash.go.
 * Licensed under the Apache License, Version 2.0; see hashtron-LICENSE.
 * This C++ adaptation decodes UTF-8 without allocating a rune array.
 */
#pragma once
#include <stdint.h>
#include <stddef.h>

static inline uint32_t hashtron_mix(uint32_t n, uint32_t salt)
{
  uint32_t m = n - salt;
  m ^= m << 2;
  m ^= m << 3;
  m ^= m >> 5;
  m ^= m >> 7;
  m ^= m << 11;
  m ^= m << 13;
  m ^= m >> 17;
  m ^= m << 19;
  m += salt;
  return (uint32_t)(((uint64_t)m * UINT32_MAX) >> 32);
}

static inline bool hashtron_continuation(uint8_t byte)
{
  return (byte & 0xc0U) == 0x80U;
}

// Go's range over a string replaces each invalid UTF-8 byte with U+FFFD.
static inline uint32_t hashtron_string(const void *key, size_t len, uint32_t state)
{
  const uint8_t *p = (const uint8_t *)key;
  while (len) {
    uint32_t rune = 0xfffd;
    size_t n = 1;
    uint8_t b = p[0];
    if (b < 0x80) {
      rune = b;
    } else if (b >= 0xc2 && b <= 0xdf && len >= 2 && hashtron_continuation(p[1])) {
      rune = ((b & 0x1fU) << 6) | (p[1] & 0x3fU);
      n = 2;
    } else if (b >= 0xe0 && b <= 0xef && len >= 3 &&
               hashtron_continuation(p[1]) && hashtron_continuation(p[2]) &&
               (b != 0xe0 || p[1] >= 0xa0) && (b != 0xed || p[1] < 0xa0)) {
      rune = ((b & 0x0fU) << 12) | ((p[1] & 0x3fU) << 6) | (p[2] & 0x3fU);
      n = 3;
    } else if (b >= 0xf0 && b <= 0xf4 && len >= 4 &&
               hashtron_continuation(p[1]) && hashtron_continuation(p[2]) &&
               hashtron_continuation(p[3]) && (b != 0xf0 || p[1] >= 0x90) &&
               (b != 0xf4 || p[1] < 0x90)) {
      rune = ((b & 7U) << 18) | ((p[1] & 0x3fU) << 12) |
             ((p[2] & 0x3fU) << 6) | (p[3] & 0x3fU);
      n = 4;
    }
    state = hashtron_mix(state, rune);
    p += n;
    len -= n;
  }
  return state;
}
