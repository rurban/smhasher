#pragma once

#ifndef DISCOHASH_H
#define DISCOHASH_H

#include <stdint.h>
#include <cstring>

static inline uint64_t disco_load_le64(const void *p) {
    const uint8_t *b = static_cast<const uint8_t *>(p);
    uint64_t v = 0;
    for (unsigned i = 0; i < 8; ++i) v |= uint64_t(b[i]) << (8 * i);
    return v;
}

static inline void disco_store_le64(void *p, uint64_t v) {
    uint8_t *b = static_cast<uint8_t *>(p);
    for (unsigned i = 0; i < 8; ++i) b[i] = uint8_t(v >> (8 * i));
}

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

