#include <cstdio>
#include <inttypes.h>
#include "beamsplitter.h"

#if defined(_MSC_VER)

#define FORCE_INLINE	__forceinline

// Other compilers

#else	// defined(_MSC_VER)

#define	FORCE_INLINE inline __attribute__((always_inline))

#endif // !defined(_MSC_VER)

const int STATE = 32;

  //--------
  // State mix function

    FORCE_INLINE uint64_t rot( uint64_t v, int n) 
    {
			n = n & 63U;
			if (n)
					v = (v >> n) | (v << (64-n));
			return v; 
    }

    FORCE_INLINE uint64_t rot8( uint8_t v, int n) 
    {
			n = n & 7U;
			if (n)
					v = (v >> n) | (v << (8-n));
			return v; 
    }

    FORCE_INLINE void mix( uint64_t * state, const uint64_t box[1024] )
    {
      const int iv = state[0] & 1023;
      const int M = T[iv];
      state[1] += M + state[0];

      state[0] ^= state[1];
      state[1] ^= state[0];
      state[0] ^= state[1];

      state[1] = rot(state[1], state[0]);
    }

  //---------
  // Hash round function 

    FORCE_INLINE void round( const uint64_t * m64, const uint8_t * m8, int len, 
            uint64_t * state64, uint8_t * state8 )
    {
      int index = 0;
      int sindex = 0;

      for( int Len = len >> 3; index < Len; index++) {
        state64[sindex] += rot(m64[index] + index + 1, state64[sindex] +index +1);
        if ( sindex == 1 ) {
          mix(state64, T);
        } else if ( sindex == 3 ) {
          mix(state64+2, T);
          sindex = -1;
        }
        sindex++;
      }

      mix(state64, T);

      index <<= 3;
      sindex = index&31;
      for( ; index < len; index++) {
        state8[sindex] += rot8(m8[index] + index + 1, state8[sindex] + index+1);
        // state64+[0,1,2]
        mix(state64+(index%3), T);
        if ( sindex >= 31 ) {
          sindex = -1;
        }
        sindex++;
      }

      mix(state64, T);
      mix(state64+1, T);
      mix(state64+2, T);
    }

  //---------
  // main hash function 

    void beamsplitter_64 ( const void * key, int len, unsigned seed, void * out )
    {
      const uint8_t *key8Arr = (uint8_t *)key;
      const uint64_t *key64Arr = (uint64_t *)key;

      const uint8_t seedbuf[8] = {0};
      uint64_t *seed64Arr = (uint64_t *)seedbuf;
      uint32_t *seed32Arr = (uint32_t *)seedbuf;
      const uint8_t *seed8Arr = (uint8_t *)seedbuf;

      const uint8_t buf[STATE] = {255};
      uint8_t *state8 = (uint8_t *)buf;
      uint32_t *state32 = (uint32_t *)buf;
      uint64_t *state = (uint64_t *)buf;

      seed32Arr[0] -= seed;
      seed32Arr[1] = ~(1 - seed);

      state[0] = 0x123456789abcdef0;
      state[1] = 0x0fedcba987654321;
      state[2] = 0xaccadacca80081e5;
      state[3] = 0xf00baaf00f00baaa;

      round( key64Arr, key8Arr, len, state, state8 );
      round( key64Arr, key8Arr, len, state, state8 );
      round( key64Arr, key8Arr, len, state, state8 );
      round( key64Arr, key8Arr, len, state, state8 );
      round( key64Arr, key8Arr, len, state, state8 );
      round( seed64Arr, seed8Arr, 8, state, state8 );
      round( seed64Arr, seed8Arr, 8, state, state8 );
      round( state, state8, STATE, state, state8 );
      round( state, state8, STATE, state, state8 );
      round( state, state8, STATE, state, state8 );



      /*
      //printf("state = %#018" PRIx64 " %#018" PRIx64 " %#018" PRIx64 " %#018" PRIx64 "\n",
      //  state[0], state[1], state[2], state[3] );
      */

      //printf("state = %#018" PRIx64 " %#018" PRIx64 "\n",
      //  state[0], state[1] );

      const uint8_t output[STATE] = {0};
      uint64_t *h = (uint64_t *)output;

      // The new combination step
      h[0] = state[2];
      h[1] = state[3];

      h[0] += h[1];

      ((uint64_t *)out)[0] = h[0];
    }