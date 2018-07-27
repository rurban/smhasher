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
typedef uint64_t jodyhash_t;

/* Version increments when algorithm changes incompatibly */
#define JODY_HASH_VERSION 5

jodyhash_t jody_block_hash(const jodyhash_t *data,
                           const jodyhash_t start_hash, const size_t count);

#ifdef __cplusplus
}
#endif

#endif	/* JODY_HASH_H */
