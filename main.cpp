#include "Platform.h"
#include "Hashes.h"
#include "KeysetTest.h"
#include "SpeedTest.h"
#include "AvalancheTest.h"
#include "DifferentialTest.h"
#include "PMurHash.h"
#include "beagle_hash.h"
#include "phat_hash.h"
#include "zaphod32_hash.h"
#include "zaphod64_hash.h"
#include "sbox_hash.h"
#include "Marvin32.h"
#include "siphash.h"
#include "RunTests.h"
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
bool g_testWindow      = false;
bool g_testText        = false;
bool g_testZeroes      = false;
bool g_testEffs        = false;
bool g_testSeed        = false;

struct TestOpts {
  bool         &var;
  const char*  name;
};
TestOpts g_testopts[] =
{
  { g_testAll, 		"All" },
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


HashInfo g_hashes[] =
{
  // -- No-op hashes
  { DoNothingHash, NULL, NULL, 32,  32,  32, 0x00000000, "donothing32",
      "Do-Nothing function (only valid for measuring call overhead)", NULL },
  { DoNothingHash, NULL, NULL, 32,  32,  64, 0x00000000, "donothing64",
      "Do-Nothing function (only valid for measuring call overhead)", NULL },
  { DoNothingHash, NULL, NULL, 32,  32, 128, 0x00000000, "donothing128",
      "Do-Nothing function (only valid for measuring call overhead)", NULL },
  { NoopOAATReadHash, NULL, NULL, 32, 32, 64, 0x00000000, "NOP_OAAT_read64",
      "Noop function (only valid for measuring call + OAAT reading overhead)", NULL },
  // -- Crap hashes
  { BadHash, NULL, NULL, 32, 32, 32, 0xDD7BCD54, "BadHash",
      "very simple XOR shift", NULL },
  { sumhash, NULL, NULL, 32, 32, 32, 0x199E1F62, "sumhash",
      "sum all bytes", NULL },
  { sumhash32, NULL, NULL, 32, 32, 32, 0x752519AF, "sumhash32",
      "sum all 32bit words", NULL },
  { crc32, NULL, NULL, 32,  32,  32, 0x25B1FDC6, "crc32",
      "CRC-32", NULL },
#if defined(__SSE4_2__) && defined(__x86_64__)
  /* Even 32 uses crc32q, quad only */
  { crc32c_hw_test, NULL, NULL, 32, 32, 32, 0x2FA96A98, "crc32_hw",
      "SSE4.2 crc32 in HW", NULL },
  { crc32c_hw1_test, NULL, NULL, 32, 32, 32, 0x2FA96A98, "crc32_hw1",
      "Faster Adler SSE4.2 crc32 in HW", NULL },
  { crc64c_hw_test, NULL, NULL, 32, 32, 64, 0x2D944BE6, "crc64_hw",
      "SSE4.2 crc64 in HW", NULL },
#endif

  // -- Message Digests/NIST Hash Functions.
  { md5_32, NULL, NULL, 32,  32,  32, 0x15BBA3E0, "md5_32a",
      "MD5, first 32 bits", NULL },
  { sha1_32a, NULL, NULL, 32,  32,  32, 0x0929B127, "sha1_32a",
      "SHA1, first 32 bits", NULL },
  { sha1_64a, NULL, NULL, 32,  32,  64, 0xA9A6674D, "sha1_64a",
      "SHA1, first 64 bits", NULL },
  { sha1_32b, NULL, NULL, 32,  32,  32, 0x37846930, "sha1_32b",
      "SHA1, first 32 bits xored with last 32 bits", NULL },
#if 0
  { sha1_64a, NULL, NULL, 32, 32, 32, 0xA9A6674D, "sha1_64a",
      "SHA1 64-bit, first 64 bits of result", NULL },
  { sha2_32a, NULL, NULL, 32, 32, 32, 0x00000000, "sha2_32a",
      "SHA2, first 32 bits of result", NULL },
  { sha2_64a, NULL, NULL, 32, 32, 64, 0x00000000, "sha2_64a",
      "SHA2, first 64 bits of result", NULL },
  { sha3_32a, NULL, NULL, 32, 32, 32, 0x00000000, "sha3_32a",
      "SHA3, first 32 bits of result", NULL },
  { sha3_64a, NULL, NULL, 32, 32, 64, 0x00000000, "sha3_64a",
      "SHA3, first 64 bits of result", NULL },
  { BLAKE2_32a, NULL, NULL, 32, 32, 32, 0x00000000, "blake2_32a",
      "BLAKE2, first 32 bits of result", NULL },
  { BLAKE2_64a, NULL, NULL, 32, 32, 64, 0x00000000, "blake2_64a",
      "BLAKE2, first 64 bits of result", NULL },
  { bcrypt_64a, NULL, NULL, 32, 32, 64, 0x00000000, "bcrypt_64a",
      "bcrypt, first 64 bits of result", NULL },
  { scrypt_64a, NULL, NULL, 32, 32, 64, 0x00000000, "scrypt_64a",
      "scrypt, first 64 bits of result", NULL },
#endif

  // -- Others
#if 0 && defined(__x86_64__) && (defined(__linux__) || defined(__APPLE__))
  // elf64 or macho64 only
  { fhtw_test, NULL, NULL, 32, 32, 64, 0x0,        "fhtw",
      "fhtw asm", NULL },
#endif
  { FNV32a, NULL, NULL, 32, 32, 32, 0xDF0B85E1, "FNV1a",
      "Fowler-Noll-Vo hash, 32-bit", NULL },
  { FNV32a_YoshimitsuTRIAD, NULL, NULL, 32, 32, 32,0x2E5B2C34, "FNV1a_YT",
      "FNV1a-YoshimitsuTRIAD 32-bit sanmayce", NULL },
  { FNV64a, NULL, NULL, 32, 32, 64, 0xF1E96686, "FNV64",
      "Fowler-Noll-Vo hash, 64-bit", NULL },
  { Bernstein, NULL, NULL, 32, 32, 32, 0x48EABA06, "bernstein",
      "Bernstein, 32-bit", NULL },
  { lookup3_test, NULL, NULL, 32, 32, 32, 0x3C20C790, "lookup3",
      "Bob Jenkins' lookup3", NULL },
  { SuperFastHash, NULL, NULL, 32, 32, 32, 0x980ACD1D, "superfast",
      "Paul Hsieh's SuperFastHash", NULL },
  { MurmurOAAT_test, NULL, NULL, 32, 32, 32, 0xFF008FEC, "MurmurOAAT",
      "Murmur one-at-a-time", NULL },
  { Crap8_test, NULL, NULL, 32, 32, 32, 0x20595F51, "Crap8",
      "Crap8", NULL },

  // SpoookyHash
  { SpookyHash32_test, NULL, NULL, 32, 32,   32, 0xC4766A8F, "Spooky32",
      "Bob Jenkins' SpookyHash, 32-bit result", NULL },
  { SpookyHash64_test, NULL, SpookyHash64_with_state_test,
      64, 64,   64, 0x1A6C1BDF, "Spooky64",
      "Bob Jenkins' SpookyHash, 64-bit result", NULL },
  { SpookyHash128_test, NULL, SpookyHash128_with_state_test,
      128, 128, 128, 0xC633C71E, "Spooky128",
      "Bob Jenkins' SpookyHash, 128-bit result", NULL },

  // MurmurHash2
  { MurmurHash2_test, NULL, NULL, 32, 32, 32, 0xDCA95F47, "Murmur2",
    "MurmurHash2 for x86, 32-bit", NULL },
  { MurmurHash2A_test, NULL, NULL, 32, 32, 32, 0xF8186BCC, "Murmur2A",
      "MurmurHash2A for x86, 32-bit", NULL },
  { MurmurHash64A_test, NULL, NULL, 32, 32, 64, 0x4FA749F1, "Murmur2B",
      "MurmurHash2 for x64, 64-bit", NULL },
  { MurmurHash64B_test, NULL, NULL, 32, 32, 64, 0x058FC93D, "Murmur2C",
      "MurmurHash2 for x86, 64-bit", NULL },

  // MurmurHash3
  { MurmurHash3_x86_32, NULL, NULL, 32, 32,  32, 0xB3832598, "Murmur3A",
      "MurmurHash3 for x86, 32-bit", NULL },
  { MurmurHash3_x86_128, NULL, NULL, 32, 32, 128, 0xB8A4C40B, "Murmur3C",
      "MurmurHash3 for x86, 128-bit", NULL },
#if defined(__x86_64__)
  { MurmurHash3_x64_128, NULL, NULL, 32, 32, 128, 0x2D086017, "Murmur3F",
      "MurmurHash3 for x64, 128-bit", NULL },
#endif

  { PMurHash32_test, NULL, NULL, 32, 32,  32, 0xB3832598, "PMurHash32",
      "Shane Day's portable-ized MurmurHash3 for x86, 32-bit.", NULL },

  // BeagleHash_32_xx
  { beagle_hash_32_32_a_smhasher_test,
    beagle_seed_state_128_a_smhasher, beagle_hash_with_state_32_128_a_smhasher,
    32, 128,  32, 0x940EB9A6,
    "BeagleHash_32_32", "Yves Orton's hash for 64-bit in 32-bit mode (32-bit seed).",
      NULL },
  { beagle_hash_32_64_a_smhasher_test,
    beagle_seed_state_128_a_smhasher, beagle_hash_with_state_32_128_a_smhasher,
      64, 128,  32, 0x11381639,
      "BeagleHash_32_64", "Yves Orton's hash for 64-bit in 32-bit mode (64-bit seed).",
      NULL },
  { beagle_hash_32_96_a_smhasher_test,
    beagle_seed_state_128_a_smhasher, beagle_hash_with_state_32_128_a_smhasher,
      96, 128,  32, 0x870B75F3,
      "BeagleHash_32_96", "Yves Orton's hash for 64-bit in 32-bit mode (96-bit seed).",
      NULL },
  { beagle_hash_32_112_a_smhasher_test,
    beagle_seed_state_128_a_smhasher, beagle_hash_with_state_32_128_a_smhasher,
      112, 128,  32, 0xDFA0C5B1,
      "BeagleHash_32_112", "Yves Orton's hash for 64-bit in 32-bit mode (112-bit seed).",
      NULL },
  { beagle_hash_32_127_a_smhasher_test,
    beagle_seed_state_128_a_smhasher, beagle_hash_with_state_32_128_a_smhasher,
      127, 128,  32, 0x383C7851,
      "BeagleHash_32_127", "Yves Orton's hash for 64-bit in 32-bit mode (127-bit seed).",
      NULL },

  // BeagleHash_64_xx
  { beagle_hash_64_32_a_smhasher_test,
    beagle_seed_state_128_a_smhasher, beagle_hash_with_state_64_128_a_smhasher,
    32, 128,   64, 0x67FD7728,
    "BeagleHash_64_32", "Yves Orton's hash for 64-bit. (32 bit seed)",
    NULL },
  { beagle_hash_64_64_a_smhasher_test,
    beagle_seed_state_128_a_smhasher, beagle_hash_with_state_64_128_a_smhasher,
    64, 128,   64, 0xDC76CC97,
    "BeagleHash_64_64", "Yves Orton's hash for 64-bit. (64 bit seed)",
    NULL },
  { beagle_hash_64_96_a_smhasher_test,
    beagle_seed_state_128_a_smhasher, beagle_hash_with_state_64_128_a_smhasher,
    96, 128,   64, 0xEF375BD9,
    "BeagleHash_64_96", "Yves Orton's hash for 64-bit (96 bit seed).",
    NULL },
  { beagle_hash_64_112_a_smhasher_test,
    beagle_seed_state_128_a_smhasher, beagle_hash_with_state_64_128_a_smhasher,
    112, 128,  64, 0x05A1EC2C,
    "BeagleHash_64_112", "Yves Orton's hash for 64-bit (112 bit seed).",
    NULL },
  { beagle_hash_64_127_a_smhasher_test,
    beagle_seed_state_128_a_smhasher, beagle_hash_with_state_64_128_a_smhasher,
    127, 128,  64, 0xC918E5EB,
    "BeagleHash_64_127", "Yves Orton's hash for 64-bit (127 bit seed).",
    NULL },

  // ZaphodHash, Marvin32, Phat, Phat4
  { sbox_hash_smhasher_test,
    sbox_seed_state_smhasher_test, sbox_hash_with_state_smhasher_test,
    128, (32 * 256 + 3) * 64, 64, 0xEF32DD9E,
    "SBOX", "Yves Orton's 64 bit substitution box hash for up to 32 char strings",
    NULL },
  { zaphod64_hash_smhasher_test,
    zaphod64_seed_state_smhasher_test, zaphod64_hash_with_state_smhasher_test,
    191, 192, 64, 0xA0C90A68,
    "Zaphod64", "Yves Orton's 64 bit hash with 191 bit seed",
    NULL },
  { zaphod32_hash_smhasher_test,
    zaphod32_seed_state_smhasher_test, zaphod32_hash_with_state_smhasher_test,
    95, 96, 32, 0x68B920BE,
    "Zaphod32", "Yves Orton's 32 bit hash with 95 bit seed",
    NULL },
  { phat4_hash_smhasher_test, NULL, phat4_hash_with_state_smhasher_test,
    96, 96, 32, 0x6279B4AA,
    "Phat4", "Yves Orton's 32 bit hash with 96 bit seed",
    NULL },
  { marvin_32_smhasher_test, NULL, NULL,
    32, 32, 32, 0x063EC884,
    "Marvin32", "Marvin32 from MicroSoft", NULL },

#ifdef __SSE2__
  { hasshe2_test, NULL, NULL, 32, 32, 256, 0xF5D39DFE, "hasshe2",     "SSE2 hasshe2, 256-bit", NULL },
#endif


#if 0
  { fletcher2, NULL, NULL, 32, 32, 64, 0x0, "fletcher2",
      "fletcher2 ZFS"} //TODO
  { fletcher4, NULL, NULL, 32, 32, 64, 0x0, "fletcher4",
      "fletcher4 ZFS"} //TODO
  { Jesteress, NULL, NULL, 32, 32, 32, 0x0, "Jesteress",
      "FNV1a-Jesteress 32-bit sanmayce", NULL },
  { Meiyan, NULL, NULL, 32, 32, 32, 0x0, "Meiyan",
      "FNV1a-Meiyan 32-bit sanmayce", NULL },
#endif
  { sdbm, NULL, NULL, 32, 32, 32, 0x2854BFA7, "sdbm",
      "sdbm as in perl5", NULL },
  { x17_test, NULL, NULL, 32, 32, 32, 0x4E1377BC, "x17",
      "x17", NULL },
  // also called jhash:
  { JenkinsOOAT, NULL, NULL, 32, 32, 32, 0x71675B26, "JenkinsOOAT",
      "Bob Jenkins' OOAT as in perl 5.18", NULL },
  { JenkinsOOAT_perl, NULL, NULL, 32, 32, 32, 0xEF9997E4, "JenkinsOOAT_perl",
      "Bob Jenkins' OOAT as in old perl5", NULL },
  { MicroOAAT, NULL, NULL,
    32, 32, 32, 0x4571240C,
    "MicroOAAT", "Small non-mul OAAT that passes collision checks (by funny-falcon)", NULL },
  { halfsiphash_test, NULL, NULL, 32, 32, 32, 0x4FF258EF,
    "HalfSipHash", "HalfSipHash 2-4, 32bit", NULL },

  // and now the quality hash funcs, which mostly work
  // GoodOOAT passes whole SMHasher (by funny-falcon)
  { GoodOAAT, NULL, NULL, 32, 32, 32, 0xAF641C21, "GoodOAAT",
      "Small non-multiplicative OAAT", NULL },
  { siphash_test, siphash_seed_state_test, siphash_with_state_test,
    128, 256, 64, 0x72F98C9B,
    "SipHash", "SipHash 2-4", NULL },
  // as in rust and swift
  { siphash13_test, siphash_seed_state_test, siphash13_with_state_test,
    128, 256, 64, 0x540605EF,
    "SipHash13", "SipHash 1-3", NULL },
#if defined(__x86_64__)
  { fasthash32_test, NULL, NULL, 32, 32, 32, 0x3ABA1D3D, "fasthash32",
      "fast-hash 32bit", NULL },
  { fasthash64_test, NULL, NULL, 32, 32, 64, 0xD26B7923, "fasthash64",
      "fast-hash 64bit", NULL },
#endif

  // CityHash
  { CityHash32_test, NULL, NULL, 32, 32, 32, 0xE6BE7C8C, "City32",
      "Google CityHash32WithSeed (old)", NULL },
  { CityHash64_test, NULL, CityHash64_with_state_test,
    64, 64, 64, 0x29237E9B,
    "City64", "Google CityHash64WithSeed (old)", NULL },
#if defined(__SSE4_2__) && defined(__x86_64__)
  { CityHash128_test, NULL, CityHash128_with_state_test,
    128, 128, 128, 0x4D0DE603,
    "City128", "Google CityHash128WithSeed (old)", NULL },
  { CityHashCrc128_test, NULL, CityHashCrc128_with_state_test,
    128, 128, 128, 0xED41C3FA,
    "CityCrc128", "Google CityHashCrc128WithSeed SSE4.2 (old)", NULL },
#endif
#if defined(__x86_64__)
  { FarmHash64_test, NULL, NULL, 32, 32, 64, 0x141219B5, "FarmHash64",
      "Google FarmHash64WithSeed", NULL },
  { FarmHash128_test, NULL, NULL, 32, 32, 128, 0x40F9C0B1, "FarmHash128",
      "Google FarmHash128WithSeed", NULL },
  { farmhash64_c_test, NULL, NULL, 32, 32, 64, 0x141219B5, "farmhash64_c",
      "farmhash64_with_seed (C99)", NULL },
  { farmhash128_c_test, NULL, NULL, 32, 32, 128, 0x40F9C0B1, "farmhash128_c",
      "farmhash128_with_seed (C99)", NULL },
#endif
#if defined(__x86_64__)
  { xxHash32_test, NULL, NULL, 32, 32, 32, 0xD81B29EA, "xxHash32",
      "xxHash, 32-bit for x64", NULL },
  { xxHash64_test, NULL, NULL, 32, 32, 64, 0x5C9EADA4, "xxHash64",
      "xxHash, 64-bit", NULL },
#if 0
  { xxhash256_test, NULL, NULL, 32, 32, 64, 0x024B7CF4, "xxhash256",
      "xxhash256, 64-bit unportable", NULL },
#endif
#endif
#if defined(__x86_64__)
  { metrohash64_1_test, NULL, NULL, 32, 32, 64, 0x83998067, "metrohash64_1",
      "MetroHash64_1 for 64-bit", NULL },
  { metrohash64_2_test, NULL, NULL, 32, 32, 64, 0xE899286F, "metrohash64_2",
      "MetroHash64_2 for 64-bit", NULL },
  { metrohash128_1_test, NULL, NULL, 32, 32, 128, 0x36DF2A26, "metrohash128_1",
      "MetroHash128_1 for 64-bit", NULL },
  { metrohash128_2_test, NULL, NULL, 32, 32, 128, 0x819CAD48, "metrohash128_2",
      "MetroHash128_2 for 64-bit", NULL },
#if defined(__SSE4_2__) && defined(__x86_64__)
  { metrohash64crc_1_test, NULL, NULL, 32, 32, 64, 0xEDE9EE59, "metrohash64crc_1",
      "MetroHash64crc_1 for x64", NULL },
  { metrohash64crc_2_test, NULL, NULL, 32, 32, 64, 0x205FDFDD, "metrohash64crc_2",
      "MetroHash64crc_2 for x64", NULL },
  { metrohash128crc_1_test, NULL, NULL, 32, 32, 128, 0x24F6DB0F, "metrohash128crc_1",
      "MetroHash128crc_1 for x64", NULL },
  { metrohash128crc_2_test, NULL, NULL, 32, 32, 128, 0x4E6F3369, "metrohash128crc_2",
      "MetroHash128crc_2 for x64", NULL },
#endif
#endif
#if defined(__x86_64__)
  { cmetrohash64_1_optshort_test, NULL, NULL, 32, 32, 64, 0x83998067, "cmetrohash64_1o",
      "cmetrohash64_1 (shorter key optimized) , 64-bit for x64", NULL },
  { cmetrohash64_1_test, NULL, NULL, 32, 32, 64, 0x83998067, "cmetrohash64_1",
      "cmetrohash64_1, 64-bit for x64", NULL },
  { cmetrohash64_2_test, NULL, NULL, 32, 32, 64, 0xE899286F, "cmetrohash64_2",
      "cmetrohash64_2, 64-bit for x64", NULL },
#endif
#if defined(__SSE4_2__) && defined(__x86_64__)
  { falkhash_test_cxx, NULL, falkhash_with_state_test_cxx, 64, 64, 64, 0xD140F2DF,
    "falkhash", "falkhash.asm with aesenc, 64-bit for x64", NULL },
#endif
  { t1ha_test, NULL, t1ha_with_state_test,
    64, 64, 64, 0x9FEF8BAE,
    "t1ha", "Fast Positive Hash (portable, best for: 64-bit, little-endian)", NULL },
  { t1ha_64be_test, NULL, t1ha_64be_with_state_test,
    64, 64, 64, 0x8EC286C0,
    "t1ha_64be", "Fast Positive Hash (portable, best for: 64-bit, big-endian)", NULL },
  { t1ha_32le_test, NULL, NULL, 32, 32, 64, 0x96C637F2, "t1ha_32le",
      "Fast Positive Hash (portable, best for: 32-bit, little-endian)", NULL },
  { t1ha_32be_test, NULL, NULL, 32, 32, 64, 0x43ABC981, "t1ha_32be",
      "Fast Positive Hash (portable, best for: 32-bit, big-endian)", NULL },
#if (defined(__SSE4_2__) && defined(__x86_64__)) || defined(_M_X64)
  { t1ha_crc_test, NULL, t1ha_crc_with_state_test,
    64, 64, 64, 0x264E824A,
    "t1ha_crc", "Fast Positive Hash (machine-specific, requires: SSE4.2 CRC32C)", NULL },
#endif
#if defined(__AES__) || defined(_M_X64) || defined(_M_IX86)
  { t1ha_aes_test, NULL, NULL, 32, 32, 64, 0x62117254, "t1ha_aes",
      "Fast Positive Hash (machine-specific, requires: AES-NI)", NULL },
#endif
#if defined(__GNUC__) && UINT_MAX != ULONG_MAX
#define MUM_VERIFY 0xE9816A4F
#else
/* this is almost for sure wrong. if necessary just fix it */
#define MUM_VERIFY 0xA973C6C0
#endif
  { mum_hash_test, NULL, NULL, 32, 32, 64, MUM_VERIFY, "MUM",
    "github.com/vnmakarov/mum-hash", NULL },
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

#define EQ(x,y) (strncmp(x,"" y "", sizeof(y)-1) == 0)
#define NE(x,y) (strncmp(x,"" y "", sizeof(y)-1) != 0)

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

    if (NE(arg,"--")) {
      hashToTest = arg;
      break;
    }
    else
    if (EQ(arg,"--list")) {
      printf("%-18s|Seed|Hash|\n","");
      printf("%-18s|%4s|%4s|%s\n","Name","Bits","Bits","Description");
      printf("%.18s+%.4s+%.4s|%s\n",
          "-------------------","----","----","--------------------------");
      for(size_t i = 0; i < sizeof(g_hashes) / sizeof(HashInfo); i++) {
        printf("%-18s|%4d|%4d|%s\n",
            g_hashes[i].name, g_hashes[i].seedbits, g_hashes[i].hashbits,
            g_hashes[i].desc);
      }
      exit(0);
    }
    else
    if (EQ(arg,"--validate")) {
      opt_validate = true;
      continue;
    }
    else
    if (EQ(arg,"--verbose")) {
      g_verbose++;
      continue;
    }
    else
    if (EQ(arg,"-v")) {
      arg+=2;
      do {
        g_verbose++;
      } while ( *arg++ == 'v' );
    }
    else
    if (EQ(arg,"--confidence=")){
      arg += sizeof("--confidence=") - 1;
      g_confidence = atof(arg);
      if (g_confidence > 1) g_confidence /= 100;
      if (g_verbose > 2)
        printf("set confidence to %.6f via --confidence\n", g_confidence);
    }
    else
    if (EQ(arg,"--sigmas=")){
      arg += sizeof("--sigmas=") - 1;
      g_confidence = sigmasToProb(atof(arg));
      if (g_verbose > 2)
        printf("set confidence to %.6f via --sigmas\n", g_confidence);
    }
    else
    if (EQ(arg,"--test=") && arg_len > 8) {
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
      printf("--test=NAME1,NAME2    which tests to run? default is all, available:\n  ");
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
