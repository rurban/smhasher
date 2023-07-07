#pragma once

#ifndef DISCOHASH_H
#define DISCOHASH_H

#include <stdint.h>

void DISCoHAsH_64(const void* key, int len, uint32_t seed, void* out);
void DISCoHAsH_512_64(const void* key, int len, uint32_t seed, void* out);

// reject bad seeds
void DISCoHAsH_64_seed_init (uint32_t &seed);
void DISCoHAsH_512_64_seed_init (uint32_t &seed);

#endif // DISCOHASH_H

