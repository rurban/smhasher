// DISCoHAsH 512 - crypto hash 
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

constexpr int STATE = 64;
constexpr int STATEM = STATE-1;
constexpr int STATE64 = STATE >> 3;
constexpr int HSTATE64 = STATE64 >> 1;
constexpr int STATE64M = STATE64-1;
constexpr int HSTATE64M = HSTATE64-1;
alignas(uint64_t) uint8_t disco_c_buf[STATE] = {0};
constexpr uint64_t P = 0xFFFFFFFFFFFFFFFF - 58;
constexpr uint64_t Q = UINT64_C(13166748625691186689);
alignas(uint64_t) uint8_t *dcs8 = (uint8_t *)disco_c_buf;
uint64_t *dcs = (uint64_t *)disco_c_buf;

  static inline uint64_t rot( uint64_t v, int n) 
  {
    n = n & 63U;
    if (n)
        v = (v >> n) | (v << (64-n));
    return v; 
  }

  static inline uint8_t rot8( uint8_t v, int n) 
  {
    n = n & 7U;
    if (n)
        v = (v >> n) | (v << (8-n));
    return v; 
  }

  static inline void mix(const int A)
  {
    const int B = A+1;
    dcs[A] *= P;
    dcs[A] = rot(dcs[A], 23);
    dcs[A] *= Q;
    
    dcs[B] ^= dcs[A];

    dcs[B] *= P;
    dcs[B] = rot(dcs[B], 23);
    dcs[B] *= Q;
  }

  static inline void round( const uint64_t * m64, const uint8_t * m8, int len )
  {
    int index = 0;
    int sindex = 0;
    uint64_t counter = 0xfaccadaccad09997;
    uint8_t counter8 = 137;

    for( int Len = len >> 3; index < Len; index++) {
      dcs[sindex] += rot(m64[index] + index + counter + 1, 23);
      counter += ~m64[index] + 1;
      switch(sindex) {
        case 1:
          mix(0);
          sindex++;
          break;
        case 3:
          mix(2);
          sindex++;
          break;
        case 5: 
          mix(4);
          sindex++;
          break;
        case 7:
          mix(6);
          sindex = 0;
          break;
        default:
          sindex++;
          break;
      }
    }

    mix(1);
    mix(3);
    mix(5);

    index <<= 3;
    sindex = index&(STATEM);
    for( ; index < len; index++) {
      dcs8[sindex] += rot8(m8[index] + index + counter8 + 1, 23);
      counter8 += ~m8[index] + 1;
      mix(index%STATE64M);
      if ( sindex >= STATEM ) {
        sindex = 0;
      } else {
        sindex++;
      }
    }

    mix(0);
    mix(1);
    mix(2);
    mix(3);
    mix(4);
    mix(5);
    mix(6);
  }

  void DISCoHAsH_512_64 ( const void * key, int len, unsigned seed, void * out )
  {
    int tempLen = len;
    if ( tempLen == 0 ) {
      tempLen = 1;
    }
    alignas(uint64_t) uint8_t* tempBuf = new uint8_t[tempLen];
    const uint8_t *key8Arr = (uint8_t *)key;

    // nothing up my sleeve
    dcs[0] = UINT64_C(0x123456789abcdef0) + seed;
    dcs[1] = UINT64_C(0x0fedcba987654321) - seed;
    dcs[2] = UINT64_C(0xaccadacca80081e5) ^ seed;
    dcs[3] = UINT64_C(0xf00baaf00f00baaa) - seed;
    dcs[4] = UINT64_C(0xbeefdeadbeefc0de) + seed;
    dcs[5] = UINT64_C(0xabad1deafaced00d) - seed;
    dcs[6] = UINT64_C(0xfaceb00cfacec0de) ^ seed;
    dcs[7] = UINT64_C(0xdeadc0dedeadbeef) - seed;

    memcpy(tempBuf, key, len);
    uint64_t* temp64 = reinterpret_cast<uint64_t*>(tempBuf);

    round( temp64, key8Arr, len );
    round( dcs, dcs8, STATE );

    // 512-bit internal state 256-bit output
    uint64_t h[4] = {0}; // This will hold the final output

    h[0] -= dcs[2];
    h[0] -= dcs[3];
    h[0] -= dcs[6];
    h[0] -= dcs[7];

    // full 256-bit output
    /*
      h[1] = dcs[2] ^ dcs[3] ^ dcs[6] ^ dcs[7];

      h[2] -= dcs[0];
      h[2] -= dcs[1];
      h[2] -= dcs[4];
      h[2] -= dcs[5];

      h[3] = dcs[0] ^ dcs[1] ^ dcs[4] ^ dcs[5];
    */

    memcpy(out, h, sizeof(h)/4);  // divide by 4 as smhasher only uses 64 bits

    delete[] tempBuf;
  }
