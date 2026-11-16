// Independent implementation of the low-Hamming-distance collision method
// described in https://github.com/hmakholm/smhasher/blob/master/src/LongNeighborTest.md.
#pragma once

#include "Types.h"
#include "Random.h"
#include <algorithm>
#include <cmath>
#include <stdio.h>
#include <vector>

struct LongNeighborConfig {
  int minBytes, maxBytes, bases, pairBits, tripleBits;
};

struct LongNeighborDelta {
  uint16_t bit[3];
  uint8_t count, appended;
};

template <typename H> struct alignas(16) LongNeighborRecord {
  H hash;
  LongNeighborDelta delta;
};

template <typename H> struct LongNeighborLess {
  bool operator()(const LongNeighborRecord<H> &a, const LongNeighborRecord<H> &b) const {
    return (a.hash) < (b.hash);
  }
};

static inline void LongNeighborFlip(std::vector<uint8_t> &key, unsigned bit) {
  key[bit / 8] ^= (uint8_t)(1U << (bit % 8));
}
// Bound the chance of the same low-distance delta for any prefix of this
// base, with up to four possible zero-extension lengths.
static bool LongNeighborSevere(const LongNeighborDelta &a, const LongNeighborDelta &b,
                               int length) {
  unsigned distance = 0, first = (unsigned)(length * 8);
  for (unsigned side = 0; side < 2; ++side) {
    const LongNeighborDelta &x = side ? b : a, &y = side ? a : b;
    for (unsigned i = 0; i < x.count; ++i) {
      bool shared = false;
      for (unsigned j = 0; j < y.count; ++j) shared |= x.bit[i] == y.bit[j];
      if (!shared) { ++distance; first = std::min(first, (unsigned)x.bit[i]); }
    }
  }
  if (!distance && a.appended == b.appended) return false;
  const unsigned span = (unsigned)(length * 8) - first;
  double choices = 1;
  for (unsigned i = 1; i <= distance; ++i) choices *= (double)(span - i + 1) / i;
  const double chance = std::ldexp(choices * length * 4.0, -64);
  return chance < 1e-12;
}


static inline void LongNeighborPrintKey(const std::vector<uint8_t> &base, int length,
                                        const LongNeighborDelta &delta) {
  std::vector<uint8_t> key(base.begin(), base.begin() + length + delta.appended);
  for (unsigned i = 0; i < delta.count; ++i) LongNeighborFlip(key, delta.bit[i]);
  for (size_t i = 0; i < key.size(); ++i) printf("%02x", (unsigned)key[i]);
}

template <typename H>
static inline void LongNeighborAdd(pfHash hash, std::vector<uint8_t> &key, int length,
                                   LongNeighborDelta delta,
                                   std::vector<LongNeighborRecord<H> > &records) {
  LongNeighborRecord<H> record = {};
  record.delta = delta;
  hash(key.data(), length, 0, &record.hash);
  records.push_back(record);
}

static size_t LongNeighborVariantCount(int length, const LongNeighborConfig &cfg) {
  const int bits = length * 8;
  const int pairs = std::min(bits, cfg.pairBits);
  const int triples = std::min(bits, cfg.tripleBits);
  size_t count = 1 + (size_t)bits + (size_t)pairs * (pairs - 1) / 2;
  if (triples >= 3) count += (size_t)triples * (triples - 1) * (triples - 2) / 6;
  return count + 3 * (size_t)triples * (triples - 1) / 2;
}

template <typename H>
static bool LongNeighborBase(pfHash hash, std::vector<uint8_t> &key, int length,
                             const LongNeighborConfig &cfg, bool verbose,
                             size_t &variants, bool &severeCollision,
                             std::vector<LongNeighborRecord<H> > &records) {
  const int bits = length * 8;
  const int pairs = std::min(bits, cfg.pairBits);
  const int triples = std::min(bits, cfg.tripleBits);
  const int appendBits = triples;
  records.clear();
  LongNeighborDelta delta = {{0, 0, 0}, 0, 0};
  LongNeighborAdd(hash, key, length, delta, records);
  delta.count = 1;
  for (int i = 0; i < bits; ++i) {
    delta.bit[0] = (uint16_t)i;
    LongNeighborFlip(key, i); LongNeighborAdd(hash, key, length, delta, records); LongNeighborFlip(key, i);
  }
  delta.count = 2;
  for (int i = bits - pairs; i < bits; ++i) {
    delta.bit[0] = (uint16_t)i; LongNeighborFlip(key, i);
    for (int j = i + 1; j < bits; ++j) {
      delta.bit[1] = (uint16_t)j; LongNeighborFlip(key, j);
      LongNeighborAdd(hash, key, length, delta, records); LongNeighborFlip(key, j);
    }
    LongNeighborFlip(key, i);
  }
  delta.count = 3;
  for (int i = bits - triples; i < bits; ++i) {
    delta.bit[0] = (uint16_t)i; LongNeighborFlip(key, i);
    for (int j = i + 1; j < bits; ++j) {
      delta.bit[1] = (uint16_t)j; LongNeighborFlip(key, j);
      for (int k = j + 1; k < bits; ++k) {
        delta.bit[2] = (uint16_t)k; LongNeighborFlip(key, k);
        LongNeighborAdd(hash, key, length, delta, records); LongNeighborFlip(key, k);
      }
      LongNeighborFlip(key, j);
    }
    LongNeighborFlip(key, i);
  }
  delta.count = 2;
  for (int extra = 1; extra <= 3; ++extra) {
    delta.appended = (uint8_t)extra;
    for (int i = bits - appendBits; i < bits; ++i) {
      delta.bit[0] = (uint16_t)i; LongNeighborFlip(key, i);
      for (int j = i + 1; j < bits; ++j) {
        delta.bit[1] = (uint16_t)j; LongNeighborFlip(key, j);
        LongNeighborAdd(hash, key, length + extra, delta, records); LongNeighborFlip(key, j);
      }
      LongNeighborFlip(key, i);
    }
  }
  variants = records.size();
  if (sizeof(H) == 8)
    for (int i = 1; i <= bits; ++i)
      if (records[0].hash == records[i].hash) severeCollision = true;
  std::sort(records.begin(), records.end(), LongNeighborLess<H>());
  bool found = false;
  const bool rareTriple = sizeof(H) == 8 &&
      std::ldexp((double)variants * (variants - 1) * (variants - 2) / 6.0, -128) < 1e-12;
  for (size_t first = 0, end = 1; first < records.size(); first = end, ++end) {
    while (end < records.size() && records[first].hash == records[end].hash) ++end;
    if (end - first < 2) continue;
    const LongNeighborDelta &a = records[first].delta, &b = records[first + 1].delta;
    if (verbose && !found) {
      printf("  Collision at %d bytes: ", length);
      LongNeighborPrintKey(key, length, a); printf(" == ");
      LongNeighborPrintKey(key, length, b); printf("\n");
    }
    found = true;
    if (sizeof(H) == 8 &&
        ((end - first >= 3 && rareTriple) || LongNeighborSevere(a, b, length)))
      severeCollision = true;
    if (sizeof(H) != 8 || severeCollision) break;
  }
  return found;
}

template <typename H>
bool LongNeighborTest(pfHash hash, const LongNeighborConfig &cfg, bool verbose) {
  size_t bad = 0, tested = 0;
  double expected = 0;
  bool severeCollision = false;
  const int hashbits = (int)(sizeof(H) * 8);
  printf("LongNeighbors: lengths %d..%d bytes, %d bases per length, %d-bit hashes\n",
         cfg.minBytes, cfg.maxBytes, cfg.bases, hashbits);
  std::vector<uint8_t> key((size_t)cfg.maxBytes + 3);
  std::vector<LongNeighborRecord<H> > records;
  records.reserve(LongNeighborVariantCount(cfg.maxBytes, cfg));
  for (int len = cfg.minBytes; len <= cfg.maxBytes; ++len) {
    for (int base = 0; base < cfg.bases; ++base) {
      std::fill(key.begin(), key.begin() + len + 3, 0);
      if (base == 1) std::fill(key.begin(), key.begin() + len, 0xff);
      if (base >= 2) {
        Rand rng((uint32_t)(len * 131 + base));
        for (int i = 0; i < len; ++i) key[i] = (uint8_t)rng.rand_u32();
      }
      size_t n = 0;
      if (LongNeighborBase<H>(hash, key, len, cfg, verbose, n, severeCollision, records)) ++bad;
      expected += -std::expm1(-std::ldexp((double)n * (n - 1) / 2.0, -hashbits));
      ++tested;
    }
  }
  bool pass;
  if (hashbits >= 128) pass = bad == 0;
  else if (hashbits >= 64) pass = bad <= 1 && !severeCollision;
  else pass = bad <= std::max(1.0, expected * 1.3) && !(bad && expected < 1e-6);
  printf("LongNeighbors: %zu/%zu bases with collisions, %.4g expected: %s\n",
         bad, tested, expected, pass ? "PASS" : "FAIL");
  return pass;
}
