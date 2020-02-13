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

// PMP_Multilinear_naive.h: naive implementtation of a naive 32-bit version of PMP+-Multilinear hash family

#if !defined __PMP_MULTILINEAR_HASHER_NAIVE_H__
#define __PMP_MULTILINEAR_HASHER_NAIVE_H__

#include "PMP_Multilinear_common_naive.h"

// these values are used for printing pre-generated random values
#define PMPML_LEVELS_MAX 8
#define PMPML_CHUNK_SIZE_MAX 128



class PMP_Multilinear_Hasher_Naive
{
  private:
  const random_data_for_MPSHF* curr_rd;

  // calls to be done from LEVEL=0
  uint64_t hash_of_string_chunk( const uint32_t* coeff, uint64_t constTerm, const uint32_t* x ) const
  {
	int128_t ret = constTerm;
	int128_t temp;
	for ( int i=0; i<PMPML_CHUNK_SIZE; i++ )
	{
	  temp = x[ i ];
	  temp = temp * coeff[ i ];
	  ret += temp;
	}
	ret = ret % PMPML_MAIN_PRIME;
	return ret.template convert_to<uint64_t>();
  }

  uint64_t hash_of_beginning_of_string_chunk( const uint32_t* coeff, uint64_t constTerm, const uint32_t* x, uint32_t size ) const
  {
	int128_t ret = constTerm;
	int128_t temp;
	for ( int i=0; i<size; i++ )
	{
	  temp = x[ i ];
	  temp = temp * coeff[ i ];
	  ret += temp;
	}
	ret = ret % PMPML_MAIN_PRIME;
	return ret.template convert_to<uint64_t>();
  }

  uint64_t updated_string_chunk_hash_withElement32( const uint32_t coeff, uint32_t x, uint64_t iniHashVal ) const
  {
	int128_t ret = x;
	ret = ret * coeff;
	ret = ret + iniHashVal;
	ret = ret % PMPML_MAIN_PRIME;
	return ret.template convert_to<uint64_t>();
  }

  // a call to be done from subsequent levels
  uint64_t hash_of_num_chunk( const uint32_t* coeff, uint64_t constTerm, const uint64_t* x ) const
  {
	int128_t ret = constTerm;
	int128_t temp;
	for ( int i=0; i<PMPML_CHUNK_SIZE; i++ )
	{
	  temp = x[ i ];
	  temp *= coeff[ i ];
	  ret += temp;
	}
	ret = ret % PMPML_MAIN_PRIME;
	return ret.template convert_to<uint64_t>();
  }

  void procesNextValue( int level, uint64_t value, uint64_t * allValues, uint32_t * cnts, uint32_t& flag ) const
  {
	for ( int i=level;;i++ )
	{
		// NOTE: it's not necessary to check whether ( i < PMPML_LEVELS ), 
		// if it is guaranteed that the string size is less than 1 << USHF_MACHINE_WORD_SIZE_BITS
		allValues[ ( i << PMPML_CHUNK_SIZE_LOG2 ) + cnts[ i ] ] = value;
		(cnts[ i ]) ++;
		if ( cnts[ i ] != PMPML_CHUNK_SIZE )
			break;
		cnts[ i ] = 0;
		value = hash_of_num_chunk( curr_rd[ i ].random_coeff, curr_rd[ i ].const_term, allValues + ( i << PMPML_CHUNK_SIZE_LOG2 ) );
		if ( ( flag & ( 1 << i ) ) == 0 )
		{
			cnts[ i + 1] = 0;
			flag |= 1 << i;
		}
	}
  }

  uint64_t finalize( int level, uint64_t * allValues, uint32_t * cnts, uint32_t& flag ) const
  {
	for ( int i=level;;i++ )
	{
//		ASSERT ( level != PMPML_LEVELS )
		if ( ( ( flag & ( 1 << i ) ) == 0 ) && cnts[ i ] == 1 )
		{
			return allValues[ i << PMPML_CHUNK_SIZE_LOG2 ];
		}
		if ( cnts[ i ] )
		{
			for ( int j=cnts[ i ]; j<PMPML_CHUNK_SIZE; j++ )
				( allValues + ( i << PMPML_CHUNK_SIZE_LOG2 ) )[ j ] = curr_rd[ i - 1 ].const_term;
			if ( ( flag & ( 1 << i ) ) == 0 )
			{
				cnts[ i + 1] = 0;
				flag |= 1 << i;
			}
			procesNextValue( i + 1, 
							 hash_of_num_chunk( curr_rd[ i ].random_coeff,
												curr_rd[ i ].const_term,
												allValues + ( i << PMPML_CHUNK_SIZE_LOG2 ) ), 
							 allValues, cnts, flag );
		}
	}
  }

  public:
  uint32_t hash( const unsigned char* chars, uint32_t cnt ) const
  {
		uint64_t allValues[ PMPML_LEVELS * PMPML_CHUNK_SIZE ];
		uint32_t cnts[ PMPML_LEVELS ];
		uint32_t flag;
		cnts[ 1 ] = 0;
		flag = 0;

		uint32_t i;
		uint64_t tmp_hash;
		// process full chunks
		for ( i=0; i<(cnt>>PMPML_CHUNK_SIZE_BYTES_LOG2); i++ )
		{
			tmp_hash = hash_of_string_chunk( curr_rd[ 0 ].random_coeff, curr_rd[ 0 ].const_term, ((const uint32_t*)(chars)) + ( i << PMPML_CHUNK_SIZE_LOG2 ) );
			procesNextValue( 1, tmp_hash, allValues, cnts, flag );
		}
		// process remaining incomplete chunk(s)
		// note: if string size is a multiple of chunk size, we create a new chunk (1,0,0,...0),
		// so THIS PROCESSING IS ALWAYS PERFORMED
		uint32_t tailCnt = cnt & ( PMPML_CHUNK_SIZE_BYTES - 1 );
		if ( tailCnt )
		{
			const unsigned char* tail = chars + ( (cnt>>PMPML_CHUNK_SIZE_BYTES_LOG2) << PMPML_CHUNK_SIZE_BYTES_LOG2 );
			tmp_hash = hash_of_beginning_of_string_chunk( curr_rd[0].random_coeff, curr_rd[0].const_term, (const uint32_t*)tail, tailCnt >> PMPML_WORD_SIZE_BYTES_LOG2 );
			static const uint32_t masks[ 4 ] = { 0x0, 0xFF, 0xFFFF, 0xFFFFFF };
			static const uint32_t ones[ 4 ] = { 0x1, 0x100, 0x10000, 0x1000000 };
			uint32_t temp = *( ((const uint32_t*)tail) + ( tailCnt >> PMPML_WORD_SIZE_BYTES_LOG2 ) );
			temp &= masks[ tailCnt & ( PMPML_WORD_SIZE_BYTES - 1 ) ];
			temp |= ones[ tailCnt & ( PMPML_WORD_SIZE_BYTES - 1 ) ];
			tmp_hash = updated_string_chunk_hash_withElement32( curr_rd[ 0 ].random_coeff[ tailCnt >> PMPML_WORD_SIZE_BYTES_LOG2 ], temp, tmp_hash );
		}
		else
		{
			// BIG/LITTLE endian issue
			tmp_hash = ( curr_rd[ 0 ].random_coeff[0] + curr_rd[0].const_term ) % PMPML_MAIN_PRIME;
		}
		procesNextValue( 1, tmp_hash, allValues, cnts, flag );
		tmp_hash = finalize( 1, allValues, cnts, flag );
		if ( ( tmp_hash >> 32 ) == 0 )
			return fmix32_short( (uint32_t)tmp_hash );
		else
			return (uint32_t)tmp_hash;
  }

  PMP_Multilinear_Hasher_Naive()
  {
    curr_rd = rd_for_MPSHF;
  }
  virtual ~PMP_Multilinear_Hasher_Naive()
  {
    if ( curr_rd != NULL && curr_rd != rd_for_MPSHF )
		delete [] curr_rd;
  }

  //NOTE: no random stuff can be called by any of the functions above
  void randomize( UniformRandomNumberGenerator& rng )
  {
    random_data_for_MPSHF * temp_curr_rd = new random_data_for_MPSHF[ PMPML_LEVELS ];

	int i, j;
	for ( i=0; i<PMPML_LEVELS; i++ )
		for ( j=0; j<PMPML_CHUNK_SIZE; j++ )
		{
			do
			{
				temp_curr_rd[ i ].random_coeff[ j ] = rng.rand();
			}
			while ( !IS_VALID_COEFFICIENT( temp_curr_rd[ i ].random_coeff[ j ], i ) );
		}

	for ( i=0; i<PMPML_LEVELS; i++ )
	{
		uint64_t rv;
		do
		{
			rv = rng.rand();
			rv <<= 32;
			rv |= rng.rand();
		}
		while ( rv == 0 );
		rv = rv % PMPML_MAIN_PRIME;
		temp_curr_rd[ i ].const_term = rv;
	}

	if ( curr_rd == rd_for_MPSHF )
		curr_rd = temp_curr_rd;
	else
	{
		if ( curr_rd != NULL )
			delete [] curr_rd;
		curr_rd = temp_curr_rd;
	}
  }
};



#endif // __PMP_MULTILINEAR_HASHER_NAIVE_H__
