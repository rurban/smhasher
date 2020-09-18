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

// PMP_Multilinear_test.cpp: interface of PMP+-Multilinear hash family with SMHasher

#include "Platform.h"

#include "PMP_Multilinear_test.h"
#include "PMP_Multilinear_common.h"

#include <stdlib.h>
class UniformRandomNumberGenerator_ : public UniformRandomNumberGenerator
{
public:
	uint32_t rand() { return (::rand() << 20) | (::rand() << 12) | ::rand(); }
};

static UniformRandomNumberGenerator_ rng;



#if !(defined __arm__ || defined __aarch64__)
#if (defined _WIN64) || (defined __x86_64__)

#include "PMP_Multilinear_64_out_32.h"


static PMP_Multilinear_Hasher_64_out_32 pmpml_hasher_64_out_32;

void PMPML_64_CPP_out_32( const void * key, int len, uint32_t seed, void * res )
{
  //pmpml_hasher_64_out_32.seed (seed);
	*(uint64_t*)res = pmpml_hasher_64_out_32.hash( (const unsigned char*)key, len );
}

void PMPML_64_CPP_out_32_randomize()
{
	pmpml_hasher_64_out_32.randomize( rng );
}

int PMPML_TestSpeedAlt_64_out_32( const void * key, int len, int iter )
{
	int dummy = 0;
	for ( int i=0; i<iter; i++ )
	{
		dummy += pmpml_hasher_64_out_32.hash( (unsigned char*)key, len );
	}
	return dummy;
}
#include "PMP_Multilinear_64.h"

static PMP_Multilinear_Hasher_64 pmpml_hasher_64;

void PMPML_64_CPP( const void * key, int len, uint32_t seed, void * res )
{
  //pmpml_hasher_64.seed (seed);
	*(uint64_t*)res = pmpml_hasher_64.hash( (const unsigned char*)key, len );
}

void PMPML_64_CPP_randomize()
{
	pmpml_hasher_64.randomize( rng );
}

int PMPML_TestSpeedAlt_64( const void * key, int len, int iter )
{
	int dummy = 0;
	for ( int i=0; i<iter; i++ )
	{
		dummy += pmpml_hasher_64.hash( (unsigned char*)key, len );
	}
	return dummy;
}

#endif
#endif // __arm__



#include "PMP_Multilinear.h"

static PMP_Multilinear_Hasher pmpml_hasher;
void PMPML_32_CPP( const void * key, int len, uint32_t seed, void * res )
{
  //pmpml_hasher.seed (seed);
	*(uint32_t*)res = pmpml_hasher.hash( (unsigned char*)key, len );
}

void PMPML_32_CPP_randomize()
{
	pmpml_hasher.randomize( rng );
}

int PMPML_TestSpeedAlt_32( const void * key, int len, int iter )
{
	int dummy = 0;
	for ( int i=0; i<iter; i++ )
	{
		dummy += pmpml_hasher.hash( (unsigned char*)key, len );
	}
	return dummy;
}

