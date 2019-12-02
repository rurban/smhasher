#ifndef _TIFUHASH_H_
#define _TIFUHASH_H_

#if defined(_MSC_VER) && (_MSC_VER < 1600)

typedef unsigned char uint8_t;
typedef unsigned int uint32_t;
typedef unsigned __int64 uint64_t;

// Other compilers

#else	// defined(_MSC_VER)

#include <stdint.h>

#endif // !defined(_MSC_VER)

void tifuhash_64          ( const void * key, int len, uint32_t seed, void * out );

#endif // _TIFUHASH_H_
