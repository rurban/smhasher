/* Jody Bruchon's fast hashing function (headers)
 * See jody_hash.c for license information */


/*
 * ****** WARNING *******
 *
 * This has been modified to integrate with SMHasher. DO NOT USE in other
 * projects. The proper source for jodyhash is available at:
 * https://github.com/jbruchon/jodyhash
 *
 * ****** WARNING *******
 */


#ifndef JODY_HASH_H
#define JODY_HASH_H

#ifdef __cplusplus
extern "C" {
#endif

/* Required for uint64_t */
#include <stdint.h>

/* Width of a jody_hash. Changing this will also require
 * changing the width of tail masks to match. */
#ifndef JODY_HASH_WIDTH
#define JODY_HASH_WIDTH 64
#endif

/* Version increments when algorithm changes incompatibly */
#define JODY_HASH_VERSION 6

/* DO NOT modify shifts/contants unless you know what you're doing. They were
 * chosen after lots of testing. Changes will likely cause lots of hash
 * collisions. The vectorized versions also use constants that have this value
 * "baked in" which must be updated before using them. */
#ifndef JODY_HASH_SHIFT
#define JODY_HASH_SHIFT 14
#endif

/* The constant value's purpose is to cause each byte in the
 * jodyhash_t word to have a positionally dependent variation.
 * It is injected into the calculation to prevent a string of
 * identical bytes from easily producing an identical hash. */

/* The tail mask table is used for block sizes that are
 * indivisible by the width of a jodyhash_t. It is ANDed with the
 * final jodyhash_t-sized element to zero out data in the buffer
 * that is not part of the data to be hashed. */

/* Set hash parameters based on requested hash width */
#if JODY_HASH_WIDTH == 64
typedef uint64_t jodyhash_t;
#ifndef JODY_HASH_CONSTANT
#define JODY_HASH_CONSTANT           0x71812e0f5463d3c8ULL
#endif
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
typedef uint32_t jodyhash_t;
#ifndef JODY_HASH_CONSTANT
#define JODY_HASH_CONSTANT 0x8748ee5dU
#endif
static const jodyhash_t tail_mask[] = {
	0x00000000,
	0x000000ff,
	0x0000ffff,
	0x00ffffff,
	0xffffffff
};
#endif /* JODY_HASH_WIDTH == 32 */
#if JODY_HASH_WIDTH == 16
typedef uint16_t jodyhash_t;
#ifndef JODY_HASH_CONSTANT
#define JODY_HASH_CONSTANT 0x1f5bU
#endif
static const jodyhash_t tail_mask[] = {
	0x0000,
	0x00ff,
	0xffff
};
#endif /* JODY_HASH_WIDTH == 16 */

/* Double-length shift for double-rotation optimization */
#define JH_SHIFT2 ((JODY_HASH_SHIFT * 2) - (((JODY_HASH_SHIFT * 2) > JODY_HASH_WIDTH) * JODY_HASH_WIDTH))
#define JODY_HASH_CONSTANT_ROR2     (JODY_HASH_CONSTANT >> JH_SHIFT2 | (JODY_HASH_CONSTANT << ((sizeof(jodyhash_t) * 8) - JH_SHIFT2)))

/* Macros for bitwise rotation */
#define JH_ROL(a)  (jodyhash_t)((a << JODY_HASH_SHIFT) | (a >> ((sizeof(jodyhash_t) * 8) - JODY_HASH_SHIFT)))
#define JH_ROR(a)  (jodyhash_t)((a >> JODY_HASH_SHIFT) | (a << ((sizeof(jodyhash_t) * 8) - JODY_HASH_SHIFT)))
#define JH_ROL2(a) (jodyhash_t)(a << JH_SHIFT2 | (a >> ((sizeof(jodyhash_t) * 8) - JH_SHIFT2)))
#define JH_ROR2(a) (jodyhash_t)(a >> JH_SHIFT2 | (a << ((sizeof(jodyhash_t) * 8) - JH_SHIFT2)))


extern jodyhash_t jody_block_hash(jodyhash_t *data, const jodyhash_t start_hash, const size_t count);

#ifdef __cplusplus
}
#endif

#endif	/* JODY_HASH_H */
