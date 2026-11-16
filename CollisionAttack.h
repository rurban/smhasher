#pragma once

#include "Types.h"
#include <chrono>
#include <stdint.h>
#include <stdio.h>

struct CollisionPrefixStats {
  uint64_t matches, first, second;
  double firstCollisionSeconds;
};

struct CollisionAttackResult {
  CollisionPrefixStats prefix[33];
  uint64_t attempts;
  double seconds;
};

// Keys are distinct 64-byte binary strings: a little-endian counter followed
// by zeroes. All prefix widths are measured in the same pass, against bucket 0.
static CollisionAttackResult CollisionAttack(pfHash hash, uint32_t seed, uint64_t attempts) {
  CollisionAttackResult result = {};
  result.attempts = attempts;
  alignas(16) uint8_t key[64] = {};
  alignas(16) uint8_t digest[64] = {};
  typedef std::chrono::steady_clock Clock;
  const Clock::time_point start = Clock::now();
  for (uint64_t i = 0; i < attempts; ++i) {
    for (unsigned j = 0; j < 8; ++j) key[j] = (uint8_t)(i >> (8 * j));
    digest[0] = digest[1] = digest[2] = digest[3] = 0;
    hash(key, sizeof(key), seed, digest);
    uint32_t prefix = (uint32_t)digest[0] | ((uint32_t)digest[1] << 8) |
                      ((uint32_t)digest[2] << 16) | ((uint32_t)digest[3] << 24);
    unsigned zeroBits = 0;
    while (zeroBits < 32 && !(prefix & 1)) { ++zeroBits; prefix >>= 1; }
    for (unsigned bits = 0; bits <= zeroBits; ++bits) {
      CollisionPrefixStats &stat = result.prefix[bits];
      if (++stat.matches == 1) stat.first = i;
      else if (stat.matches == 2) {
        stat.second = i;
        stat.firstCollisionSeconds = std::chrono::duration<double>(Clock::now() - start).count();
      }
    }
  }
  result.seconds = std::chrono::duration<double>(Clock::now() - start).count();
  return result;
}

static void PrintCollisionAttack(const CollisionAttackResult &result, uint32_t seed) {
  printf("Known seed %u; %llu binary 64-byte keys tested in %.6f seconds\n",
         seed, (unsigned long long)result.attempts, result.seconds);
  printf("low bits | matches | collisions | seconds to first collision | first two counters\n");
  for (unsigned bits = 0; bits <= 32; ++bits) {
    const CollisionPrefixStats &s = result.prefix[bits];
    printf("%8u | %llu | %llu | ", bits, (unsigned long long)s.matches,
           (unsigned long long)(s.matches ? s.matches - 1 : 0));
    if (s.matches >= 2)
      printf("%.6f | %016llx %016llx\n", s.firstCollisionSeconds,
             (unsigned long long)s.first, (unsigned long long)s.second);
    else printf("- | -\n");
  }
}
