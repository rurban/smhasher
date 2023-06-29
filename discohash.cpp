// also known as BEBB4185
// Copyright 2020 Cris Stringfellow
// https://github.com/dosyago/discohash
#include <cstdio>
#include <inttypes.h>
#include <cstring>
#include "discohash.h"

constexpr int STATE = 32;
constexpr int STATE64 = STATE >> 3;
constexpr int STATEM = STATE-1;
constexpr int HSTATE64M = (STATE64 >> 1)-1;
constexpr int STATE64M = STATE64-1;
alignas(uint64_t) uint8_t disco_buf[STATE] = {0};
constexpr uint64_t P = 0xFFFFFFFFFFFFFFFF - 58;
constexpr uint64_t Q = 13166748625691186689U;
alignas(uint64_t) uint8_t *ds8 = (uint8_t *)disco_buf;
uint64_t *ds = (uint64_t *)disco_buf;

  //--------
  // State mix function

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
      ds[A] *= P;
      ds[A] = rot(ds[A], 23);
      ds[A] *= Q;
      
      ds[B] ^= ds[A];

      ds[B] *= P;
      ds[B] = rot(ds[B], 23);
      ds[B] *= Q;
    }

  //---------
  // Hash round function 

    static inline void round( const uint64_t * m64, const uint8_t * m8, int len )
    {
      int index = 0;
      int sindex = 0;
      uint64_t counter = 0xfaccadaccad09997;
      uint8_t counter8 = 137;

      for( int Len = len >> 3; index < Len; index++) {
        ds[sindex] += rot(m64[index] + index + counter + 1, 23);
        counter += ~m64[index] + 1;
        if ( sindex == HSTATE64M ) {
          mix(0);
          sindex++;
        } else if ( sindex == STATE64M ) {
          mix(2);
          sindex = 0;
        } else {
          sindex++;
        }
      }

      mix(1);

      index <<= 3;
      sindex = index&(STATEM);
      for( ; index < len; index++) {
        ds8[sindex] += rot8(m8[index] + index + counter8 + 1, 23);
        counter8 += ~m8[sindex] + 1;
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

      if ( STATE == 64 ) {
        mix(3);
        mix(4);
        mix(5);
        mix(6);
      }
    }

  //---------
  // main hash function 

    void DISCoHAsH_64 ( const void * key, int len, unsigned seed, void * out )
    {
      int tempLen = len;
      if ( tempLen == 0 ) {
        tempLen = 1;
      }
      alignas(uint64_t) uint8_t* tempBuf = new uint8_t[tempLen];
      const uint8_t *key8Arr = (uint8_t *)key;

      alignas(uint64_t) const uint8_t seedbuf[16] = {0};
      const uint8_t *seed8Arr = (uint8_t *)seedbuf;
      const uint64_t *seed64Arr = (uint64_t *)seedbuf;
      uint32_t *seed32Arr = (uint32_t *)seedbuf;

      // the cali number from the Matrix (1999)
      seed32Arr[0] = 0xc5550690;
      seed32Arr[0] -= seed;
      seed32Arr[1] = 1 + seed;
      seed32Arr[2] = ~(1 - seed);
      seed32Arr[3] = (1+seed) * 0xf00dacca;

      // nothing up my sleeve
      ds[0] = 0x123456789abcdef0;
      ds[1] = 0x0fedcba987654321;
      ds[2] = 0xaccadacca80081e5;
      ds[3] = 0xf00baaf00f00baaa;
      if ( STATE == 64 ) {
        ds[4] = 0xbeefdeadbeefc0de;
        ds[5] = 0xabad1deafaced00d;
        ds[6] = 0xfaceb00cfacec0de;
        ds[7] = 0xdeadc0dedeadbeef;
      }

      memcpy(tempBuf, key, len);
      uint64_t* temp64 = reinterpret_cast<uint64_t*>(tempBuf);

      round( temp64, key8Arr, len );
      round( seed64Arr, seed8Arr, 16 );
      round( ds, ds8, STATE   );


      // 512-bit internal state 256-bit output
      uint64_t h[4] = {0}; // This will hold the final 256-bit output

      if ( STATE == 64 ) {
        h[0] -= ds[0];
        h[0] -= ds[4];

        h[1] = ds[1] ^ ds[5];

        h[2] -= ds[2];
        h[2] -= ds[6];

        h[3] = ds[3] ^ ds[7];
      } else {
        // 256-bit internal state
        uint64_t h[4] = {0, ds[3] ^ ds[2], 0, ds[1] ^ ds[0]}; // full 256-bit output 

        // subtraction here (discohash v2) more readily mixes the bits than addition, 
        // achieves non-compressibility (using gzip -9) and maintains the statistical properties
        // (smhasher quality tests) of discohash v1
        h[0] -= ds[2];
        h[0] -= ds[3];
        h[2] -= ds[0];
        h[2] -= ds[1];
      }

      /*
        // 64-bit output
        uint64_t h[1] = {0};
        h[0] -= ds[2];
        h[0] -= ds[3];
      */

      // how to make a cryptographic output? A finalization that only outputs half the state is one start 
      // as it stands the current finalization can fairly easily recover 
      // a hash-sized amount of the internal state in 128-bit and larger output
      // I don't have good ideas now for that, as making the state twice as large (8 64-bit words) will require some modification 
      // and it will be slower. Plus, adding multiplication instead of subtraction, for finalization ruins the statistical properties.
      // I think for now I focus on keeping it a great non-cryptographic hash, and try to expand it into a cryptohash later.
      // Maybe I need to invent a new hash, or maybe I can make this one faster somehow.
      // One idea is maybe I can take ds[0] * ds[1] mod P, which relies on factorization in a finite field being hard.

      memcpy(out, h, sizeof(h)/4); 

      delete[] tempBuf;
    }
