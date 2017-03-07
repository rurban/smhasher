//-----------------------------------------------------------------------------
// MurmurHash3 was written by Austin Appleby, and is placed in the public
// domain. The author hereby disclaims copyright to this source code.

#ifndef _MURMURHASH3_H_
#define _MURMURHASH3_H_

//-----------------------------------------------------------------------------
// Platform-specific functions and macros

// Microsoft Visual Studio

#if defined(_MSC_VER) && (_MSC_VER < 1600)

typedef unsigned char uint8_t;
typedef unsigned int uint32_t;
typedef unsigned __int64 uint64_t;

// Other compilers

#else	// defined(_MSC_VER)

#include <stdint.h>

#endif // !defined(_MSC_VER)

//-----------------------------------------------------------------------------

void MurmurHash3_x86_32_with_state  ( const void * key, int len, const void *state, void * out );
void MurmurHash3_x86_128_with_state ( const void * key, int len, const void *state, void * out );
void MurmurHash3_x64_128_with_state ( const void * key, int len, const void *state, void * out );

//-----------------------------------------------------------------------------

#endif // _MURMURHASH3_H_
