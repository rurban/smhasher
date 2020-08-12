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
 * @version 2.0
 */

//$ nocpp

#ifndef PRVHASH42_INCLUDED
#define PRVHASH42_INCLUDED

#include <stdint.h>
#include <string.h>

/**
 * PRVHASH hash function (64-bit with 32-bit hash word). Produces hash of the
 * specified Message.
 *
 * @param Message Message to produce hash from.
 * @param MessageLen Message length, in bytes.
 * @param[out] Hash The resulting hash.
 * @param HashLen The required hash length, in bytes, should be >= 4, in
 * increments of 4.
 * @param SeedXOR Optional value, to XOR the default seed with. To use the
 * default seed, set to 0.
 * @param InitLCG For development purposes, should be set to 0. If != 0, "lcg"
 * value to use.
 * @param InitSeed For development purposes, should be set to 0. If != 0,
 * "Seed" value to use.
 */

inline void prvhash42( const uint8_t* const Message, const int MessageLen,
	uint8_t* const Hash, const int HashLen, const uint64_t SeedXOR,
	const uint64_t InitLCG, const uint64_t InitSeed )
{
	// Initialize the hash.

	memset( Hash, 0, HashLen );

	uint64_t lcg = ( InitLCG == 0 ? 15267459991392010589ULL : InitLCG );
		// Multiplier inspired by LCG. This is not a prime number. It is a
		// random sequence of bits. This value can be regenerated at will,
		// possibly using various statistical search methods. The best
		// strategies: 1) Compose both this and seed numbers of 8-bit values
		// that have 4 random bits set; 2) Compose the 64-bit value that has
		// 32 random bits set; same for seed. An important consideration here
		// is to pass the 16-bit Sparse test.

	uint64_t Seed = ( InitSeed == 0 ? 7928988912013905173ULL : InitSeed );
		// Generated similarly to "lcg".

	Seed ^= SeedXOR;
	int k;

	for( k = 0; k < MessageLen; k++ )
	{
		const uint64_t m = Message[ k ];

		Seed ^= m;
		int i;

		for( i = 0; i < HashLen; i += 4 )
		{
			Seed *= lcg;
			uint32_t* const hc = (uint32_t*) &Hash[ i ];
			const uint64_t ph = *hc;
			*hc ^= (uint32_t) ( Seed >> 32 );
			Seed ^= ph ^ m;
		}

		lcg += Seed;
	}
}

#endif // PRVHASH42_INCLUDED
