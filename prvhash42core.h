/**
 * prvhash42core.h version 2.30
 *
 * The inclusion file for the "prvhash42_core64", "prvhash42_core32",
 * "prvhash42_core16", "prvhash42_core8", "prvhash42_core4",
 * "prvhash42_core2" PRVHASH core functions for various state-variable sizes
 * and hash-word sizes.
 *
 * Description is available at https://github.com/avaneev/prvhash
 *
 * License
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
 */

#ifndef PRVHASH42CORE_INCLUDED
#define PRVHASH42CORE_INCLUDED

#include <stdint.h>

/**
 * This function runs a single PRVHASH random number generation round. This
 * function can be used both as a hash generator and as a general-purpose
 * random number generator. In the latter case, it is advisable to initially
 * run this function at least 4 times before using its random output, to
 * neutralize any possible oddities of "Seed"'s and "lcg"'s initial values.
 *
 * To generate hashes, the "lcg" variable should be XORed with entropy input
 * prior to calling this function.
 *
 * @param[in,out] Seed0 The current "Seed" value. Can be initialized to any
 * value.
 * @param[in,out] lcg0 The current "lcg" value. Can be initialized to any
 * value.
 * @param[in,out] Hash0 Current hash word in a hash word array.
 * @return Current random value.
 */

inline uint32_t prvhash42_core64( uint64_t* const Seed0, uint64_t* const lcg0,
	uint32_t* const Hash0 )
{
	uint64_t Seed = *Seed0; uint64_t lcg = *lcg0; uint32_t Hash = *Hash0;

	const uint64_t xr = ~lcg;
	Seed += lcg;
	Seed *= lcg - xr;
	lcg += ~Seed;
	const uint64_t hs = Seed >> 32ULL;
	const uint32_t out = (uint32_t) ( Seed ^ hs );
	const uint64_t ph = Hash ^ hs;
	Seed ^= ph;
	Hash = (uint32_t) ph;

	*Seed0 = Seed; *lcg0 = lcg; *Hash0 = Hash;

	return( out );
}

inline uint16_t prvhash42_core32( uint32_t* const Seed0, uint32_t* const lcg0,
	uint16_t* const Hash0 )
{
	uint32_t Seed = *Seed0; uint32_t lcg = *lcg0; uint16_t Hash = *Hash0;

	const uint32_t xr = ~lcg;
	Seed += lcg;
	Seed *= lcg - xr;
	lcg += ~Seed;
	const uint32_t hs = Seed >> 16;
	const uint16_t out = (uint16_t) ( Seed ^ hs );
	const uint32_t ph = Hash ^ hs;
	Seed ^= ph;
	Hash = (uint16_t) ph;

	*Seed0 = Seed; *lcg0 = lcg; *Hash0 = Hash;

	return( out );
}

inline uint8_t prvhash42_core16( uint16_t* const Seed0, uint16_t* const lcg0,
	uint8_t* const Hash0 )
{
	uint16_t Seed = *Seed0; uint16_t lcg = *lcg0; uint8_t Hash = *Hash0;

	const uint16_t xr = ~lcg;
	Seed += lcg;
	Seed *= lcg - xr;
	lcg += ~Seed;
	const uint16_t hs = Seed >> 8;
	const uint8_t out = (uint8_t) ( Seed ^ hs );
	const uint16_t ph = Hash ^ hs;
	Seed ^= ph;
	Hash = (uint8_t) ph;

	*Seed0 = Seed; *lcg0 = lcg; *Hash0 = Hash;

	return( out );
}

inline uint8_t prvhash42_core8( uint8_t* const Seed0, uint8_t* const lcg0,
	uint8_t* const Hash0 )
{
	uint8_t Seed = *Seed0; uint8_t lcg = *lcg0; uint8_t Hash = *Hash0;

	const uint8_t xr = ~lcg;
	Seed += lcg;
	Seed *= lcg - xr;
	lcg += ~Seed;
	const uint8_t hs = (uint8_t) ( Seed >> 4 );
	const uint8_t out = ( Seed ^ hs ) & 15;
	const uint8_t ph = ( Hash ^ hs ) & 15;
	Seed ^= ph;
	Hash = ph;

	*Seed0 = Seed; *lcg0 = lcg; *Hash0 = Hash;

	return( out );
}

inline uint8_t prvhash42_core4( uint8_t* const Seed0, uint8_t* const lcg0,
	uint8_t* const Hash0 )
{
	uint8_t Seed = *Seed0; uint8_t lcg = *lcg0; uint8_t Hash = *Hash0;

	const uint8_t xr = ~lcg & 15;
	Seed += lcg;
	Seed &= 15;
	Seed *= ( lcg - xr ) & 15;
	Seed &= 15;
	lcg += ~Seed;
	lcg &= 15;
	const uint8_t hs = (uint8_t) ( Seed >> 2 );
	const uint8_t out = ( Seed ^ hs ) & 3;
	const uint8_t ph = ( Hash ^ hs ) & 3;
	Seed ^= ph;
	Hash = ph;

	*Seed0 = Seed; *lcg0 = lcg; *Hash0 = Hash;

	return( out );
}

inline uint8_t prvhash42_core2( uint8_t* const Seed0, uint8_t* const lcg0,
	uint8_t* const Hash0 )
{
	uint8_t Seed = *Seed0; uint8_t lcg = *lcg0; uint8_t Hash = *Hash0;

	const uint8_t xr = ~lcg & 3;
	Seed += lcg;
	Seed &= 3;
	Seed *= ( lcg - xr ) & 3;
	Seed &= 3;
	lcg += ~Seed;
	lcg &= 3;
	const uint8_t hs = (uint8_t) ( Seed >> 1 );
	const uint8_t out = ( Seed ^ hs ) & 1;
	const uint8_t ph = ( Hash ^ hs ) & 1;
	Seed ^= ph;
	Hash = ph;

	*Seed0 = Seed; *lcg0 = lcg; *Hash0 = Hash;

	return( out );
}

#endif // PRVHASH42CORE_INCLUDED
