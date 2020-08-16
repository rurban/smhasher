/**
 * @file prvhash82.h
 *
 * @brief The inclusion file for the "prvhash82" hash function. Hash length
 * increment = 8.
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
 * @version 2.8
 */

//$ nocpp

#ifndef PRVHASH82_INCLUDED
#define PRVHASH82_INCLUDED

#include <stdint.h>
#include <string.h>

/**
 * PRVHASH hash function (128-bit variables with 64-bit hash word). Produces
 * hash of the specified Message.
 *
 * @param Message Message to produce hash from.
 * @param MessageLen Message length, in bytes.
 * @param[out] Hash The resulting hash. If both InitLCG and InitSeed are
 * non-zero, the hash will not be initially reset to 0, otherwise the hash
 * should be pre-initialized with random bytes.
 * @param HashLen The required hash length, in bytes, should be >= 8, in
 * increments of 8.
 * @param SeedXOR Optional value, to XOR the default seed with. To use the
 * default seed, set to 0. If both InitLCG and InitSeed are non-zero, this
 * SeedXOR is ignored and should be set to 0. Otherwise, the SeedXOR value
 * can have any bit length, and is used only as an additional entropy source.
 * @param InitLCG If both InitLCG and InitSeed are non-zero, both values
 * will be used as initial state of the hash function. Full 128-bit random
 * value should be supplied in this case. See the considerations below.
 * @param InitSeed If both InitLCG and InitSeed are non-zero, both values
 * will be used as initial state of the hash function. Full 128-bit random
 * value should be supplied in this case.
 */

inline void prvhash82( const uint8_t* const Message, const int MessageLen,
	uint8_t* const Hash, const int HashLen, const __uint128_t SeedXOR,
	const __uint128_t InitLCG, const __uint128_t InitSeed )
{
	__uint128_t lcg; // Multiplier inspired by LCG. This is not a prime
		// number. It is a random sequence of bits. This value can be
		// regenerated at will, possibly using various statistical search
		// methods. The best strategies: 1) Compose this number from 16-bit
		// random values that have 6 to 10 random bits set; 2) Use a 128-bit
		// random value that has 60-68 random bits set. An important
		// consideration here is to pass the 16-bit Sparse test by default.
	__uint128_t Seed; // Generated similarly to "lcg".

	if( InitLCG == 0 && InitSeed == 0 )
	{
		lcg = 15267459991392010589ULL;
		lcg <<= 64;
		lcg |= 14473286605592752231ULL;
		Seed = 7928988912013905173ULL;
		Seed <<= 64;
		Seed |= 1846177453121048234ULL;
		Seed ^= SeedXOR;

		memset( Hash, 0, HashLen );
	}
	else
	{
		lcg = InitLCG;
		Seed = InitSeed;
	}

	const int hl84 = ( HashLen >> 1 );
	const int mlext = MessageLen + ( MessageLen < 4 ? 4 - MessageLen : 0 );
	int c = mlext + hl84 + hl84 - mlext % hl84;
	int hpos = 0;
	int k;

	for( k = 0; k < c; k += 4 )
	{
		const uint64_t msg =
			( k < MessageLen ? (uint64_t) Message[ k ] : 0x100 ) |
			( k < MessageLen - 1 ? (uint64_t) Message[ k + 1 ] << 8 : 0x10000 ) |
			( k < MessageLen - 2 ? (uint64_t) Message[ k + 2 ] << 16 : 0x1000000 ) |
			( k < MessageLen - 3 ? (uint64_t) Message[ k + 3 ] << 24 : 0x100000000ULL );

		Seed *= lcg;
		uint64_t* const hc = (uint64_t*) &Hash[ hpos ];
		const uint64_t ph = *hc;
		*hc ^= (uint64_t) ( Seed >> 64 );
		Seed ^= ph ^ msg;
		lcg += Seed;

		hpos += 8;

		if( hpos == HashLen )
		{
			hpos = 0;
		}
	}
}

#endif // PRVHASH82_INCLUDED
