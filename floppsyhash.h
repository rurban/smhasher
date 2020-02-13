/* https://github.com/crislin2046/floppsy */
#ifndef _FLOPPSYHASH_H_
#define _FLOPPSYHASH_H_

#if defined(_MSC_VER) && (_MSC_VER < 1600)

typedef unsigned char uint8_t;
typedef unsigned int uint32_t;
typedef unsigned __int64 uint64_t;

// Other compilers

#else	// defined(_MSC_VER)

#include <stdint.h>

#endif // !defined(_MSC_VER)

void floppsyhash_64          ( const void * key, int len, uint32_t seed, void * out );

#endif // _FLOPPSYHASH_H_
