#include "siphash_impl.h"

/* 0,2,1,3 */
static const packedelem64 siphash_init[2] = {
	{{0x736f6d6570736575ull,0x6c7967656e657261ull}},
	{{0x646f72616e646f6dull,0x7465646279746573ull}}
};

static const packedelem64 siphash_final = {
	{0x0000000000000000ull,0x00000000000000ffull}
};

uint64_t
siphash(const unsigned char key[16], const unsigned char *m, size_t len) {
	xmmi k,v02,v20,v13,v11,v33,mi;
	uint64_t last7;
	uint32_t lo, hi;
	size_t i, blocks;

	k = _mm_loadu_si128((xmmi *)(key + 0));
	v02 = siphash_init[0].v;
	v13 = siphash_init[1].v;
	v02 = _mm_xor_si128(v02, _mm_unpacklo_epi64(k, k));
	v13 = _mm_xor_si128(v13, _mm_unpackhi_epi64(k, k));

	last7 = (uint64_t)(len & 0xff) << 56;

#define sipcompress() \
	v11 = v13; \
	v33 = _mm_shuffle_epi32(v13, _MM_SHUFFLE(1,0,3,2)); \
	v11 = _mm_or_si128(_mm_slli_epi64(v11, 13), _mm_srli_epi64(v11, 64-13)); \
	v02 = _mm_add_epi64(v02, v13); \
	v33 = _mm_or_si128(_mm_slli_epi64(v33, 16), _mm_srli_epi64(v33, 64-16)); \
	v13 = _mm_unpacklo_epi64(v11, v33); \
	v13 = _mm_xor_si128(v13, v02); \
	v20 = _mm_shuffle_epi32(v02, _MM_SHUFFLE(0,1,3,2)); \
	v11 = v13; \
	v33 = _mm_shuffle_epi32(v13, _MM_SHUFFLE(1,0,3,2)); \
	v11 = _mm_or_si128(_mm_slli_epi64(v11, 17), _mm_srli_epi64(v11, 64-17)); \
	v20 = _mm_add_epi64(v20, v13); \
	v33 = _mm_or_si128(_mm_slli_epi64(v33, 21), _mm_srli_epi64(v33, 64-21)); \
	v13 = _mm_unpacklo_epi64(v11, v33); \
	v13 = _mm_unpacklo_epi64(v11, v33); \
	v02 = _mm_shuffle_epi32(v20, _MM_SHUFFLE(0,1,3,2)); \
	v13 = _mm_xor_si128(v13, v20);

	for (i = 0, blocks = (len & ~7); i < blocks; i += 8) {
		mi = _mm_loadl_epi64((xmmi *)(m + i));
		v13 = _mm_xor_si128(v13, _mm_slli_si128(mi, 8));
		sipcompress()
		sipcompress()
		v02 = _mm_xor_si128(v02, mi);
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

	mi = _mm_unpacklo_epi32(_mm_cvtsi32_si128((uint32_t)last7),_mm_cvtsi32_si128((uint32_t)(last7 >> 32)));
	v13 = _mm_xor_si128(v13, _mm_slli_si128(mi, 8));
	sipcompress()
	sipcompress()
	v02 = _mm_xor_si128(v02, mi);
	v02 = _mm_xor_si128(v02, siphash_final.v);
	sipcompress()
	sipcompress()
	sipcompress()
	sipcompress()

	v02 = _mm_xor_si128(v02, v13);
	v02 = _mm_xor_si128(v02, _mm_shuffle_epi32(v02, _MM_SHUFFLE(1,0,3,2)));
	lo = _mm_cvtsi128_si32(v02);
	hi = _mm_cvtsi128_si32(_mm_srli_si128(v02, 4));
	return ((uint64_t)hi << 32) | lo;
}

uint64_t
siphash13(const unsigned char key[16], const unsigned char *m, size_t len) {
	xmmi k,v02,v20,v13,v11,v33,mi;
	uint64_t last7;
	uint32_t lo, hi;
	size_t i, blocks;

	k = _mm_loadu_si128((xmmi *)(key + 0));
	v02 = siphash_init[0].v;
	v13 = siphash_init[1].v;
	v02 = _mm_xor_si128(v02, _mm_unpacklo_epi64(k, k));
	v13 = _mm_xor_si128(v13, _mm_unpackhi_epi64(k, k));

	last7 = (uint64_t)(len & 0xff) << 56;

	for (i = 0, blocks = (len & ~7); i < blocks; i += 8) {
		mi = _mm_loadl_epi64((xmmi *)(m + i));
		v13 = _mm_xor_si128(v13, _mm_slli_si128(mi, 8));
		sipcompress()
		v02 = _mm_xor_si128(v02, mi);
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

	mi = _mm_unpacklo_epi32(_mm_cvtsi32_si128((uint32_t)last7),_mm_cvtsi32_si128((uint32_t)(last7 >> 32)));
	v13 = _mm_xor_si128(v13, _mm_slli_si128(mi, 8));
	sipcompress()
	v02 = _mm_xor_si128(v02, mi);
	v02 = _mm_xor_si128(v02, siphash_final.v);
	sipcompress()
	sipcompress()
	sipcompress()

	v02 = _mm_xor_si128(v02, v13);
	v02 = _mm_xor_si128(v02, _mm_shuffle_epi32(v02, _MM_SHUFFLE(1,0,3,2)));
	lo = _mm_cvtsi128_si32(v02);
	hi = _mm_cvtsi128_si32(_mm_srli_si128(v02, 4));
	return ((uint64_t)hi << 32) | lo;
}

#include "halfsiphash.c"
