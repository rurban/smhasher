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

/* Vector intrinsic multi-case handler header
 * Shamelessly stolen from Marat Dukhan's answer on Stack Overflow:
 * https://stackoverflow.com/a/22291538
 */
#if defined(_MSC_VER)
     /* Microsoft C/C++-compatible compiler */
     #include <intrin.h>
#elif defined(__GNUC__) && (defined(__x86_64__) || defined(__i386__))
     /* GCC-compatible compiler, targeting x86/x86-64 */
     #include <x86intrin.h>
#elif defined(__GNUC__) && defined(__ARM_NEON__)
     /* GCC-compatible compiler, targeting ARM with NEON */
     #include <arm_neon.h>
#elif defined(__GNUC__) && defined(__IWMMXT__)
     /* GCC-compatible compiler, targeting ARM with WMMX */
     #include <mmintrin.h>
#elif (defined(__GNUC__) || defined(__xlC__)) && (defined(__VEC__) || defined(__ALTIVEC__))
     /* XLC or GCC-compatible compiler, targeting PowerPC with VMX/VSX */
     #include <altivec.h>
#elif defined(__GNUC__) && defined(__SPE__)
     /* GCC-compatible compiler, targeting PowerPC with SPE */
     #include <spe.h>
#endif

/* Hash a block of arbitrary size; must be divisible by sizeof(jodyhash_t)
 * The first block should pass a start_hash of zero.
 * All blocks after the first should pass start_hash as the value
 * returned by the last call to this function. This allows hashing
 * of any amount of data. If data is not divisible by the size of
 * jodyhash_t, it is MANDATORY that the caller provide a data buffer
 * which is divisible by sizeof(jodyhash_t). */
extern jodyhash_t jody_block_hash(const jodyhash_t *data, const jodyhash_t start_hash, const size_t count)
{
	const jodyhash_t s_constant = JH_ROR2(JODY_HASH_CONSTANT);
	jodyhash_t hash = start_hash;
	jodyhash_t element, element2, partial_constant;
	size_t length = 0;

	union UINT128 {
		__m128i  v128;
		uint64_t v64[2];
	};
	union UINT128 vec_constant, vec_constant_ror2;
	size_t vec_allocsize;
	__m128i *aligned_data, *aligned_data_e;
	__m128i v1, v2, v3, v4, v5, v6, v7, v8, v9, v10, v11, v12;
	__m128i vec_const, vec_ror2;


	/* Don't bother trying to hash a zero-length block */
	if (count == 0) return hash;

	__builtin_cpu_init ();
	if (__builtin_cpu_supports ("sse2")) {
			/* Use SSE2 if possible */
			vec_constant.v64[0]      = JODY_HASH_CONSTANT;
			vec_constant.v64[1]      = JODY_HASH_CONSTANT;
			vec_constant_ror2.v64[0] = JODY_HASH_CONSTANT_ROR2;
			vec_constant_ror2.v64[1] = JODY_HASH_CONSTANT_ROR2;
			/* Constants preload */
			vec_const = _mm_load_si128(&vec_constant.v128);
			vec_ror2  = _mm_load_si128(&vec_constant_ror2.v128);
			if (count > 63) {
				/* Only handle 64-byte sized chunks and leave the rest */
				vec_allocsize =  count & 0xffffffffffffffc0U;
				aligned_data_e = (__m128i *)aligned_alloc(32, vec_allocsize);
				aligned_data  = (__m128i *)aligned_alloc(32, vec_allocsize);
				if (!aligned_data_e || !aligned_data) goto oom;
				memcpy(aligned_data, data, vec_allocsize);
				length = vec_allocsize / 16; // sizeof(__m128i)

				uint64_t *ep1 = (uint64_t *)(aligned_data_e);
				uint64_t *ep2 = (uint64_t *)(aligned_data);
				for (size_t i = 0; i < length; i += 4) {

					v1  = _mm_load_si128(&aligned_data[i]);
					v3  = _mm_load_si128(&aligned_data[i]);
					v4  = _mm_load_si128(&aligned_data[i+1]);
					v6  = _mm_load_si128(&aligned_data[i+1]);
					v7  = _mm_load_si128(&aligned_data[i+2]);
					v9  = _mm_load_si128(&aligned_data[i+2]);
					v10 = _mm_load_si128(&aligned_data[i+3]);
					v12 = _mm_load_si128(&aligned_data[i+3]);

					/* "element2" gets RORed */
					v1  = _mm_srli_epi64(v1, JODY_HASH_SHIFT);
					v2  = _mm_slli_epi64(v3, (64 - JODY_HASH_SHIFT));
					v1  = _mm_or_si128(v1, v2);
					v1  = _mm_xor_si128(v1, vec_ror2);  // XOR against the ROR2 constant
					v4  = _mm_srli_epi64(v4, JODY_HASH_SHIFT);  // Repeat for all vectors
					v5  = _mm_slli_epi64(v6, (64 - JODY_HASH_SHIFT));
					v4  = _mm_or_si128(v4, v5);
					v4  = _mm_xor_si128(v4, vec_ror2);
					v7  = _mm_srli_epi64(v7, JODY_HASH_SHIFT);
					v8  = _mm_slli_epi64(v9, (64 - JODY_HASH_SHIFT));
					v7  = _mm_or_si128(v7, v8);
					v7  = _mm_xor_si128(v7, vec_ror2);
					v10 = _mm_srli_epi64(v10, JODY_HASH_SHIFT);
					v11 = _mm_slli_epi64(v12, (64 - JODY_HASH_SHIFT));
					v10 = _mm_or_si128(v10, v11);
					v10 = _mm_xor_si128(v10, vec_ror2);

					/* Add the constant to "element" */
					v3  = _mm_add_epi64(v3,  vec_const);
					v6  = _mm_add_epi64(v6,  vec_const);
					v9  = _mm_add_epi64(v9,  vec_const);
					v12 = _mm_add_epi64(v12, vec_const);

					/* Store everything */
					_mm_store_si128(&aligned_data[i], v1);
					_mm_store_si128(&aligned_data_e[i], v3);
					_mm_store_si128(&aligned_data[i+1], v4);
					_mm_store_si128(&aligned_data_e[i+1], v6);
					_mm_store_si128(&aligned_data[i+2], v7);
					_mm_store_si128(&aligned_data_e[i+2], v9);
					_mm_store_si128(&aligned_data[i+3], v10);
					_mm_store_si128(&aligned_data_e[i+3], v12);

					/* Perform the rest of the hash normally */
					for (size_t j = 0; j < 8; j++) {
						element = *(ep1 + j);
						element2 = *(ep2 + j);
						hash += element;
						hash ^= element2;
						hash = JH_ROL2(hash);
						hash += element;
					}
				ep1 += 8; ep2 += 8;
				}

				free(aligned_data_e); free(aligned_data);
				data += vec_allocsize / sizeof(jodyhash_t);
				length = (count - vec_allocsize) / sizeof(jodyhash_t);
			} else {
				length = count / sizeof(jodyhash_t);
			}
		}

	/* Handle tails or everything */
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
		partial_constant = JODY_HASH_CONSTANT & tail_mask[length];
		element = *data & tail_mask[length];
		hash += partial_constant;
		hash += element;
		hash = JH_ROL(hash);
		hash ^= element;
		hash = JH_ROL(hash);
		hash ^= partial_constant;
		hash += element;
	}

	return hash;
#ifndef NO_SIMD
oom:
#endif
	fprintf(stderr, "out of memory\n");
	exit(EXIT_FAILURE);
}
