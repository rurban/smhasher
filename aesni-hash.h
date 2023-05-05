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

static inline __m128i AESNI_Hash128(const uint8_t* msg, unsigned len, uint32_t seed = 0) {
	auto a = _mm_set1_epi32(seed);
	auto b = _mm_set1_epi32(len);
	auto m = _mm_set_epi32(0xdeadbeef, 0xffff0000, 0x01234567, 0x89abcdef);
	auto s = _mm_set_epi8(3, 7, 11, 15, 2, 6, 10, 14, 1, 5, 9, 13, 0, 4, 8, 12);

	if (len > 80) {
		auto c = _mm_aesenc_si128(a, m);
		auto d = _mm_aesdec_si128(b, m);
		do {
			a = _mm_aesenc_si128(_mm_xor_si128(a, _mm_lddqu_si128((const __m128i*)msg)), m);
			b = _mm_aesdec_si128(_mm_xor_si128(b, _mm_lddqu_si128((const __m128i*)(msg + 16))), m);
			c = _mm_aesenc_si128(_mm_xor_si128(c, _mm_lddqu_si128((const __m128i*)(msg + 32))), m);
			d = _mm_aesdec_si128(_mm_xor_si128(d, _mm_lddqu_si128((const __m128i*)(msg + 48))), m);
			a = _mm_shuffle_epi8(a, s);
			b = _mm_shuffle_epi8(b, s);
			c = _mm_shuffle_epi8(c, s);
			d = _mm_shuffle_epi8(d, s);
			msg += 64;
			len -= 64;
		} while (len > 80);
		c = _mm_aesenc_si128(a, c);
		d = _mm_aesdec_si128(b, d);
		a = _mm_aesenc_si128(c, d);
		b = _mm_aesdec_si128(d, c);
	}

	auto mix = [&a, &b, m, s](__m128i x) {
		a = _mm_aesenc_si128(x, a);
		a = _mm_aesenc_si128(a, m);
		b = _mm_shuffle_epi8(_mm_xor_si128(x, b), s);
		b = _mm_shuffle_epi8(_mm_aesdec_si128(b, m), s);
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
		case 0:
			break;
	}
	return _mm_aesenc_si128(a, b);
}

static inline uint64_t AESNI_Hash64(const uint8_t* msg, unsigned len, uint32_t seed = 0) {
	union {
		uint64_t x[2];
		__m128i v;
	} t;
	t.v = AESNI_Hash128(msg, len, seed);
	return t.x[0];
}

#ifdef __cplusplus
} // C
#endif

#endif // AESNI_HASH_H