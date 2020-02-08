#include <cstdio>
#include <inttypes.h>
#include "discohash.h"

#if defined(_MSC_VER)

#define FORCE_INLINE  __forceinline

// Other compilers

#else // defined(_MSC_VER)

#define FORCE_INLINE inline __attribute__((always_inline))

#endif // !defined(_MSC_VER)

const int STATE = 16;
uint8_t disco_buf[STATE] = {0};
uint8_t *ds8 = (uint8_t *)disco_buf;
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

    FORCE_INLINE uint8_t rot8( uint8_t v, int n) 
    {
      n = n & 7U;
      if (n)
          v = (v >> n) | (v << (8-n));
      return v; 
    }

    FORCE_INLINE void mix()
    {
      ds[0] += ds[1];
      ds[0] = rot(ds[0], ds[1]);
      ds[1] ^= ds[0] + 0xf012ac43;
      ds[1] *= ds[0] + 0xfac728289;
      ds[1] = rot(ds[1], 37);

      ds[1] += ds[0];
      ds[1] = rot(ds[1], ds[0]);
      ds[0] ^= ds[1] + 0xaf9217faccceb1;
      ds[0] = rot(ds[1], 19);
    }

  //---------
  // Hash round function 

    FORCE_INLINE void round( const uint64_t * m64, const uint8_t * m8, int len )
    {
      int index = 0;
      int sindex = 0;

      for( int Len = len >> 3; index < Len; index++) {
        ds[sindex] += rot(m64[index] + index + 1, ds[sindex] +index +1);
        if ( sindex == 1 ) {
          mix();
          sindex = -1;
        }
        sindex++;
      }

      mix();

      index <<= 3;
      sindex = index&15;
      for( ; index < len; index++) {
        ds8[sindex] += rot8(m8[index] + index + 1, ds8[sindex] + index+1);
        mix();
        if ( sindex >= 15 ) {
          sindex = -1;
        }
        sindex++;
      }

      mix();
    }

  //---------
  // main hash function 

    void discohash_64 ( const void * key, int len, unsigned seed, void * out )
    {
      const uint8_t *key8Arr = (uint8_t *)key;
      const uint64_t *key64Arr = (uint64_t *)key;

      const uint8_t seedbuf[8] = {0};
      const uint8_t *seed8Arr = (uint8_t *)seedbuf;
      const uint64_t *seed64Arr = (uint64_t *)seedbuf;
      uint32_t *seed32Arr = (uint32_t *)seedbuf;

      // the cali number from the Matrix (1999)
      seed32Arr[0] = 0xc5550690;
      seed32Arr[0] -= seed;
      seed32Arr[1] = ~(1 - seed);

      // nothing up my sleeve
      ds[0] = 0x123456789abcdef0;
      ds[1] = 0x0fedcba987654321;
      ds[2] = 0xaccadacca80081e5;
      ds[3] = 0xf00baaf00f00baaa;

      round( key64Arr, key8Arr, len );
      round( seed64Arr, seed8Arr, 8 );
      //round( ds, ds8, STATE   );

      /*
      //printf("ds = %#018" PRIx64 " %#018" PRIx64 "\n",
      //  ds[0], ds[1]  );
      */

      const uint8_t output[STATE] = {0};
      uint64_t *h = (uint64_t *)output;

      h[0] = ds[1];

      ((uint64_t *)out)[0] = h[0];
    }
