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

typedef uint32_t jodyhash32_t;

/* Version increments when algorithm changes incompatibly */
#define JODY_HASH_VERSION 5

extern jodyhash32_t jody_block_hash32(const jodyhash32_t *data,
		const jodyhash32_t start_hash, const size_t count);

#ifdef __cplusplus
}
#endif

#endif	/* JODY_HASH32_H */
