#include <cstdio>
#include <inttypes.h>
#include "beamsplitter.h"

#if defined(_MSC_VER)

#define FORCE_INLINE	__forceinline

// Other compilers

#else	// defined(_MSC_VER)

#define	FORCE_INLINE inline __attribute__((always_inline))

#endif // !defined(_MSC_VER)

  //---------
  // mix function : mix 128 bits of state to 128 bits of state 

    FORCE_INLINE void mix ( uint64_t * s64, uint8_t * s8, 
             const uint8_t box1[8][256], const uint8_t box2[8][256] )
    {
      uint8_t x[32] = {0};
      uint64_t * t = (uint64_t *)x;
      uint8_t * q = (uint8_t *)x;

      uint64_t counter = 0x0123456789abcdef;

      for( int i = 0; i < 8; i++ ) {
        q[i] = s8[i] + counter + i; 
        q[i] = box1[i][q[i]];
        counter += q[i];
      }

      for( int i = 8; i < 16; i++ ) {
        q[i] = s8[i] + q[i-5] + counter + i; 
        q[i] = box2[i-8][q[i]];
        counter += q[i];
      }

      for( int i = 16; i < 24; i++ ) {
        q[i] = s8[i] + q[i-9] + counter + i; 
        q[i] = box2[i-16][q[i]];
        counter += q[i];
      }

      for( int i = 24; i < 32; i++ ) {
        q[i] = s8[i] + q[i-17] + counter + i;  
        q[i] = box1[i-24][q[i]];
        counter += q[i];
      }

      s64[0] = t[2];
      s64[1] = t[3];
      s64[2] = t[0];
      s64[3] = t[1];
    }

  //---------
  // Hash round function 

    FORCE_INLINE void round( const uint64_t * m64, const uint8_t * m8, int len, 
            uint64_t * state64, uint8_t * state8 )
    {
      int index = 0;

      for( int Len = len >> 3; index < Len; index++ ) {
        state64[index&3] ^= (m64[index] + index);
        if ( index&3 == 3 ) {
          mix( state64, state8, S[0], S[1] );
        }
      }

      for( index <<= 3; index < len; index++ ) {
        state8[index&31] ^= (m8[index] + index);
        if ( index&15 == 15 ) {
          mix( state64, state8, S[1], S[2] );
        }
      }

      mix( state64, state8, S[2], S[3] );
      mix( state64, state8, S[3], S[0] );
    }

  //---------
  // main hash function 

    void beamsplitter_64 ( const void * key, int len, unsigned seed, void * out )
    {
      const uint8_t *key8Arr = (uint8_t *)key;
      const uint64_t *key64Arr = (uint64_t *)key;

      const uint8_t seedbuf[16] = {0};
      uint64_t *seed64Arr = (uint64_t *)seedbuf;
      uint32_t *seed32Arr = (uint32_t *)seedbuf;
      const uint8_t *seed8Arr = (uint8_t *)seedbuf;

      const uint8_t buf[32] = {0};
      uint8_t *state8 = (uint8_t *)buf;
      uint32_t *state32 = (uint32_t *)buf;
      uint64_t *state = (uint64_t *)buf;

      seed32Arr[0] -= seed;
      seed32Arr[1] = 1 - seed;
      seed32Arr[2] = seed + 1;
      seed32Arr[3] = ~seed;

      round( seed64Arr, seed8Arr, 16, state, state8 );
      round( key64Arr, key8Arr, len, state, state8 );

      //printf("state = %#018" PRIx64 " %#018" PRIx64 " %#018" PRIx64 " %#018" PRIx64 "\n",
      //  state[0], state[1], state[2], state[3] );

      const uint8_t output[32] = {0};
      uint64_t *h = (uint64_t *)output;

      // The new combination step
      h[0] = state[0];
      h[1] = state[1];
      h[2] = state[2];
      h[3] = state[3];

      h[0] = h[0] + h[3];
      h[1] = h[1] + h[2];

      h[0] ^= h[1];

      ((uint64_t *)out)[0] = h[0];
    }

