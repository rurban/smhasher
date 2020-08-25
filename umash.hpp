#pragma once

#include "umash.h"

namespace {

struct umash_params make_umash_params()
{
	struct umash_params ret;

	umash_params_derive(&ret, 0, NULL);
	return ret;
}
  
static struct umash_params umash_params = make_umash_params();

//void umash_init()
//{
//  umash_params = make_umash_params();
//}

void umash_seed_init(size_t seed)
{
  umash_params_derive (&umash_params, (uint64_t)seed, NULL);
}

void umash(const void *blob, int len, uint32_t seed, void *out)
{
	uint64_t hash;

	hash = umash_full(&umash_params, seed, 0, blob, len);
	memcpy(out, &hash, sizeof(hash));
	return;
}

void umash32(const void *blob, int len, uint32_t seed, void *out)
{
	uint32_t hash;

	hash = umash_full(&umash_params, seed, 0, blob, len);
	memcpy(out, &hash, sizeof(hash));
	return;
}

void umash32_hi(const void *blob, int len, uint32_t seed, void *out)
{
	uint32_t hash;

	hash = umash_full(&umash_params, seed, 0, blob, len) >> 32;
	memcpy(out, &hash, sizeof(hash));
	return;
}

void umash128(const void *blob, int len, uint32_t seed, void *out)
{
	struct umash_fp hash;

	hash = umash_fprint(&umash_params, seed, blob, len);
	memcpy(out, &hash, sizeof(hash));
	return;
}
}  // namespace
