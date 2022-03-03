/* -*- mode: c; -*- vim: set ft=c: */

/*
 * UMASH is distributed under the MIT license.
 *
 * SPDX-License-Identifier: MIT
 *
 * Copyright 2020-2022 Backtrace I/O, Inc.
 * Copyright 2022 Paul Khuong
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

/**
 * More wasteful routine for longer inputs (that can absorb I$ misses
 * and fixed setup work).
 */
#if UMASH_LONG_INPUTS
/*
 * Minimum byte size before switching to `umash_multiple_blocks`.
 *
 * Leaving this variable undefined disable calls to
 * `umash_multiple_blocks.
 */
#define UMASH_MULTIPLE_BLOCKS_THRESHOLD 1024
#endif

typedef uint64_t umash_multiple_blocks_fn(uint64_t initial,
    const uint64_t multipliers[static 2], const uint64_t *oh_ptr, uint64_t seed,
    const void *blocks, size_t n_blocks);

typedef struct umash_fp umash_fprint_multiple_blocks_fn(struct umash_fp initial,
    const uint64_t multipliers[static 2][2], const uint64_t *oh, uint64_t seed,
    const void *data, size_t n_blocks);

/**
 * Updates a 64-bit UMASH state for `n_blocks` 256-byte blocks in
 * `data`.
 */
TEST_DEF umash_multiple_blocks_fn umash_multiple_blocks_generic;

/**
 * Updates a 128-bit UMASH fingerprint state for `n_blocks` 256-byte
 * blocks in `data`.
 */
TEST_DEF umash_fprint_multiple_blocks_fn umash_fprint_multiple_blocks_generic;

/**
 * Runtime dispatch logic.  When dynamic dispatch is enabled,
 * `umash_multiple_blocks` just forwards the call to
 * `umash_multiple_blocks_impl`, a function pointer.  That pointer is
 * initialised with a function that updates
 * `umash_multiple_blocks_impl` with an implementation appropriate for
 * the current CPU and tail-calls to that chosen implementation.
 *
 * When dynamic dispatch is disabled, `umash_multiple_blocks` just
 * always forwards to `umash_multiple_blocks_generic`.
 */
#if defined(__x86_64__) && UMASH_DYNAMIC_DISPATCH
#include <cpuid.h>
#include <stdatomic.h>

static umash_multiple_blocks_fn umash_multiple_blocks_initial,
    umash_multiple_blocks_vpclmulqdq;

static umash_fprint_multiple_blocks_fn umash_fprint_multiple_blocks_initial,
    umash_fprint_multiple_blocks_vpclmulqdq;

static umash_multiple_blocks_fn *_Atomic umash_multiple_blocks_impl =
    umash_multiple_blocks_initial;

static umash_fprint_multiple_blocks_fn *_Atomic umash_fprint_multiple_blocks_impl =
    umash_fprint_multiple_blocks_initial;

static COLD FN void
umash_long_pick(void)
{
	umash_multiple_blocks_fn *umash;
	umash_fprint_multiple_blocks_fn *fprint;
	bool has_vpclmulqdq = false;

	{
		const uint32_t extended_features_level = 7;
		const uint32_t vpclmulqdq_bit = 1UL << 10;
		uint32_t eax, ebx, ecx, edx;

		eax = ebx = ecx = edx = 0;
		__asm__("cpuid" : "+a"(eax), "+b"(ebx), "+c"(ecx), "+d"(edx));
		if (eax >= extended_features_level) {
			eax = extended_features_level;
			ebx = ecx = edx = 0;
			__asm__("cpuid" : "+a"(eax), "+b"(ebx), "+c"(ecx), "+d"(edx));
			has_vpclmulqdq = (ecx & vpclmulqdq_bit) != 0;
		}
	}

	if (has_vpclmulqdq) {
		umash = umash_multiple_blocks_vpclmulqdq;
		fprint = umash_fprint_multiple_blocks_vpclmulqdq;
	} else {
		umash = umash_multiple_blocks_generic;
		fprint = umash_fprint_multiple_blocks_generic;
	}

	atomic_store_explicit(&umash_multiple_blocks_impl, umash, memory_order_relaxed);
	atomic_store_explicit(
	    &umash_fprint_multiple_blocks_impl, fprint, memory_order_relaxed);
	return;
}

static COLD FN uint64_t
umash_multiple_blocks_initial(uint64_t initial, const uint64_t multipliers[static 2],
    const uint64_t *oh_ptr, uint64_t seed, const void *blocks, size_t n_blocks)
{
	umash_multiple_blocks_fn *impl;

	umash_long_pick();
	impl = atomic_load_explicit(&umash_multiple_blocks_impl, memory_order_relaxed);
	return impl(initial, multipliers, oh_ptr, seed, blocks, n_blocks);
}

TEST_DEF inline uint64_t
umash_multiple_blocks(uint64_t initial, const uint64_t multipliers[static 2],
    const uint64_t *oh_ptr, uint64_t seed, const void *blocks, size_t n_blocks)
{
	/*
	 * We should use a relaxed load here, but compilers have
	 * trouble fusing that with the call.  Directly calling the
	 * function pointer gets us a one-instruction memory indirect
	 * CALL on x86-64, versus a load, and a roundtrip through the
	 * stack if we use a discrete atomic load (on gcc 8).
	 */
	return umash_multiple_blocks_impl(
	    initial, multipliers, oh_ptr, seed, blocks, n_blocks);
}

static COLD FN struct umash_fp
umash_fprint_multiple_blocks_initial(struct umash_fp initial,
    const uint64_t multipliers[static 2][2], const uint64_t *oh, uint64_t seed,
    const void *data, size_t n_blocks)
{
	umash_fprint_multiple_blocks_fn *impl;

	umash_long_pick();
	impl = atomic_load_explicit(
	    &umash_fprint_multiple_blocks_impl, memory_order_relaxed);
	return impl(initial, multipliers, oh, seed, data, n_blocks);
}

TEST_DEF inline struct umash_fp
umash_fprint_multiple_blocks(struct umash_fp initial,
    const uint64_t multipliers[static 2][2], const uint64_t *oh, uint64_t seed,
    const void *data, size_t n_blocks)
{

	/*
	 * We should use a relaxed load here, but compilers have
	 * trouble fusing that with the call.  Directly calling the
	 * function pointer gets us a one-instruction memory indirect
	 * CALL on x86-64, versus a load, and a roundtrip through the
	 * stack if we use a discrete atomic load (on gcc 8).
	 */
	return umash_fprint_multiple_blocks_impl(
	    initial, multipliers, oh, seed, data, n_blocks);
}
#else
TEST_DEF inline uint64_t
umash_multiple_blocks(uint64_t initial, const uint64_t multipliers[static 2],
    const uint64_t *oh_ptr, uint64_t seed, const void *blocks, size_t n_blocks)
{
	return umash_multiple_blocks_generic(
	    initial, multipliers, oh_ptr, seed, blocks, n_blocks);
}

TEST_DEF inline struct umash_fp
umash_fprint_multiple_blocks(struct umash_fp initial,
    const uint64_t multipliers[static 2][2], const uint64_t *oh, uint64_t seed,
    const void *data, size_t n_blocks)
{

	return umash_fprint_multiple_blocks_generic(
	    initial, multipliers, oh, seed, data, n_blocks);
}
#endif

#define SPLIT_ACCUMULATOR_MAX_FIXUP 3

/**
 * A split accumulator represents a value (mod 2**64 - 8) as the
 * evaluated sum `base + 8 * fixup`, where `fixup <=
 * SPLIT_ACCUMULATOR_MAX_FIXUP`
 */
#ifndef UMASH_TEST_ONLY /* Also defined in `umash_test_only.h` */
struct split_accumulator {
	uint64_t base;
	uint64_t fixup;
};
#endif

/**
 * Reduces the split accumulator's value to a value < 2**64 - 8.
 */
TEST_DEF inline uint64_t
split_accumulator_eval(struct split_accumulator acc)
{
	return add_mod_slow(acc.base, 8 * acc.fixup);
}

/**
 * Returns the updated version of `acc` after a double-pumped Horner
 * update: acc = m0 * (acc + h0) + m1 * h1 (mod 2**64 - 8).
 */
TEST_DEF inline struct split_accumulator
split_accumulator_update(const struct split_accumulator acc, const uint64_t m0,
    const uint64_t m1, uint64_t h0, const uint64_t h1)
{
	uint64_t partial;
	uint64_t lo0, hi0, lo1, hi1;
	uint64_t hi;
	unsigned long long sum;
	int8_t fixup;

	mul128(m1, h1, &hi1, &lo1);

	/* partial \eqv (acc.base + h0 + 8 * acc.fixup)  mod 2**64 - 8 */
	if (UNLIKELY(h0 > -8ULL * (SPLIT_ACCUMULATOR_MAX_FIXUP + 1))) {
		h0 = add_mod_slow(h0, 8 * acc.fixup);
	} else {
		/*
		 * h0 is a hash value, so it's unlikely to be
		 * extremely high.  In the common case, this addition
		 * doesn't overflows.
		 */
		h0 += 8 * acc.fixup;
	}

	partial = add_mod_fast(acc.base, h0);

	mul128(partial, m0, &hi0, &lo0);

#if defined(__x86_64__) && UMASH_INLINE_ASM
	{
		uint8_t cf0, cf1;

		__asm__("addq %3, %0"
			: "=r"(sum), "=@ccc"(cf0)
			: "%0"(lo0), "r"(lo1)
			: "cc");

		hi = hi0 + hi1;

		__asm__("shlq $3, %0" : "+r"(hi), "=@ccc"(cf1) : : "cc");
		__asm__("addq %3, %0\n\t"
			"adcb %5, %1"
			: "=r"(sum), "=r"(fixup)
			: "%0"(sum), "r"(hi), "%1"(cf0), "r"(cf1)
			: "cc");
	}
#else
	fixup = __builtin_uaddll_overflow(lo0, lo1, &sum);

	assert(hi0 < (1UL << 61));
	assert(hi1 < (1UL << 61));
	/* hi0 and hi1 < 2**61, so this addition never overflows. */
	hi = hi0 + hi1;

	fixup += (hi & (1ULL << 61)) != 0;
	hi *= 8;

	fixup += __builtin_uaddll_overflow(sum, hi, &sum);
#endif

	return (struct split_accumulator) {
		.base = sum,
		/* Avoid sign extension: we know `fixup` is non-negative. */
		.fixup = (uint8_t)fixup,
	};
}

TEST_DEF HOT uint64_t
umash_multiple_blocks_generic(uint64_t initial, const uint64_t multipliers[static 2],
    const uint64_t *oh_ptr, uint64_t seed, const void *blocks, size_t n_blocks)
{
	const uint64_t m0 = multipliers[0];
	const uint64_t m1 = multipliers[1];
	const uint64_t kx = oh_ptr[UMASH_OH_PARAM_COUNT - 2];
	const uint64_t ky = oh_ptr[UMASH_OH_PARAM_COUNT - 1];
	struct split_accumulator ret = { .base = initial };

	assert(n_blocks > 0);

	do {
		const void *data = blocks;
		struct umash_oh oh;
		v128 acc = V128_ZERO;

		blocks = (const char *)blocks + BLOCK_SIZE;

		/*
		 * FORCE() makes sure the compiler computes the value
		 * of `acc` at that program points.  Forcing a full
		 * computation prevents the compiler from evaluating
		 * the inner loop's xor-reduction tree widely: the
		 * bottleneck is in the carryless multiplications.
		 */
#if UMASH_INLINE_ASM
#define FORCE() __asm__("" : "+x"(acc))
#else
#define FORCE() ((void)0)
#endif

#define PH(I)                                          \
	do {                                           \
		v128 x, k;                             \
                                                       \
		memcpy(&x, data, sizeof(x));           \
		data = (const char *)data + sizeof(x); \
                                                       \
		memcpy(&k, &oh_ptr[I], sizeof(k));     \
		x ^= k;                                \
		acc ^= v128_clmul_cross(x);            \
	} while (0)

		PH(0);
		PH(2);
		FORCE();

		PH(4);
		PH(6);
		FORCE();

		PH(8);
		PH(10);
		FORCE();

		PH(12);
		PH(14);
		FORCE();

		PH(16);
		PH(18);
		FORCE();

		PH(20);
		PH(22);
		FORCE();

		PH(24);
		PH(26);
		FORCE();

		PH(28);
#undef PH
#undef FORCE

		memcpy(&oh, &acc, sizeof(oh));

		/* Final ENH chunk. */
		{
			uint64_t x, y, enh_hi, enh_lo;

			memcpy(&x, data, sizeof(x));
			data = (const char *)data + sizeof(x);
			memcpy(&y, data, sizeof(y));
			data = (const char *)data + sizeof(y);

			x += kx;
			y += ky;

			mul128(x, y, &enh_hi, &enh_lo);
			enh_hi += seed;

			oh.bits[0] ^= enh_lo;
			oh.bits[1] ^= enh_hi ^ enh_lo;
		}

		ret = split_accumulator_update(ret, m0, m1, oh.bits[0], oh.bits[1]);
	} while (--n_blocks);

	return split_accumulator_eval(ret);
}

TEST_DEF HOT struct umash_fp
umash_fprint_multiple_blocks_generic(struct umash_fp initial,
    const uint64_t multipliers[static 2][2], const uint64_t *oh, uint64_t seed,
    const void *data, size_t n_blocks)
{
	const v128 lrc_init =
	    v128_create(oh[UMASH_OH_PARAM_COUNT], oh[UMASH_OH_PARAM_COUNT + 1]);
	const uint64_t m00 = multipliers[0][0];
	const uint64_t m01 = multipliers[0][1];
	const uint64_t m10 = multipliers[1][0];
	const uint64_t m11 = multipliers[1][1];
	struct split_accumulator acc0 = { .base = initial.hash[0] };
	struct split_accumulator acc1 = { .base = initial.hash[1] };

	do {
		struct umash_oh compressed[2];
		v128 acc = V128_ZERO; /* Base umash */
		v128 acc_shifted = V128_ZERO; /* Accumulates shifted values */
		v128 lrc = lrc_init;
		const void *block = data;

		data = (const char *)data + BLOCK_SIZE;

#if UMASH_INLINE_ASM
#define FORCE() __asm__("" : "+x"(acc), "+x"(acc_shifted), "+x"(lrc))
#else
#define FORCE() ((void)0)
#endif

#define TWIST(I)                                         \
	do {                                             \
		v128 x, k;                               \
                                                         \
		memcpy(&x, block, sizeof(x));            \
		block = (const char *)block + sizeof(x); \
                                                         \
		memcpy(&k, &oh[I], sizeof(k));           \
                                                         \
		x ^= k;                                  \
		lrc ^= x;                                \
                                                         \
		x = v128_clmul_cross(x);                 \
                                                         \
		acc ^= x;                                \
                                                         \
		if (I == 28)                             \
			break;                           \
                                                         \
		acc_shifted ^= x;                        \
		acc_shifted = v128_shift(acc_shifted);   \
	} while (0)

		TWIST(0);
		FORCE();
		TWIST(2);
		FORCE();
		TWIST(4);
		FORCE();
		TWIST(6);
		FORCE();
		TWIST(8);
		FORCE();
		TWIST(10);
		FORCE();
		TWIST(12);
		FORCE();
		TWIST(14);
		FORCE();
		TWIST(16);
		FORCE();
		TWIST(18);
		FORCE();
		TWIST(20);
		FORCE();
		TWIST(22);
		FORCE();
		TWIST(24);
		FORCE();
		TWIST(26);
		FORCE();
		TWIST(28);
		FORCE();

#undef TWIST
#undef FORCE

		{
			v128 x, k;

			memcpy(&x, block, sizeof(x));
			memcpy(&k, &oh[30], sizeof(k));

			lrc ^= x ^ k;
		}

		acc_shifted ^= acc;
		acc_shifted = v128_shift(acc_shifted);

		acc_shifted ^= v128_clmul_cross(lrc);

		memcpy(&compressed[0], &acc, sizeof(compressed[0]));
		memcpy(&compressed[1], &acc_shifted, sizeof(compressed[1]));

		{
			uint64_t x, y, kx, ky, enh_hi, enh_lo;

			memcpy(&x, block, sizeof(x));
			block = (const char *)block + sizeof(x);
			memcpy(&y, block, sizeof(y));

			kx = x + oh[30];
			ky = y + oh[31];

			mul128(kx, ky, &enh_hi, &enh_lo);
			enh_hi += seed;

			enh_hi ^= enh_lo;
			compressed[0].bits[0] ^= enh_lo;
			compressed[0].bits[1] ^= enh_hi;

			compressed[1].bits[0] ^= enh_lo;
			compressed[1].bits[1] ^= enh_hi;
		}

		acc0 = split_accumulator_update(
		    acc0, m00, m01, compressed[0].bits[0], compressed[0].bits[1]);
		acc1 = split_accumulator_update(
		    acc1, m10, m11, compressed[1].bits[0], compressed[1].bits[1]);
	} while (--n_blocks);

	return (struct umash_fp) {
                .hash = {
                        split_accumulator_eval(acc0),
                        split_accumulator_eval(acc1),
                },
        };
}

#if defined(__x86_64__) && UMASH_DYNAMIC_DISPATCH
/**
 * Updates a 64-bit UMASH state for `n_blocks` 256-byte blocks in data.
 */
TEST_DEF HOT __attribute__((__target__(("avx2,vpclmulqdq")))) uint64_t
umash_multiple_blocks_vpclmulqdq(uint64_t initial, const uint64_t multipliers[static 2],
    const uint64_t *oh_ptr, uint64_t seed, const void *blocks, size_t n_blocks)
{
	const uint64_t m0 = multipliers[0];
	const uint64_t m1 = multipliers[1];
	__m256i k0, k4, k8, k12, k16, k20, k24;
	v128 k28;
	const uint64_t kx = oh_ptr[UMASH_OH_PARAM_COUNT - 2];
	const uint64_t ky = oh_ptr[UMASH_OH_PARAM_COUNT - 1];
	struct split_accumulator ret = { .base = initial };

	assert(n_blocks > 0);

#if UMASH_INLINE_ASM
	/*
	 * This type represents unaligned AVX2-sized regions of
	 * memory.
	 */
	struct b256 {
		char bytes[32];
	};

#define LOADU(DST, PTR) \
	__asm__("vmovdqu %1, %0" : "=x"(DST) : "m"(*(const struct b256 *)PTR))
#else
#define LOADU(DST, PTR) ((DST) = _mm256_loadu_si256((void *)(PTR)))
#endif

	LOADU(k0, &oh_ptr[0]);
	LOADU(k4, &oh_ptr[4]);
	LOADU(k8, &oh_ptr[8]);
	LOADU(k12, &oh_ptr[12]);
	LOADU(k16, &oh_ptr[16]);
	LOADU(k20, &oh_ptr[20]);
	LOADU(k24, &oh_ptr[24]);
	memcpy(&k28, &oh_ptr[28], sizeof(k28));

	do {
		const void *data = blocks;
		struct umash_oh oh;
		__m256i acc2 = _mm256_setzero_si256();

		blocks = (const char *)blocks + BLOCK_SIZE;

#define PH(I)                                          \
	do {                                           \
		__m256i x;                             \
                                                       \
		LOADU(x, data);                        \
		data = (const char *)data + sizeof(x); \
                                                       \
		x = _mm256_xor_si256(x, k##I);         \
		x = _mm256_clmulepi64_epi128(x, x, 1); \
		acc2 = _mm256_xor_si256(acc2, x);      \
	} while (0)

		PH(0);
		PH(4);
		PH(8);
		PH(12);
		PH(16);
		PH(20);
		PH(24);

#undef PH
#undef LOADU

		{
			v128 x;

			memcpy(&x, data, sizeof(x));
			data = (const char *)data + sizeof(x);
			x ^= k28;
			x = v128_clmul_cross(x);

			x ^= _mm256_extracti128_si256(acc2, 0) ^
			    _mm256_extracti128_si256(acc2, 1);

			memcpy(&oh, &x, sizeof(oh));
		}

		/* Final ENH chunk. */
		{
			uint64_t x, y, enh_hi, enh_lo;

			memcpy(&x, data, sizeof(x));
			data = (const char *)data + sizeof(x);
			memcpy(&y, data, sizeof(y));
			data = (const char *)data + sizeof(y);

			x += kx;
			y += ky;
			mul128(x, y, &enh_hi, &enh_lo);
			enh_hi += seed;

			oh.bits[0] ^= enh_lo;
			oh.bits[1] ^= enh_hi ^ enh_lo;
		}

		ret = split_accumulator_update(ret, m0, m1, oh.bits[0], oh.bits[1]);
	} while (--n_blocks);

	return split_accumulator_eval(ret);
}

TEST_DEF HOT __attribute__((__target__(("avx2,vpclmulqdq")))) struct umash_fp
umash_fprint_multiple_blocks_vpclmulqdq(struct umash_fp initial,
    const uint64_t multipliers[static 2][2], const uint64_t *oh, uint64_t seed,
    const void *data, size_t n_blocks)
{
	__m256i k0, k4, k8, k12, k16, k20, k24, k28;
	__m256i lrc_init = { 0 };
	const uint64_t m00 = multipliers[0][0];
	const uint64_t m01 = multipliers[0][1];
	const uint64_t m10 = multipliers[1][0];
	const uint64_t m11 = multipliers[1][1];
	struct split_accumulator acc0 = { .base = initial.hash[0] };
	struct split_accumulator acc1 = { .base = initial.hash[1] };

#if UMASH_INLINE_ASM
	/*
	 * This type represents unaligned AVX2-sized regions of
	 * memory.
	 */
	struct b256 {
		char bytes[32];
	};

#define LOADU(DST, PTR) \
	__asm__("vmovdqu %1, %0" : "=x"(DST) : "m"(*(const struct b256 *)PTR))
#else
#define LOADU(DST, PTR) ((DST) = _mm256_loadu_si256((void *)(PTR)))
#endif

	LOADU(k0, &oh[0]);
	LOADU(k4, &oh[4]);
	LOADU(k8, &oh[8]);
	LOADU(k12, &oh[12]);
	LOADU(k16, &oh[16]);
	LOADU(k20, &oh[20]);
	LOADU(k24, &oh[24]);
	LOADU(k28, &oh[28]);

	{
		__m128i lrc_key;

		memcpy(&lrc_key, &oh[UMASH_OH_PARAM_COUNT], sizeof(lrc_key));
		lrc_init = _mm256_insertf128_si256(lrc_init, lrc_key, 0);
	}

	/*
	 * A UMASH fingerprint combines a regular (primary) 64-bit
	 * UMASH hash value with a secondary hash value that mostly
	 * reuses the primary's OH-mixed values.
	 *
	 * The key differences are:
	 *
	 * 1. the secondary hash generates an additional chunk by
	 *    xoring together all the input chunks, themselves xor-ed
	 *    with the corresponding random bits in the OH key.
	 * 2. the first 14 PH-mixed values are shifted by different
	 *    values (the first one by 15, second by 14, etc.)
	 * 3. the first 15 PH-mixed values are also shifted by 1
	 * 4. the values obtained by 14 and 15 are xored together
	 *    before xoring in the new chunk's PH-mixed value and
	 *    the ENH value.
	 *
	 * This AVX2 implementation derives the additional chunk in a
	 * 256-bit AVX register, and only xors its two halves together
	 * before CLMUL.
	 *
	 * Similarly, the PH-mixed values that are only shifted by 1
	 * are xored in an AVX register, and the two halves are only
	 * xored together before shifting the 128-bit result by 1.
	 *
	 * As for the 14 PH values with distinct shifts, they too are
	 * first accumulated in an AVX register, where each 128-bit
	 * half is shifted by 2.  When merging the result in a 128-bit
	 * result, the low half is shifted left (by 1) once more
	 * before xoring the halves together.
	 */
	do {
		struct umash_oh compressed[2];
		__m256i acc2 = { 0 }; /* Base umash */
		/*
		 * Accumulates shifted values in odd/even pairs, so
		 * each iteration must shift by two, until the final
		 * merge.
		 */
		__m256i acc_shifted2 = { 0 };
		__m256i lrc2 = lrc_init;
		v128 acc, acc_shifted, lrc;
		const void *block = data;

		data = (const char *)data + BLOCK_SIZE;

#define TWIST(I)                                         \
	do {                                             \
		__m256i x;                               \
                                                         \
		LOADU(x, block);                         \
		block = (const char *)block + sizeof(x); \
                                                         \
		x ^= k##I;                               \
		lrc2 ^= x;                               \
		x = _mm256_clmulepi64_epi128(x, x, 1);   \
                                                         \
		acc2 ^= x;                               \
		x = _mm256_slli_epi64(x, (24 - I) / 2);  \
		acc_shifted2 ^= x;                       \
	} while (0)

		TWIST(0);
		TWIST(4);
		TWIST(8);
		TWIST(12);
		TWIST(16);
		TWIST(20);
		TWIST(24);

#undef TWIST

		{
			__m256i x;
			v128 x_lo;

			LOADU(x, block);
			block = (const char *)block + sizeof(x_lo);

			x ^= k28;
			lrc2 ^= x;
			lrc = _mm256_extracti128_si256(lrc2, 0) ^
			    _mm256_extracti128_si256(lrc2, 1);

			acc_shifted =
			    v128_shift(_mm256_extracti128_si256(acc_shifted2, 0)) ^
			    _mm256_extracti128_si256(acc_shifted2, 1);
			acc_shifted = v128_shift(acc_shifted);

			x_lo = _mm256_extracti128_si256(x, 0);
			x_lo = v128_clmul_cross(x_lo);

			acc = _mm256_extracti128_si256(acc2, 0) ^
			    _mm256_extracti128_si256(acc2, 1);

			acc ^= x_lo;
		}

#undef LOADU

		acc_shifted ^= acc;
		acc_shifted = v128_shift(acc_shifted);

		acc_shifted ^= v128_clmul_cross(lrc);

		memcpy(&compressed[0], &acc, sizeof(compressed[0]));
		memcpy(&compressed[1], &acc_shifted, sizeof(compressed[1]));

		{
			uint64_t x, y, kx, ky, enh_hi, enh_lo;

			memcpy(&x, block, sizeof(x));
			block = (const char *)block + sizeof(x);
			memcpy(&y, block, sizeof(y));

			kx = x + oh[30];
			ky = y + oh[31];

			mul128(kx, ky, &enh_hi, &enh_lo);
			enh_hi += seed;

			enh_hi ^= enh_lo;
			compressed[0].bits[0] ^= enh_lo;
			compressed[0].bits[1] ^= enh_hi;

			compressed[1].bits[0] ^= enh_lo;
			compressed[1].bits[1] ^= enh_hi;
		}

		acc0 = split_accumulator_update(
		    acc0, m00, m01, compressed[0].bits[0], compressed[0].bits[1]);
		acc1 = split_accumulator_update(
		    acc1, m10, m11, compressed[1].bits[0], compressed[1].bits[1]);
	} while (--n_blocks);

	return (struct umash_fp) {
                .hash = {
                        split_accumulator_eval(acc0),
                        split_accumulator_eval(acc1),
                },
        };
}
#endif
