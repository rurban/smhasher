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

// PMP_Multilinear_common_naive.h: common defs for PMP+-Multilinear hash family naive implementation

#if !defined __MULTILINEARPRIMESTRINGHASHFUNCTOR_COMMON_NAIVE_H__
#define __MULTILINEARPRIMESTRINGHASHFUNCTOR_COMMON_NAIVE_H__

 
#include <boost/multiprecision/cpp_int.hpp>
using namespace boost::multiprecision;
#if !defined(_MSC_VER)
#ifndef UINT64_C
#define UINT64_C(x) (x##LLU)
#endif
#endif

//#define UInt32x32To64(a, b) ((uint64_t)(((uint64_t)((uint32_t)(a))) * ((uint32_t)(b))))

#if !defined NULL
#define NULL 0
#endif


typedef union _ULARGE_INTEGER__XX
{
  struct {
    uint32_t LowPart;
    uint32_t HighPart;
  };
  struct {
    uint32_t LowPart;
    uint32_t HighPart;
  } u;
  uint64_t QuadPart;
} ULARGE_INTEGER__XX;

typedef struct _ULARGELARGE_INTEGER__XX
{
    uint64_t LowPart;
    uint64_t HighPart;
} ULARGELARGE_INTEGER__XX;

class UniformRandomNumberGenerator
{
public:
	virtual uint32_t rand() = 0;
};


#ifdef __arm__
typedef struct { uint32_t value __attribute__((__packed__)); } unaligned_uint32;
typedef struct { uint64_t value __attribute__((__packed__)); } unaligned_uint64;
#else
typedef struct { uint32_t value; } unaligned_uint32;
typedef struct { uint64_t value; } unaligned_uint64;
#endif // __arm__

#include <functional>
using namespace std;


inline
unsigned int fmix32_short ( unsigned int h )
{
  h ^= h >> 13;
  h *= 0xab3be54f;
  h ^= h >> 16;

  return h;
}

inline
uint64_t fmix64_short ( uint64_t k )
{
  k ^= k >> 33;
  k *= UINT64_C( 0xc4ceb9fe1a85ec53 );
  k ^= k >> 33;

  return k;
}

/////////////////    32-BIT OUTPUT STUFF    /////////////////

// constants
#define PMPML_MAIN_PRIME UINT64_C(4294967311) // 2**32+15
#define POW_2_64_MOD_PMPML_MAIN_PRIME UINT64_C(225) // 2^64 % 4294967311

#define PMPML_MAXMULKEY_VALUE_32  0xfffffff2 // we have that  (2**32 - 14) * (2**32+14) <2**64 or 0xFFFFFFF2 * (2^32+14) < 2^64
#define IS_VALID_COEFFICIENT_LEVEL_0( x ) ( (x) > 0 )
#define IS_VALID_COEFFICIENT_LEVEL_1PLUS( x ) ( ( (x) > 0 ) && ( (x) <= PMPML_MAXMULKEY_VALUE_32 ) )
#define IS_VALID_COEFFICIENT( x, level ) ( (level) > 0 ? IS_VALID_COEFFICIENT_LEVEL_1PLUS( (x) ) : IS_VALID_COEFFICIENT_LEVEL_0( (x) ) )

#define PMPML_CHUNK_SIZE 128
#define PMPML_CHUNK_SIZE_LOG2 7 // derived
#define PMPML_WORD_SIZE_BYTES 4
#define PMPML_CHUNK_SIZE_BYTES ( PMPML_CHUNK_SIZE * PMPML_WORD_SIZE_BYTES )
#define PMPML_WORD_SIZE_BYTES_LOG2 2
#define PMPML_CHUNK_SIZE_BYTES_LOG2 ( PMPML_CHUNK_SIZE_LOG2 + PMPML_WORD_SIZE_BYTES_LOG2 ) // derived
#define PMPML_LEVELS 8

// container for coefficients
typedef struct _random_data_for_MPSHF
{
    uint64_t const_term;
    uint32_t random_coeff[ PMPML_CHUNK_SIZE ];
} random_data_for_MPSHF;
extern const random_data_for_MPSHF rd_for_MPSHF[ PMPML_LEVELS ];



/////////////////    64-BIT OUTPUT STUFF    /////////////////

// constants
#define PMPML_MAIN_PRIME_64 UINT64_C(13)
#define POW_2_128_MOD_PMPML_MAIN_PRIME_64 UINT64_C(169) // 2^128 % (2^64+13)

#define PMPML_MAXMULKEY_VALUE_64  UINT64_C( 0xFFFFFFFFFFFFFFF4 ) // we have that (2**64-12) * (2**64+12) <2**128 or  0xfffffffffffffff4 * (2^64+12) < 2^128
#define IS_VALID_COEFFICIENT_LEVEL_0_64( x ) ( (x) > 0 )
#define IS_VALID_COEFFICIENT_LEVEL_1PLUS_64( x ) ( ( (x) > 0 ) && ( (x) <= PMPML_MAXMULKEY_VALUE_64 ) )
#define IS_VALID_COEFFICIENT_64( x, level ) ( (level) > 0 ? IS_VALID_COEFFICIENT_LEVEL_1PLUS_64( (x) ) : IS_VALID_COEFFICIENT_LEVEL_0_64( (x) ) )

#define PMPML_CHUNK_SIZE_64 128
#define PMPML_CHUNK_SIZE_LOG2_64 7 // derived
#define PMPML_WORD_SIZE_BYTES_64 8
#define PMPML_CHUNK_SIZE_BYTES_64 ( PMPML_CHUNK_SIZE_64 * PMPML_WORD_SIZE_BYTES_64 )
#define PMPML_WORD_SIZE_BYTES_LOG2_64 3
#define PMPML_CHUNK_SIZE_BYTES_LOG2_64 ( PMPML_CHUNK_SIZE_LOG2_64 + PMPML_WORD_SIZE_BYTES_LOG2_64 ) // derived
#define PMPML_LEVELS_64 8

// container for coefficients
typedef struct _random_data_for_PMPML_64
{
    uint64_t const_term;
    uint64_t random_coeff[ PMPML_CHUNK_SIZE_64 ];
} random_data_for_PMPML_64;
extern const random_data_for_PMPML_64 rd_for_PMPML_64[ PMPML_LEVELS_64 ];

// some macros


#endif // __MULTILINEARPRIMESTRINGHASHFUNCTOR_COMMON_NAIVE_H__
