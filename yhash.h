#ifndef YHASH_H
#define YHASH_H

#define HASH_UINT U64
#define HASH_BITS 64
#include <stdint.h>

#if !defined(U64) 
  #define U64 uint64_t
#endif

#if !defined(U32) 
  #define U32 uint32_t
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

#define U8TO64_LE(ptr)  (*((const U64 *)(ptr)))
#define ROTL(x,y)       ( ( (x) << (y) ) | ( (x) >> ( HASH_BITS - (y) ) ) )

#ifdef __cplusplus
extern "C" {
#endif

void yhash_test(const void *key, int len, U32 seed_base, void *out);

#ifdef __cplusplus
}
#endif


#endif
