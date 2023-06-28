#pragma once

#ifndef DISCOHASH_H
#define DISCOHASH_H

#include <stdint.h>

void DISCoHAsH_64(const void* key, int len, uint32_t seed, void* out);
void DISCoHAsH_512_64(const void* key, int len, uint32_t seed, void* out);

#endif // DISCOHASH_H

