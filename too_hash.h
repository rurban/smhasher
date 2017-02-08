#ifndef TOO_HASH_H
#define TOO_HASH_H

#if !defined(U32) && ( \
     defined(__STDC_VERSION__) && __STDC_VERSION__ >= 199901L )
  #include <stdint.h>
  #define U32 uint32_t
#endif

#if !defined(U32)
  #include  <limits.h>
  #if   (USHRT_MAX == 0xffffffffUL)
    #define U32 unsigned short
  #elif (UINT_MAX == 0xffffffffUL)
    #define U32 unsigned int
  #elif (ULONG_MAX == 0xffffffffUL)
    #define U32 unsigned long
  #endif
#endif

#if !defined(U8)
    #define U8 unsigned char
#endif

#define STRLEN int
#define PERL_STATIC_INLINE static inline 
#ifndef STMT_START
#define STMT_START do
#define STMT_END while(0)
#endif

#define U8TO32_LE(ptr)   (*((const U32*)(ptr)))
#define ROTL(x,y) (U32)( ( (x) << (y) ) | ( (x) >> ( 32 - (y) ) ) )

#define DEBUG_TOO_HASH 0
#if DEBUG_TOO_HASH
#define WARN3(pat,v0,v1,v2)         warn(pat, v0, v1, v2)
#else
#define WARN3(pat,v0,v1,v2)
#endif

#ifdef __cplusplus
extern "C" {
#endif

void too_hash_test(const void *key, int len, U32 seed_base, void *out);

#ifdef __cplusplus
}
#endif


#endif
