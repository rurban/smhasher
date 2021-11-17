/*
MIT License

Copyright (c) 2021 Keith-Cancel

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the “Software”), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED “AS IS”, WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#ifndef KEITHS_HASH_H
#define KEITHS_HASH_H

#include <stdint.h>

#define ROTR32(x,n) (((x) >> (n)) | ((x) << (32 - (n))))
#define ROTR64(x,n) (((x) >> (n)) | ((x) << (64 - (n))))

// Just initialize with the fractional part of sqrt(2)
#define khash64(input) khash64_fn(input, 0x6a09e667f3bcc908)
#define khash32(input) khash32_fn(input, 0x6a09e667, 0xf3bcc908)

static inline uint64_t khash64_fn(uint64_t input, uint64_t func) {
    uint64_t h = func;
    h ^= input - 7;
    h ^= ROTR64(h, 31);
    h -= ROTR64(h, 11);
    h -= ROTR64(h, 17);

    h ^= input - 13;
    h ^= ROTR64(h, 23);
    h += ROTR64(h, 31);
    h -= ROTR64(h, 13);

    h ^= input - 2;
    h -= ROTR64(h, 19);
    h += ROTR64(h, 5);
    h -= ROTR64(h, 31);
    return h;
}

static inline uint32_t khash32_fn(uint32_t input, uint32_t func1, uint32_t func2) {
	uint32_t h = input;
	h  = ROTR32(h, 16);
	h ^= func2;
	h -= 5;
	h  = ROTR32(h, 17);
	h += func1;
	h  = ROTR32(h, 1);

	h += ROTR32(h, 27);
	h ^= ROTR32(h, 3);
	h -= ROTR32(h, 17);
	h -= ROTR32(h, 27);

	h ^= input - 107;
	h -= ROTR32(h, 11);
	h ^= ROTR32(h, 7);
	h -= ROTR32(h, 5);
    return h;
}

#endif