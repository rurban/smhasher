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

// PMP_Multilinear_naive.h: naive implementtation of a naive 64-bit version of PMP+-Multilinear hash family

#if !defined __PMP_MULTILINEAR_HASHER_NAIVE_64_H__
#define __PMP_MULTILINEAR_HASHER_NAIVE_64_H__

#include "PMP_Multilinear_common_naive.h"

// these values are used for printing pre-generated random values
#define PMPML_LEVELS_MAX 8
#define PMPML_CHUNK_SIZE_MAX 128

#define PMPML_DECLARE_PRIME_64( X ) \
	boost::multiprecision::uint256_t X = 1; \
	X <<= 64; \
	X += 13;

class PMP_Multilinear_Hasher_64_naive
{
  private:
  const random_data_for_PMPML_64* curr_rd;

  // calls to be done from LEVEL=0
  void hash_of_string_chunk( const uint64_t* coeff, ULARGE_INTEGER__XX constTerm, const uint64_t* x, ULARGELARGE_INTEGER__XX& retVal ) const
  {
	boost::multiprecision::uint256_t ret = constTerm.QuadPart;
	boost::multiprecision::uint256_t temp;
	for ( int i=0; i<PMPML_CHUNK_SIZE_64; i++ )
	{
	  temp = x[ i ];
	  temp = temp * coeff[ i ];
	  ret += temp;
	}
	PMPML_DECLARE_PRIME_64( prime )
	ret = ret % prime;
	retVal.LowPart = ret.template convert_to<uint64_t>();
	ret >>= 64;
	retVal.HighPart = ret.template convert_to<uint64_t>();
  }

  void hash_of_beginning_of_string_chunk( const uint64_t* coeff, ULARGE_INTEGER__XX constTerm, const uint64_t* x, std::size_t size, uint64_t xLast, ULARGELARGE_INTEGER__XX& retVal ) const
  {
	boost::multiprecision::uint256_t ret = constTerm.QuadPart;
	boost::multiprecision::uint256_t temp;
	for ( int i=0; i<size; i++ )
	{
	  temp = x[ i ];
	  temp = temp * coeff[ i ];
	  ret += temp;
	}

	temp = xLast;
	temp = temp * coeff[ size ];
	ret += temp;

	PMPML_DECLARE_PRIME_64( prime )
	ret = ret % prime;
	retVal.LowPart = ret.template convert_to<uint64_t>();
	ret >>= 64;
	retVal.HighPart = ret.template convert_to<uint64_t>();
  }

  // a call to be done from subsequent levels
  void hash_of_num_chunk( const uint64_t* coeff, ULARGE_INTEGER__XX constTerm, const ULARGELARGE_INTEGER__XX* x, ULARGELARGE_INTEGER__XX& retVal ) const
  {
	boost::multiprecision::uint256_t ret = constTerm.QuadPart;
	boost::multiprecision::uint256_t temp;
	for ( int i=0; i<PMPML_CHUNK_SIZE_64; i++ )
	{
	  temp = x[ i ].HighPart;
	  temp <<= 64;
	  temp += x[ i ].LowPart;
	  temp *= coeff[ i ];
	  ret += temp;
	}
	PMPML_DECLARE_PRIME_64( prime )
	ret = ret % prime;
	retVal.LowPart = ret.template convert_to<uint64_t>();
	ret >>= 64;
	retVal.HighPart = ret.template convert_to<uint64_t>();
  }

  void procesNextValue( int level, _ULARGELARGE_INTEGER__XX& value, _ULARGELARGE_INTEGER__XX * allValues, std::size_t * cnts, std::size_t& flag ) const
  {
	for ( int i=level;;i++ )
	{
		// NOTE: it's not necessary to check whether ( i < PMPML_LEVELS_64 ), 
		// if it is guaranteed that the string size is less than 1 << USHF_MACHINE_WORD_SIZE_BITS
		allValues[ ( i << PMPML_CHUNK_SIZE_LOG2_64 ) + cnts[ i ] ] = value;
		(cnts[ i ]) ++;
		if ( cnts[ i ] != PMPML_CHUNK_SIZE_64 )
			break;
		cnts[ i ] = 0;
		hash_of_num_chunk( curr_rd[ i ].random_coeff, *(ULARGE_INTEGER__XX*)(&(curr_rd[0].const_term)), allValues + ( i << PMPML_CHUNK_SIZE_LOG2_64 ), value );
		if ( ( flag & ( 1 << i ) ) == 0 )
		{
			cnts[ i + 1] = 0;
			flag |= 1 << i;
		}
	}
  }

  _ULARGELARGE_INTEGER__XX& finalize( int level, _ULARGELARGE_INTEGER__XX * allValues, std::size_t * cnts, std::size_t& flag ) const
  {
    ULARGELARGE_INTEGER__XX value;
	for ( int i=level;;i++ )
	{
		assert ( level != PMPML_LEVELS_64 );
		if ( ( ( flag & ( 1 << i ) ) == 0 ) && cnts[ i ] == 1 )
		{
			return allValues[ i << PMPML_CHUNK_SIZE_LOG2_64 ];
		}
		if ( cnts[ i ] )
		{
			for ( int j=cnts[ i ]; j<PMPML_CHUNK_SIZE_64; j++ )
			{
				( allValues + ( i << PMPML_CHUNK_SIZE_LOG2_64 ) )[ j ].LowPart = curr_rd[ i ].const_term;
				( allValues + ( i << PMPML_CHUNK_SIZE_LOG2_64 ) )[ j ].HighPart = 0;
			}
			if ( ( flag & ( 1 << i ) ) == 0 )
			{
				cnts[ i + 1] = 0;
				flag |= 1 << i;
			}
			hash_of_num_chunk( curr_rd[ i ].random_coeff,
												*(ULARGE_INTEGER__XX*)(&(curr_rd[i].const_term)),
												allValues + ( i << PMPML_CHUNK_SIZE_LOG2_64 ), value );
			procesNextValue( i + 1, 
							 value, 
							 allValues, cnts, flag );
		}
	}
  }

  public:
  uint64_t hash( const unsigned char* chars, std::size_t cnt ) const
  {
		_ULARGELARGE_INTEGER__XX allValues[ PMPML_LEVELS_64 * PMPML_CHUNK_SIZE_64 ];
		std::size_t cnts[ PMPML_LEVELS_64 ];
		std::size_t flag;
		cnts[ 1 ] = 0;
		flag = 0;

		std::size_t i;
		_ULARGELARGE_INTEGER__XX tmp_hash;
		// process full chunks
		for ( i=0; i<(cnt>>PMPML_CHUNK_SIZE_BYTES_LOG2_64); i++ )
		{
			hash_of_string_chunk( curr_rd[ 0 ].random_coeff, *(ULARGE_INTEGER__XX*)(&(curr_rd[0].const_term)), ((const uint64_t*)(chars)) + ( i << PMPML_CHUNK_SIZE_LOG2_64 ), tmp_hash );
			procesNextValue( 1, tmp_hash, allValues, cnts, flag );
		}
		// process remaining incomplete chunk(s)
		// note: if string size is a multiple of chunk size, we create a new chunk (1,0,0,...0),
		// so THIS PROCESSING IS ALWAYS PERFORMED
		std::size_t tailCnt = cnt & ( PMPML_CHUNK_SIZE_BYTES_64 - 1 );
		if ( tailCnt )
		{
			const unsigned char* tailnum = chars + ( (cnt>>PMPML_WORD_SIZE_BYTES_LOG2_64) << PMPML_WORD_SIZE_BYTES_LOG2_64 );
			const unsigned char* tailchunk = chars + ( (cnt>>PMPML_CHUNK_SIZE_BYTES_LOG2_64) << PMPML_CHUNK_SIZE_BYTES_LOG2_64 );
			int tailsize = cnt & ( PMPML_WORD_SIZE_BYTES_64 - 1 );
			uint64_t temp = 0;
			memcpy( &temp, tailnum, tailsize );
			((char*)(&temp))[tailsize] = 1;
			hash_of_beginning_of_string_chunk( curr_rd[0].random_coeff, *(ULARGE_INTEGER__XX*)(&(curr_rd[0].const_term)), (const uint64_t*)tailchunk, tailCnt >> PMPML_WORD_SIZE_BYTES_LOG2_64, temp, tmp_hash );
		}
		else
		{
			// BIG/LITTLE endian issue
			boost::multiprecision::uint256_t tempVal = curr_rd[0].const_term;
			boost::multiprecision::uint256_t tempVal2 = curr_rd[ 0 ].random_coeff[0];
			tempVal = tempVal + tempVal2;
			PMPML_DECLARE_PRIME_64( prime )
			tempVal = tempVal % prime;
			tmp_hash.LowPart = tempVal.template convert_to<uint64_t>();
			tempVal >>= 64;
			tmp_hash.HighPart = tempVal.template convert_to<uint64_t>();
		}
		procesNextValue( 1, tmp_hash, allValues, cnts, flag );
		tmp_hash = finalize( 1, allValues, cnts, flag );
		if ( tmp_hash.HighPart == 0 )
			return fmix64_short( tmp_hash.LowPart );
		else
			return tmp_hash.LowPart;
  }

  PMP_Multilinear_Hasher_64_naive()
  {
    curr_rd = rd_for_PMPML_64;
  }
  virtual ~PMP_Multilinear_Hasher_64_naive()
  {
    if ( curr_rd != NULL && curr_rd != rd_for_PMPML_64 )
		delete [] curr_rd;
  }

  //NOTE: no random stuff can be called by any of the functions above
  void randomize( UniformRandomNumberGenerator& rng )
  {
    random_data_for_PMPML_64 * temp_curr_rd = new random_data_for_PMPML_64[ PMPML_LEVELS_64 ];

	int i, j;
	for ( i=0; i<PMPML_LEVELS_64; i++ )
		for ( j=0; j<PMPML_CHUNK_SIZE_64; j++ )
		{
			do
			{
				temp_curr_rd[ i ].random_coeff[ j ] = rng.rand();
				temp_curr_rd[ i ].random_coeff[ j ] <<= 32;
				temp_curr_rd[ i ].random_coeff[ j ] |= rng.rand();
			}
			while ( !IS_VALID_COEFFICIENT_64( temp_curr_rd[ i ].random_coeff[ j ], i ) );
		}

	for ( i=0; i<PMPML_LEVELS_64; i++ )
	{
		uint64_t rv = rng.rand();
		rv <<= 32;
		rv += rng.rand();
		temp_curr_rd[ i ].const_term = rv;
	}

	if ( curr_rd == rd_for_PMPML_64 )
		curr_rd = temp_curr_rd;
	else
	{
		if ( curr_rd != NULL )
			delete [] curr_rd;
		curr_rd = temp_curr_rd;
	}
  }
};



#endif // __PMP_MULTILINEAR_HASHER_NAIVE_64_H__
