// Original: metrohash64.cpp
// Port to C: Frank Denis (jedisct1@github) 
// Optimized variants: Paul G (paulie-g@github, paulg@perforge.net)
//
//
// The MIT License (MIT)
//
// Copyright (c) 2015 J. Andrew Rogers
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.
//

#ifndef OPT_CMETROHASH_METROHASH_H
#define OPT_CMETROHASH_METROHASH_H

#pragma once

#if defined (__cplusplus)
extern "C" {
#endif

/* 
 * Optimized variants of a c port of MetroHash 64-bit hash functions
 *
 * *_optshort: optimized for speed with relatively short keys at small expense to bulk hashing throughput
 * 
*/

void cmetrohash64_1_optshort(const uint8_t * key, uint64_t len, uint32_t seed, uint8_t * out);



#define BRANCH_MAXNO(x)			(sizeof x / sizeof x[0])
#define BRANCH_MAXINDEX(x)		(BRANCH_MAXNO(x) - 1)
#define BRANCH_THRESHHOLD(x)	(BRANCH_MAXNO(x) - 2)

#if defined (__cplusplus)
}
#endif
#endif // #ifndef OPT_CMETROHASH_METROHASH_H
