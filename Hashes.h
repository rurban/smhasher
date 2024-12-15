#pragma once

#include "Platform.h"
#include "Types.h"
#include <algorithm>

#include "MurmurHash1.h"
#include "MurmurHash2.h"
#include "MurmurHash3.h"
#include "PMurHash.h"
#include "xmsx.h"

#define XXH_INLINE_ALL
#include "xxhash.h"
#include "gxhash.h"

#include "metrohash/metrohash64.h"
#include "metrohash/metrohash128.h"
#include "cmetrohash.h"
#include "opt_cmetrohash.h"

#if defined(HAVE_SSE42) && (defined(__x86_64__) ||  defined(__aarch64__))
#include "metrohash/metrohash64crc.h"
#include "metrohash/metrohash128crc.h"
#endif

#ifdef HAVE_AHASH_C
#include "ahash.h"
#endif
#include "fasthash.h"
#include "jody_hash32.h"
#include "jody_hash64.h"

// objsize: 0-0x113 = 276
#include "tifuhash.h"
// objsize: 5f0-85f = 623
#include "floppsyhash.h"

#include "siphash.h"

#include "vmac.h"

#include "tabulation.h"

//----------
// These are _not_ hash functions (even though people tend to use crc32 as one...)

static inline bool BadHash_bad_seeds(std::vector<uint32_t> &seeds)
{
  seeds = std::vector<uint32_t> { UINT32_C(0) };
  return true;
}
void BadHash(const void *key, int len, uint32_t seed, void *out);
static inline bool sumhash_bad_seeds(std::vector<uint32_t> &seeds)
{
  seeds = std::vector<uint32_t> { UINT32_C(0) };
  return true;
}
void sumhash(const void *key, int len, uint32_t seed, void *out);
void sumhash32(const void *key, int len, uint32_t seed, void *out);

void DoNothingHash(const void *key, int len, uint32_t seed, void *out);
void NoopOAATReadHash(const void *key, int len, uint32_t seed, void *out);
void crc32(const void *key, int len, uint32_t seed, void *out);

static inline bool crc32c_bad_seeds(std::vector<uint32_t> &seeds)
{
  seeds = std::vector<uint32_t> { UINT32_C(0x111c2232) };
  return true;
}
//----------
// General purpose hashes

#if defined(HAVE_SSE2)
void hasshe2_test(const void *key, int len, uint32_t seed, void *out);
#endif
#if defined(HAVE_SSE42)
// This falls into a MSVC CL 14.16.27023 32bit compiler bug. 14.28.29910 works fine.
# ifndef HAVE_BROKEN_MSVC_CRC32C_HW
void crc32c_hw_test(const void *key, int len, uint32_t seed, void *out);
void crc64c_hw_test(const void *key, int len, uint32_t seed, void *out);
# endif
# if defined(__SSE4_2__) && (defined(__i686__) || defined(__x86_64__)) && !defined(_MSC_VER)
void crc32c_hw1_test(const void *key, int len, uint32_t seed, void *out);
# endif
static inline bool crc64c_bad_seeds(std::vector<uint64_t> &seeds)
{
  seeds = std::vector<uint64_t> { UINT64_C(0) };
  return true;
}
#if defined(__x86_64__)
void CityHashCrc128_test(const void *key, int len, uint32_t seed, void *out);
void CityHashCrc256_test(const void *key, int len, uint32_t seed, void *out);
#endif
#endif

#if defined(HAVE_CLMUL) && !defined(_MSC_VER)
/* Function from linux kernel 3.14. It computes the CRC over the given
 * buffer with initial CRC value <crc32>. The buffer is <len> byte in length,
 * and must be 16-byte aligned, and larger than 63. */
extern "C" uint32_t crc32_pclmul_le_16(unsigned char const *buffer, size_t len,
                                       uint32_t crc32);
void crc32c_pclmul_test(const void *key, int len, uint32_t seed, void *out);
static inline bool crc32c_pclmul_bad_seeds(std::vector<uint32_t> &seeds)
{
  seeds = std::vector<uint32_t> { UINT32_C(0) };
  return true;
}
static inline bool need_minlen64_align16(pfHash hash) {
  return hash == crc32c_pclmul_test;
}
void falkhash_test_cxx(const void *key, int len, uint32_t seed, void *out);
#else
static inline bool need_minlen64_align16(pfHash hash) {
  return false;
}
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
static inline bool FNV32a_bad_seeds(std::vector<uint32_t> &seeds)
{
  seeds = std::vector<uint32_t> { UINT32_C(0x811c9dc5) };
  return true;
}
inline void FNV32a_test(const void *key, int len, uint32_t seed, void *out) {
  *(uint32_t *)out = FNV32a((const char *)key, len, seed);
}
uint32_t FNV32a_YoshimitsuTRIAD(const char *key, int len, uint32_t seed);
static inline bool FNV32a_YT_bad_seeds(std::vector<uint32_t> &seeds)
{
  seeds = std::vector<uint32_t> { UINT32_C(0x811c9dc5) };
  return true;
}
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
void FNV128(uint64_t buf[2], const char *key, int len, uint64_t seed);
inline void FNV128_test(const void *key, int len, uint32_t seed, void *out) {
  FNV128((uint64_t*)out, (const char *)key, len, (uint64_t)seed);
}
uint64_t FNV64a(const char *key, int len, uint64_t seed);
inline void FNV64a_test(const void *key, int len, uint32_t seed, void *out) {
  *(uint64_t *)out = FNV64a((const char *)key, len, (uint64_t)seed);
}
static inline bool fletcher_bad_seeds(std::vector<uint64_t> &seeds)
{
  seeds = std::vector<uint64_t> { UINT64_C(0) };
  return true;
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
static inline bool Bernstein_bad_seeds(std::vector<uint32_t> &seeds)
{
  seeds = std::vector<uint32_t> { UINT32_C(0) };
  return true;
}
inline void Bernstein_test(const void *key, int len, uint32_t seed, void *out) {
  *(uint32_t *) out = Bernstein((const char *)key, len, seed);
}
uint32_t sdbm(const char *key, int len, uint32_t hash);
static inline bool sdbm_bad_seeds(std::vector<uint32_t> &seeds)
{
  seeds = std::vector<uint32_t> { UINT32_C(0) };
  return true;
}
inline void sdbm_test(const void *key, int len, uint32_t seed, void *out) {
  *(uint32_t *) out = sdbm((const char *)key, len, seed);
}
uint32_t x17(const char *key, int len, uint32_t h);
inline void x17_test(const void *key, int len, uint32_t seed, void *out) {
  *(uint32_t *) out = x17((const char *)key, len, seed);
}
uint32_t libiberty_hash(unsigned char *key, int len, uint32_t seed);
inline void libiberty_test(const void *key, int len, uint32_t seed, void *out) {
  *(uint32_t *) out = libiberty_hash((unsigned char *)key, len, seed);
}
inline void gcc_test(const void *key, int len, uint32_t seed, void *out) {
  *(uint32_t *) out = len + libiberty_hash((unsigned char *)key, len, seed);
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
uint32_t SuperFastHash (const char * data, int len, uint32_t hash);
static inline bool SuperFastHash_bad_seeds(std::vector<uint32_t> &seeds)
{
  seeds = std::vector<uint32_t> { UINT32_C(0) };
  return true;
}
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

// objsize: 465b90-4663c0: 2069
void SpookyV2_32_test     ( const void * key, int len, uint32_t seed, void * out );
void SpookyV2_64_test     ( const void * key, int len, uint32_t seed, void * out );
void SpookyV2_128_test    ( const void * key, int len, uint32_t seed, void * out );

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
// 2^32 bad seeds for Murmur2C
static void MurmurHash64B_seed_init(uint32_t &seed) {
  if ((seed & 0x10) == 0x10)
    seed++;
}
inline void MurmurHash64B_test ( const void * key, int len, uint32_t seed, void * out )
{
  *(uint64_t*)out = MurmurHash64B(key,len,seed);
}
#endif

inline void xmsx32_test ( const void * key, int len, uint32_t seed, void * out )
{
  *(uint32_t*)out = xmsx32(key, (size_t)len, seed);
}

inline void jodyhash32_test( const void * key, int len, uint32_t seed, void * out ) {
  *(uint32_t*)out = jody_block_hash32((const jodyhash32_t *)key, (jodyhash32_t) seed, (size_t) len);
}
#ifdef HAVE_INT64
inline void jodyhash64_test( const void * key, int len, uint32_t seed, void * out ) {
  *(uint64_t*)out = jody_block_hash((jodyhash_t *)key, (jodyhash_t) seed, (size_t) len);
}
#endif

inline void xxHash32_test( const void * key, int len, uint32_t seed, void * out ) {
  // objsize 10-104 + 3e0-5ce: 738
  *(uint32_t*)out = (uint32_t) XXH32(key, (size_t) len, (unsigned) seed);
}
// objsize 4183e0 - 4186c0: 736
inline void gxhash32_test( const void * key, int len, uint32_t seed, void * out ) {
  *(uint32_t*)out = gxhash32((uint8_t*)key, (size_t) len, seed);
}
static inline bool gxhash32_bad_seeds(std::vector<uint32_t> &seeds)
{
  seeds = std::vector<uint32_t>{
    0xe1c1ec7d, 0x0376a937, 0x64ef3cc9, 0x282c25a4,
    0xab1d407e, 0xef794206, 0x90626a1e, 0x9c0731c3,
    0x3c1daeaa, 0xbd359253 };
  return true;
}
void gxhash32_seed_init(uint32_t &seed);

#ifdef HAVE_INT64
inline void xxHash64_test( const void * key, int len, uint32_t seed, void * out ) {
  // objsize 630-7fc + c10-1213: 1999
  *(uint64_t*)out = (uint64_t) XXH64(key, (size_t) len, (unsigned long long) seed);
}
// objsize 418110 - 4183e0: 720
inline void gxhash64_test( const void * key, int len, uint32_t seed, void * out ) {
  *(uint64_t*)out = gxhash64((uint8_t*)key, (size_t) len, seed);
}
#endif

#define restrict // oddly enough, seems to choke on this keyword
#include "xxh3.h"

#ifdef HAVE_INT64
static inline bool xxh3_bad_seeds(std::vector<uint64_t> &seeds) {
  return false;
}
inline void xxh3_test( const void * key, int len, uint32_t seed, void * out ) {
  // objsize 12d0-15b8: 744
  *(uint64_t*)out = (uint64_t) XXH3_64bits_withSeed(key, (size_t) len, seed);
}
#endif

inline void xxh3low_test( const void * key, int len, uint32_t seed, void * out ) {
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
#if defined(HAVE_SSE42) && (defined(__x86_64__) ||  defined(__aarch64__)) && !defined(_MSC_VER)
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
#ifdef HAVE_AHASH_C
// objsize: 4c48a0-4c4a3c: 412
inline void ahash64_test ( const void * key, int len, uint32_t seed, void * out ) {
  *(uint64_t*)out = ahash64(key, (size_t) len, (uint64_t)seed);
}
#endif

#endif

// objsize 0-778: 1912
void mum_hash_test(const void * key, int len, uint32_t seed, void * out);
static inline bool mum_hash_bad_seeds(std::vector<uint64_t> &seeds)
{
  seeds = std::vector<uint64_t> { UINT64_C(0) };
  return true;
}

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

#if defined(HAVE_SSE42) && defined(__x86_64__)
#include "clhash.h"
void clhash_init();
void clhash_seed_init(size_t &seed);
void clhash_test (const void * key, int len, uint32_t seed, void * out);
#endif

#ifndef _MSC_VER
// objsize 454880-4554f3: 2911
void halftime_hash_style64_test(const void *key, int len, uint32_t seed, void *out);
// 455e90 - 45682e: 2462
void halftime_hash_style128_test(const void *key, int len, uint32_t seed, void *out);
// 458840 - 45927e: 2622
void halftime_hash_style256_test(const void *key, int len, uint32_t seed, void *out);
// 457a60 - 45883e: 3550 (without AVX512 on Ryzen3)
void halftime_hash_style512_test(const void *key, int len, uint32_t seed, void *out);
void halftime_hash_init();
void halftime_hash_seed_init(size_t &seed);
#endif

#ifdef __SIZEOF_INT128__
   void multiply_shift_init();
   void multiply_shift_seed_init(uint32_t &seed);
   bool multiply_shift_bad_seeds(std::vector<uint64_t> &seeds);
   void multiply_shift (const void * key, int len, uint32_t seed, void * out);
   void pair_multiply_shift (const void *key, int len, uint32_t seed, void *out);
   void poly_mersenne_init();
   void poly_mersenne_seed_init(uint32_t &seed);
   // insecure: hashes cancel itself out, as with CRC
   // void poly_0_mersenne (const void* key, int len, uint32_t seed, void* out);
   void poly_1_mersenne (const void* key, int len, uint32_t seed, void* out);
   void poly_2_mersenne (const void* key, int len, uint32_t seed, void* out);
   void poly_3_mersenne (const void* key, int len, uint32_t seed, void* out);
   void poly_4_mersenne (const void* key, int len, uint32_t seed, void* out);
#endif

#ifdef __SIZEOF_INT128__
   inline void tabulation_init() {
     size_t seed = 2;
     tabulation_seed_init(seed);
   }
   // insecure: hashes cancel itself out, as with poly_X and CRC
   // objsize: 40b780 - 40b9aa: 554
   inline void tabulation_test (const void * key, int len, uint32_t seed, void * out) {
      *(uint64_t*)out = tabulation_hash(key, len, seed);
   }
#endif

inline void tabulation_32_init() {
  size_t seed = 0;
  tabulation_32_seed_init(seed);
}
// objsize: 40b9b0 - 40bd00: 848
inline void tabulation_32_test (const void * key, int len, uint32_t seed, void * out) {
   *(uint32_t*)out = tabulation_32_hash(key, len, seed);
}

void HighwayHash_init();
// objsize 20-a12: 2546
void HighwayHash64_test (const void * key, int len, uint32_t seed, void * out);

#if defined HAVE_BIT32
// native 32bit. objsize: 8055230 - 80553da: 426
#include "wyhash32.h"
inline void wyhash32_test (const void * key, int len, uint32_t seed, void * out) {
  *(uint32_t*)out = wyhash32(key, (uint64_t)len, (unsigned)seed);
}
#elif defined HAVE_INT64
#include "wyhash.h"
// objsize 40dbe0-40ddba: 474
inline void wyhash_test (const void * key, int len, uint32_t seed, void * out) {
  *(uint64_t*)out = wyhash(key, (uint64_t)len, (uint64_t)seed, _wyp);
}
// objsize: 40da00-40dbda: 474
inline void wyhash32low (const void * key, int len, uint32_t seed, void * out) {
  *(uint32_t*)out = 0xFFFFFFFF & wyhash(key, (uint64_t)len, (uint64_t)seed, _wyp);
}

#include "w1hash.h"
inline void w1hash_test (const void * key, int len, uint32_t seed, void * out) {
  *(uint64_t*)out = w1hash_with_seed(key, len, seed);
}
#endif

#ifdef HAVE_INT64
#include "rapidhash.h"
// objsize: 181e0-1841e: 574
inline void rapidhash_test (const void * key, int len, uint32_t seed, void * out) {
  *(uint64_t*)out = rapidhash_withSeed_compact(key, (uint64_t)len, (uint64_t)seed);
}

// objsize: 18420-1872e: 782
inline void rapidhash_unrolled_test (const void * key, int len, uint32_t seed, void * out) {
  *(uint64_t*)out = rapidhash_withSeed_unrolled(key, (uint64_t)len, (uint64_t)seed);
}
#endif

#ifdef HAVE_INT64
#include "o1hash.h"
// unseeded. objsize: 101
// This is vulnerable to keys len>4 and key[len/2 -2]..[len/2 +2] being 0 (binary keys).
inline void o1hash_test (const void * key, int len, uint32_t seed, void * out) {
  *(uint64_t*)out = o1hash(key, (uint64_t)len);
}

//https://github.com/vnmakarov/mir/blob/master/mir-hash.h
#include "mir-hash.h"
static inline bool mirhash_bad_seeds(std::vector<uint64_t> &seeds)
{
  seeds = std::vector<uint64_t> { 0x0, 0x5e74c778, 0xa521f17b, 0xe0ab70e3 };
  // plus all 64bit variants << 32 !!
  return true;
}
inline void mirhash_test (const void * key, int len, uint32_t seed, void * out) {
  // objsize 2950-2da8: 1112
  *(uint64_t*)out = mir_hash(key, (uint64_t)len, (uint64_t)seed);
}
static inline bool mirhash32_bad_seeds(std::vector<uint32_t> &seeds)
{
  seeds = std::vector<uint32_t> { 0x0, 0x5e74c778, 0xa521f17b, 0xe0ab70e3 };
  return true;
}
inline void mirhash32low (const void * key, int len, uint32_t seed, void * out) {
  *(uint32_t*)out = 0xFFFFFFFF & mir_hash(key, (uint64_t)len, (uint64_t)seed);
}
inline void mirhashstrict_test (const void * key, int len, uint32_t seed, void * out) {
  // objsize 2950-2da8: 1112
  *(uint64_t*)out = mir_hash_strict(key, (uint64_t)len, (uint64_t)seed);
}
static inline bool mirhashstrict32low_bad_seeds(std::vector<uint32_t> &seeds)
{
  seeds = std::vector<uint32_t> { 0x7fcc747f };
  return true;
}
inline void mirhashstrict32low (const void * key, int len, uint32_t seed, void * out) {
  *(uint32_t*)out = 0xFFFFFFFF & mir_hash_strict(key, (uint64_t)len, (uint64_t)seed);
}
static void mirhash_seed_init(uint32_t &seed)
{
  // reject bad seeds
  std::vector<uint64_t> bad_seeds;
  mirhash_bad_seeds(bad_seeds);
  for (uint64_t s : bad_seeds) {
    if (s == seed) {
      seed++; break;
    }
    // also all hi ranges
    if ((s << 32) & seed) {
      seed++; break;
    }
  }
}
static void mirhash32_seed_init(uint32_t &seed)
{
  // reject bad seeds
  std::vector<uint32_t> bad_seeds;
  mirhash32_bad_seeds(bad_seeds);
  while (std::find(bad_seeds.begin(), bad_seeds.end(), (uint32_t)seed) != bad_seeds.end())
    seed++;
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

  // The "B" state was modified last in the hash round, so return the
  // second word of output.
  memcpy(out, hash + 4, 4);
}

inline void md5_64(const void *key, int len, uint32_t seed, void *out) {
  unsigned char hash[16];
  md5_context md5_ctx;
  md5_starts( &md5_ctx );
  md5_ctx.state[0] ^= seed;
  md5_update( &md5_ctx, (unsigned char *)key, len );
  md5_finish( &md5_ctx, hash );
  //memset( &md5_ctx.buffer, 0, 64+64+64 ); // for buffer, ipad, opad

  // The "B" and "C" states were modified last in the hash rounds, so
  // return the second and third word of output.
  memcpy(out, hash + 4, 8);
}

#include "sha1.h"
inline void sha1_160(const void *key, int len, uint32_t seed, void *out) {
  SHA1_CTX context;

  SHA1_Init(&context);
  context.state[0] ^= seed;
  SHA1_Update(&context, (uint8_t*)key, len);
  SHA1_Final(&context, SHA1_DIGEST_SIZE, (uint8_t*)out);
}

inline void sha1_32(const void *key, int len, uint32_t seed, void *out) {
  SHA1_CTX context;

  SHA1_Init(&context);
  context.state[0] ^= seed;
  SHA1_Update(&context, (uint8_t *)key, len);
  SHA1_Final(&context, 4, (uint8_t *)out);
}

inline void sha1_64(const void *key, int len, uint32_t seed, void *out) {
  SHA1_CTX context;

  SHA1_Init(&context);
  context.state[0] ^= seed;
  SHA1_Update(&context, (uint8_t *)key, len);
  SHA1_Final(&context, 8, (uint8_t *)out);
}

#include "tomcrypt.h"

int blake2b_init(hash_state * md, unsigned long outlen,
                 const unsigned char *key, unsigned long keylen);
inline void blake2b160_test(const void *key, int len, uint32_t seed, void *out)
{
  // objsize
  hash_state ltc_state;
  blake2b_init(&ltc_state, 20, NULL, 0);
  ltc_state.blake2b.h[0] ^= seed; // mix seed into lowest int
  blake2b_process(&ltc_state, (unsigned char *)key, len);
  blake2b_done(&ltc_state, (unsigned char *)out);
}
inline void blake2b224_test(const void *key, int len, uint32_t seed, void *out)
{
  // objsize
  hash_state ltc_state;
  blake2b_init(&ltc_state, 28, NULL, 0);
  ltc_state.blake2b.h[0] ^= seed;
  blake2b_process(&ltc_state, (unsigned char *)key, len);
  blake2b_done(&ltc_state, (unsigned char *)out);
}
inline void blake2b256_test(const void *key, int len, uint32_t seed, void *out)
{
  // objsize
  hash_state ltc_state;
  blake2b_init(&ltc_state, 32, NULL, 0);
  ltc_state.blake2b.h[0] ^= seed;
  blake2b_process(&ltc_state, (unsigned char *)key, len);
  blake2b_done(&ltc_state, (unsigned char *)out);
}
inline void blake2b256_64(const void *key, int len, uint32_t seed, void *out)
{
  // objsize
  hash_state ltc_state;
  unsigned char buf[32];
  blake2b_init(&ltc_state, 32, NULL, 0);
  ltc_state.blake2b.h[0] ^= seed;
  blake2b_process(&ltc_state, (unsigned char *)key, len);
  blake2b_done(&ltc_state, buf);
  memcpy(out, buf, 8);
}
int blake2s_init(hash_state * md, unsigned long outlen,
                 const unsigned char *key, unsigned long keylen);
inline void blake2s128_test(const void * key, int len, uint32_t seed, void * out)
{
  // objsize
  hash_state ltc_state;
  blake2s_init(&ltc_state, 16, NULL, 0);
  ltc_state.blake2s.h[0] ^= seed;
  blake2s_process(&ltc_state, (unsigned char *)key, len);
  blake2s_done(&ltc_state, (unsigned char *)out);
}
inline void blake2s160_test(const void * key, int len, uint32_t seed, void * out)
{
  // objsize
  hash_state ltc_state;
  blake2s_init(&ltc_state, 20, NULL, 0);
  ltc_state.blake2s.h[0] ^= seed;
  blake2s_process(&ltc_state, (unsigned char *)key, len);
  blake2s_done(&ltc_state, (unsigned char *)out);
}
inline void blake2s224_test(const void * key, int len, uint32_t seed, void * out)
{
  // objsize
  hash_state ltc_state;
  blake2s_init(&ltc_state, 28, NULL, 0);
  ltc_state.blake2s.h[0] ^= seed;
  blake2s_process(&ltc_state, (unsigned char *)key, len);
  blake2s_done(&ltc_state, (unsigned char *)out);
}
inline void blake2s256_test(const void * key, int len, uint32_t seed, void * out)
{
  // objsize
  hash_state ltc_state;
  blake2s_init(&ltc_state, 32, NULL, 0);
  ltc_state.blake2s.h[0] ^= seed;
  blake2s_process(&ltc_state, (unsigned char *)key, len);
  blake2s_done(&ltc_state, (unsigned char *)out);
}
inline void blake2s256_64(const void * key, int len, uint32_t seed, void * out)
{
  // objsize
  hash_state ltc_state;
  unsigned char buf[32];
  blake2s_init(&ltc_state, 32, NULL, 0);
  ltc_state.blake2s.h[0] ^= seed;
  blake2s_process(&ltc_state, (unsigned char *)key, len);
  blake2s_done(&ltc_state, buf);
  memcpy(out, buf, 8);
}
inline void sha2_224(const void *key, int len, uint32_t seed, void *out)
{
  // objsize
  hash_state ltc_state;
  sha224_init(&ltc_state);
  ltc_state.sha256.state[0] ^= seed;
  ltc_state.sha256.state[0] += len; // hardened against padding
  sha224_process(&ltc_state, (unsigned char *)key, len);
  sha224_done(&ltc_state, (unsigned char *)out);
}
inline void sha2_224_64(const void *key, int len, uint32_t seed, void *out)
{
  // objsize
  unsigned char buf[28];
  hash_state ltc_state;
  sha224_init(&ltc_state);
  ltc_state.sha256.state[0] ^= seed;
  ltc_state.sha256.state[0] += len; // hardened against padding
  sha224_process(&ltc_state, (unsigned char *)key, len);
  sha224_done(&ltc_state, buf);
  memcpy(out, buf, 8);
}
inline void sha2_256(const void *key, int len, uint32_t seed, void *out)
{
  // objsize
  hash_state ltc_state;
  sha256_init(&ltc_state);
  ltc_state.sha256.state[0] ^= seed;
  ltc_state.sha256.state[0] += len; // hardened against padding
  sha256_process(&ltc_state, (unsigned char *)key, len);
  sha256_done(&ltc_state, (unsigned char *)out);
}
inline void sha2_256_64(const void *key, int len, uint32_t seed, void *out)
{
  // objsize
  unsigned char buf[32];
  hash_state ltc_state;
  sha256_init(&ltc_state);
  ltc_state.sha256.state[0] ^= seed;
  ltc_state.sha256.state[0] += len; // hardened against padding
  sha256_process(&ltc_state, (unsigned char *)key, len);
  sha256_done(&ltc_state, buf);
  memcpy(out, buf, 8);
}
inline void rmd128(const void *key, int len, uint32_t seed, void *out)
{
  // objsize
  hash_state ltc_state;
  rmd128_init(&ltc_state);
  ltc_state.rmd128.state[0] ^= seed;
  rmd128_process(&ltc_state, (unsigned char *)key, len);
  rmd128_done(&ltc_state, (unsigned char *)out);
}
inline void rmd160(const void *key, int len, uint32_t seed, void *out)
{
  // objsize
  hash_state ltc_state;
  rmd160_init(&ltc_state);
  ltc_state.rmd160.state[0] = 0x67452301UL ^ seed;
  rmd160_process(&ltc_state, (unsigned char *)key, len);
  rmd160_done(&ltc_state, (unsigned char *)out);
}
inline void rmd256(const void *key, int len, uint32_t seed, void *out)
{
  // objsize
  hash_state ltc_state;
  rmd256_init(&ltc_state);
  ltc_state.rmd256.state[0] ^= seed;
  rmd256_process(&ltc_state, (unsigned char *)key, len);
  rmd256_done(&ltc_state, (unsigned char *)out);
}
#if !defined(__BYTE_ORDER__) || !defined(__ORDER_BIG_ENDIAN__) || !defined(__ORDER_LITTLE_ENDIAN__)
#  error __BYTE_ORDER__ and __ORDER_BIG_ENDIAN__ should be defined.
#elif __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
#  undef HAVE_EDONR // BE build fails due to missing rhash_swap_copy_str_to_u32()
#else
#  define HAVE_EDONR
#endif
#ifdef HAVE_EDONR
#include "edonr.h"
inline void edonr224(const void *key, int len, uint32_t seed, void *out)
{
  // objsize
  struct edonr_ctx ctx;
  rhash_edonr224_init(&ctx);
  ctx.u.data256.hash[0] ^= seed;
  rhash_edonr256_update(&ctx, (unsigned char *)key, len);
  rhash_edonr256_final(&ctx, (unsigned char *)out);
}
inline void edonr256(const void *key, int len, uint32_t seed, void *out)
{
  // objsize
  struct edonr_ctx ctx;
  rhash_edonr256_init(&ctx);
  ctx.u.data256.hash[0] ^= seed;
  rhash_edonr256_update(&ctx, (unsigned char *)key, len);
  rhash_edonr256_final(&ctx, (unsigned char *)out);
}
#endif
// Keccak:
inline void sha3_256_64(const void *key, int len, uint32_t seed, void *out)
{
  // objsize
  unsigned char buf[32];
  hash_state ltc_state;
  sha3_256_init(&ltc_state);
  ltc_state.sha3.s[0] ^= seed;
  sha3_process(&ltc_state, (unsigned char *)key, len);
  sha3_done(&ltc_state, buf);
  memcpy(out, buf, 8);
}
inline void sha3_256(const void *key, int len, uint32_t seed, void *out)
{
  // objsize
  hash_state ltc_state;
  sha3_256_init(&ltc_state);
  ltc_state.sha3.s[0] ^= seed;
  sha3_process(&ltc_state, (unsigned char *)key, len);
  sha3_done(&ltc_state, (unsigned char *)out);
}

#if defined(HAVE_INT64) && !defined(HAVE_BIT32)
inline void wysha(const void *key, int len, unsigned seed, void *out) {
  uint64_t s[4] = {wyhash(key, len, seed + 0, _wyp), wyhash(key, len, seed + 1, _wyp),
                   wyhash(key, len, seed + 2, _wyp), wyhash(key, len, seed + 3, _wyp)};
  memcpy(out, s, 32);
}
#endif

#if defined(HAVE_AESNI) && defined(__SIZEOF_INT128__) && \
  (defined(__x86_64__) || defined(_M_AMD64) || defined(__i386__)  || defined(_M_IX86))
#define HAVE_MEOW_HASH
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
  state[0] ^= seed;
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
  memcpy(out, state, 20);
}
// Note: improved native SHA functions with seed and len encoded into the seed, to prevent Zeroes.
// These functions need to be padded to 64byte blocks, so it would be trivial to create max
// 64 collisions for each key.
inline void sha1ni_32(const void *key, int len, uint32_t seed, void *out)
{
  uint32_t state[5] = {0x67452301, 0xefcdab89, 0x98badcfe, 0x10325476, 0xc3d2e1f0};
  state[0] ^= seed;
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
  uint32_t state[8] = {0x6a09e667, 0xbb67ae85, 0x3c6ef372, 0xa54ff53a,
                       0x510e527f, 0x9b05688c, 0x1f83d9ab, 0x5be0cd19};
  state[0] ^= seed;
  if (len < 64) {
    uint8_t padded[64];
    memcpy (padded, key, len);
    memset (&padded[len], 0, 64-len);
    state[0] += len; // to prevent Zeroes
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
  memcpy(out, state, 32);
}
inline void sha2ni_256_64(const void *key, int len, uint32_t seed, void *out)
{
  uint32_t state[8] = {0x6a09e667, 0xbb67ae85, 0x3c6ef372, 0xa54ff53a,
                       0x510e527f, 0x9b05688c, 0x1f83d9ab, 0x5be0cd19};
  state[0] ^= seed;
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
void farsh32_test ( const void * key, int len, unsigned seed, void * out );
void farsh64_test ( const void * key, int len, unsigned seed, void * out );
void farsh128_test ( const void * key, int len, unsigned seed, void * out );
void farsh256_test ( const void * key, int len, unsigned seed, void * out );
#endif

extern "C" {
#include "blake3/blake3_impl.h"
// The C API, serially
  inline void blake3c_test ( const void * key, int len, uint32_t seed, void * out )
  {
    blake3_hasher hasher;
#if 1
    blake3_hasher_init (&hasher);
    // See GH #168
    hasher.key[0] ^= seed;
    hasher.chunk.cv[0] ^= seed;
#else
    // same speed
    uint32_t seed_key[8] = {0x6A09E667 ^ (uint32_t)seed, 0xBB67AE85, 0x3C6EF372,
      0xA54FF53A, 0x510E527F, 0x9B05688C,
      0x1F83D9AB, 0x5BE0CD19};    // Copied the default IV from blake3_impl.h
    blake3_hasher_init_keyed(&hasher, (uint8_t*)seed_key); // Changed to the KEYED variant
#endif
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

// objsize: 452010-45251e: 1294 (DISCoHAsH)
#include "discohash.h"

#if defined(HAVE_SSE2) && defined(HAVE_AESNI) && !defined(_MSC_VER)
/* https://gist.github.com/majek/96dd615ed6c8aa64f60aac14e3f6ab5a plus seed */
/* objsize: 41f530-41f6cb: 1209 */
uint64_t aesnihash(uint8_t *in, unsigned long src_sz, uint32_t seed);
inline void aesnihash_test ( const void * key, int len, unsigned seed, void * out )
{
  uint64_t result = aesnihash ((uint8_t *)key, (unsigned long)len, (uint32_t)seed);
  *(uint64_t *)out = result;
}

extern void aesni128_test ( const void * key, int len, unsigned seed, void * out );
extern void aesni64_test ( const void * key, int len, unsigned seed, void * out );
#endif

#ifdef HAVE_INT64
// https://github.com/avaneev/prvhash
#include "prvhash/prvhash64.h"
// objsize: 4113ad - 411250: 349
inline void prvhash64_64mtest ( const void * key, int len, unsigned seed, void * out )
{
  *(uint64_t*)out = prvhash64_64m ((const uint8_t *)key, len, (uint64_t)seed);
}
// objsize: 411b40 - 411cc0: 384
inline void prvhash64_64test ( const void * key, int len, unsigned seed, void * out )
{
  prvhash64 ((const uint8_t *)key, len, out, 8, (uint64_t)seed);
}
// objsize: 411870 - 411b3e: 718
inline void prvhash64_128test ( const void * key, int len, unsigned seed, void * out )
{
  prvhash64 ((const uint8_t *)key, len, out, 16, (uint64_t)seed);
}

// the more secure variants
#include "prvhash/prvhash64s.h"
#define PRVHASH64S_PAR 4
// objsize: 411cc0 - 412710: 2640
inline void prvhash64s_64test ( const void * key, int len, unsigned seed, void * out )
{
  // if seedless: prvhash64s_oneshot(key, len, out, 8);
  PRVHASH64S_CTX ctx;
  uint64_t SeedXOR[ PRVHASH64S_PAR ] = { (uint64_t)seed, (uint64_t)seed, (uint64_t)seed, (uint64_t)seed };
  prvhash64s_init( &ctx, 8, SeedXOR );
  prvhash64s_update( &ctx, (const uint8_t*)key, (size_t)len );
  prvhash64s_final( &ctx, out );
}
// objsize: 412710 - 4131ff: 2799
inline void prvhash64s_128test ( const void * key, int len, unsigned seed, void * out )
{
  PRVHASH64S_CTX ctx;
  uint64_t SeedXOR[ PRVHASH64S_PAR ] = { (uint64_t)seed, (uint64_t)seed, (uint64_t)seed, (uint64_t)seed };
  prvhash64s_init( &ctx, 16, SeedXOR );
  prvhash64s_update( &ctx, (const uint8_t*)key, (size_t)len );
  prvhash64s_final( &ctx, out );
}
#endif

#include "komihash/komihash.h"
// objsize: 25830 - 25d5b: 1323
inline void komihash_test ( const void * key, int len, unsigned seed, void * out )
{
#if 0
  *(uint64_t*)out = komihash_stream_oneshot ((const uint8_t *)key, (const size_t)len,
                                             (uint64_t)seed);
#else
  *(uint64_t*)out = komihash ((const uint8_t *)key, (size_t)len, (uint64_t)seed);
#endif
}

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

// requires modern builtins, like __builtin_uaddll_overflow, and 64bit
#if defined(HAVE_SSE42) &&  (defined(__x86_64__) ||  defined(__aarch64__)) && !defined(_MSC_VER)
// objsize: 4bcb90 - 4bd18a
#include "umash.hpp"
#endif

extern "C" {
  // objsize: b200 - c2f5: 4341
  void asconhashv12_64  ( const void * key, int len, uint32_t seed, void * out );
  // objsize: c300 - dc5a: 6490
  void asconhashv12_256 ( const void * key, int len, uint32_t seed, void * out );
}

extern const char * const nmhash32_desc;
extern const char * const nmhash32x_desc;
bool nmhash32_broken ( void );
void nmhash32_test ( const void * key, int len, uint32_t seed, void * out );
void nmhash32x_test ( const void * key, int len, uint32_t seed, void * out );

#ifdef HAVE_INT64
extern "C" {
#include "pearson_hash/pearsonb.h"
// objsize: 417b50-417dfb = 683
inline void pearsonb64_test ( const void * key, int len, uint32_t seed, void * out ) {
  // pearsonb_hash_64(): caller is responsible for storing it the big endian way to memory
  *(uint64_t*)out = htobe64(pearsonb_hash_64 ((const uint8_t*)key, (size_t) len, (uint64_t) seed));
}
// objsize: 41a1f0-41a65e: 1134
inline void pearsonb128_test ( const void * key, int len, uint32_t seed, void * out ) {
  pearsonb_hash_128 ((uint8_t*)out, (const uint8_t*)key, (size_t) len, (uint64_t) seed);
}
// 41a660-41a9ac: 844
inline void pearsonb256_test ( const void * key, int len, uint32_t seed, void * out ) {
  pearsonb_hash_256 ((uint8_t*)out, (const uint8_t*)key, (size_t) len, (uint64_t) seed);
}

#if defined(HAVE_SSE42) && defined(__x86_64__)
#include "pearson_hash/pearson.h"
inline void pearson64_test ( const void * key, int len, uint32_t seed, void * out ) {
  *(uint64_t*)out = pearson_hash_64 ((const uint8_t*)key, (size_t) len, seed);
}
inline void pearson128_test ( const void * key, int len, uint32_t seed, void * out ) {
  pearson_hash_128 ((uint8_t*)out, (const uint8_t*)key, (size_t) len);
}
inline void pearson256_test ( const void * key, int len, uint32_t seed, void * out ) {
  pearson_hash_256 ((uint8_t*)out, (const uint8_t*)key, (size_t) len);
}
#endif

// not implemented
#ifndef HAVE_ALIGNED_ACCESS_REQUIRED
  
#undef ROTR32
#undef ROTR64
#include "khash.h"
//objsize: 418eb0-4191d8: 808
inline void khash32_test ( const void *key, int len, uint32_t seed, void *out) {
  uint32_t hash = ~seed;
  uint32_t *dw = (uint32_t*)key;
  const uint32_t *const endw = &((const uint32_t*)key)[len/4];
  while (dw < endw) {
    hash ^= khash32_fn (*dw++, seed, UINT32_C(0xf3bcc908));
  }
  if (len & 3) {
    // the unsafe variant with overflow. see FNV2 for a safe byte-stepper.
    hash ^= khash32_fn (*dw, seed, UINT32_C(0xf3bcc908));
  }
  *(uint32_t*)out = hash;
}
//objsize: 4191e0-419441: 609
inline void khash64_test ( const void *key, int len, uint32_t seed, void *out) {
  uint64_t* dw = (uint64_t*)key;
  const uint64_t *const endw = &((const uint64_t*)key)[len/8];
  const uint64_t seed64 = (uint64_t)seed | UINT64_C(0x6a09e66700000000);
  uint64_t hash = ~seed64;
  while (dw < endw) {
    hash ^= khash64_fn (*dw++, seed64);
  }
  if (len & 7) {
    // unsafe variant with overflow
    hash ^= khash32_fn (*dw, seed, UINT32_C(0xf3bcc908));
  }
  *(uint64_t*)out = hash;
}
#endif // HAVE_ALIGNED_ACCESS_REQUIRED

} // extern C
#endif

#ifndef HAVE_BIT32
void khashv_seed_init(size_t &seed);
// call to khashv_hash_vector not inlined.
extern const char * const khashv32_desc;
extern const char * const khashv64_desc;
void khashv32_test ( const void *key, int len, uint32_t seed, void *out);
void khashv64_test ( const void *key, int len, uint32_t seed, void *out);
#endif

void polymur_seed_init (size_t seed);
void polymur_test ( const void *key, int len, uint32_t seed, void *out);

/* This version of CRC64 can approach HW crc speeds without hardware support,
   and can be applied to any polynomial. */
#include "crc64.h"
#include "crcspeed.h"
#include "crccombine.h"

extern "C" void crc64_jones_test1(const void *input, int len, uint32_t seed, void *out);
extern "C" void crc64_jones_test2(const void *input, int len, uint32_t seed, void *out);
extern "C" void crc64_jones_test3(const void *input, int len, uint32_t seed, void *out);
extern "C" void crc64_jones_default(const void *input, int len, uint32_t seed, void *out);

#if defined(HAVE_SSE2) && defined(HAVE_AESNI)
void aesnihash_peterrk(const void * in, int len0, uint32_t seed, void * out);
#endif
