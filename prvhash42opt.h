/**
 * @file prvhash42opt.h
 *
 * @brief The inclusion file for the "prvhash42" hash function, optimized for
 * 32-bit hash length, without endianness-correction.
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
 * @version 2.5
 */

//$ nocpp

#ifndef PRVHASH42OPT_INCLUDED
#define PRVHASH42OPT_INCLUDED

#include <stdint.h>

/**
 * Optimized PRVHASH hash function. Produces 32-bit hash of the specified
 * Message using default initial "Hash", "lcg" and "Seed" values.
 *
 * @param Message Message to produce hash from.
 * @param MessageLen Message length, in bytes.
 * @param[out] Hash The resulting hash, 32-bit, not endianness-corrected.
 * @param SeedXOR Optional value, to XOR the default seed with. To use the
 * default seed, set to 0.
 */

inline void prvhash42_32( const uint8_t* const Message, const int MessageLen,
	uint8_t* const Hash0, const uint64_t SeedXOR )
{
	uint64_t Hash = 0;
	uint64_t lcg = 15267459991392010589ULL;
	uint64_t Seed = 7928988912013905173ULL ^ SeedXOR;

	int k;

	for( k = 0; k < MessageLen; k++ )
	{
		const uint64_t msg = Message[ k ];

		Seed *= lcg;
		const uint64_t ph = Hash;
		Hash ^= Seed >> 32;
		Seed ^= ph ^ msg;
		lcg += Seed;
	}

	const uint64_t lmsg = ( MessageLen == 0 ? 0 : ~Message[ MessageLen - 1 ]);

	Seed *= lcg;
	const uint64_t ph = Hash;
	Hash ^= Seed >> 32;
	Seed ^= ph ^ lmsg;
	lcg += Seed;

	Seed *= lcg;
	Hash ^= Seed >> 32;

	*(uint32_t*) Hash0 = (uint32_t) Hash;
}

#endif // PRVHASH42OPT_INCLUDED
