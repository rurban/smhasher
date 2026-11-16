/* pengyhash v0.2 */

#include "pengyhash.h"

#if defined(__BYTE_ORDER__) && __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
static uint64_t pengy_swap64(uint64_t x)
{
	return ((x & UINT64_C(0x00000000000000ff)) << 56) |
	       ((x & UINT64_C(0x000000000000ff00)) << 40) |
	       ((x & UINT64_C(0x0000000000ff0000)) << 24) |
	       ((x & UINT64_C(0x00000000ff000000)) << 8) |
	       ((x & UINT64_C(0x000000ff00000000)) >> 8) |
	       ((x & UINT64_C(0x0000ff0000000000)) >> 24) |
	       ((x & UINT64_C(0x00ff000000000000)) >> 40) |
	       ((x & UINT64_C(0xff00000000000000)) >> 56);
}
#endif

uint64_t pengyhash(const void *p, size_t size, uint32_t seed)
{
	uint64_t b[4] = { 0 };
	uint64_t s[4] = { 0, 0, 0, size };
	int i;

	for(; size >= 32; size -= 32, p = (const char*)p + 32) {
		memcpy(b, p, 32);
#if defined(__BYTE_ORDER__) && __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
		for (i = 0; i < 4; ++i) b[i] = pengy_swap64(b[i]);
#endif
		
		s[1] = (s[0] += s[1] + b[3]) + (s[1] << 14 | s[1] >> 50);
		s[3] = (s[2] += s[3] + b[2]) + (s[3] << 23 | s[3] >> 41);
		s[3] = (s[0] += s[3] + b[1]) ^ (s[3] << 16 | s[3] >> 48);
		s[1] = (s[2] += s[1] + b[0]) ^ (s[1] << 40 | s[1] >> 24);
	}

#if defined(__BYTE_ORDER__) && __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
	for (i = 0; i < 4; ++i) b[i] = pengy_swap64(b[i]);
#endif
	memcpy(b, p, size);
#if defined(__BYTE_ORDER__) && __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
	for (i = 0; i < 4; ++i) b[i] = pengy_swap64(b[i]);
#endif

	for(i = 0; i < 6; i++) {
		s[1] = (s[0] += s[1] + b[3]) + (s[1] << 14 | s[1] >> 50) + seed;
		s[3] = (s[2] += s[3] + b[2]) + (s[3] << 23 | s[3] >> 41);
		s[3] = (s[0] += s[3] + b[1]) ^ (s[3] << 16 | s[3] >> 48);
		s[1] = (s[2] += s[1] + b[0]) ^ (s[1] << 40 | s[1] >> 24);
	}

	return s[0] + s[1] + s[2] + s[3];
}
