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
#ifndef JODY_HASH_WIDTH
#define JODY_HASH_WIDTH 32
#endif

#if JODY_HASH_WIDTH == 64
typedef uint64_t jodyhash32_t;
#endif
#if JODY_HASH_WIDTH == 32
typedef uint32_t jodyhash32_t;
#endif
#if JODY_HASH_WIDTH == 16
typedef uint16_t jodyhash32_t;
#endif

/* Version increments when algorithm changes incompatibly */
#define JODY_HASH_VERSION 5

extern jodyhash32_t jody_block_hash32(const jodyhash32_t *data,
		const jodyhash32_t start_hash, const size_t count);

#ifdef __cplusplus
}
#endif

#endif	/* JODY_HASH32_H */
