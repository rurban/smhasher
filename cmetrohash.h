// metrohash.h
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

#ifndef CMETROHASH_METROHASH_H
#define CMETROHASH_METROHASH_H

#pragma once

#if defined (__cplusplus)
extern "C" {
#endif

#include <stdint.h>
#include <string.h>

// MetroHash 64-bit hash functions
void cmetrohash64_1(const uint8_t * key, uint64_t len, uint32_t seed, uint8_t * out);
void cmetrohash64_2(const uint8_t * key, uint64_t len, uint32_t seed, uint8_t * out);


/* rotate right idiom recognized by compiler*/
inline static uint64_t crotate_right(uint64_t v, unsigned k)
{
    return (v >> k) | (v << (64 - k));
}

// unaligned reads, fast and safe on Nehalem and later microarchitectures
inline static uint64_t cread_u64(const void * const ptr)
{
    return * (uint64_t *) ptr;
}

inline static uint64_t cread_u32(const void * const ptr)
{
    return * (uint32_t *) ptr;
}

inline static uint64_t cread_u16(const void * const ptr)
{
    return * (uint16_t *) ptr;
}

inline static uint64_t cread_u8 (const void * const ptr)
{
    return * (uint8_t *) ptr;
}


#if defined (__cplusplus)
}
#endif
#endif // #ifndef CMETROHASH_METROHASH_H
