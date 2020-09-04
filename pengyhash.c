/* pengyhash v0.2 */

#include "pengyhash.h"

uint64_t pengyhash(const void *p, size_t size, uint32_t seed)
{
	uint64_t b[4] = { 0 };
	uint64_t s[4] = { 0, 0, 0, size };
	int i;

	for(; size >= 32; size -= 32, p = (const char*)p + 32) {
		memcpy(b, p, 32);
		
		s[1] = (s[0] += s[1] + b[3]) + (s[1] << 14 | s[1] >> 50);
		s[3] = (s[2] += s[3] + b[2]) + (s[3] << 23 | s[3] >> 41);
		s[3] = (s[0] += s[3] + b[1]) ^ (s[3] << 16 | s[3] >> 48);
		s[1] = (s[2] += s[1] + b[0]) ^ (s[1] << 40 | s[1] >> 24);
	}

	memcpy(b, p, size);

	for(i = 0; i < 6; i++) {
		s[1] = (s[0] += s[1] + b[3]) + (s[1] << 14 | s[1] >> 50) + seed;
		s[3] = (s[2] += s[3] + b[2]) + (s[3] << 23 | s[3] >> 41);
		s[3] = (s[0] += s[3] + b[1]) ^ (s[3] << 16 | s[3] >> 48);
		s[1] = (s[2] += s[1] + b[0]) ^ (s[1] << 40 | s[1] >> 24);
	}

	return s[0] + s[1] + s[2] + s[3];
}
