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

// PMP_Multilinear.h: implementation of a 32-bit version of PMP+-Multilinear hash family

#if !defined __PMP_MULTILINEAR_HASHER_H__
#define __PMP_MULTILINEAR_HASHER_H__
#define __MULTILINEARPRIMESTRINGHASHFUNCTOR_CPP_H__REVISION_ "$Rev: 453 $" /* for automated version information update; could be removed, if not desired */

#include "PMP_Multilinear_common.h"

#ifdef _MSC_VER
#define PMPML_CHUNK_OPTIMIZATION_TYPE 2 // 1 or 2 is recommended
#define PMPML_MSC_32_WORKAROUND	// enables MSVC-specific code that appears to be more efficient than a regular one; comment out, if not desired
#else // _MSC_VER
#if (defined __arm__ || defined __aarch64__)
#define PMPML_CHUNK_OPTIMIZATION_TYPE 1 // 1 is recommended
#else // __arm__
#ifdef __INTEL_COMPILER
#define PMPML_CHUNK_OPTIMIZATION_TYPE 1 // add64 function fails to build on some Intel compilers
#else 
#define PMPML_CHUNK_OPTIMIZATION_TYPE 2 // 2 is recommended
#endif //__INTEL_COMPILER
#endif // __arm __
#endif // _MSC_VER

#if defined __arm__ || defined __aarch64__
//#define PMPML_STRICT_UNALIGNED_HANDLING
#endif // __arm__

#ifdef PMPML_STRICT_UNALIGNED_HANDLING
#endif

#if (PMPML_CHUNK_OPTIMIZATION_TYPE== 1)

#define PMPML_CHUNK_LOOP_INTRO_L0 \
	ULARGE_INTEGER__XX ctr; \
	ctr.QuadPart = 0; \
	ULARGE_INTEGER__XX mul;

#define PMPML_CHUNK_LOOP_BODY_ULI_T1( i ) \
	mul.QuadPart = UInt32x32To64( coeff[ i ], x[ i ] ); \
	constTerm.QuadPart += mul.LowPart; \
	ctr.QuadPart += mul.HighPart;

#ifdef PMPML_STRICT_UNALIGNED_HANDLING

#define PMPML_CHUNK_LOOP_BODY_ULI_T1_UNL( shift, i ) \
	mul.QuadPart = UInt32x32To64( coeff[ i ], x | ( xx[ i ] << (32-shift) ) ); \
	x = xx[ i ] >> shift; \
	constTerm.QuadPart += mul.LowPart; \
	ctr.QuadPart += mul.HighPart;

#define PMPML_CHUNK_LOOP_BODY_ULI_T1_UNL_PTR( i ) \
	mul.QuadPart = UInt32x32To64( coeff[ i ], xun[ i ].value ); \
	constTerm.QuadPart += mul.LowPart; \
	ctr.QuadPart += mul.HighPart;

#endif // PMPML_STRICT_UNALIGNED_HANDLING

#define PMPML_CHUNK_LOOP_BODY_ULI_T1_LAST \
    mul.QuadPart = UInt32x32To64(xLast, (uint32_t)(coeff[ size ])); \
    constTerm.QuadPart += mul.LowPart; \
	ctr.QuadPart += mul.HighPart;


#define PMPML_CHUNK_LOOP_BODY_ULI_T1_LAST_FOR_JUST_1 \
    constTerm.QuadPart += coeff[ size ];

#define PMPML_CHUNK_LOOP_PRE_REDUCE_L0

#define PMPML_FULL_REDUCE_MOD_2_32_PLUS_15_AND_RETURN \
{ \
	ctr.QuadPart += constTerm.HighPart; \
	constTerm.HighPart = ctr.LowPart; \
	ULARGE_INTEGER__XX hilo; \
	hilo.QuadPart = constTerm.HighPart * UINT64_C( 15 ); \
	uint32_t part = ctr.HighPart * 225 + (hilo.HighPart << 4) - hilo.HighPart + 15; \
	constTerm.LowPart += part; \
	constTerm.HighPart = 1 + (constTerm.LowPart < part); \
	constTerm.HighPart -= (constTerm.LowPart < hilo.LowPart); \
	constTerm.LowPart -= hilo.LowPart; \
	if ( _LIKELY_BRANCH_( constTerm.LowPart >= 30) ) { constTerm.LowPart -= constTerm.HighPart * 15; constTerm.HighPart = 0; } \
	else \
	{ \
		constTerm.LowPart -= constTerm.HighPart * 15; \
		if ( constTerm.LowPart < 30 ) constTerm.HighPart = 0; \
		else \
		{ \
			constTerm.LowPart += 15; \
			if ( constTerm.LowPart < 15 ) constTerm.HighPart = 1; \
			else constTerm.HighPart = 0; \
		} \
	} \
	return constTerm.QuadPart; \
}

#define PMPML_FULL_REDUCE_MOD_2_32_PLUS_15_AND_RETURN_RETURN \
{ \
	ctr.QuadPart += constTerm.HighPart; \
	constTerm.HighPart = ctr.LowPart; \
	ULARGE_INTEGER__XX hilo; \
	hilo.QuadPart = constTerm.HighPart * UINT64_C( 15 ); \
	uint32_t part = ctr.HighPart * 225 + (hilo.HighPart << 4) - hilo.HighPart + 15; \
	constTerm.LowPart += part; \
	constTerm.HighPart = 1 + (constTerm.LowPart < part); \
	constTerm.HighPart -= (constTerm.LowPart < hilo.LowPart); \
	constTerm.LowPart -= hilo.LowPart; \
	if ( _LIKELY_BRANCH_( constTerm.LowPart >= 30) ) { constTerm.LowPart -= (constTerm.HighPart << 4) - constTerm.HighPart; return fmix32_short( constTerm.LowPart ); } \
	else \
	{ \
		constTerm.LowPart -= constTerm.HighPart * 15; \
		if ( constTerm.LowPart < 30 ) return fmix32_short( constTerm.LowPart ); \
		else \
		{ \
			constTerm.LowPart += 15; \
			return fmix32_short( constTerm.LowPart ); \
		} \
	} \
}

#define PMPML_FULL_REDUCE_MOD_2_32_PLUS_15_AND_RETURN_RETURN_32x32_ONLY \
{ \
	constTerm.QuadPart = constTerm.LowPart + PMPML_MAIN_PRIME - constTerm.HighPart * UINT64_C( 15 ); \
	if ( _LIKELY_BRANCH_( constTerm.LowPart >= 30) ) { constTerm.LowPart -= (constTerm.HighPart << 4) - constTerm.HighPart; return fmix32_short( constTerm.LowPart ); } \
	else \
	{ \
		constTerm.LowPart -= constTerm.HighPart * 15; \
		if ( constTerm.LowPart < 30 ) return fmix32_short( constTerm.LowPart ); \
		else \
		{ \
			constTerm.LowPart += 15; \
			return fmix32_short( constTerm.LowPart ); \
		} \
	} \
}

#elif (PMPML_CHUNK_OPTIMIZATION_TYPE == 2)

#if defined(_MSC_VER)
	
FORCE_INLINE
void multiply32x32to64(uint32_t& rhi, uint32_t& rlo, uint32_t a, uint32_t b)
{
	ULARGE_INTEGER__XX mul;
	mul.QuadPart = UInt32x32To64( a, b );
	rhi = mul.HighPart;
	rlo = mul.LowPart;
}

FORCE_INLINE
void add64(uint32_t& loWord, uint32_t& hiWord, uint32_t& hhWord, uint32_t& loAdd, uint32_t& hiAdd )
{
	loWord += loAdd;
	uint64_t carry = loWord < loAdd;
	hiWord += hiAdd;
	uint64_t c1 = hiWord < hiAdd;
	hhWord += c1;
	hiWord += carry;
	c1 = hiWord < carry;
	hhWord += c1;
}

FORCE_INLINE
void mul32x32to64addto96(uint32_t& loWord, uint32_t& hiWord, uint32_t& hhWord, uint32_t a, uint32_t b )
{
	ULARGE_INTEGER__XX mul;
	mul.QuadPart = UInt32x32To64( a, b );
/*	loWord += mul.LowPart;
	uint64_t carry = loWord < mul.LowPart;
	hiWord += mul.HighPart;
	uint64_t c1 = hiWord < mul.HighPart;
	hhWord += c1;
	hiWord += carry;
	c1 = hiWord < carry;
	hhWord += c1;*/
	_addcarry_u32(_addcarry_u32(_addcarry_u32(0, loWord, mul.LowPart, reinterpret_cast<unsigned int*>(&(loWord))), hiWord, mul.HighPart, reinterpret_cast<unsigned int*>(&(hiWord))), hhWord, 0, reinterpret_cast<unsigned int*>(&(hhWord))); \
}

#else

inline
void multiply32x32to64(uint32_t& rhi, uint32_t& rlo, uint32_t a, uint32_t b)
{
#if defined __arm__ || defined __aarch64__

__asm("UMULL %0, %1, %2, %3\n"
      : "+r" (rlo), "+r" (rhi)
      : "r" (a), "r" (b) : "cc", "memory" );
#else // __arm__

__asm__(
"    mull  %[b]\n"
:"=d"(rhi),"=a"(rlo)
:"1"(a),[b]"rm"(b));

#endif
}

inline
void add64(uint32_t& loWord, uint32_t& hiWord, uint32_t& hhWord, uint32_t& loAdd, uint32_t& hiAdd, uint32_t& hhAdd)
{
#if defined __arm__ || defined __aarch64__
__asm("ADDS %3, %3, %0\n"
      "ADCS %1, %4, %1\n"
      "ADC %2, %5, %2"
      : "+r" (loWord), "+r" (hiWord), "+r" (hhWord)
      : "r" (loAdd), "r" (hiAdd), "r" (hhAdd) : "cc", "memory" );
#else
__asm("addl %3, %0\n"
      "adcl %4, %1\n"
      "adcl %5, %2"
      : "+g" (loWord), "+g" (hiWord), "+g" (hhWord)
      : "g" (loAdd), "g" (hiAdd), "g" (hhAdd) : "cc", "memory" );
#endif
}

FORCE_INLINE
void mul32x32to64addto96(uint32_t& loWord, uint32_t& hiWord, uint32_t& hhWord, uint32_t a, uint32_t b)
{
	uint32_t rhi;
#if defined __arm__ || defined __aarch64__
	uint32_t rlo;

__asm("UMULL %3, %4, %5, %6\n"
      "ADDS %0, %3, %0\n"
      "ADCS %1, %4, %1\n"
      "ADC %2, %2, #0x0"
      : "+r" (loWord), "+r" (hiWord), "+r" (hhWord), "=r" (rlo), "=r" (rhi)
      : "r" (a), "r" (b) : "cc" );

#else // __arm__

__asm__( "mull %5\n"
        "addl %%eax, %0\n"
        "adcl %%edx, %1\n"
        "adcl $0, %2\n"
        : "+g" (loWord), "+g" (hiWord), "+g" (hhWord), "=a" (rhi)
        :"a"(a), "g"(b) : "edx", "cc" );

#endif
}

#endif

#define PMPML_CHUNK_LOOP_INTRO_L0 \
	uint32_t ctr; \
	ctr = 0; \
	ULARGE_INTEGER__XX mul;

#ifdef _MSC_VER

#define PMPML_CHUNK_LOOP_BODY_ULI_T1( i ) \
{ \
	ULARGE_INTEGER__XX mul; \
	mul.QuadPart = UInt32x32To64(x[i], coeff[i]); \
	_addcarry_u32(_addcarry_u32(_addcarry_u32(0, constTerm.LowPart, mul.LowPart, reinterpret_cast<unsigned int*>(&(constTerm.LowPart))), constTerm.HighPart, mul.HighPart, reinterpret_cast<unsigned int*>(&(constTerm.HighPart))), ctr, 0, reinterpret_cast<unsigned int*>(&(ctr))); \
}

#define PMPML_CHUNK_LOOP_BODY_ULI_T1_LAST \
{\
	ULARGE_INTEGER__XX mul; \
	mul.QuadPart = UInt32x32To64(xLast, coeff[size]); \
	_addcarry_u32(_addcarry_u32(_addcarry_u32(0, constTerm.LowPart, mul.LowPart, reinterpret_cast<unsigned int*>(&(constTerm.LowPart))), constTerm.HighPart, mul.HighPart, reinterpret_cast<unsigned int*>(&(constTerm.HighPart))), ctr, 0, reinterpret_cast<unsigned int*>(&(ctr))); \
}

#define PMPML_CHUNK_LOOP_BODY_ULI_T1_LAST_FOR_JUST_1 \
{\
	_addcarry_u32(_addcarry_u32(_addcarry_u32(0, constTerm.LowPart, coeff[ size ], reinterpret_cast<unsigned int*>(&(constTerm.LowPart))), constTerm.HighPart, 0, reinterpret_cast<unsigned int*>(&(constTerm.HighPart))), ctr, 0, reinterpret_cast<unsigned int*>(&(ctr))); \
}

#else // _MSC_VER

#define PMPML_CHUNK_LOOP_BODY_ULI_T1( i ) \
    /*multiply32x32to64(mul.HighPart, mul.LowPart, x[i], coeff[ i ]); \
    add64(constTerm.LowPart, constTerm.HighPart, ctr, mul.LowPart, mul.HighPart, zero);*/ \
    mul32x32to64addto96(constTerm.LowPart, constTerm.HighPart, ctr, x[i], coeff[ i ]); \

#define PMPML_CHUNK_LOOP_BODY_ULI_T1_LAST \
    /*multiply32x32to64(mul.HighPart, mul.LowPart, xLast, coeff[ size ]); \
	add64(constTerm.LowPart, constTerm.HighPart, ctr, mul.LowPart, mul.HighPart);*/ \
    mul32x32to64addto96(constTerm.LowPart, constTerm.HighPart, ctr, xLast, coeff[ size ]); \

#endif // _MSC_VER

#define PMPML_CHUNK_LOOP_PRE_REDUCE_L0

/*
#define PMPML_MOD_2_32_PLUS_15( x, y ) \
	x = (uint32_t)x + UINT64_C(0xF000000E1) - (( (uint64_t)x >> 32 ) << 4) + ( x >> 32 ); \
	y = (uint32_t)x; \
	y -= ((uint32_t)(x >> 32 )) * 15; \
	if ( y < 0 ) y += PMPML_MAIN_PRIME; // y += PMPML_MAIN_PRIME * ( y < 0 );
	*/

#define PMPML_CHUNK_REDUCE_96_TO_64

#define PMPML_CHUNK_REDUCE_64_TO_32 \
{ \
	uint32_t lo, hi; \
	multiply32x32to64(hi, lo, constTerm.HighPart, 15); \
	uint32_t part = ctr * 225 + (hi << 4) - hi + 15; \
	constTerm.LowPart += part; \
	constTerm.HighPart = 1 + (constTerm.LowPart < part); \
	constTerm.HighPart -= (constTerm.LowPart < lo); \
	constTerm.LowPart -= lo; \
	if ( _LIKELY_BRANCH_( constTerm.LowPart >= 30) ) { constTerm.LowPart -= constTerm.HighPart * 15; constTerm.HighPart = 0; } \
	else \
	{ \
		if ( constTerm.HighPart ) \
		{ \
			constTerm.LowPart -= constTerm.HighPart * 15; \
			constTerm.HighPart = 1; \
			if ( _LIKELY_BRANCH_( constTerm.LowPart >= 15)) { constTerm.LowPart -= 15; constTerm.HighPart = 0; } \
			else \
			{ \
				constTerm.LowPart -= 15; \
				constTerm.HighPart = 0; \
			} \
		} \
	} \
}

#define PMPML_FULL_REDUCE_MOD_2_32_PLUS_15_AND_RETURN \
	PMPML_CHUNK_REDUCE_96_TO_64 \
	PMPML_CHUNK_REDUCE_64_TO_32 \
	return constTerm.QuadPart;

#define PMPML_FULL_REDUCE_MOD_2_32_PLUS_15_AND_RETURN_RETURN_32x32_ONLY \
{ \
	constTerm.QuadPart = constTerm.LowPart + PMPML_MAIN_PRIME - constTerm.HighPart * UINT64_C( 15 ); \
	if ( _LIKELY_BRANCH_( constTerm.LowPart >= 30) ) { constTerm.LowPart -= (constTerm.HighPart << 4) - constTerm.HighPart; return fmix32_short( constTerm.LowPart ); } \
	else \
	{ \
		constTerm.LowPart -= constTerm.HighPart * 15; \
		if ( constTerm.LowPart < 30 ) return fmix32_short( constTerm.LowPart ); \
		else \
		{ \
			constTerm.LowPart += 15; \
			return fmix32_short( constTerm.LowPart ); \
		} \
	} \
}

#define PMPML_FULL_REDUCE_MOD_2_32_PLUS_15_AND_RETURN_RETURN \
{ \
	uint32_t lo, hi; \
	multiply32x32to64(hi, lo, constTerm.HighPart, 15); \
	uint32_t part = ctr * 225 + (hi << 4) - hi + 15; \
	constTerm.LowPart += part; \
	constTerm.HighPart = 1 + (constTerm.LowPart < part); \
	constTerm.HighPart -= (constTerm.LowPart < lo); \
	constTerm.LowPart -= lo; \
	if ( _LIKELY_BRANCH_( constTerm.LowPart >= 30) ) { constTerm.LowPart -= (constTerm.HighPart << 4) - constTerm.HighPart/*constTerm.HighPart * 15*/; return fmix32_short( constTerm.LowPart ); } \
	else \
	{ \
		if ( constTerm.HighPart ) \
		{ \
			constTerm.LowPart -= constTerm.HighPart * 15 - 15; \
			constTerm.HighPart = 1; \
			if ( _LIKELY_BRANCH_( constTerm.LowPart >= 15)) { constTerm.LowPart -= 15; return fmix32_short( constTerm.LowPart ); } \
			else \
			{ \
				return constTerm.LowPart; \
			} \
		} \
		else \
			return fmix32_short( constTerm.LowPart ); \
	} \
}
#endif


class PMP_Multilinear_Hasher
{
  private:
  random_data_for_MPSHF* curr_rd;
#ifdef PMPML_USE_SSE
  const unsigned char* base_addr;
#endif

  // calls to be done from LEVEL=0
  FORCE_INLINE 
  uint64_t hash_of_string_chunk_compact( const uint32_t* coeff, ULARGE_INTEGER__XX constTerm, const uint32_t* x ) const
  {
	PMPML_CHUNK_LOOP_INTRO_L0

#ifdef PMPML_USE_SSE
#if PMPML_USE_SSE_SIZE == 128
	__m128i ctr0, ctr1, mask_low;
	__m128i a, data, product, temp;
	uint64_t temp_fin;
	int i;

	ctr0 = _mm_setzero_si128 (); // Sets the 128-bit value to zero.
	ctr1 = _mm_setzero_si128 ();
	mask_low = _mm_set_epi32 ( 0, -1, 0 , -1 );

	uint32_t *x1, *x2, *x3, *c1, *c2, *c3;

#if (PMPML_CHUNK_SIZE >= 64)
	for ( i=0; i<PMPML_CHUNK_SIZE; i+=64 )
#elif (PMPML_CHUNK_SIZE >= 32)
	for ( i=0; i<PMPML_CHUNK_SIZE; i+=32 )
#elif (PMPML_CHUNK_SIZE >= 16)
	for ( i=0; i<PMPML_CHUNK_SIZE; i+=16 )
#elif (PMPML_CHUNK_SIZE >= 8)
	for ( i=0; i<PMPML_CHUNK_SIZE; i+=8 )
#elif (PMPML_CHUNK_SIZE >= 4)
	for ( i=0; i<PMPML_CHUNK_SIZE; i+=4 )
#else
#error PMPML_CHUNK_SIZE is incompatible with PMPML_USE_SSE option 
#endif
	{
		a = _mm_load_si128 ((__m128i *)(coeff+i)); // Loads 128-bit value. Address p must be 16-byte aligned.
		data = _mm_loadu_si128 ((__m128i *)(x+i)); // Loads 128-bit value. Address p does not need be 16-byte aligned.
		product = _mm_mul_epu32 ( data, a); // A 128-bit value that contains two 64-bit unsigned integers. The result can be expressed by the following equations. r0 := a0 * b0; r1 := a2 * b2
		temp = _mm_srli_epi64( product, 32 ); // Shifts the 2 signed or unsigned 64-bit integers in a right by count bits while shifting in zeros.
		ctr1 = _mm_add_epi64 ( ctr1, temp );
		//temp = _mm_and_si128 ( mask_low, product );
		ctr0 = _mm_add_epi64 ( ctr0, product );//ctr0 = _mm_add_epi64 ( ctr0, temp );

		a = _mm_shuffle_epi32( a, 1*1+0*4+3*16+2*64 );
		data = _mm_shuffle_epi32( data, 1*1+0*4+3*16+2*64 );
		product = _mm_mul_epu32 ( data, a);
		temp = _mm_srli_epi64( product, 32 );
		ctr1 = _mm_add_epi64 ( ctr1, temp );
		//temp = _mm_and_si128 ( mask_low, product );
		ctr0 = _mm_add_epi64 ( ctr0, product );//ctr0 = _mm_add_epi64 ( ctr0, temp );

#if ( PMPML_CHUNK_SIZE > 4 )

		a = _mm_load_si128 ((__m128i *)(coeff+i+4));
		data = _mm_loadu_si128 ((__m128i *)(x+i+4));
		product = _mm_mul_epu32 ( data, a);
		temp = _mm_srli_epi64( product, 32 );
		ctr1 = _mm_add_epi64 ( ctr1, temp );
		//temp = _mm_and_si128 ( mask_low, product );
		ctr0 = _mm_add_epi64 ( ctr0, product );//ctr0 = _mm_add_epi64 ( ctr0, temp );

		a = _mm_shuffle_epi32( a, 1*1+0*4+3*16+2*64 );
		data = _mm_shuffle_epi32( data, 1*1+0*4+3*16+2*64 );
		product = _mm_mul_epu32 ( data, a);
		temp = _mm_srli_epi64( product, 32 );
		ctr1 = _mm_add_epi64 ( ctr1, temp );
		//temp = _mm_and_si128 ( mask_low, product );
		ctr0 = _mm_add_epi64 ( ctr0, product );//ctr0 = _mm_add_epi64 ( ctr0, temp );

#endif // ( PMPML_CHUNK_SIZE > 4 )
#if ( PMPML_CHUNK_SIZE > 8 )

		a = _mm_load_si128 ((__m128i *)(coeff+i+8));
		data = _mm_loadu_si128 ((__m128i *)(x+i+8));
		product = _mm_mul_epu32 ( data, a);
		temp = _mm_srli_epi64( product, 32 );
		ctr1 = _mm_add_epi64 ( ctr1, temp );
		//temp = _mm_and_si128 ( mask_low, product );
		ctr0 = _mm_add_epi64 ( ctr0, product );//ctr0 = _mm_add_epi64 ( ctr0, temp );

		a = _mm_shuffle_epi32( a, 1*1+0*4+3*16+2*64 );
		data = _mm_shuffle_epi32( data, 1*1+0*4+3*16+2*64 );
		product = _mm_mul_epu32 ( data, a);
		temp = _mm_srli_epi64( product, 32 );
		ctr1 = _mm_add_epi64 ( ctr1, temp );
		//temp = _mm_and_si128 ( mask_low, product );
		ctr0 = _mm_add_epi64 ( ctr0, product );//ctr0 = _mm_add_epi64 ( ctr0, temp );


		a = _mm_load_si128 ((__m128i *)(coeff+i+12));
		data = _mm_loadu_si128 ((__m128i *)(x+i+12));
		product = _mm_mul_epu32 ( data, a);
		temp = _mm_srli_epi64( product, 32 );
		ctr1 = _mm_add_epi64 ( ctr1, temp );
		//temp = _mm_and_si128 ( mask_low, product );
		ctr0 = _mm_add_epi64 ( ctr0, product );//ctr0 = _mm_add_epi64 ( ctr0, temp );

		a = _mm_shuffle_epi32( a, 1*1+0*4+3*16+2*64 );
		data = _mm_shuffle_epi32( data, 1*1+0*4+3*16+2*64 );
		product = _mm_mul_epu32 ( data, a);
		temp = _mm_srli_epi64( product, 32 );
		ctr1 = _mm_add_epi64 ( ctr1, temp );
		//temp = _mm_and_si128 ( mask_low, product );
		ctr0 = _mm_add_epi64 ( ctr0, product );//ctr0 = _mm_add_epi64 ( ctr0, temp );

#endif // ( PMPML_CHUNK_SIZE > 8 )
#if ( PMPML_CHUNK_SIZE > 16 )

		a = _mm_load_si128 ((__m128i *)(coeff+i+16));
		data = _mm_loadu_si128 ((__m128i *)(x+i+16));
		product = _mm_mul_epu32 ( data, a);
		temp = _mm_srli_epi64( product, 32 );
		ctr1 = _mm_add_epi64 ( ctr1, temp );
		//temp = _mm_and_si128 ( mask_low, product );
		ctr0 = _mm_add_epi64 ( ctr0, product );//ctr0 = _mm_add_epi64 ( ctr0, temp );

		a = _mm_shuffle_epi32( a, 1*1+0*4+3*16+2*64 );
		data = _mm_shuffle_epi32( data, 1*1+0*4+3*16+2*64 );
		product = _mm_mul_epu32 ( data, a);
		temp = _mm_srli_epi64( product, 32 );
		ctr1 = _mm_add_epi64 ( ctr1, temp );
		//temp = _mm_and_si128 ( mask_low, product );
		ctr0 = _mm_add_epi64 ( ctr0, product );//ctr0 = _mm_add_epi64 ( ctr0, temp );


		a = _mm_load_si128 ((__m128i *)(coeff+i+20));
		data = _mm_loadu_si128 ((__m128i *)(x+i+20));
		product = _mm_mul_epu32 ( data, a);
		temp = _mm_srli_epi64( product, 32 );
		ctr1 = _mm_add_epi64 ( ctr1, temp );
		//temp = _mm_and_si128 ( mask_low, product );
		ctr0 = _mm_add_epi64 ( ctr0, product );//ctr0 = _mm_add_epi64 ( ctr0, temp );

		a = _mm_shuffle_epi32( a, 1*1+0*4+3*16+2*64 );
		data = _mm_shuffle_epi32( data, 1*1+0*4+3*16+2*64 );
		product = _mm_mul_epu32 ( data, a);
		temp = _mm_srli_epi64( product, 32 );
		ctr1 = _mm_add_epi64 ( ctr1, temp );
		//temp = _mm_and_si128 ( mask_low, product );
		ctr0 = _mm_add_epi64 ( ctr0, product );//ctr0 = _mm_add_epi64 ( ctr0, temp );


		a = _mm_load_si128 ((__m128i *)(coeff+i+24));
		data = _mm_loadu_si128 ((__m128i *)(x+i+24));
		product = _mm_mul_epu32 ( data, a);
		temp = _mm_srli_epi64( product, 32 );
		ctr1 = _mm_add_epi64 ( ctr1, temp );
		//temp = _mm_and_si128 ( mask_low, product );
		ctr0 = _mm_add_epi64 ( ctr0, product );//ctr0 = _mm_add_epi64 ( ctr0, temp );

		a = _mm_shuffle_epi32( a, 1*1+0*4+3*16+2*64 );
		data = _mm_shuffle_epi32( data, 1*1+0*4+3*16+2*64 );
		product = _mm_mul_epu32 ( data, a);
		temp = _mm_srli_epi64( product, 32 );
		ctr1 = _mm_add_epi64 ( ctr1, temp );
		//temp = _mm_and_si128 ( mask_low, product );
		ctr0 = _mm_add_epi64 ( ctr0, product );//ctr0 = _mm_add_epi64 ( ctr0, temp );


		a = _mm_load_si128 ((__m128i *)(coeff+i+28));
		data = _mm_loadu_si128 ((__m128i *)(x+i+28));
		product = _mm_mul_epu32 ( data, a);
		temp = _mm_srli_epi64( product, 32 );
		ctr1 = _mm_add_epi64 ( ctr1, temp );
		//temp = _mm_and_si128 ( mask_low, product );
		ctr0 = _mm_add_epi64 ( ctr0, product );//ctr0 = _mm_add_epi64 ( ctr0, temp );

		a = _mm_shuffle_epi32( a, 1*1+0*4+3*16+2*64 );
		data = _mm_shuffle_epi32( data, 1*1+0*4+3*16+2*64 );
		product = _mm_mul_epu32 ( data, a);
		temp = _mm_srli_epi64( product, 32 );
		ctr1 = _mm_add_epi64 ( ctr1, temp );
		//temp = _mm_and_si128 ( mask_low, product );
		ctr0 = _mm_add_epi64 ( ctr0, product );//ctr0 = _mm_add_epi64 ( ctr0, temp );

#endif // ( PMPML_CHUNK_SIZE > 16 )
#if ( PMPML_CHUNK_SIZE > 32 )		
		
		x1 = const_cast<uint32_t*>( x+i+36 );
		x2 = const_cast<uint32_t*>( x+i+40 );
		x3 = const_cast<uint32_t*>( x+i+44 );
		c1 = const_cast<uint32_t*>( coeff+i+36 );
		c2 = const_cast<uint32_t*>( coeff+i+40 );
		c3 = const_cast<uint32_t*>( coeff+i+44 );
		a = _mm_load_si128 ((__m128i *)(coeff+i+32));
		data = _mm_loadu_si128 ((__m128i *)(x+i+32));
		product = _mm_mul_epu32 ( data, a);
		temp = _mm_srli_epi64( product, 32 );
		ctr1 = _mm_add_epi64 ( ctr1, temp );
		//temp = _mm_and_si128 ( mask_low, product );
		ctr0 = _mm_add_epi64 ( ctr0, product );//ctr0 = _mm_add_epi64 ( ctr0, temp );

		a = _mm_shuffle_epi32( a, 1*1+0*4+3*16+2*64 );
		data = _mm_shuffle_epi32( data, 1*1+0*4+3*16+2*64 );
		product = _mm_mul_epu32 ( data, a);
		temp = _mm_srli_epi64( product, 32 );
		ctr1 = _mm_add_epi64 ( ctr1, temp );
		//temp = _mm_and_si128 ( mask_low, product );
		ctr0 = _mm_add_epi64 ( ctr0, product );//ctr0 = _mm_add_epi64 ( ctr0, temp );


		a = _mm_load_si128 ((__m128i *)(c1));
		data = _mm_loadu_si128 ((__m128i *)(x1));
		product = _mm_mul_epu32 ( data, a);
		temp = _mm_srli_epi64( product, 32 );
		ctr1 = _mm_add_epi64 ( ctr1, temp );
		//temp = _mm_and_si128 ( mask_low, product );
		ctr0 = _mm_add_epi64 ( ctr0, product );//ctr0 = _mm_add_epi64 ( ctr0, temp );

		a = _mm_shuffle_epi32( a, 1*1+0*4+3*16+2*64 );
		data = _mm_shuffle_epi32( data, 1*1+0*4+3*16+2*64 );
		product = _mm_mul_epu32 ( data, a);
		temp = _mm_srli_epi64( product, 32 );
		ctr1 = _mm_add_epi64 ( ctr1, temp );
		//temp = _mm_and_si128 ( mask_low, product );
		ctr0 = _mm_add_epi64 ( ctr0, product );//ctr0 = _mm_add_epi64 ( ctr0, temp );


		a = _mm_load_si128 ((__m128i *)(c2));
		data = _mm_loadu_si128 ((__m128i *)(x2));
		product = _mm_mul_epu32 ( data, a);
		temp = _mm_srli_epi64( product, 32 );
		ctr1 = _mm_add_epi64 ( ctr1, temp );
		//temp = _mm_and_si128 ( mask_low, product );
		ctr0 = _mm_add_epi64 ( ctr0, product );//ctr0 = _mm_add_epi64 ( ctr0, temp );

		a = _mm_shuffle_epi32( a, 1*1+0*4+3*16+2*64 );
		data = _mm_shuffle_epi32( data, 1*1+0*4+3*16+2*64 );
		product = _mm_mul_epu32 ( data, a);
		temp = _mm_srli_epi64( product, 32 );
		ctr1 = _mm_add_epi64 ( ctr1, temp );
		//temp = _mm_and_si128 ( mask_low, product );
		ctr0 = _mm_add_epi64 ( ctr0, product );//ctr0 = _mm_add_epi64 ( ctr0, temp );


		a = _mm_load_si128 ((__m128i *)(c3));
		data = _mm_loadu_si128 ((__m128i *)(x3));
		product = _mm_mul_epu32 ( data, a);
		temp = _mm_srli_epi64( product, 32 );
		ctr1 = _mm_add_epi64 ( ctr1, temp );
		//temp = _mm_and_si128 ( mask_low, product );
		ctr0 = _mm_add_epi64 ( ctr0, product );//ctr0 = _mm_add_epi64 ( ctr0, temp );

		a = _mm_shuffle_epi32( a, 1*1+0*4+3*16+2*64 );
		data = _mm_shuffle_epi32( data, 1*1+0*4+3*16+2*64 );
		product = _mm_mul_epu32 ( data, a);
		temp = _mm_srli_epi64( product, 32 );
		ctr1 = _mm_add_epi64 ( ctr1, temp );
		//temp = _mm_and_si128 ( mask_low, product );
		ctr0 = _mm_add_epi64 ( ctr0, product );//ctr0 = _mm_add_epi64 ( ctr0, temp );


		x1 = const_cast<uint32_t*>( x+i+52 );
		x2 = const_cast<uint32_t*>( x+i+56 );
		x3 = const_cast<uint32_t*>( x+i+60 );
		c1 = const_cast<uint32_t*>( coeff+i+52 );
		c2 = const_cast<uint32_t*>( coeff+i+56 );
		c3 = const_cast<uint32_t*>( coeff+i+60 );
		a = _mm_load_si128 ((__m128i *)(coeff+i+48));
		data = _mm_loadu_si128 ((__m128i *)(x+i+48));
		product = _mm_mul_epu32 ( data, a);
		temp = _mm_srli_epi64( product, 32 );
		ctr1 = _mm_add_epi64 ( ctr1, temp );
		//temp = _mm_and_si128 ( mask_low, product );
		ctr0 = _mm_add_epi64 ( ctr0, product );//ctr0 = _mm_add_epi64 ( ctr0, temp );

		a = _mm_shuffle_epi32( a, 1*1+0*4+3*16+2*64 );
		data = _mm_shuffle_epi32( data, 1*1+0*4+3*16+2*64 );
		product = _mm_mul_epu32 ( data, a);
		temp = _mm_srli_epi64( product, 32 );
		ctr1 = _mm_add_epi64 ( ctr1, temp );
		//temp = _mm_and_si128 ( mask_low, product );
		ctr0 = _mm_add_epi64 ( ctr0, product );//ctr0 = _mm_add_epi64 ( ctr0, temp );


		a = _mm_load_si128 ((__m128i *)(c1));
		data = _mm_loadu_si128 ((__m128i *)(x1));
		product = _mm_mul_epu32 ( data, a);
		temp = _mm_srli_epi64( product, 32 );
		ctr1 = _mm_add_epi64 ( ctr1, temp );
		//temp = _mm_and_si128 ( mask_low, product );
		ctr0 = _mm_add_epi64 ( ctr0, product );//ctr0 = _mm_add_epi64 ( ctr0, temp );

		a = _mm_shuffle_epi32( a, 1*1+0*4+3*16+2*64 );
		data = _mm_shuffle_epi32( data, 1*1+0*4+3*16+2*64 );
		product = _mm_mul_epu32 ( data, a);
		temp = _mm_srli_epi64( product, 32 );
		ctr1 = _mm_add_epi64 ( ctr1, temp );
		//temp = _mm_and_si128 ( mask_low, product );
		ctr0 = _mm_add_epi64 ( ctr0, product );//ctr0 = _mm_add_epi64 ( ctr0, temp );


		a = _mm_load_si128 ((__m128i *)(c2));
		data = _mm_loadu_si128 ((__m128i *)(x2));
		product = _mm_mul_epu32 ( data, a);
		temp = _mm_srli_epi64( product, 32 );
		ctr1 = _mm_add_epi64 ( ctr1, temp );
		//temp = _mm_and_si128 ( mask_low, product );
		ctr0 = _mm_add_epi64 ( ctr0, product );//ctr0 = _mm_add_epi64 ( ctr0, temp );

		a = _mm_shuffle_epi32( a, 1*1+0*4+3*16+2*64 );
		data = _mm_shuffle_epi32( data, 1*1+0*4+3*16+2*64 );
		product = _mm_mul_epu32 ( data, a);
		temp = _mm_srli_epi64( product, 32 );
		ctr1 = _mm_add_epi64 ( ctr1, temp );
		//temp = _mm_and_si128 ( mask_low, product );
		ctr0 = _mm_add_epi64 ( ctr0, product );//ctr0 = _mm_add_epi64 ( ctr0, temp );


		a = _mm_load_si128 ((__m128i *)(c3));
		data = _mm_loadu_si128 ((__m128i *)(x3));
		product = _mm_mul_epu32 ( data, a);
		temp = _mm_srli_epi64( product, 32 );
		ctr1 = _mm_add_epi64 ( ctr1, temp );
		//temp = _mm_and_si128 ( mask_low, product );
		ctr0 = _mm_add_epi64 ( ctr0, product );//ctr0 = _mm_add_epi64 ( ctr0, temp );

		a = _mm_shuffle_epi32( a, 1*1+0*4+3*16+2*64 );
		data = _mm_shuffle_epi32( data, 1*1+0*4+3*16+2*64 );
		product = _mm_mul_epu32 ( data, a);
		temp = _mm_srli_epi64( product, 32 );
		ctr1 = _mm_add_epi64 ( ctr1, temp );
		//temp = _mm_and_si128 ( mask_low, product );
		ctr0 = _mm_add_epi64 ( ctr0, product );//ctr0 = _mm_add_epi64 ( ctr0, temp );
#endif // ( PMPML_CHUNK_SIZE > 32 )
	}


	temp = _mm_unpackhi_epi64 ( ctr0, ctr1 ); // Interleaves the upper signed or unsigned 64-bit integer in a with the upper signed or unsigned 64-bit integer in b. r0 := a1 ; r1 := b1
	data = _mm_unpacklo_epi64 ( ctr0, ctr1 ); // Interleaves the lower signed or unsigned 64-bit integer in a with the lower signed or unsigned 64-bit integer in b. r0 := a0 ; r1 := b0
	ctr1 = _mm_add_epi64 ( data, temp );

#if (PMPML_CHUNK_OPTIMIZATION_TYPE== 1) 
#if defined(_MSC_VER)
	constTerm.QuadPart += ctr1.m128i_u32[0]; // Microsoft specific
	ctr.QuadPart += ctr1.m128i_u64[1] + ctr1.m128i_u32[1];
#elif defined( __SSE4_1__)
	constTer.QuadPart += _mm_extract_epi32(ctr1,0);
	ctr.QuadPart += _mm_extract_epi64(ctr1,0) + _mm_extract_epi32(ctr1,1);
#else
        uint32_t b[4]; 
        _mm_storeu_si128((__m128i *)b,ctr1);
	constTerm.QuadPart += b[0]; 
	ctr.QuadPart += b[1] + b[2] + ((uint64_t) b[3] <<32);
#endif
#elif (PMPML_CHUNK_OPTIMIZATION_TYPE== 2) 
	uint64_t lo = ((uint64_t*)(&ctr1))[0];
	uint64_t hi = ((uint64_t*)(&ctr1))[1];
/*	constTerm.QuadPart += lo;
	ctr += constTerm.QuadPart < lo;
	constTerm.HighPart += ((uint32_t*)(&ctr1))[2];
	ctr += constTerm.HighPart < ((uint32_t*)(&ctr1))[2];
	ctr +=  ((uint32_t*)(&ctr1))[3];*/
	uint32_t lohi = lo >> 32;
	uint32_t hilo = hi;
	uint32_t diff = lohi - hilo;
	hi += diff;
	lo = (uint32_t)lo + (((uint64_t)(uint32_t)hi) << 32 );
	constTerm.QuadPart += lo;
	ctr += constTerm.QuadPart < lo;
	ctr += hi >> 32;
#else
#error unexpected PMPML_CHUNK_OPTIMIZATION_TYPE
#endif

#elif PMPML_USE_SSE_SIZE == 256
// DANIEL: Code below will fail to compile on a machine with no AVX2 support, or without the right
// -march flag. Please consider testing whether AVX2 is supported, e.g., see http://stackoverflow.com/questions/25820290/how-verify-that-operating-system-support-avx2-instructions
	__m256i ctr0, ctr1, mask_low;
	__m256i a, data, product, temp;
	uint64_t temp_fin;
	int i;

	ctr0 = _mm256_setzero_si256 (); // Sets the 128-bit value to zero.
	ctr1 = _mm256_setzero_si256 ();
	mask_low = _mm256_set_epi32 ( 0, -1, 0 , -1, 0, -1, 0 , -1 );

	uint32_t *x1, *x2, *x3, *c1, *c2, *c3;

#if (PMPML_CHUNK_SIZE >= 64)
	for ( i=0; i<PMPML_CHUNK_SIZE; i+=64 )
#elif (PMPML_CHUNK_SIZE >= 32)
	for ( i=0; i<PMPML_CHUNK_SIZE; i+=32 )
#elif (PMPML_CHUNK_SIZE >= 16)
	for ( i=0; i<PMPML_CHUNK_SIZE; i+=16 )
#elif (PMPML_CHUNK_SIZE >= 8)
	for ( i=0; i<PMPML_CHUNK_SIZE; i+=8 )
#else
#error PMPML_CHUNK_SIZE is incompatible with PMPML_USE_SSE option 
#endif
	{
		a = _mm256_load_si256 ((__m256i *)(coeff+i)); // Loads 256-bit value. Address p must be 32-byte aligned.
		data = _mm256_loadu_si256 ((__m256i *)(x+i)); // Loads 256-bit value. Address p does not need be 32-byte aligned.
		product = _mm256_mul_epu32 ( data, a); // A 256-bit value that contains four 64-bit unsigned integers. The result can be expressed by the following equations. r0 := a0 * b0; r1 := a2 * b2; ...
		temp = _mm256_srli_epi64( product, 32 ); // Shifts the 4 signed or unsigned 64-bit integers in a right by count bits while shifting in zeros.
		ctr1 = _mm256_add_epi64 ( ctr1, temp );
		//temp = _mm256_and_si256 ( mask_low, product );
		ctr0 = _mm256_add_epi64 ( ctr0, product );//ctr0 = _mm256_add_epi64 ( ctr0, temp );

		a = _mm256_shuffle_epi32( a, 1*1+0*4+3*16+2*64 );
		data = _mm256_shuffle_epi32( data, 1*1+0*4+3*16+2*64 );
		product = _mm256_mul_epu32 ( data, a);
		temp = _mm256_srli_epi64( product, 32 );
		ctr1 = _mm256_add_epi64 ( ctr1, temp );
		//temp = _mm256_and_si256 ( mask_low, product );
		ctr0 = _mm256_add_epi64 ( ctr0, product );//ctr0 = _mm256_add_epi64 ( ctr0, temp );

#if ( PMPML_CHUNK_SIZE > 8 )

		a = _mm256_load_si256 ((__m256i *)(coeff+i+8));
		data = _mm256_loadu_si256 ((__m256i *)(x+i+8));
		product = _mm256_mul_epu32 ( data, a);
		temp = _mm256_srli_epi64( product, 32 );
		ctr1 = _mm256_add_epi64 ( ctr1, temp );
		//temp = _mm256_and_si256 ( mask_low, product );
		ctr0 = _mm256_add_epi64 ( ctr0, product );//ctr0 = _mm256_add_epi64 ( ctr0, temp );

		a = _mm256_shuffle_epi32( a, 1*1+0*4+3*16+2*64 );
		data = _mm256_shuffle_epi32( data, 1*1+0*4+3*16+2*64 );
		product = _mm256_mul_epu32 ( data, a);
		temp = _mm256_srli_epi64( product, 32 );
		ctr1 = _mm256_add_epi64 ( ctr1, temp );
		//temp = _mm256_and_si256 ( mask_low, product );
		ctr0 = _mm256_add_epi64 ( ctr0, product );//ctr0 = _mm256_add_epi64 ( ctr0, temp );

#endif // ( PMPML_CHUNK_SIZE > 8 )
#if ( PMPML_CHUNK_SIZE > 16 )

		a = _mm256_load_si256 ((__m256i *)(coeff+i+16));
		data = _mm256_loadu_si256 ((__m256i *)(x+i+16));
		product = _mm256_mul_epu32 ( data, a);
		temp = _mm256_srli_epi64( product, 32 );
		ctr1 = _mm256_add_epi64 ( ctr1, temp );
		//temp = _mm256_and_si256 ( mask_low, product );
		ctr0 = _mm256_add_epi64 ( ctr0, product );//ctr0 = _mm256_add_epi64 ( ctr0, temp );

		a = _mm256_shuffle_epi32( a, 1*1+0*4+3*16+2*64 );
		data = _mm256_shuffle_epi32( data, 1*1+0*4+3*16+2*64 );
		product = _mm256_mul_epu32 ( data, a);
		temp = _mm256_srli_epi64( product, 32 );
		ctr1 = _mm256_add_epi64 ( ctr1, temp );
		//temp = _mm256_and_si256 ( mask_low, product );
		ctr0 = _mm256_add_epi64 ( ctr0, product );//ctr0 = _mm256_add_epi64 ( ctr0, temp );

		a = _mm256_load_si256 ((__m256i *)(coeff+i+24));
		data = _mm256_loadu_si256 ((__m256i *)(x+i+24));
		product = _mm256_mul_epu32 ( data, a);
		temp = _mm256_srli_epi64( product, 32 );
		ctr1 = _mm256_add_epi64 ( ctr1, temp );
		//temp = _mm256_and_si256 ( mask_low, product );
		ctr0 = _mm256_add_epi64 ( ctr0, product );//ctr0 = _mm256_add_epi64 ( ctr0, temp );

		a = _mm256_shuffle_epi32( a, 1*1+0*4+3*16+2*64 );
		data = _mm256_shuffle_epi32( data, 1*1+0*4+3*16+2*64 );
		product = _mm256_mul_epu32 ( data, a);
		temp = _mm256_srli_epi64( product, 32 );
		ctr1 = _mm256_add_epi64 ( ctr1, temp );
		//temp = _mm256_and_si256 ( mask_low, product );
		ctr0 = _mm256_add_epi64 ( ctr0, product );//ctr0 = _mm256_add_epi64 ( ctr0, temp );

#endif // ( PMPML_CHUNK_SIZE > 16 )
#if ( PMPML_CHUNK_SIZE > 32 )

		a = _mm256_load_si256 ((__m256i *)(coeff+i+32));
		data = _mm256_loadu_si256 ((__m256i *)(x+i+32));
		product = _mm256_mul_epu32 ( data, a);
		temp = _mm256_srli_epi64( product, 32 );
		ctr1 = _mm256_add_epi64 ( ctr1, temp );
		//temp = _mm256_and_si256 ( mask_low, product );
		ctr0 = _mm256_add_epi64 ( ctr0, product );//ctr0 = _mm256_add_epi64 ( ctr0, temp );

		a = _mm256_shuffle_epi32( a, 1*1+0*4+3*16+2*64 );
		data = _mm256_shuffle_epi32( data, 1*1+0*4+3*16+2*64 );
		product = _mm256_mul_epu32 ( data, a);
		temp = _mm256_srli_epi64( product, 32 );
		ctr1 = _mm256_add_epi64 ( ctr1, temp );
		//temp = _mm256_and_si256 ( mask_low, product );
		ctr0 = _mm256_add_epi64 ( ctr0, product );//ctr0 = _mm256_add_epi64 ( ctr0, temp );

		a = _mm256_load_si256 ((__m256i *)(coeff+i+40));
		data = _mm256_loadu_si256 ((__m256i *)(x+i+40));
		product = _mm256_mul_epu32 ( data, a);
		temp = _mm256_srli_epi64( product, 32 );
		ctr1 = _mm256_add_epi64 ( ctr1, temp );
		//temp = _mm256_and_si256 ( mask_low, product );
		ctr0 = _mm256_add_epi64 ( ctr0, product );//ctr0 = _mm256_add_epi64 ( ctr0, temp );

		a = _mm256_shuffle_epi32( a, 1*1+0*4+3*16+2*64 );
		data = _mm256_shuffle_epi32( data, 1*1+0*4+3*16+2*64 );
		product = _mm256_mul_epu32 ( data, a);
		temp = _mm256_srli_epi64( product, 32 );
		ctr1 = _mm256_add_epi64 ( ctr1, temp );
		//temp = _mm256_and_si256 ( mask_low, product );
		ctr0 = _mm256_add_epi64 ( ctr0, product );//ctr0 = _mm256_add_epi64 ( ctr0, temp );

		a = _mm256_load_si256 ((__m256i *)(coeff+i+48));
		data = _mm256_loadu_si256 ((__m256i *)(x+i+48));
		product = _mm256_mul_epu32 ( data, a);
		temp = _mm256_srli_epi64( product, 32 );
		ctr1 = _mm256_add_epi64 ( ctr1, temp );
		//temp = _mm256_and_si256 ( mask_low, product );
		ctr0 = _mm256_add_epi64 ( ctr0, product );//ctr0 = _mm256_add_epi64 ( ctr0, temp );

		a = _mm256_shuffle_epi32( a, 1*1+0*4+3*16+2*64 );
		data = _mm256_shuffle_epi32( data, 1*1+0*4+3*16+2*64 );
		product = _mm256_mul_epu32 ( data, a);
		temp = _mm256_srli_epi64( product, 32 );
		ctr1 = _mm256_add_epi64 ( ctr1, temp );
		//temp = _mm256_and_si256 ( mask_low, product );
		ctr0 = _mm256_add_epi64 ( ctr0, product );//ctr0 = _mm256_add_epi64 ( ctr0, temp );

		a = _mm256_load_si256 ((__m256i *)(coeff+i+56));
		data = _mm256_loadu_si256 ((__m256i *)(x+i+56));
		product = _mm256_mul_epu32 ( data, a);
		temp = _mm256_srli_epi64( product, 32 );
		ctr1 = _mm256_add_epi64 ( ctr1, temp );
		//temp = _mm256_and_si256 ( mask_low, product );
		ctr0 = _mm256_add_epi64 ( ctr0, product );//ctr0 = _mm256_add_epi64 ( ctr0, temp );

		a = _mm256_shuffle_epi32( a, 1*1+0*4+3*16+2*64 );
		data = _mm256_shuffle_epi32( data, 1*1+0*4+3*16+2*64 );
		product = _mm256_mul_epu32 ( data, a);
		temp = _mm256_srli_epi64( product, 32 );
		ctr1 = _mm256_add_epi64 ( ctr1, temp );
		//temp = _mm256_and_si256 ( mask_low, product );
		ctr0 = _mm256_add_epi64 ( ctr0, product );//ctr0 = _mm256_add_epi64 ( ctr0, temp );


#endif // ( PMPML_CHUNK_SIZE > 32 )
	}


	temp = _mm256_unpackhi_epi64 ( ctr0, ctr1 ); // Interleaves the upper signed or unsigned 64-bit integer in a with the upper signed or unsigned 64-bit integer in b. r0 := a1 ; r1 := b1 ; ...
	data = _mm256_unpacklo_epi64 ( ctr0, ctr1 ); // Interleaves the lower signed or unsigned 64-bit integer in a with the lower signed or unsigned 64-bit integer in b. r0 := a0 ; r1 := b0 ; ...
	ctr1 = _mm256_add_epi64 ( data, temp );

#if (PMPML_CHUNK_OPTIMIZATION_TYPE== 1)
#ifdef 0
// revision pending 
#if defined(_MSC_VER)
	constTerm.QuadPart += ctr1.m128i_u32[0]; // Microsoft specific
	ctr.QuadPart += ctr1.m128i_u64[1] + ctr1.m128i_u32[1];
#elif defined( __SSE4_1__)
	constTer.QuadPart += _mm_extract_epi32(ctr1,0);
	ctr.QuadPart += _mm_extract_epi64(ctr1,0) + _mm_extract_epi32(ctr1,1);
#else
        uint32_t b[4]; 
        _mm_storeu_si128((__m128i *)b,ctr1);
	constTerm.QuadPart += b[0]; 
	ctr.QuadPart += b[1] + b[2] + ((uint64_t) b[3] <<32);
#endif
#else // 0
#error unsupported
#endif // 0
#elif (PMPML_CHUNK_OPTIMIZATION_TYPE== 2) 
	uint64_t lo = *(uint64_t*)(&ctr1) + ((uint64_t*)(&ctr1))[2];
	uint64_t hi = ((uint64_t*)(&ctr1))[1] + ((uint64_t*)(&ctr1))[3];
	uint32_t lohi = lo >> 32;
	uint32_t hilo = hi;
	uint32_t diff = lohi - hilo;
	hi += diff;
	lo = (uint32_t)lo + (((uint64_t)(uint32_t)hi) << 32 );
	constTerm.QuadPart += lo;
	ctr += constTerm.QuadPart < lo;
	ctr += hi >> 32;
#else
#error unexpected PMPML_CHUNK_OPTIMIZATION_TYPE
#endif

#else
#error unsupported PMPML_USE_SSE_SIZE value
#endif

#else //PMPML_USE_SSE
	for ( uint32_t i=0; i<PMPML_CHUNK_SIZE; i+=8 )
	{
		PMPML_CHUNK_LOOP_BODY_ULI_T1( 0 + i )
		PMPML_CHUNK_LOOP_BODY_ULI_T1( 1 + i )
		PMPML_CHUNK_LOOP_BODY_ULI_T1( 2 + i )
		PMPML_CHUNK_LOOP_BODY_ULI_T1( 3 + i )
#if ( PMPML_CHUNK_SIZE > 4 )
		PMPML_CHUNK_LOOP_BODY_ULI_T1( 4 + i )
		PMPML_CHUNK_LOOP_BODY_ULI_T1( 5 + i )
		PMPML_CHUNK_LOOP_BODY_ULI_T1( 6 + i )
		PMPML_CHUNK_LOOP_BODY_ULI_T1( 7 + i )
#endif
	}
#endif // PMPML_USE_SSE

	PMPML_CHUNK_LOOP_PRE_REDUCE_L0

	PMPML_FULL_REDUCE_MOD_2_32_PLUS_15_AND_RETURN
  }

#ifdef PMPML_STRICT_UNALIGNED_HANDLING
  FORCE_INLINE
  uint64_t hash_of_string_chunk_compact_un_1( const uint32_t* coeff, ULARGE_INTEGER__XX constTerm, const unsigned char* _x ) const
  {
	PMPML_CHUNK_LOOP_INTRO_L0

	const uint32_t* xx = (const uint32_t*)( _x + 3 );
	uint32_t x = xx[ -1 ] >> 8;

	for ( uint32_t i=0; i<PMPML_CHUNK_SIZE; i+=8 )
	{
		PMPML_CHUNK_LOOP_BODY_ULI_T1_UNL( 8, 0 + i )
		PMPML_CHUNK_LOOP_BODY_ULI_T1_UNL( 8, 1 + i )
		PMPML_CHUNK_LOOP_BODY_ULI_T1_UNL( 8, 2 + i )
		PMPML_CHUNK_LOOP_BODY_ULI_T1_UNL( 8, 3 + i )
#if ( PMPML_CHUNK_SIZE > 4 )
		PMPML_CHUNK_LOOP_BODY_ULI_T1_UNL( 8, 4 + i )
		PMPML_CHUNK_LOOP_BODY_ULI_T1_UNL( 8, 5 + i )
		PMPML_CHUNK_LOOP_BODY_ULI_T1_UNL( 8, 6 + i )
		PMPML_CHUNK_LOOP_BODY_ULI_T1_UNL( 8, 7 + i )
#endif
	}

	PMPML_CHUNK_LOOP_PRE_REDUCE_L0

	PMPML_FULL_REDUCE_MOD_2_32_PLUS_15_AND_RETURN
  }

  FORCE_INLINE
  uint64_t hash_of_string_chunk_compact_un_2( const uint32_t* coeff, ULARGE_INTEGER__XX constTerm, const unsigned char* _x ) const
  {
	PMPML_CHUNK_LOOP_INTRO_L0

	const uint32_t* xx = (const uint32_t*)( _x + 2 );
	uint32_t x = xx[ -1 ] >> 16;

	for ( uint32_t i=0; i<PMPML_CHUNK_SIZE; i+=8 )
	{
		PMPML_CHUNK_LOOP_BODY_ULI_T1_UNL( 16, 0 + i )
		PMPML_CHUNK_LOOP_BODY_ULI_T1_UNL( 16, 1 + i )
		PMPML_CHUNK_LOOP_BODY_ULI_T1_UNL( 16, 2 + i )
		PMPML_CHUNK_LOOP_BODY_ULI_T1_UNL( 16, 3 + i )
#if ( PMPML_CHUNK_SIZE > 4 )
		PMPML_CHUNK_LOOP_BODY_ULI_T1_UNL( 16, 4 + i )
		PMPML_CHUNK_LOOP_BODY_ULI_T1_UNL( 16, 5 + i )
		PMPML_CHUNK_LOOP_BODY_ULI_T1_UNL( 16, 6 + i )
		PMPML_CHUNK_LOOP_BODY_ULI_T1_UNL( 16, 7 + i )
#endif
	}

	PMPML_CHUNK_LOOP_PRE_REDUCE_L0

	PMPML_FULL_REDUCE_MOD_2_32_PLUS_15_AND_RETURN
  }

  FORCE_INLINE
  uint64_t hash_of_string_chunk_compact_un_3( const uint32_t* coeff, ULARGE_INTEGER__XX constTerm, const unsigned char* _x ) const
  {
	PMPML_CHUNK_LOOP_INTRO_L0

	const uint32_t* xx = (const uint32_t*)( _x + 1 );
	uint32_t x = xx[ -1 ] >> 24;

	for ( uint32_t i=0; i<PMPML_CHUNK_SIZE; i+=8 )
	{
		PMPML_CHUNK_LOOP_BODY_ULI_T1_UNL( 16, 0 + i )
		PMPML_CHUNK_LOOP_BODY_ULI_T1_UNL( 16, 1 + i )
		PMPML_CHUNK_LOOP_BODY_ULI_T1_UNL( 16, 2 + i )
		PMPML_CHUNK_LOOP_BODY_ULI_T1_UNL( 16, 3 + i )
#if ( PMPML_CHUNK_SIZE > 4 )
		PMPML_CHUNK_LOOP_BODY_ULI_T1_UNL( 16, 4 + i )
		PMPML_CHUNK_LOOP_BODY_ULI_T1_UNL( 16, 5 + i )
		PMPML_CHUNK_LOOP_BODY_ULI_T1_UNL( 16, 6 + i )
		PMPML_CHUNK_LOOP_BODY_ULI_T1_UNL( 16, 7 + i )
#endif
	}

	PMPML_CHUNK_LOOP_PRE_REDUCE_L0

	PMPML_FULL_REDUCE_MOD_2_32_PLUS_15_AND_RETURN
  }

#endif // PMPML_STRICT_UNALIGNED_HANDLING


  FORCE_INLINE 
  uint64_t hash_of_beginning_of_string_chunk_type2( const uint32_t* coeff, ULARGE_INTEGER__XX constTerm, const unsigned char* tail, unsigned int tail_size  ) const
  {
	PMPML_CHUNK_LOOP_INTRO_L0
	uint32_t size = tail_size >> PMPML_WORD_SIZE_BYTES_LOG2;
	const uint32_t* x = (const uint32_t*)tail;

#ifdef PMPML_USE_SSE
	__m128i ctr0, ctr1, a, data, product, temp, mask_low;
	int i;

	ctr0 = _mm_setzero_si128 (); // Sets the 128-bit value to zero.
	ctr1 = _mm_setzero_si128 ();
	mask_low = _mm_set_epi32 ( 0, -1, 0 , -1 );

	for ( i=0; i<(size&0xFFFFFFF8); i+=4 )
	{
		a = _mm_load_si128 ((__m128i *)(coeff+i)); // Loads 128-bit value. Address p must be 16-byte aligned.
		data = _mm_loadu_si128 ((__m128i *)(x+i)); // Loads 128-bit value. Address p does not need be 16-byte aligned.
		product = _mm_mul_epu32 ( data, a); // A 128-bit value that contains two 64-bit unsigned integers. The result can be expressed by the following equations. r0 := a0 * b0; r1 := a2 * b2
		temp = _mm_srli_epi64( product, 32 ); // Shifts the 2 signed or unsigned 64-bit integers in a right by count bits while shifting in zeros.
		ctr1 = _mm_add_epi64 ( ctr1, temp );
		temp = _mm_and_si128 ( mask_low, product );
		ctr0 = _mm_add_epi64 ( ctr0, temp );

//		a = _mm_srli_epi64 ( a, 32 );
//		data = _mm_srli_epi64 ( data, 32 );
		a = _mm_shuffle_epi32( a, 1*1+0*4+3*16+2*64 );
		data = _mm_shuffle_epi32( data, 1*1+0*4+3*16+2*64 );
		product = _mm_mul_epu32 ( data, a); // A 128-bit value that contains two 64-bit unsigned integers. The result can be expressed by the following equations. r0 := a0 * b0; r1 := a2 * b2
		temp = _mm_srli_epi64( product, 32 ); // Shifts the 2 signed or unsigned 64-bit integers in a right by count bits while shifting in zeros.
		ctr1 = _mm_add_epi64 ( ctr1, temp );
		temp = _mm_and_si128 ( mask_low, product );
		ctr0 = _mm_add_epi64 ( ctr0, temp );
	}

	temp = _mm_unpackhi_epi64 ( ctr0, ctr1 ); // Interleaves the upper signed or unsigned 64-bit integer in a with the upper signed or unsigned 64-bit integer in b. r0 := a1 ; r1 := b1
	data = _mm_unpacklo_epi64 ( ctr0, ctr1 ); // Interleaves the lower signed or unsigned 64-bit integer in a with the lower signed or unsigned 64-bit integer in b. r0 := a0 ; r1 := b0
	ctr1 = _mm_add_epi64 ( data, temp );

#if (PMPML_CHUNK_OPTIMIZATION_TYPE== 1)
#if defined(_MSC_VER)
        constTerm.QuadPart += ctr1.m128i_u32[0]; // Microsoft specific
        ctr.QuadPart += ctr1.m128i_u64[1] + ctr1.m128i_u32[1];
#elif defined( __SSE4_1__)
        constTer.QuadPart += _mm_extract_epi32(ctr1,0);
        ctr.QuadPart += _mm_extract_epi64(ctr1,0) + _mm_extract_epi32(ctr1,1);
#else
        uint32_t b[4];
        _mm_storeu_si128((__m128i *)b,ctr1);
        constTerm.QuadPart += b[0];
        ctr.QuadPart += b[1] + b[2] + ((uint64_t) b[3] <<32);
#endif
#elif (PMPML_CHUNK_OPTIMIZATION_TYPE== 2)
	constTerm.QuadPart += *(uint64_t*)(&ctr1);
	ctr += constTerm.QuadPart < *(uint64_t*)(&ctr1);
	constTerm.HighPart += ((uint32_t*)(&ctr1))[2];
	ctr += constTerm.HighPart < ((uint32_t*)(&ctr1))[2];
	ctr +=  ((uint32_t*)(&ctr1))[3];
#else
#error unxpected PMPML_CHUNK_OPTIMIZATION_TYPE
#endif

#else // PMPML_USE_SSE

	for ( uint32_t i=0; i<(size&0xFFFFFFF8); i+=8 )
	{
		PMPML_CHUNK_LOOP_BODY_ULI_T1( 0 + i )
		PMPML_CHUNK_LOOP_BODY_ULI_T1( 1 + i )
		PMPML_CHUNK_LOOP_BODY_ULI_T1( 2 + i )
		PMPML_CHUNK_LOOP_BODY_ULI_T1( 3 + i )
#if ( PMPML_CHUNK_SIZE > 4 )
		PMPML_CHUNK_LOOP_BODY_ULI_T1( 4 + i )
		PMPML_CHUNK_LOOP_BODY_ULI_T1( 5 + i )
		PMPML_CHUNK_LOOP_BODY_ULI_T1( 6 + i )
		PMPML_CHUNK_LOOP_BODY_ULI_T1( 7 + i )
#endif
	}

#endif // PMPML_USE_SSE

	uint32_t offset = size & 0xFFFFFFF8;

	switch( size & 0x7 )
	{
		case 0: { break; }
		case 1:	{	PMPML_CHUNK_LOOP_BODY_ULI_T1( 0 + offset ) } break;
		case 2:	{	PMPML_CHUNK_LOOP_BODY_ULI_T1( 0 + offset ) PMPML_CHUNK_LOOP_BODY_ULI_T1( 1 + offset ) } break;
		case 3:	{	PMPML_CHUNK_LOOP_BODY_ULI_T1( 0 + offset ) PMPML_CHUNK_LOOP_BODY_ULI_T1( 1 + offset ) PMPML_CHUNK_LOOP_BODY_ULI_T1( 2 + offset ) } break;
		case 4:	{	PMPML_CHUNK_LOOP_BODY_ULI_T1( 0 + offset ) PMPML_CHUNK_LOOP_BODY_ULI_T1( 1 + offset ) PMPML_CHUNK_LOOP_BODY_ULI_T1( 2 + offset ) PMPML_CHUNK_LOOP_BODY_ULI_T1( 3 + offset ) } break;
		case 5:	{	PMPML_CHUNK_LOOP_BODY_ULI_T1( 0 + offset ) PMPML_CHUNK_LOOP_BODY_ULI_T1( 1 + offset ) PMPML_CHUNK_LOOP_BODY_ULI_T1( 2 + offset ) PMPML_CHUNK_LOOP_BODY_ULI_T1( 3 + offset ) PMPML_CHUNK_LOOP_BODY_ULI_T1( 4 + offset ) } break;
		case 6:	{	PMPML_CHUNK_LOOP_BODY_ULI_T1( 0 + offset ) PMPML_CHUNK_LOOP_BODY_ULI_T1( 1 + offset ) PMPML_CHUNK_LOOP_BODY_ULI_T1( 2 + offset ) PMPML_CHUNK_LOOP_BODY_ULI_T1( 3 + offset ) PMPML_CHUNK_LOOP_BODY_ULI_T1( 4 + offset ) PMPML_CHUNK_LOOP_BODY_ULI_T1( 5 + offset ) } break;
		case 7:	{	PMPML_CHUNK_LOOP_BODY_ULI_T1( 0 + offset ) PMPML_CHUNK_LOOP_BODY_ULI_T1( 1 + offset ) PMPML_CHUNK_LOOP_BODY_ULI_T1( 2 + offset ) PMPML_CHUNK_LOOP_BODY_ULI_T1( 3 + offset ) PMPML_CHUNK_LOOP_BODY_ULI_T1( 4 + offset ) PMPML_CHUNK_LOOP_BODY_ULI_T1( 5 + offset ) PMPML_CHUNK_LOOP_BODY_ULI_T1( 6 + offset ) } break;
	}

	uint32_t xLast;
	switch ( tail_size & ( PMPML_WORD_SIZE_BYTES - 1 ) )
	{
		case 0: { xLast = 0x1; break;}
		case 1: { xLast = 0x100 | tail[tail_size-1]; break;}
		case 2: { xLast = *((const unsigned short*)(tail + tail_size - 2 )) | 0x10000; break; }
		case 3: { xLast = tail[ tail_size - 1 ]; xLast = ( xLast << 16 ) | *((const unsigned short*)(tail + tail_size - 3 )) | 0x1000000; break;}
	}

	PMPML_CHUNK_LOOP_BODY_ULI_T1_LAST

	PMPML_CHUNK_LOOP_PRE_REDUCE_L0

	PMPML_FULL_REDUCE_MOD_2_32_PLUS_15_AND_RETURN
  }

#ifdef PMPML_STRICT_UNALIGNED_HANDLING

  FORCE_INLINE 
  uint64_t hash_of_beginning_of_string_chunk_type2_un1( const uint32_t* coeff, ULARGE_INTEGER__XX constTerm, const unsigned char* tail, unsigned int tail_size  ) const
  {
	PMPML_CHUNK_LOOP_INTRO_L0
	uint32_t size = tail_size >> PMPML_WORD_SIZE_BYTES_LOG2;
	const uint32_t* xx = (const uint32_t*)( tail + 3 );
	uint32_t x = xx[ -1 ] >> 8;

	for ( uint32_t i=0; i<(size&0xFFFFFFF8); i+=8 )
	{
		PMPML_CHUNK_LOOP_BODY_ULI_T1_UNL( 8, 0 + i )
		PMPML_CHUNK_LOOP_BODY_ULI_T1_UNL( 8, 1 + i )
		PMPML_CHUNK_LOOP_BODY_ULI_T1_UNL( 8, 2 + i )
		PMPML_CHUNK_LOOP_BODY_ULI_T1_UNL( 8, 3 + i )
#if ( PMPML_CHUNK_SIZE > 4 )
		PMPML_CHUNK_LOOP_BODY_ULI_T1_UNL( 8, 4 + i )
		PMPML_CHUNK_LOOP_BODY_ULI_T1_UNL( 8, 5 + i )
		PMPML_CHUNK_LOOP_BODY_ULI_T1_UNL( 8, 6 + i )
		PMPML_CHUNK_LOOP_BODY_ULI_T1_UNL( 8, 7 + i )
#endif
	}

	uint32_t offset = size & 0xFFFFFFF8;

	switch( size & 0x7 )
	{
		case 0: { break; }
		case 1:	{	PMPML_CHUNK_LOOP_BODY_ULI_T1_UNL( 8, 0 + offset ) } break;
		case 2:	{	PMPML_CHUNK_LOOP_BODY_ULI_T1_UNL( 8, 0 + offset ) PMPML_CHUNK_LOOP_BODY_ULI_T1_UNL( 8, 1 + offset ) } break;
		case 3:	{	PMPML_CHUNK_LOOP_BODY_ULI_T1_UNL( 8, 0 + offset ) PMPML_CHUNK_LOOP_BODY_ULI_T1_UNL( 8, 1 + offset ) PMPML_CHUNK_LOOP_BODY_ULI_T1_UNL( 8, 2 + offset ) } break;
		case 4:	{	PMPML_CHUNK_LOOP_BODY_ULI_T1_UNL( 8, 0 + offset ) PMPML_CHUNK_LOOP_BODY_ULI_T1_UNL( 8, 1 + offset ) PMPML_CHUNK_LOOP_BODY_ULI_T1_UNL( 8, 2 + offset ) PMPML_CHUNK_LOOP_BODY_ULI_T1_UNL( 8, 3 + offset ) } break;
		case 5:	{	PMPML_CHUNK_LOOP_BODY_ULI_T1_UNL( 8, 0 + offset ) PMPML_CHUNK_LOOP_BODY_ULI_T1_UNL( 8, 1 + offset ) PMPML_CHUNK_LOOP_BODY_ULI_T1_UNL( 8, 2 + offset ) PMPML_CHUNK_LOOP_BODY_ULI_T1_UNL( 8, 3 + offset ) PMPML_CHUNK_LOOP_BODY_ULI_T1_UNL( 8, 4 + offset ) } break;
		case 6:	{	PMPML_CHUNK_LOOP_BODY_ULI_T1_UNL( 8, 0 + offset ) PMPML_CHUNK_LOOP_BODY_ULI_T1_UNL( 8, 1 + offset ) PMPML_CHUNK_LOOP_BODY_ULI_T1_UNL( 8, 2 + offset ) PMPML_CHUNK_LOOP_BODY_ULI_T1_UNL( 8, 3 + offset ) PMPML_CHUNK_LOOP_BODY_ULI_T1_UNL( 8, 4 + offset ) PMPML_CHUNK_LOOP_BODY_ULI_T1_UNL( 8, 5 + offset ) } break;
		case 7:	{	PMPML_CHUNK_LOOP_BODY_ULI_T1_UNL( 8, 0 + offset ) PMPML_CHUNK_LOOP_BODY_ULI_T1_UNL( 8, 1 + offset ) PMPML_CHUNK_LOOP_BODY_ULI_T1_UNL( 8, 2 + offset ) PMPML_CHUNK_LOOP_BODY_ULI_T1_UNL( 8, 3 + offset ) PMPML_CHUNK_LOOP_BODY_ULI_T1_UNL( 8, 4 + offset ) PMPML_CHUNK_LOOP_BODY_ULI_T1_UNL( 8, 5 + offset ) PMPML_CHUNK_LOOP_BODY_ULI_T1_UNL( 8, 6 + offset ) } break;
	}

	uint32_t xLast;
	switch ( tail_size & ( PMPML_WORD_SIZE_BYTES - 1 ) )
	{
		case 0: { xLast = 0x1; break;}
		case 1: { xLast = 0x100 | tail[tail_size-1]; break;}
		case 2: { xLast = 0x100 | tail[tail_size-1]; xLast = ( xLast << 8 ) | tail[tail_size - 2]; break; }
		case 3: { xLast = 0x100 | tail[tail_size-1]; xLast = ( xLast << 8 ) | tail[tail_size - 2]; xLast = ( xLast << 8 ) | tail[tail_size - 3]; break;}
	}

	PMPML_CHUNK_LOOP_BODY_ULI_T1_LAST

	PMPML_CHUNK_LOOP_PRE_REDUCE_L0

	PMPML_FULL_REDUCE_MOD_2_32_PLUS_15_AND_RETURN
  }

  FORCE_INLINE 
  uint64_t hash_of_beginning_of_string_chunk_type2_un2( const uint32_t* coeff, ULARGE_INTEGER__XX constTerm, const unsigned char* tail, unsigned int tail_size  ) const
  {
	PMPML_CHUNK_LOOP_INTRO_L0
	uint32_t size = tail_size >> PMPML_WORD_SIZE_BYTES_LOG2;
	const uint32_t* xx = (const uint32_t*)( tail + 2 );
	uint32_t x = xx[ -1 ] >> 16;

	for ( uint32_t i=0; i<(size&0xFFFFFFF8); i+=8 )
	{
		PMPML_CHUNK_LOOP_BODY_ULI_T1_UNL( 16, 0 + i )
		PMPML_CHUNK_LOOP_BODY_ULI_T1_UNL( 16, 1 + i )
		PMPML_CHUNK_LOOP_BODY_ULI_T1_UNL( 16, 2 + i )
		PMPML_CHUNK_LOOP_BODY_ULI_T1_UNL( 16, 3 + i )
#if ( PMPML_CHUNK_SIZE > 4 )
		PMPML_CHUNK_LOOP_BODY_ULI_T1_UNL( 16, 4 + i )
		PMPML_CHUNK_LOOP_BODY_ULI_T1_UNL( 16, 5 + i )
		PMPML_CHUNK_LOOP_BODY_ULI_T1_UNL( 16, 6 + i )
		PMPML_CHUNK_LOOP_BODY_ULI_T1_UNL( 16, 7 + i )
#endif
	}

	uint32_t offset = size & 0xFFFFFFF8;

	switch( size & 0x7 )
	{
		case 0: { break; }
		case 1:	{	PMPML_CHUNK_LOOP_BODY_ULI_T1_UNL( 16, 0 + offset ) } break;
		case 2:	{	PMPML_CHUNK_LOOP_BODY_ULI_T1_UNL( 16, 0 + offset ) PMPML_CHUNK_LOOP_BODY_ULI_T1_UNL( 16, 1 + offset ) } break;
		case 3:	{	PMPML_CHUNK_LOOP_BODY_ULI_T1_UNL( 16, 0 + offset ) PMPML_CHUNK_LOOP_BODY_ULI_T1_UNL( 16, 1 + offset ) PMPML_CHUNK_LOOP_BODY_ULI_T1_UNL( 16, 2 + offset ) } break;
		case 4:	{	PMPML_CHUNK_LOOP_BODY_ULI_T1_UNL( 16, 0 + offset ) PMPML_CHUNK_LOOP_BODY_ULI_T1_UNL( 16, 1 + offset ) PMPML_CHUNK_LOOP_BODY_ULI_T1_UNL( 16, 2 + offset ) PMPML_CHUNK_LOOP_BODY_ULI_T1_UNL( 16, 3 + offset ) } break;
		case 5:	{	PMPML_CHUNK_LOOP_BODY_ULI_T1_UNL( 16, 0 + offset ) PMPML_CHUNK_LOOP_BODY_ULI_T1_UNL( 16, 1 + offset ) PMPML_CHUNK_LOOP_BODY_ULI_T1_UNL( 16, 2 + offset ) PMPML_CHUNK_LOOP_BODY_ULI_T1_UNL( 16, 3 + offset ) PMPML_CHUNK_LOOP_BODY_ULI_T1_UNL( 16, 4 + offset ) } break;
		case 6:	{	PMPML_CHUNK_LOOP_BODY_ULI_T1_UNL( 16, 0 + offset ) PMPML_CHUNK_LOOP_BODY_ULI_T1_UNL( 16, 1 + offset ) PMPML_CHUNK_LOOP_BODY_ULI_T1_UNL( 16, 2 + offset ) PMPML_CHUNK_LOOP_BODY_ULI_T1_UNL( 16, 3 + offset ) PMPML_CHUNK_LOOP_BODY_ULI_T1_UNL( 16, 4 + offset ) PMPML_CHUNK_LOOP_BODY_ULI_T1_UNL( 16, 5 + offset ) } break;
		case 7:	{	PMPML_CHUNK_LOOP_BODY_ULI_T1_UNL( 16, 0 + offset ) PMPML_CHUNK_LOOP_BODY_ULI_T1_UNL( 16, 1 + offset ) PMPML_CHUNK_LOOP_BODY_ULI_T1_UNL( 16, 2 + offset ) PMPML_CHUNK_LOOP_BODY_ULI_T1_UNL( 16, 3 + offset ) PMPML_CHUNK_LOOP_BODY_ULI_T1_UNL( 16, 4 + offset ) PMPML_CHUNK_LOOP_BODY_ULI_T1_UNL( 16, 5 + offset ) PMPML_CHUNK_LOOP_BODY_ULI_T1_UNL( 16, 6 + offset ) } break;
	}

	uint32_t xLast;
	switch ( tail_size & ( PMPML_WORD_SIZE_BYTES - 1 ) )
	{
		case 0: { xLast = 0x1; break;}
		case 1: { xLast = 0x100 | tail[tail_size-1]; break;}
		case 2: { xLast = 0x100 | tail[tail_size-1]; xLast = ( xLast << 8 ) | tail[tail_size - 2]; break; }
		case 3: { xLast = 0x100 | tail[tail_size-1]; xLast = ( xLast << 8 ) | tail[tail_size - 2]; xLast = ( xLast << 8 ) | tail[tail_size - 3]; break;}
	}

	PMPML_CHUNK_LOOP_BODY_ULI_T1_LAST

	PMPML_CHUNK_LOOP_PRE_REDUCE_L0

	PMPML_FULL_REDUCE_MOD_2_32_PLUS_15_AND_RETURN
  }

  FORCE_INLINE 
  uint64_t hash_of_beginning_of_string_chunk_type2_un3( const uint32_t* coeff, ULARGE_INTEGER__XX constTerm, const unsigned char* tail, unsigned int tail_size  ) const
  {
	PMPML_CHUNK_LOOP_INTRO_L0
	uint32_t size = tail_size >> PMPML_WORD_SIZE_BYTES_LOG2;
	const uint32_t* xx = (const uint32_t*)( tail + 1 );
	uint32_t x = xx[ -1 ] >> 24;

	for ( uint32_t i=0; i<(size&0xFFFFFFF8); i+=8 )
	{
		PMPML_CHUNK_LOOP_BODY_ULI_T1_UNL( 24, 0 + i )
		PMPML_CHUNK_LOOP_BODY_ULI_T1_UNL( 24, 1 + i )
		PMPML_CHUNK_LOOP_BODY_ULI_T1_UNL( 24, 2 + i )
		PMPML_CHUNK_LOOP_BODY_ULI_T1_UNL( 24, 3 + i )
#if ( PMPML_CHUNK_SIZE > 4 )
		PMPML_CHUNK_LOOP_BODY_ULI_T1_UNL( 24, 4 + i )
		PMPML_CHUNK_LOOP_BODY_ULI_T1_UNL( 24, 5 + i )
		PMPML_CHUNK_LOOP_BODY_ULI_T1_UNL( 24, 6 + i )
		PMPML_CHUNK_LOOP_BODY_ULI_T1_UNL( 24, 7 + i )
#endif
	}

	uint32_t offset = size & 0xFFFFFFF8;

	switch( size & 0x7 )
	{
		case 0: { break; }
		case 1:	{	PMPML_CHUNK_LOOP_BODY_ULI_T1_UNL( 24, 0 + offset ) } break;
		case 2:	{	PMPML_CHUNK_LOOP_BODY_ULI_T1_UNL( 24, 0 + offset ) PMPML_CHUNK_LOOP_BODY_ULI_T1_UNL( 24, 1 + offset ) } break;
		case 3:	{	PMPML_CHUNK_LOOP_BODY_ULI_T1_UNL( 24, 0 + offset ) PMPML_CHUNK_LOOP_BODY_ULI_T1_UNL( 24, 1 + offset ) PMPML_CHUNK_LOOP_BODY_ULI_T1_UNL( 24, 2 + offset ) } break;
		case 4:	{	PMPML_CHUNK_LOOP_BODY_ULI_T1_UNL( 24, 0 + offset ) PMPML_CHUNK_LOOP_BODY_ULI_T1_UNL( 24, 1 + offset ) PMPML_CHUNK_LOOP_BODY_ULI_T1_UNL( 24, 2 + offset ) PMPML_CHUNK_LOOP_BODY_ULI_T1_UNL( 24, 3 + offset ) } break;
		case 5:	{	PMPML_CHUNK_LOOP_BODY_ULI_T1_UNL( 24, 0 + offset ) PMPML_CHUNK_LOOP_BODY_ULI_T1_UNL( 24, 1 + offset ) PMPML_CHUNK_LOOP_BODY_ULI_T1_UNL( 24, 2 + offset ) PMPML_CHUNK_LOOP_BODY_ULI_T1_UNL( 24, 3 + offset ) PMPML_CHUNK_LOOP_BODY_ULI_T1_UNL( 24, 4 + offset ) } break;
		case 6:	{	PMPML_CHUNK_LOOP_BODY_ULI_T1_UNL( 24, 0 + offset ) PMPML_CHUNK_LOOP_BODY_ULI_T1_UNL( 24, 1 + offset ) PMPML_CHUNK_LOOP_BODY_ULI_T1_UNL( 24, 2 + offset ) PMPML_CHUNK_LOOP_BODY_ULI_T1_UNL( 24, 3 + offset ) PMPML_CHUNK_LOOP_BODY_ULI_T1_UNL( 24, 4 + offset ) PMPML_CHUNK_LOOP_BODY_ULI_T1_UNL( 24, 5 + offset ) } break;
		case 7:	{	PMPML_CHUNK_LOOP_BODY_ULI_T1_UNL( 24, 0 + offset ) PMPML_CHUNK_LOOP_BODY_ULI_T1_UNL( 24, 1 + offset ) PMPML_CHUNK_LOOP_BODY_ULI_T1_UNL( 24, 2 + offset ) PMPML_CHUNK_LOOP_BODY_ULI_T1_UNL( 24, 3 + offset ) PMPML_CHUNK_LOOP_BODY_ULI_T1_UNL( 24, 4 + offset ) PMPML_CHUNK_LOOP_BODY_ULI_T1_UNL( 24, 5 + offset ) PMPML_CHUNK_LOOP_BODY_ULI_T1_UNL( 24, 6 + offset ) } break;
	}

	uint32_t xLast;
	switch ( tail_size & ( PMPML_WORD_SIZE_BYTES - 1 ) )
	{
		case 0: { xLast = 0x1; break;}
		case 1: { xLast = 0x100 | tail[tail_size-1]; break;}
		case 2: { xLast = 0x100 | tail[tail_size-1]; xLast = ( xLast << 8 ) | tail[tail_size - 2]; break; }
		case 3: { xLast = 0x100 | tail[tail_size-1]; xLast = ( xLast << 8 ) | tail[tail_size - 2]; xLast = ( xLast << 8 ) | tail[tail_size - 3]; break;}
	}

	PMPML_CHUNK_LOOP_BODY_ULI_T1_LAST

	PMPML_CHUNK_LOOP_PRE_REDUCE_L0

	PMPML_FULL_REDUCE_MOD_2_32_PLUS_15_AND_RETURN
  }

#endif // PMPML_STRICT_UNALIGNED_HANDLING


  // a call to be done from subsequent levels
  FORCE_INLINE 
  uint64_t hash_of_num_chunk( const uint32_t* coeff, ULARGE_INTEGER__XX constTerm, const uint64_t* x ) const
  {
	PMPML_CHUNK_LOOP_INTRO_L0

	for ( uint32_t i=0; i<PMPML_CHUNK_SIZE; i+=8 )
	{
		PMPML_CHUNK_LOOP_BODY_ULI_T1( 0 + i )
		PMPML_CHUNK_LOOP_BODY_ULI_T1( 1 + i )
		PMPML_CHUNK_LOOP_BODY_ULI_T1( 2 + i )
		PMPML_CHUNK_LOOP_BODY_ULI_T1( 3 + i )
#if ( PMPML_CHUNK_SIZE > 4 )
		PMPML_CHUNK_LOOP_BODY_ULI_T1( 4 + i )
		PMPML_CHUNK_LOOP_BODY_ULI_T1( 5 + i )
		PMPML_CHUNK_LOOP_BODY_ULI_T1( 6 + i )
		PMPML_CHUNK_LOOP_BODY_ULI_T1( 7 + i )
#endif
	}

	PMPML_CHUNK_LOOP_PRE_REDUCE_L0

	PMPML_FULL_REDUCE_MOD_2_32_PLUS_15_AND_RETURN
  }

  // a call to be done from subsequent levels
  FORCE_INLINE 
  uint64_t hash_of_num_chunk_incomplete( const uint32_t* coeff, ULARGE_INTEGER__XX constTerm, ULARGE_INTEGER__XX prevConstTerm, ULARGE_INTEGER__XX coeffSum, const uint64_t* x, size_t count ) const
  {
	PMPML_CHUNK_LOOP_INTRO_L0

	ULARGE_INTEGER__XX c_ctr;
	c_ctr.QuadPart = 0;

	uint32_t i;

	if ( count < ( PMPML_CHUNK_SIZE >> 1 ) )
	{
		for ( i=0; i<count; i++ )
		{
			PMPML_CHUNK_LOOP_BODY_ULI_T1( 0 + i )
			c_ctr.QuadPart += coeff[ i ];
		}
		c_ctr.QuadPart = coeffSum.QuadPart - c_ctr.QuadPart;
	}
	else
	{
		for ( i=0; i<count; i++ )
			PMPML_CHUNK_LOOP_BODY_ULI_T1( 0 + i )
		for ( ; i<PMPML_CHUNK_SIZE; i++ )
			c_ctr.QuadPart += coeff[ i ];
	}

	ULARGE_INTEGER__XX lowProduct;
	lowProduct.QuadPart = UInt32x32To64( c_ctr.LowPart, prevConstTerm.LowPart );
	ULARGE_INTEGER__XX midProduct;
	midProduct.QuadPart = UInt32x32To64( c_ctr.LowPart, prevConstTerm.HighPart ) + UInt32x32To64( c_ctr.HighPart, prevConstTerm.LowPart );
	midProduct.QuadPart += lowProduct.HighPart;
	lowProduct.HighPart = midProduct.LowPart;
	uint32_t hiProduct = c_ctr.HighPart * prevConstTerm.HighPart + midProduct.HighPart;

	constTerm.QuadPart += lowProduct.QuadPart;
	ctr += hiProduct + ( constTerm.QuadPart < lowProduct.QuadPart );

/*	for ( uint32_t i=0; i<PMPML_CHUNK_SIZE; i+=8 )
	{
		PMPML_CHUNK_LOOP_BODY_ULI_T1( 0 + i )
		PMPML_CHUNK_LOOP_BODY_ULI_T1( 1 + i )
		PMPML_CHUNK_LOOP_BODY_ULI_T1( 2 + i )
		PMPML_CHUNK_LOOP_BODY_ULI_T1( 3 + i )
#if ( PMPML_CHUNK_SIZE > 4 )
		PMPML_CHUNK_LOOP_BODY_ULI_T1( 4 + i )
		PMPML_CHUNK_LOOP_BODY_ULI_T1( 5 + i )
		PMPML_CHUNK_LOOP_BODY_ULI_T1( 6 + i )
		PMPML_CHUNK_LOOP_BODY_ULI_T1( 7 + i )
#endif
	}*/

	PMPML_CHUNK_LOOP_PRE_REDUCE_L0

	PMPML_FULL_REDUCE_MOD_2_32_PLUS_15_AND_RETURN
  }

  FORCE_INLINE 
  void procesNextValue( int level, uint64_t value, uint64_t * allValues, unsigned int * cnts, unsigned int& flag ) const
  {
	for ( int i=level;;i++ )
	{
		// NOTE: it's not necessary to check whether ( i < PMPML_LEVELS ), 
		// if it is guaranteed that the string size is less than 1 << USHF_MACHINE_WORD_SIZE_BITS
		allValues[ ( i << PMPML_CHUNK_SIZE_LOG2 ) + cnts[ i ] ] = value;
		(cnts[ i ]) ++;
		if ( cnts[ i ] != PMPML_CHUNK_SIZE )
			break;
		cnts[ i ] = 0;
		value = hash_of_num_chunk( curr_rd[ i ].random_coeff, *(ULARGE_INTEGER__XX*)(&(curr_rd[i].const_term)), allValues + ( i << PMPML_CHUNK_SIZE_LOG2 ) );
		if ( ( flag & ( 1 << i ) ) == 0 )
		{
			cnts[ i + 1] = 0;
			flag |= 1 << i;
		}
	}
  }

  FORCE_INLINE 
  uint64_t finalize( int level, uint64_t * allValues, unsigned int * cnts, unsigned int& flag ) const
  {
	for ( int i=level;;i++ )
	{
//		assert ( level != PMPML_LEVELS )
		if ( ( ( flag & ( 1 << i ) ) == 0 ) && cnts[ i ] == 1 )
		{
			return allValues[ i << PMPML_CHUNK_SIZE_LOG2 ];
		}
		if ( cnts[ i ] )
		{
/*			for ( int j=cnts[ i ]; j<PMPML_CHUNK_SIZE; j++ )
				( allValues + ( i << PMPML_CHUNK_SIZE_LOG2 ) )[ j ] = curr_rd[ i - 1 ].const_term;*/
			if ( ( flag & ( 1 << i ) ) == 0 )
			{
				cnts[ i + 1] = 0;
				flag |= 1 << i;
			}
			procesNextValue( i + 1, 
/*							 hash_of_num_chunk( curr_rd[ i ].random_coeff,
												*(ULARGE_INTEGER__XX*)(&(curr_rd[i].const_term)),
												allValues + ( i << PMPML_CHUNK_SIZE_LOG2 ) ), */
							 hash_of_num_chunk_incomplete( curr_rd[ i ].random_coeff,
												*(ULARGE_INTEGER__XX*)(&(curr_rd[i].const_term)),
												*(ULARGE_INTEGER__XX*)(&(curr_rd[i-1].const_term)),
												*(ULARGE_INTEGER__XX*)(&(curr_rd[i].cachedSum)),
												allValues + ( i << PMPML_CHUNK_SIZE_LOG2 ),
												cnts[ i ]), 
							 allValues, cnts, flag );
		}
	}
  }

#ifdef PMPML_MSC_32_WORKAROUND	// enables MSVC-specific code that appears to be more efficient than a regular one; comment out, if not desired
  FORCE_INLINE uint32_t hash_size_0( const unsigned char* chars ) const
  {
		const uint32_t* coeff = curr_rd[0].random_coeff;
		ULARGE_INTEGER__XX constTerm = *(ULARGE_INTEGER__XX*)(&(curr_rd[0].const_term));
		constTerm.QuadPart += coeff[ 0 ];	PMPML_FULL_REDUCE_MOD_2_32_PLUS_15_AND_RETURN_RETURN_32x32_ONLY;
  }

  FORCE_INLINE uint32_t hash_size_1( const unsigned char* chars ) const
  {
		const uint32_t* coeff = curr_rd[0].random_coeff;
		ULARGE_INTEGER__XX constTerm = *(ULARGE_INTEGER__XX*)(&(curr_rd[0].const_term));
		uint32_t xLast;

		xLast = 0x100 + chars[0];	constTerm.QuadPart += UInt32x32To64( coeff[ 0 ], xLast );	PMPML_FULL_REDUCE_MOD_2_32_PLUS_15_AND_RETURN_RETURN_32x32_ONLY;
  }

  FORCE_INLINE uint32_t hash_size_2( const unsigned char* chars ) const
  {
		const uint32_t* coeff = curr_rd[0].random_coeff;
		ULARGE_INTEGER__XX constTerm = *(ULARGE_INTEGER__XX*)(&(curr_rd[0].const_term));
		uint32_t xLast;

		xLast = *((const unsigned short*)(chars )) + 0x10000;	constTerm.QuadPart += UInt32x32To64( coeff[ 0 ], xLast );	PMPML_FULL_REDUCE_MOD_2_32_PLUS_15_AND_RETURN_RETURN_32x32_ONLY;
  }

  FORCE_INLINE uint32_t hash_size_3( const unsigned char* chars ) const
  {
		const uint32_t* coeff = curr_rd[0].random_coeff;
		ULARGE_INTEGER__XX constTerm = *(ULARGE_INTEGER__XX*)(&(curr_rd[0].const_term));
		uint32_t xLast;

		xLast = chars[ 2 ]; xLast = ( xLast << 16 ) + *((const unsigned short*)(chars )) + 0x1000000;	constTerm.QuadPart += UInt32x32To64( coeff[ 0 ], xLast );	PMPML_FULL_REDUCE_MOD_2_32_PLUS_15_AND_RETURN_RETURN_32x32_ONLY;
  }

  FORCE_INLINE uint32_t hash_size_4( const unsigned char* chars ) const
  {
		const uint32_t* coeff = curr_rd[0].random_coeff;
		ULARGE_INTEGER__XX constTerm = *(ULARGE_INTEGER__XX*)(&(curr_rd[0].const_term));
		uint32_t xLast;

		constTerm.QuadPart += UInt32x32To64( coeff[ 0 ], *(uint32_t*)(chars) ) + coeff[ 1 ];	PMPML_FULL_REDUCE_MOD_2_32_PLUS_15_AND_RETURN_RETURN_32x32_ONLY;
  }

#define HASH_SIZE_XXX_BEGIN( XXX ) \
	FORCE_INLINE uint32_t hash_size_##XXX( const unsigned char* chars ) const \
  { \
		const uint32_t* coeff = curr_rd[0].random_coeff; \
		const uint32_t* x = (const uint32_t*)chars; \
		ULARGE_INTEGER__XX constTerm = *(ULARGE_INTEGER__XX*)(&(curr_rd[0].const_term)); \
		uint32_t xLast; \
		PMPML_CHUNK_LOOP_INTRO_L0 \
  		uint32_t size = XXX >> PMPML_WORD_SIZE_BYTES_LOG2;

#define HASH_SIZE_XXX_END \
		PMPML_CHUNK_LOOP_PRE_REDUCE_L0 \
		PMPML_FULL_REDUCE_MOD_2_32_PLUS_15_AND_RETURN_RETURN \
}

HASH_SIZE_XXX_BEGIN(28 )
		PMPML_CHUNK_LOOP_BODY_ULI_T1( 0 ) PMPML_CHUNK_LOOP_BODY_ULI_T1( 1 ) PMPML_CHUNK_LOOP_BODY_ULI_T1( 2 ) PMPML_CHUNK_LOOP_BODY_ULI_T1( 3 ) PMPML_CHUNK_LOOP_BODY_ULI_T1( 4 ) PMPML_CHUNK_LOOP_BODY_ULI_T1( 5 ) PMPML_CHUNK_LOOP_BODY_ULI_T1( 6 ) 	xLast = 0x1;	PMPML_CHUNK_LOOP_BODY_ULI_T1_LAST_FOR_JUST_1;		
HASH_SIZE_XXX_END 

HASH_SIZE_XXX_BEGIN(29 )
		PMPML_CHUNK_LOOP_BODY_ULI_T1( 0 ) PMPML_CHUNK_LOOP_BODY_ULI_T1( 1 ) PMPML_CHUNK_LOOP_BODY_ULI_T1( 2 ) PMPML_CHUNK_LOOP_BODY_ULI_T1( 3 ) PMPML_CHUNK_LOOP_BODY_ULI_T1( 4 ) PMPML_CHUNK_LOOP_BODY_ULI_T1( 5 ) PMPML_CHUNK_LOOP_BODY_ULI_T1( 6 ) 	xLast = 0x100 + chars[28];	PMPML_CHUNK_LOOP_BODY_ULI_T1_LAST;		
HASH_SIZE_XXX_END 

HASH_SIZE_XXX_BEGIN(30 )
		PMPML_CHUNK_LOOP_BODY_ULI_T1( 0 ) PMPML_CHUNK_LOOP_BODY_ULI_T1( 1 ) PMPML_CHUNK_LOOP_BODY_ULI_T1( 2 ) PMPML_CHUNK_LOOP_BODY_ULI_T1( 3 ) PMPML_CHUNK_LOOP_BODY_ULI_T1( 4 ) PMPML_CHUNK_LOOP_BODY_ULI_T1( 5 ) PMPML_CHUNK_LOOP_BODY_ULI_T1( 6 ) 	xLast = *((const unsigned short*)(chars + 28 )) + 0x10000;	PMPML_CHUNK_LOOP_BODY_ULI_T1_LAST		
HASH_SIZE_XXX_END 

HASH_SIZE_XXX_BEGIN(31 )
		PMPML_CHUNK_LOOP_BODY_ULI_T1( 0 ) PMPML_CHUNK_LOOP_BODY_ULI_T1( 1 ) PMPML_CHUNK_LOOP_BODY_ULI_T1( 2 ) PMPML_CHUNK_LOOP_BODY_ULI_T1( 3 ) PMPML_CHUNK_LOOP_BODY_ULI_T1( 4 ) PMPML_CHUNK_LOOP_BODY_ULI_T1( 5 ) PMPML_CHUNK_LOOP_BODY_ULI_T1( 6 ) 	xLast = chars[ 30 ]; xLast = ( xLast << 16 ) + *((const unsigned short*)(chars + 28 )) + 0x1000000;	PMPML_CHUNK_LOOP_BODY_ULI_T1_LAST;	
HASH_SIZE_XXX_END 

#endif // PMPML_MSC_32_WORKAROUND

public:
  FORCE_INLINE uint32_t hash( const unsigned char* chars, unsigned int cnt ) const
  {
	if ( _LIKELY_BRANCH_(cnt < 32) )
	{
		const uint32_t* coeff = curr_rd[0].random_coeff;
		ULARGE_INTEGER__XX constTerm = *(ULARGE_INTEGER__XX*)(&(curr_rd[0].const_term));
		PMPML_CHUNK_LOOP_INTRO_L0
		uint32_t size = cnt >> PMPML_WORD_SIZE_BYTES_LOG2;
		uint32_t xLast;

#if !defined PMPML_STRICT_UNALIGNED_HANDLING
		const uint32_t* x = (const uint32_t*)chars;

#if !defined PMPML_MSC_32_WORKAROUND
		switch( size )
		{
			case 0: { break; }
			case 1:	{	PMPML_CHUNK_LOOP_BODY_ULI_T1( 0 ) } break;
			case 2:	{	PMPML_CHUNK_LOOP_BODY_ULI_T1( 0 ) PMPML_CHUNK_LOOP_BODY_ULI_T1( 1 ) } break;
			case 3:	{	PMPML_CHUNK_LOOP_BODY_ULI_T1( 0 ) PMPML_CHUNK_LOOP_BODY_ULI_T1( 1 ) PMPML_CHUNK_LOOP_BODY_ULI_T1( 2 ) } break;
			case 4:	{	PMPML_CHUNK_LOOP_BODY_ULI_T1( 0 ) PMPML_CHUNK_LOOP_BODY_ULI_T1( 1 ) PMPML_CHUNK_LOOP_BODY_ULI_T1( 2 ) PMPML_CHUNK_LOOP_BODY_ULI_T1( 3 ) } break;
			case 5:	{	PMPML_CHUNK_LOOP_BODY_ULI_T1( 0 ) PMPML_CHUNK_LOOP_BODY_ULI_T1( 1 ) PMPML_CHUNK_LOOP_BODY_ULI_T1( 2 ) PMPML_CHUNK_LOOP_BODY_ULI_T1( 3 ) PMPML_CHUNK_LOOP_BODY_ULI_T1( 4 ) } break;
			case 6:	{	PMPML_CHUNK_LOOP_BODY_ULI_T1( 0 ) PMPML_CHUNK_LOOP_BODY_ULI_T1( 1 ) PMPML_CHUNK_LOOP_BODY_ULI_T1( 2 ) PMPML_CHUNK_LOOP_BODY_ULI_T1( 3 ) PMPML_CHUNK_LOOP_BODY_ULI_T1( 4 ) PMPML_CHUNK_LOOP_BODY_ULI_T1( 5 ) } break;
			default:	{	PMPML_CHUNK_LOOP_BODY_ULI_T1( 0 ) PMPML_CHUNK_LOOP_BODY_ULI_T1( 1 ) PMPML_CHUNK_LOOP_BODY_ULI_T1( 2 ) PMPML_CHUNK_LOOP_BODY_ULI_T1( 3 ) PMPML_CHUNK_LOOP_BODY_ULI_T1( 4 ) PMPML_CHUNK_LOOP_BODY_ULI_T1( 5 ) PMPML_CHUNK_LOOP_BODY_ULI_T1( 6 ) } break;
		}

		switch ( cnt & ( PMPML_WORD_SIZE_BYTES - 1 ) )
		{
			case 0: { xLast = 0x1; break;}
			case 1: { xLast = 0x100 + chars[cnt-1]; break;}
			case 2: { xLast = *((const unsigned short*)(chars + cnt - 2 )) + 0x10000; break; }
			default: { xLast = chars[ cnt - 1 ]; xLast = ( xLast << 16 ) + *((const unsigned short*)(chars + cnt - 3 )) + 0x1000000; break;}
		}
	
#else // PMPML_MSC_32_WORKAROUND
// enables MSVC-specific code that appears to be more efficient than a regular one; comment out, if not desired
		switch ( cnt )
		{
/*			case 0: { 			xLast = 0x1;	constTerm.QuadPart += coeff[ 0 ];	PMPML_FULL_REDUCE_MOD_2_32_PLUS_15_AND_RETURN_RETURN_32x32_ONLY;	}		
			case 1: { 			xLast = 0x100 + chars[cnt-1];	constTerm.QuadPart += UInt32x32To64( coeff[ 0 ], xLast );		PMPML_FULL_REDUCE_MOD_2_32_PLUS_15_AND_RETURN_RETURN_32x32_ONLY;	}		
			case 2: { 			xLast = *((const unsigned short*)(chars + cnt - 2 )) + 0x10000;	constTerm.QuadPart += UInt32x32To64( coeff[ 0 ], xLast );		PMPML_FULL_REDUCE_MOD_2_32_PLUS_15_AND_RETURN_RETURN_32x32_ONLY;	}		
			case 3: { 			xLast = chars[ cnt - 1 ]; xLast = ( xLast << 16 ) + *((const unsigned short*)(chars + cnt - 3 )) + 0x1000000;	constTerm.QuadPart += UInt32x32To64( coeff[ 0 ], xLast );		PMPML_FULL_REDUCE_MOD_2_32_PLUS_15_AND_RETURN_RETURN_32x32_ONLY;	}		
								
			case 0:	{	xLast = 0x1;	PMPML_CHUNK_LOOP_BODY_ULI_T1_LAST;	break;	}		
			case 1:	{	xLast = 0x100 + chars[cnt-1];	PMPML_CHUNK_LOOP_BODY_ULI_T1_LAST;	break;	}		
			case 2:	{	xLast = *((const unsigned short*)(chars + cnt - 2 )) + 0x10000;	PMPML_CHUNK_LOOP_BODY_ULI_T1_LAST;	break;	}		
			case 3:	{	xLast = chars[ cnt - 1 ]; xLast = ( xLast << 16 ) + *((const unsigned short*)(chars + cnt - 3 )) + 0x1000000;	PMPML_CHUNK_LOOP_BODY_ULI_T1_LAST;	break;	}		
*/								
			case 0:	{	return hash_size_0( chars );	}		
			case 1:	{	return hash_size_1( chars );	}		
			case 2:	{	return hash_size_2( chars );	}		
			case 3:	{	return hash_size_3( chars );	}		

			case 4:	{	return hash_size_4( chars );	}		
//			case 4:	{	PMPML_CHUNK_LOOP_BODY_ULI_T1( 0 ) 	xLast = 0x1;	PMPML_CHUNK_LOOP_BODY_ULI_T1_LAST_FOR_JUST_1;	break;	}		
			case 5:	{	PMPML_CHUNK_LOOP_BODY_ULI_T1( 0 ) 	xLast = 0x100 + chars[4];	PMPML_CHUNK_LOOP_BODY_ULI_T1_LAST;	break;	}		
			case 6:	{	PMPML_CHUNK_LOOP_BODY_ULI_T1( 0 ) 	xLast = *((const unsigned short*)(chars + 4 )) + 0x10000;	PMPML_CHUNK_LOOP_BODY_ULI_T1_LAST;	break;	}		
			case 7:	{	PMPML_CHUNK_LOOP_BODY_ULI_T1( 0 ) 	xLast = chars[ 6 ]; xLast = ( xLast << 16 ) + *((const unsigned short*)(chars + 4 )) + 0x1000000;	PMPML_CHUNK_LOOP_BODY_ULI_T1_LAST;	break;	}		
								
			case 8:	{	PMPML_CHUNK_LOOP_BODY_ULI_T1( 0 ) PMPML_CHUNK_LOOP_BODY_ULI_T1( 1 ) 	xLast = 0x1;	PMPML_CHUNK_LOOP_BODY_ULI_T1_LAST_FOR_JUST_1;	break;	}		
			case 9:	{	PMPML_CHUNK_LOOP_BODY_ULI_T1( 0 ) PMPML_CHUNK_LOOP_BODY_ULI_T1( 1 ) 	xLast = 0x100 + chars[8];	PMPML_CHUNK_LOOP_BODY_ULI_T1_LAST;	break;	}		
			case 10:	{	PMPML_CHUNK_LOOP_BODY_ULI_T1( 0 ) PMPML_CHUNK_LOOP_BODY_ULI_T1( 1 ) 	xLast = *((const unsigned short*)(chars + 8 )) + 0x10000;	PMPML_CHUNK_LOOP_BODY_ULI_T1_LAST;	break;	}		
			case 11:	{	PMPML_CHUNK_LOOP_BODY_ULI_T1( 0 ) PMPML_CHUNK_LOOP_BODY_ULI_T1( 1 ) 	xLast = chars[ 10 ]; xLast = ( xLast << 16 ) + *((const unsigned short*)(chars + 8 )) + 0x1000000;	PMPML_CHUNK_LOOP_BODY_ULI_T1_LAST;	break;	}		
								
			case 12:	{	PMPML_CHUNK_LOOP_BODY_ULI_T1( 0 ) PMPML_CHUNK_LOOP_BODY_ULI_T1( 1 ) PMPML_CHUNK_LOOP_BODY_ULI_T1( 2 ) 	xLast = 0x1;	PMPML_CHUNK_LOOP_BODY_ULI_T1_LAST_FOR_JUST_1;	break;	}		
			case 13:	{	PMPML_CHUNK_LOOP_BODY_ULI_T1( 0 ) PMPML_CHUNK_LOOP_BODY_ULI_T1( 1 ) PMPML_CHUNK_LOOP_BODY_ULI_T1( 2 ) 	xLast = 0x100 + chars[12];	PMPML_CHUNK_LOOP_BODY_ULI_T1_LAST;	break;	}		
			case 14:	{	PMPML_CHUNK_LOOP_BODY_ULI_T1( 0 ) PMPML_CHUNK_LOOP_BODY_ULI_T1( 1 ) PMPML_CHUNK_LOOP_BODY_ULI_T1( 2 ) 	xLast = *((const unsigned short*)(chars + 12 )) + 0x10000;	PMPML_CHUNK_LOOP_BODY_ULI_T1_LAST;	break;	}		
			case 15:	{	PMPML_CHUNK_LOOP_BODY_ULI_T1( 0 ) PMPML_CHUNK_LOOP_BODY_ULI_T1( 1 ) PMPML_CHUNK_LOOP_BODY_ULI_T1( 2 ) 	xLast = chars[ 14 ]; xLast = ( xLast << 16 ) + *((const unsigned short*)(chars + 12 )) + 0x1000000;	PMPML_CHUNK_LOOP_BODY_ULI_T1_LAST;	break;	}		
								
			case 16:	{	PMPML_CHUNK_LOOP_BODY_ULI_T1( 0 ) PMPML_CHUNK_LOOP_BODY_ULI_T1( 1 ) PMPML_CHUNK_LOOP_BODY_ULI_T1( 2 ) PMPML_CHUNK_LOOP_BODY_ULI_T1( 3 ) 	xLast = 0x1;	PMPML_CHUNK_LOOP_BODY_ULI_T1_LAST_FOR_JUST_1;	break;	}		
			case 17:	{	PMPML_CHUNK_LOOP_BODY_ULI_T1( 0 ) PMPML_CHUNK_LOOP_BODY_ULI_T1( 1 ) PMPML_CHUNK_LOOP_BODY_ULI_T1( 2 ) PMPML_CHUNK_LOOP_BODY_ULI_T1( 3 ) 	xLast = 0x100 + chars[16];	PMPML_CHUNK_LOOP_BODY_ULI_T1_LAST;	break;	}		
			case 18:	{	PMPML_CHUNK_LOOP_BODY_ULI_T1( 0 ) PMPML_CHUNK_LOOP_BODY_ULI_T1( 1 ) PMPML_CHUNK_LOOP_BODY_ULI_T1( 2 ) PMPML_CHUNK_LOOP_BODY_ULI_T1( 3 ) 	xLast = *((const unsigned short*)(chars + 16 )) + 0x10000;	PMPML_CHUNK_LOOP_BODY_ULI_T1_LAST;	break;	}		
			case 19:	{	PMPML_CHUNK_LOOP_BODY_ULI_T1( 0 ) PMPML_CHUNK_LOOP_BODY_ULI_T1( 1 ) PMPML_CHUNK_LOOP_BODY_ULI_T1( 2 ) PMPML_CHUNK_LOOP_BODY_ULI_T1( 3 ) 	xLast = chars[ 18 ]; xLast = ( xLast << 16 ) + *((const unsigned short*)(chars + 16 )) + 0x1000000;	PMPML_CHUNK_LOOP_BODY_ULI_T1_LAST;	break;	}		
								
			case 20:	{	PMPML_CHUNK_LOOP_BODY_ULI_T1( 0 ) PMPML_CHUNK_LOOP_BODY_ULI_T1( 1 ) PMPML_CHUNK_LOOP_BODY_ULI_T1( 2 ) PMPML_CHUNK_LOOP_BODY_ULI_T1( 3 ) PMPML_CHUNK_LOOP_BODY_ULI_T1( 4 ) 	xLast = 0x1;	PMPML_CHUNK_LOOP_BODY_ULI_T1_LAST_FOR_JUST_1;	break;	}		
			case 21:	{	PMPML_CHUNK_LOOP_BODY_ULI_T1( 0 ) PMPML_CHUNK_LOOP_BODY_ULI_T1( 1 ) PMPML_CHUNK_LOOP_BODY_ULI_T1( 2 ) PMPML_CHUNK_LOOP_BODY_ULI_T1( 3 ) PMPML_CHUNK_LOOP_BODY_ULI_T1( 4 ) 	xLast = 0x100 + chars[20];	PMPML_CHUNK_LOOP_BODY_ULI_T1_LAST;	break;	}		
			case 22:	{	PMPML_CHUNK_LOOP_BODY_ULI_T1( 0 ) PMPML_CHUNK_LOOP_BODY_ULI_T1( 1 ) PMPML_CHUNK_LOOP_BODY_ULI_T1( 2 ) PMPML_CHUNK_LOOP_BODY_ULI_T1( 3 ) PMPML_CHUNK_LOOP_BODY_ULI_T1( 4 ) 	xLast = *((const unsigned short*)(chars + 20 )) + 0x10000;	PMPML_CHUNK_LOOP_BODY_ULI_T1_LAST;	break;	}		
			case 23:	{	PMPML_CHUNK_LOOP_BODY_ULI_T1( 0 ) PMPML_CHUNK_LOOP_BODY_ULI_T1( 1 ) PMPML_CHUNK_LOOP_BODY_ULI_T1( 2 ) PMPML_CHUNK_LOOP_BODY_ULI_T1( 3 ) PMPML_CHUNK_LOOP_BODY_ULI_T1( 4 ) 	xLast = chars[ 22 ]; xLast = ( xLast << 16 ) + *((const unsigned short*)(chars + 20 )) + 0x1000000;	PMPML_CHUNK_LOOP_BODY_ULI_T1_LAST;	break;	}		
								
			case 24:	{	PMPML_CHUNK_LOOP_BODY_ULI_T1( 0 ) PMPML_CHUNK_LOOP_BODY_ULI_T1( 1 ) PMPML_CHUNK_LOOP_BODY_ULI_T1( 2 ) PMPML_CHUNK_LOOP_BODY_ULI_T1( 3 ) PMPML_CHUNK_LOOP_BODY_ULI_T1( 4 ) PMPML_CHUNK_LOOP_BODY_ULI_T1( 5 ) 	xLast = 0x1;	PMPML_CHUNK_LOOP_BODY_ULI_T1_LAST_FOR_JUST_1;	break;	}		
			case 25:	{	PMPML_CHUNK_LOOP_BODY_ULI_T1( 0 ) PMPML_CHUNK_LOOP_BODY_ULI_T1( 1 ) PMPML_CHUNK_LOOP_BODY_ULI_T1( 2 ) PMPML_CHUNK_LOOP_BODY_ULI_T1( 3 ) PMPML_CHUNK_LOOP_BODY_ULI_T1( 4 ) PMPML_CHUNK_LOOP_BODY_ULI_T1( 5 ) 	xLast = 0x100 + chars[24];	PMPML_CHUNK_LOOP_BODY_ULI_T1_LAST;	break;	}		
			case 26:	{	PMPML_CHUNK_LOOP_BODY_ULI_T1( 0 ) PMPML_CHUNK_LOOP_BODY_ULI_T1( 1 ) PMPML_CHUNK_LOOP_BODY_ULI_T1( 2 ) PMPML_CHUNK_LOOP_BODY_ULI_T1( 3 ) PMPML_CHUNK_LOOP_BODY_ULI_T1( 4 ) PMPML_CHUNK_LOOP_BODY_ULI_T1( 5 ) 	xLast = *((const unsigned short*)(chars + 24 )) + 0x10000;	PMPML_CHUNK_LOOP_BODY_ULI_T1_LAST;	break;	}		
			case 27:	{	PMPML_CHUNK_LOOP_BODY_ULI_T1( 0 ) PMPML_CHUNK_LOOP_BODY_ULI_T1( 1 ) PMPML_CHUNK_LOOP_BODY_ULI_T1( 2 ) PMPML_CHUNK_LOOP_BODY_ULI_T1( 3 ) PMPML_CHUNK_LOOP_BODY_ULI_T1( 4 ) PMPML_CHUNK_LOOP_BODY_ULI_T1( 5 ) 	xLast = chars[ 26 ]; xLast = ( xLast << 16 ) + *((const unsigned short*)(chars + 24 )) + 0x1000000;	PMPML_CHUNK_LOOP_BODY_ULI_T1_LAST;	break;	}		
								
/*			case 28:	{	PMPML_CHUNK_LOOP_BODY_ULI_T1( 0 ) PMPML_CHUNK_LOOP_BODY_ULI_T1( 1 ) PMPML_CHUNK_LOOP_BODY_ULI_T1( 2 ) PMPML_CHUNK_LOOP_BODY_ULI_T1( 3 ) PMPML_CHUNK_LOOP_BODY_ULI_T1( 4 ) PMPML_CHUNK_LOOP_BODY_ULI_T1( 5 ) PMPML_CHUNK_LOOP_BODY_ULI_T1( 6 ) 	xLast = 0x1;	PMPML_CHUNK_LOOP_BODY_ULI_T1_LAST_FOR_JUST_1;	break;	}		
			case 29:	{	PMPML_CHUNK_LOOP_BODY_ULI_T1( 0 ) PMPML_CHUNK_LOOP_BODY_ULI_T1( 1 ) PMPML_CHUNK_LOOP_BODY_ULI_T1( 2 ) PMPML_CHUNK_LOOP_BODY_ULI_T1( 3 ) PMPML_CHUNK_LOOP_BODY_ULI_T1( 4 ) PMPML_CHUNK_LOOP_BODY_ULI_T1( 5 ) PMPML_CHUNK_LOOP_BODY_ULI_T1( 6 ) 	xLast = 0x100 + chars[28];	PMPML_CHUNK_LOOP_BODY_ULI_T1_LAST;	break;	}		
			case 30:	{	PMPML_CHUNK_LOOP_BODY_ULI_T1( 0 ) PMPML_CHUNK_LOOP_BODY_ULI_T1( 1 ) PMPML_CHUNK_LOOP_BODY_ULI_T1( 2 ) PMPML_CHUNK_LOOP_BODY_ULI_T1( 3 ) PMPML_CHUNK_LOOP_BODY_ULI_T1( 4 ) PMPML_CHUNK_LOOP_BODY_ULI_T1( 5 ) PMPML_CHUNK_LOOP_BODY_ULI_T1( 6 ) 	xLast = *((const unsigned short*)(chars + 28 )) + 0x10000;	PMPML_CHUNK_LOOP_BODY_ULI_T1_LAST;	break;	}		
			default:	{	PMPML_CHUNK_LOOP_BODY_ULI_T1( 0 ) PMPML_CHUNK_LOOP_BODY_ULI_T1( 1 ) PMPML_CHUNK_LOOP_BODY_ULI_T1( 2 ) PMPML_CHUNK_LOOP_BODY_ULI_T1( 3 ) PMPML_CHUNK_LOOP_BODY_ULI_T1( 4 ) PMPML_CHUNK_LOOP_BODY_ULI_T1( 5 ) PMPML_CHUNK_LOOP_BODY_ULI_T1( 6 ) 	xLast = chars[ 30 ]; xLast = ( xLast << 16 ) + *((const unsigned short*)(chars + 28 )) + 0x1000000;	PMPML_CHUNK_LOOP_BODY_ULI_T1_LAST;	break;	}	
*/
			case 28:	{	return hash_size_28( chars );	}		
			case 29:	{	return hash_size_29( chars );	}		
			case 30:	{	return hash_size_30( chars );	}		
			default:	{	return hash_size_31( chars );	}		
		}
#endif // PMPML_MSC_32_WORKAROUND
#else // PMPML_STRICT_UNALIGNED_HANDLING
		if ( _LIKELY_BRANCH_( ( cnt & ( PMPML_WORD_SIZE_BYTES - 1 ) ) == 0 ) )
		{
			const uint32_t* x = (const uint32_t*)chars;

			switch( size )
			{
				case 0: { break; }
				case 1:	{	PMPML_CHUNK_LOOP_BODY_ULI_T1( 0 ) } break;
				case 2:	{	PMPML_CHUNK_LOOP_BODY_ULI_T1( 0 ) PMPML_CHUNK_LOOP_BODY_ULI_T1( 1 ) } break;
				case 3:	{	PMPML_CHUNK_LOOP_BODY_ULI_T1( 0 ) PMPML_CHUNK_LOOP_BODY_ULI_T1( 1 ) PMPML_CHUNK_LOOP_BODY_ULI_T1( 2 ) } break;
				case 4:	{	PMPML_CHUNK_LOOP_BODY_ULI_T1( 0 ) PMPML_CHUNK_LOOP_BODY_ULI_T1( 1 ) PMPML_CHUNK_LOOP_BODY_ULI_T1( 2 ) PMPML_CHUNK_LOOP_BODY_ULI_T1( 3 ) } break;
				case 5:	{	PMPML_CHUNK_LOOP_BODY_ULI_T1( 0 ) PMPML_CHUNK_LOOP_BODY_ULI_T1( 1 ) PMPML_CHUNK_LOOP_BODY_ULI_T1( 2 ) PMPML_CHUNK_LOOP_BODY_ULI_T1( 3 ) PMPML_CHUNK_LOOP_BODY_ULI_T1( 4 ) } break;
				case 6:	{	PMPML_CHUNK_LOOP_BODY_ULI_T1( 0 ) PMPML_CHUNK_LOOP_BODY_ULI_T1( 1 ) PMPML_CHUNK_LOOP_BODY_ULI_T1( 2 ) PMPML_CHUNK_LOOP_BODY_ULI_T1( 3 ) PMPML_CHUNK_LOOP_BODY_ULI_T1( 4 ) PMPML_CHUNK_LOOP_BODY_ULI_T1( 5 ) } break;
				default:	{	PMPML_CHUNK_LOOP_BODY_ULI_T1( 0 ) PMPML_CHUNK_LOOP_BODY_ULI_T1( 1 ) PMPML_CHUNK_LOOP_BODY_ULI_T1( 2 ) PMPML_CHUNK_LOOP_BODY_ULI_T1( 3 ) PMPML_CHUNK_LOOP_BODY_ULI_T1( 4 ) PMPML_CHUNK_LOOP_BODY_ULI_T1( 5 ) PMPML_CHUNK_LOOP_BODY_ULI_T1( 6 ) } break;
			}

			switch ( cnt & ( PMPML_WORD_SIZE_BYTES - 1 ) )
			{
				case 0: { xLast = 0x1; break;}
				case 1: { xLast = 0x100 + chars[cnt-1]; break;}
				case 2: { xLast = *((const unsigned short*)(chars + cnt - 2 )) + 0x10000; break; }
				default: { xLast = chars[ cnt - 1 ]; xLast = ( xLast << 16 ) + *((const unsigned short*)(chars + cnt - 3 )) + 0x1000000; break;}
			}
		}
		else
		{
			const unaligned_uint32* xun = (const unaligned_uint32*)chars;

			switch( size )
			{
				case 0: { break; }
				case 1:	{	PMPML_CHUNK_LOOP_BODY_ULI_T1_UNL_PTR( 0 ) } break;
				case 2:	{	PMPML_CHUNK_LOOP_BODY_ULI_T1_UNL_PTR( 0 ) PMPML_CHUNK_LOOP_BODY_ULI_T1_UNL_PTR( 1 ) } break;
				case 3:	{	PMPML_CHUNK_LOOP_BODY_ULI_T1_UNL_PTR( 0 ) PMPML_CHUNK_LOOP_BODY_ULI_T1_UNL_PTR( 1 ) PMPML_CHUNK_LOOP_BODY_ULI_T1_UNL_PTR( 2 ) } break;
				case 4:	{	PMPML_CHUNK_LOOP_BODY_ULI_T1_UNL_PTR( 0 ) PMPML_CHUNK_LOOP_BODY_ULI_T1_UNL_PTR( 1 ) PMPML_CHUNK_LOOP_BODY_ULI_T1_UNL_PTR( 2 ) PMPML_CHUNK_LOOP_BODY_ULI_T1_UNL_PTR( 3 ) } break;
				case 5:	{	PMPML_CHUNK_LOOP_BODY_ULI_T1_UNL_PTR( 0 ) PMPML_CHUNK_LOOP_BODY_ULI_T1_UNL_PTR( 1 ) PMPML_CHUNK_LOOP_BODY_ULI_T1_UNL_PTR( 2 ) PMPML_CHUNK_LOOP_BODY_ULI_T1_UNL_PTR( 3 ) PMPML_CHUNK_LOOP_BODY_ULI_T1_UNL_PTR( 4 ) } break;
				case 6:	{	PMPML_CHUNK_LOOP_BODY_ULI_T1_UNL_PTR( 0 ) PMPML_CHUNK_LOOP_BODY_ULI_T1_UNL_PTR( 1 ) PMPML_CHUNK_LOOP_BODY_ULI_T1_UNL_PTR( 2 ) PMPML_CHUNK_LOOP_BODY_ULI_T1_UNL_PTR( 3 ) PMPML_CHUNK_LOOP_BODY_ULI_T1_UNL_PTR( 4 ) PMPML_CHUNK_LOOP_BODY_ULI_T1_UNL_PTR( 5 ) } break;
				default:	{	PMPML_CHUNK_LOOP_BODY_ULI_T1_UNL_PTR( 0 ) PMPML_CHUNK_LOOP_BODY_ULI_T1_UNL_PTR( 1 ) PMPML_CHUNK_LOOP_BODY_ULI_T1_UNL_PTR( 2 ) PMPML_CHUNK_LOOP_BODY_ULI_T1_UNL_PTR( 3 ) PMPML_CHUNK_LOOP_BODY_ULI_T1_UNL_PTR( 4 ) PMPML_CHUNK_LOOP_BODY_ULI_T1_UNL_PTR( 5 ) PMPML_CHUNK_LOOP_BODY_ULI_T1_UNL_PTR( 6 ) } break;
			}

			switch ( cnt & ( PMPML_WORD_SIZE_BYTES - 1 ) )
			{
				case 1: { xLast = 0x100 | chars[cnt-1]; break;}
				case 2: { xLast = 0x100 | chars[cnt-1]; xLast = ( xLast << 8 ) | chars[cnt - 2]; break; }
				case 3: { xLast = 0x100 | chars[cnt-1]; xLast = ( xLast << 8 ) | chars[cnt - 2]; xLast = ( xLast << 8 ) | chars[cnt - 3]; break;}
			}
		}
#endif // PMPML_STRICT_UNALIGNED_HANDLING

#if !defined PMPML_MSC_32_WORKAROUND
		PMPML_CHUNK_LOOP_BODY_ULI_T1_LAST
#endif

		PMPML_CHUNK_LOOP_PRE_REDUCE_L0

		PMPML_FULL_REDUCE_MOD_2_32_PLUS_15_AND_RETURN_RETURN
	}
	else if ( cnt < PMPML_CHUNK_SIZE_BYTES )
	{
		return _hash_noRecursionNoInline_forLessThanChunk( chars, cnt );
	}
	else
	{
		return _hash_noRecursionNoInline_type2( chars, cnt );
	}
  }

  NOINLINE uint32_t _hash_noRecursionNoInline_forLessThanChunk(const unsigned char* chars, unsigned int cnt) const
  {
			unsigned int i;
			ULARGE_INTEGER__XX tmp_hash;
#if !defined PMPML_STRICT_UNALIGNED_HANDLING
			tmp_hash.QuadPart = hash_of_beginning_of_string_chunk_type2( curr_rd[0].random_coeff, *(ULARGE_INTEGER__XX*)(&(curr_rd[0].const_term)), chars, cnt );
#else // PMPML_STRICT_UNALIGNED_HANDLING
			switch (( ( (uint32_t)chars) & 3))
			{
			case 0: tmp_hash.QuadPart = hash_of_beginning_of_string_chunk_type2( curr_rd[0].random_coeff, *(ULARGE_INTEGER__XX*)(&(curr_rd[0].const_term)), chars, cnt ); break;
			case 1: tmp_hash.QuadPart = hash_of_beginning_of_string_chunk_type2_un1( curr_rd[0].random_coeff, *(ULARGE_INTEGER__XX*)(&(curr_rd[0].const_term)), chars, cnt ); break;
			case 2: tmp_hash.QuadPart = hash_of_beginning_of_string_chunk_type2_un2( curr_rd[0].random_coeff, *(ULARGE_INTEGER__XX*)(&(curr_rd[0].const_term)), chars, cnt ); break;
			case 3: tmp_hash.QuadPart = hash_of_beginning_of_string_chunk_type2_un3( curr_rd[0].random_coeff, *(ULARGE_INTEGER__XX*)(&(curr_rd[0].const_term)), chars, cnt ); break;
			}
#endif // PMPML_STRICT_UNALIGNED_HANDLING
			if ( tmp_hash.HighPart == 0 ) //LIKELY
			{
				return fmix32_short( tmp_hash.LowPart );
			}
			return tmp_hash.LowPart;
  }

  NOINLINE uint32_t _hash_noRecursionNoInline_type2(const unsigned char* chars, unsigned int cnt) const
  {
			uint64_t allValues[ PMPML_LEVELS * PMPML_CHUNK_SIZE ];
			unsigned int cnts[ PMPML_LEVELS ];
			unsigned int flag;
			cnts[ 1 ] = 0;
			flag = 0;

			unsigned int i;
			ULARGE_INTEGER__XX tmp_hash;
#if !defined PMPML_STRICT_UNALIGNED_HANDLING
			// process full chunks
			for ( i=0; i<(cnt>>PMPML_CHUNK_SIZE_BYTES_LOG2); i++ )
			{
				tmp_hash.QuadPart = hash_of_string_chunk_compact( curr_rd[0].random_coeff, *(ULARGE_INTEGER__XX*)(&(curr_rd[0].const_term)), ((const uint32_t*)(chars)) + ( i << PMPML_CHUNK_SIZE_LOG2 ) );
				procesNextValue( 1, tmp_hash.QuadPart, allValues, cnts, flag );
			}
#else // PMPML_STRICT_UNALIGNED_HANDLING
			// process full chunks
			switch (( ( (uint32_t)chars) & 3))
			{
			case 0: for ( i=0; i<(cnt>>PMPML_CHUNK_SIZE_BYTES_LOG2); i++ ){ tmp_hash.QuadPart = hash_of_string_chunk_compact( curr_rd[0].random_coeff, *(ULARGE_INTEGER__XX*)(&(curr_rd[0].const_term)), ((const uint32_t*)(chars)) + ( i << PMPML_CHUNK_SIZE_LOG2 ) ); procesNextValue( 1, tmp_hash.QuadPart, allValues, cnts, flag );} break;
			case 1: for ( i=0; i<(cnt>>PMPML_CHUNK_SIZE_BYTES_LOG2); i++ ){ tmp_hash.QuadPart = hash_of_string_chunk_compact_un_1( curr_rd[0].random_coeff, *(ULARGE_INTEGER__XX*)(&(curr_rd[0].const_term)), chars + ( i << PMPML_CHUNK_SIZE_BYTES_LOG2 ) ); procesNextValue( 1, tmp_hash.QuadPart, allValues, cnts, flag );} break;
			case 2: for ( i=0; i<(cnt>>PMPML_CHUNK_SIZE_BYTES_LOG2); i++ ){ tmp_hash.QuadPart = hash_of_string_chunk_compact_un_2( curr_rd[0].random_coeff, *(ULARGE_INTEGER__XX*)(&(curr_rd[0].const_term)), chars + ( i << PMPML_CHUNK_SIZE_BYTES_LOG2 ) ); procesNextValue( 1, tmp_hash.QuadPart, allValues, cnts, flag );} break;
			case 3: for ( i=0; i<(cnt>>PMPML_CHUNK_SIZE_BYTES_LOG2); i++ ){ tmp_hash.QuadPart = hash_of_string_chunk_compact_un_3( curr_rd[0].random_coeff, *(ULARGE_INTEGER__XX*)(&(curr_rd[0].const_term)), chars + ( i << PMPML_CHUNK_SIZE_BYTES_LOG2 ) ); procesNextValue( 1, tmp_hash.QuadPart, allValues, cnts, flag );} break;
			}
#endif // PMPML_STRICT_UNALIGNED_HANDLING
			// process remaining incomplete chunk(s)
			// note: if string size is a multiple of chunk size, we create a new chunk (1,0,0,...0),
			// so THIS PROCESSING IS ALWAYS PERFORMED
			unsigned int tailCnt = cnt & ( PMPML_CHUNK_SIZE_BYTES - 1 );
			const unsigned char* tail = chars + ( (cnt>>PMPML_CHUNK_SIZE_BYTES_LOG2) << PMPML_CHUNK_SIZE_BYTES_LOG2 );
#if !defined PMPML_STRICT_UNALIGNED_HANDLING
			tmp_hash.QuadPart = hash_of_beginning_of_string_chunk_type2( curr_rd[0].random_coeff, *(ULARGE_INTEGER__XX*)(&(curr_rd[0].const_term)), tail, tailCnt );
#else // PMPML_STRICT_UNALIGNED_HANDLING
			switch (( ( (uint32_t)chars) & 3))
			{
			case 0: tmp_hash.QuadPart = hash_of_beginning_of_string_chunk_type2( curr_rd[0].random_coeff, *(ULARGE_INTEGER__XX*)(&(curr_rd[0].const_term)), tail, tailCnt ); break;
			case 1: tmp_hash.QuadPart = hash_of_beginning_of_string_chunk_type2_un1( curr_rd[0].random_coeff, *(ULARGE_INTEGER__XX*)(&(curr_rd[0].const_term)), tail, tailCnt ); break;
			case 2: tmp_hash.QuadPart = hash_of_beginning_of_string_chunk_type2_un2( curr_rd[0].random_coeff, *(ULARGE_INTEGER__XX*)(&(curr_rd[0].const_term)), tail, tailCnt ); break;
			case 3: tmp_hash.QuadPart = hash_of_beginning_of_string_chunk_type2_un3( curr_rd[0].random_coeff, *(ULARGE_INTEGER__XX*)(&(curr_rd[0].const_term)), tail, tailCnt ); break;
			}
#endif // PMPML_STRICT_UNALIGNED_HANDLING
			procesNextValue( 1, tmp_hash.QuadPart, allValues, cnts, flag );
			ULARGE_INTEGER__XX ret64;
			ret64.QuadPart = finalize( 1, allValues, cnts, flag );
			if ( ret64.HighPart == 0 ) //LIKELY
			{
				return fmix32_short( ret64.LowPart );
			}
			return ret64.LowPart;
  }

  public:
  PMP_Multilinear_Hasher()
  {
#ifdef PMPML_USE_SSE
	base_addr = NULL;
#endif
        curr_rd = (random_data_for_MPSHF*)rd_for_MPSHF;
  }
  virtual ~PMP_Multilinear_Hasher()
  {
#ifdef PMPML_USE_SSE
    if ( base_addr != NULL )
        delete [] base_addr;
#else
    if ( curr_rd != NULL && curr_rd != rd_for_MPSHF )
        delete [] curr_rd;
#endif
  }


  //NOTE: no random stuff can be called by any of the functions above
  void randomize( UniformRandomNumberGenerator& rng )
  {

#if !defined PMPML_USE_SSE
    random_data_for_MPSHF * temp_curr_rd = new random_data_for_MPSHF[ PMPML_LEVELS ];
#else
	if ( base_addr != NULL ) delete [] base_addr;
	base_addr = new unsigned char [ sizeof( random_data_for_MPSHF ) * PMPML_LEVELS + 32 ];
	random_data_for_MPSHF * temp_curr_rd = (random_data_for_MPSHF*)((((size_t)base_addr) + 15) &  UINT64_C(0xFFFFFFFFFFFFFFF0));
#endif

	int i, j;
	for ( i=0; i<PMPML_LEVELS; i++ )
		for ( j=0; j<PMPML_CHUNK_SIZE; j++ )
		{
			do
			{
				temp_curr_rd[ i ].random_coeff[ j ] = rng.rand();
			}
			while ( !IS_VALID_COEFFICIENT( temp_curr_rd[ i ].random_coeff[ j ], i ) );
		}

	for ( i=0; i<PMPML_LEVELS; i++ )
	{
		uint64_t rv;
		do
		{
			rv = rng.rand();
			rv <<= 32;
			rv |= rng.rand();
		}
		while ( rv == 0 );
		rv = rv % PMPML_MAIN_PRIME;
		temp_curr_rd[ i ].const_term = rv;
	}

#ifdef PMPML_USE_SSE
	curr_rd = temp_curr_rd;
#else
	if ( curr_rd == rd_for_MPSHF )
		curr_rd = temp_curr_rd;
	else
	{
		if ( curr_rd != NULL )
			delete [] curr_rd;
		curr_rd = temp_curr_rd;
	}
#endif
  }
  void seed( uint32_t seed )
  {
    curr_rd[0].const_term ^= seed;
  }
#if 0
  void calc_vals()
  {
	  int i, j;
	for ( i=0; i<PMPML_LEVELS; i++ )
	{
		uint64_t sum = 0;
		for ( j=0; j<PMPML_CHUNK_SIZE; j++ )
		{
			sum += curr_rd[ i ].random_coeff[ j ];
		}
		printf( "UINT64_C( 0x%x%08x )\n", (uint32_t)( sum >> 32 ), (uint32_t)( sum ) );
	}
	printf( "\n\n\n\n\n" );
  }
#endif // 0
};

#endif // __PMP_MULTILINEAR_HASHER_H__
