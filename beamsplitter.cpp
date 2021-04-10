// Copyright 2020 Cris Stringfellow
// Licensed under GPL-3.0
// https://github.com/cris691/beamsplitter
#include <cstdio>
#include <inttypes.h>
#include "beamsplitter.h"

#if defined(_MSC_VER)

#define FORCE_INLINE  __forceinline

// Other compilers

#else // defined(_MSC_VER)

#define FORCE_INLINE inline __attribute__((always_inline))

#endif // !defined(_MSC_VER)

const int STATE = 32;
uint8_t buf[STATE] = {0};
uint64_t MASK = 0xffffffffffffff;
uint8_t *state8 = (uint8_t *)buf;
uint64_t *state = (uint64_t *)buf;

  //--------
  // State mix function

    FORCE_INLINE uint64_t rot( uint64_t v, int n) 
    {
      n = n & 63U;
      if (n)
          v = (v >> n) | (v << (64-n));
      return v; 
    }

    FORCE_INLINE uint8_t rot8( uint8_t v, int n) 
    {
      n = n & 7U;
      if (n)
          v = (v >> n) | (v << (8-n));
      return v; 
    }

    FORCE_INLINE void mix(const int A)
    {
      const int B = A+1;
      const int iv = state[A] & 1023;
      const uint64_t M = T[iv];
      state[B] += M + state[A];

      state[A] ^= state[B];
      state[B] ^= state[A];
      state[A] ^= state[B];

      state[B] = rot(state[B], state[A]);
    }

  //---------
  // Hash round function 

    FORCE_INLINE void round( const uint64_t * m64, const uint8_t * m8, int len )
    {
      int index = 0;
      int sindex = 0;

      for( int Len = len >> 3; index < Len; index++) {
        state[sindex] += rot(m64[index] + index + 1, state[sindex] +index +1);
        if ( sindex == 1 ) {
          mix(0);
        } else if ( sindex == 3 ) {
          mix(2);
          sindex = -1;
        }
        sindex++;
      }

      mix(0);

      index <<= 3;
      sindex = index&31;
      for( ; index < len; index++) {
        state8[sindex] += rot8(m8[index] + index + 1, state8[sindex] + index+1);
        // state+[0,1,2]
        mix(index%3);
        if ( sindex >= 31 ) {
          sindex = -1;
        }
        sindex++;
      }

      mix(0);
      mix(1);
      mix(2);
    }

  //---------
  // main hash function

  // Note that a large number of seeds lead to very bad quality, i.e. collisions
  // See some_bad_seeds[] and
  // https://github.com/rurban/smhasher/blob/master/doc/beamsplitter.txt

    void beamsplitter_64 ( const void * key, int len, unsigned seed, void * out )
    {
      const uint8_t *key8Arr = (uint8_t *)key;
      const uint64_t *key64Arr = (uint64_t *)key;

      uint8_t seedbuf[8] = {0};
      uint8_t *seed8Arr = (uint8_t *)seedbuf;
      uint64_t *seed64Arr = (uint64_t *)seedbuf;
      uint32_t *seed32Arr = (uint32_t *)seedbuf;

      // the cali number from the Matrix (1999)
      seed32Arr[0] = 0xc5550690;
      seed32Arr[0] -= seed;
      seed32Arr[1] = ~(1 - seed);

      // nothing up my sleeve
      state[0] = 0x123456789abcdef0;
      state[1] = 0x0fedcba987654321;
      state[2] = 0xaccadacca80081e5;
      state[3] = 0xf00baaf00f00baaa;

      round( key64Arr, key8Arr, len );
      round( key64Arr, key8Arr, len );
      round( key64Arr, key8Arr, len );
      round( seed64Arr, seed8Arr, 8 );
      //round( state, state8, STATE   );
      round( seed64Arr, seed8Arr, 8 );
      round( key64Arr, key8Arr, len );
      round( key64Arr, key8Arr, len );
      round( key64Arr, key8Arr, len );

      /*
      //printf("state = %#018" PRIx64 " %#018" PRIx64 " %#018" PRIx64 " %#018" PRIx64 "\n",
      //  state[0], state[1], state[2], state[3] );
      */

      //printf("state = %#018" PRIx64 " %#018" PRIx64 "\n",
      //  state[0], state[1] );

      uint8_t output[STATE] = {0};
      uint64_t *h = (uint64_t *)output;

      // The new combination step
      h[0] = state[2];
      h[1] = state[3];

      h[0] += h[1];

      ((uint64_t *)out)[0] = h[0];
    }
