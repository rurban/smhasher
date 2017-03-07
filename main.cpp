#include "Platform.h"
#include "Types.h"
#include "HashFunc.h"
#include "Blob.h"
#include "Random.h"
#include "RunTests.h"
#include "Hashes.h"
#include "SimpleStats.h"
#include "PMurHash.h"
#include "beagle_hash.h"
#include "phat_hash.h"
#include "zaphod32_hash.h"
#include "zaphod64_hash.h"
#include "sbox_hash.h"
#include "Marvin32.h"
#include "md5.h"
#include "siphash.h"
#include <stdio.h>
#include <time.h>

//-----------------------------------------------------------------------------
// Configuration. This are defaults.

bool g_testAll         = true;
bool g_testReallyAll   = false;


bool g_testSanity      = false;
bool g_testSpeed       = false;
bool g_testBulkSpeed   = false;
bool g_testKeySpeed    = false;
bool g_testDiff        = false;
bool g_testDiffDist    = false; /* only ReallyAll */
bool g_testAvalanche   = false;
bool g_testBIC         = false; /* only ReallyAll */
bool g_testCyclic      = false;
bool g_testTwoBytes    = false;
bool g_testSparse      = false;
bool g_testPermutation = false;
bool g_testWindow      = false; /* only ReallyAll */
bool g_testText        = false;
bool g_testZeroes      = false;
bool g_testEffs        = false;
bool g_testSeed        = false;

bool g_runCtrStream    = false; /* this is special */
uint64_t g_rngSeed = 1234567809;
uint64_t g_streamKeyLen = 8;

struct TestOpts {
  bool         &var;
  const char*  name;
};
TestOpts g_testopts[] =
{
  { g_testAll, 		"All" },
  { g_testReallyAll,	"ReallyAll" },
  { g_testSanity, 	"Sanity" },
  { g_testSpeed, 	"Speed" },
  { g_testBulkSpeed, 	"BulkSpeed" },
  { g_testKeySpeed, 	"KeySpeed" },
  { g_testDiff, 	"Diff" },
  { g_testDiffDist, 	"DiffDist" },
  { g_testAvalanche, 	"Avalanche" },
  { g_testBIC, 		"BIC" },
  { g_testCyclic,	"Cyclic" },
  { g_testTwoBytes,	"TwoBytes" },
  { g_testSparse,	"Sparse" },
  { g_testPermutation,	"Permutation" },
  { g_testWindow,	"Window" },
  { g_testText,		"Text" },
  { g_testZeroes,	"Zeroes" },
  { g_testEffs,	        "Effs" },
  { g_testSeed,		"Seed" }
};

//-----------------------------------------------------------------------------
// This is the list of all hashes that SMHasher can test.
#define bitsizeof(x) (sizeof(x) * 8)

HashInfo g_hashes[] =
{
  // -- No-op hashes
  { "donothing32", "Do-Nothing function (only valid for measuring call overhead)",
    32, 32, 32, 0x00000000,
    DoNothingHash_seed_state, DoNothingHash_with_state, DoNothingHash },
  { "donothing64", "Do-Nothing function (only valid for measuring call overhead)",
    64, 64, 64, 0x00000000,
    DoNothingHash_seed_state, DoNothingHash_with_state, DoNothingHash },
  { "donothing128", "Do-Nothing function (only valid for measuring call overhead)",
    128, 128, 128, 0x00000000,
    DoNothingHash_seed_state, DoNothingHash_with_state, DoNothingHash },
  { "NOP_OAAT_read64", "Noop function (only valid for measuring call + OAAT reading overhead)",
    32, 32, 64, 0x00000000,
    NULL, NoopOAATReadHash_with_state, NoopOAATReadHash },
  // -- Crap hashes
  { "BadHash", "very simple XOR shift",
    32, 32, 32, 0xDD7BCD54,
    NULL, BadHash_with_state, BadHash },
  { "crc32", "CRC-32",
    32, 32, 32, 0x25B1FDC6,
    NULL, crc32_with_state_test, crc32_test },
#if defined(__SSE4_2__) && defined(__x86_64__)
  /* Even 32 uses crc32q, quad only */
  { "crc32_hw", "SSE4.2 crc32 in HW",
    32, 32, 32, 0x2FA96A98,
    NULL, crc32c_hw_with_state_test, crc32c_hw_test },
  { "crc32_hw1", "Faster Adler SSE4.2 crc32 in HW",
    32, 32, 32, 0x2FA96A98,
    NULL, crc32c_hw1_with_state_test, crc32c_hw1_test },
  { "crc64_hw", "SSE4.2 crc64 in HW",
    64, 64, 64, 0x222F1BFE,
    NULL, crc64c_hw_with_state_test, crc64c_hw_test },
#endif

  // -- Message Digests/NIST Hash Functions.
  { "md5_128a", "MD5, with a 64 bit seed of the start state",
    64, bitsizeof(md5_context), 128, 0x1D760569,
    md5_seed_state, md5_with_state, md5_test },
  { "md5_32a", "MD5, first 32 bits, with a 64 bit seed of the start start",
    64, bitsizeof(md5_context), 32, 0xB9CE2A3B,
    md5_seed_state, md5_32_with_state, md5_32 },
  { "sha1_32a", "SHA1, 32 bit seed, returning first 32 bits",
    32, 32, 32, 0x0929B127,
    NULL, sha1_32a_with_state, sha1_32a },
  { "sha1_64a", "SHA1, 32 bit seed, returning first 64 bits",
    32, 32, 64, 0xA9A6674D,
    NULL, sha1_64a_with_state, sha1_64a },
  { "sha1_32b", "SHA1, 32 bit seed, first 32 bits xored with last 32 bits",
    32, 32, 32, 0x37846930,
    NULL, sha1_32b_with_state, sha1_32b },
#if 0
  { "sha1_64a", "SHA1 64-bit, first 64 bits of result",
    32, 32, 32, 0xA9A6674D,
    NULL, NULL, sha1_64a },
  { "sha2_32a", "SHA2, first 32 bits of result",
    32, 32, 32, 0x00000000,
    NULL, NULL, sha2_32a },
  { "sha2_64a", "SHA2, first 64 bits of result",
    32, 32, 64, 0x00000000,
    NULL, NULL, sha2_64a },
  { "sha3_32a", "SHA3, first 32 bits of result",
    32, 32, 32, 0x00000000,
    NULL, NULL, sha3_32a },
  { "sha3_64a", "SHA3, first 64 bits of result",
    32, 32, 64, 0x00000000,
    NULL, NULL, sha3_64a },
  { "blake2_32a", "BLAKE2, first 32 bits of result",
    32, 32, 32, 0x00000000,
    NULL, NULL, BLAKE2_32a },
  { "blake2_64a", "BLAKE2, first 64 bits of result",
    32, 32, 64, 0x00000000,
    NULL, NULL, BLAKE2_64a },
  { "bcrypt_64a", "bcrypt, first 64 bits of result",
    32, 32, 64, 0x00000000,
    NULL, NULL, bcrypt_64a },
  { "scrypt_64a", "scrypt, first 64 bits of result",
    32, 32, 64, 0x00000000,
    NULL, NULL, scrypt_64a },
#endif

  // -- Others
#if 0 && defined(__x86_64__) && (defined(__linux__) || defined(__APPLE__))
  // elf64 or macho64 only
  { "fhtw", "fhtw asm",
    32, 32, 64, 0x0,
    NULL, NULL, fhtw_test },
#endif
  { "FNV1a", "Fowler-Noll-Vo hash, 32-bit",
    32, 32, 32, 0xDF0B85E1,
    NULL, FNV32a_with_state_test, FNV32a_test },
  { "FNV1a_YT", "FNV1a-YoshimitsuTRIAD 32-bit sanmayce",
    32, 32, 32, 0x2E5B2C34,
    NULL, FNV32a_YoshimitsuTRIAD_with_state_test, FNV32a_YoshimitsuTRIAD_test },
  { "FNV64", "Fowler-Noll-Vo hash, 64-bit",
    64, 64, 64, 0xA9E47EDA,
    NULL, FNV64a_with_state_test, FNV64a_test },
  { "bernstein", "Bernstein, 32-bit",
    32, 32, 32, 0x48EABA06,
    NULL, Bernstein_with_state, Bernstein },
  { "lookup3", "Bob Jenkins' lookup3",
    32, 32, 32, 0x3C20C790,
    NULL, lookup3_with_state_test, lookup3_test },
  { "superfast", "Paul Hsieh's SuperFastHash",
    32, 32, 32, 0x9A5C37F0,
    NULL, SuperFastHash, NULL },
  { "MurmurOAAT", "Murmur one-at-a-time",
    32, 32, 32, 0xFF008FEC,
    NULL, MurmurOAAT_with_state_test, MurmurOAAT_test },
  { "Crap8", "Crap8",
    32, 32, 32, 0x20595F51,
    NULL, Crap8_test, NULL },

  // SpoookyHash
  { "Spooky32", "Bob Jenkins' SpookyHash, 32-bit seed, 32-bit result",
    32, 128, 32, 0xC4766A8F,
    SpookyHash_seed_state_test, SpookyHash32_with_state_test, SpookyHash32_test },
  { "Spooky64", "Bob Jenkins' SpookyHash, 64-bit seed, 64-bit result",
    64, 128, 64, 0x1A6C1BDF,
    SpookyHash_seed_state_test, SpookyHash64_with_state_test, SpookyHash64_test },
  { "Spooky128", "Bob Jenkins' SpookyHash, 128-bit seed, 128-bit result",
    128, 128, 128, 0xC633C71E,
    SpookyHash_seed_state_test, SpookyHash128_with_state_test, SpookyHash128_test },

  // MurmurHash2
  { "Murmur2", "MurmurHash2 for x86, 32-bit",
    32, 32, 32, 0xDCA95F47,
    NULL, MurmurHash2_with_state_test, MurmurHash2_test },
  { "Murmur2A", "MurmurHash2A for x86, 32-bit",
    32, 32, 32, 0xF8186BCC,
    NULL, MurmurHash2A_with_state_test, MurmurHash2A_test },
  { "Murmur2B", "MurmurHash2 for x64, 64-bit",
    64, 64, 64, 0xBDB5B9A8,
    NULL, MurmurHash64A_with_state_test, MurmurHash64A_test },
  { "Murmur2C", "MurmurHash2 for x86, 64-bit",
    64, 64, 64, 0x378C7E87,
    NULL, MurmurHash64B_with_state_test, MurmurHash64B_test },

  // MurmurHash3
  { "Murmur3A", "MurmurHash3 for x86, 32-bit",
    32, 32, 32, 0xB3832598,
    NULL, MurmurHash3_x86_32_with_state, MurmurHash3_x86_32 },
  { "Murmur3C", "MurmurHash3 for x86, 128-bit",
    32, 32, 128, 0xB8A4C40B,
    NULL, MurmurHash3_x86_128_with_state, MurmurHash3_x86_128 },
#if defined(__x86_64__)
  { "Murmur3F", "MurmurHash3 for x64, 128-bit",
    32, 32, 128, 0x2D086017,
    NULL, MurmurHash3_x64_128_with_state, MurmurHash3_x64_128 },
#endif

  { "PMurHash32", "Shane Day's portable-ized MurmurHash3 for x86, 32-bit.",
    32, 32, 32, 0xB3832598,
    NULL, PMurHash32_with_state_test, PMurHash32_test },

  // BeagleHash_32_xx
  { "BeagleHash_32_32", "Yves Orton's hash for 64-bit in 32-bit mode (32-bit seed).",
    32, 128, 32, 0x940EB9A6,
    beagle_seed_state_128_a_smhasher, beagle_hash_with_state_32_128_a_smhasher, beagle_hash_32_32_a_smhasher_test },
  { "BeagleHash_32_64", "Yves Orton's hash for 64-bit in 32-bit mode (64-bit seed).",
    64, 128, 32, 0x11381639,
    beagle_seed_state_128_a_smhasher, beagle_hash_with_state_32_128_a_smhasher, beagle_hash_32_64_a_smhasher_test },
  { "BeagleHash_32_96", "Yves Orton's hash for 64-bit in 32-bit mode (96-bit seed).",
    96, 128, 32, 0x870B75F3,
    beagle_seed_state_128_a_smhasher, beagle_hash_with_state_32_128_a_smhasher, beagle_hash_32_96_a_smhasher_test },
  { "BeagleHash_32_112", "Yves Orton's hash for 64-bit in 32-bit mode (112-bit seed).",
    112, 128, 32, 0xDFA0C5B1,
    beagle_seed_state_128_a_smhasher, beagle_hash_with_state_32_128_a_smhasher, beagle_hash_32_112_a_smhasher_test },
  { "BeagleHash_32_127", "Yves Orton's hash for 64-bit in 32-bit mode (127-bit seed).",
    127, 128, 32, 0x383C7851,
    beagle_seed_state_128_a_smhasher, beagle_hash_with_state_32_128_a_smhasher, beagle_hash_32_127_a_smhasher_test },

  // BeagleHash_64_xx
  { "BeagleHash_64_32", "Yves Orton's hash for 64-bit. (32 bit seed)",
    32, 128, 64, 0x67FD7728,
    beagle_seed_state_128_a_smhasher, beagle_hash_with_state_64_128_a_smhasher, beagle_hash_64_32_a_smhasher_test },
  { "BeagleHash_64_64", "Yves Orton's hash for 64-bit. (64 bit seed)",
    64, 128, 64, 0xDC76CC97,
    beagle_seed_state_128_a_smhasher, beagle_hash_with_state_64_128_a_smhasher, beagle_hash_64_64_a_smhasher_test },
  { "BeagleHash_64_96", "Yves Orton's hash for 64-bit (96 bit seed).",
    96, 128, 64, 0xEF375BD9,
    beagle_seed_state_128_a_smhasher, beagle_hash_with_state_64_128_a_smhasher, beagle_hash_64_96_a_smhasher_test },
  { "BeagleHash_64_112", "Yves Orton's hash for 64-bit (112 bit seed).",
    112, 128, 64, 0x05A1EC2C,
    beagle_seed_state_128_a_smhasher, beagle_hash_with_state_64_128_a_smhasher, beagle_hash_64_112_a_smhasher_test },
  { "BeagleHash_64_127", "Yves Orton's hash for 64-bit (127 bit seed).",
    127, 128, 64, 0xC918E5EB,
    beagle_seed_state_128_a_smhasher, beagle_hash_with_state_64_128_a_smhasher, beagle_hash_64_127_a_smhasher_test },

  { "SBOX", "Yves Orton's 64 bit substitution box hash for up to 32 char strings",
    128, (32 * 256 + 3) * 64, 64, 0xEF32DD9E,
    sbox_seed_state_smhasher_test, sbox_hash_with_state_smhasher_test, sbox_hash_smhasher_test },
  { "Zaphod64", "Yves Orton's 64 bit hash with 191 bit seed",
    191, 192, 64, 0xA0C90A68,
    zaphod64_seed_state_smhasher_test, zaphod64_hash_with_state_smhasher_test, zaphod64_hash_smhasher_test },
  { "Zaphod32", "Yves Orton's 32 bit hash with 95 bit seed",
    95, 96, 32, 0x68B920BE,
    zaphod32_seed_state_smhasher_test, zaphod32_hash_with_state_smhasher_test, zaphod32_hash_smhasher_test },
  { "Phat4", "Yves Orton's 32 bit hash with 96 bit seed",
    96, 96, 32, 0x6279B4AA,
    NULL, phat4_hash_with_state_smhasher_test, phat4_hash_smhasher_test },
  { "Lua53oaat", "Hash function from Lua53, pure one-at-a-time",
    32, 32, 32, 0x12D1F661,
    NULL, lua_v53_string_hash_oaat, NULL },
  { "Lua53", "Hash function from Lua53, (skip forward)",
    32, 32, 32, 0xCDA7039F,
    NULL, lua_v53_string_hash, NULL },
  { "Marvin32", "Marvin32 from MicroSoft",
    64, 64, 32, 0x27BA096C,
    NULL, marvin_32_smhasher_test, NULL },

#ifdef __SSE2__
  { "hasshe2", "SSE2 hasshe2, 256-bit",
    128, 128, 256, 0xBF469506,
    NULL, hasshe2_test, NULL },
#endif


#if 0
  { "fletcher2", "fletcher2 ZFS",
    32, 32, 64, 0x0,
    NULL, NULL, fletcher2 } //TODO
  { "fletcher4", "fletcher4 ZFS",
    32, 32, 64, 0x0,
    NULL, NULL, fletcher4 } //TODO
  { "Jesteress", "FNV1a-Jesteress 32-bit sanmayce",
    32, 32, 32, 0x0,
    NULL, NULL, Jesteress },
  { "Meiyan", "FNV1a-Meiyan 32-bit sanmayce",
    32, 32, 32, 0x0,
    NULL, NULL, Meiyan },
#endif
  { "sdbm", "sdbm - with seeding (as in perl5)",
    32, 32, 32, 0xFBDF4231,
    NULL, sdbm, NULL },
  { "x17", "x17",
    32, 32, 32, 0x4E1377BC,
    NULL, x17_test, NULL },
  // also called jhash:
  { "JenkinsOAATH", "Bob Jenkins' one-at-a-time with hardening (as in perl 5.18)",
    64, 64, 32, 0xADFF9537,
    NULL, JenkinsOAATH_with_state, JenkinsOAATH },
  { "JenkinsOAAT", "Bob Jenkins' one-at-a-time as in old perl5",
    32, 32, 32, 0xEF9997E4,
    NULL, JenkinsOAAT_with_state, JenkinsOAAT },
  { "MicroOAAT", "Small non-mul OAAT that passes collision checks (by funny-falcon)",
    32, 32, 32, 0x4571240C,
    NULL, MicroOAAT, NULL },
  { "HalfSipHash", "HalfSipHash 2-4, 32bit",
    64, 128, 32, 0x3D8A7D20,
    halfsiphash_seed_state_test, halfsiphash_with_state_test, halfsiphash_test },

  // and now the quality hash funcs, which mostly work
  // GoodOOAT passes whole SMHasher (by funny-falcon)
  { "GoodOAAT", "Small non-multiplicative OAAT",
    32, 32, 32, 0xAF641C21,
    NULL, GoodOAAT, NULL },
  { "SipHash", "SipHash 2-4",
    128, 256, 64, 0x72F98C9B,
    siphash_seed_state_test, siphash_with_state_test, siphash_test },
  // as in rust and swift
  { "SipHash13", "SipHash 1-3",
    128, 256, 64, 0x540605EF,
    siphash_seed_state_test, siphash13_with_state_test, siphash13_test },
#if defined(__x86_64__)
  { "fasthash32", "fast-hash 32bit",
    32, 32, 32, 0x3ABA1D3D,
    NULL, fasthash32_test, NULL },
  { "fasthash64", "fast-hash 64bit",
    64, 64, 64, 0x5E7615DA,
    NULL, fasthash64_test, NULL },
#endif

  // CityHash
  { "City32", "Google CityHash32WithSeed (old)",
    32, 32, 32, 0xE6BE7C8C,
    NULL, CityHash32_with_state_test, CityHash32_test },
  { "City64", "Google CityHash64WithSeed (old)",
    64, 64, 64, 0x29237E9B,
    NULL, CityHash64_with_state_test, CityHash64_test },
#if defined(__SSE4_2__) && defined(__x86_64__)
  { "City128", "Google CityHash128WithSeed (old)",
    128, 128, 128, 0x4D0DE603,
    NULL, CityHash128_with_state_test, CityHash128_test },
  { "CityCrc128", "Google CityHashCrc128WithSeed SSE4.2 (old)",
    128, 128, 128, 0xED41C3FA,
    NULL, CityHashCrc128_with_state_test, CityHashCrc128_test },
#endif
#if defined(__x86_64__)
  { "FarmHash64", "Google FarmHash64WithSeed",
    64, 64, 64, 0x9DD56012,
    NULL, FarmHash64_with_state_test, FarmHash64_test },
  { "FarmHash128", "Google FarmHash128WithSeed",
    128, 128, 128, 0x299A9ED2,
    NULL, FarmHash128_with_state_test, FarmHash128_test },
  { "farmhash64_c", "farmhash64_with_seed (C99)",
    64, 64, 64, 0x9DD56012,
    NULL, farmhash64_c_with_state_test, farmhash64_c_test },
  { "farmhash128_c", "farmhash128_with_seed (C99)",
    128, 128, 128, 0x299A9ED2,
    NULL, farmhash128_c_with_state_test, farmhash128_c_test },
#endif
#if defined(__x86_64__)
  { "xxHash32", "xxHash, 32-bit for x64",
    32, 32, 32, 0xD81B29EA,
    NULL, xxHash32_with_state_test, xxHash32_test },
  { "xxHash64", "xxHash, 64-bit",
    64, 64, 64, 0xFA0E3AE0,
    NULL, xxHash64_with_state_test, xxHash64_test },
#if 0
  { "xxhash256", "xxhash256, 64-bit unportable",
    32, 32, 64, 0x024B7CF4,
    NULL, NULL, xxhash256_test },
#endif
#endif
#if defined(__x86_64__)
  { "metrohash64_1", "MetroHash64_1 for 64-bit",
    32, 32, 64, 0x83998067,
    NULL, metrohash64_1_with_state_test, metrohash64_1_test },
  { "metrohash64_2", "MetroHash64_2 for 64-bit",
    32, 32, 64, 0xE899286F,
    NULL, metrohash64_2_with_state_test, metrohash64_2_test },
  { "metrohash128_1", "MetroHash128_1 for 64-bit",
    32, 32, 128, 0x36DF2A26,
    NULL, metrohash128_1_with_state_test, metrohash128_1_test },
  { "metrohash128_2", "MetroHash128_2 for 64-bit",
    32, 32, 128, 0x819CAD48,
    NULL, metrohash128_2_with_state_test, metrohash128_2_test },
#if defined(__SSE4_2__) && defined(__x86_64__)
  { "metrohash64crc_1", "MetroHash64crc_1 for x64",
    32, 32, 64, 0xEDE9EE59,
    NULL, metrohash64crc_1_with_state_test, metrohash64crc_1_test },
  { "metrohash64crc_2", "MetroHash64crc_2 for x64",
    32, 32, 64, 0x205FDFDD,
    NULL, metrohash64crc_2_with_state_test, metrohash64crc_2_test },
  { "metrohash128crc_1", "MetroHash128crc_1 for x64",
    32, 32, 128, 0x24F6DB0F,
    NULL, metrohash128crc_1_with_state_test, metrohash128crc_1_test },
  { "metrohash128crc_2", "MetroHash128crc_2 for x64",
    32, 32, 128, 0x4E6F3369,
    NULL, metrohash128crc_2_with_state_test, metrohash128crc_2_test },
#endif
#endif
#if defined(__x86_64__)
  { "cmetrohash64_1o", "cmetrohash64_1 (shorter key optimized) , 64-bit for x64",
    32, 32, 64, 0x83998067,
    NULL, cmetrohash64_1_optshort_with_state_test, cmetrohash64_1_optshort_test },
  { "cmetrohash64_1", "cmetrohash64_1, 64-bit for x64",
    32, 32, 64, 0x83998067,
    NULL, cmetrohash64_1_with_state_test, cmetrohash64_1_test },
  { "cmetrohash64_2", "cmetrohash64_2, 64-bit for x64",
    32, 32, 64, 0xE899286F,
    NULL, cmetrohash64_2_with_state_test, cmetrohash64_2_test },
#endif
#if defined(__SSE4_2__) && defined(__x86_64__)
  { "falkhash", "falkhash.asm with aesenc, 64-bit for x64",
    64, 64, 64, 0xD140F2DF,
    NULL, falkhash_with_state_test_cxx, falkhash_test_cxx },
#endif
  { "t1ha", "Fast Positive Hash (portable, best for: 64-bit, little-endian)",
    64, 64, 64, 0x9FEF8BAE,
    NULL, t1ha_with_state_test, t1ha_test },
  { "t1ha_64be", "Fast Positive Hash (portable, best for: 64-bit, big-endian)",
    64, 64, 64, 0x8EC286C0,
    NULL, t1ha_64be_with_state_test, t1ha_64be_test },
  { "t1ha_32le", "Fast Positive Hash (portable, best for: 32-bit, little-endian)",
    64, 64, 64, 0x093A3895,
    NULL, t1ha_32le_with_state_test, t1ha_32le_test },
  { "t1ha_32be", "Fast Positive Hash (portable, best for: 32-bit, big-endian)",
    64, 64, 64, 0xF2F797FF,
    NULL, t1ha_32be_with_state_test, t1ha_32be_test },
#if (defined(__SSE4_2__) && defined(__x86_64__)) || defined(_M_X64)
  { "t1ha_crc", "Fast Positive Hash (machine-specific, requires: SSE4.2 CRC32C)",
    64, 64, 64, 0x264E824A,
    NULL, t1ha_crc_with_state_test, t1ha_crc_test },
#endif
#if defined(__AES__) || defined(_M_X64) || defined(_M_IX86)
  { "t1ha_aes", "Fast Positive Hash (machine-specific, requires: AES-NI)",
    64, 64, 64, 0x8A4F7A68,
    NULL, t1ha_aes_with_state_test, t1ha_aes_test },
#endif
#if defined(__GNUC__) && UINT_MAX != ULONG_MAX
#define MUM_VERIFY 0xE9816A4F
#else
/* this is almost for sure wrong. if necessary just fix it */
#define MUM_VERIFY 0xA973C6C0
#endif
  { "MUM", "github.com/vnmakarov/mum-hash",
    64, 64, 64, 0x5762CA72,
    NULL, mum_hash_with_state_test, mum_hash_test },
};
int g_hashes_sizeof= sizeof(g_hashes);

/* length of a common prefix */
int _strni_common_prefix_len(const char *s1, const char *s2, size_t n) {
  int f, l;
  int o_n= n;

  do {
    if (((f = (unsigned char)(*(s1++))) >= 'A') && (f <= 'Z')) f -= 'A' - 'a';
    if (((l = (unsigned char)(*(s2++))) >= 'A') && (l <= 'Z')) l -= 'A' - 'a';
  } while (--n && f && (f == l));
  if (f != l) n++;
  return  o_n - n;
}

HashInfo * findHash ( const char * name )
{
  for(size_t i = 0; i < sizeof(g_hashes) / sizeof(HashInfo); i++)
  {
    if(_stricmp(name,g_hashes[i].name) == 0) return &g_hashes[i];
  }
  printf("Invalid hash '%s' specified\n",name);
  int longest= 0;
  int name_len= strlen(name);
  for(size_t i = 0; i < sizeof(g_hashes) / sizeof(HashInfo); i++)
  {
    int l= _strni_common_prefix_len(name,g_hashes[i].name,name_len);
    if (l > longest) longest = l;
  }
  if (longest) {
    printf("Possibly you meant one of the following...\n");
    for(size_t i = 0; i < sizeof(g_hashes) / sizeof(HashInfo); i++)
    {
      if(_strnicmp(name,g_hashes[i].name,longest) == 0)
        printf("  %s\n",g_hashes[i].name);
    }
  } else {
    printf("No hashes start with '%s'\n",name);
  }
  printf("You can use the --list option to see what functions are available to test.\n");
  return NULL;
}

//-----------------------------------------------------------------------------

uint32_t g_inputVCode = 1;
uint32_t g_outputVCode = 1;
uint32_t g_resultVCode = 1;

HashInfo * g_hashUnderTest = NULL;

void VerifyHash ( const void * key, int len, uint32_t seed, void * out )
{
  g_inputVCode = MurmurOAAT(key,len,g_inputVCode);
  g_inputVCode = MurmurOAAT(&seed,sizeof(uint32_t),g_inputVCode);

  g_hashUnderTest->hash(key,len,seed,out);

  g_outputVCode = MurmurOAAT(out,g_hashUnderTest->hashbits/8,g_outputVCode);
}


#ifdef _MSC_VER
static char* strndup(char const *s, size_t n)
{
  size_t len = strnlen(s, n);
  char *p = (char*) malloc(len + 1);

  if (p == NULL)
    return NULL;

  p[len] = '\0';
  return (char*) memcpy(p, s, len);
}
#endif

#define PFX_EQ(x,l,y) ((l >= sizeof(y)-1) && strncmp(x,"" y "", sizeof(y)-1) == 0)
#define PFX_NE(x,l,y) ((l < sizeof(y)-1) || strncmp(x,"" y "", sizeof(y)-1) != 0)
#define EQ(x,l,y) ((l == sizeof(y)-1) && strncmp(x,"" y "", sizeof(y)-1) == 0)
#define NE(x,l,y) ((l != sizeof(y)-1) || strncmp(x,"" y "", sizeof(y)-1) != 0)


uint32_t g_verbose;
double   g_confidence;

int main ( int argc, char ** argv )
{
#if (defined(__x86_64__) && __SSE4_2__) || defined(_M_X64) || defined(_X86_64_)
  const char * defaulthash = "metrohash64crc_1"; /* "murmur3a"; */
#else
  const char * defaulthash = "t1ha_32le";
#endif
  const char * hashToTest = defaulthash;
  bool opt_validate = false;
  
  setvbuf(stdout, NULL, _IONBF, 0); /* autoflush stdout */
  g_verbose = 0;
  g_confidence = sigmasToProb(5.0);

  for(int i = 1; i < argc; i++) {
    char * arg= argv[i];
    int arg_len= strlen(arg);

    if (PFX_NE(arg,arg_len,"--")) {
      hashToTest = arg;
      break;
    }
    else
    if (EQ(arg,arg_len,"--list")) {
      printf("%-18s|Seed|State|Hash|\n","");
      printf("%-18s|%4s|%5s|%4s|%s\n","Name","Bits","Bits","Bits","Description");
      printf("%.18s+%.4s+%5s+%.4s|%s\n",
          "-------------------","----","-----","----","--------------------------");
      int old_api= 0;
      for(size_t i = 0; i < sizeof(g_hashes) / sizeof(HashInfo); i++) {
        if (!g_hashes[i].hash_with_state) old_api++;
        printf("%-18s|%4d|%5d|%4d|%s%s\n",
            g_hashes[i].name, g_hashes[i].seedbits, g_hashes[i].statebits, g_hashes[i].hashbits,
            g_hashes[i].hash_with_state ? "" : "*", g_hashes[i].desc);
      }
      if (old_api)
        printf(
          "# NOTE: Hash functions whose description starts with a star (*) character\n"
          "# are possibly not being seeded correctly and results may be misleading.\n");
      exit(0);
    }
    else
    if (EQ(arg,arg_len,"--validate")) {
      opt_validate = true;
      continue;
    }
    else
    if (EQ(arg,arg_len,"--verbose")) {
      g_verbose++;
      continue;
    }
    else
    if (PFX_EQ(arg,arg_len,"-v")) {
      arg+=2;
      do {
        g_verbose++;
      } while ( *arg++ == 'v' );
    }
    else
    if (PFX_EQ(arg,arg_len,"--confidence=")){
      arg += sizeof("--confidence=") - 1;
      g_confidence = atof(arg);
      if (g_confidence > 1) g_confidence /= 100;
      if (g_verbose > 2)
        printf("set confidence to %.6f via --confidence\n", g_confidence);
    }
    else
    if (PFX_EQ(arg,arg_len,"--sigmas=")){
      arg += sizeof("--sigmas=") - 1;
      g_confidence = sigmasToProb(atof(arg));
      if (g_verbose > 2)
        printf("set confidence to %.6f via --sigmas\n", g_confidence);
    }
    else
    if (EQ(arg,arg_len,"--stream")) {
      g_runCtrStream = true;
    }
    else
    if (PFX_EQ(arg,arg_len,"--stream-key-len=")) {
      arg += sizeof("--stream-key-len=") - 1;
      char *endptr;
      g_streamKeyLen = strtol(arg, &endptr, 16);
      if (endptr == arg) {
        printf("No argument for --rng-seed\n");
        exit(1);
      }
      else
      if (endptr != '\0') {
        printf("Bad argument for --rng-seed '%s'\n",arg);
        exit(1);
      }
    }
    else
    if (PFX_EQ(arg,arg_len,"--rng-seed=")) {
      arg += sizeof("--rng-seed=") - 1;
      char *endptr;
      g_rngSeed = strtol(arg, &endptr, 16);
      if (endptr == arg) {
        printf("No argument for --rng-seed\n");
        exit(1);
      }
      else
      if (endptr != '\0') {
        printf("Bad argument for --rng-seed '%s'\n",arg);
        exit(1);
      }
    }
    else
    if (PFX_EQ(arg,arg_len,"--test=") && arg_len > 8) {
      /* default: --test=All. comma seperated list of options */
      char *opt = (char *)&arg[7];
      char *rest = opt;
      char *p;
      bool found = false;
      g_testAll = false;

      do {
        if ((p = strchr(rest, ','))) {
          opt = strndup(rest, p-rest);
          rest = p+1;
        } else {
          opt = rest;
        }
        for(size_t i = 0; i < sizeof(g_testopts) / sizeof(TestOpts); i++) {
          if (strcmp(opt, g_testopts[i].name) == 0) {
            g_testopts[i].var = true; found = true; break;
          }
        }
        if (!found) {
          printf("Invalid option: --test=%s\n", opt);
          printf("Valid tests: --test=%s", g_testopts[0].name);
          for(size_t i = 1; i < sizeof(g_testopts) / sizeof(TestOpts); i++) {
            printf(",%s", g_testopts[i].name);
          }
          printf("\n");
          exit(0);
        }
      } while (p);
    }
    else {
      printf("SMHasher: unknown option: %s\n", arg);
      printf("Valid options:\n");
      printf("--list                list hashes available for testing\n");
      printf("--confidence          set the confidence level as a percentage\n");
      printf("--sigmas              set the confidence level as a sigma number\n");
      printf("--verbose             run verbose?\n");
      printf("--validate            validate supported hashes but do not run tests\n");
      printf("--stream              use hash function to print random stream to stdout\n");
      printf("--stream-key-len=NUM  set key len for stream mode (defaults to 8)\n");
      printf("--rng-seed=NUM        seed for rng used to seed stream mode hashing\n");
      printf("--test=NAME1,NAME2    which tests to run? default is all, available:\n");
      for(size_t i = 0; i < sizeof(g_testopts) / sizeof(TestOpts); i++)
        printf("%s%s", i ? ", " : "", g_testopts[i].name);
      printf("\n");
      exit(1);
    }
  }

  // Code runs on the 3rd CPU by default
  SetAffinity((1 << 2));

  SelfTest(opt_validate);

  //----------
  HashInfo * pInfo = findHash(hashToTest);
  if (!pInfo) {
    printf("Unknown hash '%s'\n",hashToTest);
    exit(1);
  }

  clock_t timeBegin = clock();

  testHashByInfo(pInfo,0,g_confidence);

  clock_t timeEnd = clock();

  //----------

  printf("\n");
  printf("Input vcode 0x%08x, Output vcode 0x%08x, Result vcode 0x%08x\n",
          g_inputVCode,g_outputVCode,g_resultVCode);
  printf( "Verification value is 0x%08x - Testing took %f seconds\n",
          g_verify, double(timeEnd - timeBegin) / double(CLOCKS_PER_SEC) );
  printf("-------------------------------------------------------------------------------\n");
  return 0;
}
/* vim: set sts=2 sw=2 et: */
