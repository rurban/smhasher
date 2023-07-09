// -*- c++ -*-
#pragma once
/*
 * Discohash (aka BEBB4185)
 * Copyright (c) 2020-2023 Cris Stringfellow
 * Copyright (C) 2021-2022  Frank J. T. Wojcik
 * Copyright (c) 2020-2021 Reini Urban
 *
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation
 * files (the "Software"), to deal in the Software without
 * restriction, including without limitation the rights to use, copy,
 * modify, merge, publish, distribute, sublicense, and/or sell copies
 * of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS
 * BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
 * ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */
#include <string>
#include <cstdint>
#include <cstdio>
#include <inttypes.h>
#include <cstring>
#include <iostream>
#include <vector>
#include <algorithm>
#include "discohash.h"

namespace discohash1 { 
  static const uint32_t STATE     = 32; // Must be divisible by 8
  static const uint32_t STATE64   = STATE >> 3;
  static const uint32_t STATEM    = STATE - 1;
  static const uint32_t HSTATE64M = (STATE64 >> 1) - 1;
  static const uint32_t STATE64M  = STATE64 - 1;
  static const uint64_t P         = UINT64_C(  0xFFFFFFFFFFFFFFFF) - 58;
  static const uint64_t Q         = UINT64_C(13166748625691186689);

  static inline uint64_t GET_U64(const uint8_t * b, const uint32_t i) {
      uint64_t n;
      memcpy(&n, &b[i], 8);
      return n;
  }

  static inline void PUT_U64(uint64_t n, uint8_t * b, const uint32_t i) {
      memcpy(&b[i], &n, 8);
  }

  //--------
  // State mix function
  static inline uint8_t ROTR8( uint8_t v, int n ) {
      n = n & 7U;
      if (n) {
          v = (v >> n) | (v << (8 - n));
      }
      return v;
  }

  static inline uint64_t ROTR64( uint64_t v, int n ) {
      n = n & 63U;
      if (n) {
          v = (v >> n) | (v << (64 - n));
      }
      return v;
  }

  static inline void mix( uint64_t * ds, const uint32_t A ) {
      const uint32_t B = A + 1;

      ds[A] *= P;
      ds[A]  = ROTR64(ds[A], 23);
      ds[A] *= Q;
      // ds[A] = ROTR64(ds[A], 23);

      ds[B] ^= ds[A];

      ds[B] *= P;
      ds[B]  = ROTR64(ds[B], 23);
      ds[B] *= Q;
      // ds[B] = ROTR64(ds[B], 23);
  }

  //---------
  // Hash round function

  // The reread parameter is needed because sometimes the same array is
  // read-from and written-to via different pointers (m8 and ds), but it
  // usually isn't. This lets those cases avoid a possible bswap().
  //
  template <bool reread>
  static inline void round( uint64_t * ds, const uint8_t * m8, uint32_t len ) {
      uint32_t index;
      uint32_t sindex   = 0;
      uint32_t Len      = len >> 3;
      uint64_t counter  = UINT64_C(0xfaccadaccad09997);
      uint8_t  counter8 = 137;

      for (index = 0; index < Len; index++) {
          uint64_t blk = GET_U64(m8, index * 8);
          ds[sindex] += ROTR64(blk + index + counter + 1, 23);
          if (reread) { blk = GET_U64(m8, index * 8); }
          counter    += ~blk + 1;
          if (sindex == HSTATE64M) {
              mix(ds, 0);
          } else if (sindex == STATE64M) {
              mix(ds, 2);
              sindex = -1;
          }
          sindex++;
      }

      mix(ds, 1);

      Len = index << 3;
      sindex = Len & (STATEM);

      for (index = Len; index < len; index++) {
          ((uint8_t *)ds)[false ? (sindex ^ 7) : sindex] += ROTR8(m8[index] + index + counter8 + 1, 23);
          // I also wonder if this was intended to be m8[index], to
          // mirror the primary 8-byte loop above...
          //
          // Regardless, m8[sindex] can never read past EOB here, which
          // is the important thing. This is because the maximum value
          // of sindex is (len & ~7) 
          counter8 += ~m8[sindex] + 1;
          mix(ds, index % STATE64M);
          if (sindex >= STATEM) {
              sindex = -1;
          }
          sindex++;
      }

      mix(ds, 0);
      mix(ds, 1);
      mix(ds, 2);
  }

  //---------
  // main hash function

  template <uint32_t hashsize>
  void DISCoHAsH( const void * in, int len, const seed_t seed, void * out ) {
      const uint8_t * key8Arr = (const uint8_t *)in;
      uint8_t *       out8    = (uint8_t *      )out;
      uint32_t        seedbuf[4];

      if (len >= UINT32_C(0xffffffff)) { return; }

      // the cali number from the Matrix (1999)
      uint32_t seed32 = seed;
      if (!false) {
          seedbuf[0]  = 0xc5550690;
          seedbuf[0] -= seed32;
          seedbuf[1]  =   1 + seed32;
          seedbuf[2]  = ~(1 - seed32);
          seedbuf[3]  =  (1 + seed32) * 0xf00dacca;
      } else {
          seedbuf[1]  = 0xc5550690;
          seedbuf[1] -= seed32;
          seedbuf[0]  =   1 + seed32;
          seedbuf[3]  = ~(1 - seed32);
          seedbuf[2]  =  (1 + seed32) * 0xf00dacca;
      }

      uint64_t ds[STATE / 8];
      // nothing up my sleeve
      ds[0] = UINT64_C(0x123456789abcdef0);
      ds[1] = UINT64_C(0x0fedcba987654321);
      ds[2] = UINT64_C(0xaccadacca80081e5);
      ds[3] = UINT64_C(0xf00baaf00f00baaa);

      // The mixing in of the seed array does not need bswap set, since
      // the if() above will order the bytes correctly for that
      // variable. The mixing of the state with itself also doesn't need
      // bswap set, because the endianness of the data will naturally
      // always match the endianness of the ds[] values.
      round<false>(ds, key8Arr           , (uint32_t)len);
      round<false>(ds, (uint8_t *)seedbuf,            16);
      round< true>(ds, (uint8_t *)ds     , STATE        );

      uint64_t h[STATE64] = { 0 };

      h[0]  = ds[2];
      h[1]  = ds[3];

      h[0] += h [1];

      if (hashsize == 128) {
          round< true>(ds, (uint8_t *)ds     , STATE        );
          h[2]  = ds[2];
          h[3]  = ds[3];

          h[2] ^= h [3];
          PUT_U64(h[2], out8, 8);
      }
      if (hashsize >= 64) {
          PUT_U64(h[0], out8, 0);
      }
  }
}
