/* Jody Bruchon's fast hashing function
 *
 * This function was written to generate a fast hash that also has a
 * fairly low collision rate. The collision rate is much higher than
 * a secure hash algorithm, but the calculation is drastically simpler
 * and faster.
 *
 * Copyright (C) 2014-2017 by Jody Bruchon <jody@jodybruchon.com>
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
#include "jody_hash64.h"

/* Width of a jody_hash. Changing this will also require
 * changing the width of tail masks to match. */
#define JODY_HASH_WIDTH 64

/* DO NOT modify the shift unless you know what you're doing.
 * This shift was decided upon after lots of testing and
 * changing it will likely cause lots of hash collisions. */
#ifndef JODY_HASH_SHIFT
#define JODY_HASH_SHIFT 14
#endif

/* The salt value's purpose is to cause each byte in the
 * jodyhash_t word to have a positionally dependent variation.
 * It is injected into the calculation to prevent a string of
 * identical bytes from easily producing an identical hash. */

/* The tail mask table is used for block sizes that are
 * indivisible by the width of a jodyhash_t. It is ANDed with the
 * final jodyhash_t-sized element to zero out data in the buffer
 * that is not part of the data to be hashed. */

/* Set hash parameters based on requested hash width */
#if JODY_HASH_WIDTH == 64
#define JODY_HASH_CONSTANT 0x1f3d5b79U
static const jodyhash_t tail_mask[] = {
	0x0000000000000000,
	0x00000000000000ff,
	0x000000000000ffff,
	0x0000000000ffffff,
	0x00000000ffffffff,
	0x000000ffffffffff,
	0x0000ffffffffffff,
	0x00ffffffffffffff,
	0xffffffffffffffff
};
#endif /* JODY_HASH_WIDTH == 64 */
#if JODY_HASH_WIDTH == 32
#define JODY_HASH_CONSTANT 0x1f3d5b79U
static const jodyhash_t tail_mask[] = {
	0x00000000,
	0x000000ff,
	0x0000ffff,
	0x00ffffff,
	0xffffffff,
};
#endif /* JODY_HASH_WIDTH == 32 */
#if JODY_HASH_WIDTH == 16
#define JODY_HASH_CONSTANT 0x1f5bU
static const jodyhash_t tail_mask[] = {
	0x0000,
	0x00ff,
	0xffff,
};
#endif /* JODY_HASH_WIDTH == 16 */


/* Hash a block of arbitrary size; must be divisible by sizeof(jodyhash_t)
 * The first block should pass a start_hash of zero.
 * All blocks after the first should pass start_hash as the value
 * returned by the last call to this function. This allows hashing
 * of any amount of data. If data is not divisible by the size of
 * jodyhash_t, it is MANDATORY that the caller provide a data buffer
 * which is divisible by sizeof(jodyhash_t). */
jodyhash_t jody_block_hash(const jodyhash_t *data,
                           const jodyhash_t start_hash, const size_t count)
{
	jodyhash_t hash = start_hash;
	jodyhash_t element;
	jodyhash_t partial_salt;
	size_t len;

	/* Don't bother trying to hash a zero-length block */
	if (count == 0) return hash;

	len = count / sizeof(jodyhash_t);
	for (; len > 0; len--) {
		element = *data;
		hash += element;
		hash += JODY_HASH_CONSTANT;
		hash = (hash << JODY_HASH_SHIFT) | hash >> (sizeof(jodyhash_t) * 8 - JODY_HASH_SHIFT); /* bit rotate left */
		hash ^= element;
		hash = (hash << JODY_HASH_SHIFT) | hash >> (sizeof(jodyhash_t) * 8 - JODY_HASH_SHIFT);
		hash ^= JODY_HASH_CONSTANT;
		hash += element;
		data++;
	}

	/* Handle data tail (for blocks indivisible by sizeof(jodyhash_t)) */
	len = count & (sizeof(jodyhash_t) - 1);
	if (len) {
		partial_salt = JODY_HASH_CONSTANT & tail_mask[len];
		element = *data & tail_mask[len];
		hash += element;
		hash += partial_salt;
		hash = (hash << JODY_HASH_SHIFT) | hash >> (sizeof(jodyhash_t) * 8 - JODY_HASH_SHIFT);
		hash ^= element;
		hash = (hash << JODY_HASH_SHIFT) | hash >> (sizeof(jodyhash_t) * 8 - JODY_HASH_SHIFT);
		hash ^= partial_salt;
		hash += element;
	}

	return hash;
}
