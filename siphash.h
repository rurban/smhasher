#ifndef SIPHASH_H
#define SIPHASH_H

#if defined(_MSC_VER)
	typedef unsigned __int64 uint64_t;
#else
	#include <stdint.h>
	#include <stdlib.h>
#endif

#ifdef __cplusplus /* If this is a C++ compiler, use C linkage */
extern "C" {
#endif
 
uint64_t siphash(const unsigned char key[16], const unsigned char *m, size_t len);
 
#ifdef __cplusplus /* If this is a C++ compiler, end C linkage */
}
#endif


#endif // SIPHASH_H
