//-----------------------------------------------------------------------------
// Platform-specific functions and macros

#pragma once

void SetAffinity ( int cpu );

#ifndef __x86_64__
 #if defined(__x86_64) || defined(_M_AMD64) || defined(_M_X64)
  #define  __x86_64__
 #endif
#endif

#ifndef HAVE_INT64
 #if (__WORDSIZE >= 64) || defined(HAVE_SSE42)
  #define HAVE_INT64
 #endif
#endif

//-----------------------------------------------------------------------------
// Microsoft Visual Studio

#if defined(_MSC_VER)

#define FORCE_INLINE	__forceinline
#define	NEVER_INLINE  __declspec(noinline)

#include <stdlib.h>
#include <math.h>   // Has to be included before intrin.h or VC complains about 'ceil'
#include <intrin.h> // for __rdtsc
#include <stdint.h>

#define ROTL32(x,y)	_rotl(x,y)
#define ROTL64(x,y)	_rotl64(x,y)
#define ROTR32(x,y)	_rotr(x,y)
#define ROTR64(x,y)	_rotr64(x,y)

#pragma warning(disable : 4127) // "conditional expression is constant" in the if()s for avalanchetest
#pragma warning(disable : 4100)
#pragma warning(disable : 4702)

#pragma intrinsic(__rdtsc)
// Read Time Stamp Counter
#define rdtsc()       __rdtsc()
#define timer_start() __rdtsc()
#define timer_end()   __rdtsc()

#ifdef HAVE_BIT32
#define popcount8(x)  __popcnt(x)
#else
#define popcount8(x)  __popcnt64(x)
#endif

//-----------------------------------------------------------------------------
// Other compilers

#else	//	!defined(_MSC_VER)

#include <stdlib.h>
#include <stdint.h>

#define	FORCE_INLINE inline __attribute__((always_inline))
#define	NEVER_INLINE __attribute__((noinline))

#ifdef HAVE_BIT32
#define popcount8(x) __builtin_popcountll(x)
#else
#define popcount8(x) __builtin_popcountl(x)
#endif

inline uint32_t rotl32 ( uint32_t x, int8_t r )
{
  return (x << r) | (x >> (32 - r));
}

inline uint64_t rotl64 ( uint64_t x, int8_t r )
{
  return (x << r) | (x >> (64 - r));
}

inline uint32_t rotr32 ( uint32_t x, int8_t r )
{
  return (x >> r) | (x << (32 - r));
}

inline uint64_t rotr64 ( uint64_t x, int8_t r )
{
  return (x >> r) | (x << (64 - r));
}

#define	ROTL32(x,y)	rotl32(x,y)
#define ROTL64(x,y)	rotl64(x,y)
#define	ROTR32(x,y)	rotr32(x,y)
#define ROTR64(x,y)	rotr64(x,y)

__inline__ uint64_t rdtsc()
{
#ifdef _MSC_VER
    return __rdtsc();
#elif defined (__i386__) || defined (__x86_64__)
    return __builtin_ia32_rdtsc();
#else
#define NO_CYCLE_COUNTER
    return 0;
#endif
}

// see https://www.intel.com/content/dam/www/public/us/en/documents/white-papers/ia-32-ia-64-benchmark-code-execution-paper.pdf 3.2.1 The Improved Benchmarking Method
__inline__ uint64_t timer_start()
{
#if defined(__i386__) && !defined(HAVE_INT64)
  uint32_t cycles_high, cycles_low;
  __asm__ volatile
      ("cpuid\n\t"
       "rdtsc\n\t"
       "mov %%edx, %0\n\t"
       "mov %%eax, %1\n\t": "=r" (cycles_high), "=r" (cycles_low)::
       "%eax", "%ebx", "%ecx", "%edx");
    return ((uint64_t)cycles_high << 32) | cycles_low;
#elif defined __x86_64__
  uint32_t cycles_high, cycles_low;
  __asm__ volatile
      ("cpuid\n\t"
       "rdtsc\n\t"
       "mov %%edx, %0\n\t"
       "mov %%eax, %1\n\t": "=r" (cycles_high), "=r" (cycles_low)::
       "%rax", "%rbx", "%rcx", "%rdx");
    return ((uint64_t)cycles_high << 32) | cycles_low;
#else
#define NO_CYCLE_COUNTER
    return 0;
#endif
}

__inline__ uint64_t timer_end()
{
#if defined(__i386__) && !defined(HAVE_INT64)
  uint32_t cycles_high, cycles_low;
  __asm__ volatile
      ("rdtscp\n\t"
       "mov %%edx, %0\n\t"
       "mov %%eax, %1\n\t"
       "cpuid\n\t": "=r" (cycles_high), "=r" (cycles_low)::
       "%eax", "%ebx", "%ecx", "%edx");
    return ((uint64_t)cycles_high << 32) | cycles_low;
#elif defined __x86_64__
  uint32_t cycles_high, cycles_low;
  __asm__ volatile
      ("rdtscp\n\t"
       "mov %%edx, %0\n\t"
       "mov %%eax, %1\n\t"
       "cpuid\n\t": "=r" (cycles_high), "=r" (cycles_low)::
       "%rax", "%rbx", "%rcx", "%rdx");
    return ((uint64_t)cycles_high << 32) | cycles_low;
#else
#define NO_CYCLE_COUNTER
    return 0;
#endif
}


#include <strings.h>
#define _stricmp strcasecmp

#endif	//	!defined(_MSC_VER)

//-----------------------------------------------------------------------------

#ifndef __WORDSIZE
# ifdef HAVE_BIT32
#  define __WORDSIZE 32
# else
#  define __WORDSIZE 64
# endif
#endif
