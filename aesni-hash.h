// Copyright (c) 2023, Ruan Kunliang.
// Use of this source code is governed by a BSD-style
// license that can be found in the LICENSE file.

#pragma once
#ifndef AESNI_HASH_H
#define AESNI_HASH_H

#include <stdint.h>
#include <immintrin.h>

#ifdef __cplusplus
extern "C" {
#endif

static inline __m128i AESNI_Hash128(const uint8_t* msg, unsigned len, uint32_t seed=0) {
	__m128i a = _mm_set1_epi32(seed);
	__m128i b = _mm_set1_epi32(len);
	__m128i c = _mm_set_epi32(0xdeadbeef, 0xffff0000, 0x01234567, 0x89abcdef);

	auto mix = [&a, &b, &c](__m128i x) {
		a = _mm_aesenc_si128(x, a);
		b = _mm_aesdec_si128(x, b);
		a = _mm_aesenc_si128(a, c);
		b = _mm_aesdec_si128(b, c);
	};

	while (len >= 16) {
		mix(_mm_lddqu_si128((const __m128i*)msg));
		msg += 16;
		len -= 16;
	}

	uint64_t x = 0;
	switch (len) {
		case 15:
			x |= ((uint64_t)msg[14]) << 48U;
		case 14:
			x |= ((uint64_t)msg[13]) << 40U;
		case 13:
			x |= ((uint64_t)msg[12]) << 32U;
		case 12:
			x |= *(const uint32_t*)(msg + 8);
			mix(_mm_set_epi64x(x, *(const uint64_t*)msg));
			break;
		case 11:
			x |= ((uint32_t)msg[10]) << 16U;
		case 10:
			x |= ((uint32_t)msg[9]) << 8U;
		case 9:
			x |= msg[8];
		case 8:
			mix(_mm_set_epi64x(x, *(const uint64_t*)msg));
			break;
		case 7:
			x |= ((uint64_t)msg[6]) << 48U;
		case 6:
			x |= ((uint64_t)msg[5]) << 40U;
		case 5:
			x |= ((uint64_t)msg[4]) << 32U;
		case 4:
			x |= *(const uint32_t*)msg;
			mix(_mm_set_epi64x(0, x));
			break;
		case 3:
			x |= ((uint32_t)msg[2]) << 16U;
		case 2:
			x |= ((uint32_t)msg[1]) << 8U;
		case 1:
			x |= msg[0];
			mix(_mm_set_epi64x(0, x));
			break;
		case 0:
			break;
	}
	return _mm_aesenc_si128(a, b);
}

static inline uint64_t AESNI_Hash64(const uint8_t* msg, unsigned len, uint32_t seed=0) {
	union {
		uint64_t x[2];
		__m128i v;
	} t;
	t.v = AESNI_Hash128(msg, len, seed);
	return t.x[0];
}

#ifdef __cplusplus
}
#endif

#endif