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
 * @version 2.15
 */

//$ nocpp

#ifndef PRVHASH42_INCLUDED
#define PRVHASH42_INCLUDED

#include <string.h>
#include "prvhash42ec.h"

/**
 * PRVHASH hash function (64-bit variables with 32-bit hash word). Produces
 * hash of the specified message. This function applies endianness correction
 * automatically (on little- and big-endian processors).
 *
 * @param Msg The message to produce hash from. The alignment of the message
 * is unimportant.
 * @param MsgLen Message's length, in bytes.
 * @param[in,out] Hash The resulting hash. If InitVec is non-NULL, the hash
 * will not be initially reset to 0, and it should be pre-initialized with
 * uniformly random bytes (it will be automatically endianness-corrected). On
 * systems where this is relevant, this address should be aligned to 32 bits.
 * @param HashLen The required hash length, in bytes, should be >= 4, in
 * increments of 4.
 * @param SeedXOR Optional value, to XOR the default seed with. To use the
 * default seed, set to 0. If InitVec is non-NULL, this SeedXOR is ignored and
 * should be set to 0. Otherwise, the SeedXOR value can have any bit length,
 * and is used only as an additional entropy source. It should be
 * endianness-corrected.
 * @param InitVec If non-NULL, an "initialization vector" for the internal
 * "lcg" and "Seed" variables. Full 16-byte uniformly random value should be
 * supplied in this case.
 */

inline void prvhash42( const uint8_t* const Msg, const int MsgLen,
	uint8_t* const Hash, const int HashLen, const uint64_t SeedXOR,
	const uint8_t InitVec[ 16 ])
{
	uint64_t lcg;
	uint64_t Seed;

	if( InitVec == 0 )
	{
		memset( Hash, 0, HashLen );

		lcg = 15430973964284598734ULL;
		Seed = 1691555508060032701ULL ^ SeedXOR;
	}
	else
	{
		prvhash42_ec( Hash, HashLen );

		lcg = prvhash42_u64ec( InitVec );
		Seed = prvhash42_u64ec( InitVec + 8 );
	}

	const uint8_t lb = (uint8_t) ( MsgLen > 0 ? ~Msg[ MsgLen - 1 ] : 0xFF );
	const int mlext = MsgLen + 4;
	const int c = mlext + HashLen + ( HashLen - mlext % HashLen );
	int hpos = 0;
	int k;

	for( k = 0; k < c; k += 4 )
	{
		uint64_t msgw;

		if( k < MsgLen - 3 )
		{
			msgw = prvhash42_u32ec( Msg + k );
		}
		else
		{
			msgw = (uint32_t) ( k < MsgLen ? Msg[ k ] : lb ) |
				(uint32_t) ( k < MsgLen - 1 ? Msg[ k + 1 ] : lb ) << 8 |
				(uint32_t) ( k < MsgLen - 2 ? Msg[ k + 2 ] : lb ) << 16 |
				(uint32_t) lb << 24;
		}

		Seed *= lcg;
		uint32_t* const hc = (uint32_t*) &Hash[ hpos ];
		const uint64_t ph = *hc ^ ( Seed >> 32 );
		Seed ^= ph ^ msgw;
		*hc = (uint32_t) ph;
		lcg += Seed;

		hpos += 4;

		if( hpos == HashLen )
		{
			hpos = 0;
		}
	}

	prvhash42_ec( Hash, HashLen );
}

#endif // PRVHASH42_INCLUDED
