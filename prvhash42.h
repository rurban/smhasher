/**
 * @file prvhash42.h
 *
 * @brief The inclusion file for the "prvhash42" hash function.
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
 * @version 2.9
 */

//$ nocpp

#ifndef PRVHASH42_INCLUDED
#define PRVHASH42_INCLUDED

#include <stdint.h>
#include <string.h>

/**
 * PRVHASH hash function (64-bit variables with 32-bit hash word). Produces
 * hash of the specified message. This function applies endianness correction
 * automatically (on little- and big-endian processors).
 *
 * @param Msg Message to produce hash from.
 * @param MsgLen Message length, in bytes.
 * @param[out] Hash The resulting hash. If both InitLCG and InitSeed are
 * non-zero, the hash will not be initially reset to 0, and it should be
 * pre-initialized with random bytes.
 * @param HashLen The required hash length, in bytes, should be >= 4, in
 * increments of 4.
 * @param SeedXOR Optional value, to XOR the default seed with. To use the
 * default seed, set to 0. If both InitLCG and InitSeed are non-zero, this
 * SeedXOR is ignored and should be set to 0. Otherwise, the SeedXOR value
 * can have any bit length, and is used only as an additional entropy source.
 * @param InitLCG If both InitLCG and InitSeed are non-zero, both values
 * will be used as initial state of the hash function. Full 64-bit random
 * value should be supplied in this case. See the considerations below.
 * @param InitSeed If both InitLCG and InitSeed are non-zero, both values
 * will be used as initial state of the hash function. Full 64-bit random
 * value should be supplied in this case.
 */

inline void prvhash42( const uint8_t* const Msg, const int MsgLen,
	uint8_t* const Hash, const int HashLen, const uint64_t SeedXOR,
	const uint64_t InitLCG, const uint64_t InitSeed )
{
	uint64_t lcg; // Multiplier inspired by LCG. This is not a prime number.
		// It is a random sequence of bits. This value can be regenerated at
		// will, possibly using various statistical search methods. The best
		// strategies: 1) Compose this number from 16-bit random values that
		// have 6 to 10 random bits set; 2) Use a 64-bit random value that has
		// 30-34 random bits set.
	uint64_t Seed; // Generated similarly to "lcg".

	if( InitLCG == 0 && InitSeed == 0 )
	{
		lcg = 1899840123779380929ULL;
		Seed = 3198392702252199417ULL ^ SeedXOR;
		memset( Hash, 0, HashLen );
	}
	else
	{
		lcg = InitLCG;
		Seed = InitSeed;
	}

	int e = 1;
	e = *(uint8_t*) &e;

	const int mlext = MsgLen + ( MsgLen < 4 ? 4 - MsgLen : 0 );
	int c = mlext + HashLen * 3 - mlext % HashLen;
	int hpos = 0;
	int k;

	for( k = 0; k < c; k += 4 )
	{
		uint64_t msgw;

		if( k < MsgLen - 3 )
		{
			msgw = ( e != 0 ? (uint64_t) *(uint32_t*) &Msg[ k ] :
				(uint64_t) Msg[ k ] |
				(uint64_t) Msg[ k + 1 ] << 8 |
				(uint64_t) Msg[ k + 2 ] << 16 |
				(uint64_t) Msg[ k + 3 ] << 24 );
		}
		else
		{
			msgw = (uint64_t) ( k < MsgLen ? Msg[ k ] : 0x100 ) |
				(uint64_t) ( k < MsgLen - 1 ? Msg[ k + 1 ] : 0x100 ) << 8 |
				(uint64_t) ( k < MsgLen - 2 ? Msg[ k + 2 ] : 0x100 ) << 16 |
				(uint64_t) ( k < MsgLen - 3 ? Msg[ k + 3 ] : 0x100 ) << 24;
		}

		Seed *= lcg;
		uint32_t* const hc = (uint32_t*) &Hash[ hpos ];
		const uint64_t ph = *hc;
		const uint64_t ient = Seed >> 32;
		*hc ^= (uint32_t) ient;
		Seed ^= ph ^ ient ^ msgw;
		lcg += Seed;

		hpos += 4;

		if( hpos == HashLen )
		{
			hpos = 0;
		}
	}

	if( e == 0 )
	{
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

#endif // PRVHASH42_INCLUDED
