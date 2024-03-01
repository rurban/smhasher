#ifndef GXHASH_H
#define GXHASH_H

#include <stdint.h>

#ifdef _MSC_VER
#  ifdef HAVE_SSE
#    define __SSE__
#  endif
#  ifdef HAVE_AES
#    define __AES__
#  endif
#  ifdef HAVE_AVX2
#    define __AVX2__
#  endif
#  ifdef HAVE_AESNI
#    define __AES__
#  endif
#endif

#if defined (__cplusplus)
extern "C" {
#endif
uint32_t gxhash32(const uint8_t* input, int len, uint32_t seed);
uint64_t gxhash64(const uint8_t* input, int len, uint32_t seed);
#if defined (__cplusplus)
}
#endif

#endif // GXHASH_H
