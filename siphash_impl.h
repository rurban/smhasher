#ifndef SIPHASH_IMPL_H
#define SIPHASH_IMPL_H

#include "siphash.h"

#if defined(_MSC_VER)
	#include <intrin.h>

	#define INLINE __forceinline
	#define NOINLINE __declspec(noinline)
	#define ROTL64(a,b) _rotl64(a,b)
	#define MM16 __declspec(align(16))

	typedef unsigned int uint32_t;
	
	#if (_MSC_VER >= 1500)
		#define __SSSE3__
	#endif
	#if (_MSC_VER > 1200) || defined(_mm_free)
		#define __SSE2__
	#endif
#else
	#define INLINE __attribute__((always_inline))
	#define NOINLINE __attribute__((noinline))
	#define ROTL64(a,b) (((a)<<(b))|((a)>>(64-b)))
	#define MM16 __attribute__((aligned(16)))
#endif

#if defined(__SSE2__)
	#include <emmintrin.h>
	typedef __m128i xmmi;
	typedef __m64 qmm;

	typedef union packedelem64_t {
		uint64_t u[2];
		xmmi v;	
	} packedelem64;
	
	typedef union packedelem8_t {
		unsigned char u[16];
		xmmi v;	
	} packedelem8;
#endif

#if defined(__SSSE3__)
	#include <tmmintrin.h>
#endif

#endif // SIPHASH_IMPL_H

