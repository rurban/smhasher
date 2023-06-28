// DISCoHAsH v2 - also known as BEBB4185
// https://github.com/dosyago/discohash
//   Copyright 2023 Cris Stringfellow & The Dosyago Corporation
//
//   Licensed under the Apache License, Version 2.0 (the "License");
//   you may not use this file except in compliance with the License.
//   You may obtain a copy of the License at
//
//       http://www.apache.org/licenses/LICENSE-2.0
#include <cstdio>
#include <inttypes.h>
#include <cstring>
#include <iostream>
#include "discohash.h"

constexpr int STATE = 32;
constexpr int STATEM = STATE-1;
constexpr int STATE64 = STATE >> 3;
constexpr int HSTATE64 = STATE64 >> 1;
constexpr int STATE64M = STATE64-1;
constexpr int HSTATE64M = HSTATE64-1;
alignas(uint64_t) uint8_t disco_buf[STATE] = {0};
constexpr uint64_t P = 0xFFFFFFFFFFFFFFFF - 58;
constexpr uint64_t Q = UINT64_C(13166748625691186689);
alignas(uint64_t) uint8_t *ds8 = (uint8_t *)disco_buf;
uint64_t *ds = (uint64_t *)disco_buf;

static inline uint64_t
rot (uint64_t v, int n)
{
  n = n & 63U;
  if (n)
    v = (v >> n) | (v << (64 - n));
  return v;
}

static inline uint8_t
rot8 (uint8_t v, int n)
{
  n = n & 7U;
  if (n)
    v = (v >> n) | (v << (8 - n));
  return v;
}

static inline void
mix (const int A)
{
  const int B = A + 1;
  ds[A] *= P;
  ds[A] = rot (ds[A], 23);
  ds[A] *= Q;

  ds[B] ^= ds[A];

  ds[B] *= P;
  ds[B] = rot (ds[B], 23);
  ds[B] *= Q;
}

static inline void
round (const uint64_t *m64, const uint8_t *m8, int len)
{
  int index = 0;
  int sindex = 0;
  uint64_t counter = UINT64_C (0xfaccadaccad09997);
  uint8_t counter8 = 137;

  for (int Len = len >> 3; index < Len; index++)
    {
      ds[sindex] += rot (m64[index] + index + counter + 1, 23);
      counter += ~m64[index] + 1;
      switch (sindex)
        {
        case 1:
          mix (0);
          sindex++;
          break;
        case 3:
          mix (2);
          sindex = 0;
          break;
        default:
          sindex++;
          break;
        }
    }

  mix (1);

  index <<= 3;
  sindex = index & (STATEM);
  for (; index < len; index++)
    {
      ds8[sindex] += rot8 (m8[index] + index + counter8 + 1, 23);
      counter8 += ~m8[index] + 1;
      mix (index % STATE64M);
      if (sindex >= STATEM)
        {
          sindex = 0;
        }
      else
        {
          sindex++;
        }
    }

  mix (0);
  mix (1);
  mix (2);
}

void
DISCoHAsH_64 (const void *key, int len, unsigned seed, void *out)
{
  int tempLen = len;
  if (tempLen == 0)
    {
      tempLen = 1;
    }
  alignas (uint64_t) uint8_t *tempBuf = new uint8_t[tempLen];
  const uint8_t *key8Arr = (uint8_t *)key;

  alignas (uint64_t) const uint8_t seedbuf[16] = { 0 };
  const uint8_t *seed8Arr = (uint8_t *)seedbuf;
  const uint64_t *seed64Arr = (uint64_t *)seedbuf;
  uint32_t *seed32Arr = (uint32_t *)seedbuf;

  // the cali number from the Matrix (1999)
  seed32Arr[0] = UINT32_C (0xc5550690);
  seed32Arr[0] -= seed;
  seed32Arr[1] = 1 + seed;
  seed32Arr[2] = ~(1 - seed);
  seed32Arr[3] = (1 + seed) * UINT32_C (0xf00dacca);

  // nothing up my sleeve
  ds[0] = UINT64_C (0x123456789abcdef0);
  ds[1] = UINT64_C (0x0fedcba987654321);
  ds[2] = UINT64_C (0xaccadacca80081e5);
  ds[3] = UINT64_C (0xf00baaf00f00baaa);

  memcpy (tempBuf, key, len);
  uint64_t *temp64 = reinterpret_cast<uint64_t *> (tempBuf);

  round (temp64, key8Arr, len);
  round (seed64Arr, seed8Arr, 16);
  round (ds, ds8, STATE);

  // 256-bit internal state 64-bit output
  uint64_t h[1] = { 0 };

  h[0] -= ds[2];
  h[0] -= ds[3];

  memcpy (out, h, sizeof (h)); // divide by 4 as smhasher only uses 64 bits

  delete[] tempBuf;
}
