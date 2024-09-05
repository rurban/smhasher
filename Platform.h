//-----------------------------------------------------------------------------
// Platform-specific functions and macros

#pragma once

#include <stdint.h>

#if defined(__aarch64__) && defined(HAVE_INT64)
// fixme: bad system call with threads (8 cpu octocore)
# define NCPU 0
#elif !defined NCPU
# define NCPU 8
#endif

#if NCPU > 1
#include <thread>
void SetThreadAffinity ( std::thread &t, int cpu );
# if __APPLE__
#  include <mach/mach.h>
#  include <mach/thread_act.h>
# endif
#endif
void SetAffinity ( int cpu );
void SampleCpuFreq(void);
unsigned int GetCpuFreqMHz();
long getenvlong(const char *name, long minval, long defval, long maxval);

// That's not UINT64_MAX as it's converted to int64_t sometimes.
constexpr uint64_t timer_inf = INT64_MAX;

static inline uint64_t timer_sub(uint64_t a, uint64_t b)
{
#if defined(__mips__)
  // Tune SafeMHzFor32CC to reflect max clock freq. of your machine with 32-bit cycle counter.
  const uint32_t SafeMHzFor32CC = 1500;

  if ((a >> 60) != (b >> 60)) return timer_inf;
  const uint32_t scale = (a >> 60) + 1;
  const uint32_t acc = a & UINT32_MAX, bcc = b & UINT32_MAX;
  uint32_t as = (a >> 32) & (UINT32_MAX >> 4), bs = (b >> 32) & (UINT32_MAX >> 4);
  if (as < bs) as += 1 << 28; // 28-bit seconds value overflows every 8 years, so wrap is safe
  const uint32_t dsceil = as - bs + 1;
  const uint32_t safeds = UINT32_MAX / 1000000 * scale / SafeMHzFor32CC; // that's floor()
  if (dsceil > safeds) return timer_inf; // 32-bit CC value wraps every ~two seconds @ 2 GHz
  const uint32_t diff = acc - bcc; // well-defined unsigned overflow
  return uint64_t(diff) * scale;
#else
  return (a > b) ? a - b : timer_inf; // true 64-bit cycle counter overflows once a century @ 4 GHz
#endif
}

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
#define ALIGNED(n)    __declspec(align(n))

#include <stdlib.h>
#include <math.h>   // Has to be included before intrin.h or VC complains about 'ceil'
#include <intrin.h> // for __rdtsc

#define ROTL32(x,y)	_rotl(x,y)
#define ROTL64(x,y)	_rotl64(x,y)
#define ROTR32(x,y)	_rotr(x,y)
#define ROTR64(x,y)	_rotr64(x,y)

#pragma warning(disable : 4127) // "conditional expression is constant" in the if()s for avalanchetest
#pragma warning(disable : 4100)
#pragma warning(disable : 4702)

#pragma intrinsic(__rdtsc)
// Read Time Stamp Counter
#define timer_counts_ns() (false)
uint64_t timeofday(void);
#define rdtsc()       __rdtsc()
#define timer_start() __rdtsc()
#define timer_end()   __rdtsc()

#ifdef HAVE_BIT32
#define popcount8(x)  __popcnt(x)
#else
#define popcount8(x)  __popcnt64(x)
#endif

// strange windows quirks
#if defined(HAVE_SSE42) && !defined(__SSE4_2__)
#define __SSE4_2__
#endif

//-----------------------------------------------------------------------------
// Other compilers

#else	//	!defined(_MSC_VER)

#if !defined (__i386__) && !defined (__x86_64__)
#include <cstddef>
#endif
#include <stdlib.h>
#include <sys/time.h>
#if NCPU > 1
#include <pthread.h>
#endif

#define	FORCE_INLINE inline __attribute__((always_inline))
#define	NEVER_INLINE __attribute__((noinline))
#define ALIGNED(n)   __attribute__ ((aligned(n)))

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

// CLOCK_MONOTONIC_RAW access is measurably faster on some platforms.
#if defined(CLOCK_MONOTONIC_RAW)
#   define CLOCK_MONOTONIC_FASTEST CLOCK_MONOTONIC_RAW
#elif defined(CLOCK_MONOTONIC)
#   define CLOCK_MONOTONIC_FASTEST CLOCK_MONOTONIC
#else
#   undef CLOCK_MONOTONIC_FASTEST
#endif

#ifdef CLOCK_MONOTONIC_COARSE
#   define CLOCK_MNTCOARSE_FASTEST CLOCK_MONOTONIC_COARSE
#else
#   define CLOCK_MNTCOARSE_FASTEST CLOCK_MONOTONIC_FASTEST
#endif

__inline__ uint64_t timeofday()
{
#ifdef CLOCK_MONOTONIC_FASTEST
  struct timespec ts;
  clock_gettime(CLOCK_MONOTONIC_FASTEST, &ts);
  return int64_t(ts.tv_sec) * 1000000000 + ts.tv_nsec;
#else
# warning neither CLOCK_MONOTONIC nor CLOCK_MONOTONIC_RAW is defined, no nanosecond precision.
  struct timeval tv;
  gettimeofday(&tv, NULL);
  return int64_t(tv.tv_sec) * 1000000000 + tv.tv_usec * 1000;
#endif
}

__inline__ uint64_t rdtsc()
{
#ifdef _MSC_VER
    return __rdtsc();
#elif defined (__i386__) || defined (__x86_64__)
    return __builtin_ia32_rdtsc();
#elif defined(__ARM_ARCH) && (__ARM_ARCH >= 6) && defined(HAVE_INT32)
  // V6 is the earliest arch that has a standard cyclecount (some say V7)
  uint32_t pmccntr;
  uint32_t pmuseren;
  uint32_t pmcntenset;
  // Read the user mode perf monitor counter access permissions.
  asm volatile("mrc p15, 0, %0, c9, c14, 0" : "=r"(pmuseren));
  if (pmuseren & 1) {  // Allows reading perfmon counters for user mode code.
    asm volatile("mrc p15, 0, %0, c9, c12, 1" : "=r"(pmcntenset));
    if (pmcntenset & 0x80000000ul) {  // Is it counting?
      asm volatile("mrc p15, 0, %0, c9, c13, 0" : "=r"(pmccntr));
      // The counter is set up to count every 64th cycle
      return static_cast<int64_t>(pmccntr) * 64;  // Should optimize to << 6
    }
  }
  return timeofday();
#elif defined(__aarch64__) && defined(HAVE_INT64)
  uint64_t pmccntr;
  uint64_t pmuseren = 1UL;
  // Read the user mode perf monitor counter access permissions.
  //asm volatile("mrs cntv_ctl_el0,  %0" : "=r" (pmuseren));
  if (pmuseren & 1) {  // Allows reading perfmon counters for user mode code.
    asm volatile("mrs %0, cntvct_el0" : "=r" (pmccntr));
    return (uint64_t)(pmccntr) * 64;  // Should optimize to << 6
  }
  return timeofday();
#else
  return timeofday();
#endif
}

#if defined(__mips__)
#if defined(__mips16) && !defined(__mips16e2) && (_MIPS_ISA == _MIPS_ISA_MIPS32 && __mips_isa_rev >= 2)
// `rdhwr` is MIPS32r2 or MIPS16e2 and not MIPS16. Some OpenWRT builds run
// with `-mips32r2 -mtune=24kc -mips16`, so MIPS16 has to be disabled for alike
// builds to get acces to `rdhwr` from assembler's standpoint, otherwise build fails.
__attribute__((nomips16))
#endif
__inline__ uint64_t timer_mips()
{
  // Access to these registers _might_ be prohibited to user-mode code,
  // but there is no way to check it. Linux allows it in configure_hwrena():
  // https://github.com/torvalds/linux/blob/v6.9/arch/mips/kernel/traps.c#L2190-L2194
  uint32_t cntr, scale;
  asm volatile(
      "rdhwr %0, $2\n\t" // MIPS_HWR_CC
      "rdhwr %1, $3\n\t" // MIPS_HWR_CCRES
      : "=r" (cntr), "=r" (scale));
  scale--;
  if (scale > 15)
      return timer_inf;
  // Unfortunately, 32-bit counter overflows in a few seconds, so wall clock timestamp
  // has to be embedded into the timer value. Hopefully, clock_gettime call is VDSO...
  struct timespec ts;
  clock_gettime(CLOCK_MNTCOARSE_FASTEST, &ts);
  const uint32_t s28 = ts.tv_sec & (UINT32_MAX >> 4);
  return uint64_t(scale) << 60 | (uint64_t(s28) << 32) | cntr;
}
#endif // __mips__

// see https://www.intel.com/content/dam/www/public/us/en/documents/white-papers/ia-32-ia-64-benchmark-code-execution-paper.pdf 3.2.1 The Improved Benchmarking Method
__inline__ uint64_t timer_start()
{
#if defined (__i386__) && defined (__PIC__)
  // or see https://gcc.gnu.org/legacy-ml/gcc-patches/2007-09/msg00324.html
  return rdtsc();
#elif defined (__i386__) || (defined(__x86_64__) && defined (HAVE_BIT32))
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
#elif defined(__mips__)
  return timer_mips();
#else
  return rdtsc();
#endif
}

__inline__ uint64_t timer_end()
{
#if defined (__i386__) && defined (__PIC__)
  // or see https://gcc.gnu.org/legacy-ml/gcc-patches/2007-09/msg00324.html
  return rdtsc();
#elif defined (__i386__) || (defined(__x86_64__) && defined (HAVE_BIT32))
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
#elif defined(__mips__)
  return timer_mips();
#else
  return rdtsc();
#endif
}

__inline__ bool timer_counts_ns ( void )
{
  const double ratio = double(timer_start()) / timeofday();
  return (0.999 < ratio && ratio < 1.001);
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
