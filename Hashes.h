#pragma once

#include "Platform.h"
#include "Types.h"

#include "MurmurHash1.h"
#include "MurmurHash2.h"
#include "MurmurHash3.h"
#include "PMurHash.h"

#define XXH_INLINE_ALL
#include "xxhash.h"

#include "metrohash/metrohash64.h"
#include "metrohash/metrohash128.h"
#include "cmetrohash.h"
#include "opt_cmetrohash.h"

#if defined(__SSE4_2__) && defined(__x86_64__)
#include "metrohash/metrohash64crc.h"
#include "metrohash/metrohash128crc.h"
#endif

#include "fasthash.h"
#include "jody_hash32.h"
#include "jody_hash64.h"

// objsize: 0-0x113 = 276
#include "tifuhash.h"
// objsize: 5f0-85f = 623
#include "floppsyhash.h"

#include "vmac.h"

#include "tabulation.h"

//----------
// These are _not_ hash functions (even though people tend to use crc32 as one...)

void BadHash(const void *key, int len, uint32_t seed, void *out);
void sumhash(const void *key, int len, uint32_t seed, void *out);
void sumhash32(const void *key, int len, uint32_t seed, void *out);

void DoNothingHash(const void *key, int len, uint32_t seed, void *out);
void NoopOAATReadHash(const void *key, int len, uint32_t seed, void *out);
void crc32(const void *key, int len, uint32_t seed, void *out);

//----------
// General purpose hashes

#ifdef __SSE2__
void hasshe2_test(const void *key, int len, uint32_t seed, void *out);
#endif
#if defined(__SSE4_2__) && defined(__x86_64__)
void crc32c_hw_test(const void *key, int len, uint32_t seed, void *out);
void crc32c_hw1_test(const void *key, int len, uint32_t seed, void *out);
void crc64c_hw_test(const void *key, int len, uint32_t seed, void *out);
#endif
#if defined(HAVE_CLMUL) && !defined(_MSC_VER)
/* Function from linux kernel 3.14. It computes the CRC over the given
 * buffer with initial CRC value <crc32>. The buffer is <len> byte in length,
 * and must be 16-byte aligned. */
extern "C" uint32_t crc32_pclmul_le_16(unsigned char const *buffer, size_t len,
                                       uint32_t crc32);
inline void crc32c_pclmul_test(const void *key, int len, uint32_t seed, void *out)
{
  if (!len) {
    *(uint32_t *) out = 0;
    return;
  }
  // objsize: 0x1e1 = 481
#ifdef NDEBUG
  if (((uintptr_t)key & 15) != 0) {
#if 0 // slower
    if (len < 1024) {
      unsigned char input[1024];
      memcpy((void*)input, key, len);
      *(uint32_t *) out = crc32_pclmul_le_16(input, (size_t)len, seed);
    }
    else
#endif
    {
      unsigned char const *input = (unsigned char const *)malloc(len);
      memcpy((void*)input, key, len);
      *(uint32_t *) out = crc32_pclmul_le_16(input, (size_t)len, seed);
      free ((void*)input);
    }
  }
#else
  assert(((uintptr_t)key & 15) == 0); // input must be 16byte aligned
  *(uint32_t *) out = crc32_pclmul_le_16((unsigned char const *)key, (size_t)len, seed);
#endif
}
void CityHashCrc64_test(const void *key, int len, uint32_t seed, void *out);
void CityHashCrc128_test(const void *key, int len, uint32_t seed, void *out);
void falkhash_test_cxx(const void *key, int len, uint32_t seed, void *out);
#endif
size_t fibonacci(const char *key, int len, uint32_t seed);
inline void fibonacci_test(const void *key, int len, uint32_t seed, void *out) {
  *(size_t *)out = fibonacci((const char *)key, len, seed);
}
size_t FNV2(const char *key, int len, size_t seed);
inline void FNV2_test(const void *key, int len, uint32_t seed, void *out) {
  *(size_t *)out = FNV2((const char *)key, len, (size_t)seed);
}
uint32_t FNV32a(const void *key, int len, uint32_t seed);
inline void FNV32a_test(const void *key, int len, uint32_t seed, void *out) {
  *(uint32_t *)out = FNV32a((const char *)key, len, seed);
}
uint32_t FNV32a_YoshimitsuTRIAD(const char *key, int len, uint32_t seed);
inline void FNV32a_YT_test(const void *key, int len, uint32_t seed, void *out) {
  *(uint32_t *)out = FNV32a_YoshimitsuTRIAD((const char *)key, len, seed);
}
#ifdef HAVE_INT64
uint32_t FNV1A_Totenschiff(const char *key, int len, uint32_t seed);
inline void FNV1A_Totenschiff_test(const void *key, int len, uint32_t seed,
                                   void *out) {
  *(uint32_t *)out = FNV1A_Totenschiff((const char *)key, len, seed);
}
uint32_t FNV1A_Pippip_Yurii(const char *key, int wrdlen, uint32_t seed);
inline void FNV1A_PY_test(const void *key, int len, uint32_t seed, void *out) {
  *(uint32_t *)out = FNV1A_Pippip_Yurii((const char *)key, len, seed);
}
#endif
uint64_t FNV64a(const char *key, int len, uint64_t seed);
inline void FNV64a_test(const void *key, int len, uint32_t seed, void *out) {
  *(uint64_t *)out = FNV64a((const char *)key, len, (uint64_t)seed);
}
uint64_t fletcher2(const char *key, int len, uint64_t seed);
inline void fletcher2_test(const void *key, int len, uint32_t seed, void *out) {
  *(uint64_t *) out = fletcher2((const char *)key, len, (uint64_t)seed);
}
uint64_t fletcher4(const char *key, int len, uint64_t seed);
inline void fletcher4_test(const void *key, int len, uint32_t seed, void *out) {
  *(uint64_t *) out = fletcher2((const char *)key, len, (uint64_t)seed);
}
uint32_t Bernstein(const char *key, int len, uint32_t seed);
inline void Bernstein_test(const void *key, int len, uint32_t seed, void *out) {
  *(uint32_t *) out = Bernstein((const char *)key, len, seed);
}
uint32_t sdbm(const char *key, int len, uint32_t hash);
inline void sdbm_test(const void *key, int len, uint32_t seed, void *out) {
  *(uint32_t *) out = sdbm((const char *)key, len, seed);
}
uint32_t x17(const char *key, int len, uint32_t h);
inline void x17_test(const void *key, int len, uint32_t seed, void *out) {
  *(uint32_t *) out = x17((const char *)key, len, seed);
}
uint32_t JenkinsOOAT(const char *key, int len, uint32_t hash);
inline void JenkinsOOAT_test(const void *key, int len, uint32_t seed, void *out) {
  *(uint32_t *) out = JenkinsOOAT((const char *)key, len, seed);
}
uint32_t JenkinsOOAT_perl(const char *key, int len, uint32_t hash);
inline void JenkinsOOAT_perl_test(const void *key, int len, uint32_t seed, void *out) {
  *(uint32_t *) out = JenkinsOOAT_perl((const char *)key, len, seed);
}
uint32_t GoodOAAT(const char *key, int len, uint32_t hash);
inline void GoodOAAT_test(const void *key, int len, uint32_t seed, void *out) {
  *(uint32_t *) out = GoodOAAT((const char *)key, len, seed);
}
uint32_t MicroOAAT(const char *key, int len, uint32_t hash);
inline void MicroOAAT_test(const void *key, int len, uint32_t seed, void *out) {
  *(uint32_t *) out = MicroOAAT((const char *)key, len, seed);
}
uint32_t SuperFastHash (const char * data, int len, int32_t hash);
inline void SuperFastHash_test(const void *key, int len, uint32_t seed, void *out) {
  *(uint32_t*)out = SuperFastHash((const char*)key, len, seed);
}
uint32_t lookup3(const char *key, int len, uint32_t hash);
inline void lookup3_test(const void *key, int len, uint32_t seed, void *out) {
  *(uint32_t *) out = lookup3((const char *)key, len, seed);
}
uint32_t MurmurOAAT(const char *key, int len, uint32_t hash);
inline void MurmurOAAT_test(const void *key, int len, uint32_t seed, void *out) {
  *(uint32_t *) out = MurmurOAAT((const char *)key, len, seed);
}
uint32_t Crap8(const uint8_t * key, uint32_t len, uint32_t seed);
inline void Crap8_test(const void *key, int len, uint32_t seed, void *out) {
  *(uint32_t *) out = Crap8((const uint8_t *)key, len, seed);
}

void CityHash32_test(const void *key, int len, uint32_t seed, void *out);
void CityHash64noSeed_test(const void *key, int len, uint32_t seed, void *out);
void CityHash64_test(const void *key, int len, uint32_t seed, void *out);
inline void CityHash64_low_test(const void *key, int len, uint32_t seed, void *out) {
  uint64_t result;
  CityHash64_test(key, len, seed, &result);
  *(uint32_t *)out = (uint32_t)result;
}
void CityHash128_test(const void *key, int len, uint32_t seed, void *out);
// objsize: eb0-3b91: 11489 (mult. variants per len)
void FarmHash32_test       ( const void * key, int len, uint32_t seed, void * out );
// objsize: 0-eae: 3758 (mult. variants per len)
void FarmHash64_test       ( const void * key, int len, uint32_t seed, void * out );
void FarmHash64noSeed_test ( const void * key, int len, uint32_t seed, void * out );
// objsize: 44a0-4543: 163
void FarmHash128_test      ( const void * key, int len, uint32_t seed, void * out );
// objsize: 0x2c70-0x2f6a farmhash32_su_with_seed
void farmhash32_c_test     ( const void * key, int len, uint32_t seed, void * out );
// objsize: 4a20-4a82/5b0-5fd/660-1419: 3688 farmhash64_na_with_seeds
void farmhash64_c_test     ( const void * key, int len, uint32_t seed, void * out );
// objsize: 4140-48a2: 1890
void farmhash128_c_test    ( const void * key, int len, uint32_t seed, void * out );

// all 3 using the same Hash128
// objsize: 0-8ad: 2221
void SpookyHash32_test     ( const void * key, int len, uint32_t seed, void * out );
void SpookyHash64_test     ( const void * key, int len, uint32_t seed, void * out );
void SpookyHash128_test    ( const void * key, int len, uint32_t seed, void * out );

//----------
// Used internally as C++
uint32_t MurmurOAAT ( const char * key, int len, uint32_t seed );

// MurmurHash2
void MurmurHash2_test      ( const void * key, int len, uint32_t seed, void * out );
void MurmurHash2A_test     ( const void * key, int len, uint32_t seed, void * out );

void siphash_test          ( const void * key, int len, uint32_t seed, void * out );
void siphash13_test        ( const void * key, int len, uint32_t seed, void * out );
void halfsiphash_test      ( const void * key, int len, uint32_t seed, void * out );
extern "C" void chaskey_c  ( const void * key, int len, uint64_t seed, void * out );
extern "C" void chaskey_init();
inline void
chaskey_test(const void *input, int len, uint32_t seed, void *out)
{
  uint64_t lseed = (uint64_t)seed;
  chaskey_c (input, len, lseed, out);
}

//-----------------------------------------------------------------------------
// Test harnesses for Murmur1/2

inline void MurmurHash1_test ( const void * key, int len, uint32_t seed, void * out )
{
  *(uint32_t*)out = MurmurHash1(key,len,seed);
}

inline void MurmurHash2_test ( const void * key, int len, uint32_t seed, void * out )
{
  *(uint32_t*)out = MurmurHash2(key,len,seed);
}

inline void MurmurHash2A_test ( const void * key, int len, uint32_t seed, void * out )
{
  *(uint32_t*)out = MurmurHash2A(key,len,seed);
}

#if __WORDSIZE >= 64
inline void MurmurHash64A_test ( const void * key, int len, uint32_t seed, void * out )
{
  *(uint64_t*)out = MurmurHash64A(key,len,seed);
}
#endif
#ifdef HAVE_INT64
inline void MurmurHash64B_test ( const void * key, int len, uint32_t seed, void * out )
{
  *(uint64_t*)out = MurmurHash64B(key,len,seed);
}
#endif

inline void jodyhash32_test( const void * key, int len, uint32_t seed, void * out ) {
  *(uint32_t*)out = jody_block_hash32((const jodyhash32_t *)key, (jodyhash32_t) seed, (size_t) len);
}
#ifdef HAVE_INT64
inline void jodyhash64_test( const void * key, int len, uint32_t seed, void * out ) {
  *(uint64_t*)out = jody_block_hash((const jodyhash_t *)key, (jodyhash_t) seed, (size_t) len);
}
#endif

inline void xxHash32_test( const void * key, int len, uint32_t seed, void * out ) {
  // objsize 10-104 + 3e0-5ce: 738
  *(uint32_t*)out = (uint32_t) XXH32(key, (size_t) len, (unsigned) seed);
}
#ifdef HAVE_INT64
inline void xxHash64_test( const void * key, int len, uint32_t seed, void * out ) {
  // objsize 630-7fc + c10-1213: 1999
  *(uint64_t*)out = (uint64_t) XXH64(key, (size_t) len, (unsigned long long) seed);
}
#endif

#define restrict // oddly enough, seems to choke on this keyword
#include "xxh3.h"

#ifdef HAVE_INT64
inline void xxh3_test( const void * key, int len, uint32_t seed, void * out ) {
  // objsize 12d0-15b8: 744
  *(uint64_t*)out = (uint64_t) XXH3_64bits_withSeed(key, (size_t) len, seed);
}
#endif

inline void xxh3low_test( const void * key, int len, uint32_t seed, void * out ) {
  (void)seed;
  // objsize 12d0-15b8: 744 + 1f50-1f5c: 756
  *(uint32_t*)out = (uint32_t) XXH3_64bits_withSeed(key, (size_t) len, seed);
}

#ifdef HAVE_INT64
inline void xxh128_test( const void * key, int len, uint32_t seed, void * out ) {
  // objsize 1f60-2354: 1012
  *(XXH128_hash_t*)out = XXH128(key, (size_t) len, seed);
}

inline void xxh128low_test( const void * key, int len, uint32_t seed, void * out ) {
  *(uint64_t*)out = (uint64_t) (XXH128(key, (size_t) len, seed).low64);
}

#ifdef HAVE_INT64
inline void metrohash64_test ( const void * key, int len, uint32_t seed, void * out ) {
  MetroHash64::Hash((const uint8_t *)key, (uint64_t)len, (uint8_t *)out, seed);
}
inline void metrohash64_1_test ( const void * key, int len, uint32_t seed, void * out ) {
  metrohash64_1((const uint8_t *)key, (uint64_t)len, seed, (uint8_t *)out);
}
inline void metrohash64_2_test ( const void * key, int len, uint32_t seed, void * out ) {
  metrohash64_2((const uint8_t *)key, (uint64_t)len, seed, (uint8_t *)out);
}
inline void metrohash128_test ( const void * key, int len, uint32_t seed, void * out ) {
  MetroHash128::Hash((const uint8_t *)key, (uint64_t)len, (uint8_t *)out, seed);
}
inline void metrohash128_1_test ( const void * key, int len, uint32_t seed, void * out ) {
  metrohash128_1((const uint8_t *)key, (uint64_t)len, seed, (uint8_t *)out);
}
inline void metrohash128_2_test ( const void * key, int len, uint32_t seed, void * out ) {
  metrohash128_2((const uint8_t *)key, (uint64_t)len, seed, (uint8_t *)out);
}
#endif
#if defined(__SSE4_2__) && defined(__x86_64__)
inline void metrohash64crc_1_test ( const void * key, int len, uint32_t seed, void * out ) {
  metrohash64crc_1((const uint8_t *)key, (uint64_t)len, seed, (uint8_t *)out);
}
inline void metrohash64crc_2_test ( const void * key, int len, uint32_t seed, void * out ) {
  metrohash64crc_2((const uint8_t *)key, (uint64_t)len, seed, (uint8_t *)out);
}
inline void metrohash128crc_1_test ( const void * key, int len, uint32_t seed, void * out ) {
  metrohash128crc_1((const uint8_t *)key, (uint64_t)len, seed, (uint8_t *)out);
}
inline void metrohash128crc_2_test ( const void * key, int len, uint32_t seed, void * out ) {
  metrohash128crc_2((const uint8_t *)key, (uint64_t)len, seed, (uint8_t *)out);
}
#endif
inline void cmetrohash64_1_test ( const void * key, int len, uint32_t seed, void * out ) {
  // objsize 0-28c: 652
  cmetrohash64_1((const uint8_t *)key,(uint64_t)len,seed,(uint8_t *)out);
}
inline void cmetrohash64_1_optshort_test ( const void * key, int len, uint32_t seed, void * out ) {
  // objsize 0-db2: 3506
  cmetrohash64_1_optshort((const uint8_t *)key,(uint64_t)len,seed,(uint8_t *)out);
}
inline void cmetrohash64_2_test ( const void * key, int len, uint32_t seed, void * out ) {
  // objsize 290-51f: 655
  cmetrohash64_2((const uint8_t *)key,(uint64_t)len,seed,(uint8_t *)out);
}
#endif

inline void fasthash32_test ( const void * key, int len, uint32_t seed, void * out ) {
  *(uint32_t*)out = fasthash32(key, (size_t) len, seed);
}
#ifdef HAVE_INT64
inline void fasthash64_test ( const void * key, int len, uint32_t seed, void * out ) {
  *(uint64_t*)out = fasthash64(key, (size_t) len, (uint64_t)seed);
}
#endif

// objsize 0-778: 1912
void mum_hash_test(const void * key, int len, uint32_t seed, void * out);

inline void mum_low_test ( const void * key, int len, uint32_t seed, void * out ) {
  uint64_t result;
  mum_hash_test(key, len, seed, &result);
  *(uint32_t*)out = (uint32_t)result;
}


//-----------------------------------------------------------------------------

#define T1HA0_RUNTIME_SELECT 0
#ifdef HAVE_AESNI
# define T1HA0_AESNI_AVAILABLE 1
#else
# define T1HA0_AESNI_AVAILABLE 0
#endif
#include "t1ha.h"

inline void t1ha2_atonce_test(const void * key, int len, uint32_t seed, void * out)
{
  // objsize 0-21d: 541
  *(uint64_t*)out = t1ha2_atonce(key, len, seed);
}

inline void t1ha2_stream_test(const void * key, int len, uint32_t seed, void * out)
{
  t1ha_context_t ctx;
  // objsize 570-bf1: 1665
  t1ha2_init(&ctx, seed, 0);
  t1ha2_update(&ctx, key, len);
  *(uint64_t*)out = t1ha2_final(&ctx, NULL);
}

inline void t1ha2_atonce128_test(const void * key, int len, uint32_t seed, void * out)
{
  // objsize b50-db5: 613
  *(uint64_t*)out = t1ha2_atonce128((uint64_t*)out + 1, key, len, seed);
}

inline void t1ha2_stream128_test(const void * key, int len, uint32_t seed, void * out)
{
  t1ha_context_t ctx;
  // objsize e20-14a1: 1665
  t1ha2_init(&ctx, seed, 0);
  t1ha2_update(&ctx, key, len);
  *(uint64_t*)out = t1ha2_final(&ctx, (uint64_t*)out + 1);
}

inline void t1ha1_64le_test(const void * key, int len, uint32_t seed, void * out)
{
  // objsize 0-205: 517
  *(uint64_t*)out = t1ha1_le(key, len, seed);
}

inline void t1ha1_64be_test(const void * key, int len, uint32_t seed, void * out)
{
  // objsize 280-4ab: 555
  *(uint64_t*)out = t1ha1_be(key, len, seed);
}

inline void t1ha0_32le_test(const void * key, int len, uint32_t seed, void * out)
{
  // objsize 0-1fd: 509
  *(uint64_t*)out = t1ha0_32le(key, len, seed);
}

inline void t1ha0_32be_test(const void * key, int len, uint32_t seed, void * out)
{
  // objsize 250-465: 533
  *(uint64_t*)out = t1ha0_32be(key, len, seed);
}

#if T1HA0_AESNI_AVAILABLE
#ifndef _MSC_VER
inline void t1ha0_ia32aes_noavx_test(const void * key, int len, uint32_t seed, void * out)
{
  // objsize 0-39d: 925
  *(uint64_t*)out = t1ha0_ia32aes_noavx(key, len, seed);
}
#endif
#if defined(__AVX__)
inline void t1ha0_ia32aes_avx1_test(const void * key, int len, uint32_t seed, void * out)
{
  // objsize 0-34b: 843
  *(uint64_t*)out = t1ha0_ia32aes_avx(key, len, seed);
}
#endif /* __AVX__ */
#if defined(__AVX2__)
inline void t1ha0_ia32aes_avx2_test(const void * key, int len, uint32_t seed, void * out)
{
  // objsize 0-318: 792
  *(uint64_t*)out = t1ha0_ia32aes_avx2(key, len, seed);
}
#endif /* __AVX2__ */
#endif /* T1HA0_AESNI_AVAILABLE */

#if defined(__SSE4_2__) && defined(__x86_64__)
#include "clhash.h"
void clhash_init();
void clhash_seed_init(size_t seed);
void clhash_test (const void * key, int len, uint32_t seed, void * out);
#endif


#ifdef __SIZEOF_INT128__
   void multiply_shift_init();
   void multiply_shift_seed_init(size_t seed);
   void multiply_shift (const void * key, int len, uint32_t seed, void * out);
   void pair_multiply_shift (const void *key, int len, uint32_t seed, void *out);
   void poly_mersenne_init();
   void poly_mersenne_seed_init(size_t seed);
   void poly_0_mersenne (const void* key, int len, uint32_t seed, void* out);
   void poly_1_mersenne (const void* key, int len, uint32_t seed, void* out);
   void poly_2_mersenne (const void* key, int len, uint32_t seed, void* out);
   void poly_3_mersenne (const void* key, int len, uint32_t seed, void* out);
   void poly_4_mersenne (const void* key, int len, uint32_t seed, void* out);
#endif

#ifdef __SIZEOF_INT128__
   inline void tabulation_init() {
      tabulation_seed_init(2);
   }
   inline void tabulation_test (const void * key, int len, uint32_t seed, void * out) {
      *(uint64_t*)out = tabulation_hash(key, len, seed);
   }
#endif

inline void tabulation_32_init() {
   tabulation_32_seed_init(0);
}
inline void tabulation_32_test (const void * key, int len, uint32_t seed, void * out) {
   *(uint32_t*)out = tabulation_32_hash(key, len, seed);
}

void HighwayHash_init();
// objsize 20-a12: 2546
void HighwayHash64_test (const void * key, int len, uint32_t seed, void * out);

#ifdef HAVE_BIT32
// native 32bit. objsize: 0x80f6a30-0x80f6bca: 410
#include "wyhash32.h"
inline void wyhash32_test (const void * key, int len, uint32_t seed, void * out) {
  *(uint32_t*)out = wyhash32(key, (uint64_t)len, (unsigned)seed);
}
#endif

#ifdef HAVE_INT64
//https://github.com/wangyi-fudan/wyhash
#include "wyhash.h"
// objsize 40c8f0-40cc9a: 938
inline void wyhash_test (const void * key, int len, uint32_t seed, void * out) {
  *(uint64_t*)out = wyhash(key, (uint64_t)len, (uint64_t)seed, _wyp);
}
#ifndef HAVE_BIT32
inline void wyhash32low (const void * key, int len, uint32_t seed, void * out) {
  *(uint32_t*)out = 0xFFFFFFFF & wyhash(key, (uint64_t)len, (uint64_t)seed, _wyp);
}
#endif

#include "o1hash.h"
// unseeded. objsize: 101
inline void o1hash_test (const void * key, int len, uint32_t seed, void * out) {
  *(uint64_t*)out = o1hash(key, (uint64_t)len);
}

//https://github.com/vnmakarov/mir/blob/master/mir-hash.h
#include "mir-hash.h"
inline void mirhash_test (const void * key, int len, uint32_t seed, void * out) {
  // objsize 2950-2da8: 1112
  *(uint64_t*)out = mir_hash(key, (uint64_t)len, (uint64_t)seed);
}
inline void mirhash32low (const void * key, int len, uint32_t seed, void * out) {
  *(uint32_t*)out = 0xFFFFFFFF & mir_hash(key, (uint64_t)len, (uint64_t)seed);
}
inline void mirhashstrict_test (const void * key, int len, uint32_t seed, void * out) {
  // objsize 2950-2da8: 1112
  *(uint64_t*)out = mir_hash_strict(key, (uint64_t)len, (uint64_t)seed);
}
inline void mirhashstrict32low (const void * key, int len, uint32_t seed, void * out) {
  *(uint32_t*)out = 0xFFFFFFFF & mir_hash_strict(key, (uint64_t)len, (uint64_t)seed);
}

//TODO MSVC
#ifndef _MSC_VER
void tsip_init();
void tsip_test (const void * key, int len, uint32_t seed, void * out);
// objsize 0-207: 519
extern "C" uint64_t tsip(const unsigned char *seed, const unsigned char *m, uint64_t len);

extern "C" uint64_t seahash(const char *key, int len, uint64_t seed);
// objsize 29b0-2d17: 871
inline void seahash_test (const void *key, int len, uint32_t seed, void *out) {
  *(uint64_t*)out = seahash((const char *)key, len, (uint64_t)seed);
}
inline void seahash32low (const void *key, int len, uint32_t seed, void *out) {
  uint64_t result = seahash((const char *)key, len, (uint64_t)seed);
  *(uint32_t*)out = (uint32_t)(UINT64_C(0xffffffff) & result);
}
#endif /* !MSVC */
#endif /* HAVE_INT64 */

//----------
// Cryptographic hashes

#include "md5.h"

inline void md5_128(const void *key, int len, uint32_t seed, void *out) {
  md5_context md5_ctx;
  md5_starts( &md5_ctx );
  md5_ctx.state[0] ^= seed;
  md5_update( &md5_ctx, (unsigned char *)key, len );
  md5_finish( &md5_ctx, (unsigned char *)out );
  //memset( &md5_ctx.buffer, 0, 64+64+64 ); // for buffer, ipad, opad
}

inline void md5_32(const void *key, int len, uint32_t seed, void *out) {
  unsigned char hash[16];
  md5_context md5_ctx;
  md5_starts( &md5_ctx );
  md5_ctx.state[0] ^= seed;
  md5_update( &md5_ctx, (unsigned char *)key, len );
  md5_finish( &md5_ctx, hash );
  //memset( &md5_ctx.buffer, 0, 64+64+64 ); // for buffer, ipad, opad
  memcpy(out, hash, 4);
}

#include "sha1.h"
inline void sha1_160(const void *key, int len, uint32_t seed, void *out) {
  SHA1_CTX context;

  SHA1_Init(&context);
  context.state[0] ^= seed;
  SHA1_Update(&context, (uint8_t*)key, len);
  SHA1_Final(&context, (uint8_t*)out);
}

void SHA1_Update(SHA1_CTX *context, const uint8_t *data, const size_t len);
inline void sha1_32a(const void *key, int len, uint32_t seed, void *out) {
  SHA1_CTX context;
  uint8_t *digest = (uint8_t *)out;

  SHA1_Init(&context);
  context.state[0] ^= seed;
  SHA1_Update(&context, (uint8_t *)key, len);
  //SHA1_Final(&context, digest); //inlined below
  //memcpy(out, digest, 4);

  unsigned i;
  uint8_t finalcount[8];
  uint8_t c;
  for (i = 0; i < 8; i++) {
    finalcount[i] =
        /* Endian independent */
        (uint8_t)(context.count[(i >= 4 ? 0 : 1)] >> ((3 - (i & 3)) * 8));
  }
  c = 0200;
  SHA1_Update(&context, &c, 1);
  while ((context.count[0] & 504) != 448) {
    c = 0000;
    SHA1_Update(&context, &c, 1);
  }
  SHA1_Update(&context, finalcount, 8); /* Should cause a SHA1_Transform() */
  for (i = 0; i < 4; i++) { // only the needed bytes
    digest[i] = (uint8_t)(context.state[i >> 2] >> ((3 - (i & 3)) * 8));
  }
}

#include "tomcrypt.h"
#ifndef _MAIN_CPP
extern
#endif
       hash_state ltc_state;

int blake2b_init(hash_state * md, unsigned long outlen,
                 const unsigned char *key, unsigned long keylen);
inline void blake2b160_test(const void *key, int len, uint32_t seed, void *out)
{
  // objsize
  blake2b_init(&ltc_state, 20, NULL, 0);
  ltc_state.blake2b.h[0] = CONST64(0x6a09e667f3bcc908) ^ seed; // mix seed into lowest int
  blake2b_process(&ltc_state, (unsigned char *)key, len);
  blake2b_done(&ltc_state, (unsigned char *)out);
}
inline void blake2b224_test(const void *key, int len, uint32_t seed, void *out)
{
  // objsize
  blake2b_init(&ltc_state, 28, NULL, 0);
  ltc_state.blake2b.h[0] = CONST64(0x6a09e667f3bcc908) ^ seed;
  blake2b_process(&ltc_state, (unsigned char *)key, len);
  blake2b_done(&ltc_state, (unsigned char *)out);
}
inline void blake2b256_test(const void *key, int len, uint32_t seed, void *out)
{
  // objsize
  blake2b_init(&ltc_state, 32, NULL, 0);
  ltc_state.blake2b.h[0] = CONST64(0x6a09e667f3bcc908) ^ seed;
  blake2b_process(&ltc_state, (unsigned char *)key, len);
  blake2b_done(&ltc_state, (unsigned char *)out);
}
inline void blake2b256_64(const void *key, int len, uint32_t seed, void *out)
{
  // objsize
  unsigned char buf[32];
  blake2b_init(&ltc_state, 32, NULL, 0);
  ltc_state.blake2b.h[0] = CONST64(0x6a09e667f3bcc908) ^ seed;
  blake2b_process(&ltc_state, (unsigned char *)key, len);
  blake2b_done(&ltc_state, buf);
  memcpy(out, buf, 8);
}
int blake2s_init(hash_state * md, unsigned long outlen,
                 const unsigned char *key, unsigned long keylen);
inline void blake2s128_test(const void * key, int len, uint32_t seed, void * out)
{
  // objsize
  blake2s_init(&ltc_state, 16, NULL, 0);
  ltc_state.blake2s.h[0] = 0x6A09E667UL ^ seed;
  blake2s_process(&ltc_state, (unsigned char *)key, len);
  blake2s_done(&ltc_state, (unsigned char *)out);
}
inline void blake2s160_test(const void * key, int len, uint32_t seed, void * out)
{
  // objsize
  blake2s_init(&ltc_state, 20, NULL, 0);
  ltc_state.blake2s.h[0] = 0x6A09E667UL ^ seed;
  blake2s_process(&ltc_state, (unsigned char *)key, len);
  blake2s_done(&ltc_state, (unsigned char *)out);
}
inline void blake2s224_test(const void * key, int len, uint32_t seed, void * out)
{
  // objsize
  blake2s_init(&ltc_state, 28, NULL, 0);
  ltc_state.blake2s.h[0] = 0x6A09E667UL ^ seed;
  blake2s_process(&ltc_state, (unsigned char *)key, len);
  blake2s_done(&ltc_state, (unsigned char *)out);
}
inline void blake2s256_test(const void * key, int len, uint32_t seed, void * out)
{
  // objsize
  blake2s_init(&ltc_state, 32, NULL, 0);
  ltc_state.blake2s.h[0] = 0x6A09E667UL ^ seed;
  blake2s_process(&ltc_state, (unsigned char *)key, len);
  blake2s_done(&ltc_state, (unsigned char *)out);
}
inline void blake2s256_64(const void * key, int len, uint32_t seed, void * out)
{
  // objsize
  unsigned char buf[32];
  blake2s_init(&ltc_state, 32, NULL, 0);
  ltc_state.blake2s.h[0] = 0x6A09E667UL ^ seed;
  blake2s_process(&ltc_state, (unsigned char *)key, len);
  blake2s_done(&ltc_state, buf);
  memcpy(out, buf, 8);
}
inline void sha2_224(const void *key, int len, uint32_t seed, void *out)
{
  // objsize
  unsigned char buf[28];
  sha224_init(&ltc_state);
  ltc_state.sha256.state[0] = 0xc1059ed8UL ^ seed;
  sha224_process(&ltc_state, (unsigned char *)key, len);
  sha224_done(&ltc_state, (unsigned char *)out);
}
inline void sha2_224_64(const void *key, int len, uint32_t seed, void *out)
{
  // objsize
  unsigned char buf[28];
  sha224_init(&ltc_state);
  ltc_state.sha256.state[0] = 0xc1059ed8UL ^ seed;
  sha224_process(&ltc_state, (unsigned char *)key, len);
  sha224_done(&ltc_state, buf);
  memcpy(out, buf, 8);
}
inline void sha2_256(const void *key, int len, uint32_t seed, void *out)
{
  // objsize
  sha256_init(&ltc_state);
  ltc_state.sha256.state[0] = 0x6A09E667UL ^ seed;
  sha256_process(&ltc_state, (unsigned char *)key, len);
  sha256_done(&ltc_state, (unsigned char *)out);
}
inline void sha2_256_64(const void *key, int len, uint32_t seed, void *out)
{
  // objsize
  unsigned char buf[32];
  sha256_init(&ltc_state);
  ltc_state.sha256.state[0] = 0x6A09E667UL ^ seed;
  sha256_process(&ltc_state, (unsigned char *)key, len);
  sha256_done(&ltc_state, buf);
  memcpy(out, buf, 8);
}
inline void rmd128(const void *key, int len, uint32_t seed, void *out)
{
  // objsize
  rmd128_init(&ltc_state);
  ltc_state.rmd128.state[0] = 0x67452301UL ^ seed;
  rmd128_process(&ltc_state, (unsigned char *)key, len);
  rmd128_done(&ltc_state, (unsigned char *)out);
}
inline void rmd160(const void *key, int len, uint32_t seed, void *out)
{
  // objsize
  rmd160_init(&ltc_state);
  ltc_state.rmd160.state[0] = 0x67452301UL ^ seed;
  rmd160_process(&ltc_state, (unsigned char *)key, len);
  rmd160_done(&ltc_state, (unsigned char *)out);
}
inline void rmd256(const void *key, int len, uint32_t seed, void *out)
{
  // objsize
  rmd256_init(&ltc_state);
  ltc_state.rmd256.state[0] = 0x67452301UL ^ seed;
  rmd256_process(&ltc_state, (unsigned char *)key, len);
  rmd256_done(&ltc_state, (unsigned char *)out);
}
// Keccak:
inline void sha3_256_64(const void *key, int len, uint32_t seed, void *out)
{
  // objsize
  unsigned char buf[32];
  sha3_256_init(&ltc_state);
  ltc_state.sha3.s[0] = CONST64(1) ^ seed;
  sha3_process(&ltc_state, (unsigned char *)key, len);
  sha3_done(&ltc_state, buf);
  memcpy(out, buf, 8);
}
inline void sha3_256(const void *key, int len, uint32_t seed, void *out)
{
  // objsize
  unsigned char buf[32];
  sha3_256_init(&ltc_state);
  ltc_state.sha3.s[0] = CONST64(1) ^ seed;
  sha3_process(&ltc_state, (unsigned char *)key, len);
  sha3_done(&ltc_state, (unsigned char *)out);
}
inline void wysha(const void *key, int len, unsigned seed, void *out) {
  uint64_t s[4] = {wyhash(key, len, seed + 0, _wyp), wyhash(key, len, seed + 1, _wyp),
                   wyhash(key, len, seed + 2, _wyp), wyhash(key, len, seed + 3, _wyp)};
  memcpy(out, s, 32);
}

#if defined(HAVE_AESNI) && defined(__SIZEOF_INT128__) /*&& !defined(HAVE_UBSAN)*/
#include "meow_hash_x64_aesni.h"
// objsize: 0x84b0-8b94 = 1764
inline void MeowHash128_test(const void *key, int len, unsigned seed, void *out) {
  *(int unsigned *)MeowDefaultSeed = seed;
  meow_u128 h = MeowHash(MeowDefaultSeed, (meow_umm)len, (void*)key);
  ((uint64_t *)out)[0] = MeowU64From(h, 0);
  ((uint64_t *)out)[1] = MeowU64From(h, 1);
}
inline void MeowHash64_test(const void *key, int len, unsigned seed, void *out) {
  *(int unsigned *)MeowDefaultSeed = seed;
  meow_u128 h = MeowHash(MeowDefaultSeed, (meow_umm)len, (void*)key);
  *(uint64_t *)out = MeowU64From(h, 0);
}
inline void MeowHash32_test(const void *key, int len, unsigned seed, void *out) {
  *(int unsigned *)MeowDefaultSeed = seed;
  meow_u128 h = MeowHash(MeowDefaultSeed, (meow_umm)len, (void*)key);
  *(uint32_t *)out = MeowU32From(h, 0);
}
#endif

#if defined(HAVE_SHANI) && defined(__x86_64__)
extern "C" void sha1_process_x86(uint32_t *state, const uint8_t *data, uint32_t length);
extern "C" void sha256_process_x86(uint32_t *state, const uint8_t *data, uint32_t length);

// Note: improved native SHA functions with seed.
// These functions need to be padded to 64byte blocks, so its trivial to create max
// 64 collisions for each key.
inline void sha1ni(const void *key, int len, uint32_t seed, void *out)
{
  // objsize: 0x2c1 + this (0x408bac - 0x408a90) = 989
  uint32_t state[5] = {0x67452301, 0xefcdab89, 0x98badcfe, 0x10325476, 0xc3d2e1f0};
  state[0] = 0x67452301U ^ seed;
  if (len < 64) {
    uint8_t padded[64];
    memcpy (padded, key, len);
    memset (&padded[len], 0, 64-len);
    sha1_process_x86(state, (const uint8_t*)padded, 64);
  } else if (len % 64) {
    uint32_t lenpadded = len + (64 - len % 64);
    uint8_t *padded = (uint8_t*)malloc (lenpadded);
    memcpy (padded, key, len);
    memset (&padded[len], 0, lenpadded-len);
    sha1_process_x86(state, (const uint8_t*)padded, lenpadded);
    free (padded);
  } else {
    sha1_process_x86(state, (const uint8_t*)key, (uint32_t)len);
  }
  memcpy(out, state, 20);
}
// Note: improved native SHA functions with seed and len encoded into the seed, to prevent Zeroes.
// These functions need to be padded to 64byte blocks, so it would be trivial to create max
// 64 collisions for each key.
inline void sha1ni_32(const void *key, int len, uint32_t seed, void *out)
{
  uint32_t state[5] = {0x67452301, 0xefcdab89, 0x98badcfe, 0x10325476, 0xc3d2e1f0};
  state[0] = 0x67452301U ^ seed;
  if (len < 64) {
    uint8_t padded[64];
    memcpy (padded, key, len);
    memset (&padded[len], 0, 64-len);
    state[0] += len;
    sha1_process_x86(state, (const uint8_t*)padded, 64);
  } else if (len % 64) {
    uint32_t lenpadded = len + (64 - len % 64);
    uint8_t *padded = (uint8_t*)malloc (lenpadded);
    memcpy (padded, key, len);
    memset (&padded[len], 0, lenpadded-len);
    state[0] += len;
    sha1_process_x86(state, (const uint8_t*)padded, lenpadded);
    free (padded);
  } else {
    sha1_process_x86(state, (const uint8_t*)key, (uint32_t)len);
  }
  *(uint32_t *)out = *(uint32_t *)state;
}
inline void sha2ni_256(const void *key, int len, uint32_t seed, void *out)
{
  // objsize: 0x2f1 + this (0x4095c0-0x408820) = 4241
  uint32_t state[8] = {
                       0x6a09e667, 0xbb67ae85, 0x3c6ef372, 0xa54ff53a,
                       0x510e527f, 0x9b05688c, 0x1f83d9ab, 0x5be0cd19
  };
  state[0] = 0x6a09e667U ^ seed;
  if (len < 64) {
    uint8_t padded[64];
    memcpy (padded, key, len);
    memset (&padded[len], 0, 64-len);
    //state[0] += len; to prevent Zeroes
    sha256_process_x86(state, (const uint8_t*)padded, 64);
  } else if (len % 64) {
    uint32_t lenpadded = len + (64 - len % 64);
    uint8_t *padded = (uint8_t*)malloc (lenpadded);
    memcpy (padded, key, len);
    memset (&padded[len], 0, lenpadded-len);
    //state[0] += len;
    sha256_process_x86(state, (const uint8_t*)padded, lenpadded);
    free (padded);
  } else {
    sha256_process_x86(state, (const uint8_t*)key, (uint32_t)len);
  }
  memcpy(out, state, 32);
}
inline void sha2ni_256_64(const void *key, int len, uint32_t seed, void *out)
{
  uint32_t state[8] = {
                       0x6a09e667, 0xbb67ae85, 0x3c6ef372, 0xa54ff53a,
                       0x510e527f, 0x9b05688c, 0x1f83d9ab, 0x5be0cd19
  };
  state[0] = 0x6a09e667U ^ seed;
  if (len < 64) {
    uint8_t padded[64];
    memcpy (padded, key, len);
    memset (&padded[len], 0, 64-len);
    state[0] += len;
    sha256_process_x86(state, (const uint8_t*)padded, 64);
  } else if (len % 64) {
    uint32_t lenpadded = len + (64 - len % 64);
    uint8_t *padded = (uint8_t*)malloc (lenpadded);
    memcpy (padded, key, len);
    memset (&padded[len], 0, lenpadded-len);
    state[0] += len;
    sha256_process_x86(state, (const uint8_t*)padded, lenpadded);
    free (padded);
  } else {
    sha256_process_x86(state, (const uint8_t*)key, (uint32_t)len);
  }
  *(uint64_t *)out = *(uint64_t *)state;
}
#endif

#ifdef HAVE_SSE2
# if defined(_MAIN_CPP)
#  include "farsh.h"
# else
#  ifdef __AVX2__
#   define FARSH_AVX2
#  elif defined HAVE_SSE42
#   define FARSH_SSE2
#  endif
# endif
# include "farsh.c"

// objsize: 0-3b0: 944
inline void farsh32_test ( const void * key, int len, unsigned seed, void * out )
{
  farsh_n(key,len,0,1,seed,out);
}
inline void farsh64_test ( const void * key, int len, unsigned seed, void * out )
{
  farsh_n(key,len,0,2,seed,out);
}
inline void farsh128_test ( const void * key, int len, unsigned seed, void * out )
{
  farsh_n(key,len,0,4,seed,out);
}
inline void farsh256_test ( const void * key, int len, unsigned seed, void * out )
{
  farsh_n(key,len,0,8,seed,out);
}
#endif

extern "C" {
#include "blake3/blake3_impl.h"
// The C API, serially
  inline void blake3c_test ( const void * key, int len, unsigned seed, void * out )
  {
    blake3_hasher hasher;
    blake3_hasher_init (&hasher);
    // mix-in seed. key0 is not enough to pass MomentChi2. but even mixing all does not help
    //for (int i = 0; i < 8; i++)
    //  hasher.key[i] ^= (uint32_t)seed;
    hasher.key[0] ^= (uint32_t)seed;
    blake3_hasher_update (&hasher, (uint8_t*)key, (size_t)len);
    blake3_hasher_finalize (&hasher, (uint8_t*)out, BLAKE3_OUT_LEN);
  }
}

#ifdef HAVE_BLAKE3
// The Rust API, parallized
typedef struct rust_array {
  char *ptr;
  size_t len;
} rs_arr;
extern "C" rs_arr *blake3_hash (const rs_arr *input);

inline void blake3_test ( const void * key, int len, unsigned seed, void * out )
{
  rs_arr input;
  rs_arr *result;
  input.ptr = (char*)key;
  input.len = (size_t)len;
  result = blake3_hash (&input);
  memcpy (out, result->ptr, 32);
}
inline void blake3_64 ( const void * key, int len, unsigned seed, void * out )
{
  rs_arr input;
  rs_arr *result;
  input.ptr = (char*)key;
  input.len = (size_t)len;
  result = blake3_hash (&input);
  *(uint64_t *)out = *(uint64_t *)result->ptr;
}
#endif

//64 objsize: a50-f69: 1305
//32 objsize: 1680-1abc: 1084

#ifndef DEBUG
#include "PMP_Multilinear_test.h"
#endif

// objsize: 452520-45358b: 4203
#include "beamsplitter.h"

// objsize: 452010-45251e: 1294 (BEBB4185)
#include "discohash.h"

#ifdef HAVE_AESNI
/* https://gist.github.com/majek/96dd615ed6c8aa64f60aac14e3f6ab5a */
uint64_t aesnihash(uint8_t *in, unsigned long src_sz);
inline void aesnihash_test ( const void * key, int len, unsigned seed, void * out )
{
  uint64_t result = aesnihash ((uint8_t *)key, (unsigned long)len);
  *(uint64_t *)out = result;
}
#endif

#ifdef HAVE_INT64
// https://github.com/avaneev/prvhash
// objsize: 4129f0 - 412bcc: 476
#include "prvhash42.h"
inline void prvhash42_32test ( const void * key, int len, unsigned seed, void * out )
{
  uint8_t hash[4] = {0};
  prvhash42 ((const uint8_t *)key, len, hash, 4, (uint64_t)seed, NULL);
  memcpy (out, hash, 4);
}
// objsize: 412850 - 4129ea: 960
inline void prvhash42_64test ( const void * key, int len, unsigned seed, void * out )
{
  uint8_t hash[8] = {0};
  prvhash42 ((const uint8_t *)key, len, hash, 8, (uint64_t)seed, NULL);
  memcpy (out, hash, 8);
}
// objsize: 412bd0 - 412d80: 432
inline void prvhash42_128test ( const void * key, int len, unsigned seed, void * out )
{
  uint8_t hash[16] = {0};
  prvhash42 ((const uint8_t *)key, len, hash, 16, (uint64_t)seed, NULL);
  memcpy (out, hash, 16);
}

#include "prvhash42s.h"
// objsize: 4137e0 - 4141ee: 2574
inline void prvhash42s_32test ( const void * key, int len, unsigned seed, void * out )
{
  PRVHASH42S_CTX ctx;
  uint64_t SeedXOR[ 4 ] = { (uint64_t)seed, (uint64_t)seed, (uint64_t)seed, (uint64_t)seed };
  prvhash42s_init( &ctx, (uint8_t* const)out, 4, SeedXOR, 0 );
  prvhash42s_update( &ctx, (const uint8_t*)key, (size_t)len );
  prvhash42s_final( &ctx );
}
// objsize: 4141f0 - 414c3d: 2637
inline void prvhash42s_64test ( const void * key, int len, unsigned seed, void * out )
{
  PRVHASH42S_CTX ctx;
  uint64_t SeedXOR[ 4 ] = { (uint64_t)seed, (uint64_t)seed, (uint64_t)seed, (uint64_t)seed };
  prvhash42s_init( &ctx, (uint8_t* const)out, 8, SeedXOR, 0 );
  prvhash42s_update( &ctx, (const uint8_t*)key, (size_t)len );
  prvhash42s_final( &ctx );
}
// objsize: 414230 - 4137dd: 2653
inline void prvhash42s_128test ( const void * key, int len, unsigned seed, void * out )
{
  PRVHASH42S_CTX ctx;
  uint64_t SeedXOR[ 4 ] = { (uint64_t)seed, (uint64_t)seed, (uint64_t)seed, (uint64_t)seed };
  prvhash42s_init( &ctx, (uint8_t* const)out, 16, SeedXOR, 0 );
  prvhash42s_update( &ctx, (const uint8_t*)key, (size_t)len );
  prvhash42s_final( &ctx );
}
#endif

// objsize: 408dd0 - 4090ae: 734
#include "mx3/mx3.h"
inline void mx3hash64_test ( const void * key, int len, uint32_t seed, void * out ) {
  *(uint64_t*)out = mx3::hash((const uint8_t*)(key), (size_t) len, (uint64_t)seed);
}

// objsize: 63d0 - 6575: 421
extern "C" {
#include "pengyhash.h"
}
inline void pengyhash_test ( const void * key, int len, uint32_t seed, void * out ) {
  *(uint64_t*)out = pengyhash (key, (size_t) len, seed);
}

// requires modern builtins, like __builtin_uaddll_overflow
#if defined(__SSE4_2__) && defined(__x86_64__) && !defined(_MSC_VER)

// objsize: 4bcb90 - 4bd18a
#include "umash.hpp"

#endif
