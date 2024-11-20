// Copyright (c) 2024, Ruan Kunliang.
// Use of this source code is governed by a BSD-style
// license that can be found in the LICENSE file.

// A variant of wyhash(https://github.com/wangyi-fudan/wyhash)
// It's optimized for short input, and faster than wyhash in such workflows.
// main repo: https://github.com/peterrk/w1hash

#pragma once
#ifndef W1HASH_H_
#define W1HASH_H_

#include <stdint.h>
#include <stddef.h>

static inline uint64_t _w1r1(const uint8_t* p) { return *p; }

#if defined(W1_LITTLE_ENDIAN_UNALIGNED_READ) \
	|| defined(__amd64__) || defined(__aarch64__) || defined(_M_AMD64) || defined(_M_ARM64)
#ifndef W1_PAGE_SIZE
#define W1_PAGE_SIZE 4096
#endif
static inline uint64_t _w1r2(const uint8_t* p) { return *(uint16_t*)p; }
static inline uint64_t _w1r4(const uint8_t* p) { return *(uint32_t*)p; }
static inline uint64_t _w1r8(const uint8_t* p) { return *(uint64_t*)p; }
static inline uint64_t _w1r3(const uint8_t* p) {
	if (((uintptr_t)p & (W1_PAGE_SIZE-1)) <= W1_PAGE_SIZE-4) {
		return _w1r4(p) & ((1ULL<<24)-1);
	}
	return _w1r2(p) | (_w1r1(p+2)<<16);
}
static inline uint64_t _w1r5(const uint8_t* p) {
	if (((uintptr_t)p & (W1_PAGE_SIZE-1)) <= W1_PAGE_SIZE-8) {
		return _w1r8(p) & ((1ULL<<40)-1);
	}
	return _w1r4(p) | (_w1r1(p+4)<<32);
}
static inline uint64_t _w1r6(const uint8_t* p) {
	if (((uintptr_t)p & (W1_PAGE_SIZE-1)) <= W1_PAGE_SIZE-8) {
		return _w1r8(p) & ((1ULL<<48)-1);
	}
	return _w1r4(p) | (_w1r2(p+4)<<32);
}
static inline uint64_t _w1r7(const uint8_t* p) {
	if (((uintptr_t)p & (W1_PAGE_SIZE-1)) <= W1_PAGE_SIZE-8) {
		return _w1r8(p) & ((1ULL<<56)-1);
	}
	return _w1r4(p) | (_w1r2(p+4)<<32) | (_w1r1(p+6)<<48);
}
#undef W1_PAGE_SIZE
#else
static inline uint64_t _w1r2(const uint8_t* p) { return _w1r1(p) | (_w1r1(p+1)<<8); }
static inline uint64_t _w1r3(const uint8_t* p) { return _w1r2(p) | (_w1r1(p+2)<<16); }
static inline uint64_t _w1r4(const uint8_t* p) { return _w1r2(p) | (_w1r2(p+2)<<16); }
static inline uint64_t _w1r5(const uint8_t* p) { return _w1r4(p) | (_w1r1(p+4)<<32); }
static inline uint64_t _w1r6(const uint8_t* p) { return _w1r4(p) | (_w1r2(p+4)<<32); }
static inline uint64_t _w1r7(const uint8_t* p) { return _w1r4(p) | (_w1r2(p+4)<<32) | (_w1r1(p+6)<<48); }
static inline uint64_t _w1r8(const uint8_t* p) { return _w1r4(p) | (_w1r4(p+4)<<32); }
#endif

typedef struct {
	uint64_t a;
	uint64_t b;
} _w1u128;

static inline _w1u128 _w1mum(uint64_t a, uint64_t b) {
	_w1u128 x;
#ifdef __SIZEOF_INT128__
	__uint128_t t = a;
	t *= b;
	x.a = t;
	x.b = t >> 64;
#elif defined(_MSC_VER) && defined(_M_X64)
	x.a =_umul128(a,b, &x.b);
#else
	uint64_t ax = a >> 32;
	uint64_t bx = b >> 32;
	a = (uint32_t)a;
	b = (uint32_t)b;
	uint64_t c = ax * b;
	uint64_t d = a * bx;
	_w1u128 x;
	x.a = a * b;
	x.b = ax * bx;
	uint64_t e = (x.a>>32) + (uint32_t)c + (uint32_t)d;
	x.a = (uint32_t)x.a | (e<<32);
	x.b += (c>>32) + (d>>32) + (e>>32);
#endif
	return x;
}

static inline uint64_t _w1mix(uint64_t a, uint64_t b) {
	_w1u128 t = _w1mum(a, b);
	return t.a ^ t.b;
}

static inline uint64_t w1hash_with_seed(const void* key, size_t len, uint64_t seed) {
	const uint64_t s0 = 0x2d358dccaa6c78a5ull;
	const uint64_t s1 = 0x8bb84b93962eacc9ull;
	const uint64_t s2 = 0x4b33a62ed433d4a3ull;
	const uint64_t s3 = 0x4d5a2da51de1aa47ull;

	seed ^= _w1mix(seed^s0, len^s1);

	_w1u128 t;
	const uint8_t* p = (const uint8_t*)key;
	size_t l = len;
_w1_tail:
	switch (l) {
		case 0: t.a = 0; t.b = 0; break;
		case 1: t.a = _w1r1(p); t.b = 0; break;
		case 2: t.a = _w1r2(p); t.b = 0; break;
		case 3: t.a = _w1r3(p); t.b = 0; break;
		case 4: t.a = _w1r4(p); t.b = 0; break;
		case 5: t.a = _w1r5(p); t.b = 0; break;
		case 6: t.a = _w1r6(p); t.b = 0; break;
		case 7: t.a = _w1r7(p); t.b = 0; break;
		case 8: t.a = _w1r8(p); t.b = 0; break;
		case 9:  t.a = _w1r8(p); t.b = _w1r1(p+8); break;
		case 10: t.a = _w1r8(p); t.b = _w1r2(p+8); break;
		case 11: t.a = _w1r8(p); t.b = _w1r3(p+8); break;
		case 12: t.a = _w1r8(p); t.b = _w1r4(p+8); break;
		case 13: t.a = _w1r8(p); t.b = _w1r5(p+8); break;
		case 14: t.a = _w1r8(p); t.b = _w1r6(p+8); break;
		case 15: t.a = _w1r8(p); t.b = _w1r7(p+8); break;
		case 16: t.a = _w1r8(p); t.b = _w1r8(p+8); break;
		default:
			if (l > 64) {
				uint64_t x = seed;
				uint64_t y = seed;
				uint64_t z = seed;
				do {
					seed = _w1mix(_w1r8(p)^s0, _w1r8(p+8)^seed);
					x = _w1mix(_w1r8(p+16)^s1, _w1r8(p+24)^x);
					y = _w1mix(_w1r8(p+32)^s2, _w1r8(p+40)^y);
					z = _w1mix(_w1r8(p+48)^s3, _w1r8(p+56)^z);
					p += 64;
					l -= 64;
				} while (l > 64);
				seed ^= x ^ y ^ z;
			}
			if (l > 32) {
				uint64_t x = seed;
				seed = _w1mix(_w1r8(p)^s0, _w1r8(p+8)^seed);
				x = _w1mix(_w1r8(p+16)^s1, _w1r8(p+24)^x);
				seed ^= x;
				p += 32;
				l -= 32;
			}
			if (l > 16) {
				seed = _w1mix(_w1r8(p)^s0, _w1r8(p+8)^seed);
				p += 16;
				l -= 16;
			}
			goto _w1_tail;
	}
	t = _w1mum(t.a^s1, t.b^seed);
	return _w1mix(t.a^(s0^len), t.b^s1);
}

static inline uint64_t w1hash(const void* key, size_t len) {
	return w1hash_with_seed(key, len, 0);
}

// short for w1hash(&x, 8)
static inline uint64_t w1hash64(uint64_t x) {
	const uint64_t s0 = 0x2d358dccaa6c78adull;
	const uint64_t s1 = 0x8bb84b93962eacc9ull;
	_w1u128 t = _w1mum(x^s1, 0x702daa6e740fb546ull);
	return _w1mix(t.a^s0, t.b^s1);
}

#endif