#ifndef _PENGYHASH_H
#define _PENGYHASH_H

#include <stdint.h>
#include <string.h>

uint64_t pengyhash(const void *p, size_t size, uint32_t seed);

#endif
