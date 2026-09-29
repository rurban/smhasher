// Copyright (c) 2026 Thomas Dybdahl Ahle. MIT; see chainhash/LICENSE.
#ifndef SMHASHER_CHAINHASH_H
#define SMHASHER_CHAINHASH_H
#include <stddef.h>
#include <stdint.h>
void chainhash_test(const void *, int, uint32_t, void *);
void chainhash128_test(const void *, int, uint32_t, void *);
void chainhash_seed_init(size_t seed);
#endif
