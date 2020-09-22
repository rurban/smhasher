// also known as BEBB4185
// Copyright 2020 Cris Stringfellow
// Licensed under GPL-3.0
// https://github.com/cris691/discohash
#include <cstdio>
#include <inttypes.h>
#include "discohash.h"

#if defined(_MSC_VER)

#define FORCE_INLINE  __forceinline

// Other compilers

#else // defined(_MSC_VER)

#define FORCE_INLINE inline __attribute__((always_inline))

#endif // !defined(_MSC_VER)

const int STATE = 32;
const int STATE64 = STATE >> 3;
const int STATEM = STATE-1;
const int HSTATE64M = (STATE64 >> 1)-1;
const int STATE64M = STATE64-1;
uint8_t disco_buf[STATE] = {0};
uint64_t P = 0xFFFFFFFFFFFFFFFF - 58;
uint64_t Q = 13166748625691186689U;
uint8_t *ds8 = (uint8_t *)disco_buf;
uint32_t *ds32 = (uint32_t *)disco_buf;
uint64_t *ds = (uint64_t *)disco_buf;

  //--------
  // State mix function

    FORCE_INLINE uint64_t rot( uint64_t v, int n) 
    {
      n = n & 63U;
      if (n)
          v = (v >> n) | (v << (64-n));
      return v; 
    }

    FORCE_INLINE uint64_t rot32( uint64_t v, int n) 
    {
      n = n & 31U;
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

    FORCE_INLINE void mixA()
    {
      int i = ds32[0] & 1;
      int j = ds32[3] & 3;

      ds[0] = rot(ds[0], ds[i]);
      ds[i] *= (P % (ds32[j] + 1) + 1);
      ds[1] += ds32[j];
    }

    FORCE_INLINE void mix(const int A)
    {
      const int B = A+1;
      ds[A] *= P;
      ds[A] = rot(ds[A], 23);
      ds[A] *= Q;
      //ds[A] = rot(ds[A], 23);
      
      ds[B] ^= ds[A];

      ds[B] *= P;
      ds[B] = rot(ds[B], 23);
      ds[B] *= Q;
      //ds[B] = rot(ds[B], 23);
    }

  //---------
  // Hash round function (requires aligned m64)

    FORCE_INLINE void round( const uint64_t * m64, const uint8_t * m8, int len )
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
        } else if ( sindex == STATE64M ) {
          mix(2);
          sindex = -1;
        }
        sindex++;
      }

      mix(1);

      index <<= 3;
      sindex = index & (STATEM);
      for( ; index < len; index++) {
        ds8[sindex] += rot8(m8[index] + index + counter8 + 1, 23);
        counter8 += ~m8[sindex] + 1;
        mix(index % STATE64M);
        if ( sindex >= STATEM ) {
          sindex = -1;
        }
        sindex++;
      }

      mix(0);
      mix(1);
      mix(2);
    }

#ifdef HAVE_ALIGNED_ACCESS_REQUIRED
    FORCE_INLINE void round8( const uint8_t * m8, int len )
    {
      int index = 0;
      int sindex = 0;
      uint8_t counter8 = 137;

      mix(1);

      sindex = index & (STATEM);
      for( ; index < len; index++) {
        ds8[sindex] += rot8(m8[index] + index + counter8 + 1, 23);
        counter8 += ~m8[sindex] + 1;
        mix(index % STATE64M);
        if ( sindex >= STATEM ) {
          sindex = -1;
        }
        sindex++;
      }

      mix(1);
    }
#endif

  //---------
  // main hash function 

    void BEBB4185_64 ( const void * key, int len, unsigned seed, void * out )
    {
      const uint8_t *key8Arr = (uint8_t *)key;
      uint64_t *key64Arr = (uint64_t *)key;

      const uint8_t seedbuf[16] = {0};
      const uint8_t *seed8Arr = (uint8_t *)seedbuf;
      const uint64_t *seed64Arr = (uint64_t *)seedbuf;
      uint32_t *seed32Arr = (uint32_t *)seedbuf;
      int i;

      // the cali number from the Matrix (1999)
      seed32Arr[0] = 0xc5550690;
      seed32Arr[0] -= seed;
      // if seed mod doesn't work let's try reverse order of seed/key round calls
      seed32Arr[1] = 1 + seed;
      seed32Arr[2] = ~(1 - seed);
      seed32Arr[3] = (1+seed) * 0xf00dacca;

      // nothing up my sleeve
      ds[0] = 0x123456789abcdef0;
      ds[1] = 0x0fedcba987654321;
      ds[2] = 0xaccadacca80081e5;
      ds[3] = 0xf00baaf00f00baaa;

#ifdef HAVE_ALIGNED_ACCESS_REQUIRED
      // avoid ubsan, misaligned accesses
      if ((i = (uintptr_t)key64Arr % sizeof (size_t))) {
        round8 (key8Arr, i);
        if (len >= i) {
          uintptr_t p = (uintptr_t)key64Arr;
          p += 8 - i;
          key64Arr = (uint64_t*)p;
          round( key64Arr, (uint8_t*)p, len-i );
        }
      }
#else
      round( key64Arr, key8Arr, len );
#endif
      round( seed64Arr, seed8Arr, 16 );
      round( ds, ds8, STATE   );

      /**
      printf("ds = %#018" PRIx64 " %#018" PRIx64 " %#018" PRIx64 " %#018" PRIx64 "\n",
        ds[0], ds[1], ds[2], ds[3] );
      **/

      const uint8_t output[STATE] = {0};
      uint64_t *h = (uint64_t *)output;

      //h[0] = ds[1];
      h[0] = ds[2];
      h[1] = ds[3];

      h[0] += h[1];

      ((uint64_t *)out)[0] = h[0];
    }
