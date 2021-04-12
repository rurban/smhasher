#include "umash.h"

#if defined(__aarch64__)
#include "sse2neon.h"
#else
/* The PH block reduction code is x86-only for now. */
#include <immintrin.h>
#endif
#include <string.h>

/*
 * UMASH is distributed under the MIT license.
 *
 * SPDX-License-Identifier: MIT
 *
 * Copyright 2020 Backtrace I/O, Inc.
 *
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation
 * files (the "Software"), to deal in the Software without
 * restriction, including without limitation the rights to use, copy,
 * modify, merge, publish, distribute, sublicense, and/or sell copies
 * of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS
 * BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
 * ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

/*
 * Defining UMASH_TEST_ONLY switches to a debug build with internal
 * symbols exposed.
 */
#ifdef UMASH_TEST_ONLY
#define TEST_DEF
#include "t/umash_test_only.h"
#else
#define TEST_DEF static
#endif

#ifdef __GNUC__
#define LIKELY(X) __builtin_expect(!!(X), 1)
#define UNLIKELY(X) __builtin_expect(!!(X), 0)
#else
#define LIKELY(X) X
#define UNLIKELY(X) X
#endif

#define ARRAY_SIZE(ARR) (sizeof(ARR) / sizeof(ARR[0]))

#define BLOCK_SIZE (sizeof(uint64_t) * UMASH_PH_PARAM_COUNT)

/* Incremental UMASH consumes 16 bytes at a time. */
#define INCREMENTAL_GRANULARITY 16

/**
 * Modular arithmetic utilities.
 *
 * The code below uses GCC extensions.  It should be possible to add
 * support for other compilers.
 */
TEST_DEF inline uint64_t
add_mod_fast(uint64_t x, uint64_t y)
{
	unsigned long long sum;

	/* If `sum` overflows, `sum + 8` does not. */
	return (__builtin_uaddll_overflow(x, y, &sum) ? sum + 8 : sum);
}

static uint64_t
add_mod_slow_slow_path(uint64_t sum, uint64_t fixup)
{
	/* Reduce sum, mod 2**64 - 8. */
	sum = (sum >= (uint64_t)-8) ? sum + 8 : sum;
	/* sum < 2**64 - 8, so this doesn't overflow. */
	sum += fixup;
	/* Reduce again. */
	sum = (sum >= (uint64_t)-8) ? sum + 8 : sum;
	return sum;
}

TEST_DEF inline uint64_t
add_mod_slow(uint64_t x, uint64_t y)
{
	unsigned long long sum;
	uint64_t fixup = 0;

	/* x + y \equiv sum + fixup */
	if (__builtin_uaddll_overflow(x, y, &sum))
		fixup = 8;

	/*
	 * We must ensure `sum + fixup < 2**64 - 8`.
	 *
	 * We want a conditional branch here, but not in the
	 * overflowing add: overflows happen roughly half the time on
	 * pseudorandom inputs, but `sum < 2**64 - 16` is almost
	 * always true, for pseudorandom `sum`.
	 */
	if (LIKELY(sum < (uint64_t)-16))
		return sum + fixup;

	return add_mod_slow_slow_path(sum, fixup);
}

TEST_DEF inline uint64_t
mul_mod_fast(uint64_t m, uint64_t x)
{
	__uint128_t product = m;

	product *= x;
	return add_mod_fast((uint64_t)product, 8 * (uint64_t)(product >> 64));
}

TEST_DEF inline uint64_t
horner_double_update(uint64_t acc, uint64_t m0, uint64_t m1, uint64_t x, uint64_t y)
{

	acc = add_mod_fast(acc, x);
	return add_mod_slow(mul_mod_fast(m0, acc), mul_mod_fast(m1, y));
}

/**
 * Salsa20 stream generator, used to derive struct umash_param.
 *
 * Slightly prettified version of D. J. Bernstein's public domain NaCL
 * (version 20110121), without paying any attention to constant time
 * execution or any other side-channel.
 */
static inline uint32_t
rotate(uint32_t u, int c)
{

	return (u << c) | (u >> (32 - c));
}

static uint32_t
load_littleendian(const void *buf)
{
	uint32_t ret = 0;
	uint8_t x[4];

	memcpy(x, buf, sizeof(x));
	for (size_t i = 0; i < 4; i++)
		ret |= (uint32_t)x[i] << (8 * i);

	return ret;
}

static void
store_littleendian(void *dst, uint32_t u)
{

	for (size_t i = 0; i < 4; i++) {
		uint8_t lo = u;

		memcpy(dst, &lo, 1);
		u >>= 8;
		dst = (char *)dst + 1;
	}

	return;
}

static void
core_salsa20(char *out, const uint8_t in[static 16], const uint8_t key[static 32],
    const uint8_t constant[16])
{
	enum { ROUNDS = 20 };
	uint32_t x0, x1, x2, x3, x4, x5, x6, x7, x8, x9, x10, x11, x12, x13, x14, x15;
	uint32_t j0, j1, j2, j3, j4, j5, j6, j7, j8, j9, j10, j11, j12, j13, j14, j15;

	j0 = x0 = load_littleendian(constant + 0);
	j1 = x1 = load_littleendian(key + 0);
	j2 = x2 = load_littleendian(key + 4);
	j3 = x3 = load_littleendian(key + 8);
	j4 = x4 = load_littleendian(key + 12);
	j5 = x5 = load_littleendian(constant + 4);
	j6 = x6 = load_littleendian(in + 0);
	j7 = x7 = load_littleendian(in + 4);
	j8 = x8 = load_littleendian(in + 8);
	j9 = x9 = load_littleendian(in + 12);
	j10 = x10 = load_littleendian(constant + 8);
	j11 = x11 = load_littleendian(key + 16);
	j12 = x12 = load_littleendian(key + 20);
	j13 = x13 = load_littleendian(key + 24);
	j14 = x14 = load_littleendian(key + 28);
	j15 = x15 = load_littleendian(constant + 12);

	for (size_t i = 0; i < ROUNDS; i += 2) {
		x4 ^= rotate(x0 + x12, 7);
		x8 ^= rotate(x4 + x0, 9);
		x12 ^= rotate(x8 + x4, 13);
		x0 ^= rotate(x12 + x8, 18);
		x9 ^= rotate(x5 + x1, 7);
		x13 ^= rotate(x9 + x5, 9);
		x1 ^= rotate(x13 + x9, 13);
		x5 ^= rotate(x1 + x13, 18);
		x14 ^= rotate(x10 + x6, 7);
		x2 ^= rotate(x14 + x10, 9);
		x6 ^= rotate(x2 + x14, 13);
		x10 ^= rotate(x6 + x2, 18);
		x3 ^= rotate(x15 + x11, 7);
		x7 ^= rotate(x3 + x15, 9);
		x11 ^= rotate(x7 + x3, 13);
		x15 ^= rotate(x11 + x7, 18);
		x1 ^= rotate(x0 + x3, 7);
		x2 ^= rotate(x1 + x0, 9);
		x3 ^= rotate(x2 + x1, 13);
		x0 ^= rotate(x3 + x2, 18);
		x6 ^= rotate(x5 + x4, 7);
		x7 ^= rotate(x6 + x5, 9);
		x4 ^= rotate(x7 + x6, 13);
		x5 ^= rotate(x4 + x7, 18);
		x11 ^= rotate(x10 + x9, 7);
		x8 ^= rotate(x11 + x10, 9);
		x9 ^= rotate(x8 + x11, 13);
		x10 ^= rotate(x9 + x8, 18);
		x12 ^= rotate(x15 + x14, 7);
		x13 ^= rotate(x12 + x15, 9);
		x14 ^= rotate(x13 + x12, 13);
		x15 ^= rotate(x14 + x13, 18);
	}

	x0 += j0;
	x1 += j1;
	x2 += j2;
	x3 += j3;
	x4 += j4;
	x5 += j5;
	x6 += j6;
	x7 += j7;
	x8 += j8;
	x9 += j9;
	x10 += j10;
	x11 += j11;
	x12 += j12;
	x13 += j13;
	x14 += j14;
	x15 += j15;

	store_littleendian(out + 0, x0);
	store_littleendian(out + 4, x1);
	store_littleendian(out + 8, x2);
	store_littleendian(out + 12, x3);
	store_littleendian(out + 16, x4);
	store_littleendian(out + 20, x5);
	store_littleendian(out + 24, x6);
	store_littleendian(out + 28, x7);
	store_littleendian(out + 32, x8);
	store_littleendian(out + 36, x9);
	store_littleendian(out + 40, x10);
	store_littleendian(out + 44, x11);
	store_littleendian(out + 48, x12);
	store_littleendian(out + 52, x13);
	store_littleendian(out + 56, x14);
	store_littleendian(out + 60, x15);
	return;
}

TEST_DEF void
salsa20_stream(
    void *dst, size_t len, const uint8_t nonce[static 8], const uint8_t key[static 32])
{
	static const uint8_t sigma[16] = "expand 32-byte k";
	uint8_t in[16];

	if (len == 0)
		return;

	memcpy(in, nonce, 8);
	memset(in + 8, 0, 8);

	while (len >= 64) {
		unsigned int u;

		core_salsa20(dst, in, key, sigma);
		u = 1;
		for (size_t i = 8; i < 16; i++) {
			u += in[i];
			in[i] = u;
			u >>= 8;
		}

		dst = (char *)dst + 64;
		len -= 64;
	}

	if (len > 0) {
		char block[64];

		core_salsa20(block, in, key, sigma);
		memcpy(dst, block, len);
	}

	return;
}

/**
 * PH block compression.
 */
TEST_DEF struct umash_ph
ph_one_block(const uint64_t *params, uint64_t seed, const void *block)
{
	struct umash_ph ret;
	__m128i acc = _mm_cvtsi64_si128(seed);

	for (size_t i = 0; i < UMASH_PH_PARAM_COUNT; i += 2) {
		__m128i x, k;

		memcpy(&x, block, sizeof(x));
		block = (const char *)block + sizeof(x);

		memcpy(&k, &params[i], sizeof(k));
		x ^= k;
		acc ^= _mm_clmulepi64_si128(x, x, 1);
	}

	memcpy(&ret, &acc, sizeof(ret));
	return ret;
}

static void
ph_one_block_toeplitz(struct umash_ph dst[static 2], const uint64_t *params,
    uint64_t seed, const void *block)
{
	__m128i acc[2] = { _mm_cvtsi64_si128(seed), _mm_cvtsi64_si128(seed) };

	for (size_t i = 0; i < UMASH_PH_PARAM_COUNT; i += 2) {
		__m128i x, k0, k1;

		memcpy(&x, block, sizeof(x));
		block = (const char *)block + sizeof(x);

		memcpy(&k0, &params[i], sizeof(k1));
		memcpy(&k1, &params[i + UMASH_PH_TOEPLITZ_SHIFT], sizeof(k1));

		k0 ^= x;
		acc[0] ^= _mm_clmulepi64_si128(k0, k0, 1);
		k1 ^= x;
		acc[1] ^= _mm_clmulepi64_si128(k1, k1, 1);
	}

	memcpy(dst, acc, sizeof(acc));
	return;
}

TEST_DEF struct umash_ph
ph_last_block(const uint64_t *params, uint64_t seed, const void *block, size_t n_bytes)
{
	struct umash_ph ret;
	__m128i acc = _mm_cvtsi64_si128(seed);

	/* The final block processes `remaining > 0` bytes. */
	size_t remaining = 1 + ((n_bytes - 1) % sizeof(__m128i));
	size_t end_full_pairs = (n_bytes - remaining) / sizeof(uint64_t);
	const void *last_ptr = (const char *)block + n_bytes - sizeof(__m128i);
	size_t i;

	for (i = 0; i < end_full_pairs; i += 2) {
		__m128i x, k;

		memcpy(&x, block, sizeof(x));
		block = (const char *)block + sizeof(x);

		memcpy(&k, &params[i], sizeof(k));
		x ^= k;
		acc ^= _mm_clmulepi64_si128(x, x, 1);
	}

	/* Compress the final (potentially partial) pair. */
	{
		uint64_t x, y;

		memcpy(&x, last_ptr, sizeof(x));
		last_ptr = (const char *)last_ptr + sizeof(x);
		memcpy(&y, last_ptr, sizeof(y));

		x ^= params[i];
		y ^= params[i + 1];

		acc ^=
		    _mm_clmulepi64_si128(_mm_cvtsi64_si128(x), _mm_cvtsi64_si128(y), 0);
	}

	memcpy(&ret, &acc, sizeof(ret));
	return ret;
}

static void
ph_last_block_toeplitz(struct umash_ph dst[static 2], const uint64_t *params,
    uint64_t seed, const void *block, size_t n_bytes)
{
	__m128i acc[2] = { _mm_cvtsi64_si128(seed), _mm_cvtsi64_si128(seed) };

	/* The final block processes `remaining > 0` bytes. */
	size_t remaining = 1 + ((n_bytes - 1) % sizeof(__m128i));
	size_t end_full_pairs = (n_bytes - remaining) / sizeof(uint64_t);
	const void *last_ptr = (const char *)block + n_bytes - sizeof(__m128i);
	size_t i;

	for (i = 0; i < end_full_pairs; i += 2) {
		__m128i x, k0, k1;

		memcpy(&x, block, sizeof(x));
		block = (const char *)block + sizeof(x);

		memcpy(&k0, &params[i], sizeof(k1));
		memcpy(&k1, &params[i + UMASH_PH_TOEPLITZ_SHIFT], sizeof(k1));

		k0 ^= x;
		acc[0] ^= _mm_clmulepi64_si128(k0, k0, 1);
		k1 ^= x;
		acc[1] ^= _mm_clmulepi64_si128(k1, k1, 1);
	}

	{
		__m128i x, k0, k1;
		uint64_t x0, x1;

		memcpy(&x0, last_ptr, sizeof(x0));
		last_ptr = (const char *)last_ptr + sizeof(x0);
		memcpy(&x1, last_ptr, sizeof(x1));

		x = _mm_set_epi64x(x1, x0);
		memcpy(&k0, &params[i], sizeof(k0));
		memcpy(&k1, &params[i + UMASH_PH_TOEPLITZ_SHIFT], sizeof(k1));

		k0 ^= x;
		acc[0] ^= _mm_clmulepi64_si128(k0, k0, 1);
		k1 ^= x;
		acc[1] ^= _mm_clmulepi64_si128(k1, k1, 1);
	}

	memcpy(dst, &acc, sizeof(acc));
	return;
}

/**
 * Short UMASH (<= 8 bytes).
 */
TEST_DEF inline uint64_t
vec_to_u64(const void *data, size_t n_bytes)
{
	const char zeros[2] = { 0 };
	uint32_t hi, lo;

	/*
	 * If there are at least 4 bytes to read, read the first 4 in
	 * `lo`, and the last 4 in `hi`.  This covers the whole range,
	 * since `n_bytes` is at most 8.
	 */
	if (LIKELY(n_bytes >= sizeof(lo))) {
		memcpy(&lo, data, sizeof(lo));
		memcpy(&hi, (const char *)data + n_bytes - sizeof(hi), sizeof(hi));
	} else {
		/* 0 <= n_bytes < 4.  Decode the size in binary. */
		uint16_t word;
		uint8_t byte;

		/*
		 * If the size is odd, load the first byte in `byte`;
		 * otherwise, load in a zero.
		 */
		memcpy(&byte, ((n_bytes & 1) != 0) ? data : zeros, 1);
		lo = byte;

		/*
		 * If the size is 2 or 3, load the last two bytes in `word`;
		 * otherwise, load in a zero.
		 */
		memcpy(&word,
		    ((n_bytes & 2) != 0) ? (const char *)data + n_bytes - 2 : zeros, 2);
		/*
		 * We have now read `bytes[0 ... n_bytes - 1]`
		 * exactly once without overwriting any data.
		 */
		hi = word;
	}

	/*
	 * Mix `hi` with the `lo` bits: SplitMix64 seems to have
	 * trouble with the top 4 bits.
	 */
	return ((uint64_t)hi << 32) | (lo + hi);
}

TEST_DEF uint64_t
umash_short(const uint64_t *params, uint64_t seed, const void *data, size_t n_bytes)
{
	uint64_t h;

	seed += params[n_bytes];
	h = vec_to_u64(data, n_bytes);
	h ^= h >> 30;
	h *= 0xbf58476d1ce4e5b9ULL;
	h = (h ^ seed) ^ (h >> 27);
	h *= 0x94d049bb133111ebULL;
	h ^= h >> 31;
	return h;
}

static struct umash_fp
umash_fp_short(const uint64_t *params, uint64_t seed, const void *data, size_t n_bytes)
{
	struct umash_fp ret;
	uint64_t h;

	ret.hash[0] = seed + params[n_bytes];
	ret.hash[1] = seed + params[n_bytes + UMASH_PH_TOEPLITZ_SHIFT];

	h = vec_to_u64(data, n_bytes);
	h ^= h >> 30;
	h *= 0xbf58476d1ce4e5b9ULL;
	h ^= h >> 27;

#define TAIL(i)                                       \
	do {                                          \
		ret.hash[i] ^= h;                     \
		ret.hash[i] *= 0x94d049bb133111ebULL; \
		ret.hash[i] ^= ret.hash[i] >> 31;     \
	} while (0)

	TAIL(0);
	TAIL(1);
#undef TAIL

	return ret;
}

TEST_DEF inline uint64_t
finalize(uint64_t x)
{

	x ^= x >> 27;
	x *= 0x94d049bb133111ebUL;
	return x;
}

TEST_DEF uint64_t
umash_medium(const uint64_t multipliers[static 2], const uint64_t *ph, uint64_t seed,
    const void *data, size_t n_bytes)
{
	union {
		__m128i vec;
		uint64_t u64[2];
	} acc = { .vec = _mm_cvtsi64_si128(seed ^ n_bytes) };

	{
		uint64_t x, y;

		memcpy(&x, data, sizeof(x));
		memcpy(&y, (const char *)data + n_bytes - sizeof(y), sizeof(y));
		x ^= ph[0];
		y ^= ph[1];

		acc.vec ^=
		    _mm_clmulepi64_si128(_mm_cvtsi64_si128(x), _mm_cvtsi64_si128(y), 0);
	}

	return finalize(horner_double_update(
	    /*acc=*/0, multipliers[0], multipliers[1], acc.u64[0], acc.u64[1]));
}

static struct umash_fp
umash_fp_medium(const uint64_t multipliers[static 2][2], const uint64_t *ph,
    uint64_t seed, const void *data, size_t n_bytes)
{
	const __m128i offset = _mm_cvtsi64_si128(seed ^ n_bytes);
	__m128i expanded;
	struct umash_fp ret;

	/* Expand the 9-16 bytes to 16. */
	{
		uint64_t x, y;

		memcpy(&x, data, sizeof(x));
		memcpy(&y, (const char *)data + n_bytes - sizeof(y), sizeof(y));
		expanded = _mm_set_epi64x(y, x);
	}

#define HASH(i, shift)                                                                \
	do {                                                                          \
		union {                                                               \
			__m128i vec;                                                  \
			uint64_t u64[2];                                              \
		} hash;                                                               \
                                                                                      \
		memcpy(&hash.vec, &ph[shift], sizeof(hash));                          \
		hash.vec ^= expanded;                                                 \
		hash.vec = _mm_clmulepi64_si128(hash.vec, hash.vec, 1);               \
		hash.vec ^= offset;                                                   \
                                                                                      \
		ret.hash[i] = finalize(horner_double_update(/*acc=*/0,                \
		    multipliers[i][0], multipliers[i][1], hash.u64[0], hash.u64[1])); \
	} while (0)

	HASH(0, 0);
	HASH(1, UMASH_PH_TOEPLITZ_SHIFT);
#undef HASH

	return ret;
}

TEST_DEF uint64_t
umash_long(const uint64_t multipliers[static 2], const uint64_t *ph, uint64_t seed,
    const void *data, size_t n_bytes)
{
	uint64_t acc = 0;

	while (n_bytes > BLOCK_SIZE) {
		struct umash_ph compressed;

		compressed = ph_one_block(ph, seed, data);
		data = (const char *)data + BLOCK_SIZE;
		n_bytes -= BLOCK_SIZE;

		acc = horner_double_update(acc, multipliers[0], multipliers[1],
		    compressed.bits[0], compressed.bits[1]);
	}

	/* Do the final block. */
	{
		struct umash_ph compressed;

		seed ^= (uint8_t)n_bytes;
		compressed = ph_last_block(ph, seed, data, n_bytes);
		acc = horner_double_update(acc, multipliers[0], multipliers[1],
		    compressed.bits[0], compressed.bits[1]);
	}

	return finalize(acc);
}

static struct umash_fp
umash_fp_long(const uint64_t multipliers[static 2][2], const uint64_t *ph, uint64_t seed,
    const void *data, size_t n_bytes)
{
	struct umash_ph compressed[2];
	struct umash_fp ret;
	uint64_t acc[2] = { 0, 0 };

	while (n_bytes > BLOCK_SIZE) {
		ph_one_block_toeplitz(compressed, ph, seed, data);

#define UPDATE(i)                                                                     \
	do {                                                                          \
		acc[i] = horner_double_update(acc[i], multipliers[i][0],              \
		    multipliers[i][1], compressed[i].bits[0], compressed[i].bits[1]); \
	} while (0)

		UPDATE(0);
		UPDATE(1);
#undef UPDATE

		data = (const char *)data + BLOCK_SIZE;
		n_bytes -= BLOCK_SIZE;
	}

	seed ^= (uint8_t)n_bytes;
	ph_last_block_toeplitz(compressed, ph, seed, data, n_bytes);

#define FINAL(i, shift)                                                               \
	do {                                                                          \
		acc[i] = horner_double_update(acc[i], multipliers[i][0],              \
		    multipliers[i][1], compressed[i].bits[0], compressed[i].bits[1]); \
		ret.hash[i] = finalize(acc[i]);                                       \
	} while (0)

	FINAL(0, 0);
	FINAL(1, UMASH_PH_TOEPLITZ_SHIFT);
#undef FINAL

	return ret;
}

static bool
value_is_repeated(const uint64_t *values, size_t n, uint64_t needle)
{

	for (size_t i = 0; i < n; i++) {
		if (values[i] == needle)
			return true;
	}

	return false;
}

bool
umash_params_prepare(struct umash_params *params)
{
	static const uint64_t modulo = (1UL << 61) - 1;
	/*
	 * The polynomial parameters have two redundant fields (for
	 * the pre-squared multipliers).  Use them as our source of
	 * extra entropy if needed.
	 */
	uint64_t buf[] = { params->poly[0][0], params->poly[1][0] };
	size_t buf_idx = 0;

#define GET_RANDOM(DST)                         \
	do {                                    \
		if (buf_idx >= ARRAY_SIZE(buf)) \
			return false;           \
                                                \
		(DST) = buf[buf_idx++];         \
	} while (0)

	/* Check the polynomial multipliers: we don't want 0s. */
	for (size_t i = 0; i < ARRAY_SIZE(params->poly); i++) {
		uint64_t f = params->poly[i][1];

		while (true) {
			/*
			 * Zero out bits and use rejection sampling to
			 * guarantee uniformity.
			 */
			f &= (1UL << 61) - 1;
			if (f != 0 && f < modulo)
				break;

			GET_RANDOM(f);
		}

		/* We can work in 2**64 - 8 and reduce after the fact. */
		params->poly[i][0] = mul_mod_fast(f, f) % modulo;
		params->poly[i][1] = f;
	}

	/* Avoid repeated PH noise values. */
	for (size_t i = 0; i < ARRAY_SIZE(params->ph); i++) {
		while (value_is_repeated(params->ph, i, params->ph[i]))
			GET_RANDOM(params->ph[i]);
	}

	return true;
}

void
umash_params_derive(struct umash_params *params, uint64_t bits, const void *key)
{
	uint8_t umash_key[32] = "Do not use UMASH VS adversaries.";

	if (key != NULL)
		memcpy(umash_key, key, sizeof(umash_key));

	while (true) {
		uint8_t nonce[8];

		for (size_t i = 0; i < 8; i++)
			nonce[i] = bits >> (8 * i);

		salsa20_stream(params, sizeof(*params), nonce, umash_key);
		if (umash_params_prepare(params))
			return;

		/*
		 * This should practically never fail, so really
		 * shouldn't happen multiple times.  If it does, an
		 * infinite loop is as good as anything else.
		 */
		bits++;
	}
}

/*
 * Updates the polynomial state at the end of a block.
 */
static void
sink_update_poly(struct umash_sink *sink)
{
	const __m128i ph_acc = _mm_cvtsi64_si128(sink->seed);
	/*
	 * Size of the current block in bytes, modulo 256.  May only
	 * be non-zero for the last block.
	 */
	uint8_t block_size = sink->block_size;

#define UPDATE(i)                                                                       \
	do {                                                                            \
		uint64_t ph0 = sink->ph_acc[i].bits[0] ^ block_size;                    \
		uint64_t ph1 = sink->ph_acc[i].bits[1];                                 \
                                                                                        \
		sink->poly_state[i].acc = horner_double_update(sink->poly_state[i].acc, \
		    sink->poly_state[i].mul[0], sink->poly_state[i].mul[1], ph0, ph1);  \
                                                                                        \
		memcpy(&sink->ph_acc[i], &ph_acc, sizeof(ph_acc));                      \
		if (!sink->fingerprinting)                                              \
			return;                                                         \
	} while (0)

	UPDATE(0);
	UPDATE(1);
#undef UPDATE

	return;
}

/* Updates the PH state with 16 bytes of data. */
static void
sink_consume_buf(struct umash_sink *sink, const char buf[static INCREMENTAL_GRANULARITY])
{
	const size_t buf_begin = sizeof(sink->buf) - INCREMENTAL_GRANULARITY;
	uint64_t x, y;

	memcpy(&x, buf, sizeof(x));
	memcpy(&y, buf + sizeof(x), sizeof(y));

#define UPDATE(i, param)                                                            \
	do {                                                                        \
		__m128i acc;                                                        \
                                                                                    \
		/* Use GPR loads to avoid forwarding stalls.  */                    \
		memcpy(&acc, &sink->ph_acc[i], sizeof(acc));                        \
		acc ^= _mm_clmulepi64_si128(_mm_cvtsi64_si128(x ^ sink->ph[param]), \
		    _mm_cvtsi64_si128(y ^ sink->ph[param + 1]), 0);                 \
		memcpy(&sink->ph_acc[i], &acc, sizeof(acc));                        \
                                                                                    \
		if (!sink->fingerprinting)                                          \
			goto next;                                                  \
	} while (0)

	UPDATE(0, sink->ph_iter);
	UPDATE(1, (sink->ph_iter + UMASH_PH_TOEPLITZ_SHIFT));
#undef UPDATE

next:
	memmove(&sink->buf, buf, buf_begin);
	sink->block_size += sink->bufsz;
	sink->bufsz = 0;
	sink->ph_iter += 2;
	sink->large_umash = true;

	if (sink->ph_iter == UMASH_PH_PARAM_COUNT) {
		sink_update_poly(sink);
		sink->block_size = 0;
		sink->ph_iter = 0;
	}

	return;
}

void
umash_sink_update(struct umash_sink *sink, const void *data, size_t n_bytes)
{
	const size_t buf_begin = sizeof(sink->buf) - INCREMENTAL_GRANULARITY;
	size_t remaining = INCREMENTAL_GRANULARITY - sink->bufsz;

	if (n_bytes < remaining) {
		memcpy(&sink->buf[buf_begin + sink->bufsz], data, n_bytes);
		sink->bufsz += n_bytes;
		return;
	}

	memcpy(&sink->buf[buf_begin + sink->bufsz], data, remaining);
	data = (const char *)data + remaining;
	n_bytes -= remaining;
	sink->bufsz = INCREMENTAL_GRANULARITY;
	sink_consume_buf(sink, sink->buf + buf_begin);

	while (n_bytes >= INCREMENTAL_GRANULARITY) {
		n_bytes -= INCREMENTAL_GRANULARITY;

		sink->bufsz = INCREMENTAL_GRANULARITY;
		/* Copy if this is the last full chunk. */
		sink_consume_buf(sink, data);
		data = (const char *)data + INCREMENTAL_GRANULARITY;
	}

	memcpy(&sink->buf[buf_begin], data, n_bytes);
	sink->bufsz = n_bytes;
	return;
}

uint64_t
umash_full(const struct umash_params *params, uint64_t seed, int which, const void *data,
    size_t n_bytes)
{
	const size_t shift = (which == 0) ? 0 : UMASH_PH_TOEPLITZ_SHIFT;

	which = (which == 0) ? 0 : 1;
	/*
	 * It's not that short inputs are necessarily more likely, but
	 * we want to make sure they fall through correctly to
	 * minimise latency.
	 */
	if (LIKELY(n_bytes <= sizeof(__m128i))) {
		if (LIKELY(n_bytes <= sizeof(uint64_t)))
			return umash_short(&params->ph[shift], seed, data, n_bytes);

		return umash_medium(
		    params->poly[which], &params->ph[shift], seed, data, n_bytes);
	}

	return umash_long(params->poly[which], &params->ph[shift], seed, data, n_bytes);
}

struct umash_fp
umash_fprint(
    const struct umash_params *params, uint64_t seed, const void *data, size_t n_bytes)
{

	if (LIKELY(n_bytes <= sizeof(__m128i))) {
		if (LIKELY(n_bytes <= sizeof(uint64_t)))
			return umash_fp_short(params->ph, seed, data, n_bytes);

		return umash_fp_medium(params->poly, params->ph, seed, data, n_bytes);
	}

	return umash_fp_long(params->poly, params->ph, seed, data, n_bytes);
}

void
umash_init(struct umash_state *state, const struct umash_params *params, uint64_t seed,
    int which)
{
	const size_t shift = (which == 0) ? 0 : UMASH_PH_TOEPLITZ_SHIFT;

	which = (which == 0) ? 0 : 1;
	state->sink = (struct umash_sink) {
		.poly_state[0] = {
			.mul = {
				params->poly[which][0],
				params->poly[which][1],
			},
		},
		.ph = &params->ph[shift],
		.ph_acc[0].bits[0] = seed,
		.seed = seed,
	};

	return;
}

void
umash_fp_init(
    struct umash_fp_state *state, const struct umash_params *params, uint64_t seed)
{

	state->sink = (struct umash_sink) {
		.poly_state[0] = {
			.mul = {
				params->poly[0][0],
				params->poly[0][1],
			},
		},
		.poly_state[1]= {
			.mul = {
				params->poly[1][0],
				params->poly[1][1],
			},
		},
		.ph = params->ph,
		.fingerprinting = true,
		.ph_acc[0].bits[0] = seed,
		.ph_acc[1].bits[0] = seed,
		.seed = seed,
	};

	return;
}

/**
 * Pumps any last block out of the incremental state.
 */
static void
digest_flush(struct umash_sink *sink)
{

	if (sink->bufsz > 0)
		sink_consume_buf(sink, &sink->buf[sink->bufsz]);

	if (sink->block_size != 0)
		sink_update_poly(sink);
	return;
}

/**
 * Finalizes a digest out of `sink`'s current state.
 *
 * The `sink` must be `digest_flush`ed if it is a `large_umash`.
 *
 * @param index 0 to return the first (only, if hashing) value, 1 for the
 *   second independent value for fingerprinting.
 */
static uint64_t
digest(const struct umash_sink *sink, int index)
{
	const size_t buf_begin = sizeof(sink->buf) - INCREMENTAL_GRANULARITY;
	const size_t shift = index * UMASH_PH_TOEPLITZ_SHIFT;

	if (sink->large_umash)
		return finalize(sink->poly_state[index].acc);

	if (sink->bufsz <= sizeof(uint64_t))
		return umash_short(
		    &sink->ph[shift], sink->seed, &sink->buf[buf_begin], sink->bufsz);

	return umash_medium(sink->poly_state[index].mul, &sink->ph[shift], sink->seed,
	    &sink->buf[buf_begin], sink->bufsz);
}

uint64_t
umash_digest(const struct umash_state *state)
{
	struct umash_sink copy;
	const struct umash_sink *sink = &state->sink;

	if (sink->large_umash) {
		copy = *sink;
		digest_flush(&copy);
		sink = &copy;
	}

	return digest(sink, 0);
}

struct umash_fp
umash_fp_digest(const struct umash_fp_state *state)
{
	struct umash_sink copy;
	struct umash_fp ret;
	const size_t buf_begin = sizeof(state->sink.buf) - INCREMENTAL_GRANULARITY;
	const struct umash_sink *sink = &state->sink;

	if (sink->large_umash) {
		copy = *sink;
		digest_flush(&copy);
		sink = &copy;
	} else if (sink->bufsz <= sizeof(uint64_t)) {
		return umash_fp_short(
		    sink->ph, sink->seed, &sink->buf[buf_begin], sink->bufsz);
	} else {
		const struct umash_params *params;

		/*
		 * Back out the params struct from our pointer to its
		 * `ph` member.
		 */
		params = (const void *)((const char *)sink->ph -
		    __builtin_offsetof(struct umash_params, ph));
		return umash_fp_medium(params->poly, sink->ph, sink->seed,
		    &sink->buf[buf_begin], sink->bufsz);
	}

	for (size_t i = 0; i < ARRAY_SIZE(ret.hash); i++)
		ret.hash[i] = digest(sink, i);

	return ret;
}
