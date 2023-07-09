#pragma once

#ifndef DISCOHASH_H
#define DISCOHASH_H

#include <stdint.h>

typedef uint32_t seed_t;

template <uint32_t hashsize>
void DISCoHAsH(const void* key, int len, seed_t seed, void* out);
template <uint32_t hashsize>
void DISCoHAsH_2( const void * in, int len, seed_t seed, void * out );

void DISCoHAsH_512_64(const void* key, int len, seed_t seed, void* out);

// reject bad seeds
void DISCoHAsH_512_64_seed_init (uint32_t &seed);

#include "discohash1.tpp"
#include "discohash2.tpp"


#endif // DISCOHASH_H

