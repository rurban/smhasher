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
#include "jody_hash32.h"

/* Hash a block of arbitrary size; must be divisible by sizeof(jodyhash_t)
 * The first block should pass a start_hash of zero.
 * All blocks after the first should pass start_hash as the value
 * returned by the last call to this function. This allows hashing
 * of any amount of data. If data is not divisible by the size of
 * jodyhash_t, it is MANDATORY that the caller provide a data buffer
 * which is divisible by sizeof(jodyhash_t). */
jodyhash32_t jody_block_hash32(const jodyhash32_t * data, const jodyhash32_t start_hash, const size_t count)
{
	const jodyhash32_t s_constant = JH32_ROR2(JODY_HASH32_CONSTANT);
	jodyhash32_t hash = start_hash;
	jodyhash32_t element, element2, partial_constant;
	size_t length = 0;

	/* Don't bother trying to hash a zero-length block */
	if (count == 0) return hash;

	length = count / sizeof(jodyhash32_t);

	/* Handle tails or everything */
	for (; length > 0; length--) {
		element = *data;
		element2 = JH32_ROR(element);
		element2 ^= s_constant;
		element += JODY_HASH32_CONSTANT;
		hash += element;
		hash ^= element2;
		hash = JH32_ROL2(hash);
		hash += element;
		data++;
	}

	/* Handle data tail (for blocks indivisible by sizeof(jodyhash32_t)) */
	length = count & (sizeof(jodyhash32_t) - 1);
	if (length) {
		partial_constant = JODY_HASH32_CONSTANT & tail32_mask[length];
		element = *data & tail32_mask[length];
		hash += partial_constant;
		hash += element;
		hash = JH32_ROL(hash);
		hash ^= element;
		hash = JH32_ROL(hash);
		hash ^= partial_constant;
		hash += element;
	}

	return hash;
	fprintf(stderr, "out of memory\n");
	exit(EXIT_FAILURE);
}
