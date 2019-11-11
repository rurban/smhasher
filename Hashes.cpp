#include "Hashes.h"
#include "Random.h"

#include <stdlib.h>
//#include <stdint.h>
#include <assert.h>
//#include <emmintrin.h>
//#include <xmmintrin.h>

// ----------------------------------------------------------------------------
//fake / bad hashes

// objsize: 0x2f-0x0: 47
void
BadHash(const void *key, int len, uint32_t seed, void *out)
{
  uint32_t	  h = seed;
  const uint8_t  *data = (const uint8_t *)key;
  const uint8_t *const end = &data[len];

  while (data < end) {
    h ^= h >> 3;
    h ^= h << 5;
    h ^= *data++;
  }

  *(uint32_t *) out = h;
}

// objsize: 0x19b-0x30: 363
void
sumhash(const void *key, int len, uint32_t seed, void *out)
{
  uint32_t	 h = seed;
  const uint8_t *data = (const uint8_t *)key;
  const uint8_t *const end = &data[len];

  while (data < end) {
    h += *data++;
  }

  *(uint32_t *) out = h;
}

// objsize: 0x4ff-0x1a0: 863
void
sumhash32(const void *key, int len, uint32_t seed, void *out)
{
  uint32_t	  h = seed;
  const uint32_t *data = (const uint32_t *)key;
  const uint32_t *const end = &data[len/4];

  while (data < end) {
    h += *data++;
  }
  if (len & 3) {
    uint8_t *dc = (uint8_t*)data; //byte stepper
    const uint8_t *const endc = &((const uint8_t*)key)[len];
    while (dc < endc) {
      h += *dc++ * BIG_CONSTANT(11400714819323198485);
    }
  }

  *(uint32_t *) out = h;
}

// objsize: 0x50d-0x500: 13
void
DoNothingHash(const void *, int, uint32_t, void *)
{
}

// objsize: 0x53f-0x510: 47
void
NoopOAATReadHash(const void *key, int len, uint32_t seed, void *out)
{
  uint32_t	 h = seed;
  const uint8_t *data = (const uint8_t *)key;
  const uint8_t *const end = &data[len];

  while (data < end) {
    h = *data++;
  }
  *(uint32_t *) out = h;
}

//-----------------------------------------------------------------------------
//One - byte - at - a - time hash based on Murmur 's mix

// objsize: 0x540-0x56f: 47
uint32_t MurmurOAAT(const void *key, int len, uint32_t seed)
{
  uint32_t	 h = seed;
  const uint8_t *data = (const uint8_t *)key;
  const uint8_t *const end = &data[len];

  while (data < end) {
    h ^= *data++;
    h *= 0x5bd1e995;
    h ^= h >> 15;
  }

  return h;
}

void
MurmurOAAT_test(const void *key, int len, uint32_t seed, void *out)
{
  *(uint32_t *) out = MurmurOAAT(key, len, seed);
}

//----------------------------------------------------------------------------

// objsize: 0x5a0-0xc3c: 1692
void
fibonacci(const void *key, int len, uint32_t seed, void *out)
{
  size_t h = (size_t)seed;
  size_t *dw = (size_t *)key; //word stepper
  const size_t *const endw = &((const size_t*)key)[len/sizeof(size_t)];
  while (dw < endw) {
    h += *dw++ * BIG_CONSTANT(11400714819323198485);
  }
  if (len & (sizeof(size_t)-1)) {
    uint8_t *dc = (uint8_t*)dw; //byte stepper
    const uint8_t *const endc = &((const uint8_t*)key)[len];
    while (dc < endc) {
      h += *dc++ * BIG_CONSTANT(11400714819323198485);
    }
  }
  *(size_t *) out = h;
}

// objsize: 0xc40-0xd56: 278
void
FNV2(const void *key, int len, uint32_t seed, void *out)
{
  size_t h = (size_t)seed;
  size_t *dw = (size_t *)key; //word stepper
  const size_t *const endw = &((const size_t*)key)[len/sizeof(size_t)];
#ifdef HAVE_BIT32
  h ^= BIG_CONSTANT(2166136261);
#else
  h ^= BIG_CONSTANT(0xcbf29ce484222325);
#endif
  while (dw < endw) {
    h ^= *dw++;
#ifdef HAVE_BIT32
    h *= BIG_CONSTANT(16777619);
#else
    h *= BIG_CONSTANT(0x100000001b3);
#endif
  }
  if (len & (sizeof(size_t)-1)) {
    uint8_t *dc = (uint8_t*)dw; //byte stepper
    const uint8_t *const endc = &((const uint8_t*)key)[len];
    while (dc < endc) {
      h ^= *dc++;
#ifdef HAVE_BIT32
      h *= BIG_CONSTANT(16777619);
#else
      h *= BIG_CONSTANT(0x100000001b3);
#endif
    }
  }
  *(size_t *) out = h;
}

// i.e. FNV1a
// objsize: 0xd60-0xe2c: 204
void
FNV32a(const void *key, int len, uint32_t seed, void *out)
{
  unsigned int	  h = seed;
  const uint8_t  *data = (const uint8_t *)key;

  h ^= BIG_CONSTANT(2166136261);

  for (int i = 0; i < len; i++) {
    h ^= data[i];
    h *= 16777619;
  }

  *(uint32_t *) out = h;
}

// objsize: 0xe30-0xf71: 321
void
FNV32a_YoshimitsuTRIAD(const void *key, int len, uint32_t seed, void *out)
{
  const uint8_t  *p = (const uint8_t *)key;
  const uint32_t  PRIME = 709607;
  uint32_t	  hash32A = seed ^ BIG_CONSTANT(2166136261);
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

#ifdef HAVE_INT64
// objsize: 0xf80-0x108e: 270
void FNV1A_Totenschiff(const void *key, int len, uint32_t seed, void *out)
{
#define _PADr_KAZE(x, n) (((x) << (n)) >> (n))

  const char *p = (char *)key;
  const uint32_t PRIME = 591798841;
  uint32_t hash32;
  uint64_t hash64 = (uint64_t)seed ^ BIG_CONSTANT(14695981039346656037);
  uint64_t PADDEDby8;

  for (; len > 8; len -= 8, p += 8) {
    PADDEDby8 = *(uint64_t *)(p + 0);
    hash64 = (hash64 ^ PADDEDby8) * PRIME;
  }

  // Here len is 1..8. when (8-8) the QWORD remains intact
  PADDEDby8 = _PADr_KAZE(*(uint64_t *)(p + 0), (8 - len) << 3);
  hash64 = (hash64 ^ PADDEDby8) * PRIME;

  hash32 = (uint32_t)(hash64 ^ (hash64 >> 32));
  *(uint32_t *)out = hash32 ^ (hash32 >> 16);
#undef _PADr_KAZE
}

// objsize: 0x1090-0x10df: 79
void
FNV64a(const void *key, int len, uint32_t seed, void *out)
{
  uint64_t  h = (uint64_t) seed & 0xFFFFFFFF;
  uint8_t  *data = (uint8_t *)key;
  const uint8_t *const end = &data[len];

  h ^= BIG_CONSTANT(0xcbf29ce484222325);
  while (data < end) {
    h ^= *data++;
    h *= BIG_CONSTANT(0x100000001b3);
  }

  *(uint64_t *) out = h;
}
#endif

//-----------------------------------------------------------------------------

// objsize: 0x1090-0x10df: 79
static inline
uint32_t x17(const void *key, int len, uint32_t h)
{
  uint8_t *data = (uint8_t *)key;
  const uint8_t *const end = &data[len];

  while (data < end) {
    h = 17 * h + (*data++ - ' ');
  }
  return h ^ (h >> 16);
}

void
x17_test(const void *key, int len, uint32_t seed, void *out)
{
  *(uint32_t *) out = x17(key, len, seed);
}

//64bit, ZFS
//note the original fletcher2 assumes 128bit aligned data, and
//can hereby advance the inner loop by 2 64bit words.
//both fletcher's return 4 words, 256 bit. Both are nevertheless very weak hashes.
// objsize: 0x1120-0x1218: 248
void
fletcher2(const void *key, int len, uint32_t seed, void *out)
{
  uint64_t *dataw = (uint64_t *)key;
  const uint64_t *const endw = &((const uint64_t*)key)[len/8];
  uint64_t A = seed, B = 0;
  for (; dataw < endw; dataw++) {
    A += *dataw;
    B += A;
  }
  if (len & 7) {
    uint8_t *datac = (uint8_t*)dataw; //byte stepper
    const uint8_t *const endc = &((const uint8_t*)key)[len];
    for (; datac < endc; datac++) {
      A += *datac;
      B += A;
    }
  }
  *(uint64_t *) out = B;
}

//64bit, ZFS
// objsize: 0x1220-0x1393: 371
void
fletcher4(const void *key, int len, uint32_t seed, void *out)
{
  uint32_t *dataw = (uint32_t *)key;
  const uint32_t *const endw = &((const uint32_t*)key)[len/4];
  uint64_t A = seed, B = 0, C = 0, D = 0;
  while (dataw < endw) {
    A += *dataw++;
    B += A;
    C += B;
    D += C;
  }
  if (len & 3) {
    uint8_t *datac = (uint8_t*)dataw; //byte stepper
    const uint8_t *const endc = &((const uint8_t*)key)[len];
    while (datac < endc) {
      A += *datac++;
      B += A;
      C += B;
      D += C;
    }
  }
  *(uint64_t *) out = D;
}

//-----------------------------------------------------------------------------

//also used in perl5 as djb2
// objsize: 0x13a0-0x13c9: 41
void
Bernstein(const void *key, int len, uint32_t seed, void *out)
{
  const uint8_t  *data = (const uint8_t *)key;
  const uint8_t *const end = &data[len];
  while (data < end) {
    //seed = ((seed << 5) + seed) + *data++;
    seed = 33 * seed + *data++;
  }
  *(uint32_t *) out = seed;
}

//as used in perl5
// objsize: 0x13a0-0x13c9: 41
void
sdbm(const void *key, int len, uint32_t hash, void *out)
{
  unsigned char  *str = (unsigned char *)key;
  const unsigned char *const end = (const unsigned char *)str + len;
  //note that perl5 adds the seed to the end of key, which looks like cargo cult
  while (str < end) {
    hash = (hash << 6) + (hash << 16) - hash + *str++;
  }
  *(uint32_t *) out = hash;
}

//as used in perl5 as one_at_a_time_hard
// objsize: 0x1400-0x1499: 153
void
JenkinsOOAT(const void *key, int len, uint32_t hash, void *out)
{
  unsigned char  *str = (unsigned char *)key;
  const unsigned char *const end = (const unsigned char *)str + len;
  uint64_t	  s = (uint64_t) hash;
  unsigned char  *seed = (unsigned char *)&s;
  //unsigned char seed[8];
  //note that perl5 adds the seed to the end of key, which looks like cargo cult
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
// objsize: 0x14a0-0x14e1: 65
void JenkinsOOAT_perl(const void *key, int len, uint32_t hash, void *out)
{
  unsigned char  *str = (unsigned char *)key;
  const unsigned char *const end = (const unsigned char *)str + len;
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

//------------------------------------------------
// One of a smallest non-multiplicative One-At-a-Time function
// that passes whole SMHasher.
// Author: Sokolov Yura aka funny-falcon <funny.falcon@gmail.com>
// objsize: 0x14f0-0x15dd: 237
void GoodOAAT(const void *key, int len, uint32_t seed, void *out) {
#define grol(x,n) (((x)<<(n))|((x)>>(32-(n))))
#define gror(x,n) (((x)>>(n))|((x)<<(32-(n))))
  unsigned char  *str = (unsigned char *)key;
  const unsigned char *const end = (const unsigned char *)str + len;
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
// objsize: 0x15e0-0x1624: 68
void MicroOAAT(const void *key, int len, uint32_t seed, void *out) {
#define grol(x,n) (((x)<<(n))|((x)>>(32-(n))))
#define gror(x,n) (((x)>>(n))|((x)<<(32-(n))))
  unsigned char  *str = (unsigned char *)key;
  const unsigned char *const end = (const unsigned char *)str + len;
  uint32_t h1 = seed ^ 0x3b00;
  uint32_t h2 = grol(seed, 15);
  while (str < end) {
    h1 += *str++;
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

// objsize: 0x1630-0x1786: 342
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
Crap8_test(const void *key, int len, uint32_t seed, void *out)
{
  *(uint32_t *) out = Crap8((const uint8_t *)key, len, seed);
}

extern "C" {
#ifdef __SSE2__
  void		  hasshe2 (const void *input, int len, uint32_t seed, void *out);
#endif
#if defined(__SSE4_2__) && defined(__x86_64__)
  uint32_t	  crc32c_hw(const void *input, int len, uint32_t seed);
  uint32_t	  crc32c(const void *input, int len, uint32_t seed);
  uint64_t	  crc64c_hw(const void *input, int len, uint32_t seed);
#endif
}

#ifdef __SSE2__
void
hasshe2_test(const void *input, int len, uint32_t seed, void *out)
{
  if (!len) {
    *(uint32_t *) out = 0;
    return;
  }
  if (len % 16) {
    //add pad NUL
    len += 16 - (len % 16);
  }
  // objsize: 0-1bd: 445
  hasshe2(input, len, seed, out);
}
#endif

#if defined(__SSE4_2__) && (defined(__i686__) || defined(_M_IX86) || defined(__x86_64__))
/* Compute CRC-32C using the Intel hardware instruction.
   TODO: arm8
 */
void
crc32c_hw_test(const void *input, int len, uint32_t seed, void *out)
{
  if (!len) {
    *(uint32_t *) out = 0;
    return;
  }
  // objsize: 0-28d: 653
  *(uint32_t *) out = crc32c_hw(input, len, seed);
}
/* Faster Adler SSE4.2 crc32 in HW */
void
crc32c_hw1_test(const void *input, int len, uint32_t seed, void *out)
{
  if (!len) {
    *(uint32_t *) out = 0;
    return;
  }
  // objsize: 0-29f: 671
  *(uint32_t *) out = crc32c(input, len, seed);
}
#if defined(__SSE4_2__) && defined(__x86_64__)
/* Compute CRC-64C using the Intel hardware instruction. */
void
crc64c_hw_test(const void *input, int len, uint32_t seed, void *out)
{
  if (!len) {
    *(uint64_t *) out = 0;
    return;
  }
  // objsize: 0x290-0x51c: 652
  *(uint64_t *) out = crc64c_hw(input, len, seed);
}
#endif
#endif

/* Cloudflare optimized zlib crc32 with PCLMUL */
#if 0
void
zlib_crc32_test(const void *input, int len, uint32_t seed, void *out)
{
    if (!len) {
      *(uint32_t *) out = 0;
      return;
    }
    *(uint32_t *) out = crc32(seed, input, (unsigned)len);
}
#endif

#if 0 && defined(__x86_64__) && (defined(__linux__) || defined(__APPLE__))
/* asm */
extern "C" {
  int fhtw_hash(const void* key, int key_len);
}
void
fhtw_test(const void *input, int len, uint32_t seed, void *out)
{
  *(uint32_t *) out = fhtw_hash(input, len);
}
#endif

#include "siphash.h"

/* https://github.com/floodyberry/siphash */
void
siphash_test(const void *input, int len, uint32_t seed, void *out)
{
  /* 128bit state, filled with a 32bit seed */
  unsigned char	key[16] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
  if (!len) {
    *(uint32_t *) out = 0;
    return;
  }
  memcpy(key, &seed, sizeof(seed));
  // objsize: 0-0x42f: 1071
  *(uint64_t *) out = siphash(key, (const unsigned char *)input, (size_t) len);
}
void
siphash13_test(const void *input, int len, uint32_t seed, void *out)
{
  unsigned char	key[16] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
  if (!len) {
    *(uint32_t *) out = 0;
    return;
  }
  memcpy(key, &seed, sizeof(seed));
  // objsize: 0x450-0x75a: 778
  *(uint64_t *) out = siphash13(key, (const unsigned char *)input, (size_t) len);
}
void
halfsiphash_test(const void *input, int len, uint32_t seed, void *out)
{
  unsigned char	key[16] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
  if (!len) {
    *(uint32_t *) out = 0;
    return;
  }
  memcpy(key, &seed, sizeof(seed));
  // objsize: 0x780-0xa3c: 700
  *(uint32_t *) out = halfsiphash(key, (const unsigned char *)input, (size_t) len);
}

/* https://github.com/gamozolabs/falkhash */
#if defined(__SSE4_2__) && defined(__x86_64__)
extern "C" {
  uint64_t falkhash_test(uint8_t *data, uint64_t len, uint32_t seed, void *out);
}
void
falkhash_test_cxx(const void *input, int len, uint32_t seed, void *out)
{
  uint64_t hash[2] = {0ULL, 0ULL};
  if (!len) {
    *(uint32_t *) out = 0;
    return;
  }
  // objsize: 0-0x108: 264
  falkhash_test((uint8_t *)input, (uint64_t)len, seed, hash);
  *(uint64_t *) out = hash[0];
}
#endif

#if defined(__SSE4_2__) && defined(__x86_64__)

#include "clhash.h"
static char clhash_random[RANDOM_BYTES_NEEDED_FOR_CLHASH];
void clhash_test (const void * key, int len, uint32_t seed, void * out) {
  memcpy(clhash_random, &seed, 4);
  // objsize: 0-0x711: 1809  
  *(uint64_t*)out = clhash(&clhash_random, (char*)key, (size_t)len);
}
void clhash_init()
{
  void* data = get_random_key_for_clhash(UINT64_C(0xb3816f6a2c68e530), 711);
  memcpy(clhash_random, data, RANDOM_BYTES_NEEDED_FOR_CLHASH);
}

#endif

// just to prove how bad academic papers really are:
// Thorup "High Speed Hashing for Integers and Strings" 2018
// https://arxiv.org/pdf/1504.06804.pdf
// objsize: 0x1bc0-0x1d19: 345
void multiply_shift (const void *key, int len, uint32_t seed, void *out) {
  size_t h   = (size_t)(seed | 1);
  size_t *dw = (size_t *)key; //word stepper
  const size_t *const endw = &((const size_t*)key)[len/sizeof(size_t)];
  const unsigned shift = sizeof(size_t) - len;
  // hashes x universally into len bits using the random odd seed.
  while (dw < endw) {
    h += (*dw++ * h) >> shift;
  }
  if (len & (sizeof(size_t)-1)) {
    uint8_t *dc = (uint8_t*)dw; //byte stepper
    const uint8_t *const endc = &((const uint8_t*)key)[len];
    while (dc < endc) {
      h += (*dc++ * h) >> shift;
    }
  }
  *(size_t *) out = h + (seed & 8);
}

// objsize: 0x1d20-0x1f81: 609
void pair_multiply_shift (const void *key, int len, uint32_t seed, void *out) {
  const uint16_t h1 = (seed & 0xffff) | 0x423d0001;
  const uint16_t h2 = (seed >> 8)     | 0x1f380001;
  const uint8_t b   = seed & 8;
  size_t h = seed | 1;
  size_t *dw = (size_t *)key; //word stepper
  const size_t *const endw = &((const size_t*)key)[len/sizeof(size_t) - 1];
  const unsigned shift = sizeof(size_t) - len;
  // hashes x universally into len bits using the random odd seed pair.
  while (dw < endw) {
    h += (*dw + h1) * (*(dw+1) + h2) + b;
    dw++; dw++;
  }
  h >>= shift;
  if (len & (sizeof(size_t)-1)) {
    uint8_t *dc = (uint8_t*)dw; //byte stepper
    const uint8_t *const endc = &((const uint8_t*)key)[len-1];
    while (dc < endc) {
      h += (*dc + h1) * (*(dc+1) + h2) + b;
      dc++; dc++;
    }
  }
  *(size_t *) out = h;
}

void wyhash_test (const void * key, int len, uint32_t seed, void * out) {
  // objsize 20-a12: 2546
  *(uint64_t*)out = wyhash(key, (uint64_t)len, (uint64_t)seed);
}
void wyhash32low (const void * key, int len, uint32_t seed, void * out) {
  *(uint32_t*)out = 0xFFFFFFFF & wyhash(key, (uint64_t)len, (uint64_t)seed);
}

void mirhash_test (const void * key, int len, uint32_t seed, void * out) {
  // objsize 2950-2da8: 1112
  *(uint64_t*)out = mir_hash(key, (uint64_t)len, (uint64_t)seed);
}
void mirhash32low (const void * key, int len, uint32_t seed, void * out) {
  *(uint32_t*)out = 0xFFFFFFFF & mir_hash(key, (uint64_t)len, (uint64_t)seed);
}

void mirhashstrict_test (const void * key, int len, uint32_t seed, void * out) {
  // objsize 2950-2da8: 1112
  *(uint64_t*)out = mir_hash_strict(key, (uint64_t)len, (uint64_t)seed);
}
void mirhashstrict32low (const void * key, int len, uint32_t seed, void * out) {
  *(uint32_t*)out = 0xFFFFFFFF & mir_hash_strict(key, (uint64_t)len, (uint64_t)seed);
}

static uint8_t tsip_key[16];
void tsip_init()
{
  uint64_t r = random();
  memcpy(&tsip_key[0], &r, 8);
  r = random();
  memcpy(&tsip_key[8], &r, 8);
}
void tsip_test(const void *bytes, int len, uint32_t seed, void *out)
{
  memcpy(&tsip_key[0], &seed, 4);
  memcpy(&tsip_key[8], &seed, 4);
  *(uint64_t*)out = tsip(tsip_key, (const unsigned char*)bytes, (uint64_t)len);
}
