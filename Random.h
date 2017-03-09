#pragma once

#include "Types.h"
#include <sys/types.h>
#include <unistd.h>
//-----------------------------------------------------------------------------
// Xorshift RNG, based on code by George Marsaglia
// Actual implementation by Sebastiano Vigna
// See:
// http://vigna.di.unimi.it/ftp/papers/xorshift.pdf
// http://en.wikipedia.org/wiki/Xorshift
#define RANDSTATES 16
struct Rand
{
  uint64_t state[RANDSTATES];
  uint32_t *state32;
  int p;

  Rand()
  {
    state32= (uint32_t *)state;
    reseed(0);
  }

  Rand( uint64_t seed )
  {
    state32= (uint32_t *)state;
    reseed(seed);
  }

  void reseed ( uint64_t seed )
  {
    int i = 0;
    state[i++]= seed;
    if (!seed) state[i++] = seed = 0x0139408dcbbf7a44ULL;
    do {
      seed ^= seed << 13;
      seed ^= seed >> 7;
      seed ^= seed << 17;
      state[i++] = seed;
    } while ( i < RANDSTATES );
    _reseed();
  }

  void _reseed() {
    p= 0;
    for(int i = 0; i < 1000; i++)
      mix();
  }

  //-----------------------------------------------------------------------------

  void mix ( void )
  {
    const uint64_t s0 = state[p];
    uint64_t s1 = state[p = (p + 1) % RANDSTATES];
    s1 ^= s1 << 31; // a
    state[p] = s1 ^ s0 ^ (s1 >> 11) ^ (s0 >> 30); // b, c
  }

  uint32_t rand_u32 ( void )
  {
    return uint32_t(rand_u64() & 0xFFFFFFFF);
  }

  uint64_t rand_u64 ( void ) 
  {
    mix();

    uint64_t v = state[p] * 0x106689d45497fdb5ULL;
    return v;
  }

  void rand_p ( void * blob, int bytes )
  {
    uint64_t * blocks = reinterpret_cast<uint64_t*>(blob);

    while(bytes >= 8)
    {
      blocks[0] = rand_u64();
      blocks++;
      bytes -= 8;
    }

    if (bytes) {
      uint8_t * tail = reinterpret_cast<uint8_t*>(blocks);
      uint64_t last = rand_u64();
      for(int i = 0; i < bytes; i++)
      {
        tail[i] = uint8_t(last & 0xFF);
        last = last >> 8;
      }
    }
  }
};

//-----------------------------------------------------------------------------

extern Rand g_rand1;

inline uint32_t rand_u32 ( void ) { return g_rand1.rand_u32(); }
inline uint64_t rand_u64 ( void ) { return g_rand1.rand_u64(); }

inline void rand_p ( void * blob, int bytes )
{
  g_rand1.rand_p(blob, bytes);
}

//-----------------------------------------------------------------------------
/* vim: set sts=2 sw=2 et: */
