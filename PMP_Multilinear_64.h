/* -------------------------------------------------------------------------------
 * Copyright (c) 2014, Dmytro Ivanchykhin, Sergey Ignatchenko, Daniel Lemire
 * All rights reserved.
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met: 
 * 
 * 1. Redistributions of source code must retain the above copyright notice, this
 *    list of conditions and the following disclaimer. 
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution. 
 * 
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 * -------------------------------------------------------------------------------
 * 
 * PMP+-Multilinear hash family implementation
 * 
 * v.1.00    Apr-14-2014    Initial release
 * 
 * -------------------------------------------------------------------------------*/

// PMP_Multilinear.h: implementtation of a 64-bit version of PMP+-Multilinear hash family

#if !defined __PMP_MULTILINEAR_HASHER_64_H__
#define __PMP_MULTILINEAR_HASHER_64_H__
#define __MULTILINEARPRIMESTRINGHASHFUNCTOR_CPP_64_H__REVISION_ "$Rev: 461 $" /* for automated version information update; could be removed, if not desired */
#if !defined __arm__
#if (defined _WIN64) || (defined __x86_64__)


#include "PMP_Multilinear_common.h"
#ifdef PMPML_USE_SSE_64
#include <immintrin.h>
#endif



////    GENERAL STAFF    ////
/////////////////////////////

#if defined(_MSC_VER)

#include <intrin.h>
#define MultiplyWordLoHi(p0,p1,a,b) p0 = _umul128((a),(b),&(p1));

#else //  defined(_MSC_VER)

FORCE_INLINE
void MultiplyWordLoHi(uint64_t& rlo, uint64_t& rhi, uint64_t a, uint64_t b)
{
__asm__(
"    mulq  %[b]\n"
:"=d"(rhi),"=a"(rlo)
:"1"(a),[b]"rm"(b));
}

#endif // _MSC_VER

#define ADD_SHIFT_ADD_NORMALIZE( lo, hi ) {\
	uint32_t lohi = lo >> 32; \
	uint32_t hilo = hi; \
	uint32_t diff = lohi - hilo; \
	hi += diff; \
	lo = (uint32_t)lo + (((uint64_t)(uint32_t)hi) << 32 ); \
	hi >>= 32; \
}

#define ADD_SHIFT_ADD_NORMALIZE_TO_UPPER( lo, hi ) {\
	uint32_t lohi = lo >> 32; \
	uint32_t hilo = hi; \
	uint32_t diff = lohi - hilo; \
	hi += diff; \
	lo = (uint32_t)lo; \
}


////    DECLARATIONS    ////
////////////////////////////

#ifdef PMPML_CHUNK_LOOP_USE_TWO_ACCUMULATORS_64
#define PMPML_CHUNK_LOOP_INTRO_L0_64 \
	  ULARGE_INTEGER__XX ctr0, ctr1, ctr2; \
	  ctr0.QuadPart = constTerm.QuadPart; \
	  ctr1.QuadPart = 0; \
	  ctr2.QuadPart = 0;\
	  ULARGE_INTEGER__XX ctr2_0, ctr2_1, ctr2_2, ctr2_3; \
	  ctr2_0.QuadPart = 0; \
	  ctr2_1.QuadPart = 0;\
	  ctr2_2.QuadPart = 0; \
	  ctr2_3.QuadPart = 0;\
	  ULARGE_INTEGER__XX mulLow, mulHigh;
#else
#define PMPML_CHUNK_LOOP_INTRO_L0_64 \
	  ULARGE_INTEGER__XX ctr0, ctr1, ctr2; \
	  ctr0.QuadPart = constTerm.QuadPart; \
	  ctr1.QuadPart = 0; \
	  ctr2.QuadPart = 0;\
	  ULARGE_INTEGER__XX mulLow, mulHigh;
#endif // PMPML_CHUNK_LOOP_USE_TWO_ACCUMULATORS_64



////    MAIN EXECUTION BLOCKS (MUL-ADD-ADD)    ////
///////////////////////////////////////////////////

#if defined(_MSC_VER)

#define PMPML_CHUNK_LOOP_BODY_ULI_T1_64( i ) \
{ \
	MultiplyWordLoHi(mulLow.QuadPart, mulHigh.QuadPart, x[i], coeff[i]); \
	unsigned char carry = 0; \
	carry = _addcarry_u64(carry, ctr0.QuadPart, mulLow.QuadPart, &(ctr0.QuadPart)); \
	carry = _addcarry_u64(carry, ctr1.QuadPart, mulHigh.QuadPart, &(ctr1.QuadPart)); \
	_addcarry_u64(carry, ctr2.QuadPart, 0, &(ctr2.QuadPart)); \
}

#ifdef PMPML_CHUNK_LOOP_USE_TWO_ACCUMULATORS_64
#define PMPML_CHUNK_LOOP_BODY_ULI_T1_64__( i ) \
{ \
	MultiplyWordLoHi(mulLow.QuadPart, mulHigh.QuadPart, x[i], coeff[i]); \
	unsigned char carry = 0; \
	carry = _addcarry_u64(carry, ctr2_0.QuadPart, mulLow.QuadPart, &(ctr2_0.QuadPart)); \
	carry = _addcarry_u64(carry, ctr2_1.QuadPart, mulHigh.QuadPart, &(ctr2_1.QuadPart)); \
	_addcarry_u64(carry, ctr2_2.QuadPart, 0, &(ctr2_2.QuadPart)); \
}
#define compensate { \
	unsigned char carry = 0; \
	carry = _addcarry_u64(carry, ctr2_0.QuadPart, ctr0.QuadPart, &(ctr0.QuadPart)); \
	carry = _addcarry_u64(carry, ctr2_1.QuadPart, ctr1.QuadPart, &(ctr1.QuadPart)); \
	_addcarry_u64(carry, ctr2_2.QuadPart, ctr2.QuadPart, &(ctr2.QuadPart)); \
}
#endif // PMPML_CHUNK_LOOP_USE_TWO_ACCUMULATORS_64

#else //  defined(_MSC_VER)

#ifdef __clang__
// clang cannot work properly with "g" and silently produces hardly-workging code, if "g" is specified; see, for instance, here:
// http://stackoverflow.com/questions/16850309/clang-llvm-inline-assembly-multiple-constraints-with-useless-spills-reload
// To avoid 3x performance hit we have to specify sources/destinations
#define PMPML_CHUNK_LOOP_BODY_ULI_T1_64( i ) { \
uint64_t rhi;  /*Dummy variable to tell the compiler that the register rax is input and clobbered but not actually output; see assembler code below. Better syntactic expression is very welcome.*/ \
__asm__( "mulq %5\n" \
        "addq %%rax, %0\n" \
        "adcq %%rdx, %1\n" \
        "adcq $0, %2\n" \
        : "+r" (ctr0.QuadPart), "+r" (ctr1.QuadPart), "+r" (ctr2.QuadPart), "=a" (rhi) \
        :"a"(x[i]), "m"(coeff[ i ]) : "rdx", "cc" ); \
}
#else // __clang__
#define PMPML_CHUNK_LOOP_BODY_ULI_T1_64( i ) { \
uint64_t rhi;  /*Dummy variable to tell the compiler that the register rax is input and clobbered but not actually output; see assembler code below. Better syntactic expression is very welcome.*/ \
__asm__( "mulq %5\n" \
        "addq %%rax, %0\n" \
        "adcq %%rdx, %1\n" \
        "adcq $0, %2\n" \
        : "+g" (ctr0.QuadPart), "+g" (ctr1.QuadPart), "+g" (ctr2.QuadPart), "=a" (rhi) \
        :"a"(x[i]), "g"(coeff[ i ]) : "rdx", "cc" ); \
}
#endif  // __clang__

#ifdef PMPML_CHUNK_LOOP_USE_TWO_ACCUMULATORS_64

#ifdef __clang__
// see remark above

#error NOT IMPLEMENTED

#else // __clang__ not defined

#define PMPML_CHUNK_LOOP_BODY_ULI_T1_64__( ii ) { \
uint64_t rhi;  /*Dummy variable to tell the compiler that the register rax is input and clobbered but not actually output; see assembler code below. Better syntactic expression is very welcome.*/ \
__asm__( "mulq %5\n" \
        "addq %%rax, %0\n" \
        "adcq %%rdx, %1\n" \
        "adcq $0, %2\n" \
        : "+g" (ctr2_0), "+g" (ctr2_1), "+g" (ctr2_2), "=a" (rhi) \
        :"a"(x[ii]), "g"(coeff[ ii ]) : "rdx", "cc" ); \
}

#define compensate { \
__asm__("addq %3, %0\n" \
        "adcq %4, %1\n" \
        "adcq %5, %2\n" \
        : "+g" (ctr0), "+g" (ctr1), "+g" (ctr2) \
        : "g"(ctr2_0), "g"(ctr2_1), "g"(ctr2_2) : "cc" ); \
}

#endif // __clang__

#endif // PMPML_CHUNK_LOOP_USE_TWO_ACCUMULATORS_64

#endif //  defined(_MSC_VER)


#ifdef PMPML_CHUNK_LOOP_USE_TWO_ACCUMULATORS_64

#define PMPML_CHUNK_LOOP_BODY_ULI_T1_64_FIRST PMPML_CHUNK_LOOP_BODY_ULI_T1_64__
#define PMPML_CHUNK_LOOP_BODY_ULI_T1_64_SECOND PMPML_CHUNK_LOOP_BODY_ULI_T1_64
#define _compensate_ compensate

#else // PMPML_CHUNK_LOOP_USE_TWO_ACCUMULATORS_64

#define PMPML_CHUNK_LOOP_BODY_ULI_T1_64_FIRST PMPML_CHUNK_LOOP_BODY_ULI_T1_64
#define PMPML_CHUNK_LOOP_BODY_ULI_T1_64_SECOND PMPML_CHUNK_LOOP_BODY_ULI_T1_64
#define _compensate_

#endif // PMPML_CHUNK_LOOP_USE_TWO_ACCUMULATORS_64



////    OTHER EXECUTION BLOCKS    ////
//////////////////////////////////////

#ifdef _MSC_VER

#define PMPML_CHUNK_LOOP_BODY_ULI_T1_64_LAST( size ) \
{ \
	MultiplyWordLoHi(mulLow.QuadPart, mulHigh.QuadPart, xLast, coeff[size]); \
	unsigned char carry = 0; \
	carry = _addcarry_u64(carry, ctr0.QuadPart, mulLow.QuadPart, &(ctr0.QuadPart)); \
	carry = _addcarry_u64(carry, ctr1.QuadPart, mulHigh.QuadPart, &(ctr1.QuadPart)); \
	_addcarry_u64(carry, ctr2.QuadPart, 0, &(ctr2.QuadPart)); \
}

#define PMPML_CHUNK_LOOP_BODY_ULI_T2_64( i ) \
{ \
	MultiplyWordLoHi(mulLow.QuadPart, mulHigh.QuadPart, x[i].LowPart, coeff[i]); \
	if ( x[ i ].HighPart == 0 ); \
	else \
		mulHigh.QuadPart += x[i].HighPart * coeff[i]; \
	unsigned char carry = 0; \
	carry = _addcarry_u64(carry, ctr0.QuadPart, mulLow.QuadPart, &(ctr0.QuadPart)); \
	carry = _addcarry_u64(carry, ctr1.QuadPart, mulHigh.QuadPart, &(ctr1.QuadPart)); \
	_addcarry_u64(carry, ctr2.QuadPart, 0, &(ctr2.QuadPart)); \
}

#define PMPML_CHUNK_LOOP_BODY_ULI_ADD_COEFF_64( i ) \
{ \
	unsigned char carry = 0; \
	carry = _addcarry_u64(carry, c_ctr0.QuadPart, coeff[i], &(c_ctr0.QuadPart)); \
	_addcarry_u64(carry, c_ctr1.QuadPart, 0, &(c_ctr1.QuadPart)); \
}

#define PMPML_CHUNK_LOOP_BODY_ULI_T2_AND_ADD_COEFF_64( i ) \
{ \
	unsigned char carry = 0; \
	carry = _addcarry_u64(carry, c_ctr0.QuadPart, coeff[i], &(c_ctr0.QuadPart)); \
	_addcarry_u64(carry, c_ctr1.QuadPart, 0, &(c_ctr1.QuadPart)); \
	MultiplyWordLoHi(mulLow.QuadPart, mulHigh.QuadPart, x[i].LowPart, coeff[i]); \
	if ( x[ i ].HighPart == 0 ); \
	else \
		mulHigh.QuadPart += x[i].HighPart * coeff[i]; \
	carry = 0; \
	carry = _addcarry_u64(carry, ctr0.QuadPart, mulLow.QuadPart, &(ctr0.QuadPart)); \
	carry = _addcarry_u64(carry, ctr1.QuadPart, mulHigh.QuadPart, &(ctr1.QuadPart)); \
	_addcarry_u64(carry, ctr2.QuadPart, 0, &(ctr2.QuadPart)); \
}

#define PMPML_CHUNK_LOOP_BODY_ULI_T2_AND_ADD_SUM_OF_COEFF_64 \
	unsigned char carry = 0; \
	MultiplyWordLoHi(mulLow.QuadPart, mulHigh.QuadPart, c_ctr0.QuadPart, prevConstTerm); \
	carry = 0; \
	carry = _addcarry_u64(carry, ctr0.QuadPart, mulLow.QuadPart, &(ctr0.QuadPart)); \
	carry = _addcarry_u64(carry, ctr1.QuadPart, mulHigh.QuadPart, &(ctr1.QuadPart)); \
	_addcarry_u64(carry, ctr2.QuadPart, 0, &(ctr2.QuadPart)); \
	MultiplyWordLoHi(mulLow.QuadPart, mulHigh.QuadPart, c_ctr1.QuadPart, prevConstTerm); \
	carry = 0; \
	carry = _addcarry_u64(carry, ctr1.QuadPart, mulLow.QuadPart, &(ctr1.QuadPart)); \
	carry = _addcarry_u64(carry, ctr2.QuadPart, mulHigh.QuadPart, &(ctr2.QuadPart)); 

#else // _MSC_VER

#ifdef __clang__
#define PMPML_CHUNK_LOOP_BODY_ULI_T1_64_LAST( size ) { \
uint64_t rhi;  /*Dummy variable to tell the compiler that the register rax is input and clobbered but not actually output; see assembler code below. Better syntactic expression is very welcome.*/ \
__asm__( "mulq %5\n" \
        "addq %%rax, %0\n" \
        "adcq %%rdx, %1\n" \
        "adcq $0, %2\n" \
        : "+r" (ctr0.QuadPart), "+r" (ctr1.QuadPart), "+r" (ctr2.QuadPart), "=a" (rhi) \
        :"a"(xLast), "m"(coeff[size]) : "rdx", "cc" ); \
}
#else // __clang__
#define PMPML_CHUNK_LOOP_BODY_ULI_T1_64_LAST( size ) { \
uint64_t rhi;  /*Dummy variable to tell the compiler that the register rax is input and clobbered but not actually output; see assembler code below. Better syntactic expression is very welcome.*/ \
__asm__( "mulq %5\n" \
        "addq %%rax, %0\n" \
        "adcq %%rdx, %1\n" \
        "adcq $0, %2\n" \
        : "+g" (ctr0.QuadPart), "+g" (ctr1.QuadPart), "+g" (ctr2.QuadPart), "=a" (rhi) \
        :"a"(xLast), "g"(coeff[size]) : "rdx", "cc" ); \
}
#endif // __clang__

#define PMPML_CHUNK_LOOP_BODY_ULI_T2_64( i ) \
{ \
	if ( _LIKELY_BRANCH_( x[ i ].HighPart == 0 ) ) \
	{ \
uint64_t rhi;  /*Dummy variable to tell the compiler that the register rax is input and clobbered but not actually output; see assembler code below. Better syntactic expression is very welcome.*/ \
__asm__( "mulq %5\n" \
        "addq %%rax, %0\n" \
        "adcq %%rdx, %1\n" \
        "adcq $0, %2\n" \
        : "+g" (ctr0.QuadPart), "+g" (ctr1.QuadPart), "+g" (ctr2.QuadPart), "=a" (rhi) \
        :"a"(x[i].LowPart), "g"(coeff[i]) : "rdx", "cc" ); \
	} \
	else \
	{ \
		MultiplyWordLoHi(mulLow.QuadPart, mulHigh.QuadPart, x[i].LowPart, coeff[i]); \
		mulHigh.QuadPart += x[i].HighPart * coeff[i]; \
__asm__("addq %3, %0\n" \
        "adcq %4, %1\n" \
        "adcq $0, %2\n" \
        : "+r" (ctr0.QuadPart), "+r" (ctr1.QuadPart), "+r" (ctr2.QuadPart) \
        :"a"(mulLow.QuadPart), "m"(mulHigh.QuadPart) : "cc" ); \
	} \
}


#define PMPML_CHUNK_LOOP_BODY_ULI_ADD_COEFF_64( i ) \
{ \
__asm("addq %2, %0\n" \
      "adcq $0, %1\n" \
      : "+g" (c_ctr0.QuadPart), "+g" (c_ctr1.QuadPart) \
      : "g" (coeff[i]) : "cc", "memory" );}

#define PMPML_CHUNK_LOOP_BODY_ULI_T2_AND_ADD_COEFF_64( i ) \
{ \
__asm("addq %2, %0\n" \
      "adcq $0, %1\n" \
      : "+g" (c_ctr0.QuadPart), "+g" (c_ctr1.QuadPart) \
      : "g" (coeff[i]) : "cc", "memory" ); \
	if ( _LIKELY_BRANCH_( x[ i ].HighPart == 0 ) ) \
	{ \
uint64_t rhi;  /*Dummy variable to tell the compiler that the register rax is input and clobbered but not actually output; see assembler code below. Better syntactic expression is very welcome.*/ \
__asm__( "mulq %5\n" \
        "addq %%rax, %0\n" \
        "adcq %%rdx, %1\n" \
        "adcq $0, %2\n" \
        : "+g" (ctr0.QuadPart), "+g" (ctr1.QuadPart), "+g" (ctr2.QuadPart), "=a" (rhi) \
        :"a"(x[i].LowPart), "g"(coeff[i]) : "rdx", "cc" ); \
	} \
	else \
	{ \
		MultiplyWordLoHi(mulLow.QuadPart, mulHigh.QuadPart, x[i].LowPart, coeff[i]); \
		mulHigh.QuadPart += x[i].HighPart * coeff[i]; \
__asm__("addq %3, %0\n" \
        "adcq %4, %1\n" \
        "adcq $0, %2\n" \
        : "+r" (ctr0.QuadPart), "+r" (ctr1.QuadPart), "+r" (ctr2.QuadPart) \
        :"a"(mulLow.QuadPart), "m"(mulHigh.QuadPart) : "cc" ); \
	} \
}

#define PMPML_CHUNK_LOOP_BODY_ULI_T2_AND_ADD_SUM_OF_COEFF_64 { \
uint64_t rhi;  /*Dummy variable to tell the compiler that the register rax is input and clobbered but not actually output; see assembler code below. Better syntactic expression is very welcome.*/ \
__asm__( "mulq %5\n" \
        "addq %%rax, %0\n" \
        "adcq %%rdx, %1\n" \
        "adcq $0, %2\n" \
        : "+g" (ctr0.QuadPart), "+g" (ctr1.QuadPart), "+g" (ctr2.QuadPart), "=a" (rhi) \
        :"a"(c_ctr0.QuadPart), "g"(prevConstTerm) : "rdx", "cc" ); \
__asm__( "mulq %4\n" \
        "addq %%rax, %0\n" \
        "adcq %%rdx, %1\n" \
        : "+g" (ctr1.QuadPart), "+g" (ctr2.QuadPart), "=a" (rhi) \
        :"a"(c_ctr1.QuadPart), "g"(prevConstTerm) : "rdx", "cc" ); \
}

#endif // _MSC_VER



////    MOD PRIME REDUCTION    ////
///////////////////////////////////

#define PMPML_CHUNK_LOOP_PRE_REDUCE_L0_64

#define PMPML_CHUNK_REDUCE_128_TO_64 \
{ \
	uint64_t hi, lo; \
	MultiplyWordLoHi(lo, hi, ctr1.QuadPart, 13); \
	uint64_t part = ctr2.QuadPart * 169 + hi * 13 + 13; \
	ctr0.QuadPart += part; \
	ctr1.QuadPart = 1 + (ctr0.QuadPart < part); \
	ctr1.QuadPart -= (ctr0.QuadPart < lo); \
	ctr0.QuadPart -= lo; \
	if ( _LIKELY_BRANCH_( ctr0.QuadPart >= 26) ) { ctr0.QuadPart -= ctr1.QuadPart * 13; ctr1.QuadPart = 0; } \
	else \
	{ \
		ctr0.QuadPart -= ctr1.QuadPart * 13; \
		if ( ctr0.QuadPart < 26 ) ctr1.QuadPart = 0; \
		else \
		{ \
			ctr0.QuadPart += 13; \
			if ( ctr0.QuadPart < 13 ) ctr1.QuadPart = 1; \
			else ctr1.QuadPart = 0; \
		} \
	} \
}

#define PMPML_CHUNK_REDUCE_128_TO_64____ \
{ \
	_compensate_ \
	uint64_t hi, lo; \
	MultiplyWordLoHi(lo, hi, ctr1.QuadPart, 13); \
	uint64_t part = ctr2.QuadPart * 169 + hi * 13 + 13; \
	ctr0.QuadPart += part; \
	ctr1.QuadPart = 1 + (ctr0.QuadPart < part); \
	ctr1.QuadPart -= (ctr0.QuadPart < lo); \
	ctr0.QuadPart -= lo; \
	if ( _LIKELY_BRANCH_( ctr0.QuadPart >= 26) ) { ctr0.QuadPart -= ctr1.QuadPart * 13; ctr1.QuadPart = 0; } \
	else \
	{ \
		ctr0.QuadPart -= ctr1.QuadPart * 13; \
		if ( ctr0.QuadPart < 26 ) ctr1.QuadPart = 0; \
		else \
		{ \
			ctr0.QuadPart += 13; \
			if ( ctr0.QuadPart < 13 ) ctr1.QuadPart = 1; \
			else ctr1.QuadPart = 0; \
		} \
	} \
}

#define PMPML_CHUNK_REDUCE_128_TO_64_AND_RETURN \
{ \
	uint64_t hi, lo; \
	MultiplyWordLoHi(lo, hi, ctr1.QuadPart, 13); \
	uint64_t part = ctr2.QuadPart * 169 + hi * 13 + 13; \
	ctr0.QuadPart += part; \
	ctr1.QuadPart = 1 + (ctr0.QuadPart < part); \
	ctr1.QuadPart -= (ctr0.QuadPart < lo); \
	ctr0.QuadPart -= lo; \
	if ( _LIKELY_BRANCH_( ctr0.QuadPart >= 26) ) { ctr0.QuadPart -= ctr1.QuadPart * 13; return fmix64_short( ctr0.QuadPart ); } \
	else \
	{ \
		ctr0.QuadPart -= ctr1.QuadPart * 13; \
		if ( ctr0.QuadPart < 26 ) return fmix64_short( ctr0.QuadPart ); \
		else \
		{ \
			ctr0.QuadPart += 13; \
			return fmix64_short( ctr0.QuadPart ); \
		} \
	} \
}




////   EXECUTION LOGIC    ////
//////////////////////////////


class PMP_Multilinear_Hasher_64
{
  private:
  random_data_for_PMPML_64* curr_rd;

  // calls to be done from LEVEL=0
  FORCE_INLINE void hash_of_string_chunk_compact( const uint64_t* coeff, ULARGE_INTEGER__XX constTerm, const uint64_t* x, ULARGELARGE_INTEGER__XX& ret ) const
  {
	PMPML_CHUNK_LOOP_INTRO_L0_64

#ifdef PMPML_USE_SSE_64

	__m256i sse_ctr0_0, sse_ctr0_1, sse_ctr1, sse_ctr2, sse_ctr3_0, sse_ctr3_1, a, a_shifted, a_low, data, data_low, product, temp, mask_low;
	sse_ctr0_0 = _mm256_setzero_si256 (); // Sets the 128-bit value to zero.
	sse_ctr0_1 = _mm256_setzero_si256 (); // Sets the 128-bit value to zero.
	sse_ctr1 = _mm256_setzero_si256 ();
	sse_ctr2 = _mm256_setzero_si256 ();
	sse_ctr3_0 = _mm256_setzero_si256 ();
	sse_ctr3_1 = _mm256_setzero_si256 ();
	mask_low = _mm256_set_epi32 ( 0, -1, 0 , -1, 0, -1, 0 , -1 );

#if ( PMPML_CHUNK_SIZE_64 >= 16 )
	for ( uint64_t i=0; i<(PMPML_CHUNK_SIZE_64); i+=16 )
#elif  ( PMPML_CHUNK_SIZE_64 >= 8 )
	for ( uint64_t i=0; i<(PMPML_CHUNK_SIZE_64); i+=8 )
#else
#error PMPML_USE_SSE_64 is incompatible with PMPML_CHUNK_SIZE_64 < 8 in a current implementation
#endif
	{
#if ( PMPML_CHUNK_SIZE_64 >= 8 )
		a = _mm256_load_si256 ((__m256i *)(coeff+i)); // Loads 128-bit value. Address p must be 16-byte aligned.
		data = _mm256_loadu_si256 ((__m256i *)(x+i)); // Loads 128-bit value. Address p does not need be 16-byte aligned.

		// lower 32 bits
		a_low = _mm256_and_si256 ( mask_low, a );
		data_low = _mm256_and_si256 ( mask_low, data );
		product = _mm256_mul_epu32 ( data_low, a_low); // A 128-bit value that contains two 64-bit unsigned integers. The result can be expressed by the following equations. r0 := a0 * b0; r1 := a2 * b2
		sse_ctr0_0 = _mm256_add_epi64 ( sse_ctr0_0, product );//sse_ctr0 = _mm256_add_epi64 ( sse_ctr0, temp );
		temp = _mm256_srli_epi64( product, 32 ); // Shifts the 2 signed or unsigned 64-bit integers in a right by count bits while shifting in zeros.
		sse_ctr0_1 = _mm256_add_epi64 ( sse_ctr0_1, temp );
		//temp = _mm256_and_si256 ( mask_low, product );

		PMPML_CHUNK_LOOP_BODY_ULI_T1_64_FIRST( 4 + i )
		// first cross
		a_shifted = _mm256_srli_epi64( a, 32 );
		product = _mm256_mul_epu32 ( data_low, a_shifted ); // A 128-bit value that contains two 64-bit unsigned integers. The result can be expressed by the following equations. r0 := a0 * b0; r1 := a2 * b2
		sse_ctr1 = _mm256_add_epi64 ( sse_ctr1, product );//sse_ctr1 = _mm256_add_epi64 ( sse_ctr1, temp );
		temp = _mm256_srli_epi64( product, 32 ); // Shifts the 2 signed or unsigned 64-bit integers in a right by count bits while shifting in zeros.
		sse_ctr2 = _mm256_add_epi64 ( sse_ctr2, temp );
		//temp = _mm256_and_si256 ( mask_low, product );

		PMPML_CHUNK_LOOP_BODY_ULI_T1_64_SECOND( 5 + i )
		// second cross
		data = _mm256_srli_epi64( data, 32 );
		product = _mm256_mul_epu32 ( data, a_low ); // A 128-bit value that contains two 64-bit unsigned integers. The result can be expressed by the following equations. r0 := a0 * b0; r1 := a2 * b2
		sse_ctr1 = _mm256_add_epi64 ( sse_ctr1, product );//sse_ctr1 = _mm256_add_epi64 ( sse_ctr1, temp );
		temp = _mm256_srli_epi64( product, 32 ); // Shifts the 2 signed or unsigned 64-bit integers in a right by count bits while shifting in zeros.
		sse_ctr2 = _mm256_add_epi64 ( sse_ctr2, temp );
		//temp = _mm256_and_si256 ( mask_low, product );

		PMPML_CHUNK_LOOP_BODY_ULI_T1_64_FIRST( 6 + i )
		// upper 32 bits
		product = _mm256_mul_epu32 ( data, a_shifted ); // A 128-bit value that contains two 64-bit unsigned integers. The result can be expressed by the following equations. r0 := a0 * b0; r1 := a2 * b2
		sse_ctr3_0 = _mm256_add_epi64 ( sse_ctr3_0, product );//sse_ctr2 = _mm256_add_epi64 ( sse_ctr2, temp );
		temp = _mm256_srli_epi64( product, 32 ); // Shifts the 2 signed or unsigned 64-bit integers in a right by count bits while shifting in zeros.
		sse_ctr3_1 = _mm256_add_epi64 ( sse_ctr3_1, temp );
		//temp = _mm256_and_si256 ( mask_low, product );
		PMPML_CHUNK_LOOP_BODY_ULI_T1_64_SECOND( 7 + i )		
#endif

#if ( PMPML_CHUNK_SIZE_64 >= 16 )
		a = _mm256_load_si256 ((__m256i *)(coeff+i+8)); // Loads 128-bit value. Address p must be 16-byte aligned.
		data = _mm256_loadu_si256 ((__m256i *)(x+i+8)); // Loads 128-bit value. Address p does not need be 16-byte aligned.

		// lower 32 bits
		a_low = _mm256_and_si256 ( mask_low, a );
		data_low = _mm256_and_si256 ( mask_low, data );
		product = _mm256_mul_epu32 ( data_low, a_low); // A 128-bit value that contains two 64-bit unsigned integers. The result can be expressed by the following equations. r0 := a0 * b0; r1 := a2 * b2
		sse_ctr0_0 = _mm256_add_epi64 ( sse_ctr0_0, product );//sse_ctr0 = _mm256_add_epi64 ( sse_ctr0, temp );
		temp = _mm256_srli_epi64( product, 32 ); // Shifts the 2 signed or unsigned 64-bit integers in a right by count bits while shifting in zeros.
		sse_ctr0_1 = _mm256_add_epi64 ( sse_ctr0_1, temp );
		//temp = _mm256_and_si256 ( mask_low, product );

		PMPML_CHUNK_LOOP_BODY_ULI_T1_64_FIRST( 12 + i )

		// first cross
		a_shifted = _mm256_srli_epi64( a, 32 );
		product = _mm256_mul_epu32 ( data_low, a_shifted ); // A 128-bit value that contains two 64-bit unsigned integers. The result can be expressed by the following equations. r0 := a0 * b0; r1 := a2 * b2
		sse_ctr1 = _mm256_add_epi64 ( sse_ctr1, product );//sse_ctr1 = _mm256_add_epi64 ( sse_ctr1, temp );
		temp = _mm256_srli_epi64( product, 32 ); // Shifts the 2 signed or unsigned 64-bit integers in a right by count bits while shifting in zeros.
		sse_ctr2 = _mm256_add_epi64 ( sse_ctr2, temp );
		//temp = _mm256_and_si256 ( mask_low, product );

		PMPML_CHUNK_LOOP_BODY_ULI_T1_64_SECOND( 13 + i )

		// second cross
		data = _mm256_srli_epi64( data, 32 );
		product = _mm256_mul_epu32 ( data, a_low ); // A 128-bit value that contains two 64-bit unsigned integers. The result can be expressed by the following equations. r0 := a0 * b0; r1 := a2 * b2
		sse_ctr1 = _mm256_add_epi64 ( sse_ctr1, product );//sse_ctr1 = _mm256_add_epi64 ( sse_ctr1, temp );
		temp = _mm256_srli_epi64( product, 32 ); // Shifts the 2 signed or unsigned 64-bit integers in a right by count bits while shifting in zeros.
		sse_ctr2 = _mm256_add_epi64 ( sse_ctr2, temp );
		//temp = _mm256_and_si256 ( mask_low, product );

		PMPML_CHUNK_LOOP_BODY_ULI_T1_64_FIRST( 14 + i )
		// upper 32 bits
		product = _mm256_mul_epu32 ( data, a_shifted ); // A 128-bit value that contains two 64-bit unsigned integers. The result can be expressed by the following equations. r0 := a0 * b0; r1 := a2 * b2
		sse_ctr3_0 = _mm256_add_epi64 ( sse_ctr3_0, product );//sse_ctr2 = _mm256_add_epi64 ( sse_ctr2, temp );
		temp = _mm256_srli_epi64( product, 32 ); // Shifts the 2 signed or unsigned 64-bit integers in a right by count bits while shifting in zeros.
		sse_ctr3_1 = _mm256_add_epi64 ( sse_ctr3_1, temp );
		//temp = _mm256_and_si256 ( mask_low, product );
		PMPML_CHUNK_LOOP_BODY_ULI_T1_64_SECOND( 15 + i )		
#endif
	}

#if (PMPML_CHUNK_OPTIMIZATION_TYPE_64 == 1)
	uint64_t t0_0, t0_1, t1, t2, t3_0, t3_1;
	t0_0 = ((uint64_t*)(&sse_ctr0_0))[0] + ((uint64_t*)(&sse_ctr0_0))[1] + ((uint64_t*)(&sse_ctr0_0))[2] + ((uint64_t*)(&sse_ctr0_0))[3];
	t0_1 = ((uint64_t*)(&sse_ctr0_1))[0] + ((uint64_t*)(&sse_ctr0_1))[1] + ((uint64_t*)(&sse_ctr0_1))[2] + ((uint64_t*)(&sse_ctr0_1))[3];
	t1 = ((uint64_t*)(&sse_ctr1))[0] + ((uint64_t*)(&sse_ctr1))[1] + ((uint64_t*)(&sse_ctr1))[2] + ((uint64_t*)(&sse_ctr1))[3];
	t2 = ((uint64_t*)(&sse_ctr2))[0] + ((uint64_t*)(&sse_ctr2))[1] + ((uint64_t*)(&sse_ctr2))[2] + ((uint64_t*)(&sse_ctr2))[3];
	t3_0 = ((uint64_t*)(&sse_ctr3_0))[0] + ((uint64_t*)(&sse_ctr3_0))[1] + ((uint64_t*)(&sse_ctr3_0))[2] + ((uint64_t*)(&sse_ctr3_0))[3];
	t3_1 = ((uint64_t*)(&sse_ctr3_1))[0] + ((uint64_t*)(&sse_ctr3_1))[1] + ((uint64_t*)(&sse_ctr3_1))[2] + ((uint64_t*)(&sse_ctr3_1))[3];

	ADD_SHIFT_ADD_NORMALIZE_TO_UPPER( t0_0, t0_1 )
	ADD_SHIFT_ADD_NORMALIZE_TO_UPPER( t1, t2 )
	ADD_SHIFT_ADD_NORMALIZE_TO_UPPER( t3_0, t3_1 )

	uint64_t add_sse1, add_sse2;

	t1 += t0_1;
	add_sse1 = t0_0 + ( ((uint64_t)(uint32_t)t1) << 32 );
	ctr0.QuadPart += add_sse1;
	add_sse2 = ctr0.QuadPart < add_sse1;

	t2 += t3_0 + (t1>>32);
	t3_1 += t2>>32;

	add_sse2 += (uint32_t)t2 + ( ( (uint64_t)(uint32_t)t3_1 ) << 32 );
	ctr1.QuadPart += add_sse2;

	ctr2.QuadPart += (t3_1 >> 32) + (ctr1.QuadPart < add_sse2);


/*	ctr0.LowPart = (uint32_t)t0_0;
	uint64_t upper64 = t0_1 + (t0_0>>32) + (uint64_t)(uint32_t)t1;
	ctr0.HighPart = (uint32_t)upper64;

	upper64 = (upper64>>32) + (t1>>32) + t2 + (uint32_t)t3_0;
	ctr1.LowPart = (uint32_t)upper64;

	upper64 = (upper64>>32) + (t3_0>>32) + (uint32_t)t3_1;
	ctr1.HighPart += (uint32_t)upper64;

	ctr2.QuadPart = (upper64>>32) + (t3_1>>32);*/
#elif (PMPML_CHUNK_OPTIMIZATION_TYPE_64 == 2)
#error Not yet implemented
#else
#error unxpected PMPML_CHUNK_OPTIMIZATION_TYPE_64
#endif

#else // PMPML_USE_SSE_64

	for ( uint64_t i=0; i<(PMPML_CHUNK_SIZE_64); i+=32 )
	{
		PMPML_CHUNK_LOOP_BODY_ULI_T1_64_FIRST( 0 + i )
		PMPML_CHUNK_LOOP_BODY_ULI_T1_64_SECOND( 1 + i )
		PMPML_CHUNK_LOOP_BODY_ULI_T1_64_FIRST( 2 + i )
		PMPML_CHUNK_LOOP_BODY_ULI_T1_64_SECOND( 3 + i )
#if ( PMPML_CHUNK_SIZE_64 > 4 )
		PMPML_CHUNK_LOOP_BODY_ULI_T1_64_FIRST( 4 + i )
		PMPML_CHUNK_LOOP_BODY_ULI_T1_64_SECOND( 5 + i )
		PMPML_CHUNK_LOOP_BODY_ULI_T1_64_FIRST( 6 + i )
		PMPML_CHUNK_LOOP_BODY_ULI_T1_64_SECOND( 7 + i )
#endif
#if ( PMPML_CHUNK_SIZE_64 > 8 )
		PMPML_CHUNK_LOOP_BODY_ULI_T1_64_FIRST( 8 + i )
		PMPML_CHUNK_LOOP_BODY_ULI_T1_64_SECOND( 9 + i )
		PMPML_CHUNK_LOOP_BODY_ULI_T1_64_FIRST( 10 + i )
		PMPML_CHUNK_LOOP_BODY_ULI_T1_64_SECOND( 11 + i )
		PMPML_CHUNK_LOOP_BODY_ULI_T1_64_FIRST( 12 + i )
		PMPML_CHUNK_LOOP_BODY_ULI_T1_64_SECOND( 13 + i )
		PMPML_CHUNK_LOOP_BODY_ULI_T1_64_FIRST( 14 + i )
		PMPML_CHUNK_LOOP_BODY_ULI_T1_64_SECOND( 15 + i )
#endif
#if ( PMPML_CHUNK_SIZE_64 > 16 )
		PMPML_CHUNK_LOOP_BODY_ULI_T1_64_FIRST( 16 + i )
		PMPML_CHUNK_LOOP_BODY_ULI_T1_64_SECOND( 17 + i )
		PMPML_CHUNK_LOOP_BODY_ULI_T1_64_FIRST( 18 + i )
		PMPML_CHUNK_LOOP_BODY_ULI_T1_64_SECOND( 19 + i )
		PMPML_CHUNK_LOOP_BODY_ULI_T1_64_FIRST( 20 + i )
		PMPML_CHUNK_LOOP_BODY_ULI_T1_64_SECOND( 21 + i )
		PMPML_CHUNK_LOOP_BODY_ULI_T1_64_FIRST( 22 + i )
		PMPML_CHUNK_LOOP_BODY_ULI_T1_64_SECOND( 23 + i )
		PMPML_CHUNK_LOOP_BODY_ULI_T1_64_FIRST( 24 + i )
		PMPML_CHUNK_LOOP_BODY_ULI_T1_64_SECOND( 25 + i )
		PMPML_CHUNK_LOOP_BODY_ULI_T1_64_FIRST( 26 + i )
		PMPML_CHUNK_LOOP_BODY_ULI_T1_64_SECOND( 27 + i )
		PMPML_CHUNK_LOOP_BODY_ULI_T1_64_FIRST( 28 + i )
		PMPML_CHUNK_LOOP_BODY_ULI_T1_64_SECOND( 29 + i )
		PMPML_CHUNK_LOOP_BODY_ULI_T1_64_FIRST( 30 + i )
		PMPML_CHUNK_LOOP_BODY_ULI_T1_64_SECOND( 31 + i )
#endif
#if 0
#if ( PMPML_CHUNK_SIZE_64 > 32 )
		PMPML_CHUNK_LOOP_BODY_ULI_T1_64_FIRST( 32 + i )
		PMPML_CHUNK_LOOP_BODY_ULI_T1_64_SECOND( 33 + i )
		PMPML_CHUNK_LOOP_BODY_ULI_T1_64_FIRST( 34 + i )
		PMPML_CHUNK_LOOP_BODY_ULI_T1_64_SECOND( 35 + i )
		PMPML_CHUNK_LOOP_BODY_ULI_T1_64_FIRST( 36 + i )
		PMPML_CHUNK_LOOP_BODY_ULI_T1_64_SECOND( 37 + i )
		PMPML_CHUNK_LOOP_BODY_ULI_T1_64_FIRST( 38 + i )
		PMPML_CHUNK_LOOP_BODY_ULI_T1_64_SECOND( 39 + i )
		PMPML_CHUNK_LOOP_BODY_ULI_T1_64_FIRST( 40 + i )
		PMPML_CHUNK_LOOP_BODY_ULI_T1_64_SECOND( 41 + i )
		PMPML_CHUNK_LOOP_BODY_ULI_T1_64_FIRST( 42 + i )
		PMPML_CHUNK_LOOP_BODY_ULI_T1_64_SECOND( 43 + i )
		PMPML_CHUNK_LOOP_BODY_ULI_T1_64_FIRST( 44 + i )
		PMPML_CHUNK_LOOP_BODY_ULI_T1_64_SECOND( 45 + i )
		PMPML_CHUNK_LOOP_BODY_ULI_T1_64_FIRST( 46 + i )
		PMPML_CHUNK_LOOP_BODY_ULI_T1_64_SECOND( 47 + i )
		PMPML_CHUNK_LOOP_BODY_ULI_T1_64_FIRST( 48 + i )
		PMPML_CHUNK_LOOP_BODY_ULI_T1_64_SECOND( 49 + i )
		PMPML_CHUNK_LOOP_BODY_ULI_T1_64_FIRST( 50 + i )
		PMPML_CHUNK_LOOP_BODY_ULI_T1_64_SECOND( 51 + i )
		PMPML_CHUNK_LOOP_BODY_ULI_T1_64_FIRST( 52 + i )
		PMPML_CHUNK_LOOP_BODY_ULI_T1_64_SECOND( 53 + i )
		PMPML_CHUNK_LOOP_BODY_ULI_T1_64_FIRST( 54 + i )
		PMPML_CHUNK_LOOP_BODY_ULI_T1_64_SECOND( 55 + i )
		PMPML_CHUNK_LOOP_BODY_ULI_T1_64_FIRST( 56 + i )
		PMPML_CHUNK_LOOP_BODY_ULI_T1_64_SECOND( 57 + i )
		PMPML_CHUNK_LOOP_BODY_ULI_T1_64_FIRST( 58 + i )
		PMPML_CHUNK_LOOP_BODY_ULI_T1_64_SECOND( 59 + i )
		PMPML_CHUNK_LOOP_BODY_ULI_T1_64_FIRST( 60 + i )
		PMPML_CHUNK_LOOP_BODY_ULI_T1_64_SECOND( 61 + i )
		PMPML_CHUNK_LOOP_BODY_ULI_T1_64_FIRST( 62 + i )
		PMPML_CHUNK_LOOP_BODY_ULI_T1_64_SECOND( 63 + i )
#endif
#if ( PMPML_CHUNK_SIZE_64 > 64 )
		PMPML_CHUNK_LOOP_BODY_ULI_T1_64_FIRST( 64 + i )
		PMPML_CHUNK_LOOP_BODY_ULI_T1_64_SECOND( 65 + i )
		PMPML_CHUNK_LOOP_BODY_ULI_T1_64_FIRST( 66 + i )
		PMPML_CHUNK_LOOP_BODY_ULI_T1_64_SECOND( 67 + i )
		PMPML_CHUNK_LOOP_BODY_ULI_T1_64_FIRST( 68 + i )
		PMPML_CHUNK_LOOP_BODY_ULI_T1_64_SECOND( 69 + i )
		PMPML_CHUNK_LOOP_BODY_ULI_T1_64_FIRST( 70 + i )
		PMPML_CHUNK_LOOP_BODY_ULI_T1_64_SECOND( 71 + i )
		PMPML_CHUNK_LOOP_BODY_ULI_T1_64_FIRST( 72 + i )
		PMPML_CHUNK_LOOP_BODY_ULI_T1_64_SECOND( 73 + i )
		PMPML_CHUNK_LOOP_BODY_ULI_T1_64_FIRST( 74 + i )
		PMPML_CHUNK_LOOP_BODY_ULI_T1_64_SECOND( 75 + i )
		PMPML_CHUNK_LOOP_BODY_ULI_T1_64_FIRST( 76 + i )
		PMPML_CHUNK_LOOP_BODY_ULI_T1_64_SECOND( 77 + i )
		PMPML_CHUNK_LOOP_BODY_ULI_T1_64_FIRST( 78 + i )
		PMPML_CHUNK_LOOP_BODY_ULI_T1_64_SECOND( 79 + i )
		PMPML_CHUNK_LOOP_BODY_ULI_T1_64_FIRST( 80 + i )
		PMPML_CHUNK_LOOP_BODY_ULI_T1_64_SECOND( 81 + i )
		PMPML_CHUNK_LOOP_BODY_ULI_T1_64_FIRST( 82 + i )
		PMPML_CHUNK_LOOP_BODY_ULI_T1_64_SECOND( 83 + i )
		PMPML_CHUNK_LOOP_BODY_ULI_T1_64_FIRST( 84 + i )
		PMPML_CHUNK_LOOP_BODY_ULI_T1_64_SECOND( 85 + i )
		PMPML_CHUNK_LOOP_BODY_ULI_T1_64_FIRST( 86 + i )
		PMPML_CHUNK_LOOP_BODY_ULI_T1_64_SECOND( 87 + i )
		PMPML_CHUNK_LOOP_BODY_ULI_T1_64_FIRST( 88 + i )
		PMPML_CHUNK_LOOP_BODY_ULI_T1_64_SECOND( 89 + i )
		PMPML_CHUNK_LOOP_BODY_ULI_T1_64_FIRST( 90 + i )
		PMPML_CHUNK_LOOP_BODY_ULI_T1_64_SECOND( 91 + i )
		PMPML_CHUNK_LOOP_BODY_ULI_T1_64_FIRST( 92 + i )
		PMPML_CHUNK_LOOP_BODY_ULI_T1_64_SECOND( 93 + i )
		PMPML_CHUNK_LOOP_BODY_ULI_T1_64_FIRST( 94 + i )
		PMPML_CHUNK_LOOP_BODY_ULI_T1_64_SECOND( 95 + i )
		PMPML_CHUNK_LOOP_BODY_ULI_T1_64_FIRST( 96 + i )
		PMPML_CHUNK_LOOP_BODY_ULI_T1_64_SECOND( 97 + i )
		PMPML_CHUNK_LOOP_BODY_ULI_T1_64_FIRST( 98 + i )
		PMPML_CHUNK_LOOP_BODY_ULI_T1_64_SECOND( 99 + i )
		PMPML_CHUNK_LOOP_BODY_ULI_T1_64_FIRST( 100 + i )
		PMPML_CHUNK_LOOP_BODY_ULI_T1_64_SECOND( 101 + i )
		PMPML_CHUNK_LOOP_BODY_ULI_T1_64_FIRST( 102 + i )
		PMPML_CHUNK_LOOP_BODY_ULI_T1_64_SECOND( 103 + i )
		PMPML_CHUNK_LOOP_BODY_ULI_T1_64_FIRST( 104 + i )
		PMPML_CHUNK_LOOP_BODY_ULI_T1_64_SECOND( 105 + i )
		PMPML_CHUNK_LOOP_BODY_ULI_T1_64_FIRST( 106 + i )
		PMPML_CHUNK_LOOP_BODY_ULI_T1_64_SECOND( 107 + i )
		PMPML_CHUNK_LOOP_BODY_ULI_T1_64_FIRST( 108 + i )
		PMPML_CHUNK_LOOP_BODY_ULI_T1_64_SECOND( 109 + i )
		PMPML_CHUNK_LOOP_BODY_ULI_T1_64_FIRST( 110 + i )
		PMPML_CHUNK_LOOP_BODY_ULI_T1_64_SECOND( 111 + i )
		PMPML_CHUNK_LOOP_BODY_ULI_T1_64_FIRST( 112 + i )
		PMPML_CHUNK_LOOP_BODY_ULI_T1_64_SECOND( 113 + i )
		PMPML_CHUNK_LOOP_BODY_ULI_T1_64_FIRST( 114 + i )
		PMPML_CHUNK_LOOP_BODY_ULI_T1_64_SECOND( 115 + i )
		PMPML_CHUNK_LOOP_BODY_ULI_T1_64_FIRST( 116 + i )
		PMPML_CHUNK_LOOP_BODY_ULI_T1_64_SECOND( 117 + i )
		PMPML_CHUNK_LOOP_BODY_ULI_T1_64_FIRST( 118 + i )
		PMPML_CHUNK_LOOP_BODY_ULI_T1_64_SECOND( 119 + i )
		PMPML_CHUNK_LOOP_BODY_ULI_T1_64_FIRST( 120 + i )
		PMPML_CHUNK_LOOP_BODY_ULI_T1_64_SECOND( 121 + i )
		PMPML_CHUNK_LOOP_BODY_ULI_T1_64_FIRST( 122 + i )
		PMPML_CHUNK_LOOP_BODY_ULI_T1_64_SECOND( 123 + i )
		PMPML_CHUNK_LOOP_BODY_ULI_T1_64_FIRST( 124 + i )
		PMPML_CHUNK_LOOP_BODY_ULI_T1_64_SECOND( 125 + i )
		PMPML_CHUNK_LOOP_BODY_ULI_T1_64_FIRST( 126 + i )
		PMPML_CHUNK_LOOP_BODY_ULI_T1_64_SECOND( 127 + i )
#endif
#endif // 0
	}
#endif // PMPML_USE_SSE_64

	PMPML_CHUNK_LOOP_PRE_REDUCE_L0_64

	PMPML_CHUNK_REDUCE_128_TO_64____
	ret.LowPart = ctr0.QuadPart;
	ret.HighPart = ctr1.QuadPart;
  }

  FORCE_INLINE void hash_of_beginning_of_string_chunk_short_type2( const uint64_t* coeff, ULARGE_INTEGER__XX constTerm, const unsigned char* tail, std::size_t tail_size, ULARGELARGE_INTEGER__XX& ret ) const
  {
	PMPML_CHUNK_LOOP_INTRO_L0_64
	std::size_t size = tail_size >> PMPML_WORD_SIZE_BYTES_LOG2_64;
	const uint64_t* x = (const uint64_t*)tail;

	switch( size )
	{
		case 1:	{	PMPML_CHUNK_LOOP_BODY_ULI_T1_64( 0 ) } break;
		case 2:	{	PMPML_CHUNK_LOOP_BODY_ULI_T1_64( 0 ) PMPML_CHUNK_LOOP_BODY_ULI_T1_64( 1 ) } break;
		case 3:	{	PMPML_CHUNK_LOOP_BODY_ULI_T1_64( 0 ) PMPML_CHUNK_LOOP_BODY_ULI_T1_64( 1 ) PMPML_CHUNK_LOOP_BODY_ULI_T1_64( 2 ) } break;
		case 4:	{	PMPML_CHUNK_LOOP_BODY_ULI_T1_64( 0 ) PMPML_CHUNK_LOOP_BODY_ULI_T1_64( 1 ) PMPML_CHUNK_LOOP_BODY_ULI_T1_64( 2 ) PMPML_CHUNK_LOOP_BODY_ULI_T1_64( 3 ) } break;
		case 5:	{	PMPML_CHUNK_LOOP_BODY_ULI_T1_64( 0 ) PMPML_CHUNK_LOOP_BODY_ULI_T1_64( 1 ) PMPML_CHUNK_LOOP_BODY_ULI_T1_64( 2 ) PMPML_CHUNK_LOOP_BODY_ULI_T1_64( 3 ) PMPML_CHUNK_LOOP_BODY_ULI_T1_64( 4 ) } break;
		case 6:	{	PMPML_CHUNK_LOOP_BODY_ULI_T1_64( 0 ) PMPML_CHUNK_LOOP_BODY_ULI_T1_64( 1 ) PMPML_CHUNK_LOOP_BODY_ULI_T1_64( 2 ) PMPML_CHUNK_LOOP_BODY_ULI_T1_64( 3 ) PMPML_CHUNK_LOOP_BODY_ULI_T1_64( 4 ) PMPML_CHUNK_LOOP_BODY_ULI_T1_64( 5 ) } break;
		case 7:	{	PMPML_CHUNK_LOOP_BODY_ULI_T1_64( 0 ) PMPML_CHUNK_LOOP_BODY_ULI_T1_64( 1 ) PMPML_CHUNK_LOOP_BODY_ULI_T1_64( 2 ) PMPML_CHUNK_LOOP_BODY_ULI_T1_64( 3 ) PMPML_CHUNK_LOOP_BODY_ULI_T1_64( 4 ) PMPML_CHUNK_LOOP_BODY_ULI_T1_64( 5 ) PMPML_CHUNK_LOOP_BODY_ULI_T1_64( 6 ) } break;
	}

	uint64_t xLast;
	switch ( tail_size & ( PMPML_WORD_SIZE_BYTES_64 - 1 ) )
	{
		case 0: { xLast = 0x1; break;}
		case 1: { xLast = 0x100 + tail[tail_size-1]; break;}
		case 2: { xLast = *((const unsigned short*)(tail + tail_size - 2 )) + 0x10000; break; }
		case 3: { xLast = tail[ tail_size - 1 ]; xLast = ( xLast << 16 ) + *((const unsigned short*)(tail + tail_size - 3 )) + 0x1000000; break;}
		case 4: { xLast = *((const unsigned int*)(tail + tail_size - 4)) + UINT64_C( 0x100000000 ); break; }
		case 5: { xLast = tail[ tail_size - 1 ]; xLast = ( xLast << 32 ) + UINT64_C( 0x10000000000 ) + *((const unsigned int*)(tail + tail_size - 5)); break;}
		case 6: { xLast = *((const unsigned short*)(tail + tail_size - 2 )); xLast = ( xLast << 32 ) + UINT64_C( 0x1000000000000 ) + *((const unsigned int*)(tail + tail_size - 6)); break;}
		default: { xLast = tail[ tail_size - 1 ]; xLast <<= 48; uint64_t xLast1 = *((const unsigned short*)(tail + tail_size - 3 )); xLast += (xLast1<<32) + UINT64_C( 0x100000000000000 ) + *((const unsigned int*)(tail + tail_size - 7 )); break;}
	}
	PMPML_CHUNK_LOOP_BODY_ULI_T1_64_LAST(size)

	PMPML_CHUNK_LOOP_PRE_REDUCE_L0_64
	PMPML_CHUNK_REDUCE_128_TO_64
	ret.LowPart = ctr0.QuadPart;
	ret.HighPart = ctr1.QuadPart;
  }

  FORCE_INLINE void hash_of_beginning_of_string_chunk_type2( const uint64_t* coeff, ULARGE_INTEGER__XX constTerm, const unsigned char* tail, std::size_t tail_size, ULARGELARGE_INTEGER__XX& ret ) const
  {
	PMPML_CHUNK_LOOP_INTRO_L0_64
	std::size_t size = tail_size >> PMPML_WORD_SIZE_BYTES_LOG2_64;
	const uint64_t* x = (const uint64_t*)tail;

	for ( uint32_t i=0; i<(size>>3); i++ )
	{
		PMPML_CHUNK_LOOP_BODY_ULI_T1_64( 0 + ( i << 3 ) )
		PMPML_CHUNK_LOOP_BODY_ULI_T1_64( 1 + ( i << 3 ) )
		PMPML_CHUNK_LOOP_BODY_ULI_T1_64( 2 + ( i << 3 ) )
		PMPML_CHUNK_LOOP_BODY_ULI_T1_64( 3 + ( i << 3 ) )
#if ( PMPML_CHUNK_SIZE_64 > 4 )
		PMPML_CHUNK_LOOP_BODY_ULI_T1_64( 4 + ( i << 3 ) )
		PMPML_CHUNK_LOOP_BODY_ULI_T1_64( 5 + ( i << 3 ) )
		PMPML_CHUNK_LOOP_BODY_ULI_T1_64( 6 + ( i << 3 ) )
		PMPML_CHUNK_LOOP_BODY_ULI_T1_64( 7 + ( i << 3 ) )
#endif
	}

	uint64_t offset = size & 0xFFFFFFF8;

	switch( size & 0x7 )
	{
		case 0: { break; }
		case 1:	{	PMPML_CHUNK_LOOP_BODY_ULI_T1_64( 0 + offset ) } break;
		case 2:	{	PMPML_CHUNK_LOOP_BODY_ULI_T1_64( 0 + offset ) PMPML_CHUNK_LOOP_BODY_ULI_T1_64( 1 + offset ) } break;
		case 3:	{	PMPML_CHUNK_LOOP_BODY_ULI_T1_64( 0 + offset ) PMPML_CHUNK_LOOP_BODY_ULI_T1_64( 1 + offset ) PMPML_CHUNK_LOOP_BODY_ULI_T1_64( 2 + offset ) } break;
		case 4:	{	PMPML_CHUNK_LOOP_BODY_ULI_T1_64( 0 + offset ) PMPML_CHUNK_LOOP_BODY_ULI_T1_64( 1 + offset ) PMPML_CHUNK_LOOP_BODY_ULI_T1_64( 2 + offset ) PMPML_CHUNK_LOOP_BODY_ULI_T1_64( 3 + offset ) } break;
		case 5:	{	PMPML_CHUNK_LOOP_BODY_ULI_T1_64( 0 + offset ) PMPML_CHUNK_LOOP_BODY_ULI_T1_64( 1 + offset ) PMPML_CHUNK_LOOP_BODY_ULI_T1_64( 2 + offset ) PMPML_CHUNK_LOOP_BODY_ULI_T1_64( 3 + offset ) PMPML_CHUNK_LOOP_BODY_ULI_T1_64( 4 + offset ) } break;
		case 6:	{	PMPML_CHUNK_LOOP_BODY_ULI_T1_64( 0 + offset ) PMPML_CHUNK_LOOP_BODY_ULI_T1_64( 1 + offset ) PMPML_CHUNK_LOOP_BODY_ULI_T1_64( 2 + offset ) PMPML_CHUNK_LOOP_BODY_ULI_T1_64( 3 + offset ) PMPML_CHUNK_LOOP_BODY_ULI_T1_64( 4 + offset ) PMPML_CHUNK_LOOP_BODY_ULI_T1_64( 5 + offset ) } break;
		case 7:	{	PMPML_CHUNK_LOOP_BODY_ULI_T1_64( 0 + offset ) PMPML_CHUNK_LOOP_BODY_ULI_T1_64( 1 + offset ) PMPML_CHUNK_LOOP_BODY_ULI_T1_64( 2 + offset ) PMPML_CHUNK_LOOP_BODY_ULI_T1_64( 3 + offset ) PMPML_CHUNK_LOOP_BODY_ULI_T1_64( 4 + offset ) PMPML_CHUNK_LOOP_BODY_ULI_T1_64( 5 + offset ) PMPML_CHUNK_LOOP_BODY_ULI_T1_64( 6 + offset ) } break;
	}

	uint64_t xLast;
	switch ( tail_size & ( PMPML_WORD_SIZE_BYTES_64 - 1 ) )
	{
		case 0: { xLast = 0x1; break;}
		case 1: { xLast = 0x100 + tail[tail_size-1]; break;}
		case 2: { xLast = *((const unsigned short*)(tail + tail_size - 2 )) + 0x10000; break; }
		case 3: { xLast = tail[ tail_size - 1 ]; xLast = ( xLast << 16 ) + *((const unsigned short*)(tail + tail_size - 3 )) + 0x1000000; break;}
		case 4: { xLast = *((const unsigned int*)(tail + tail_size - 4)) + UINT64_C( 0x100000000 ); break; }
		case 5: { xLast = tail[ tail_size - 1 ]; xLast = ( xLast << 32 ) + UINT64_C( 0x10000000000 ) + *((const unsigned int*)(tail + tail_size - 5)); break;}
		case 6: { xLast = *((const unsigned short*)(tail + tail_size - 2 )); xLast = ( xLast << 32 ) + UINT64_C( 0x1000000000000 ) + *((const unsigned int*)(tail + tail_size - 6)); break;}
		default: { xLast = tail[ tail_size - 1 ]; xLast <<= 48; uint64_t xLast1 = *((const unsigned short*)(tail + tail_size - 3 )); xLast += (xLast1<<32) + UINT64_C( 0x100000000000000 ) + *((const unsigned int*)(tail + tail_size - 7 )); break;}
	}

	PMPML_CHUNK_LOOP_BODY_ULI_T1_64_LAST(size)

	PMPML_CHUNK_LOOP_PRE_REDUCE_L0_64
	PMPML_CHUNK_REDUCE_128_TO_64
	ret.LowPart = ctr0.QuadPart;
	ret.HighPart = ctr1.QuadPart;
  }

  // a call to be done from subsequent levels
  FORCE_INLINE void hash_of_num_chunk( const uint64_t* coeff, ULARGE_INTEGER__XX constTerm, const ULARGELARGE_INTEGER__XX* x, ULARGELARGE_INTEGER__XX& ret ) const
  {
	ULARGE_INTEGER__XX ctr0, ctr1, ctr2;
	ctr0.QuadPart = constTerm.QuadPart;
	ctr1.QuadPart = 0;
	ctr2.QuadPart = 0;
	ULARGE_INTEGER__XX mulLow, mulHigh;

	for ( uint64_t i=0; i<(PMPML_CHUNK_SIZE_64); i+=32 )
	{
		PMPML_CHUNK_LOOP_BODY_ULI_T2_64( 0 + i )
		PMPML_CHUNK_LOOP_BODY_ULI_T2_64( 1 + i )
		PMPML_CHUNK_LOOP_BODY_ULI_T2_64( 2 + i )
		PMPML_CHUNK_LOOP_BODY_ULI_T2_64( 3 + i )
#if ( PMPML_CHUNK_SIZE_64 > 4 )
		PMPML_CHUNK_LOOP_BODY_ULI_T2_64( 4 + i )
		PMPML_CHUNK_LOOP_BODY_ULI_T2_64( 5 + i )
		PMPML_CHUNK_LOOP_BODY_ULI_T2_64( 6 + i )
		PMPML_CHUNK_LOOP_BODY_ULI_T2_64( 7 + i )
#endif
#if ( PMPML_CHUNK_SIZE_64 > 8 )
		PMPML_CHUNK_LOOP_BODY_ULI_T2_64( 8 + i )
		PMPML_CHUNK_LOOP_BODY_ULI_T2_64( 9 + i )
		PMPML_CHUNK_LOOP_BODY_ULI_T2_64( 10 + i )
		PMPML_CHUNK_LOOP_BODY_ULI_T2_64( 11 + i )
		PMPML_CHUNK_LOOP_BODY_ULI_T2_64( 12 + i )
		PMPML_CHUNK_LOOP_BODY_ULI_T2_64( 13 + i )
		PMPML_CHUNK_LOOP_BODY_ULI_T2_64( 14 + i )
		PMPML_CHUNK_LOOP_BODY_ULI_T2_64( 15 + i )
#endif
#if ( PMPML_CHUNK_SIZE_64 > 16 )
		PMPML_CHUNK_LOOP_BODY_ULI_T2_64( 16 + i )
		PMPML_CHUNK_LOOP_BODY_ULI_T2_64( 17 + i )
		PMPML_CHUNK_LOOP_BODY_ULI_T2_64( 18 + i )
		PMPML_CHUNK_LOOP_BODY_ULI_T2_64( 19 + i )
		PMPML_CHUNK_LOOP_BODY_ULI_T2_64( 20 + i )
		PMPML_CHUNK_LOOP_BODY_ULI_T2_64( 21 + i )
		PMPML_CHUNK_LOOP_BODY_ULI_T2_64( 22 + i )
		PMPML_CHUNK_LOOP_BODY_ULI_T2_64( 23 + i )
		PMPML_CHUNK_LOOP_BODY_ULI_T2_64( 24 + i )
		PMPML_CHUNK_LOOP_BODY_ULI_T2_64( 25 + i )
		PMPML_CHUNK_LOOP_BODY_ULI_T2_64( 26 + i )
		PMPML_CHUNK_LOOP_BODY_ULI_T2_64( 27 + i )
		PMPML_CHUNK_LOOP_BODY_ULI_T2_64( 28 + i )
		PMPML_CHUNK_LOOP_BODY_ULI_T2_64( 29 + i )
		PMPML_CHUNK_LOOP_BODY_ULI_T2_64( 30 + i )
		PMPML_CHUNK_LOOP_BODY_ULI_T2_64( 31 + i )
#endif
	}

	PMPML_CHUNK_REDUCE_128_TO_64


	ret.LowPart = ctr0.QuadPart;
	ret.HighPart = ctr1.QuadPart;
  }

  // a call to be done from subsequent levels
  FORCE_INLINE void hash_of_num_chunk_incomplete( const uint64_t* coeff, uint64_t constTerm, uint64_t prevConstTerm, uint64_t coeffSumLow, uint64_t coeffSumHigh, const ULARGELARGE_INTEGER__XX* x, size_t count, ULARGELARGE_INTEGER__XX& ret ) const
  {
	ULARGE_INTEGER__XX ctr0, ctr1, ctr2;
	ctr0.QuadPart = constTerm;
	ctr1.QuadPart = 0;
	ctr2.QuadPart = 0;
	ULARGE_INTEGER__XX c_ctr0, c_ctr1;
	c_ctr0.QuadPart = 0;
	c_ctr1.QuadPart = 0;
	ULARGE_INTEGER__XX mulLow, mulHigh;
	uint64_t i;
	if ( count < ( PMPML_CHUNK_SIZE_64 >> 1 ) )
	{
		for ( i=0; i<count; i++ )
		{
			PMPML_CHUNK_LOOP_BODY_ULI_T2_AND_ADD_COEFF_64( i );
		}
		if ( c_ctr0.QuadPart > coeffSumLow )
			c_ctr1.QuadPart = coeffSumHigh - c_ctr1.QuadPart - 1;
		else
			c_ctr1.QuadPart = coeffSumHigh - c_ctr1.QuadPart;
		c_ctr0.QuadPart = coeffSumLow - c_ctr0.QuadPart;
	}
	else
	{
		for ( i=0; i<count; i++ )
		{
			PMPML_CHUNK_LOOP_BODY_ULI_T2_64( i )
		}
		for ( ; i<PMPML_CHUNK_SIZE_64; i++ )
		{
			PMPML_CHUNK_LOOP_BODY_ULI_ADD_COEFF_64( i )
		}
	}
	PMPML_CHUNK_LOOP_BODY_ULI_T2_AND_ADD_SUM_OF_COEFF_64

	PMPML_CHUNK_REDUCE_128_TO_64

	ret.LowPart = ctr0.QuadPart;
	ret.HighPart = ctr1.QuadPart;
  }

  FORCE_INLINE void procesNextValue( int level, _ULARGELARGE_INTEGER__XX& value, _ULARGELARGE_INTEGER__XX * allValues, std::size_t * cnts, std::size_t& flag ) const
  {
	for ( int i=level;;i++ )
	{
		// NOTE: it's not necessary to check whether ( i < PMPML_LEVELS_64 ), 
		// if it is guaranteed that the string size is less than 1 << USHF_MACHINE_WORD_SIZE_BITS
		allValues[ ( i << PMPML_CHUNK_SIZE_LOG2_64 ) + cnts[ i ] ] = value;
		(cnts[ i ]) ++;
		if ( cnts[ i ] != PMPML_CHUNK_SIZE_64 )
			break;
		cnts[ i ] = 0;
		hash_of_num_chunk( curr_rd[ i ].random_coeff, *(ULARGE_INTEGER__XX*)(&(curr_rd[i].const_term)), allValues + ( i << PMPML_CHUNK_SIZE_LOG2_64 ), value );
		if ( ( flag & ( 1 << i ) ) == 0 )
		{
			cnts[ i + 1] = 0;
			flag |= 1 << i;
		}
	}
  }

  FORCE_INLINE _ULARGELARGE_INTEGER__XX& finalize( int level, _ULARGELARGE_INTEGER__XX * allValues, std::size_t * cnts, std::size_t& flag ) const
  {
    ULARGELARGE_INTEGER__XX value;
	for ( int i=level;;i++ )
	{
//		ASSERT ( level != PMPML_LEVELS_64 )
		if ( ( ( flag & ( 1 << i ) ) == 0 ) && cnts[ i ] == 1 )
		{
			return allValues[ i << PMPML_CHUNK_SIZE_LOG2_64 ];
		}
		if ( cnts[ i ] )
		{
/*			for ( int j=cnts[ i ]; j<PMPML_CHUNK_SIZE_64; j++ )
			{
				( allValues + ( i << PMPML_CHUNK_SIZE_LOG2_64 ) )[ j ].LowPart = curr_rd[ i ].const_term;
				( allValues + ( i << PMPML_CHUNK_SIZE_LOG2_64 ) )[ j ].HighPart = 0;
			}*/
			if ( ( flag & ( 1 << i ) ) == 0 )
			{
				cnts[ i + 1] = 0;
				flag |= 1 << i;
			}
			hash_of_num_chunk_incomplete( curr_rd[ i ].random_coeff,
								curr_rd[i].const_term, curr_rd[i].const_term,curr_rd[i].cachedSumLow, curr_rd[i].cachedSumHigh,
								allValues + ( i << PMPML_CHUNK_SIZE_LOG2_64 ), cnts[i], value ); 
/*			hash_of_num_chunk( curr_rd[ i ].random_coeff,
								*(ULARGE_INTEGER__XX*)(&(curr_rd[i].const_term)),
								allValues + ( i << PMPML_CHUNK_SIZE_LOG2_64 ), value ); */
			procesNextValue( i + 1, value,
							 allValues, cnts, flag );
		}
	}
  }

  public:
  FORCE_INLINE uint64_t hash( const unsigned char* chars, std::size_t cnt ) const
  {
			if ( _LIKELY_BRANCH_(cnt < 64) )
			{
				const uint64_t* coeff = curr_rd[0].random_coeff;
				ULARGE_INTEGER__XX constTerm = *(ULARGE_INTEGER__XX*)(&(curr_rd[0].const_term));
				PMPML_CHUNK_LOOP_INTRO_L0_64
				std::size_t size = cnt >> PMPML_WORD_SIZE_BYTES_LOG2_64;
				const uint64_t* x = (const uint64_t*)chars;

				switch( size )
				{
					case 1:	{	PMPML_CHUNK_LOOP_BODY_ULI_T1_64( 0 ) } break;
					case 2:	{	PMPML_CHUNK_LOOP_BODY_ULI_T1_64( 0 ) PMPML_CHUNK_LOOP_BODY_ULI_T1_64( 1 ) } break;
					case 3:	{	PMPML_CHUNK_LOOP_BODY_ULI_T1_64( 0 ) PMPML_CHUNK_LOOP_BODY_ULI_T1_64( 1 ) PMPML_CHUNK_LOOP_BODY_ULI_T1_64( 2 ) } break;
					case 4:	{	PMPML_CHUNK_LOOP_BODY_ULI_T1_64( 0 ) PMPML_CHUNK_LOOP_BODY_ULI_T1_64( 1 ) PMPML_CHUNK_LOOP_BODY_ULI_T1_64( 2 ) PMPML_CHUNK_LOOP_BODY_ULI_T1_64( 3 ) } break;
					case 5:	{	PMPML_CHUNK_LOOP_BODY_ULI_T1_64( 0 ) PMPML_CHUNK_LOOP_BODY_ULI_T1_64( 1 ) PMPML_CHUNK_LOOP_BODY_ULI_T1_64( 2 ) PMPML_CHUNK_LOOP_BODY_ULI_T1_64( 3 ) PMPML_CHUNK_LOOP_BODY_ULI_T1_64( 4 ) } break;
					case 6:	{	PMPML_CHUNK_LOOP_BODY_ULI_T1_64( 0 ) PMPML_CHUNK_LOOP_BODY_ULI_T1_64( 1 ) PMPML_CHUNK_LOOP_BODY_ULI_T1_64( 2 ) PMPML_CHUNK_LOOP_BODY_ULI_T1_64( 3 ) PMPML_CHUNK_LOOP_BODY_ULI_T1_64( 4 ) PMPML_CHUNK_LOOP_BODY_ULI_T1_64( 5 ) } break;
					case 7:	{	PMPML_CHUNK_LOOP_BODY_ULI_T1_64( 0 ) PMPML_CHUNK_LOOP_BODY_ULI_T1_64( 1 ) PMPML_CHUNK_LOOP_BODY_ULI_T1_64( 2 ) PMPML_CHUNK_LOOP_BODY_ULI_T1_64( 3 ) PMPML_CHUNK_LOOP_BODY_ULI_T1_64( 4 ) PMPML_CHUNK_LOOP_BODY_ULI_T1_64( 5 ) PMPML_CHUNK_LOOP_BODY_ULI_T1_64( 6 ) } break;
				}

				uint64_t xLast;
				switch ( cnt & ( PMPML_WORD_SIZE_BYTES_64 - 1 ) )
				{
					case 0: { xLast = 0x1; break;}
					case 1: { xLast = 0x100 + chars[cnt-1]; break;}
					case 2: { xLast = *((const unsigned short*)(chars + cnt - 2 )) + 0x10000; break; }
					case 3: { xLast = chars[ cnt - 1 ]; xLast = ( xLast << 16 ) + *((const unsigned short*)(chars + cnt - 3 )) + 0x1000000; break;}
					case 4: { xLast = *((const unsigned int*)(chars + cnt - 4)) + UINT64_C( 0x100000000 ); break; }
					case 5: { xLast = chars[ cnt - 1 ]; xLast = ( xLast << 32 ) + UINT64_C( 0x10000000000 ) + *((const unsigned int*)(chars + cnt - 5)); break;}
					case 6: { xLast = *((const unsigned short*)(chars + cnt - 2 )); xLast = ( xLast << 32 ) + UINT64_C( 0x1000000000000 ) + *((const unsigned int*)(chars + cnt - 6)); break;}
					default: { xLast = chars[ cnt - 1 ]; xLast <<= 48; uint64_t xLast1 = *((const unsigned short*)(chars + cnt - 3 )); xLast += (xLast1<<32) + UINT64_C( 0x100000000000000 ) + *((const unsigned int*)(chars + cnt - 7 )); break;}
				}
				PMPML_CHUNK_LOOP_BODY_ULI_T1_64_LAST(size)

				PMPML_CHUNK_LOOP_PRE_REDUCE_L0_64
				PMPML_CHUNK_REDUCE_128_TO_64_AND_RETURN
			}
			else if ( cnt < PMPML_CHUNK_SIZE_64 )
			{
				return _hash_noRecursionNoInline_SingleChunk( chars, cnt );
			}
			else
			{
				return _hash_noRecursionNoInline_type2( chars, cnt );
			}
  }

  NOINLINE uint64_t _hash_noRecursionNoInline_SingleChunk( const unsigned char* chars, std::size_t cnt ) const
  {
			_ULARGELARGE_INTEGER__XX tmp_hash;
			hash_of_beginning_of_string_chunk_type2( curr_rd[0].random_coeff, *(ULARGE_INTEGER__XX*)(&(curr_rd[0].const_term)), chars, cnt, tmp_hash );
			if ( tmp_hash.HighPart == 0 ) //LIKELY
			{
				return fmix64_short( tmp_hash.LowPart );
			}
			return tmp_hash.LowPart;
  }

  NOINLINE uint64_t _hash_noRecursionNoInline_type2( const unsigned char* chars, std::size_t cnt ) const
  {
			_ULARGELARGE_INTEGER__XX allValues[ PMPML_LEVELS_64 * PMPML_CHUNK_SIZE_64 ];
			std::size_t cnts[ PMPML_LEVELS_64 ];
			std::size_t flag;
			cnts[ 1 ] = 0;
			flag = 0;

			std::size_t i;
			_ULARGELARGE_INTEGER__XX tmp_hash;
			// process full chunks
			for ( i=0; i<(cnt>>PMPML_CHUNK_SIZE_BYTES_LOG2_64); i++ )
			{
				hash_of_string_chunk_compact( curr_rd[0].random_coeff, *(ULARGE_INTEGER__XX*)(&(curr_rd[0].const_term)), ((const uint64_t*)(chars)) + ( i << PMPML_CHUNK_SIZE_LOG2_64 ), tmp_hash );
				procesNextValue( 1, tmp_hash, allValues, cnts, flag );
			}
			// process remaining incomplete chunk(s)
			// note: if string size is a multiple of chunk size, we create a new chunk (1,0,0,...0),
			// so THIS PROCESSING IS ALWAYS PERFORMED
			std::size_t tailCnt = cnt & ( PMPML_CHUNK_SIZE_BYTES_64 - 1 );
			const unsigned char* tail = chars + ( (cnt>>PMPML_CHUNK_SIZE_BYTES_LOG2_64) << PMPML_CHUNK_SIZE_BYTES_LOG2_64 );
			hash_of_beginning_of_string_chunk_type2( curr_rd[0].random_coeff, *(ULARGE_INTEGER__XX*)(&(curr_rd[0].const_term)), tail, tailCnt, tmp_hash );
			procesNextValue( 1, tmp_hash, allValues, cnts, flag );
			_ULARGELARGE_INTEGER__XX finRet = finalize( 1, allValues, cnts, flag );
			if ( finRet.HighPart == 0 ) //LIKELY
			{
				return fmix64_short( finRet.LowPart );
			}
			return finRet.LowPart;
  }


  public:
  PMP_Multilinear_Hasher_64()
  {
    curr_rd = (random_data_for_PMPML_64*)rd_for_PMPML_64;
  }
  virtual ~PMP_Multilinear_Hasher_64()
  {
    if ( curr_rd != NULL && curr_rd != rd_for_PMPML_64 )
		delete [] curr_rd;
  }

  void randomize( UniformRandomNumberGenerator& rng )
  {
    random_data_for_PMPML_64 * temp_curr_rd = new random_data_for_PMPML_64[ PMPML_LEVELS_64 ];

	int i, j;
	for ( i=0; i<PMPML_LEVELS_64; i++ )
	{
		for ( j=0; j<PMPML_CHUNK_SIZE_64; j++ )
		{
			temp_curr_rd[ i ].cachedSumLow = 0;
			temp_curr_rd[ i ].cachedSumHigh = 0;
			do
			{
				temp_curr_rd[ i ].random_coeff[ j ] = rng.rand();
				temp_curr_rd[ i ].random_coeff[ j ] <<= 32;
				temp_curr_rd[ i ].random_coeff[ j ] |= rng.rand();
			}
			while ( !IS_VALID_COEFFICIENT_64( temp_curr_rd[ i ].random_coeff[ j ], i ) );
			uint64_t csl = temp_curr_rd[ i ].cachedSumLow;
			temp_curr_rd[ i ].cachedSumLow += temp_curr_rd[ i ].random_coeff[ j ];
			if ( temp_curr_rd[ i ].cachedSumLow < csl )
				temp_curr_rd[ i ].cachedSumHigh += 1;
		}
	}

	for ( i=0; i<PMPML_LEVELS_64; i++ )
	{
		uint64_t rv = rng.rand();
		rv <<= 32;
		rv += rng.rand();
		temp_curr_rd[ i ].const_term = rv;
	}

	if ( curr_rd == rd_for_PMPML_64 )
		curr_rd = temp_curr_rd;
	else
	{
		if ( curr_rd != NULL )
			delete [] curr_rd;
		curr_rd = temp_curr_rd;
	}
  }
  void seed( uint32_t seed )
  {
    curr_rd[0].random_coeff[0] ^= (uint64_t)seed;
  }
#if 0
  void calc_vals()
  {
	  int i, j;
	for ( i=0; i<PMPML_LEVELS_64; i++ )
	{
		uint64_t low = 0, high = 0, old;
		for ( j=0; j<PMPML_CHUNK_SIZE_64; j++ )
		{
			old = low;
			low += curr_rd[ i ].random_coeff[ j ];
			if ( low < old )
				high++;
		}
		printf( "UINT64_C( 0x%x%08x ), UINT64_C( 0x%x%08x ), 0, // sum of coeff and dummy\n", (uint32_t)( low >> 32 ), (uint32_t)( low ),  (uint32_t)( high >> 32 ), (uint32_t)( high ) );
	}
	printf( "\n\n\n\n\n" );
  }
#endif
};

#endif //  (defined _WIN64) || (defined __x86_64__)
#endif // __arm__
#endif // __PMP_MULTILINEAR_HASHER_64_H__
