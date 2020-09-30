/**
 * @file prvhash42ec.h
 *
 * @brief The inclusion file for the "prvhash42_ec" endianness-correction
 * functions.
 *
 * @mainpage
 *
 * @section intro_sec Introduction
 *
 * Description is available at https://github.com/avaneev/prvhash
 *
 * @section license License
 *
 * Copyright (c) 2020 Aleksey Vaneev
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 *
 * @version 2.24
 */

//$ nocpp

#ifndef PRVHASH42EC_INCLUDED
#define PRVHASH42EC_INCLUDED

#include <stdint.h>

#if defined( _WIN32 ) || defined( __LITTLE_ENDIAN__ ) || ( defined( __BYTE_ORDER__ ) && __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__ )
	#define PRVHASH42_LITTLE_ENDIAN 1
#elif defined( __BIG_ENDIAN__ ) || ( defined( __BYTE_ORDER__ ) && __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__ )
	#define PRVHASH42_LITTLE_ENDIAN 0
#else
	#error PRVHASH42: cannot obtain endianness
#endif

#if PRVHASH42_LITTLE_ENDIAN

/**
 * An auxiliary function that returns an unsigned 64-bit value created out of
 * individual bytes in a buffer. This function is used to preserve endianness
 * of supplied 64-bit unsigned values.
 *
 * @param p 8-byte buffer. Alignment is unimportant.
 */

inline uint64_t prvhash42_u64ec( const uint8_t* const p )
{
	uint64_t v;
	memcpy( &v, p, 8 );

	return( v );
}

/**
 * An auxiliary function that returns an unsigned 32-bit value created out of
 * individual bytes in a buffer. This function is used to preserve endianness
 * of supplied 32-bit unsigned values.
 *
 * @param p 4-byte buffer. Alignment is unimportant.
 */

inline uint32_t prvhash42_u32ec( const uint8_t* const p )
{
	uint32_t v;
	memcpy( &v, p, 4 );

	return( v );
}

#else // PRVHASH42_LITTLE_ENDIAN

#if defined( __GNUC__ ) || defined( __INTEL_COMPILER ) || defined( __clang__ )

inline uint64_t prvhash42_u64ec( const uint8_t* const p )
{
	uint64_t v;
	memcpy( &v, p, 8 );

	return( __builtin_bswap64( v ));
}

inline uint32_t prvhash42_u32ec( const uint8_t* const p )
{
	uint32_t v;
	memcpy( &v, p, 4 );

	return( __builtin_bswap32( v ));
}

#elif defined( _MSC_VER )

inline uint64_t prvhash42_u64ec( const uint8_t* const p )
{
	uint64_t v;
	memcpy( &v, p, 8 );

	return( _byteswap_uint64( v ));
}

inline uint32_t prvhash42_u32ec( const uint8_t* const p )
{
	uint32_t v;
	memcpy( &v, p, 4 );

	return( _byteswap_ulong( v ));
}

#endif // defined( _MSC_VER )

#endif // PRVHASH42_LITTLE_ENDIAN

#if PRVHASH42_LITTLE_ENDIAN

/**
 * This function corrects (inverses) endianness of the specified hash value.
 *
 * @param[in,out] Hash The hash to correct endianness of. On systems where
 * this is relevant, this address should be aligned to 32 bits.
 * @param HashLen The required hash length, in bytes, should be >= 4, in
 * increments of 4. 
 */

inline void prvhash42_ec( uint8_t* const Hash, const int HashLen )
{
}

#else // PRVHASH42_LITTLE_ENDIAN

inline void prvhash42_ec( uint8_t* const Hash, const int HashLen )
{
	int k;

	for( k = 0; k < HashLen; k += 4 )
	{
		*(uint32_t*) ( Hash + k ) = prvhash42_u32ec( Hash + k );
	}
}

#endif // PRVHASH42_LITTLE_ENDIAN

/**
 * Function loads 32-bit message word and pads it with "final byte" if read
 * occurs beyond message end.
 *
 * @param Msg Message pointer, alignment is unimportant.
 * @param MsgEnd Message's end pointer.
 * @param fb Final byte used for padding.
 */

inline uint32_t prvhash42_lp32( const uint8_t* Msg,
	const uint8_t* const MsgEnd, const uint8_t fb )
{
	if( Msg < MsgEnd - 3 )
	{
		return( prvhash42_u32ec( Msg ));
	}

	uint32_t r = ( Msg < MsgEnd ? *Msg : fb ) | (uint32_t) fb << 24;
	Msg++;
	r |= (uint32_t) ( Msg < MsgEnd ? *Msg : fb ) << 8;
	Msg++;
	r |= (uint32_t) ( Msg < MsgEnd ? *Msg : fb ) << 16;

	return( r );
}

/**
 * Function loads 32-bit message word and pads it with "final byte" if read
 * occurs beyond message end. This variant of the function assumes that
 * Msg < MsgEnd.
 *
 * @param Msg Message pointer, alignment is unimportant.
 * @param MsgEnd Message's end pointer.
 * @param fb Final byte used for padding.
 */

inline uint32_t prvhash42_lp32_1( const uint8_t* Msg,
	const uint8_t* const MsgEnd, const uint8_t fb )
{
	if( Msg < MsgEnd - 3 )
	{
		return( prvhash42_u32ec( Msg ));
	}

	uint32_t r = *Msg | (uint32_t) fb << 24;
	Msg++;
	r |= (uint32_t) ( Msg < MsgEnd ? *Msg : fb ) << 8;
	Msg++;
	r |= (uint32_t) ( Msg < MsgEnd ? *Msg : fb ) << 16;

	return( r );
}

/**
 * This function runs a single random number generation round. Several calls
 * to this function can be used to "condition" the initial state of the
 * hash function.
 *
 * This function can be also used as an effective general-purpose random
 * number generator. In this case, it is advisable to initially run this
 * function 4 times before using its random output, to neutralize any possible
 * oddities of "Seed"'s and "lcg"'s initial values.
 *
 * @param[in,out] Seed The current "Seed" value. Can be initialized to any
 * value.
 * @param[in,out] lcg The current "lcg" value. Can be initialized to any
 * value.
 * @return Current random value.
 */

inline uint32_t prvhash42_rnd( uint64_t& Seed, uint64_t& lcg )
{
	Seed += lcg;
	Seed *= ~lcg - lcg;
	lcg += ~Seed;
	const uint64_t hl = Seed >> 32;
	Seed ^= hl;

	return( hl );
}

/**
 * This function "conditions" 64-bit values contained in the InitVec buffer.
 * In essence, this function runs several random number generation rounds thus
 * tranforming any type of entropy contained in the InitVec into entropy
 * suitable for the hash function's InitVec.
 *
 * @param[in,out] InitVec Entropy buffer.
 * @param c The number of Seed-lcg pairs in the buffer.
 */

inline void prvhash42_cond( uint8_t InitVec[], const int c )
{
	int k;

	for( k = 0; k < c * 16; k += 16 )
	{
		uint64_t Seed = prvhash42_u64ec( InitVec + k );
		uint64_t lcg = prvhash42_u64ec( InitVec + k + 8 );
		int i;

		for( i = 0; i < 4; i++ )
		{
			prvhash42_rnd( Seed, lcg );
		}

		Seed = prvhash42_u64ec( (uint8_t*) &Seed );
		lcg = prvhash42_u64ec( (uint8_t*) &lcg );
		memcpy( InitVec + k, &Seed, 8 );
		memcpy( InitVec + k + 8, &lcg, 8 );
	}
}

#endif // PRVHASH42EC_INCLUDED
