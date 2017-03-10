#pragma once

#include "Types.h"

#include "MurmurHash1.h"
#include "MurmurHash2.h"
#include "MurmurHash3.h"

#if defined(__x86_64__)
#include "xxhash.h"
#include "metrohash.h"
#include "cmetrohash.h"
#include "opt_cmetrohash.h"
#endif

#include "fasthash.h"

//----------
// These are _not_ hash functions (even though people tend to use crc32 as one...)

void BadHash_with_state    ( const void * key, int len, const void *state, void * out );

void DoNothingHash_with_state( const void * key, int len, const void *state, void * out );
void DoNothingHash_seed_state( int seedbits, const void *seed, void *state);
void NoopOAATReadHash_with_state( const void * key, int len, const void *state, void * out );
void crc32_with_state_test ( const void * key, int len, const void *state, void * out );

//----------
// Cryptographic hashes

void md5_seed_state        ( int seedbits, const void *seed, void *state);
void md5_32_with_state     ( const void * key, int len, const void *state, void * out );
void md5_with_state        ( const void * key, int len, const void *state, void * out );
void sha1_32a_with_state   ( const void * key, int len, const void *state, void * out );
void sha1_64a_with_state   ( const void * key, int len, const void *state, void * out );
void sha1_32b_with_state   ( const void * key, int len, const void *state, void * out );

//----------
// General purpose hashes

#ifdef __SSE2__
void hasshe2_test          ( const void * key, int len, const void *state, void * out );
#endif
#if defined(__SSE4_2__) && defined(__x86_64__)
void crc32c_hw_with_state_test( const void * key, int len, const void *state, void * out );
void crc32c_hw1_with_state_test( const void * key, int len, const void *state, void * out );
void crc64c_hw_with_state_test( const void * key, int len, const void *state, void * out );
void falkhash_with_state_test_cxx(const void *key, int len, const void *seed, void *out);
#endif
void FNV32a_with_state_test( const void * key, int len, const void *state, void * out );
void FNV32a_YoshimitsuTRIAD_with_state_test( const void * key, int len, const void *state, void * out );
void FNV64a_with_state_test( const void * key, int len, const void *state, void * out );
void Bernstein_with_state  ( const void * key, int len, const void *state, void * out );
void sdbm                  ( const void * key, int len, const void *state, void * out );
void x17_test              ( const void * key, int len, const void *seed, void * out );
void JenkinsOAATH_with_state( const void * key, int len, const void *seed, void * out );
void JenkinsOAAT_with_state( const void * key, int len, const void *seed, void * out );
void GoodOAAT              ( const void * key, int len, const void *state, void * out );
void MicroOAAT             ( const void * key, int len, const void *state, void * out );
void SuperFastHash         ( const void * key, int len, const void *state, void * out );
void lookup3_with_state_test( const void * key, int len, const void* state, void * out );
void MurmurOAAT_with_state_test       ( const void * key, int len, const void *state, void * out );
void Crap8_test            ( const void * key, int len, const void *state, void * out );

void CityHash32_with_state_test  ( const void * key, int len, const void * seed, void * out );
void CityHash64_with_state_test  ( const void * key, int len, const void * seed, void * out );
void CityHash128_with_state_test  ( const void * key, int len, const void * seed, void * out );
void CityHashCrc128_with_state_test  ( const void * key, int len, const void * seed, void * out );
void FarmHash32_with_state_test       ( const void * key, int len, const void *state, void * out );
void FarmHash64_with_state_test       ( const void * key, int len, const void *state, void * out );
void FarmHash128_with_state_test      ( const void * key, int len, const void *state, void * out );

void farmhash32_c_with_state_test       ( const void * key, int len, const void *state, void * out );
void farmhash64_c_with_state_test       ( const void * key, int len, const void *state, void * out );
void farmhash128_c_with_state_test      ( const void * key, int len, const void *state, void * out );

void marvin_32_with_state_test ( const void * key, int len, const void * state, void * out );
void lua_v53_string_hash_oaat (const void *str, int len, const void *seed, void *out);
void lua_v53_string_hash (const void *str, int len, const void *seed, void *out);


void SpookyHash32_with_state_test ( const void * key, int len, const void * state, void * out );
void SpookyHash64_with_state_test ( const void * key, int len, const void * state, void * out );
void SpookyHash128_with_state_test ( const void * key, int len, const void * state, void * out );
void SpookyHash_seed_state_test( int seedbits, const void *seed, void *state);


//----------
void siphash_with_state_test( const void * key, int len, const void * seed, void * out );
void siphash13_with_state_test( const void * key, int len, const void * seed, void * out );
void siphash_seed_state_test( int seedbits, const void *seed, void *state);
void halfsiphash_with_state_test( const void * key, int len, const void * seed, void * out );
void halfsiphash_seed_state_test( int seedbits, const void *seed, void *state);
//-----------------------------------------------------------------------------
// Test harnesses for Murmur1/2

inline void MurmurHash1_with_state_test ( const void * key, int len, const void *state, void * out )
{
  *(uint32_t*)out = MurmurHash1(key,len,*((uint32_t *)state));
}

inline void MurmurHash2_with_state_test ( const void * key, int len, const void *state, void * out )
{
  *(uint32_t*)out = MurmurHash2(key,len,*((uint32_t *)state));
}

inline void MurmurHash2A_with_state_test ( const void * key, int len, const void *state, void * out )
{
  *(uint32_t*)out = MurmurHash2A(key,len,*((uint32_t *)state));
}

inline void MurmurHash64A_with_state_test ( const void * key, int len, const void *state, void * out )
{
  *(uint64_t*)out = MurmurHash64A(key,len,*((uint64_t *)state));
}

inline void MurmurHash64B_with_state_test ( const void * key, int len, const void *state, void * out )
{
  *(uint64_t*)out = MurmurHash64B(key,len,*((uint64_t *)state));
}

#if defined(__x86_64__)
inline void xxHash32_with_state_test( const void * key, int len, const void *state, void * out ) {
  *(uint32_t*)out = (uint32_t) XXH32(key, (size_t) len, *((unsigned *) state));
}

inline void xxHash64_with_state_test( const void * key, int len, const void *state, void * out ) {
  *(uint64_t*)out = (uint64_t) XXH64(key, (size_t) len, *((unsigned long long *)state));
}

inline void metrohash64_1_with_state_test ( const void * key, int len, const void *state, void * out ) {
  metrohash64_1((const uint8_t *)key,(uint64_t)len,*((uint64_t*)state),(uint8_t *)out);
}
inline void metrohash64_2_with_state_test ( const void * key, int len, const void *state, void * out ) {
  metrohash64_2((const uint8_t *)key,(uint64_t)len,*((uint64_t*)state),(uint8_t *)out);
}
inline void metrohash128_1_with_state_test ( const void * key, int len, const void *state, void * out ) {
  metrohash128_1((const uint8_t *)key,(uint64_t)len,*((uint64_t*)state),(uint8_t *)out);
}
inline void metrohash128_2_with_state_test ( const void * key, int len, const void *state, void * out ) {
  metrohash128_2((const uint8_t *)key,(uint64_t)len,*((uint64_t*)state),(uint8_t *)out);
}
#if defined(__SSE4_2__) && defined(__x86_64__)

inline void metrohash64crc_1_with_state_test ( const void * key, int len, const void *state, void * out ) {
  metrohash64crc_1((const uint8_t *)key,(uint64_t)len,*((uint64_t*)state),(uint8_t *)out);
}
inline void metrohash64crc_2_with_state_test ( const void * key, int len, const void *state, void * out ) {
  metrohash64crc_2((const uint8_t *)key,(uint64_t)len,*((uint64_t*)state),(uint8_t *)out);
}
inline void metrohash128crc_1_with_state_test ( const void * key, int len, const void *state, void * out ) {
  metrohash128crc_1((const uint8_t *)key,(uint64_t)len,*((uint64_t*)state),(uint8_t *)out);
}
inline void metrohash128crc_2_with_state_test ( const void * key, int len, const void *state, void * out ) {
  metrohash128crc_2((const uint8_t *)key,(uint64_t)len,*((uint64_t*)state),(uint8_t *)out);
}
#endif
inline void cmetrohash64_1_with_state_test ( const void * key, int len, const void *state, void * out ) {
  cmetrohash64_1((const uint8_t *)key,(uint64_t)len,*((uint64_t *)state),(uint8_t *)out);
}
inline void cmetrohash64_1_optshort_with_state_test ( const void * key, int len, const void *state, void * out ) {
  cmetrohash64_1_optshort((const uint8_t *)key,(uint64_t)len,*((uint64_t *)state),(uint8_t *)out);
}
inline void cmetrohash64_2_with_state_test ( const void * key, int len, const void *state, void * out ) {
  cmetrohash64_2((const uint8_t *)key,(uint64_t)len,*((uint64_t *)state),(uint8_t *)out);
}

inline void fasthash32_test ( const void * key, int len, const void *state, void * out ) {
  *(uint32_t*)out = fasthash32(key, (size_t) len, *((uint32_t*)state));
}
inline void fasthash64_test ( const void * key, int len, const void *state, void * out ) {
  *(uint64_t*)out = fasthash64(key, (size_t) len, *((uint64_t*)state));
}
#endif

//-----------------------------------------------------------------------------

#include "t1ha.h"

inline void t1ha_with_state_test(const void * key, int len, const void *seed, void * out)
{
  *(uint64_t*)out = t1ha(key, len, *((uint64_t*)seed));
}

void mum_hash_with_state_test(const void * key, int len, const void *state, void * out);

#if (defined(__SSE4_2__) && defined(__x86_64__)) || defined(_M_X64)
inline void t1ha_crc_with_state_test(const void * key, int len, const void *seed, void * out)
{
  *(uint64_t*)out = t1ha_ia32crc(key, len, *((uint64_t*)seed));
}
#endif

inline void t1ha_64be_with_state_test(const void * key, int len, const void* seed, void * out)
{
  *(uint64_t*)out = t1ha_64be(key, len, *((uint64_t*)seed));
}

inline void t1ha_32le_with_state_test(const void * key, int len, const void *state, void * out)
{
  *(uint64_t*)out = t1ha_32le(key, len, *((uint64_t*)state));
}

inline void t1ha_32be_with_state_test(const void * key, int len, const void *state, void * out)
{
  *(uint64_t*)out = t1ha_32be(key, len, *((uint64_t*)state));
}

#if defined(__AES__) || defined(_M_X64) || defined(_M_IX86)
inline void t1ha_aes_with_state_test(const void * key, int len, const void *state, void * out)
{
  *(uint64_t*)out = t1ha_ia32aes(key, len, *((uint64_t*)state));
}
#endif
