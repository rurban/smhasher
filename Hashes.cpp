#include "Hashes.h"
#include "Random.h"

#include <stdlib.h>
//#include <stdint.h>
#include <assert.h>
//#include <emmintrin.h>
//#include <xmmintrin.h>

// ----------------------------------------------------------------------------
//fake / bad hashes

void
BadHash_with_state(const void *key, int len, const void *state, void *out)
{
  uint32_t	  h = *((uint32_t *)state);
  const uint8_t  *data = (const uint8_t *)key;

  for (int i = 0; i < len; i++) {
    h ^= h >> 3;
    h ^= h << 5;
    h ^= data[i];
  }

  *(uint32_t *) out = h;
}

void
DoNothingHash_with_state(const void *, int, const void *, void *)
{
}

void
DoNothingHash_seed_state(int, const void *, void *)
{
}

void
NoopOAATReadHash_with_state(const void *key, int len, const void *state, void *out)
{
    volatile uint8_t c;
    const uint8_t *ptr = (uint8_t *)key;

    for(int i=0; i < len; i++)
    {
        c= ptr[i];
    }
}

//-----------------------------------------------------------------------------
//One - byte - at - a - time hash based on Murmur 's mix

uint32_t MurmurOAAT(const void *key, int len, uint32_t seed)
{
  const uint8_t  *data = (const uint8_t *)key;
  uint32_t	  h = seed;

  for (int i = 0; i < len; i++) {
    h ^= data[i];
    h *= 0x5bd1e995;
    h ^= h >> 15;
  }

  return h;
}

void
MurmurOAAT_with_state_test(const void *key, int len, const void *state, void *out)
{
  *(uint32_t *) out = MurmurOAAT(key, len, *((uint32_t*)state));
}
//----------------------------------------------------------------------------

void
FNV32a_with_state_test(const void *key, int len, const void *state, void *out)
{
  uint32_t  h = *((uint32_t *)state);
  const uint8_t  *data = (const uint8_t *)key;

  h ^= BIG_CONSTANT(2166136261);

  for (int i = 0; i < len; i++) {
    h ^= data[i];
    h *= 16777619;
  }

  *(uint32_t *) out = h;
}


void
FNV32a_YoshimitsuTRIAD_with_state_test(const void *key, int len, const void *state, void *out)
{
  const uint8_t  *p = (const uint8_t *)key;
  const uint32_t  PRIME = 709607;
  uint32_t	  hash32A = *((uint32_t*)state) ^ BIG_CONSTANT(2166136261);
  uint32_t	  hash32B = BIG_CONSTANT(2166136261) + len;
  uint32_t	  hash32C = BIG_CONSTANT(2166136261);

  for (; len >= 3 * 2 * sizeof(uint32_t); len -= 3 * 2 * sizeof(uint32_t), p += 3 * 2 * sizeof(uint32_t)) {
    hash32A = (hash32A ^ (ROTL32(*(uint32_t *) (p + 0), 5)  ^ *(uint32_t *) (p + 4)))  * PRIME;
    hash32B = (hash32B ^ (ROTL32(*(uint32_t *) (p + 8), 5)  ^ *(uint32_t *) (p + 12))) * PRIME;
    hash32C = (hash32C ^ (ROTL32(*(uint32_t *) (p + 16), 5) ^ *(uint32_t *) (p + 20))) * PRIME;
  }
  if (p != key) {
    hash32A = (hash32A ^ ROTL32(hash32C, 5)) * PRIME;
  }
  //Cases 0. .31
  if (len & 4 * sizeof(uint32_t)) {
    hash32A = (hash32A ^ (ROTL32(*(uint32_t *) (p + 0), 5) ^ *(uint32_t *) (p + 4))) * PRIME;
    hash32B = (hash32B ^ (ROTL32(*(uint32_t *) (p + 8), 5) ^ *(uint32_t *) (p + 12))) * PRIME;
    p += 8 * sizeof(uint16_t);
  }
  //Cases 0. .15
  if (len & 2 * sizeof(uint32_t)) {
    hash32A = (hash32A ^ *(uint32_t *) (p + 0)) * PRIME;
    hash32B = (hash32B ^ *(uint32_t *) (p + 4)) * PRIME;
    p += 4 * sizeof(uint16_t);
  }
  //Cases:0. .7
  if (len & sizeof(uint32_t)) {
    hash32A = (hash32A ^ *(uint16_t *) (p + 0)) * PRIME;
    hash32B = (hash32B ^ *(uint16_t *) (p + 2)) * PRIME;
    p += 2 * sizeof(uint16_t);
  }
  //Cases:0. .3
  if (len & sizeof(uint16_t)) {
    hash32A = (hash32A ^ *(uint16_t *) p) * PRIME;
    p += sizeof(uint16_t);
  }
  if (len & 1)
    hash32A = (hash32A ^ *p) * PRIME;

  hash32A = (hash32A ^ ROTL32(hash32B, 5)) * PRIME;
  *(uint32_t *) out = hash32A ^ (hash32A >> 16);
}

void
FNV64a_with_state_test(const void *key, int len, const void *state, void *out)
{
  uint64_t	  h = *((uint64_t*) state);
  const uint8_t  *data = (const uint8_t *)key;

  h ^= BIG_CONSTANT(0xcbf29ce484222325);

  for (int i = 0; i < len; i++) {
    h ^= data[i];
    h *= 0x100000001b3ULL;
  }

  *(uint64_t *) out = h;
}

//-----------------------------------------------------------------------------

uint32_t x17(const void *key, int len, uint32_t h)
{
  const uint8_t  *data = (const uint8_t *)key;

  for (int i = 0; i < len; ++i) {
    h = 17 * h + (data[i] - ' ');
  }

  return h ^ (h >> 16);
}

void
x17_test(const void *key, int len, const void *state, void *out)
{
  *(uint32_t *) out = x17(key, len, *((uint32_t*)state));
}

//-----------------------------------------------------------------------------

//also used in perl5 as djb2
void
Bernstein_with_state(const void *key, int len, const void *state, void *out)
{
  const uint8_t  *data = (const uint8_t *)key;
  uint32_t hash = *((uint32_t *)state);

  for (int i = 0; i < len; ++i) {
    //hash = ((hash << 5) + hash) + data[i];
    hash = 33 * hash + data[i];
  }

  *(uint32_t *) out = hash;
}

//as used in perl5
void
sdbm(const void *key, int len, const void *state, void *out)
{
  unsigned char  *str = (unsigned char *)key;
  const unsigned char *const end = (const unsigned char *)str + len;
  uint32_t hash= *((uint32_t*)state) + len;
  while (str < end) {
    hash = (hash << 6) + (hash << 16) - hash + *str++;
  }
  *(uint32_t *) out = hash;
}

//as used in perl5 as one_at_a_time_hard
void
JenkinsOAATH_with_state(const void *key, int len, const void *state, void *out)
{
  unsigned char  *str = (unsigned char *)key;
  const unsigned char *const end = (const unsigned char *)str + len;
  unsigned char  *seed = (unsigned char *)state;
  uint32_t       hash = *((uint32_t *)state) + len;
  //unsigned char seed[8];
  //note that perl5 adds part of the seed to the end of key, to frustrate
  //key extension attacks, and because the last byte or so of the key is
  //not well mixed into the final hash. Mixing four additional bytes
  //ensures that the real key has had an opportunity to affect every bit
  //of the output, and that the seed is well mixed before we return the
  //hash value. Older versions of this hash had a trivial seed discovery
  //attack, which the new one is robust to. Regardless, this is not a good
  //choice of hash function these days.
  while (str < end) {
    hash += (hash << 10);
    hash ^= (hash >> 6);
    hash += *str++;
  }

  hash += (hash << 10);
  hash ^= (hash >> 6);
  hash += seed[4];

  hash += (hash << 10);
  hash ^= (hash >> 6);
  hash += seed[5];

  hash += (hash << 10);
  hash ^= (hash >> 6);
  hash += seed[6];

  hash += (hash << 10);
  hash ^= (hash >> 6);
  hash += seed[7];

  hash += (hash << 10);
  hash ^= (hash >> 6);

  hash += (hash << 3);
  hash ^= (hash >> 11);
  hash = hash + (hash << 15);
  *(uint32_t *) out = hash;
}

//as used in perl5 until 5.17(one_at_a_time_old)
void JenkinsOAAT_with_state(const void *key, int len, const void *seed, void *out)
{
  unsigned char  *str = (unsigned char *)key;
  const unsigned char *const end = (const unsigned char *)str + len;
  uint32_t hash = *((uint32_t *)seed);
  while (str < end) {
    hash += *str++;
    hash += (hash << 10);
    hash ^= (hash >> 6);
  }
  hash += (hash << 3);
  hash ^= (hash >> 11);
  hash = hash + (hash << 15);
  *(uint32_t *) out = hash;
}

void JenkinsOAAT(const void *key, int len, uint32_t hash, void *out)
{
    JenkinsOAAT_with_state(key, len, &hash, out);
}

//------------------------------------------------
// One of a smallest non-multiplicative One-At-a-Time function
// that passes whole SMHasher. (The old one anyway, it fails test
// on the new one. */
// Author: Sokolov Yura aka funny-falcon <funny.falcon@gmail.com>
void GoodOAAT(const void *key, int len, const void *state, void *out) {
#define grol(x,n) (((x)<<(n))|((x)>>(32-(n))))
#define gror(x,n) (((x)>>(n))|((x)<<(32-(n))))
  unsigned char  *str = (unsigned char *)key;
  const unsigned char *const end = (const unsigned char *)str + len;
  uint32_t seed= *((uint32_t*)state);
  uint32_t h1 = seed ^ 0x3b00;
  uint32_t h2 = grol(seed, 15);
  for (;str != end; str++) {
    h1 += str[0];
    h1 += h1 << 3; // h1 *= 9
    h2 += h1;
    // the rest could be as in MicroOAAT: h1 = grol(h1, 7)
    // but clang doesn't generate ROTL instruction then.
    h2 = grol(h2, 7);
    h2 += h2 << 2; // h2 *= 5
  }
  h1 ^= h2;
  /* now h1 passes all collision checks,
   * so it is suitable for hash-tables with prime numbers. */
  h1 += grol(h2, 14);
  h2 ^= h1; h2 += gror(h1, 6);
  h1 ^= h2; h1 += grol(h2, 5);
  h2 ^= h1; h2 += gror(h1, 8);
  *(uint32_t *) out = h2;
#undef grol
#undef gror
}

// MicroOAAT suitable for hash-tables using prime numbers.
// It passes all collision checks.
// Author: Sokolov Yura aka funny-falcon <funny.falcon@gmail.com>
void MicroOAAT(const void *key, int len, const void *state, void *out) {
#define grol(x,n) (((x)<<(n))|((x)>>(32-(n))))
#define gror(x,n) (((x)>>(n))|((x)<<(32-(n))))
  unsigned char  *str = (unsigned char *)key;
  const unsigned char *const end = (const unsigned char *)str + len;
  uint32_t seed = *((uint32_t*)state);
  uint32_t h1 = seed ^ 0x3b00;
  uint32_t h2 = grol(seed, 15);
  for (;str != end; str++) {
    h1 += str[0];
    h1 += h1 << 3; // h1 *= 9
    h2 -= h1;
    // unfortunately, clang produces bad code here,
    // cause it doesn't generate rotl instruction.
    h1 = grol(h1, 7);
  }
  *(uint32_t *) out = h1 ^ h2;
#undef grol
#undef gror
}

//-----------------------------------------------------------------------------
//Crap8 hash from http://www.team5150.com / ~andrew / noncryptohashzoo / Crap8.html

uint32_t Crap8(const uint8_t * key, uint32_t len, uint32_t seed)
{
#define c8fold( a, b, y, z ) { p = (uint32_t)(a) * (uint64_t)(b); y ^= (uint32_t)p; z ^= (uint32_t)(p >> 32); }
#define c8mix( in ) { h *= m; c8fold( in, m, k, h ); }

  const uint32_t  m = 0x83d2e73b, n = 0x97e1cc59, *key4 = (const uint32_t *)key;
  uint32_t	  h = len + seed, k = n + len;
  uint64_t	  p;

  while (len >= 8) {
    c8mix(key4[0]) c8mix(key4[1]) key4 += 2;
    len -= 8;
  }
  if (len >= 4) {
    c8mix(key4[0]) key4 += 1;
    len -= 4;
  }
  if (len) {
    c8mix(key4[0] & ((1 << (len * 8)) - 1))
  }
  c8fold(h ^ k, n, k, k)
    return k;
}

void
Crap8_test(const void *key, int len, const void *state, void *out)
{
  *(uint32_t *) out = Crap8((const uint8_t *)key, len, *((uint32_t *)state));
}

extern		"C" {
#ifdef __SSE2__
  void		  hasshe2 (const void *input, int len, const void *state, void *out);
#endif
#if defined(__SSE4_2__) && defined(__x86_64__)
  uint32_t	  crc32c_hw(const void *input, int len, uint32_t seed);
  uint32_t	  crc32c(const void *input, int len, uint32_t seed);
  uint64_t	  crc64c_hw(const void *input, int len, uint64_t seed);
#endif
}

#ifdef __SSE2__
void
hasshe2_test(const void *input, int len, const void *state, void *out)
{
  hasshe2(input, len, state, out);
}
#endif

#if defined(__SSE4_2__) && (defined(__i686__) || defined(_M_IX86) || defined(__x86_64__))
/* Compute CRC-32C using the Intel hardware instruction.
   TODO: arm8
 */
void
crc32c_hw_with_state_test(const void *input, int len, const void *state, void *out)
{
  if (!len) {
    *(uint32_t *) out = 0;
    return;
  }
  *(uint32_t *) out = crc32c_hw(input, len, *((uint32_t*)state));
}

/* Faster Adler SSE4.2 crc32 in HW */
void
crc32c_hw1_with_state_test(const void *input, int len, const void *state, void *out)
{
  if (!len) {
    *(uint32_t *) out = 0;
    return;
  }
  *(uint32_t *) out = crc32c(input, len, *((uint32_t*)state));
}

#if defined(__SSE4_2__) && defined(__x86_64__)
/* Compute CRC-64C using the Intel hardware instruction. */
void
crc64c_hw_with_state_test(const void *input, int len, const void *state, void *out)
{
  if (!len) {
    *(uint64_t *) out = 0;
    return;
  }
  *(uint64_t *) out = crc64c_hw(input, len, *((uint64_t*)state));
}

#endif
#endif

#include "siphash.h"
/* https://github.com/floodyberry/siphash */

void
siphash_seed_state_test(int seedbits, const void *seed, void *state)
{
    siphash_seed_state((unsigned char *)seed, (unsigned char *)state);
}

void
siphash_with_state_test(const void *input, int len, const void *seed, void *out)
{
  *(uint64_t *) out = siphash_with_state((const unsigned char *)seed, (const unsigned char *)input, (size_t) len);
}

void
siphash13_with_state_test(const void *input, int len, const void *seed, void *out)
{
  *(uint64_t *) out = siphash13_with_state((const unsigned char *)seed, (const unsigned char *)input, (size_t) len);
}

void
halfsiphash_seed_state_test(int seed_bits, const void * seed, void *state)
{
    halfsiphash_seed_state((const unsigned char *)seed,(unsigned char *)state);
}

void
halfsiphash_with_state_test(const void *key, int len, const void * state, void *out)
{
  *(uint32_t *) out = halfsiphash_with_state((const unsigned char *)state, (const unsigned char *)key, (size_t) len);
}


/* https://github.com/gamozolabs/falkhash */
#if defined(__SSE4_2__) && defined(__x86_64__)
extern "C" {
  uint64_t falkhash_test(uint8_t *data, uint64_t len, uint64_t seed, void *out);
}

void
falkhash_with_state_test_cxx(const void *input, int len, const void *seed, void *out)
{
  uint64_t hash[2] = {0ULL, 0ULL};
  falkhash_test((uint8_t *)input, (uint64_t)len, *((uint64_t *)seed), hash);
  *(uint64_t *) out = hash[0];
}
#endif

void lua_v53_string_hash_oaat (const void *key, int len, const void *seed, void *out) {
  uint8_t *str= (uint8_t *)key+len-1;
  unsigned int h = *((uint32_t*)seed) ^ ((unsigned int)len);
  for (; key <= str ; str--)
    h ^= ((h<<5) + (h>>2) + *str);
  *((uint32_t*)out)=h;
}

void lua_v53_string_hash (const void *key, int len, const void *seed, void *out) {
  uint8_t *str= (uint8_t *)key;
  unsigned int h = *((uint32_t*)seed) ^ ((unsigned int)len);
  size_t step = (len >> 5) + 1;
  for (; len >= step; len -= step)
    h ^= ((h<<5) + (h>>2) + str[len-1]);
  *((uint32_t*)out)=h;
}
