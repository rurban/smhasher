#include "siphash.h"
#include "siphash_impl.h"

#ifndef U8TO64
#define U8TO64
static uint64_t INLINE
U8TO64_LE(const unsigned char *p) {
	return *(const uint64_t *)p;
}
#endif

/*
static void INLINE
U64TO8_LE(unsigned char *p, const uint64_t v) {
	*(uint64_t *)p = v;
}
*/

#define SIPCOMPRESS \
	v0 += v1; v2 += v3; \
	v1 = ROTL64(v1,13); v3 = ROTL64(v3,16); \
	v1 ^= v0; v3 ^= v2; \
	v0 = ROTL64(v0,32); \
	v2 += v1; v0 += v3; \
	v1 = ROTL64(v1,17); v3 = ROTL64(v3,21); \
	v1 ^= v2; v3 ^= v0; \
	v2 = ROTL64(v2,32)

/* The 64bit 2-4 variant */
uint64_t
siphash(const unsigned char key[16], const unsigned char *m, size_t len) {
	uint64_t v0, v1, v2, v3;
	uint64_t mi, k0, k1;
	uint64_t last7;
	size_t i, blocks;

	k0 = U8TO64_LE(key + 0);
	k1 = U8TO64_LE(key + 8);
	v0 = k0 ^ 0x736f6d6570736575ull;
	v1 = k1 ^ 0x646f72616e646f6dull;
	v2 = k0 ^ 0x6c7967656e657261ull;
	v3 = k1 ^ 0x7465646279746573ull;

	last7 = (uint64_t)(len & 0xff) << 56;

	for (i = 0, blocks = (len & ~7); i < blocks; i += 8) {
		mi = U8TO64_LE(m + i);
		v3 ^= mi;
		SIPCOMPRESS; /* 2 c rounds */
		SIPCOMPRESS;
		v0 ^= mi;
	}

	switch (len - blocks) {
		case 7: last7 |= (uint64_t)m[i + 6] << 48;
		case 6: last7 |= (uint64_t)m[i + 5] << 40;
		case 5: last7 |= (uint64_t)m[i + 4] << 32;
		case 4: last7 |= (uint64_t)m[i + 3] << 24;
		case 3: last7 |= (uint64_t)m[i + 2] << 16;
		case 2: last7 |= (uint64_t)m[i + 1] <<  8;
		case 1: last7 |= (uint64_t)m[i + 0]      ;
		case 0:
		default:;
	};
	v3 ^= last7;
	SIPCOMPRESS; /* 2 more c rounds */
	SIPCOMPRESS;
	v0 ^= last7;
	v2 ^= 0xff;
	SIPCOMPRESS; /* and 4 final d rounds */
	SIPCOMPRESS;
	SIPCOMPRESS;
	SIPCOMPRESS;
	return v0 ^ v1 ^ v2 ^ v3;
}

/* The 64bit 1-3 variant */
uint64_t
siphash13(const unsigned char key[16], const unsigned char *m, size_t len) {
	uint64_t v0, v1, v2, v3;
	uint64_t mi, k0, k1;
	uint64_t last7;
	size_t i, blocks;

	k0 = U8TO64_LE(key + 0);
	k1 = U8TO64_LE(key + 8);
	v0 = k0 ^ 0x736f6d6570736575ull;
	v1 = k1 ^ 0x646f72616e646f6dull;
	v2 = k0 ^ 0x6c7967656e657261ull;
	v3 = k1 ^ 0x7465646279746573ull;

	last7 = (uint64_t)(len & 0xff) << 56;

	for (i = 0, blocks = (len & ~7); i < blocks; i += 8) {
		mi = U8TO64_LE(m + i);
		v3 ^= mi;
		SIPCOMPRESS; /* 1 c round */
		v0 ^= mi;
	}

	switch (len - blocks) {
		case 7: last7 |= (uint64_t)m[i + 6] << 48;
		case 6: last7 |= (uint64_t)m[i + 5] << 40;
		case 5: last7 |= (uint64_t)m[i + 4] << 32;
		case 4: last7 |= (uint64_t)m[i + 3] << 24;
		case 3: last7 |= (uint64_t)m[i + 2] << 16;
		case 2: last7 |= (uint64_t)m[i + 1] <<  8;
		case 1: last7 |= (uint64_t)m[i + 0]      ;
		case 0:
		default:;
	};
	v3 ^= last7;
	SIPCOMPRESS; /* 1 more c round */
	v0 ^= last7;
	v2 ^= 0xff;
	SIPCOMPRESS; /* and 3 final d rounds */
	SIPCOMPRESS;
	SIPCOMPRESS;
	return v0 ^ v1 ^ v2 ^ v3;
}

#undef sipcompress

#include "halfsiphash.c"
