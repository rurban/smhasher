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


#ifndef JODY_HASH32_H
#define JODY_HASH32_H

#ifdef __cplusplus
extern "C" {
#endif

/* Required for uint64_t */
#include <stdint.h>

/* Width of a jody_hash. Changing this will also require
 * changing the width of tail masks to match. */
#ifndef JODY_HASH32_WIDTH
#define JODY_HASH32_WIDTH 32
#endif

/* Version increments when algorithm changes incompatibly */
#define JODY_HASH32_VERSION 6

/* DO NOT modify shifts/contants unless you know what you're doing. They were
 * chosen after lots of testing. Changes will likely cause lots of hash
 * collisions. The vectorized versions also use constants that have this value
 * "baked in" which must be updated before using them. */
#ifndef JODY_HASH32_SHIFT
#define JODY_HASH32_SHIFT 14
#endif

/* The constant value's purpose is to cause each byte in the
 * jodyhash32_t word to have a positionally dependent variation.
 * It is injected into the calculation to prevent a string of
 * identical bytes from easily producing an identical hash. */

/* The tail mask table is used for block sizes that are
 * indivisible by the width of a jodyhash32_t. It is ANDed with the
 * final jodyhash32_t-sized element to zero out data in the buffer
 * that is not part of the data to be hashed. */

/* Set hash parameters based on requested hash width */
typedef uint32_t jodyhash32_t;
#ifndef JODY_HASH32_CONSTANT
#define JODY_HASH32_CONSTANT 0xa682a37eU
#endif
static const jodyhash32_t tail32_mask[] = {
	0x00000000,
	0x000000ff,
	0x0000ffff,
	0x00ffffff,
	0xffffffff
};

/* Double-length shift for double-rotation optimization */
#define JH32_SHIFT2 ((JODY_HASH32_SHIFT * 2) - (((JODY_HASH32_SHIFT * 2) > JODY_HASH32_WIDTH) * JODY_HASH32_WIDTH))

/* Macros for bitwise rotation */
#define JH32_ROL(a)  (jodyhash32_t)((a << JODY_HASH32_SHIFT) | (a >> ((sizeof(jodyhash32_t) * 8) - JODY_HASH32_SHIFT)))
#define JH32_ROR(a)  (jodyhash32_t)((a >> JODY_HASH32_SHIFT) | (a << ((sizeof(jodyhash32_t) * 8) - JODY_HASH32_SHIFT)))
#define JH32_ROL2(a) (jodyhash32_t)(a << JH32_SHIFT2 | (a >> ((sizeof(jodyhash32_t) * 8) - JH32_SHIFT2)))
#define JH32_ROR2(a) (jodyhash32_t)(a >> JH32_SHIFT2 | (a << ((sizeof(jodyhash32_t) * 8) - JH32_SHIFT2)))


extern jodyhash32_t jody_block_hash32(const jodyhash32_t *data, const jodyhash32_t start_hash, const size_t count);

#ifdef __cplusplus
}
#endif

#endif	/* JODY_HASH32_H */
