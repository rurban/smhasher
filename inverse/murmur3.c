// -*-  c-default-style: "bsd"; indent-tabs-mode: t; c-basic-offset: 8 -*-
// WIP 64bit

#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include <bits/endian.h>
#if __BYTE_ORDER == __BIG_ENDIAN
#	define __BIG_ENDIAN__
#elif __BYTE_ORDER == __LITTLE_ENDIAN
#	define __LITTLE_ENDIAN__
#endif
#include <assert.h>

static inline uint32_t fmix32 (uint32_t h)
{
	h ^= h >> 16;
	h *= 0x85ebca6b;
	h ^= h >> 13;
	h *= 0xc2b2ae35;
	h ^= h >> 16;

	return h;
}

static inline uint32_t rotl32 ( uint32_t x, int8_t r )
{
	return (x << r) | (x >> (32 - r));
}

// MurmurHash3. 32bit WIP
uint32_t murmurhash3_32(const unsigned char *key, int len, const uint32_t seed)
{
	const uint32_t c1 = 0xcc9e2d51;
	const uint32_t c2 = 0x1b873593;
	const uint32_t m = 0xe6546b64;

	// Initialize the hash with the random seed
	uint32_t h = seed;

	while (len >= 4) {
#ifdef __BIG_ENDIAN__
		uint32_t k = (key[0]) + (key[1] << 8) + (key[2] << 16) +
			     (key[3] << 24);
#else
		uint32_t k = *(uint32_t *)key;
#endif

		k *= c1;
		k ^= rotl32(k, 15);
		k *= c2;

		h ^= k;
		h = rotl32(h, 13);
		h = h * 5 + m;

		key += 4;
		len -= 4;
	}

	// tail
	uint32_t k = 0;
	switch (len) {
	case 3: k ^= key[2] << 16;
	case 2: k ^= key[1] << 8;
	case 1: k ^= key[0];
		k *= c1;
		k = rotl32(k, 15);
		k *= c2;
		h ^= k;
	};

	// Do a few final mixes of the hash to ensure the last few
	// bytes are well-incorporated.
	h ^= len;
	h = fmix32 (h);

	return h;
}

// Multiplicative inverse of x under % 2^32
// Or rather use openssl BN_mod_inverse(3)
uint32_t invert32(uint32_t x)
{
	uint32_t xx = x * x;
	while (xx != 1) {
		x *= xx;
		xx *= xx;
	}
	return x;
}

/// Inverts a (h ^= h >> s) operation with 8 <= s <= 16
uint32_t invert_shift_xor(const uint32_t hs, const uint32_t s)
{
	assert(s >= 8 && s <= 16);
	uint32_t hs0 = hs >> 24;
	uint32_t hs1 = (hs >> 16) & 0xff;
	uint32_t hs2 = (hs >> 8) & 0xff;
	uint32_t hs3 = hs & 0xff;

	uint32_t h0 = hs0;
	uint32_t h1 = hs1 ^ (h0 >> (s - 8));
	uint32_t h2 = (hs2 ^ (h0 << (16 - s)) ^ (h1 >> (s - 8))) & 0xff;
	uint32_t h3 = (hs3 ^ (h1 << (16 - s)) ^ (h2 >> (s - 8))) & 0xff;
	return (h0 << 24) + (h1 << 16) + (h2 << 8) + h3;
}

static inline uint32_t invert_fmix32 (uint32_t h)
{
	const uint32_t m1inv = 0xa5cb9243; // invert32(0x85ebca6b);
	const uint32_t m2inv = 0x7ed1b41d; // invert32(0xc2b2ae35);
	h = invert_shift_xor(h, 16);
	h *= m2inv;
	h = invert_shift_xor(h, 13);
	h *= m1inv;
	h = invert_shift_xor(h, 16);

	return h;
}

static inline uint32_t invert_rotl32 ( uint32_t x, int8_t r )
{
	return (x >> r) | (x << (32 - r));
}

// WIP
uint32_t invert_murmurhash3_32(uint32_t h, uint32_t const seed)
{
	const int len = 4;
	const uint32_t c1 = 0xcc9e2d51;
	const uint32_t c2 = 0x1b873593;
	const uint32_t m = 0xe6546b64;
	const uint32_t c1inv = 0xdee13bb1;
	const uint32_t c2inv = 0x56ed309b;
	//const uint32_t minv = 0xe6546b64;
	uint32_t k = h;
	uint32_t hf = seed;

	hf -= 0xe6546b64;
	hf /= 5;
	hf = invert_rotl32 (hf, 13);
	hf ^= k;

	h  = hf;
	k *= c2inv;
	k = invert_rotl32(k, 15);
	k *= c1inv;

#ifdef __BIG_ENDIAN
	unsigned char *data = (unsigned char *)&k;
	k = (data[0]) + (data[1] << 8) + (data[2] << 16) + (data[3] << 24);
#endif

	return k;
}

static inline uint64_t fmix64 (uint64_t k)
{
	k ^= k >> 33;
	k *= 0xff51afd7ed558ccdULL;
	k ^= k >> 33;
	k *= 0xc4ceb9fe1a85ec53ULL;
	k ^= k >> 33;

	return k;
}

uint64_t murmurhash3_64(const unsigned char *key, const int len,
			const uint64_t seed)
{
	const uint64_t m = 0xc6a4a7935bd1e995ULL;
	const int r = 47;

	uint64_t h = seed ^ (len * m);

	const uint64_t *data = (const uint64_t *)key;
	const uint64_t *end = data + (len / 8);

	while (data != end) {
#ifdef __BIG_ENDIAN
		uint64_t k = *data++;
		unsigned char *p = (unsigned char *)&k;
		unsigned char c;
		// clang-format off
		c = p[0]; p[0] = p[7]; p[7] = c;
		c = p[1]; p[1] = p[6]; p[6] = c;
		c = p[2]; p[2] = p[5]; p[5] = c;
		c = p[3]; p[3] = p[4]; p[4] = c;
		// clang-format on
#else
		uint64_t k = *data++;
#endif

		k *= m;
		k ^= k >> r;
		k *= m;

		h ^= k;
		h *= m;
	}

	const unsigned char *data2 = (const unsigned char *)data;

	switch (len & 7) {
	case 7: h ^= (uint64_t)data2[6] << 48;
	case 6: h ^= (uint64_t)data2[5] << 40;
	case 5: h ^= (uint64_t)data2[4] << 32;
	case 4: h ^= (uint64_t)data2[3] << 24;
	case 3: h ^= (uint64_t)data2[2] << 16;
	case 2: h ^= (uint64_t)data2[1] << 8;
	case 1: h ^= (uint64_t)data2[0]; h *= m;
	};

	h ^= h >> r;
	h *= m;
	h ^= h >> r;

	return h;
}

// Multiplicative inverse of x under % 2^64
// Or rather use openssl BN_mod_inverse(3)
uint64_t invert64(uint64_t x)
{
	uint64_t xx = x * x;
	while (xx != 1) {
		x *= xx;
		xx *= xx;
	}
	return x;
}

// invert h64 ^= h64 >> 24
static inline uint64_t invert_shift64_xor_2(uint64_t h, const int s)
{
	uint64_t tmp = h^h>>s;
	h = h^tmp>>s;
	return h;
}
// invert h64 ^= h64 >> 14
static inline uint64_t invert_shift64_xor_4(uint64_t h, const int s)
{
	uint64_t tmp = h^h>>s;
	tmp = h^tmp>>s;
	tmp = h^tmp>>s;
	h = h^tmp>>s;
	return h;
}

static inline uint64_t invert_fmix64 (uint64_t h)
{
	const uint64_t m1inv = 0x4f74430c22a54005ULL; // invert64(0xff51afd7ed558ccdULL);
	const uint64_t m2inv = 0x9cb4b2f8129337dbULL; // invert64(0xc4ceb9fe1a85ec53ULL);
	h = invert_shift_xor_2(h, 33);
	h *= m2inv;
	h = invert_shift_xor_2(h, 33);
	h *= m1inv;
	h = invert_shift_xor_2(h, 33);

	return h;
}

static inline uint64_t invert_rotl64 (uint64_t x, int8_t r )
{
	return (x >> r) | (x << (64 - r));
}

// Not even started
// fixed len = 8
uint64_t invert_murmurhash3_64(uint64_t h, const uint64_t seed)
{
	const uint64_t c1 = 0x87c37b91114253d5ULL;
	const uint64_t c2 = 0x4cf5ad432745937fULL;
	const uint64_t c1inv = 0xa98409e882ce4d7dULL; // invert64(c1);
	const uint64_t c2inv = 0xa81e14edd9de2c7fULL; // invert64(c2);
	// FIXME...
	const int r = 47;

	h ^= h >> r;
	h *= c2inv;
	h ^= h >> r;
	h *= c1inv;

	uint64_t hforward = seed ^ (8 * r);
	uint64_t k = h ^ hforward;

	k *= c2inv;
	k ^= k >> r;
	k *= c1inv;

#ifdef __BIG_ENDIAN
	unsigned char *p = (unsigned char *)&k;
	unsigned char c;
	// clang-format off
	c = p[0]; p[0] = p[7]; p[7] = c;
	c = p[1]; p[1] = p[6]; p[6] = c;
	c = p[2]; p[2] = p[5]; p[5] = c;
	c = p[3]; p[3] = p[4]; p[4] = c;
	// clang-format on
#endif

	return k;
}

void check_32()
{
	const int len = 4;
	const uint32_t m1 = 0x85ebca6b;
	const uint32_t m2 = 0xc2b2ae35;
#ifndef CBMC
	const uint32_t m1inv = invert32(m1);
	const uint32_t m2inv = invert32(m2);
	printf("invert32 constant m1 %lx => %lx\n", m1, m1inv);
	printf("invert32 constant m2 %lx => %lx\n", m2, m2inv);
	assert(m1inv == 0xa5cb9243);
	assert(m2inv == 0x7ed1b41d);
#endif

	const uint32_t c1 = 0xcc9e2d51;
	const uint32_t c2 = 0x1b873593;
	const uint32_t m = 0xe6546b64;
#ifndef CBMC
	const uint32_t c1inv = invert32(c1);
	const uint32_t c2inv = invert32(c2);
	//const uint32_t minv = 0xdee13bb1; // FIXME invert32(m); // this needs openssl
	printf("invert32 constant c1 %lx => %lx\n", c1, c1inv);
	printf("invert32 constant c2 %lx => %lx\n", c2, c2inv);
	//printf("invert32 constant m %lx => %lx ??\n", m, minv);
	assert(c1inv == 0xdee13bb1);
	assert(c2inv == 0x56ed309b);
#endif

	printf("invert_murmurhash3_32\n");
	for (uint32_t seed = 0; seed < 10U; seed++) {
		uint32_t key = invert_murmurhash3_32(0, seed); // FIXME
		unsigned char ckey[4];
		printf("%3d => %x\n", seed, key);
		memcpy(ckey, &key, len);
		printf("    => %x\n", murmurhash3_32(ckey, len, seed));
	}
#ifdef CBMC
	{
		// or just invert via a solver
		// usage: cbmc -DCBMC inverse/murmur3.c --trace
		// to find a count-example
		uint32_t key = uint32_det();
		uint32_t seed = uint32_det();
		assert(murmurhash3_32(&key, len, seed) != 0);
	}
#endif
}

void check_64(void)
{
	const int len = 8;
	const uint64_t m1 = 0xff51afd7ed558ccdULL;
	const uint64_t m2 = 0xc4ceb9fe1a85ec53ULL;
	const uint64_t m1inv = 0x4f74430c22a54005ULL; // invert64(0xff51afd7ed558ccdULL);
	const uint64_t m2inv = 0x9cb4b2f8129337dbULL; // invert64(0xc4ceb9fe1a85ec53ULL);
	uint64_t m1inv_calc = invert64(m1);
	uint64_t m2inv_calc = invert64(m2);
	printf("invert64 constant m1 %lx => %lx\n", m1, m1inv_calc);
	printf("invert64 constant m2 %lx => %lx\n", m2, m2inv_calc);
	assert(m1inv == m1inv_calc);
	assert(m2inv == m2inv_calc);

	const uint64_t c1 = 0x87c37b91114253d5ULL;
	const uint64_t c2 = 0x4cf5ad432745937fULL;
	const uint64_t c1inv = 0xa98409e882ce4d7dULL; // invert64(c1);
	const uint64_t c2inv = 0xa81e14edd9de2c7fULL; // invert64(c2);
	uint64_t c1inv_calc = invert64(c1);
	uint64_t c2inv_calc = invert64(c2);
	printf("invert64 constant c1 %lx => %lx\n", c1, c1inv_calc);
	printf("invert64 constant c2 %lx => %lx\n", c2, c2inv_calc);
	assert(c1inv == c1inv_calc);
	assert(c2inv == c2inv_calc);

	printf("invert_murmurhash3_64\n");
	for (uint64_t seed = 0; seed < 10ULL; seed++) {
		uint64_t key = invert_murmurhash3_64(0LL, seed);
		printf("%3d => %lx\n", seed, key);
		printf("    => %lx\n",
		       murmurhash3_64((const unsigned char *)&key, len, seed));
#ifdef CBMC
		// usage:
		// cbmc --64 -DCBMC --function check_64 inverse/murmur3.c --trace
		assert(murmurhash3_64(&key, len, seed) != 0ULL);
#endif
	}
}

int main(int argc, const char **argv)
{
	check_32();
#if (defined CBMC && defined BITS64) || !defined CBMC
	check_64();
#endif
	// TODO: check how many collisions to produce
}
