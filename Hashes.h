#pragma once

#include "Platform.h"
#include "Types.h"

#include "MurmurHash1.h"
#include "MurmurHash2.h"
#include "MurmurHash3.h"
#include "PMurHash.h"

#define XXH_INLINE_ALL
#include "xxhash.h"

#include "metrohash.h"
#include "cmetrohash.h"
#include "opt_cmetrohash.h"

#include "fasthash.h"
#include "jody_hash32.h"
#include "jody_hash64.h"

//----------
// These are _not_ hash functions (even though people tend to use crc32 as one...)

void BadHash               ( const void * key, int len, uint32_t seed, void * out );
void sumhash               ( const void * key, int len, uint32_t seed, void * out );
void sumhash32             ( const void * key, int len, uint32_t seed, void * out );

void DoNothingHash         ( const void * key, int len, uint32_t seed, void * out );
void NoopOAATReadHash	   ( const void * key, int len, uint32_t seed, void * out );
void crc32                 ( const void * key, int len, uint32_t seed, void * out );

void randhash_32           ( const void * key, int len, uint32_t seed, void * out );
void randhash_64           ( const void * key, int len, uint32_t seed, void * out );
void randhash_128          ( const void * key, int len, uint32_t seed, void * out );

//----------
// Cryptographic hashes

void md5_32                ( const void * key, int len, uint32_t seed, void * out );
void sha1_32a              ( const void * key, int len, uint32_t seed, void * out );
#if 0
void sha1_64a              ( const void * key, int len, uint32_t seed, void * out );
void sha2_32a              ( const void * key, int len, uint32_t seed, void * out );
void sha2_64a              ( const void * key, int len, uint32_t seed, void * out );
void BLAKE2_32a            ( const void * key, int len, uint32_t seed, void * out );
void BLAKE2_64a            ( const void * key, int len, uint32_t seed, void * out );
void bcrypt_64a            ( const void * key, int len, uint32_t seed, void * out );
void scrypt_64a            ( const void * key, int len, uint32_t seed, void * out );
#endif

//----------
// General purpose hashes

#ifdef __SSE2__
void hasshe2_test          ( const void * key, int len, uint32_t seed, void * out );
#endif
#if defined(__SSE4_2__) && defined(__x86_64__)
void crc32c_hw_test        ( const void * key, int len, uint32_t seed, void * out );
void crc32c_hw1_test       ( const void * key, int len, uint32_t seed, void * out );
void crc64c_hw_test        ( const void * key, int len, uint32_t seed, void * out );
void CityHashCrc64_test    ( const void * key, int len, uint32_t seed, void * out );
void CityHashCrc128_test   ( const void * key, int len, uint32_t seed, void * out );
void falkhash_test_cxx     ( const void * key, int len, uint32_t seed, void * out );
#endif
void fibonacci             ( const void * key, int len, uint32_t seed, void * out );
void FNV32a                ( const void * key, int len, uint32_t seed, void * out );
void FNV32a_YoshimitsuTRIAD( const void * key, int len, uint32_t seed, void * out );
#ifdef HAVE_INT64
void FNV1A_Totenschiff     ( const void * key, int len, uint32_t seed, void * out );
#endif
#if 0 /* TODO */
void FNV1A_Pippip_Yurii    ( const void * key, int len, uint32_t seed, void * out );
void FNV1A_Jesteress       ( const void * key, int len, uint32_t seed, void * out );
void FNV1A_Meiyan          ( const void * key, int len, uint32_t seed, void * out );
#endif
void FNV64a                ( const void * key, int len, uint32_t seed, void * out );
void FNV2                  ( const void * key, int len, uint32_t seed, void * out );
void fletcher2             ( const void * key, int len, uint32_t seed, void * out );
void fletcher4             ( const void * key, int len, uint32_t seed, void * out );
void Bernstein             ( const void * key, int len, uint32_t seed, void * out );
void sdbm                  ( const void * key, int len, uint32_t seed, void * out );
void x17_test              ( const void * key, int len, uint32_t seed, void * out );
void JenkinsOOAT           ( const void * key, int len, uint32_t seed, void * out );
void JenkinsOOAT_perl      ( const void * key, int len, uint32_t seed, void * out );
void GoodOAAT              ( const void * key, int len, uint32_t seed, void * out );
void MicroOAAT             ( const void * key, int len, uint32_t seed, void * out );
void SuperFastHash         ( const void * key, int len, uint32_t seed, void * out );
void lookup3_test          ( const void * key, int len, uint32_t seed, void * out );
void MurmurOAAT_test       ( const void * key, int len, uint32_t seed, void * out );
void Crap8_test            ( const void * key, int len, uint32_t seed, void * out );

void CityHash32_test       ( const void * key, int len, uint32_t seed, void * out );
void CityHash64noSeed_test ( const void * key, int len, uint32_t seed, void * out );
void CityHash64_test       ( const void * key, int len, uint32_t seed, void * out );
inline void CityHash64_low_test ( const void * key, int len, uint32_t seed, void * out ) {
  uint64_t result;
  CityHash64_test(key, len, seed, &result);
  *(uint32_t*)out = (uint32_t)result;
}
void CityHash128_test      ( const void * key, int len, uint32_t seed, void * out );
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

uint32_t MurmurOAAT ( const void * key, int len, uint32_t seed );

//----------
// MurmurHash2
void MurmurHash2_test      ( const void * key, int len, uint32_t seed, void * out );
void MurmurHash2A_test     ( const void * key, int len, uint32_t seed, void * out );

void siphash_test          ( const void * key, int len, uint32_t seed, void * out );
void siphash13_test        ( const void * key, int len, uint32_t seed, void * out );
void halfsiphash_test      ( const void * key, int len, uint32_t seed, void * out );

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
  *(uint32_t*)out = (uint32_t) XXH3_64bits(key, (size_t) len);
}

#ifdef HAVE_INT64
inline void xxh128_test( const void * key, int len, uint32_t seed, void * out ) {
  // objsize 1f60-2354: 1012
  *(XXH128_hash_t*)out = XXH128(key, (size_t) len, seed);
}

inline void xxh128low_test( const void * key, int len, uint32_t seed, void * out ) {
  *(uint64_t*)out = (uint64_t) (XXH128(key, (size_t) len, seed).low64);
}


inline void metrohash64_1_test ( const void * key, int len, uint32_t seed, void * out ) {
  // objsize 0-270: 624
  metrohash64_1((const uint8_t *)key,(uint64_t)len,seed,(uint8_t *)out);
}
inline void metrohash64_2_test ( const void * key, int len, uint32_t seed, void * out ) {
  // objsize 270-4e3: 627
  metrohash64_2((const uint8_t *)key,(uint64_t)len,seed,(uint8_t *)out);
}
inline void metrohash128_1_test ( const void * key, int len, uint32_t seed, void * out ) {
  // objsize 0-305: 773
  metrohash128_1((const uint8_t *)key,(uint64_t)len,seed,(uint8_t *)out);
}
inline void metrohash128_2_test ( const void * key, int len, uint32_t seed, void * out ) {
  // objsize 310-615: 773
  metrohash128_2((const uint8_t *)key,(uint64_t)len,seed,(uint8_t *)out);
}
#if defined(__SSE4_2__) && defined(__x86_64__)
inline void metrohash64crc_1_test ( const void * key, int len, uint32_t seed, void * out ) {
  // objsize 0-278: 632
  metrohash64crc_1((const uint8_t *)key,(uint64_t)len,seed,(uint8_t *)out);
}
inline void metrohash64crc_2_test ( const void * key, int len, uint32_t seed, void * out ) {
  // objsize 280-4f8: 632
  metrohash64crc_2((const uint8_t *)key,(uint64_t)len,seed,(uint8_t *)out);
}
inline void metrohash128crc_1_test ( const void * key, int len, uint32_t seed, void * out ) {
  // objsize 0-2d3: 723
  metrohash128crc_1((const uint8_t *)key,(uint64_t)len,seed,(uint8_t *)out);
}
inline void metrohash128crc_2_test ( const void * key, int len, uint32_t seed, void * out ) {
  // objsize 2e0-5b3: 723
  metrohash128crc_2((const uint8_t *)key,(uint64_t)len,seed,(uint8_t *)out);
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
inline void t1ha0_ia32aes_noavx_test(const void * key, int len, uint32_t seed, void * out)
{
  // objsize 0-39d: 925
  *(uint64_t*)out = t1ha0_ia32aes_noavx(key, len, seed);
}

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

//https://github.com/wangyi-fudan/wyhash
#include "wyhash.h"

// objsize 20-a12: 2546
void wyhash_test (const void * key, int len, uint32_t seed, void * out);
void wyhash32low (const void * key, int len, uint32_t seed, void * out);

//https://github.com/vnmakarov/mir/blob/master/mir-hash.h
#include "mir-hash.h"

void mirhash_test (const void * key, int len, uint32_t seed, void * out);
void mirhash32low (const void * key, int len, uint32_t seed, void * out);
void mirhashstrict_test (const void * key, int len, uint32_t seed, void * out);
void mirhashstrict32low (const void * key, int len, uint32_t seed, void * out);

#if defined(__SSE4_2__) && defined(__x86_64__)
#include "clhash.h"
void clhash_init();
void clhash_test (const void * key, int len, uint32_t seed, void * out);
#endif

void multiply_shift (const void * key, int len, uint32_t seed, void * out);
void pair_multiply_shift (const void *key, int len, uint32_t seed, void *out);

void HighwayHash_init();
  // objsize 20-a12: 2546
void HighwayHash64_test (const void * key, int len, uint32_t seed, void * out);
