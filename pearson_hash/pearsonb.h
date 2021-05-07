#include <stdint.h>
#include <stddef.h>

#include "portable_endian.h"


void pearsonb_hash_256 (uint8_t *out, const uint8_t *in, size_t len, uint64_t seed);

void pearsonb_hash_128 (uint8_t *out, const uint8_t *in, size_t len, uint64_t seed);

uint64_t pearsonb_hash_64 (const uint8_t *in, size_t len, uint64_t seed);

uint32_t pearsonb_hash_32 (const uint8_t *in, size_t len, uint64_t seed);

uint16_t pearsonb_hash_16 (const uint8_t *in, size_t len, uint64_t seed);

void pearsonb_hash_init(void);
