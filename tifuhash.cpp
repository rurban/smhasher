// Copyright 2020 Cris Stringfellow 
// Licensed under GPL-3.0
// https://github.com/dosyago/floppsy
#include <math.h>
#include "tifuhash.h"

#if defined(_MSC_VER)

#define FORCE_INLINE	__forceinline
#ifndef M_PI
#define M_PI 3.14159265358979323846264338327950288
#endif
#ifndef M_E
#define M_E  2.71828182845904523536028747135266250
#endif

// Other compilers

#else	// defined(_MSC_VER)

#define	FORCE_INLINE inline __attribute__((always_inline))

#endif // !defined(_MSC_VER)

//---------
// Q function : Continued Egyptian Fraction update function

FORCE_INLINE void q ( double * state, double key_val, 
                      double numerator, double denominator )
{
  state[0] += numerator / denominator;
  state[0] = 1.0 / state[0];

  state[1] += key_val;
  state[1] = numerator / state[1];
}

//---------
// round function : process the message 

FORCE_INLINE void round ( const uint8_t * msg, long len, 
                          double * state ) 
{
  double numerator = 1.0;

  // Loop
  for( long i = 0; i < len; i++ ) {
    double val = (double)msg[i];
    double denominator = (1.0 + val + i) / state[1];

    q( state, val, numerator, denominator );

    numerator = denominator + 1.0;
  }

  state[0] *= M_PI + state[1];
  state[1] *= M_E + state[0];
}

//---------
// setup function : setup the state

FORCE_INLINE void setup ( double * state, double init = 0 ) 
{
  state[0] = init != 0 ? pow(init + 1.0/init, 1.0/3) : 3.0;
  state[1] = init != 0 ? pow(init + 1.0/init, 1.0/7) : 1.0/7;
}

//---------
// tifuhash - tiny fast universal hash ( a.k.a today I f&%@!d up hash )
// with 64 bit continued egyptian fractions

void tifuhash_64 ( const void * key, int len,
                   uint32_t seed, void * out )
{
  const uint8_t * data = (const uint8_t *)key;
  uint8_t buf [16];
  double * state = (double*)buf;
  uint32_t * state32 = (uint32_t*)buf;
  double seed32 = (double)seed;

  setup( state, seed32 );
  round( data, len, state );

  uint32_t h[2];
  
  h[0] = state32[0] + state32[3];
  h[1] = state32[1] + state32[2];

  ((uint32_t*)out)[0] = h[0];
  ((uint32_t*)out)[1] = h[1];
} 

