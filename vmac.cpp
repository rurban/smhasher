/* --------------------------------------------------------------------------
 * VMAC and VHASH Implementation by Ted Krovetz (tdk@acm.org) and Wei Dai.
 * This implementation is hereby placed in the public domain.
 * The authors offers no warranty. Use at your own risk.
 * Please send bug reports to the authors.
 * Last modified: 17 APR 08, 1700 PDT
 * ----------------------------------------------------------------------- */

#include "vmac.h"
#include <string.h>
#include <stdio.h>

/* Enable code tuned for 64-bit registers; otherwise tuned for 32-bit */
#ifndef VMAC_ARCH_64
#if ( defined __x86_64__ ) || ( defined __ppc64__ ) || ( defined _M_X64 )
#define VMAC_ARCH_64 1
#endif
#endif

#ifdef __arm__
#define VMAC_USE_SSE2    0 // always
#else // __arm__
#ifdef VMAC_ARCH_64
#define VMAC_USE_SSE2    1 // always
#else // VMAC_ARCH_64
#define VMAC_USE_SSE2    0 // 0 or 1; further considerations should be applied
#endif // VMAC_ARCH_64
#endif // __arm__

/* Enable code tuned for Intel SSE2 instruction set                   */
#if ((__SSE2__ || (_M_IX86_FP >= 2)) && ( ! VMAC_ARCH_64))
#include <emmintrin.h>
#endif

#ifdef _MSC_VER
#if !defined _WIN64
#define _mmm_empty _mm_empty();
#else // _WIN64
#define _mmm_empty
#endif // _WIN64
#else // _MSC_VER
#define _mmm_empty __asm volatile ( "emms" ::: "memory" );
#endif // _MSC_VER


/* Native word reads. Update (or define via compiler) if incorrect */
#ifndef VMAC_ARCH_BIG_ENDIAN       /* Assume big-endian unless on the list */
#define VMAC_ARCH_BIG_ENDIAN \
    (!(__x86_64__ || __i386__ || _M_IX86 || \
       _M_X64 || __ARMEL__ || __MIPSEL__))
#endif

/* ----------------------------------------------------------------------- */
/* Constants and masks                                                     */

const uint64_t p64   = UINT64_C(0xfffffffffffffeff);  /* 2^64 - 257 prime  */
const uint64_t m62   = UINT64_C(0x3fffffffffffffff);  /* 62-bit mask       */
const uint64_t m63   = UINT64_C(0x7fffffffffffffff);  /* 63-bit mask       */
const uint64_t m64   = UINT64_C(0xffffffffffffffff);  /* 64-bit mask       */
const uint64_t mpoly = UINT64_C(0x1fffffff1fffffff);  /* Poly key mask     */

/* ----------------------------------------------------------------------- *
 * The following routines are used in this implementation. They are
 * written via macros to simulate zero-overhead call-by-reference.
 * All have default implemantations for when they are not defined in an
 * architecture-specific manner.
 *
 * MUL64: 64x64->128-bit multiplication
 * PMUL64: assumes top bits cleared on inputs
 * ADD128: 128x128->128-bit addition
 * GET_REVERSED_64: load and byte-reverse 64-bit word  
 * ----------------------------------------------------------------------- */

/* ----------------------------------------------------------------------- */
#if (__GNUC__ && (__x86_64__ || __amd64__))
/* ----------------------------------------------------------------------- */

#define ADD128(rh,rl,ih,il)                                               \
    asm ("addq %3, %1 \n\t"                                               \
         "adcq %2, %0"                                                    \
    : "+r"(rh),"+r"(rl)                                                   \
    : "r"(ih),"r"(il) : "cc");

#define MUL64(rh,rl,i1,i2)                                                \
    asm ("mulq %3" : "=a"(rl), "=d"(rh) : "a"(i1), "r"(i2) : "cc")

#define PMUL64 MUL64

#define GET_REVERSED_64(p)                                                \
    ({uint64_t x;                                                         \
     asm ("bswapq %0" : "=r" (x) : "0"(*(uint64_t *)(p))); x;})

/* ----------------------------------------------------------------------- */
#elif (__GNUC__ && __i386__)
/* ----------------------------------------------------------------------- */

#define GET_REVERSED_64(p)                                                \
    ({ uint64_t x;                                                        \
    uint32_t *tp = (uint32_t *)(p);                                       \
    asm  ("bswap %%edx\n\t"                                               \
          "bswap %%eax"                                                   \
    : "=A"(x)                                                             \
    : "a"(tp[1]), "d"(tp[0]));                                            \
    x; })

/* ----------------------------------------------------------------------- */
#elif (__GNUC__ && __ppc64__)
/* ----------------------------------------------------------------------- */

#define ADD128(rh,rl,ih,il)                                               \
    asm volatile (  "addc %1, %1, %3 \n\t"                                \
                    "adde %0, %0, %2"                                     \
    : "+r"(rh),"+r"(rl)                                                   \
    : "r"(ih),"r"(il));

#define MUL64(rh,rl,i1,i2)                                                \
{ uint64_t _i1 = (i1), _i2 = (i2);                                        \
    rl = _i1 * _i2;                                                       \
    asm volatile ("mulhdu %0, %1, %2" : "=r" (rh) : "r" (_i1), "r" (_i2));\
}

#define PMUL64 MUL64

#define GET_REVERSED_64(p)                                                \
    ({ uint32_t hi, lo, *_p = (uint32_t *)(p);                            \
       asm volatile ("lwbrx %0, %1, %2" : "=r"(lo) : "b%"(0), "r"(_p) );  \
       asm volatile ("lwbrx %0, %1, %2" : "=r"(hi) : "b%"(4), "r"(_p) );  \
       ((uint64_t)hi << 32) | (uint64_t)lo; } )

/* ----------------------------------------------------------------------- */
#elif (__GNUC__ && (__ppc__ || __PPC__))
/* ----------------------------------------------------------------------- */

#define GET_REVERSED_64(p)                                                \
    ({ uint32_t hi, lo, *_p = (uint32_t *)(p);                            \
       asm volatile ("lwbrx %0, %1, %2" : "=r"(lo) : "b%"(0), "r"(_p) );  \
       asm volatile ("lwbrx %0, %1, %2" : "=r"(hi) : "b%"(4), "r"(_p) );  \
       ((uint64_t)hi << 32) | (uint64_t)lo; } )

/* ----------------------------------------------------------------------- */
#elif (__GNUC__ && (__ARMEL__ || __ARM__))
/* ----------------------------------------------------------------------- */

#define bswap32(v)                                                        \
({ uint32_t tmp,out;                                                      \
    asm volatile(                                                         \
        "eor    %1, %2, %2, ror #16\n"                                    \
        "bic    %1, %1, #0x00ff0000\n"                                    \
        "mov    %0, %2, ror #8\n"                                         \
        "eor    %0, %0, %1, lsr #8"                                       \
    : "=r" (out), "=&r" (tmp)                                             \
    : "r" (v));                                                           \
    out;})

/* ----------------------------------------------------------------------- */
#elif _MSC_VER
/* ----------------------------------------------------------------------- */

#include <intrin.h>

#if (_M_IA64 || _M_X64) && \
    (!defined(__INTEL_COMPILER) || __INTEL_COMPILER >= 1000)
#define MUL64(rh,rl,i1,i2)   (rl) = _umul128(i1,i2,&(rh));
#pragma intrinsic(_umul128)
#define PMUL64 MUL64
#endif

/* MSVC uses add, adc in this version */
#define ADD128(rh,rl,ih,il)                                          \
    {   uint64_t _il = (il);                                         \
        (rl) += (_il);                                               \
        (rh) += (ih) + ((rl) < (_il));                               \
    }

#if _MSC_VER >= 1300
#define GET_REVERSED_64(p) _byteswap_uint64(*(uint64_t *)(p))
#pragma intrinsic(_byteswap_uint64)
#endif

#if _MSC_VER >= 1400 && \
    (!defined(__INTEL_COMPILER) || __INTEL_COMPILER >= 1000)
#define MUL32(i1,i2)    (__emulu((uint32_t)(i1),(uint32_t)(i2)))
#pragma intrinsic(__emulu)
#endif

/* ----------------------------------------------------------------------- */
#endif
/* ----------------------------------------------------------------------- */

#if __GNUC__
#define ALIGN(n)      __attribute__ ((aligned(n))) 
#define NOINLINE      __attribute__ ((noinline))
#define FASTCALL
#elif _MSC_VER
#define ALIGN(n)      __declspec(align(n))
#define NOINLINE      __declspec(noinline)
#define FASTCALL      __fastcall
#else
#define ALIGN(n)
#define NOINLINE
#define FASTCALL
#endif

/* ----------------------------------------------------------------------- */
/* Default implementations, if not defined above                           */
/* ----------------------------------------------------------------------- */

#ifndef ADD128
#define ADD128(rh,rl,ih,il)                                              \
    {   uint64_t _il = (il);                                             \
        (rl) += (_il);                                                   \
        if ((rl) < (_il)) (rh)++;                                        \
        (rh) += (ih);                                                    \
    }
#endif

#ifndef MUL32
#define MUL32(i1,i2)    ((uint64_t)(uint32_t)(i1)*(uint32_t)(i2))
#endif

#ifndef PMUL64              /* rh may not be same as i1 or i2 */
#define PMUL64(rh,rl,i1,i2) /* Assumes m doesn't overflow     */         \
    {   uint64_t _i1 = (i1), _i2 = (i2);                                 \
        uint64_t m = MUL32(_i1,_i2>>32) + MUL32(_i1>>32,_i2);            \
        rh         = MUL32(_i1>>32,_i2>>32);                             \
        rl         = MUL32(_i1,_i2);                                     \
        ADD128(rh,rl,(m >> 32),(m << 32));                               \
    }
#endif

#ifndef MUL64
#define MUL64(rh,rl,i1,i2)                                               \
    {   uint64_t _i1 = (i1), _i2 = (i2);                                 \
        uint64_t m1= MUL32(_i1,_i2>>32);                                 \
        uint64_t m2= MUL32(_i1>>32,_i2);                                 \
        rh         = MUL32(_i1>>32,_i2>>32);                             \
        rl         = MUL32(_i1,_i2);                                     \
        ADD128(rh,rl,(m1 >> 32),(m1 << 32));                             \
        ADD128(rh,rl,(m2 >> 32),(m2 << 32));                             \
    }
#endif

#ifndef GET_REVERSED_64
#ifndef bswap64
#ifndef bswap32
#define bswap32(x)                                                        \
  ({ uint32_t bsx = (x);                                                  \
      ((((bsx) & 0xff000000u) >> 24) | (((bsx) & 0x00ff0000u) >>  8) |    \
       (((bsx) & 0x0000ff00u) <<  8) | (((bsx) & 0x000000ffu) << 24)); })
#endif
#define bswap64(x)                                                        \
     ({ union { uint64_t ll; uint32_t l[2]; } w, r;                       \
         w.ll = (x);                                                      \
         r.l[0] = bswap32 (w.l[1]);                                       \
         r.l[1] = bswap32 (w.l[0]);                                       \
         r.ll; })
#endif
#define GET_REVERSED_64(p) bswap64(*(uint64_t *)(p)) 
#endif

/* ----------------------------------------------------------------------- */

#if (VMAC_PREFER_BIG_ENDIAN)
#  define get64PE get64BE
#else
#  define get64PE get64LE
#endif

#if (VMAC_ARCH_BIG_ENDIAN)
#  define get64BE(ptr) (*(uint64_t *)(ptr))
#  define get64LE(ptr) GET_REVERSED_64(ptr)
#else /* assume little-endian */
#  define get64BE(ptr) GET_REVERSED_64(ptr)
#  define get64LE(ptr) (*(uint64_t *)(ptr))
#endif


/* --------------------------------------------------------------------- *
 * For highest performance the L1 NH and L2 polynomial hashes should be
 * carefully implemented to take advantage of one's target architechture.
 * Here these two hash functions are defined multiple time; once for
 * 64-bit architectures, once for 32-bit SSE2 architectures, and once
 * for the rest (32-bit) architectures.
 * For each, nh_16 *must* be defined (works on multiples of 16 bytes).
 * Optionally, nh_vmac_nhbytes can be defined (for multiples of
 * VMAC_NHBYTES), and nh_16_2 and nh_vmac_nhbytes_2 (versions that do two
 * NH computations at once).
 * --------------------------------------------------------------------- */

/* ----------------------------------------------------------------------- */
#if VMAC_ARCH_64
/* ----------------------------------------------------------------------- */

#define nh_16(mp, kp, nw, rh, rl)                                            \
{   int i; uint64_t th, tl;                                                  \
    rh = rl = 0;                                                             \
    for (i = 0; i < nw; i+= 2) {                                             \
        MUL64(th,tl,get64PE((mp)+i  )+(kp)[i  ],get64PE((mp)+i+1)+(kp)[i+1]);\
        ADD128(rh,rl,th,tl);                                                 \
    }                                                                        \
}
#define nh_16_2(mp, kp, nw, rh, rl, rh1, rl1)                                \
{   int i; uint64_t th, tl;                                                  \
    rh1 = rl1 = rh = rl = 0;                                                 \
    for (i = 0; i < nw; i+= 2) {                                             \
        MUL64(th,tl,get64PE((mp)+i  )+(kp)[i  ],get64PE((mp)+i+1)+(kp)[i+1]);\
        ADD128(rh,rl,th,tl);                                                 \
        MUL64(th,tl,get64PE((mp)+i  )+(kp)[i+2],get64PE((mp)+i+1)+(kp)[i+3]);\
        ADD128(rh1,rl1,th,tl);                                               \
    }                                                                        \
}

#if (VMAC_NHBYTES >= 64) /* These versions do 64-bytes of message at a time */
#define nh_vmac_nhbytes(mp, kp, nw, rh, rl)                                  \
{   int i; uint64_t th, tl;                                                  \
    rh = rl = 0;                                                             \
    for (i = 0; i < nw; i+= 8) {                                             \
        MUL64(th,tl,get64PE((mp)+i  )+(kp)[i  ],get64PE((mp)+i+1)+(kp)[i+1]);\
        ADD128(rh,rl,th,tl);                                                 \
        MUL64(th,tl,get64PE((mp)+i+2)+(kp)[i+2],get64PE((mp)+i+3)+(kp)[i+3]);\
        ADD128(rh,rl,th,tl);                                                 \
        MUL64(th,tl,get64PE((mp)+i+4)+(kp)[i+4],get64PE((mp)+i+5)+(kp)[i+5]);\
        ADD128(rh,rl,th,tl);                                                 \
        MUL64(th,tl,get64PE((mp)+i+6)+(kp)[i+6],get64PE((mp)+i+7)+(kp)[i+7]);\
        ADD128(rh,rl,th,tl);                                                 \
    }                                                                        \
}
#define nh_vmac_nhbytes_2(mp, kp, nw, rh, rl, rh1, rl1)                      \
{   int i; uint64_t th, tl;                                                  \
    rh1 = rl1 = rh = rl = 0;                                                 \
    for (i = 0; i < nw; i+= 8) {                                             \
        MUL64(th,tl,get64PE((mp)+i  )+(kp)[i  ],get64PE((mp)+i+1)+(kp)[i+1]);\
        ADD128(rh,rl,th,tl);                                                 \
        MUL64(th,tl,get64PE((mp)+i  )+(kp)[i+2],get64PE((mp)+i+1)+(kp)[i+3]);\
        ADD128(rh1,rl1,th,tl);                                               \
        MUL64(th,tl,get64PE((mp)+i+2)+(kp)[i+2],get64PE((mp)+i+3)+(kp)[i+3]);\
        ADD128(rh,rl,th,tl);                                                 \
        MUL64(th,tl,get64PE((mp)+i+2)+(kp)[i+4],get64PE((mp)+i+3)+(kp)[i+5]);\
        ADD128(rh1,rl1,th,tl);                                               \
        MUL64(th,tl,get64PE((mp)+i+4)+(kp)[i+4],get64PE((mp)+i+5)+(kp)[i+5]);\
        ADD128(rh,rl,th,tl);                                                 \
        MUL64(th,tl,get64PE((mp)+i+4)+(kp)[i+6],get64PE((mp)+i+5)+(kp)[i+7]);\
        ADD128(rh1,rl1,th,tl);                                               \
        MUL64(th,tl,get64PE((mp)+i+6)+(kp)[i+6],get64PE((mp)+i+7)+(kp)[i+7]);\
        ADD128(rh,rl,th,tl);                                                 \
        MUL64(th,tl,get64PE((mp)+i+6)+(kp)[i+8],get64PE((mp)+i+7)+(kp)[i+9]);\
        ADD128(rh1,rl1,th,tl);                                               \
    }                                                                        \
}
#endif

#define poly_step(ah, al, kh, kl, mh, ml)                   \
{   uint64_t t1h, t1l, t2h, t2l, t3h, t3l, z=0;             \
    /* compute ab*cd, put bd into result registers */       \
    PMUL64(t3h,t3l,al,kh);                                  \
    PMUL64(t2h,t2l,ah,kl);                                  \
    PMUL64(t1h,t1l,ah,2*kh);                                \
    PMUL64(ah,al,al,kl);                                    \
    /* add 2 * ac to result */                              \
    ADD128(ah,al,t1h,t1l);                                  \
    /* add together ad + bc */                              \
    ADD128(t2h,t2l,t3h,t3l);                                \
    /* now (ah,al), (t2l,2*t2h) need summing */             \
    /* first add the high registers, carrying into t2h */   \
    ADD128(t2h,ah,z,t2l);                                   \
    /* double t2h and add top bit of ah */                  \
    t2h = 2 * t2h + (ah >> 63);                             \
    ah &= m63;                                              \
    /* now add the low registers */                         \
    ADD128(ah,al,mh,ml);                                    \
    ADD128(ah,al,z,t2h);                                    \
}

/* ----------------------------------------------------------------------- */
#elif VMAC_USE_SSE2
/* ----------------------------------------------------------------------- */

// macros from Crypto++ for sharing inline assembly code between MSVC and GNU C
#if defined(__GNUC__)
	// define these in two steps to allow arguments to be expanded
	#define GNU_AS2(x, y) #x ", " #y ";"
	#define GNU_AS3(x, y, z) #x ", " #y ", " #z ";"
	#define GNU_ASL(x) "\n" #x ":"
	#define GNU_ASJ(x, y, z) #x " " #y #z ";"
	#define AS2(x, y) GNU_AS2(x, y)
	#define AS3(x, y, z) GNU_AS3(x, y, z)
	#define ASS(x, y, a, b, c, d) #x ", " #y ", " #a "*64+" #b "*16+" #c "*4+" #d ";"
	#define ASL(x) GNU_ASL(x)
	#define ASJ(x, y, z) GNU_ASJ(x, y, z)
#else
	#define AS2(x, y) __asm {x, y}
	#define AS3(x, y, z) __asm {x, y, z}
	#define ASS(x, y, a, b, c, d) __asm {x, y, _MM_SHUFFLE(a, b, c, d)}
	#define ASL(x) __asm {label##x:}
	#define ASJ(x, y, z) __asm {x label##y}
#endif

static void NOINLINE nh_16_func(const uint64_t *mp, const uint64_t *kp, size_t nw, uint64_t *rh, uint64_t *rl)
{
	// This assembly version, using MMX registers, is just as fast as the
	// intrinsics version (which uses XMM registers) on the Intel Core 2,
	// but is much faster on the Pentium 4. In order to schedule multiplies
	// as early as possible, the loop interleaves operations for the current
	// block and the next block. To mask out high 32-bits, we use "movd"
	// to move the lower 32-bits to the stack and then back. Surprisingly,
	// this is faster than any other method.
#ifdef __GNUC__
	__asm__ __volatile__
	(
		".intel_syntax noprefix;"
#else
		AS2(	mov		esi, mp)
		AS2(	mov		edi, kp)
		AS2(	mov		ecx, nw)
		AS2(	mov		eax, rl)
		AS2(	mov		edx, rh)
#endif
		AS2(	sub		esp, 12)
		AS2(	movq	mm6, [esi])
		AS2(	paddq	mm6, [edi])
		AS2(	movq	mm5, [esi+8])
		AS2(	paddq	mm5, [edi+8])
		AS2(	add		esi, 16)
		AS2(	add		edi, 16)
		AS2(	movq	mm4, mm6)
		ASS(	pshufw	mm2, mm6, 1, 0, 3, 2)
		AS2(	pmuludq	mm6, mm5)
		ASS(	pshufw	mm3, mm5, 1, 0, 3, 2)
		AS2(	pmuludq	mm5, mm2)
		AS2(	pmuludq	mm2, mm3)
		AS2(	pmuludq	mm3, mm4)
		AS2(	pxor	mm7, mm7)
		AS2(	movd	[esp], mm6)
		AS2(	psrlq	mm6, 32)
		AS2(	movd	[esp+4], mm5)
		AS2(	psrlq	mm5, 32)
		AS2(	sub		ecx, 2)
		ASJ(	jz,		1, f)
		ASL(0)
		AS2(	movq	mm0, [esi])
		AS2(	paddq	mm0, [edi])
		AS2(	movq	mm1, [esi+8])
		AS2(	paddq	mm1, [edi+8])
		AS2(	add		esi, 16)
		AS2(	add		edi, 16)
		AS2(	movq	mm4, mm0)
		AS2(	paddq	mm5, mm2)
		ASS(	pshufw	mm2, mm0, 1, 0, 3, 2)
		AS2(	pmuludq	mm0, mm1)
		AS2(	movd	[esp+8], mm3)
		AS2(	psrlq	mm3, 32)
		AS2(	paddq	mm5, mm3)
		ASS(	pshufw	mm3, mm1, 1, 0, 3, 2)
		AS2(	pmuludq	mm1, mm2)
		AS2(	pmuludq	mm2, mm3)
		AS2(	pmuludq	mm3, mm4)
		AS2(	movd	mm4, [esp])
		AS2(	paddq	mm7, mm4)
		AS2(	movd	mm4, [esp+4])
		AS2(	paddq	mm6, mm4)
		AS2(	movd	mm4, [esp+8])
		AS2(	paddq	mm6, mm4)
		AS2(	movd	[esp], mm0)
		AS2(	psrlq	mm0, 32)
		AS2(	paddq	mm6, mm0)
		AS2(	movd	[esp+4], mm1)
		AS2(	psrlq	mm1, 32)
		AS2(	paddq	mm5, mm1)
		AS2(	sub		ecx, 2)
		ASJ(	jnz,	0, b)
		ASL(1)
		AS2(	paddq	mm5, mm2)
		AS2(	movd	[esp+8], mm3)
		AS2(	psrlq	mm3, 32)
		AS2(	paddq	mm5, mm3)
		AS2(	movd	mm4, [esp])
		AS2(	paddq	mm7, mm4)
		AS2(	movd	mm4, [esp+4])
		AS2(	paddq	mm6, mm4)
		AS2(	movd	mm4, [esp+8])
		AS2(	paddq	mm6, mm4)

		ASS(	pshufw	mm0, mm7, 3, 2, 1, 0)
		AS2(	psrlq	mm7, 32)
		AS2(	paddq	mm6, mm7)
		AS2(	punpckldq	mm0, mm6)
		AS2(	psrlq	mm6, 32)
		AS2(	paddq	mm5, mm6)
		AS2(	movq	[eax], mm0)
		AS2(	movq	[edx], mm5)
		AS2(	add		esp, 12)
#ifdef __GNUC__
		".att_syntax prefix;"
		:
		: "S" (mp), "D" (kp), "c" (nw), "a" (rl), "d" (rh)
		: "memory", "cc"
	);
#endif
}
#define nh_16(mp, kp, nw, rh, rl)   nh_16_func(mp, kp, nw, &(rh), &(rl));

static void poly_step_func(uint64_t *ahi, uint64_t *alo, const uint64_t *kh,
               const uint64_t *kl, const uint64_t *mh, const uint64_t *ml)                  
{
	// This code tries to schedule the multiplies as early as possible to overcome
	// the long latencies on the Pentium 4. It also minimizes "movq" instructions
	// which are very expensive on the P4.

#define a0 [eax+0]
#define a1 [eax+4]
#define a2 [ebx+0]
#define a3 [ebx+4]
#define k0 [ecx+0]
#define k1 [ecx+4]
#define k2 [edx+0]
#define k3 [edx+4]

#ifdef __GNUC__
	uint32_t temp;
	__asm__ __volatile__
	(
		"mov %%ebx, %0;"
		"mov %1, %%ebx;"
		".intel_syntax noprefix;"
#else
		AS2(	mov		ebx, ahi)
		AS2(	mov		edx, kh)
		AS2(	mov		eax, alo)
		AS2(	mov		ecx, kl)
		AS2(	mov		esi, mh)
		AS2(	mov		edi, ml)
#endif

		AS2(	movd	mm0, a3)
		AS2(	movq	mm4, mm0)
		AS2(	pmuludq	mm0, k3)		// a3*k3
		AS2(	movd	mm1, a0)
		AS2(	pmuludq	mm1, k2)		// a0*k2
		AS2(	movd	mm2, a1)
		AS2(	movd	mm6, k1)
		AS2(	pmuludq	mm2, mm6)		// a1*k1
		AS2(	movd	mm3, a2)
		AS2(	movq	mm5, mm3)
		AS2(	movd	mm7, k0)
		AS2(	pmuludq	mm3, mm7)		// a2*k0
		AS2(	pmuludq	mm4, mm7)		// a3*k0
		AS2(	pmuludq	mm5, mm6)		// a2*k1
		AS2(	psllq	mm0, 1)
		AS2(	paddq	mm0, [esi])
		AS2(	paddq	mm0, mm1)
		AS2(	movd	mm1, a1)
		AS2(	paddq	mm4, mm5)
		AS2(	movq	mm5, mm1)
		AS2(	pmuludq	mm1, k2)		// a1*k2
		AS2(	paddq	mm0, mm2)
		AS2(	movd	mm2, a0)
		AS2(	paddq	mm0, mm3)
		AS2(	movq	mm3, mm2)
		AS2(	pmuludq	mm2, k3)		// a0*k3
		AS2(	pmuludq	mm3, mm7)		// a0*k0
		AS2(	movd	esi, mm0)
		AS2(	psrlq	mm0, 32)
		AS2(	pmuludq	mm7, mm5)		// a1*k0
		AS2(	pmuludq	mm5, k3)		// a1*k3
		AS2(	paddq	mm0, mm1)
		AS2(	movd	mm1, a2)
		AS2(	pmuludq	mm1, k2)		// a2*k2
		AS2(	paddq	mm0, mm2)
		AS2(	paddq	mm0, mm4)
		AS2(	movq	mm4, mm0)
		AS2(	movd	mm2, a3)
		AS2(	pmuludq	mm2, mm6)		// a3*k1
		AS2(	pmuludq	mm6, a0)		// a0*k1
		AS2(	psrlq	mm0, 31)
		AS2(	paddq	mm0, mm3)
		AS2(	movd	mm3, [edi])
		AS2(	paddq	mm0, mm3)
		AS2(	movd	mm3, a2)
		AS2(	pmuludq	mm3, k3)		// a2*k3
		AS2(	paddq	mm5, mm1)
		AS2(	movd	mm1, a3)
		AS2(	pmuludq	mm1, k2)		// a3*k2
		AS2(	paddq	mm5, mm2)
		AS2(	movd	mm2, [edi+4])
		AS2(	psllq	mm5, 1)
		AS2(	paddq	mm0, mm5)
		AS2(	movq	mm5, mm0)
		AS2(	psllq	mm4, 33)
		AS2(	psrlq	mm0, 32)
		AS2(	paddq	mm6, mm7)
		AS2(	movd	mm7, esi)
		AS2(	paddq	mm0, mm6)
		AS2(	paddq	mm0, mm2)
		AS2(	paddq	mm3, mm1)
		AS2(	psllq	mm3, 1)
		AS2(	paddq	mm0, mm3)
		AS2(	psrlq	mm4, 1)
		AS2(	punpckldq	mm5, mm0)
		AS2(	psrlq	mm0, 32)
		AS2(	por		mm4, mm7)
		AS2(	paddq	mm0, mm4)
		AS2(	movq	a0, mm5)
		AS2(	movq	a2, mm0)
#ifdef __GNUC__
		".att_syntax prefix;"
		"mov %0, %%ebx;"
		: "=m" (temp)
		: "m" (ahi), "D" (ml), "d" (kh), "a" (alo), "S" (mh), "c" (kl)
		: "memory", "cc"
	);
#endif


#undef a0
#undef a1
#undef a2
#undef a3
#undef k0
#undef k1
#undef k2
#undef k3
}

#define poly_step(ah, al, kh, kl, mh, ml)   \
        poly_step_func(&(ah), &(al), &(kh), &(kl), &(mh), &(ml))

/* ----------------------------------------------------------------------- */
#else /* not VMAC_ARCH_64 and not SSE2 */
/* ----------------------------------------------------------------------- */

#ifndef nh_16
#define nh_16(mp, kp, nw, rh, rl)                                       \
{   uint64_t t1,t2,m1,m2,t;                                             \
    int i;                                                              \
    rh = rl = t = 0;                                                    \
    for (i = 0; i < nw; i+=2)  {                                        \
        t1  = get64PE(mp+i) + kp[i];                                    \
        t2  = get64PE(mp+i+1) + kp[i+1];                                \
        m2  = MUL32(t1 >> 32, t2);                                      \
        m1  = MUL32(t1, t2 >> 32);                                      \
        ADD128(rh,rl,MUL32(t1 >> 32,t2 >> 32),MUL32(t1,t2));            \
        rh += (uint64_t)(uint32_t)(m1 >> 32) + (uint32_t)(m2 >> 32);    \
        t  += (uint64_t)(uint32_t)m1 + (uint32_t)m2;                    \
    }                                                                   \
    ADD128(rh,rl,(t >> 32),(t << 32));                                  \
}
#endif

static void poly_step_func(uint64_t *ahi, uint64_t *alo, const uint64_t *kh,
               const uint64_t *kl, const uint64_t *mh, const uint64_t *ml)                  
{

#if VMAC_ARCH_BIG_ENDIAN
#define INDEX_HIGH 0
#define INDEX_LOW 1
#else
#define INDEX_HIGH 1
#define INDEX_LOW 0
#endif

#define a0 *(((uint32_t*)alo)+INDEX_LOW)
#define a1 *(((uint32_t*)alo)+INDEX_HIGH)
#define a2 *(((uint32_t*)ahi)+INDEX_LOW)
#define a3 *(((uint32_t*)ahi)+INDEX_HIGH)
#define k0 *(((uint32_t*)kl)+INDEX_LOW)
#define k1 *(((uint32_t*)kl)+INDEX_HIGH)
#define k2 *(((uint32_t*)kh)+INDEX_LOW)
#define k3 *(((uint32_t*)kh)+INDEX_HIGH)

    uint64_t p, q, t;
    uint32_t t2;

    p = MUL32(a3, k3);
    p += p;
	p += *(uint64_t *)mh;
    p += MUL32(a0, k2);
    p += MUL32(a1, k1);
    p += MUL32(a2, k0);
    t = (uint32_t)(p);
    p >>= 32;
    p += MUL32(a0, k3);
    p += MUL32(a1, k2);
    p += MUL32(a2, k1);
    p += MUL32(a3, k0);
    t |= ((uint64_t)((uint32_t)p & 0x7fffffff)) << 32;
    p >>= 31;
    p += (uint64_t)(((uint32_t*)ml)[INDEX_LOW]);
    p += MUL32(a0, k0);
    q =  MUL32(a1, k3);
    q += MUL32(a2, k2);
    q += MUL32(a3, k1);
    q += q;
    p += q;
    t2 = (uint32_t)(p);
    p >>= 32;
    p += (uint64_t)(((uint32_t*)ml)[INDEX_HIGH]);
    p += MUL32(a0, k1);
    p += MUL32(a1, k0);
    q =  MUL32(a2, k3);
    q += MUL32(a3, k2);
    q += q;
    p += q;
    *(uint64_t *)(alo) = (p << 32) | t2;
    p >>= 32;
    *(uint64_t *)(ahi) = p + t;

#undef a0
#undef a1
#undef a2
#undef a3
#undef k0
#undef k1
#undef k2
#undef k3
}

#define poly_step(ah, al, kh, kl, mh, ml)   \
        poly_step_func(&(ah), &(al), &(kh), &(kl), &(mh), &(ml))

/* ----------------------------------------------------------------------- */
#endif  /* end of specialized NH and poly definitions */
/* ----------------------------------------------------------------------- */

/* At least nh_16 is defined. Defined others as needed  here               */
#ifndef nh_16_2
#define nh_16_2(mp, kp, nw, rh, rl, rh2, rl2)                           \
    nh_16(mp, kp, nw, rh, rl);                                          \
    nh_16(mp, ((kp)+2), nw, rh2, rl2);
#endif
#ifndef nh_vmac_nhbytes
#define nh_vmac_nhbytes(mp, kp, nw, rh, rl)                             \
    nh_16(mp, kp, nw, rh, rl)
#endif
#ifndef nh_vmac_nhbytes_2
#define nh_vmac_nhbytes_2(mp, kp, nw, rh, rl, rh2, rl2)                 \
    nh_vmac_nhbytes(mp, kp, nw, rh, rl);                                \
    nh_vmac_nhbytes(mp, ((kp)+2), nw, rh2, rl2);
#endif

/* ----------------------------------------------------------------------- */

void vhash_abort(vmac_ctx_t *ctx)
{
    ctx->polytmp[0] = ctx->polykey[0] ;
    ctx->polytmp[1] = ctx->polykey[1] ;
    #if (VMAC_TAG_LEN == 128)
    ctx->polytmp[2] = ctx->polykey[2] ;
    ctx->polytmp[3] = ctx->polykey[3] ;
    #endif
    ctx->first_block_processed = 0;
}

/* ----------------------------------------------------------------------- */
static uint64_t l3hash(uint64_t p1, uint64_t p2,
                       uint64_t k1, uint64_t k2, uint64_t len)
{
    uint64_t rh, rl, t, z=0;

    /* fully reduce (p1,p2)+(len,0) mod p127 */
    t = p1 >> 63;
    p1 &= m63;
    ADD128(p1, p2, len, t);
    /* At this point, (p1,p2) is at most 2^127+(len<<64) */
    t = (p1 > m63) + ((p1 == m63) && (p2 == m64));
    ADD128(p1, p2, z, t);
    p1 &= m63;

    /* compute (p1,p2)/(2^64-2^32) and (p1,p2)%(2^64-2^32) */
    t = p1 + (p2 >> 32);
    t += (t >> 32);
    t += (uint32_t)t > 0xfffffffeu;
    p1 += (t >> 32);
    p2 += (p1 << 32);

    /* compute (p1+k1)%p64 and (p2+k2)%p64 */
    p1 += k1;
    p1 += (0 - (p1 < k1)) & 257;
    p2 += k2;
    p2 += (0 - (p2 < k2)) & 257;

    /* compute (p1+k1)*(p2+k2)%p64 */
    MUL64(rh, rl, p1, p2);
    t = rh >> 56;
    ADD128(t, rl, z, rh);
    rh <<= 8;
    ADD128(t, rl, z, rh);
    t += t << 8;
    rl += t;
    rl += (0 - (rl < t)) & 257;
    rl += (0 - (rl > p64-1)) & 257;
    return rl;
}

/* ----------------------------------------------------------------------- */

void vhash_update(unsigned char *m,
                  unsigned int   mbytes, /* Pos multiple of VMAC_NHBYTES */
                  vmac_ctx_t    *ctx)
{
    uint64_t rh, rl, *mptr;
    const uint64_t *kptr = (uint64_t *)ctx->nhkey;
    int i;
    uint64_t ch, cl;
    uint64_t pkh = ctx->polykey[0];
    uint64_t pkl = ctx->polykey[1];
    #if (VMAC_TAG_LEN == 128)
    uint64_t ch2, cl2, rh2, rl2;
    uint64_t pkh2 = ctx->polykey[2];
    uint64_t pkl2 = ctx->polykey[3];
    #endif

    mptr = (uint64_t *)m;
    i = mbytes / VMAC_NHBYTES;  /* Must be non-zero */

    ch = ctx->polytmp[0];
    cl = ctx->polytmp[1];
    #if (VMAC_TAG_LEN == 128)
    ch2 = ctx->polytmp[2];
    cl2 = ctx->polytmp[3];
    #endif
    
    if ( ! ctx->first_block_processed) {
        ctx->first_block_processed = 1;
        #if (VMAC_TAG_LEN == 64)
        nh_vmac_nhbytes(mptr,kptr,VMAC_NHBYTES/8,rh,rl);
        #else
        nh_vmac_nhbytes_2(mptr,kptr,VMAC_NHBYTES/8,rh,rl,rh2,rl2);
        rh2 &= m62;
        ADD128(ch2,cl2,rh2,rl2);
        #endif
        rh &= m62;
        ADD128(ch,cl,rh,rl);
        mptr += (VMAC_NHBYTES/sizeof(uint64_t));
        i--;
    }

    while (i--) {
        #if (VMAC_TAG_LEN == 64)
        nh_vmac_nhbytes(mptr,kptr,VMAC_NHBYTES/8,rh,rl);
        #else
        nh_vmac_nhbytes_2(mptr,kptr,VMAC_NHBYTES/8,rh,rl,rh2,rl2);
        rh2 &= m62;
        poly_step(ch2,cl2,pkh2,pkl2,rh2,rl2);
        #endif
        rh &= m62;
        poly_step(ch,cl,pkh,pkl,rh,rl);
        mptr += (VMAC_NHBYTES/sizeof(uint64_t));
    }

    ctx->polytmp[0] = ch;
    ctx->polytmp[1] = cl;
    #if (VMAC_TAG_LEN == 128)
    ctx->polytmp[2] = ch2;
    ctx->polytmp[3] = cl2;
    #endif
    #if VMAC_USE_SSE2
    _mmm_empty /* SSE2 version of poly_step uses mmx instructions */
    #endif
}

/* ----------------------------------------------------------------------- */

uint64_t xvhash(unsigned char m[],
          unsigned int mbytes,
          uint64_t *tagl,
          vmac_ctx_t *ctx)
{
	(void) tagl; //unused
    uint64_t ch, cl, rh, rl, *mptr;
    #if (VMAC_TAG_LEN == 128)
    uint64_t ch2, cl2, rh2, rl2;
    #endif
    const uint64_t *kptr = (uint64_t *)ctx->nhkey;
    int i, remaining;

    remaining = mbytes % VMAC_NHBYTES;
    i = mbytes-remaining;
    mptr = (uint64_t *)(m+i);
    if (i) vhash_update(m,i,ctx);

    ch = ctx->polytmp[0];
    cl = ctx->polytmp[1];
    #if (VMAC_TAG_LEN == 128)
    ch2 = ctx->polytmp[2];
    cl2 = ctx->polytmp[3];
    #endif

    if (remaining) {
        #if (VMAC_TAG_LEN == 128)
        nh_16_2(mptr,kptr,2*((remaining+15)/16),rh,rl,rh2,rl2);
        rh2 &= m62;
        #else
        nh_16(mptr,kptr,2*((remaining+15)/16),rh,rl);
        #endif
        rh &= m62;
        if (i) {
            poly_step(ch,cl,ctx->polykey[0],ctx->polykey[1],rh,rl);
            #if (VMAC_TAG_LEN == 128)
            poly_step(ch2,cl2,ctx->polykey[2],ctx->polykey[3],rh2,rl2);
            #endif
        } else {
            ADD128(ch,cl,rh,rl);
            #if (VMAC_TAG_LEN == 128)
            ADD128(ch2,cl2,rh2,rl2);
            #endif
        }
    }

    #if VMAC_USE_SSE2
    _mmm_empty /* SSE2 version of poly_step uses mmx instructions */
    #endif
    vhash_abort(ctx);
    remaining *= 8;
#if (VMAC_TAG_LEN == 128)
    *tagl = l3hash(ch2, cl2, ctx->l3key[2], ctx->l3key[3],remaining);
#endif
    return l3hash(ch, cl, ctx->l3key[0], ctx->l3key[1],remaining);
}

uint64_t vhash(unsigned char m[],
          unsigned int mbytes,
          uint64_t *tagl,
          vmac_ctx_t *ctx)
{
	(void) tagl; //unused
    uint64_t rh, rl, *mptr;
    const uint64_t *kptr = (uint64_t *)ctx->nhkey;
    int i, remaining;
    uint64_t ch, cl;
    uint64_t pkh = ctx->polykey[0];
    uint64_t pkl = ctx->polykey[1];
    #if (VMAC_TAG_LEN == 128)
        uint64_t ch2, cl2, rh2, rl2;
        uint64_t pkh2 = ctx->polykey[2];
        uint64_t pkl2 = ctx->polykey[3];
    #endif

    mptr = (uint64_t *)m;
    i = mbytes / VMAC_NHBYTES;
    remaining = mbytes % VMAC_NHBYTES;

    if (ctx->first_block_processed)
    {
        ch = ctx->polytmp[0];
        cl = ctx->polytmp[1];
        #if (VMAC_TAG_LEN == 128)
        ch2 = ctx->polytmp[2];
        cl2 = ctx->polytmp[3];
        #endif
    }
    else if (i)
    {
        #if (VMAC_TAG_LEN == 64)
        nh_vmac_nhbytes(mptr,kptr,VMAC_NHBYTES/8,ch,cl);
        #else
        nh_vmac_nhbytes_2(mptr,kptr,VMAC_NHBYTES/8,ch,cl,ch2,cl2);
        ch2 &= m62;
        ADD128(ch2,cl2,pkh2,pkl2);
        #endif
        ch &= m62;
        ADD128(ch,cl,pkh,pkl);
        mptr += (VMAC_NHBYTES/sizeof(uint64_t));
        i--;
    }
    else if (remaining)
    {
        #if (VMAC_TAG_LEN == 64)
        nh_16(mptr,kptr,2*((remaining+15)/16),ch,cl);
        #else
        nh_16_2(mptr,kptr,2*((remaining+15)/16),ch,cl,ch2,cl2);
        ch2 &= m62;
        ADD128(ch2,cl2,pkh2,pkl2);
        #endif
        ch &= m62;
        ADD128(ch,cl,pkh,pkl);
        mptr += (VMAC_NHBYTES/sizeof(uint64_t));
        goto do_l3;
    }
    else /* Empty String */
    {
        ch = pkh; cl = pkl;
        #if (VMAC_TAG_LEN == 128)
        ch2 = pkh2; cl2 = pkl2;
        #endif
        goto do_l3;
    }

    while (i--) {
        #if (VMAC_TAG_LEN == 64)
        nh_vmac_nhbytes(mptr,kptr,VMAC_NHBYTES/8,rh,rl);
        #else
        nh_vmac_nhbytes_2(mptr,kptr,VMAC_NHBYTES/8,rh,rl,rh2,rl2);
        rh2 &= m62;
        poly_step(ch2,cl2,pkh2,pkl2,rh2,rl2);
        #endif
        rh &= m62;
        poly_step(ch,cl,pkh,pkl,rh,rl);
        mptr += (VMAC_NHBYTES/sizeof(uint64_t));
    }
    if (remaining) {
        #if (VMAC_TAG_LEN == 64)
        nh_16(mptr,kptr,2*((remaining+15)/16),rh,rl);
        #else
        nh_16_2(mptr,kptr,2*((remaining+15)/16),rh,rl,rh2,rl2);
        rh2 &= m62;
        poly_step(ch2,cl2,pkh2,pkl2,rh2,rl2);
        #endif
        rh &= m62;
        poly_step(ch,cl,pkh,pkl,rh,rl);
    }

do_l3:
    #if VMAC_USE_SSE2
    _mmm_empty /* SSE2 version of poly_step uses mmx instructions */
    #endif
    vhash_abort(ctx);
    remaining *= 8;
#if (VMAC_TAG_LEN == 128)
    *tagl = l3hash(ch2, cl2, ctx->l3key[2], ctx->l3key[3],remaining);
#endif
    return l3hash(ch, cl, ctx->l3key[0], ctx->l3key[1],remaining);
}

/* ----------------------------------------------------------------------- */

uint64_t vmac(unsigned char m[],
         unsigned int mbytes,
         unsigned char n[16],
         uint64_t *tagl,
         vmac_ctx_t *ctx)
{
	(void) tagl; //unused

#if (VMAC_TAG_LEN == 64)
    uint64_t *in_n, *out_p;
    uint64_t p, h;
    int i;
    
    #if VMAC_CACHE_NONCES
    in_n = ctx->cached_nonce;
    out_p = ctx->cached_aes;
    #else
    uint64_t tmp[2];
    in_n = out_p = tmp;
    #endif

    i = n[15] & 1;
    #if VMAC_CACHE_NONCES
    if ((*(uint64_t *)(n+8) != in_n[1]) ||
        (*(uint64_t *)(n  ) != in_n[0])) {
    #endif
    
        in_n[0] = *(uint64_t *)(n  );
        in_n[1] = *(uint64_t *)(n+8);
        ((unsigned char *)in_n)[15] &= 0xFE;
        aes_encryption(in_n, out_p, &ctx->cipher_key);

    #if VMAC_CACHE_NONCES
        ((unsigned char *)in_n)[15] |= (unsigned char)(1-i);
    }
    #endif
    p = get64BE(out_p + i);
    h = vhash(m, mbytes, (uint64_t *)0, ctx);
    return p + h;
#else
    uint64_t tmp[2];
    uint64_t th,tl;
    aes_encryption(n, (unsigned char *)tmp, &ctx->cipher_key);
    th = vhash(m, mbytes, &tl, ctx);
    th += get64BE(tmp);
    *tagl = tl + get64BE(tmp+1);
    return th;
#endif
}

/* ----------------------------------------------------------------------- */

void vmac_set_key(unsigned char user_key[], vmac_ctx_t *ctx)
{
    uint64_t in[2] = {0}, out[2];
    unsigned i;
    aes_key_setup(user_key, &ctx->cipher_key);
    
    /* Fill nh key */
    ((unsigned char *)in)[0] = 0x80; 
    for (i = 0; i < sizeof(ctx->nhkey)/8; i+=2) {
        aes_encryption((unsigned char *)in, (unsigned char *)out,
                                                         &ctx->cipher_key);
        ctx->nhkey[i  ] = get64BE(out);
        ctx->nhkey[i+1] = get64BE(out+1);
        ((unsigned char *)in)[15] += 1;
    }

    /* Fill poly key */
    ((unsigned char *)in)[0] = 0xC0; 
    in[1] = 0;
    for (i = 0; i < sizeof(ctx->polykey)/8; i+=2) {
        aes_encryption((unsigned char *)in, (unsigned char *)out,
                                                         &ctx->cipher_key);
        ctx->polytmp[i  ] = ctx->polykey[i  ] = get64BE(out) & mpoly;
        ctx->polytmp[i+1] = ctx->polykey[i+1] = get64BE(out+1) & mpoly;
        ((unsigned char *)in)[15] += 1;
    }

    /* Fill ip key */
    ((unsigned char *)in)[0] = 0xE0;
    in[1] = 0;
    for (i = 0; i < sizeof(ctx->l3key)/8; i+=2) {
        do {
            aes_encryption((unsigned char *)in, (unsigned char *)out,
                                                         &ctx->cipher_key);
            ctx->l3key[i  ] = get64BE(out);
            ctx->l3key[i+1] = get64BE(out+1);
            ((unsigned char *)in)[15] += 1;
        } while (ctx->l3key[i] >= p64 || ctx->l3key[i+1] >= p64);
    }
    
    /* Invalidate nonce/aes cache and reset other elements */
    #if (VMAC_TAG_LEN == 64) && (VMAC_CACHE_NONCES)
    ctx->cached_nonce[0] = (uint64_t)-1; /* Ensure illegal nonce */
    ctx->cached_nonce[1] = (uint64_t)0;  /* Ensure illegal nonce */
    #endif
    ctx->first_block_processed = 0;
}

/* ----------------------------------------------------------------------- */


#if VMAC_RUN_TESTS

#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <string.h>

unsigned prime(void)  /* Wake variable speed cpu, get rough speed estimate */
{
    volatile uint64_t i;
    volatile uint64_t j=1;
    unsigned cnt=0;
    volatile clock_t ticks = clock();
    do {
        for (i = 0; i < 500000; i++) {
            uint64_t x = get64PE(&j);
            j = x * x + (uint64_t)ticks;
        }
        cnt++;
    } while (clock() - ticks < (CLOCKS_PER_SEC/2));
    return cnt;  /* cnt is millions of iterations per second */
}

class VHASH_initializer
{
private:
	unsigned char * buff;
public:
	enum{ MAX_BUFF_LN = 0x100000};
	ALIGN(16) vmac_ctx_t ctx;
	unsigned char * aligned16Buff;
	VHASH_initializer()
	{
		ALIGN(4) unsigned char key[] = "abcdefghijklmnop";
		vmac_set_key(key, &ctx);
		buff = new unsigned char[ MAX_BUFF_LN + 128 ];
		aligned16Buff = (unsigned char*)( ( ( ( size_t(buff) ) >> 4 ) << 4 ) + 16 );
	}
	~VHASH_initializer()
	{
		delete [] buff;
	}
};

VHASH_initializer vhi;

void VHASH_32( const void * key, int len, uint32_t seed, void * res )
{
    uint64_t tagl;
#if _WORDSIZE > 32
    *(size_t*)res = 0;
#endif
#if (VMAC_TAG_LEN == 128)
    ctx.polytmp[0] = seed;
    vhash( (unsigned char *)key, len, &tagl, &ctx);
    *(uint32_t*)res = (uint32_t)tagl;
#elif (VMAC_TAG_LEN == 64)
    vhi.ctx.polytmp[0] = seed;
    *(uint32_t*)res = (uint32_t)vhash( (unsigned char *)key, len, &tagl, &(vhi.ctx) );

#else
#error VMAC_TAG_LEN could be either 64 or 128
#endif
}

void VHASH_PADDED_32(const void * key, int len, uint32_t seed, void * res)
{
	(void)seed; //unused
#ifdef __arm__
	if ((((uintptr_t)key) & 0xF) == 0) // aligned in memory
	{
#endif // __arm__
		if (len & 0xF) // size is not a multiple of 16
		{
			int blcCnt = len / VMAC_NHBYTES;
			int alignedLn = blcCnt * VMAC_NHBYTES;

			if (alignedLn)
				vhash_update((unsigned char*)key, alignedLn, &(vhi.ctx));
//			ALIGN(16) unsigned char lastBlock[VMAC_NHBYTES + 16];
			int bytesRemaining = len - alignedLn;
			memcpy(vhi.aligned16Buff, (unsigned char*)key + alignedLn, bytesRemaining);
			int bytesRemaining16aligned = ( ( bytesRemaining >> 4) << 4 ) + 16;
			memset(vhi.aligned16Buff + bytesRemaining, 0, bytesRemaining16aligned - bytesRemaining);

			uint64_t tagl;
#if (VMAC_TAG_LEN == 128)
			*(uint32_t*)res = vhash(vhi.aligned16Buff, bytesRemaining, &tagl, &(vhi.ctx));
#elif (VMAC_TAG_LEN == 64)
			*(uint32_t*)res = vhash(vhi.aligned16Buff, bytesRemaining, &tagl, &(vhi.ctx));
#else
#error VMAC_TAG_LEN could be either 64 or 128
#endif
		}
		else // size is a multiple of 16
		{
			uint64_t tagl;
#if (VMAC_TAG_LEN == 128)
			*(uint32_t*)res = vhash((unsigned char *)key, len, &tagl, &(vhi.ctx));
#elif (VMAC_TAG_LEN == 64)
			*(uint32_t*)res = vhash((unsigned char *)key, len, &tagl, &(vhi.ctx));
#else
#error VMAC_TAG_LEN could be either 64 or 128
#endif
		}
#ifdef __arm__
	}
	else // unaligned data
	{
		unsigned char * buffAligned;
		unsigned char * buff = NULL;
		{
			if ( len < VHASH_initializer::MAX_BUFF_LN )
				buffAligned = vhi.aligned16Buff;
			else
			{
				buff = new unsigned char[ len + 128 ];
				buffAligned = (unsigned char*)( ( ( ( size_t(buff) ) >> 4 ) << 4 )  + 16 );
			}
			memcpy( buffAligned, key, len );
			memset( buffAligned + len, 0, 16 );
		}


		uint64_t tagl;
#if (VMAC_TAG_LEN == 128)
		*(uint32_t*)res = (uint32_t)vhash( (unsigned char *)buffAligned, len, &tagl, &(vhi.ctx) );
#elif (VMAC_TAG_LEN == 64)
		*(uint32_t*)res = (uint32_t)vhash( (unsigned char *)buffAligned, len, &tagl, &(vhi.ctx) );
#else
#error VMAC_TAG_LEN could be either 64 or 128
#endif


		if ( buff != NULL ) delete [] buff;
	}
#endif // __arm__
	//	vhash_abort(&(vhi.ctx));
}

void VHASH_PADDED_32_but_hashing(const void * key, int len, uint32_t seed, void * res)
{
	(void)seed; //unused
#ifdef __arm__
	if ((((uintptr_t)key) & 0xF) == 0) // aligned in memory
	{
#endif // __arm__
	if ((((uintptr_t)key) & 0xF) == 0) // aligned in memory
	{
		if (len & 0xF) // size is not a multiple of 16
		{
			int blcCnt = len / VMAC_NHBYTES;
			int alignedLn = blcCnt * VMAC_NHBYTES;
			int bytesRemaining = len - alignedLn;
			memcpy(vhi.aligned16Buff, (unsigned char*)key + alignedLn, bytesRemaining);
			int bytesRemaining16aligned = ((bytesRemaining >> 4) << 4) + 16;
			memset(vhi.aligned16Buff + bytesRemaining, 0, bytesRemaining16aligned - bytesRemaining);

			*(uint32_t*)res = 0;
		}
		else // size is a multiple of 16
		{
			*(uint32_t*)res = 0;
		}
	}
	else // unaligned data
	{
		unsigned char * buffAligned;
		unsigned char * buff = NULL;
		{
			if (len < VHASH_initializer::MAX_BUFF_LN)
				buffAligned = vhi.aligned16Buff;
			else
			{
				buff = new unsigned char[len + 128];
				buffAligned = (unsigned char*)((((size_t(buff)) >> 4) << 4) + 16);
			}
			memcpy(buffAligned, key, len);
			memset(buffAligned + len, 0, 16);
		}

		*(uint32_t*)res = 0;

		if (buff != NULL) delete[] buff;
	}
#ifdef __arm__
	}
	else // unaligned data
	{
		unsigned char * buffAligned;
		unsigned char * buff = NULL;
		{
			if ( len < VHASH_initializer::MAX_BUFF_LN )
				buffAligned = vhi.aligned16Buff;
			else
			{
				buff = new unsigned char[ len + 128 ];
				buffAligned = (unsigned char*)( ( ( ( size_t(buff) ) >> 4 ) << 4 )  + 16 );
			}
			memcpy( buffAligned, key, len );
			memset( buffAligned + len, 0, 16 );
		}
		if ( buff != NULL ) delete [] buff;
	}
#endif // __arm__
	//	vhash_abort(&(vhi.ctx));
}

void VHASH_64( const void * key, int len, uint32_t seed, void * res )
{
    uint64_t tagl;
#if (VMAC_TAG_LEN == 128)
    ctx.polytmp[0] = seed;
    vhash( (unsigned char *)key, len, &tagl, &ctx);
    *(uint32_t*)res = (uint32_t)tagl;
#elif (VMAC_TAG_LEN == 64)
    vhi.ctx.polytmp[0] = seed;
    *(uint64_t*)res = vhash( (unsigned char *)key, len, &tagl, &(vhi.ctx) );
#else
#error VMAC_TAG_LEN could be either 64 or 128
#endif
}

void VHASH_PADDED_64( const void * key, int len, uint64_t seed, void * res )
{
	(void) seed; //unused
	if (len & 0xF)
	{
		int blcCnt = len / VMAC_NHBYTES;
		int alignedLn = blcCnt * VMAC_NHBYTES;

		if (alignedLn) 
			vhash_update((unsigned char*)key, alignedLn, &(vhi.ctx));

		ALIGN(16) unsigned char lastBlock[VMAC_NHBYTES + 16];
		int bytesRemaining = len - alignedLn;
		memcpy(lastBlock, (unsigned char*)key + alignedLn, bytesRemaining);
		int bytesRemaining16aligned = ( ( bytesRemaining >> 4) << 4 ) + 16;
		memset(lastBlock + bytesRemaining, 0, bytesRemaining16aligned - bytesRemaining );

		uint64_t tagl;
#if (VMAC_TAG_LEN == 128)
		*(uint64_t*)res = vhash((unsigned char *)lastBlock, bytesRemaining, &tagl, &(vhi.ctx));
#elif (VMAC_TAG_LEN == 64)
		*(uint64_t*)res = vhash((unsigned char *)lastBlock, bytesRemaining, &tagl, &(vhi.ctx));
#else
#error VMAC_TAG_LEN could be either 64 or 128
#endif
	}
	else // size is a multiple of 16
	{
		uint64_t tagl;
#if (VMAC_TAG_LEN == 128)
		*(uint64_t*)res = vhash((unsigned char *)lastBlock, bytesRemaining + 1, &tagl, &(vhi.ctx));
#elif (VMAC_TAG_LEN == 64)
		*(uint64_t*)res = vhash((unsigned char *)key, len, &tagl, &(vhi.ctx));
#else
#error VMAC_TAG_LEN could be either 64 or 128
#endif
	}
//	vhash_abort(&(vhi.ctx));
}

void VHASH_PADDED_64_but_hashing(const void * key, int len, uint64_t seed, void * res)
{
	(void)seed; //unused
	if (len & 0xF)
	{
		int blcCnt = len / VMAC_NHBYTES;
		int alignedLn = blcCnt * VMAC_NHBYTES;

		ALIGN(16) unsigned char lastBlock[VMAC_NHBYTES + 16];
		int bytesRemaining = len - alignedLn;
		memcpy(lastBlock, (unsigned char*)key + alignedLn, bytesRemaining);
		int bytesRemaining16aligned = ((bytesRemaining >> 4) << 4) + 16;
		memset(lastBlock + bytesRemaining, 0, bytesRemaining16aligned - bytesRemaining);

		*(uint32_t*)res = (*(uint32_t*)lastBlock + bytesRemaining - 1);
	}
	else // size is a multiple of 16
	{
		*(uint32_t*)res = 0;
	}
	//	vhash_abort(&(vhi.ctx));
}

int VHASH_TestSpeedAlt( const void * key, int len, int iter )
{
	int dummy = 0;
    uint64_t tagl;
	for ( int i=0; i<iter; i++ )
	{
		if (len & 0xF)
		{
			int blcCnt = len / VMAC_NHBYTES;
			int alignedLn = blcCnt * VMAC_NHBYTES;

			if (alignedLn) 
				vhash_update((unsigned char*)key, alignedLn, &(vhi.ctx));

			ALIGN(16) unsigned char lastBlock[VMAC_NHBYTES + 16];
			int bytesRemaining = len - alignedLn;
			memcpy(lastBlock, (unsigned char*)key + alignedLn, bytesRemaining);
			int bytesRemaining16aligned = ( ( bytesRemaining >> 4) << 4 ) + 16;
			memset(lastBlock + bytesRemaining, 0, bytesRemaining16aligned - bytesRemaining );

			uint64_t tagl;
#if (VMAC_TAG_LEN == 128)
			dummy += vhash((unsigned char *)lastBlock, bytesRemaining, &tagl, &(vhi.ctx));
#elif (VMAC_TAG_LEN == 64)
			dummy += vhash((unsigned char *)lastBlock, bytesRemaining, &tagl, &(vhi.ctx));
#else
#error VMAC_TAG_LEN could be either 64 or 128
#endif
		}
		else // size is a multiple of 16
		{
			uint64_t tagl;
#if (VMAC_TAG_LEN == 128)
			*(uint64_t*)res = vhash((unsigned char *)lastBlock, bytesRemaining + 1, &tagl, &(vhi.ctx));
#elif (VMAC_TAG_LEN == 64)
			dummy += vhash((unsigned char *)key, len, &tagl, &(vhi.ctx));
#else
#error VMAC_TAG_LEN could be either 64 or 128
#endif
		}
	}
	return dummy;
}

void VHASH_init() {
  vmac_set_key((unsigned char*)"abcdefghijklmnop", &vhi.ctx);
}

#endif
