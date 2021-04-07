// -*-  c-default-style: "bsd"; indent-tabs-mode: t; c-basic-offset: 8 -*-
// http://bitsquid.blogspot.com/2011/08/code-snippet-murmur-hash-inverse-pre.html

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
#define XENSURE assert

// MurmurHash2, 32bit
uint32_t murmurhash2_32(const unsigned char *key, int len, const uint32_t seed)
{
	// 'm' and 'r' are mixing constants generated offline.
	// They're not really 'magic', they just happen to work well.
	const uint32_t m = 0x5bd1e995;
	const uint32_t r = 24;

	// Initialize the hash to a 'random' value
	uint32_t h = seed ^ len;

	// Mix 4 bytes at a time into the hash
	const unsigned char *data = (const unsigned char *)key;

	while (len >= 4) {
#ifdef __BIG_ENDIAN__
		uint32_t k = (data[0]) + (data[1] << 8) + (data[2] << 16) +
			     (data[3] << 24);
#else
		uint32_t k = *(uint32_t *)data;
#endif

		k *= m;
		k ^= k >> r;
		k *= m;

		h *= m;
		h ^= k;

		data += 4;
		len -= 4;
	}

	// Handle the last few bytes of the input array
	switch (len) {
	case 3: h ^= data[2] << 16;
	case 2: h ^= data[1] << 8;
	case 1: h ^= data[0]; h *= m;
	};

	// Do a few final mixes of the hash to ensure the last few
	// bytes are well-incorporated.
	h ^= h >> 13;
	h *= m;
	h ^= h >> 15;

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
	XENSURE(s >= 8 && s <= 16);
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

// fixed len = 4
uint32_t invert_murmurhash2_32(uint32_t h, uint32_t const seed)
{
	const int len = 4;
	const uint32_t m = 0x5bd1e995;
	const uint32_t minv = 0xe59b19bd; // = inverse32(m)
	const uint32_t r = 24;

	h = invert_shift_xor(h, 15);
	h *= minv;
	h = invert_shift_xor(h, 13);

	uint32_t hforward = seed ^ len;
	hforward *= m;
	uint32_t k = hforward ^ h;
	k *= minv;
	k ^= k >> r;
	k *= minv;

#ifdef __BIG_ENDIAN
	unsigned char *data = (unsigned char *)&k;
	k = (data[0]) + (data[1] << 8) + (data[2] << 16) + (data[3] << 24);
#endif

	return k;
}

uint64_t murmurhash2_64(const unsigned char *key, const int len,
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
		char *p = (char *)&k;
		char c;
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

// fixed len = 8
uint64_t invert_murmurhash2_64(uint64_t h, const uint64_t seed)
{
	const uint64_t m = 0xc6a4a7935bd1e995ULL;
	const uint64_t minv = 0x5f7a0ea7e59b19bdULL; // = inverse64(m)
	const int r = 47;

	h ^= h >> r;
	h *= minv;
	h ^= h >> r;
	h *= minv;

	uint64_t hforward = seed ^ (8 * m);
	uint64_t k = h ^ hforward;

	k *= minv;
	k ^= k >> r;
	k *= minv;

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
	const uint32_t m = 0x5bd1e995;
	// Multiplicative inverse of m under % 2^64
	const uint32_t minv = 0xe59b19bd;
#ifndef CBMC
	uint32_t minv_calc = invert32(m);
	printf("invert32 constant m %lx => %lx\n", m, minv_calc);
	assert(minv == minv_calc);
#endif
	printf("invert_murmurhash2_32\n");
	for (uint32_t seed = 0; seed < 10U; seed++) {
		uint32_t key = invert_murmurhash2_32(0, seed); // FIXME
		unsigned char ckey[4];
		printf("%3d => %x\n", seed, key);
		memcpy(ckey, &key, len);
		printf("    => %x\n", murmurhash2_32(ckey, len, seed));
#ifdef CBMC
		// or just invert via a solver
		// usage: cbmc -DCBMC inverse/murmur2.c --trace
		// to find a count-example
		assert(murmurhash2_32(ckey, len, seed) != 0);
#endif
	}
}

void check_64(void)
{
	const int len = 8;
	const uint64_t m = 0xc6a4a7935bd1e995ULL;
	// Multiplicative inverse of m under % 2^64
	const uint64_t minv = 0x5f7a0ea7e59b19bdULL;
#ifndef CBMC
	uint64_t minv_calc = invert64(m);
	printf("invert64 constant m %lx => %lx\n", m, minv_calc);
	assert(minv == minv_calc);
#endif
	printf("invert_murmurhash2_64\n");
	for (uint64_t seed = 0; seed < 10ULL; seed++) {
		uint64_t key = invert_murmurhash2_64(0LL, seed);
		printf("%3d => %lx\n", seed, key);
		printf("    => %lx\n",
		       murmurhash2_64((const unsigned char *)&key, len, seed));
	}
#ifdef CBMC
	{
		// or just invert via a solver
		// usage: cbmc -DCBMC inverse/murmur3.c --trace
		// to find a count-example
		uint32_t key = uint32_det();
		uint32_t seed = uint32_det();
		assert(murmurhash2_32(&key, len, seed) != 0);
	}
#endif
}

int main(int argc, const char **argv)
{
	check_32();
#if (defined CBMC && defined BITS64) || !defined CBMC
	check_64();
#endif
	// TODO: check how many collisions to produce
}
