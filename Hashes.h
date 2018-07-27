#pragma once

#include "Types.h"

#include "MurmurHash1.h"
#include "MurmurHash2.h"
#include "MurmurHash3.h"

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
void FNV32a                ( const void * key, int len, uint32_t seed, void * out );
void FNV32a_YoshimitsuTRIAD( const void * key, int len, uint32_t seed, void * out );
void FNV64a                ( const void * key, int len, uint32_t seed, void * out );
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
void CityHash64_test       ( const void * key, int len, uint32_t seed, void * out );
void CityHash128_test      ( const void * key, int len, uint32_t seed, void * out );
void FarmHash32_test       ( const void * key, int len, uint32_t seed, void * out );
void FarmHash64_test       ( const void * key, int len, uint32_t seed, void * out );
void FarmHash128_test      ( const void * key, int len, uint32_t seed, void * out );
void farmhash32_c_test       ( const void * key, int len, uint32_t seed, void * out );
void farmhash64_c_test       ( const void * key, int len, uint32_t seed, void * out );
void farmhash128_c_test      ( const void * key, int len, uint32_t seed, void * out );

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

inline void MurmurHash64A_test ( const void * key, int len, uint32_t seed, void * out )
{
  *(uint64_t*)out = MurmurHash64A(key,len,seed);
}

inline void MurmurHash64B_test ( const void * key, int len, uint32_t seed, void * out )
{
  *(uint64_t*)out = MurmurHash64B(key,len,seed);
}

inline void jodyhash32_test( const void * key, int len, uint32_t seed, void * out ) {
  *(uint32_t*)out = jody_block_hash32((const jodyhash32_t *)key, (jodyhash32_t) seed, (size_t) len);
}
inline void jodyhash64_test( const void * key, int len, uint32_t seed, void * out ) {
  *(uint64_t*)out = jody_block_hash((const jodyhash_t *)key, (jodyhash_t) seed, (size_t) len);
}

inline void xxHash32_test( const void * key, int len, uint32_t seed, void * out ) {
  *(uint32_t*)out = (uint32_t) XXH32(key, (size_t) len, (unsigned) seed);
}
inline void xxHash64_test( const void * key, int len, uint32_t seed, void * out ) {
  *(uint64_t*)out = (uint64_t) XXH64(key, (size_t) len, (unsigned long long) seed);
}

inline void metrohash64_1_test ( const void * key, int len, uint32_t seed, void * out ) {
  metrohash64_1((const uint8_t *)key,(uint64_t)len,seed,(uint8_t *)out);
}
inline void metrohash64_2_test ( const void * key, int len, uint32_t seed, void * out ) {
  metrohash64_2((const uint8_t *)key,(uint64_t)len,seed,(uint8_t *)out);
}
inline void metrohash128_1_test ( const void * key, int len, uint32_t seed, void * out ) {
  metrohash128_1((const uint8_t *)key,(uint64_t)len,seed,(uint8_t *)out);
}
inline void metrohash128_2_test ( const void * key, int len, uint32_t seed, void * out ) {
  metrohash128_2((const uint8_t *)key,(uint64_t)len,seed,(uint8_t *)out);
}
#if defined(__SSE4_2__) && defined(__x86_64__)
inline void metrohash64crc_1_test ( const void * key, int len, uint32_t seed, void * out ) {
  metrohash64crc_1((const uint8_t *)key,(uint64_t)len,seed,(uint8_t *)out);
}
inline void metrohash64crc_2_test ( const void * key, int len, uint32_t seed, void * out ) {
  metrohash64crc_2((const uint8_t *)key,(uint64_t)len,seed,(uint8_t *)out);
}
inline void metrohash128crc_1_test ( const void * key, int len, uint32_t seed, void * out ) {
  metrohash128crc_1((const uint8_t *)key,(uint64_t)len,seed,(uint8_t *)out);
}
inline void metrohash128crc_2_test ( const void * key, int len, uint32_t seed, void * out ) {
  metrohash128crc_2((const uint8_t *)key,(uint64_t)len,seed,(uint8_t *)out);
}
#endif
inline void cmetrohash64_1_test ( const void * key, int len, uint32_t seed, void * out ) {
  cmetrohash64_1((const uint8_t *)key,(uint64_t)len,seed,(uint8_t *)out);
}
inline void cmetrohash64_1_optshort_test ( const void * key, int len, uint32_t seed, void * out ) {
  cmetrohash64_1_optshort((const uint8_t *)key,(uint64_t)len,seed,(uint8_t *)out);
}
inline void cmetrohash64_2_test ( const void * key, int len, uint32_t seed, void * out ) {
  cmetrohash64_2((const uint8_t *)key,(uint64_t)len,seed,(uint8_t *)out);
}
inline void fasthash32_test ( const void * key, int len, uint32_t seed, void * out ) {
  *(uint32_t*)out = fasthash32(key, (size_t) len, seed);
}
inline void fasthash64_test ( const void * key, int len, uint32_t seed, void * out ) {
  *(uint64_t*)out = fasthash64(key, (size_t) len, (uint64_t)seed);
}

void mum_hash_test(const void * key, int len, uint32_t seed, void * out);

//-----------------------------------------------------------------------------

#include "t1ha.h"

inline void t1ha2_atonce_test(const void * key, int len, uint32_t seed, void * out)
{
  *(uint64_t*)out = t1ha2_atonce(key, len, seed);
}

inline void t1ha2_stream_test(const void * key, int len, uint32_t seed, void * out)
{
  t1ha_context_t ctx;
  t1ha2_init(&ctx, seed, 0);
  t1ha2_update(&ctx, key, len);
  *(uint64_t*)out = t1ha2_final(&ctx, NULL);
}

inline void t1ha2_atonce128_test(const void * key, int len, uint32_t seed, void * out)
{
  *(uint64_t*)out = t1ha2_atonce128((uint64_t*)out + 1, key, len, seed);
}

inline void t1ha2_stream128_test(const void * key, int len, uint32_t seed, void * out)
{
  t1ha_context_t ctx;
  t1ha2_init(&ctx, seed, 0);
  t1ha2_update(&ctx, key, len);
  *(uint64_t*)out = t1ha2_final(&ctx, (uint64_t*)out + 1);
}

inline void t1ha1_64le_test(const void * key, int len, uint32_t seed, void * out)
{
  *(uint64_t*)out = t1ha1_le(key, len, seed);
}

inline void t1ha1_64be_test(const void * key, int len, uint32_t seed, void * out)
{
  *(uint64_t*)out = t1ha1_be(key, len, seed);
}

inline void t1ha0_32le_test(const void * key, int len, uint32_t seed, void * out)
{
  *(uint64_t*)out = t1ha0_32le(key, len, seed);
}

inline void t1ha0_32be_test(const void * key, int len, uint32_t seed, void * out)
{
  *(uint64_t*)out = t1ha0_32be(key, len, seed);
}

#ifdef T1HA0_AESNI_AVAILABLE
inline void t1ha0_ia32aes_noavx_test(const void * key, int len, uint32_t seed, void * out)
{
  *(uint64_t*)out = t1ha0_ia32aes_noavx(key, len, seed);
}

#if defined(__AVX__)
inline void t1ha0_ia32aes_avx1_test(const void * key, int len, uint32_t seed, void * out)
{
  *(uint64_t*)out = t1ha0_ia32aes_avx(key, len, seed);
}
#endif /* __AVX__ */

#if defined(__AVX2__)
inline void t1ha0_ia32aes_avx2_test(const void * key, int len, uint32_t seed, void * out)
{
  *(uint64_t*)out = t1ha0_ia32aes_avx2(key, len, seed);
}
#endif /* __AVX2__ */
#endif /* T1HA0_AESNI_AVAILABLE */
