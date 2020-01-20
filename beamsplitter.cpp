#include <cstdio>
#include "beamsplitter.h"

#if defined(_MSC_VER)

#define FORCE_INLINE	__forceinline

// Other compilers

#else	// defined(_MSC_VER)

#define	FORCE_INLINE inline __attribute__((always_inline))

#endif // !defined(_MSC_VER)

typedef uint8_t u8;

  //---------
  // mix function : 8 bits -> 8 bits, using an offset to specify the s-box and a key

    FORCE_INLINE int mix ( u8 * state_byte, u8 key, int offset )
    {
      const uint8_t *sbox = S[((offset>>6)&2)][offset&7];

      *state_byte = sbox[sbox[*state_byte] ^ key];

      offset += sbox[sbox[*state_byte] + key];

      return offset;
    }

  //---------
  // mix2 function : 8 bytes -> 8 bytes

    FORCE_INLINE void mix2 ( u8 *p, u8 *q, u8 *r, u8 *s, u8 *t, u8 *u, u8 *v, u8 *w, u8 k[8], int offset ) 
    {
      offset = mix(p, k[0], offset);
      offset = mix(q, k[1], offset);
      offset = mix(r, k[2], offset);
      offset = mix(s, k[3], offset);
      offset = mix(t, k[4], offset);
      offset = mix(u, k[5], offset);
      offset = mix(v, k[6], offset);
      mix(w, k[7], offset);
    }

  //---------
  // mix3 function : 64 bytes -> 64 bytes

    FORCE_INLINE uint64_t mix3 ( u8 s[64], u8 *k, int index ) 
    {
      int m = 63;
      u8 z[8] = {0};
      uint64_t *w = (uint64_t *)z;

      u8 *a = z;
      u8 *b = z+1;
      u8 *c = z+2;
      u8 *d = z+3;
      u8 *e = z+4;
      u8 *f = z+5;
      u8 *g = z+6;
      u8 *h = z+7;

      mix2(s+(*a&m),s+(*b&m),s+(*c&m),s+(*d&m),s+(*e&m),s+(*f&m),s+(*g&m),s+(*h&m), k+0, index );
      mix2(z+0,z+1,z+2,z+3,z+4,z+5,z+6,z+7, k+0, index );
      mix2(s+(*b&m),s+(*c&m),s+(*d&m),s+(*e&m),s+(*f&m),s+(*g&m),s+(*h&m),s+(*a&m), k+8, index + 8);
      mix2(z+0,z+1,z+2,z+3,z+4,z+5,z+6,z+7, k+8, index + 8);

      mix2(s+(*c&m),s+(*d&m),s+(*e&m),s+(*f&m),s+(*g&m),s+(*h&m),s+(*a&m),s+(*b&m), k+16, index + 16);
      mix2(z+0,z+1,z+2,z+3,z+4,z+5,z+6,z+7, k+16, index + 16);
      mix2(s+(*d&m),s+(*e&m),s+(*f&m),s+(*g&m),s+(*h&m),s+(*a&m),s+(*b&m),s+(*c&m), k+24, index + 24);
      mix2(z+0,z+1,z+2,z+3,z+4,z+5,z+6,z+7, k+24, index + 24);

      mix2(s+(*e&m),s+(*f&m),s+(*g&m),s+(*h&m),s+(*a&m),s+(*b&m),s+(*c&m),s+(*d&m), k+32, index + 32);
      mix2(z+0,z+1,z+2,z+3,z+4,z+5,z+6,z+7, k+32, index + 32);
      mix2(s+(*f&m),s+(*g&m),s+(*h&m),s+(*a&m),s+(*b&m),s+(*c&m),s+(*d&m),s+(*e&m), k+40, index + 40);
      mix2(z+0,z+1,z+2,z+3,z+4,z+5,z+6,z+7, k+40, index + 40);

      mix2(s+(*g&m),s+(*h&m),s+(*a&m),s+(*b&m),s+(*c&m),s+(*d&m),s+(*e&m),s+(*f&m), k+48, index + 48);
      mix2(z+0,z+1,z+2,z+3,z+4,z+5,z+6,z+7, k+48, index + 48);
      mix2(s+(*h&m),s+(*a&m),s+(*b&m),s+(*c&m),s+(*d&m),s+(*e&m),s+(*f&m),s+(*g&m), k+56, index + 56);
      mix2(z+0,z+1,z+2,z+3,z+4,z+5,z+6,z+7, k+56, index + 56);

      return *w;
    }

  //---------
  // Hash round function 

    FORCE_INLINE uint64_t round( const uint64_t * m64, const uint8_t * m8, int len, 
            uint64_t state64[8], uint8_t state8[64], uint64_t w)
    {
      int index = 0;

      for( int Len = len >> 6; index < Len; index++ ) {
        w ^= mix3( state8, (u8 *)m64+index, index<<6 );
      }

      index <<= 6;

      if ( len >= 64 ) {
        w ^= mix3( state8, (u8 *)m8+len-64, index ); 
        index = len;
      } else if ( len >= 8 ) {
        int i = 0;
        int m = 63;
        u8 z[8] = {0};
        uint64_t *w2 = (uint64_t *)z;
      
        u8 *a = z;
        u8 *b = z+1;
        u8 *c = z+2;
        u8 *d = z+3;
        u8 *e = z+4;
        u8 *f = z+5;
        u8 *g = z+6;
        u8 *h = z+7;

        while( i + 8 < len ) {
          mix2(state8+(*a&m),state8+(*b&m),state8+(*c&m),state8+(*d&m),
            state8+(*e&m),state8+(*f&m),state8+(*g&m),state8+(*h&m), (u8 *)m8+i, i);
          mix2(z+0,z+1,z+2,z+3,z+4,z+5,z+6,z+7, (u8 *)m8+i, i);
          i += 8;
        }

        w ^= *w2;
        index = i;
      }

      w ^= mix3( state8, state8, index );

      if ( index < len ) {
        int offset = index;
        for( index; index < len; index++ ) {
          offset = mix(state8+(index&63), m8[index], offset);
        }
      }

      w ^= mix3( state8, state8, index );

      return w;
    }

  //---------
  // main hash function 

    void beamsplitter_64 ( const void * key, int len, uint32_t seed, void * out )
    {
      uint64_t w = 0;

      const uint8_t *key8Arr = (uint8_t *)key;
      const uint64_t *key64Arr = (uint64_t *)key;

      const uint8_t seedbuf[8] = {0};
      uint32_t *seed32Arr = (uint32_t *)seedbuf;
      uint64_t *seed64Arr = (uint64_t *)seedbuf;
      const uint8_t *seed8Arr = (uint8_t *)seedbuf;

      const uint8_t buf[64] = {0};
      uint8_t *state8 = (uint8_t *)buf;
      uint32_t *state32 = (uint32_t *)buf;
      uint64_t *state = (uint64_t *)buf;

      seed32Arr[0] = seed;
      seed32Arr[1] = seed;

      w += round( seed64Arr, seed8Arr, 8, state, state8, w);
      w += round( key64Arr, key8Arr, len, state, state8, w);
      w += round( state, state8, 64, state, state8, w);
      w += round( key64Arr, key8Arr, len, state, state8, w);
      w += round( state, state8, 64, state, state8, w);
      w += round( key64Arr, key8Arr, len, state, state8, w);
      w += round( state, state8, 64, state, state8, w);
      w += round( state, state8, 64, state, state8, w);

      //printf("s[0..3] = %#018" PRIx64 "  %#018" PRIx64 "  %#018" PRIx64 "  %#018" PRIx64 "\n", state[0], state[1], state[2], state[3]); 
      //printf("s[4..7] = %#018" PRIx64 "  %#018" PRIx64 "  %#018" PRIx64 "  %#018" PRIx64 "\n", state[4], state[5], state[6], state[7]); 
      //printf("w = %#018" PRIx64 "\n\n", w);

      ((uint64_t *)out)[0] = w;
    }

