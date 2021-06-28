#define _HASHES_CPP
#include "Hashes.h"
#include "Random.h"

#include <stdlib.h>
#include <stdint.h>
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
      h += *dc++ * UINT64_C(11400714819323198485);
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
uint32_t MurmurOAAT(const char *key, int len, uint32_t hash)
{
  const uint8_t *data = (const uint8_t *)key;
  const uint8_t *const end = &data[len];

  while (data < end) {
    hash ^= *data++;
    hash *= 0x5bd1e995;
    hash ^= hash >> 15;
  }

  return hash;
}

//----------------------------------------------------------------------------

// objsize: 0x5a0-0xc3c: 1692
size_t
fibonacci(const char *key, int len, uint32_t seed)
{
  size_t h = (size_t)seed;
  size_t *dw = (size_t *)key; //word stepper
  const size_t *const endw = &((const size_t*)key)[len/sizeof(size_t)];
  while (dw < endw) {
    h += *dw++ * UINT64_C(11400714819323198485);
  }
  if (len & (sizeof(size_t)-1)) {
    uint8_t *dc = (uint8_t*)dw; //byte stepper
    const uint8_t *const endc = &((const uint8_t*)key)[len];
    while (dc < endc) {
      h += *dc++ * UINT64_C(11400714819323198485);
    }
  }
  return h;
}

// objsize: 0xc40-0xd56: 278
size_t
FNV2(const char *key, int len, size_t seed)
{
  size_t h;
  size_t *dw = (size_t *)key; //word stepper
  const size_t *const endw = &((const size_t*)key)[len/sizeof(size_t)];

#ifdef HAVE_BIT32
  h = seed ^ UINT32_C(2166136261);
#else
  h = seed ^ UINT64_C(0xcbf29ce484222325);
#endif

#ifdef HAVE_ALIGNED_ACCESS_REQUIRED
  // avoid ubsan, misaligned writes
  int i = (uintptr_t)dw % sizeof (size_t);
  if (i) {
    uint8_t *dc = (uint8_t*)key;
    switch (i) {
    case 1:
      h ^= *dc++;
#ifdef HAVE_BIT32
      h *= UINT32_C(16777619);
#else
      h *= UINT64_C(0x100000001b3);
#endif
    case 2:
      h ^= *dc++;
#ifdef HAVE_BIT32
      h *= UINT32_C(16777619);
#else
      h *= UINT64_C(0x100000001b3);
#endif
    case 3:
      h ^= *dc++;
#ifdef HAVE_BIT32
      h *= UINT32_C(16777619);
#else
      h *= UINT64_C(0x100000001b3);
#endif
#ifndef HAVE_BIT32
    case 4:
      h ^= *dc++;
      h *= UINT64_C(0x100000001b3);
    case 5:
      h ^= *dc++;
      h *= UINT64_C(0x100000001b3);
    case 6:
      h ^= *dc++;
      h *= UINT64_C(0x100000001b3);
    case 7:
      h ^= *dc++;
      h *= UINT64_C(0x100000001b3);
#endif
    default:
      break;
    }
    dw = (size_t*)dc; //word stepper
  }
#endif

  while (dw < endw) {
    h ^= *dw++;
#ifdef HAVE_BIT32
    h *= UINT32_C(16777619);
#else
    h *= UINT64_C(0x100000001b3);
#endif
  }
  if (len & (sizeof(size_t)-1)) {
    uint8_t *dc = (uint8_t*)dw; //byte stepper
    const uint8_t *const endc = &((const uint8_t*)key)[len];
    while (dc < endc) {
      h ^= *dc++;
#ifdef HAVE_BIT32
      h *= UINT32_C(16777619);
#else
      h *= UINT64_C(0x100000001b3);
#endif
    }
  }
  return h;
}

// i.e. FNV1a
// objsize: 0xd60-0xe2c: 204
uint32_t
FNV32a(const void *key, int len, uint32_t seed)
{
  uint32_t h = seed;
  const uint8_t  *data = (const uint8_t *)key;

  h ^= UINT32_C(2166136261);
  for (int i = 0; i < len; i++) {
    h ^= data[i];
    h *= 16777619;
  }
  return h;
}

// objsize: 0xe30-0xf71: 321
uint32_t
FNV32a_YoshimitsuTRIAD(const char *key, int len, uint32_t seed)
{
  const uint8_t  *p = (const uint8_t *)key;
  const uint32_t  PRIME = 709607;
  uint32_t	  hash32A = seed ^ UINT32_C(2166136261);
  uint32_t	  hash32B = UINT32_C(2166136261) + len;
  uint32_t	  hash32C = UINT32_C(2166136261);

  for (; len >= 3 * 2 * sizeof(uint32_t); len -= 3 * 2 * sizeof(uint32_t), p += 3 * 2 * sizeof(uint32_t)) {
    hash32A = (hash32A ^ (ROTL32(*(uint32_t *) (p + 0), 5)  ^ *(uint32_t *) (p + 4)))  * PRIME;
    hash32B = (hash32B ^ (ROTL32(*(uint32_t *) (p + 8), 5)  ^ *(uint32_t *) (p + 12))) * PRIME;
    hash32C = (hash32C ^ (ROTL32(*(uint32_t *) (p + 16), 5) ^ *(uint32_t *) (p + 20))) * PRIME;
  }
  if (p != (const uint8_t *)key) {
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
  return hash32A ^ (hash32A >> 16);
}

#ifdef HAVE_INT64
// objsize: 0xf80-0x108e: 270
uint32_t
FNV1A_Totenschiff(const char *key, int len, uint32_t seed)
{
#define _PADr_KAZE(x, n) (((x) << (n)) >> (n))

  const char *p = (char *)key;
  const uint32_t PRIME = 591798841;
  uint32_t hash32;
  uint64_t hash64 = (uint64_t)seed ^ UINT64_C(14695981039346656037);
  uint64_t PADDEDby8;

  for (; len > 8; len -= 8, p += 8) {
    PADDEDby8 = *(uint64_t *)(p + 0);
    hash64 = (hash64 ^ PADDEDby8) * PRIME;
  }

  // Here len is 1..8. when (8-8) the QWORD remains intact
  PADDEDby8 = _PADr_KAZE(*(uint64_t *)(p + 0), (8 - len) << 3);
  hash64 = (hash64 ^ PADDEDby8) * PRIME;

  hash32 = (uint32_t)(hash64 ^ (hash64 >> 32));
  return hash32 ^ (hash32 >> 16);
#undef _PADr_KAZE
}

// Dedicated to Pippip, the main character in the 'Das Totenschiff' roman, actually the B.Traven himself, his real name was Hermann Albert Otto Maksymilian Feige.
// CAUTION: Add 8 more bytes to the buffer being hashed, usually malloc(...+8) - to prevent out of boundary reads!
// Many thanks go to Yurii 'Hordi' Hordiienko, he lessened with 3 instructions the original 'Pippip', thus:
// objsize: 0x1090-0x1123: 147
uint32_t
FNV1A_Pippip_Yurii(const char *key, int wrdlen, uint32_t seed)
{
#define _PADr_KAZE(x, n) ( ((x) << (n))>>(n) )
  const char *str = (char *)key;
  const uint32_t PRIME = 591798841;
  uint32_t hash32;
  uint64_t hash64 = (uint64_t)seed ^ UINT64_C(14695981039346656037);
  size_t Cycles, NDhead;
  if (wrdlen > 8) {
    Cycles = ((wrdlen - 1) >> 4) + 1;
    NDhead = wrdlen - (Cycles << 3);
#pragma nounroll
    for (; Cycles--; str += 8) {
      hash64 = (hash64 ^ (*(uint64_t *)(str))) * PRIME;
      hash64 = (hash64 ^ (*(uint64_t *)(str + NDhead))) * PRIME;
    }
  } else {
    hash64 = (hash64 ^ _PADr_KAZE(*(uint64_t *)(str + 0), (8 - wrdlen) << 3)) *
             PRIME;
  }
  hash32 = (uint32_t)(hash64 ^ (hash64 >> 32));
  return hash32 ^ (hash32 >> 16);
#undef _PADr_KAZE
} // Last update: 2019-Oct-30, 14 C lines strong, Kaze.

// objsize: 0x1090-0x10df: 79
uint64_t
FNV64a(const char *key, int len, uint64_t seed)
{
  uint64_t  h = seed;
  uint8_t  *data = (uint8_t *)key;
  const uint8_t *const end = &data[len];

  h ^= UINT64_C(0xcbf29ce484222325);
  while (data < end) {
    h ^= *data++;
    h *= UINT64_C(0x100000001b3);
  }
  return h;
}

#endif

//-----------------------------------------------------------------------------

// objsize: 0x1090-0x10df: 79
uint32_t
x17(const char *key, int len, uint32_t h)
{
  uint8_t *data = (uint8_t *)key;
  const uint8_t *const end = &data[len];

  while (data < end) {
    h = 17 * h + (*data++ - ' ');
  }
  return h ^ (h >> 16);
}

//64bit, ZFS
//note the original fletcher2 assumes 128bit aligned data, and
//can hereby advance the inner loop by 2 64bit words.
//both fletcher's return 4 words, 256 bit. Both are nevertheless very weak hashes.
// objsize: 0x1120-0x1218: 248
uint64_t
fletcher2(const char *key, int len, uint64_t seed)
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
  return B;
}

//64bit, ZFS
// objsize: 0x1220-0x1393: 371
uint64_t
fletcher4(const char *key, int len, uint64_t seed)
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
  return D;
}

//-----------------------------------------------------------------------------

//also used in perl5 as djb2
// objsize: 0x13a0-0x13c9: 41
uint32_t
Bernstein(const char *key, int len, uint32_t seed)
{
  const uint8_t  *data = (const uint8_t *)key;
  const uint8_t *const end = &data[len];
  while (data < end) {
    //seed = ((seed << 5) + seed) + *data++;
    seed = 33 * seed + *data++;
  }
  return seed;
}

//as used in perl5
// objsize: 0x13a0-0x13c9: 41
uint32_t
sdbm(const char *key, int len, uint32_t hash)
{
  unsigned char  *str = (unsigned char *)key;
  const unsigned char *const end = (const unsigned char *)str + len;
  //note that perl5 adds the seed to the end of key, which looks like cargo cult
  while (str < end) {
    hash = (hash << 6) + (hash << 16) - hash + *str++;
  }
  return hash;
}

//as used in perl5 as one_at_a_time_hard
// objsize: 0x1400-0x1499: 153
uint32_t
JenkinsOOAT(const char *key, int len, uint32_t hash)
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

  return hash;
}

//as used in perl5 until 5.17(one_at_a_time_old)
// objsize: 0x14a0-0x14e1: 65
uint32_t JenkinsOOAT_perl(const char *key, int len, uint32_t hash)
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
  return hash;
}

//------------------------------------------------
// One of a smallest non-multiplicative One-At-a-Time function
// that passes whole SMHasher.
// Author: Sokolov Yura aka funny-falcon <funny.falcon@gmail.com>
// objsize: 0x14f0-0x15dd: 237
uint32_t
GoodOAAT(const char *key, int len, uint32_t seed) {
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
  return h2;
#undef grol
#undef gror
}

// MicroOAAT suitable for hash-tables using prime numbers.
// It passes all collision checks.
// Author: Sokolov Yura aka funny-falcon <funny.falcon@gmail.com>
// objsize: 0x15e0-0x1624: 68
uint32_t
MicroOAAT(const char *key, int len, uint32_t seed) {
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
  return h1 ^ h2;
#undef grol
#undef gror
}

//-----------------------------------------------------------------------------
//Crap8 hash from http://www.team5150.com / ~andrew / noncryptohashzoo / Crap8.html

// objsize: 0x1630-0x1786: 342
uint32_t
Crap8(const uint8_t * key, uint32_t len, uint32_t seed)
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

extern "C" {
#ifdef HAVE_SSE2
  void		  hasshe2 (const void *input, int len, uint32_t seed, void *out);
#endif
#ifdef HAVE_SSE42
# ifndef HAVE_BROKEN_MSVC_CRC32C_HW
  uint32_t	  crc32c_hw(const void *input, int len, uint32_t seed);
  uint64_t	  crc64c_hw(const void *input, int len, uint32_t seed);
# endif
  uint32_t	  crc32c(const void *input, size_t len, uint32_t seed);
#endif
}

#if defined(HAVE_SSE2)
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

#ifdef HAVE_SSE42
# ifndef HAVE_BROKEN_MSVC_CRC32C_HW
//#if defined(__SSE4_2__) && (defined(__i686__) || defined(_M_IX86) || defined(__x86_64__))
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
# endif

# if defined(__SSE4_2__) && (defined(__i686__) || defined(_M_IX86) || defined(__x86_64__))
/* Faster Adler SSE4.2 crc32 on Intel HW only. FIXME aarch64 */
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
# endif
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

#if defined(HAVE_SSE42) && defined(__x86_64__)

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
bool clhash_bad_seeds(std::vector<uint64_t> &seeds)
{
  seeds = std::vector<uint64_t> { UINT64_C(0) };
  return true;
}
void clhash_seed_init(size_t &seed)
{
  // reject bad seeds
  const std::vector<uint64_t> bad_seeds = { UINT64_C(0) };
  while (std::find(bad_seeds.begin(), bad_seeds.end(), (uint64_t)seed) != bad_seeds.end())
    seed++;
  memcpy(clhash_random, &seed, sizeof(seed));
}

#endif

#include "halftime-hash.hpp"

alignas(64) static uint64_t
    halftime_hash_random[8 * ((halftime_hash::kEntropyBytesNeeded / 64) + 1)];

void halftime_hash_style64_test(const void *key, int len, uint32_t seed, void *out) {
  *(uint64_t *)out =
      halftime_hash::HalftimeHashStyle64(halftime_hash_random, (char *)key, (size_t)len);
}

void halftime_hash_style128_test(const void *key, int len, uint32_t seed, void *out) {
  *(uint64_t *)out =
      halftime_hash::HalftimeHashStyle128(halftime_hash_random, (char *)key, (size_t)len);
}

void halftime_hash_style256_test(const void *key, int len, uint32_t seed, void *out) {
  *(uint64_t *)out =
      halftime_hash::HalftimeHashStyle256(halftime_hash_random, (char *)key, (size_t)len);
}

void halftime_hash_style512_test(const void *key, int len, uint32_t seed, void *out) {
  *(uint64_t *)out =
      halftime_hash::HalftimeHashStyle512(halftime_hash_random, (char *)key, (size_t)len);
}

void halftime_hash_init() {
  size_t seed =
#ifdef HAVE_BIT32
    0xcc70c4c1ULL;
#else
    0xcc70c4c1798e4a6fUL; // 64bit only
#endif
  halftime_hash_seed_init(seed);
}

// romu random number generator for seeding the HalftimeHash entropy

// TODO: align and increase size of outut random array

#if defined(__AVX512F__)

#include <immintrin.h>

void romuQuad32simd(const __m512i seeds[4], uint64_t *output, size_t count) {
  __m512i wState = seeds[0], xState = seeds[1], yState = seeds[2],
       zState = seeds[3];
  const auto m = _mm512_set1_epi32(3323815723u);
  for (size_t i = 0; i < count; i += 8) {
    __m512i wp = wState, xp = xState, yp = yState, zp = zState;
    wState = _mm512_mullo_epi32(m, zp);
    xState = _mm512_add_epi32(zp, _mm512_rol_epi32(wp, 26));
    yState = _mm512_sub_epi32(yp, xp);
    zState = _mm512_add_epi32(yp, wp);
    zState = _mm512_rol_epi32(zState, 9);
    _mm512_store_epi64(&output[i], xp);
  }
}

void halftime_hash_seed_init(size_t &seed) {
  __m512i seeds[4] = {
      {
          (long long)seed ^ (long long)0x9a9b4c4e44dd48d1,
          (long long)seed ^ (long long)0xf8b0cd76a61945b1,
          (long long)seed ^ (long long)0x86268b0ae8494ce2,
          (long long)seed ^ (long long)0x7d31e5469df4484d,
          (long long)seed ^ (long long)0x62cb7b3e5e334aab,
          (long long)seed ^ (long long)0xc4c4065529834f39,
          (long long)seed ^ (long long)0xcc7972121c52411f,
          (long long)seed ^ (long long)0x7e08efb9ea5a434f,
      },
      {
          (long long)seed ^ (long long)0xccbc1ec6f244430c,
          (long long)seed ^ (long long)0xecf76d38f32b4296,
          (long long)seed ^ (long long)0xdf061d7c86664fa2,
          (long long)seed ^ (long long)0x08e0da9580d44252,
          (long long)seed ^ (long long)0xd074f3685aeb4f71,
          (long long)seed ^ (long long)0x3f83eb99126d4a74,
          (long long)seed ^ (long long)0xb5d24f61b4f540fa,
          (long long)seed ^ (long long)0x33f248aa4b3c4aaf,
      },
      {
          (long long)seed ^ (long long)0xd292ecaddb1c4dc1,
          (long long)seed ^ (long long)0x94489307a0d041ed,
          (long long)seed ^ (long long)0x25a4752be4bd4b84,
          (long long)seed ^ (long long)0xa1d4010ab16c4b96,
          (long long)seed ^ (long long)0x87175e8421534efa,
          (long long)seed ^ (long long)0x0df85252bb894d2b,
          (long long)seed ^ (long long)0x1d43b52179374cb4,
          (long long)seed ^ (long long)0x5586b8bf3d4f4ca7,
      },
      {
          (long long)seed ^ (long long)0x7275e2473e0f4618,
          (long long)seed ^ (long long)0x2340093a933a4191,
          (long long)seed ^ (long long)0x849ec473349843ac,
          (long long)seed ^ (long long)0x9b8873c068ac4e41,
          (long long)seed ^ (long long)0x3b8a6084e4ec44a7,
          (long long)seed ^ (long long)0x341dadfa6e524396,
          (long long)seed ^ (long long)0xb735256ca12649e9,
          (long long)seed ^ (long long)0x1bd21c39a0694d4f,
      },
  };
  romuQuad32simd(seeds, halftime_hash_random,
                 sizeof(halftime_hash_random) / sizeof(halftime_hash_random[0]));
}

#else

void halftime_hash_seed_init(size_t &seed)
{
#define ROTL(d,lrot) ((d<<(lrot)) | (d>>(8*sizeof(d)-(lrot))))
  uint64_t wState = seed, xState= 0xecfc1357d65941ae, yState=0xbe1927f97b8c43f1,
    zState=0xf4d4beb14ae042bb;
  for (unsigned i = 0; i < sizeof(halftime_hash_random) / sizeof(halftime_hash_random[0]);
       ++i) {
    const uint64_t wp = wState, xp = xState, yp = yState, zp = zState;
    wState = 15241094284759029579u * zp;  // a-mult
    xState = zp + ROTL(wp, 52);           // b-rotl, c-add
    yState = yp - xp;                     // d-sub
    zState = yp + wp;                     // e-add
    zState = ROTL(zState, 19);            // f-rotl
    halftime_hash_random[i] = xp;
  }
#undef ROTL
}
#endif


// Multiply shift from
// Thorup "High Speed Hashing for Integers and Strings" 2018
// https://arxiv.org/pdf/1504.06804.pdf
//
#ifdef __SIZEOF_INT128__
   const static int MULTIPLY_SHIFT_RANDOM_WORDS = 1<<8;
   static __uint128_t multiply_shift_random[MULTIPLY_SHIFT_RANDOM_WORDS];
   const static __uint128_t multiply_shift_r = ((__uint128_t)0x75f17d6b3588f843 << 64) | 0xb13dea7c9c324e51;
   void multiply_shift(const void * key, int len_bytes, uint32_t seed, void * out) {
      const uint8_t* buf = (const uint8_t*) key;
      const int len = len_bytes/8;

      // The output is 64 bits, and we consider the input 64 bit as well,
      // so our intermediate values are 128.
      // We mix in len_bytes in the basis, since smhasher considers two keys
      // of different length to be different, even if all the extra bits are 0.
      // This is needed for the AppendZero test.
      uint64_t h = (seed + len_bytes) * multiply_shift_r >> 64;
      for (int i = 0; i < len; i++, buf += 8)
         h += multiply_shift_random[i % MULTIPLY_SHIFT_RANDOM_WORDS] * take64(buf) >> 64;

      // Now get the last bytes
      int remaining_bytes = len_bytes & 7;
      if (remaining_bytes) {
         uint64_t last = 0;
         if (remaining_bytes & 4) {last = take32(buf); buf += 4;}
         if (remaining_bytes & 2) {last = (last << 16) | take16(buf); buf += 2;}
         if (remaining_bytes & 1) {last = (last << 8) | take08(buf);}
         h += multiply_shift_random[len % MULTIPLY_SHIFT_RANDOM_WORDS] * last >> 64;
      }

      *(uint64_t*)out = h;
   }
   static __uint128_t rand128() {
     return rand_u128();
   }
   void multiply_shift_seed_init_slow(uint32_t seed) {
      srand(seed);
      for (int i = 0; i < MULTIPLY_SHIFT_RANDOM_WORDS; i++) {
         multiply_shift_random[i] = rand128();
         if (!multiply_shift_random[i])
           multiply_shift_random[i]++;
         // We don't need an odd multiply, when we add the seed in the beginning
         //multiply_shift_random[i] |= 1;
      }
   }
   bool multiply_shift_bad_seeds(std::vector<uint64_t> &seeds) {
     // all seeds & 0xfffffff0
     seeds = std::vector<uint64_t> { UINT64_C(0xfffffff0), UINT64_C(0x1fffffff0) };
     return true;
   }
   void multiply_shift_seed_init(uint32_t &seed) {
     // The seeds we get are not random values, but just something like 1, 2 or 3.
     // So we xor it with a random number to get something slightly more reasonable.
     // But skip really bad seed patterns: 0x...fffffff0
     if ((seed & 0xfffffff0ULL) == 0xfffffff0ULL)
       seed++;
     multiply_shift_random[0] = (__uint128_t)seed ^ multiply_shift_r;
   }
   void multiply_shift_init() {
      multiply_shift_seed_init_slow(0);
   }

   // Vector multiply-shift (3.4) from Thorup's notes.
   void pair_multiply_shift(const void * key, int len_bytes, uint32_t seed, void * out) {
      const uint8_t* buf = (const uint8_t*) key;
      int len = len_bytes/8;

      uint64_t h = (__uint128_t)(seed + len_bytes) * multiply_shift_r >> 64;
      for (int i = 0; i < len/2; i++, buf += 16)
         h += (multiply_shift_random[2*i & MULTIPLY_SHIFT_RANDOM_WORDS-1] + take64(buf+8))
            * (multiply_shift_random[2*i+1 & MULTIPLY_SHIFT_RANDOM_WORDS-1] + take64(buf)) >> 64;

      // Make sure we have the last word, if the number of words is odd
      if (len & 1) {
         h += multiply_shift_random[len-1 & MULTIPLY_SHIFT_RANDOM_WORDS-1] * take64(buf) >> 64;
         buf += 8;
      }

      // Get the last bytes when things are unaligned
      int remaining_bytes = len_bytes & 7;
      if (remaining_bytes) {
         uint64_t last = 0;
         if (remaining_bytes & 4) {last = take32(buf); buf += 4;}
         if (remaining_bytes & 2) {last = (last << 16) | take16(buf); buf += 2;}
         if (remaining_bytes & 1) {last = (last << 8) | take08(buf);}
         h += multiply_shift_random[len & MULTIPLY_SHIFT_RANDOM_WORDS-1] * last >> 64;
      }

      *(uint64_t*)out = h;
   }

  //objsize: 450fb0-45118f: 479. low32 of 128bit
   const static uint64_t MERSENNE_61 = (1ull << 61) - 1;
   static uint64_t mult_combine61(uint64_t h, uint64_t x, uint64_t a) {
      __uint128_t temp = (__uint128_t)h * x + a;
      return ((uint64_t)temp & MERSENNE_61) + (uint64_t)(temp >> 61);
   }
   const static int POLY_MERSENNE_MAX_K = 4;
   static uint64_t poly_mersenne_random[POLY_MERSENNE_MAX_K+1];
   static uint64_t poly_mersenne_a;
   static uint64_t poly_mersenne_b;
   static uint32_t poly_k_mersenne(const void * key, int len_bytes, uint32_t seed, const int k) {
      const uint8_t* buf = (const uint8_t*) key;

      // We first combine hashes using a polynomial in `a`:
      // hash = x1 + x2 * a + x3 * a^2 + ... (mod p)
      // This hash has collision probability len/p, since the polynomial is
      // degree and so can have at most len roots (values of a that make it zero).
      const uint64_t a = poly_mersenne_a;

      // We use the length as the first character.
      // We also add the seed, which is not quite how it is intended, but polyhash
      // really doesn't take a runtime seed. It is rather reseeded from seed_init.
      uint64_t h = len_bytes ^ seed;

      for (int i = 0; i < len_bytes/4; i++, buf += 4) {
         // Partial modular reduction. Since each round adds 32 bits, and this
         // subtracts (up to) 61 bits, we make sure to never overflow.
         h = mult_combine61(h, a, take32(buf));
      }

      // Get the last character
      int remaining_bytes = len_bytes % 4;
      if (remaining_bytes) {
         uint32_t last = 0;
         if (remaining_bytes & 2) {last = take16(buf); buf += 2;}
         if (remaining_bytes & 1) {last = (last << 8) | take08(buf);}
         h = mult_combine61(h, a, last);
      }

      // Increase hash strength from low collision rate to K-independence.
      // hash = a1 + a2 * h + a3 * h^2 + ... (mod p)
      if (k != 0) {
         uint64_t h0 = h;
         h = poly_mersenne_random[0];
         for (int i = 1; i <= k; i++) {
            h = mult_combine61(h, h0, poly_mersenne_random[i]);
         }
      }

      // Finally complete the modular reduction
      if (h >= MERSENNE_61)
         h -= MERSENNE_61;

      return h;
   }
   void poly_0_mersenne(const void * key, int len_bytes, uint32_t seed, void * out) {
      *(uint32_t*)out = (uint32_t)poly_k_mersenne(key, len_bytes, seed, 0);
   }
   void poly_1_mersenne(const void * key, int len_bytes, uint32_t seed, void * out) {
      *(uint32_t*)out = (uint32_t)poly_k_mersenne(key, len_bytes, seed, 1);
   }
   void poly_2_mersenne(const void * key, int len_bytes, uint32_t seed, void * out) {
      *(uint32_t*)out = (uint32_t)poly_k_mersenne(key, len_bytes, seed, 2);
   }
   void poly_3_mersenne(const void * key, int len_bytes, uint32_t seed, void * out) {
      *(uint32_t*)out = (uint32_t)poly_k_mersenne(key, len_bytes, seed, 3);
   }
   void poly_4_mersenne(const void * key, int len_bytes, uint32_t seed, void * out) {
      *(uint32_t*)out = (uint32_t)poly_k_mersenne(key, len_bytes, seed, 4);
   }
   void poly_mersenne_seed_init(uint32_t &seed) {
      srand(seed);
      // a has be at most 2^60, or the lazy modular reduction won't work.
      poly_mersenne_a = rand128() % (MERSENNE_61/2);
      poly_mersenne_b = rand128() % MERSENNE_61;
      for (int i = 0; i < POLY_MERSENNE_MAX_K+1; i++) {
         // The random values should be at most 2^61-2, or the lazy
         // modular reduction won't work.
         poly_mersenne_random[i] = rand128() % MERSENNE_61;
      }
   }
   void poly_mersenne_init() {
     uint32_t seed = 0;
     poly_mersenne_seed_init(seed);
   }

#endif


//TODO MSVC
#ifdef HAVE_INT64
#ifndef _MSC_VER
static uint8_t tsip_key[16];
void tsip_init()
{
  uint64_t r = rand_u64();
  memcpy(&tsip_key[0], &r, 8);
  r = rand_u64();
  memcpy(&tsip_key[8], &r, 8);
}
void tsip_test(const void *bytes, int len, uint32_t seed, void *out)
{
  memcpy(&tsip_key[0], &seed, 4);
  memcpy(&tsip_key[8], &seed, 4);
  *(uint64_t*)out = tsip(tsip_key, (const unsigned char*)bytes, (uint64_t)len);
}

#endif /* !MSVC */
#endif /* HAVE_INT64 */

#ifdef HAVE_SSE2
#  ifdef __AVX2__
#   define FARSH_AVX2
#  elif defined HAVE_SSE42
#   define FARSH_SSE2
#  endif
# include "farsh.c"

// objsize: 0-3b0: 944
void farsh32_test ( const void * key, int len, unsigned seed, void * out )
{
  farsh_n(key,len,0,1,seed,out);
}
void farsh64_test ( const void * key, int len, unsigned seed, void * out )
{
  farsh_n(key,len,0,2,seed,out);
}
void farsh128_test ( const void * key, int len, unsigned seed, void * out )
{
  farsh_n(key,len,0,4,seed,out);
}
void farsh256_test ( const void * key, int len, unsigned seed, void * out )
{
  farsh_n(key,len,0,8,seed,out);
}
#endif

// arm also has AESNI, check for sse
#if defined(HAVE_SSE42) && defined(HAVE_AESNI) && !defined(_MSC_VER)
/* See https://news.ycombinator.com/item?id=22463979 */
/* From https://gist.github.com/majek/96dd615ed6c8aa64f60aac14e3f6ab5a, but added a seed */
uint64_t aesnihash(uint8_t *in, unsigned long src_sz, uint32_t seed) {
  uint8_t tmp_buf[16] = {0};
  __m128i rk0 = {0x736f6d6570736575ULL, 0x646f72616e646f6dULL};
  __m128i rk1 = {0x1231236570743245ULL, 0x126f12321321456dULL};
  __m128i hash = rk0;
  uint64_t seed64 = (uint64_t)seed;
  hash[0] ^= seed64;

  while (src_sz >= 16) {
  onemoretry:
    __m128i piece = _mm_loadu_si128((__m128i *)in);
    in += 16;
    src_sz -= 16;
    hash = _mm_aesenc_si128(_mm_xor_si128(hash, piece), rk0);
    hash = _mm_aesenc_si128(hash, rk1);
  }

  if (src_sz > 0) {
    unsigned long i;
    for (i = 0; i < src_sz && i < 16; i++) {
      tmp_buf[i] = in[i];
    }
    src_sz = 16;
    in = &tmp_buf[0];
    goto onemoretry;
  }

  hash = _mm_aesenc_si128(hash, _mm_set_epi64x(src_sz, src_sz));

  return hash[0] ^ hash[1];
}
#endif

#if defined(HAVE_CLMUL) && !defined(_MSC_VER)
void crc32c_pclmul_test(const void *key, int len, uint32_t seed, void *out)
{
  if (!len) {
    *(uint32_t *) out = 0;
    return;
  }
  // objsize: 0x1e1 = 481
  if (((uintptr_t)key & 15) != 0) {
    if (len < 1024) {
      alignas(16) unsigned char stack[1024];
      memcpy(stack, key, len);
      *(uint32_t *) out = crc32_pclmul_le_16(stack, (size_t)len, seed);
    }
    else {
#ifdef _MSC_VER // TODO need to verify
      alignas(16) unsigned char const *input = (unsigned char const *)_aligned_malloc(len, 16);
#elif __STDC_VERSION__ > 201200L // macports gcc-mp-6 has 201112 but no aligned_alloc
      alignas(16) unsigned char const *input = (unsigned char const *)aligned_alloc(16, len);
#else
      alignas(16) unsigned char const *input = NULL;
      posix_memalign((void**)&input, 16, len);
#endif
      memcpy((void*)input, key, len);
      *(uint32_t *) out = crc32_pclmul_le_16(input, (size_t)len, seed);
      free ((void*)input);
    }
  }
  else {
    assert(((uintptr_t)key & 15) == 0); // input is 16byte aligned already
    *(uint32_t *) out = crc32_pclmul_le_16((unsigned char const *)key, (size_t)len, seed);
  }
}
#endif

#include "hash-garage/nmhash.h"
// objsize: 4202f0-420c7d: 2445
void nmhash32_test ( const void * key, int len, uint32_t seed, void * out ) {
  *(uint32_t*)out = NMHASH32 (key, (const size_t) len, seed);
}
// objsize: 466100-4666d6: 1494
void nmhash32x_test ( const void * key, int len, uint32_t seed, void * out ) {
  *(uint32_t*)out = NMHASH32X (key, (const size_t) len, seed);
}
