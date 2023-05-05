/* Jody Bruchon's fast hashing function
 *
 * This function was written to generate a fast hash that also has a
 * fairly low collision rate. The collision rate is much higher than
 * a secure hash algorithm, but the calculation is drastically simpler
 * and faster.
 *
 * Copyright (C) 2014-2023 by Jody Bruchon <jody@jodybruchon.com>
 * Released under The MIT License
 */


/*
 * ****** WARNING *******
 *
 * This has been modified to integrate with SMHasher. DO NOT USE in other
 * projects. The proper source for jodyhash is available at:
 * https://github.com/jbruchon/jodyhash
 *
 * ****** WARNING *******
 */


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "jody_hash64.h"

/* Disable SIMD if not 64-bit width or not 64-bit x86 code */
#if JODY_HASH_WIDTH != 64 || defined NO_SIMD || !defined __x86_64__ || (defined NO_SSE2 && defined NO_AVX2)
 #undef USE_SSE2
 #undef USE_AVX2
 #ifndef NO_SIMD
  #define NO_SIMD
 #endif
#endif

/* Use SIMD by default */
#if !defined NO_SIMD
 #if defined __SSE2__ && !defined NO_SSE2
  #define USE_SSE2
 #endif
 #if defined __AVX2__ && !defined NO_AVX2
  #define USE_AVX2
 #endif
 #if defined _MSC_VER || defined _WIN32 || defined __MINGW32__
  /* Microsoft C/C++-compatible compiler */
  #include <intrin.h>
  #define aligned_alloc(a,b) _aligned_malloc(b,a)
  #define ALIGNED_FREE(a) _aligned_free(a)
 #elif defined __GNUC__  && (defined __x86_64__  || defined __i386__ )
  /* GCC-compatible compiler, targeting x86/x86-64 */
  #include <x86intrin.h>
  #define ALIGNED_FREE(a) free(a)
 #endif
#endif /* !NO_SIMD */



/* Hash a block of arbitrary size; must be divisible by sizeof(jodyhash_t)
 * The first block should pass a start_hash of zero.
 * All blocks after the first should pass start_hash as the value
 * returned by the last call to this function. This allows hashing
 * of any amount of data. If data is not divisible by the size of
 * jodyhash_t, it is MANDATORY that the caller provide a data buffer
 * which is divisible by sizeof(jodyhash_t). */
extern jodyhash_t jody_block_hash(jodyhash_t *data,
		const jodyhash_t start_hash, const size_t count)
{
	const jodyhash_t s_constant = JH_ROR2(JODY_HASH_CONSTANT);
	jodyhash_t hash = start_hash;
	jodyhash_t element, element2;
	size_t length = 0;

#if defined USE_SSE2 || defined USE_AVX2
	union UINT256 {
		__m256i  v256;
		__m128i  v128[2];
		uint64_t v64[4];
	};
	union UINT256 vec_constant, vec_constant_ror2;
	size_t vec_allocsize;
	__m256i *aligned_data;
#endif
#ifdef USE_SSE2
	/* Regs used in groups of 3; 1=ROR/XOR work, 2=temp, 3=data+constant */
	__m128i v1, v2, v3, v4, v5, v6;
	__m128 vzero;
	__m128i vec_const, vec_ror2;
#endif /* USE_SSE2 */
#ifdef USE_AVX2
	/* Regs used in groups of 3; 1=ROR/XOR work, 2=temp, 3=data+constant */
	__m256i vx1, vx2, vx3;
	__m256i avx_const, avx_ror2;
#endif /* USE_AVX2 */


	/* Don't bother trying to hash a zero-length block */
	if (count == 0) return hash;

#ifdef USE_AVX2
#if defined (__GNUC__)
	__builtin_cpu_init ();
	if (__builtin_cpu_supports ("avx2"))
#endif /* __GNUC__ */
	{
		if (count >= 32) {
			/* Use AVX2 if possible */
			vec_constant.v64[0]      = JODY_HASH_CONSTANT;
			vec_constant.v64[1]      = JODY_HASH_CONSTANT;
			vec_constant.v64[2]      = JODY_HASH_CONSTANT;
			vec_constant.v64[3]      = JODY_HASH_CONSTANT;
			vec_constant_ror2.v64[0] = JODY_HASH_CONSTANT_ROR2;
			vec_constant_ror2.v64[1] = JODY_HASH_CONSTANT_ROR2;
			vec_constant_ror2.v64[2] = JODY_HASH_CONSTANT_ROR2;
			vec_constant_ror2.v64[3] = JODY_HASH_CONSTANT_ROR2;

			/* Constants preload */
			avx_const = _mm256_load_si256(&vec_constant.v256);
			avx_ror2  = _mm256_load_si256(&vec_constant_ror2.v256);

			/* How much memory do we need to align the data? */
			vec_allocsize = count & 0xffffffffffffffe0U;
			/* Only alloc/copy if not already aligned */
			if (((uintptr_t)data & (uintptr_t)0x1fULL) != 0) {
				aligned_data  = (__m256i *)aligned_alloc(32, vec_allocsize);
				if (!aligned_data) goto oom;
				memcpy(aligned_data, data, vec_allocsize);
			} else aligned_data = (__m256i *)data;

			for (size_t i = 0; i < (vec_allocsize / 32); i++) {
				vx1  = _mm256_load_si256(&aligned_data[i]);
				vx3  = _mm256_load_si256(&aligned_data[i]);

				/* "element2" gets RORed (two logical shifts ORed together) */
				vx1  = _mm256_srli_epi64(vx1, JODY_HASH_SHIFT);
				vx2  = _mm256_slli_epi64(vx3, (64 - JODY_HASH_SHIFT));
				vx1  = _mm256_or_si256(vx1, vx2);
				vx1  = _mm256_xor_si256(vx1, avx_ror2);  // XOR against the ROR2 constant

				/* Add the constant to "element" */
				vx3  = _mm256_add_epi64(vx3,  avx_const);

				/* Perform the rest of the hash */
				for (int j = 0; j < 4; j++) {
					uint64_t ep1, ep2;
					switch (j) {
						default:
						case 0:
						ep1 = (uint64_t)_mm256_extract_epi64(vx3, 0);
						ep2 = (uint64_t)_mm256_extract_epi64(vx1, 0);
						break;
						case 1:
						ep1 = (uint64_t)_mm256_extract_epi64(vx3, 1);
						ep2 = (uint64_t)_mm256_extract_epi64(vx1, 1);
						break;
						case 2:
						ep1 = (uint64_t)_mm256_extract_epi64(vx3, 2);
						ep2 = (uint64_t)_mm256_extract_epi64(vx1, 2);
						break;
						case 3:
						ep1 = (uint64_t)_mm256_extract_epi64(vx3, 3);
						ep2 = (uint64_t)_mm256_extract_epi64(vx1, 3);
						break;
					}
					hash += ep1;
					hash ^= ep2;
					hash = JH_ROL2(hash);
					hash += ep1;
				}  // End of hash finish loop
			}  // End of main AVX for loop
			data += vec_allocsize / sizeof(jodyhash_t);
			length = (count - vec_allocsize) / sizeof(jodyhash_t);
			if (((uintptr_t)data & (uintptr_t)0x1fULL) != 0) ALIGNED_FREE(aligned_data);
			goto skip_sse2;
		} else {
			length = count / sizeof(jodyhash_t);
		}

	}
#if defined (__GNUC__)
	else length = count / sizeof(jodyhash_t);
#endif
#else
	length = count / sizeof(jodyhash_t);
#endif /* USE_AVX2 */


#ifdef USE_SSE2
#if defined (__GNUC__)
	__builtin_cpu_init ();
	if (__builtin_cpu_supports ("sse2"))
#endif /* __GNUC__ */
	{
		if (count >= 32) {
			/* Use SSE2 if possible */
			vec_constant.v64[0]      = JODY_HASH_CONSTANT;
			vec_constant.v64[1]      = JODY_HASH_CONSTANT;
			vec_constant_ror2.v64[0] = JODY_HASH_CONSTANT_ROR2;
			vec_constant_ror2.v64[1] = JODY_HASH_CONSTANT_ROR2;

			/* Constants preload */
			vec_const = _mm_load_si128(&vec_constant.v128[0]);
			vec_ror2  = _mm_load_si128(&vec_constant_ror2.v128[0]);
			vzero = _mm_setzero_ps();

			/* How much memory do we need to align the data? */
			vec_allocsize = count & 0xffffffffffffffe0U;
			/* Only alloc/copy if not already aligned */
			if (((uintptr_t)data & (uintptr_t)0x0fULL) != 0) {
				aligned_data  = (__m256i *)aligned_alloc(16, vec_allocsize);
				if (!aligned_data) goto oom;
				memcpy(aligned_data, data, vec_allocsize);
			} else aligned_data = (__m256i *)data;

			for (size_t i = 0; i < (vec_allocsize / 16); i++) {
				v1  = _mm_load_si128(&((__m128i *)aligned_data)[i]);
				v3  = _mm_load_si128(&((__m128i *)aligned_data)[i]);
				i++;
				v4  = _mm_load_si128(&((__m128i *)aligned_data)[i]);
				v6  = _mm_load_si128(&((__m128i *)aligned_data)[i]);

				/* "element2" gets RORed (two logical shifts ORed together) */
				v1  = _mm_srli_epi64(v1, JODY_HASH_SHIFT);
				v2  = _mm_slli_epi64(v3, (64 - JODY_HASH_SHIFT));
				v1  = _mm_or_si128(v1, v2);
				v1  = _mm_xor_si128(v1, vec_ror2);  // XOR against the ROR2 constant
				v4  = _mm_srli_epi64(v4, JODY_HASH_SHIFT);
				v5  = _mm_slli_epi64(v6, (64 - JODY_HASH_SHIFT));
				v4  = _mm_or_si128(v4, v5);
				v4  = _mm_xor_si128(v4, vec_ror2);  // XOR against the ROR2 constant

				/* Add the constant to "element" */
				v3  = _mm_add_epi64(v3,  vec_const);
				v6  = _mm_add_epi64(v6,  vec_const);

				/* Perform the rest of the hash */
				for (int j = 0; j < 4; j++) {
					uint64_t ep1, ep2;
					switch (j) {
						default:
						case 0:
						/* Lower v1-v3 */
						ep1 = (uint64_t)_mm_cvtsi128_si64(v3);
						ep2 = (uint64_t)_mm_cvtsi128_si64(v1);
						break;

						case 1:
						/* Upper v1-v3 */
						ep1 = (uint64_t)_mm_cvtsi128_si64(_mm_castps_si128(_mm_movehl_ps(vzero, _mm_castsi128_ps(v3))));
						ep2 = (uint64_t)_mm_cvtsi128_si64(_mm_castps_si128(_mm_movehl_ps(vzero, _mm_castsi128_ps(v1))));
						break;

						case 2:
						/* Lower v4-v6 */
						ep1 = (uint64_t)_mm_cvtsi128_si64(v6);
						ep2 = (uint64_t)_mm_cvtsi128_si64(v4);
						break;

						case 3:
						/* Upper v4-v6 */
						ep1 = (uint64_t)_mm_cvtsi128_si64(_mm_castps_si128(_mm_movehl_ps(vzero, _mm_castsi128_ps(v6))));
						ep2 = (uint64_t)_mm_cvtsi128_si64(_mm_castps_si128(_mm_movehl_ps(vzero, _mm_castsi128_ps(v4))));
						break;
					}
					hash += ep1;
					hash ^= ep2;
					hash = JH_ROL2(hash);
					hash += ep1;
				}  // End of hash finish loop
			}  // End of main SSE for loop
			data += vec_allocsize / sizeof(jodyhash_t);
			length = (count - vec_allocsize) / sizeof(jodyhash_t);
			if (((uintptr_t)data & (uintptr_t)0x0fULL) != 0) ALIGNED_FREE(aligned_data);
		} else {
			length = count / sizeof(jodyhash_t);
		}

	}
#if defined (__GNUC__)
	else length = count / sizeof(jodyhash_t);
#endif
#else
	length = count / sizeof(jodyhash_t);
#endif /* USE_SSE2 */
#ifdef USE_AVX2
skip_sse2:
#endif

	/* Hash everything (normal) or remaining small tails (SSE2) */
	for (; length > 0; length--) {
		element = *data;
		element2 = JH_ROR(element);
		element2 ^= s_constant;
		element += JODY_HASH_CONSTANT;
		hash += element;
		hash ^= element2;
		hash = JH_ROL2(hash);
		hash += element;
		data++;
	}

	/* Handle data tail (for blocks indivisible by sizeof(jodyhash_t)) */
	length = count & (sizeof(jodyhash_t) - 1);
	if (length) {
		element = *data & tail_mask[length];
		element2 = JH_ROR(element);
		element2 ^= s_constant;
		element += JODY_HASH_CONSTANT;
		hash += element;
		hash ^= element2;
		hash = JH_ROL2(hash);
		hash += element2;
	}

	return hash;
#ifndef NO_SIMD
oom:
#endif
	fprintf(stderr, "out of memory\n");
	exit(EXIT_FAILURE);
}
