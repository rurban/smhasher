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

struct Rand
{
  uint64_t s[16];
  int p;

  Rand()
  {
    reseed(uint32_t(getpid()));
  }

  Rand( uint32_t seed32 )
  {
    reseed(seed32);
  }

  void reseed ( uint32_t seed32 )
  {
    uint64_t seed64 = (uint64_t(0xA3614B8FUL ^ seed32) << 32) | uint64_t(seed32);
    reseed(seed64);
  }

  void reseed ( uint64_t seed64 )
  {
    s[0] = seed64;
    s[1] = 0xD64242617D960D35ULL ^ seed64;
    s[2] = 0xBBF9FD407A96A81DULL ^ (seed64 + 1);
    s[3] = 0xEC1DE0D1163C9E99ULL ^ (seed64 - 1);
    s[4] = seed64 + 1;
    s[5] = seed64 - 1;
    s[6] = 314159;
    s[7] = 42;
    s[8] = ~seed64;
    s[9] = 0xEC1DE0D1163C9E99ULL ^ seed64;
    s[10] = s[11]= s[12]= s[13]= s[14]= s[15]= 0;
    p= 0;
    for(int i = 0; i < 1000; i++)
      mix();
  }

  //-----------------------------------------------------------------------------

  void mix ( void )
  {
    const uint64_t s0 = s[p];
    uint64_t s1 = s[p = (p + 1) & 15];
    s1 ^= s1 << 31; // a
    s[p] = s1 ^ s0 ^ (s1 >> 11) ^ (s0 >> 30); // b, c
  }

  uint32_t rand_u32 ( void )
  {
    return rand_u64() & 0xFFFFFFFF;
  }

  uint64_t rand_u64 ( void ) 
  {
    mix();

    return s[p] * 1181783497276652981ULL;
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

    uint8_t * tail = reinterpret_cast<uint8_t*>(blocks);
    uint64_t last = rand_u64();
    for(int i = 0; i < bytes; i++)
    {
      tail[i] = uint8_t(last && 0xFF);
      last = last >> 8;
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
