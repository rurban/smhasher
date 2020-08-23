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
 * @version 2.14
 */

//$ nocpp

#ifndef PRVHASH42EC_INCLUDED
#define PRVHASH42EC_INCLUDED

#include <stdint.h>

/**
 * This function corrects (inverses) endianness of the specified hash value.
 *
 * @param[in,out] Hash The hash to correct endianness of.
 * @param HashLen The required hash length, in bytes, should be >= 4, in
 * increments of 4.
 */

inline void prvhash42_ec( uint8_t* const Hash, const int HashLen )
{
	int e = 1;

	if( *(uint8_t*) &e == 0 )
	{
		int k;

		for( k = 0; k < HashLen; k += 4 )
		{
			const uint8_t h0 = Hash[ k + 0 ];
			const uint8_t h1 = Hash[ k + 1 ];
			Hash[ k + 0 ] = Hash[ k + 3 ];
			Hash[ k + 1 ] = Hash[ k + 2 ];
			Hash[ k + 2 ] = h1;
			Hash[ k + 3 ] = h0;
		}
	}
}

/**
 * An auxiliary function that returns an unsigned 64-bit value created out of
 * individual bytes in a buffer. This function is used to preserve endianness
 * of supplied 64-bit unsigned "initialization vector" values.
 *
 * @param Buf 8-byte buffer.
 */

inline uint64_t prvhash42_u64ec( const uint8_t* const Buf )
{
	return( (uint64_t) Buf[ 0 ] | (uint64_t) Buf[ 1 ] << 8 |
		(uint64_t) Buf[ 2 ] << 16 | (uint64_t) Buf[ 3 ] << 24 |
		(uint64_t) Buf[ 4 ] << 32 | (uint64_t) Buf[ 5 ] << 40 |
		(uint64_t) Buf[ 6 ] << 48 | (uint64_t) Buf[ 7 ] << 56 );
}

#endif // PRVHASH42EC_INCLUDED
