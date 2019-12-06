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

// PMP_Multilinear_test.h: interface of PMP+-Multilinear hash family with SMHasher

#if ! defined __MULTILINEARPRIMESTRINGHASHFUNCTOR_TEST_H__
#define __MULTILINEARPRIMESTRINGHASHFUNCTOR_TEST_H__


//-----------------------------------------------------------------------------
// Platform-specific functions and macros

#include <stdint.h>

void PMPML_32_CPP( const void * key, int len, uint32_t seed, void * res );
void PMPML_32_CPP_randomize();

#if defined(_WIN64) || defined(__x86_64__)
void PMPML_64_CPP( const void * key, int len, uint32_t seed, void * res );
void PMPML_64_CPP_randomize();

void PMPML_64_CPP_out_32( const void * key, int len, uint32_t seed, void * res );
void PMPML_64_CPP_out_32_randomize();
int PMPML_TestSpeedAlt_64_out_32( const void * key, int len, int iter );
#endif

#endif // __MULTILINEARPRIMESTRINGHASHFUNCTOR_TEST_H__
