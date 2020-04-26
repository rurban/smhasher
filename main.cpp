#define _MAIN_CPP
#include "Platform.h"
#include "Hashes.h"
#include "KeysetTest.h"
#include "SpeedTest.h"
#include "AvalancheTest.h"
#include "DifferentialTest.h"
#include "HashMapTest.h"

#include <stdio.h>
#include <stdint.h>
#include <time.h>

//-----------------------------------------------------------------------------
// Configuration.

bool g_drawDiagram     = false;
bool g_testAll         = true;
bool g_testExtra       = false; // excessive torture tests: Sparse, Avalanche, DiffDist
bool g_testVerifyAll   = false;

bool g_testSanity      = false;
bool g_testSpeed       = false;
bool g_testHashmap     = false;
bool g_testAvalanche   = false;
bool g_testSparse      = false;
bool g_testPermutation = false;
bool g_testWindow      = false;
bool g_testCyclic      = false;
bool g_testTwoBytes    = false;
bool g_testText        = false;
bool g_testZeroes      = false;
bool g_testSeed        = false;
bool g_testPerlinNoise = false;
bool g_testDiff        = false;
bool g_testDiffDist    = false;
bool g_testMomentChi2  = false;
bool g_testPrng        = false;
bool g_testBIC         = false;
//bool g_testLongNeighbors = false;

double g_speed = 0.0;

struct TestOpts {
  bool         &var;
  const char*  name;
};
TestOpts g_testopts[] =
{
  { g_testAll,          "All" },
  { g_testVerifyAll,    "VerifyAll" },
  { g_testSanity,       "Sanity" },
  { g_testSpeed,        "Speed" },
  { g_testHashmap,      "Hashmap" },
  { g_testAvalanche,    "Avalanche" },
  { g_testSparse,       "Sparse" },
  { g_testPermutation,  "Permutation" },
  { g_testWindow,       "Window" },
  { g_testCyclic,       "Cyclic" },
  { g_testTwoBytes,     "TwoBytes" },
  { g_testText,	        "Text" },
  { g_testZeroes,       "Zeroes" },
  { g_testSeed,	        "Seed" },
  { g_testPerlinNoise,	"PerlinNoise" },
  { g_testDiff,         "Diff" },
  { g_testDiffDist,     "DiffDist" },
  { g_testBIC, 	        "BIC" },
  { g_testMomentChi2,   "MomentChi2" },
  { g_testPrng,         "Prng" },
  //{ g_testLongNeighbors,"LongNeighbors" }
};

bool MomentChi2Test ( struct HashInfo *info, int inputSize );

//-----------------------------------------------------------------------------
// This is the list of all hashes that SMHasher can test.

const char* quality_str[3] = { "SKIP", "POOR", "GOOD" };

// sorted by quality and speed
HashInfo g_hashes[] =
{
  // first the bad hash funcs, failing tests:
  { DoNothingHash,        32, 0x00000000, "donothing32", "Do-Nothing function (measure call overhead)", SKIP },
  { DoNothingHash,        64, 0x00000000, "donothing64", "Do-Nothing function (measure call overhead)", SKIP },
  { DoNothingHash,       128, 0x00000000, "donothing128", "Do-Nothing function (measure call overhead)", SKIP },
  { NoopOAATReadHash,     64, 0x00000000, "NOP_OAAT_read64", "Noop function (measure call + OAAT reading overhead)", SKIP },
  { BadHash,     	  32, 0xAB432E23, "BadHash", 	 "very simple XOR shift", SKIP },
  { sumhash,     	  32, 0x0000A9AC, "sumhash", 	 "sum all bytes", SKIP },
  { sumhash32,     	  32, 0x3D6DC280, "sumhash32",   "sum all 32bit words", SKIP },

 // here start the real hashes. first the problematic ones:
#ifdef HAVE_BIT32
 #define FIBONACCI_VERIF      0x09952480
 #define FNV2_VERIF           0x739801C5
 #define MULTSHIFT_VERIF      0x0
 #define PAIRMS_VERIF         0xE6ABA97D
#else
 #define FIBONACCI_VERIF      0xFE3BD380
 #define FNV2_VERIF           0x1967C625
 #define MULTSHIFT_VERIF      0x0
 #define PAIRMS_VERIF         0xB6B5D710
#endif
  // M. Dietzfelbinger, T. Hagerup, J. Katajainen, and M. Penttonen. A reliable randomized
  // algorithm for the closest-pair problem. J. Algorithms, 25:19–51, 1997.
  // must be skipped for hashmaps, extremly bad! FIXME
  { multiply_shift, __WORDSIZE,MULTSHIFT_VERIF, "multiply_shift", "Dietzfelbinger Multiply-shift on strings", POOR },
  { pair_multiply_shift, __WORDSIZE, PAIRMS_VERIF, "pair_multiply_shift", "Pair-multiply-shift", POOR },
  { crc32,                32, 0x3719DB20, "crc32",       "CRC-32 soft", POOR },
  { md5_128,             128, 0xF263F96F, "md5-128",     "MD5", GOOD },
  { md5_32,               32, 0x634E5AEC, "md5_32a",     "MD5, low 32 bits", POOR },
#ifdef _MSC_VER /* truncated long to 32 */
#  define SHA1_VERIF          0xED2F35E4
#  define SHA1a_VERIF         0x480A2B09
#else
#  define SHA1_VERIF          0x6AF411D8
#  define SHA1a_VERIF         0xB3122757
#endif
  { sha1_160,            160, SHA1_VERIF, "sha1-160",     "SHA1", GOOD},
  { sha1_32a,             32, SHA1a_VERIF,"sha1_32a",     "SHA1, low 32 bits", POOR},
  { sha2_224,            224, 0x60424E90, "sha2-224",     "SHA2-224", GOOD },
  { sha2_224_64,          64, 0x7EF6BB61, "sha2-224_64",  "SHA2-224, low 64 bits", GOOD },
  { sha2_256,            256, 0xACFA0A78, "sha2-256",     "SHA2-256", POOR },
  { sha2_256_64,          64, 0xA6C2C1D4, "sha2-256_64",  "SHA2-256, low 64 bits", POOR },
#if defined(HAVE_SHANI) && defined(__x86_64__)
  { sha1ni,              160, 0x0B01A4A1, "sha1ni",       "SHA1_NI (amd64 HW SHA ext)", POOR },
  { sha1ni_32,            32, 0xE70686CC, "sha1ni_32",    "hardened SHA1_NI (amd64 HW SHA ext), low 32 bits", GOOD },
  { sha2ni_256,          256, 0xAA94D6CD, "sha2ni-256",   "SHA2_NI-256 (amd64 HW SHA ext)", POOR },
  { sha2ni_256_64,        64, 0xF938E80E, "sha2ni-256_64","hardened SHA2_NI-256 (amd64 HW SHA ext), low 64 bits", POOR },
#endif
  { rmd128,              128, 0xFF576977, "rmd128",       "RIPEMD-128", GOOD },
  { rmd160,              160, 0x30B37AC6, "rmd160",       "RIPEMD-160", GOOD },
  { rmd256,              256, 0xEB16FAD7, "rmd256",       "RIPEMD-256", GOOD },
#if defined(HAVE_BIT32) && !defined(_WIN32)
#  define BLAKE3_VERIF   0x5A11C03C
#else
#  define BLAKE3_VERIF   0x170AB674
#endif
  { blake3c_test,        256, BLAKE3_VERIF, "blake3_c",   "BLAKE3 c",    GOOD },
#if defined(HAVE_BLAKE3)
  { blake3_test,         256, 0x00000000, "blake3",       "BLAKE3 Rust", GOOD },
  { blake3_64,            64, 0x00000000, "blake3_64",    "BLAKE3 Rust, low 64 bits", GOOD },
#endif
  { blake2s128_test,     128, 0xC0EF86D1, "blake2s-128",  "blake2s-128", GOOD },
  { blake2s160_test,     160, 0xE56D3359, "blake2s-160",  "blake2s-160", GOOD },
  { blake2s224_test,     224, 0x1C56E1A2, "blake2s-224",  "blake2s-224", GOOD },
  { blake2s256_test,     256, 0x846611DB, "blake2s-256",  "blake2s-256", GOOD },
  { blake2s256_64,        64, 0x2521E50B, "blake2s-256_64","blake2s-256, low 64 bits", GOOD },
  { blake2b160_test,     160, 0xA5F72E2D, "blake2b-160",  "blake2b-160", GOOD },
  { blake2b224_test,     224, 0x0D95F0AE, "blake2b-224",  "blake2b-224", GOOD },
  { blake2b256_test,     256, 0xC0B0AD0C, "blake2b-256",  "blake2b-256", POOR },
  { blake2b256_64,        64, 0x3C59D62D, "blake2b-256_64","blake2b-256, low 64 bits", GOOD },
  { sha3_256,            256, 0xB85F6DD9, "sha3-256",     "SHA3-256 (Keccak)", GOOD },
  { sha3_256_64,          64, 0x86EC71EF, "sha3-256_64",  "SHA3-256 (Keccak), low 64 bits", GOOD },

#ifdef __SSE2__
  { hasshe2_test,        256, 0xF5D39DFE, "hasshe2",     "SSE2 hasshe2, 256-bit", POOR },
#endif
#if defined(__SSE4_2__) && defined(__x86_64__)
  /* Even 32 uses crc32q, quad only */
  { crc32c_hw_test,       32, 0x0C7346F0, "crc32_hw",    "SSE4.2 crc32 in HW", POOR },
  { crc32c_hw1_test,      32, 0x0C7346F0, "crc32_hw1",   "Faster Adler SSE4.2 crc32 in HW", POOR },
  { crc64c_hw_test,       64, 0xE7C3FD0E, "crc64_hw",    "SSE4.2 crc64 in HW", POOR },
#endif
  // 32bit crashes
#if defined(HAVE_CLMUL) && !defined(_MSC_VER) && defined(__x86_64__)
  { crc32c_pclmul_test,   32, 0x00000000, "crc32_pclmul","-mpclmul crc32 in asm on HW", POOR },
#endif
#ifdef HAVE_INT64
  { o1hash_test,          64, 0x85051E87, "o1hash",       "o(1)hash unseeded, from wyhash", POOR },
#endif
#if 0 && defined(__x86_64__) && (defined(__linux__) || defined(__APPLE__))
  // elf64 or macho64 only
  { fhtw_test,            64, 0x0,        "fhtw",        "fhtw asm", POOR },
#endif
  { fibonacci_test, __WORDSIZE, FIBONACCI_VERIF, "fibonacci",   "wordwise Fibonacci", POOR },
  { FNV32a_test,          32, 0xE3CBBE91, "FNV1a",       "Fowler-Noll-Vo hash, 32-bit", POOR },
#ifdef HAVE_INT64
  { FNV1A_Totenschiff_test,32,0x95D95ACF, "FNV1A_Totenschiff",  "FNV1A_Totenschiff_v1 64-bit sanmayce", POOR },
  { FNV1A_PY_test,        32, 0xE79AE3E4, "FNV1A_Pippip_Yurii", "FNV1A-Pippip_Yurii 32-bit sanmayce", POOR },
  { FNV32a_YT_test,       32, 0xD8AFFD71, "FNV1a_YT",    "FNV1a-YoshimitsuTRIAD 32-bit sanmayce", POOR },
  { FNV64a_test,          64, 0x103455FC, "FNV64",       "Fowler-Noll-Vo hash, 64-bit", POOR },
#endif
  { FNV2_test,    __WORDSIZE, FNV2_VERIF, "FNV2",        "wordwise FNV", POOR },
  { fletcher2_test,       64, 0x890767C0, "fletcher2",   "fletcher2 ZFS", POOR},
  { fletcher4_test,       64, 0x890767C0, "fletcher4",   "fletcher4 ZFS", POOR},
  { Bernstein_test,       32, 0xBDB4B640, "bernstein",   "Bernstein, 32-bit", POOR },
  { sdbm_test,            32, 0x582AF769, "sdbm",        "sdbm as in perl5", POOR },
  { x17_test,             32, 0x8128E14C, "x17",         "x17", POOR },
  // also called jhash:
  { JenkinsOOAT_test,     32, 0x83E133DA, "JenkinsOOAT", "Bob Jenkins' OOAT as in perl 5.18", POOR },
  { JenkinsOOAT_perl_test,32, 0xEE05869B, "JenkinsOOAT_perl", "Bob Jenkins' OOAT as in old perl5", POOR },
  // FIXME: seed
  { VHASH_32,             32, 0xF0077651, "VHASH_32",    "VHASH_32 by Ted Krovetz and Wei Dai", POOR },
  { VHASH_64,             64, 0xF97D84FE, "VHASH_64",    "VHASH_64 by Ted Krovetz and Wei Dai", POOR },
  { MicroOAAT_test,       32, 0x16F1BA97, "MicroOAAT",   "Small non-multiplicative OAAT (by funny-falcon)", POOR },
  { farsh32_test,         32, 0xBCDE332C, "farsh32",     "FARSH 32bit", POOR }, // insecure
  { farsh64_test,         64, 0xDE2FDAEE, "farsh64",     "FARSH 64bit", POOR }, // insecure
  //{ farsh128_test,     128, 0x82B6CBEC, "farsh128",    "FARSH 128bit", POOR },
  //{ farsh256_test,     256, 0xFEBEA0BC, "farsh256",    "FARSH 256bit", POOR },
  { jodyhash32_test,      32, 0xFB47D60D, "jodyhash32",  "jodyhash, 32-bit (v5)", POOR },
#ifdef HAVE_INT64
  { jodyhash64_test,      64, 0x9F09E57F, "jodyhash64",  "jodyhash, 64-bit (v5)", POOR },
#endif
  { lookup3_test,         32, 0x3D83917A, "lookup3",     "Bob Jenkins' lookup3", POOR },
  { SuperFastHash_test,   32, 0xC4CB7C07, "superfast",   "Paul Hsieh's SuperFastHash", POOR },
  { beamsplitter_64,      64, 0x1BDF358B, "beamsplitter","A possibly universal hash made with a 10x64 s-box.", GOOD },
  { BEBB4185_64,          64, 0xBEBB4185, "BEBB4185",    "BEBB4185 64", GOOD },
  { MurmurOAAT_test,      32, 0x5363BD98, "MurmurOAAT",  "Murmur one-at-a-time", POOR },
  { Crap8_test,           32, 0x743E97A1, "Crap8",       "Crap8", POOR },
  { xxHash32_test,        32, 0xBA88B743, "xxHash32",    "xxHash, 32-bit for x86", POOR },
  { MurmurHash2_test,     32, 0x27864C1E, "Murmur2",     "MurmurHash2 for x86, 32-bit", POOR },
  { MurmurHash2A_test,    32, 0x7FBD4396, "Murmur2A",    "MurmurHash2A for x86, 32-bit", POOR },
#if __WORDSIZE >= 64
  { MurmurHash64A_test,   64, 0x1F0D3804, "Murmur2B",    "MurmurHash64A for x64, 64-bit", POOR },
#endif
#ifdef HAVE_INT64
  { MurmurHash64B_test,   64, 0xDD537C05, "Murmur2C",    "MurmurHash64B for x86, 64-bit", POOR },
#endif
  { MurmurHash3_x86_32,   32, 0xB0F57EE3, "Murmur3A",    "MurmurHash3 for x86, 32-bit", POOR },
  { PMurHash32_test,      32, 0xB0F57EE3, "PMurHash32",  "Shane Day's portable-ized MurmurHash3 for x86, 32-bit", POOR },
  { MurmurHash3_x86_128, 128, 0xB3ECE62A, "Murmur3C",    "MurmurHash3 for x86, 128-bit", POOR },
#ifndef DEBUG
# ifndef HAVE_ASAN
  // TODO seeded
  { PMPML_32_CPP,         32, 0xEAE2E3CC, "PMPML_32",    "PMP_Multilinear 32-bit unseeded", POOR },
#  if defined(_WIN64) || defined(__x86_64__)
  { PMPML_64_CPP,         64, 0x584CC9DF, "PMPML_64",    "PMP_Multilinear 64-bit unseeded", POOR },
#  endif
# endif
#endif
  { fasthash64_test,      64, 0xA16231A7, "fasthash64",  "fast-hash 64bit", POOR },
  { CityHash32_test,      32, 0x5C28AD62, "City32",      "Google CityHash32WithSeed (old)", POOR },
#ifdef HAVE_INT64
  { metrohash64_test,      64, 0x6FA828C9, "metrohash64",    "MetroHash64, 64-bit", POOR },
  { metrohash64_1_test,    64, 0xEE88F7D2, "metrohash64_1",  "MetroHash64_1, 64-bit (legacy)", POOR },
  { metrohash64_2_test,    64, 0xE1FC7C6E, "metrohash64_2",  "MetroHash64_2, 64-bit (legacy)", GOOD },
  { metrohash128_test,    128, 0x4A6673E7, "metrohash128",   "MetroHash128, 128-bit", GOOD },
  { metrohash128_1_test,  128, 0x20E8A1D7, "metrohash128_1", "MetroHash128_1, 128-bit (legacy)", GOOD },
  { metrohash128_2_test,  128, 0x5437C684, "metrohash128_2", "MetroHash128_2, 128-bit (legacy)", GOOD },
#endif
#if defined(__SSE4_2__) && defined(__x86_64__)
  { metrohash64crc_1_test, 64, 0x29C68A50, "metrohash64crc_1", "MetroHash64crc_1 for x64 (legacy)", POOR },
  { metrohash64crc_2_test, 64, 0x2C00BD9F, "metrohash64crc_2", "MetroHash64crc_2 for x64 (legacy)", POOR },
  { cmetrohash64_1_optshort_test,64, 0xEE88F7D2, "cmetrohash64_1o", "cmetrohash64_1 (shorter key optimized), 64-bit for x64", POOR },
  { cmetrohash64_1_test,   64, 0xEE88F7D2, "cmetrohash64_1",  "cmetrohash64_1, 64-bit for x64", POOR },
  { cmetrohash64_2_test,   64, 0xE1FC7C6E, "cmetrohash64_2",  "cmetrohash64_2, 64-bit for x64", GOOD },
  { metrohash128crc_1_test,128, 0x5E75144E, "metrohash128crc_1", "MetroHash128crc_1 for x64 (legacy)", GOOD },
  { metrohash128crc_2_test,128, 0x1ACF3E77, "metrohash128crc_2", "MetroHash128crc_2 for x64 (legacy)", GOOD },
#endif
  { CityHash64noSeed_test, 64, 0x63FC6063, "City64noSeed","Google CityHash64 without seed (default version, misses one final avalanche)", POOR },
  { CityHash64_test,      64, 0x25A20825, "City64",       "Google CityHash64WithSeed (old)", POOR },
#if defined(HAVE_AESNI) && !defined(_MSC_VER)
  { aesnihash_test,       64, 0x0,        "aesnihash",    "majek's unseeded aesnihash with aesenc, 64-bit for x64", POOR },
#endif
#if defined(__SSE4_2__) && defined(__x86_64__)
  { falkhash_test_cxx,    64, 0x2F99B071, "falkhash",    "falkhash.asm with aesenc, 64-bit for x64", POOR },
#endif
  { t1ha1_64le_test,      64, 0xD6836381, "t1ha1_64le",  "Fast Positive Hash (portable, aims 64-bit, little-endian)", POOR },
  { t1ha1_64be_test,      64, 0x93F864DE, "t1ha1_64be",  "Fast Positive Hash (portable, aims 64-bit, big-engian)", POOR },
  { t1ha0_32le_test,      64, 0x7F7D7B29, "t1ha0_32le",  "Fast Positive Hash (portable, aims 32-bit, little-endian)", POOR },
  { t1ha0_32be_test,      64, 0xDA6A4061, "t1ha0_32be",  "Fast Positive Hash (portable, aims 32-bit, big-endian)", POOR },

  { xxh3_test,            64, 0x39CD9E4A, "xxh3",        "xxHash v3, 64-bit", GOOD },
  { xxh3low_test,         32, 0xFAE8467B, "xxh3low",     "xxHash v3, 64-bit, low 32-bits part", GOOD },
  { xxh128_test,         128, 0xEB61B3A0, "xxh128",      "xxHash v3, 128-bit", GOOD },
  { xxh128low_test,       64, 0x54D1CC70, "xxh128low",   "xxHash v3, 128-bit, low 64-bits part", GOOD },

#if __WORDSIZE >= 64
# define TIFU_VERIF       0x644236D4
#else
  // broken on certain travis
# define TIFU_VERIF       0x0
#endif
  // and now the quality hash funcs, slowest first
  { tifuhash_64,          64, TIFU_VERIF, "tifuhash_64", "Tiny Floatingpoint Unique Hash with continued egyptian fractions", POOR },
  // different verif on gcc vs clang
  { floppsyhash_64,       64, 0x0,        "floppsyhash", "slow hash designed for floating point hardware", GOOD },
  { chaskey_test,         64, 0x81A90131, "chaskey",     "mouha.be/chaskey/ with added seed support", GOOD },
  { siphash_test,         64, 0xC58D7F9C, "SipHash",     "SipHash 2-4 - SSSE3 optimized", GOOD },
  { halfsiphash_test,     32, 0xA7A05F72, "HalfSipHash", "HalfSipHash 2-4, 32bit", GOOD },
  { GoodOAAT_test,        32, 0x7B14EEE5, "GoodOAAT",    "Small non-multiplicative OAAT", GOOD },
  // as in rust and swift:
  { siphash13_test,       64, 0x29C010BF, "SipHash13",   "SipHash 1-3 - SSSE3 optimized", GOOD },
#ifndef _MSC_VER
  { tsip_test,            64, 0x8E48155B, "TSip",        "Damian Gryski's Tiny SipHash variant", GOOD },
#ifdef HAVE_INT64
  { seahash_test,         64, 0xF0374078, "seahash",     "seahash (64-bit, little-endian)", GOOD },
  { seahash32low,         32, 0x712F0EE8, "seahash32low","seahash - lower 32bit", GOOD },
#endif /* HAVE_INT64 */
#endif /* !MSVC */
#if defined(__SSE4_2__) && defined(__x86_64__)
  { clhash_test,          64, 0x00000000, "clhash",      "carry-less mult. hash -DBITMIX (64-bit for x64, SSE4.2)", GOOD },
#endif
#ifdef HAVE_HIGHWAYHASH
  { HighwayHash64_test,   64, 0x00000000,        "HighwayHash64", "Google HighwayHash (portable with dylib overhead)", GOOD },
#endif
#if __WORDSIZE >= 64
  { MurmurHash3_x64_128, 128, 0x6384BA69, "Murmur3F",    "MurmurHash3 for x64, 128-bit", GOOD },
#endif
  { fasthash32_test,      32, 0xE9481AFC, "fasthash32",  "fast-hash 32bit", GOOD },
#if defined(__GNUC__) && UINT_MAX != ULONG_MAX
 #define MUM_VERIF            0x3EEAE2D4
 #define MUMLOW_VERIF         0x520263F5
#else
 #define MUM_VERIF            0xA973C6C0
 #define MUMLOW_VERIF         0x7F898826
#endif
  { mum_hash_test,        64, MUM_VERIF,  "MUM",         "github.com/vnmakarov/mum-hash", GOOD },
  { mum_low_test,         32, MUMLOW_VERIF,"MUMlow",     "github.com/vnmakarov/mum-hash", GOOD },
#if defined(__GNUC__) && UINT_MAX != ULONG_MAX
 #define MIR_VERIF            0x00A393C8
 #define MIRLOW_VERIF         0xE320CE68
#else
 #define MIR_VERIF            0x422A66FC
 #define MIRLOW_VERIF         0xD50D1F09
#endif
#ifdef HAVE_INT64
  // improved MUM:
  { mirhash_test,         64, MIR_VERIF,    "mirhash",            "mirhash", GOOD },
  { mirhash32low,         32, MIRLOW_VERIF, "mirhash32low",       "mirhash - lower 32bit", GOOD },
  { mirhashstrict_test,   64, 0x422A66FC,   "mirhashstrict",      "mirhashstrict (portable, 64-bit, little-endian)", GOOD },
  { mirhashstrict32low,   32, 0xD50D1F09,   "mirhashstrict32low", "mirhashstrict - lower 32bit", POOR },
#endif
  { CityHash64_low_test,  32, 0xCC5BC861, "City64low",   "Google CityHash64WithSeed (low 32-bits)", GOOD },
#if defined(__SSE4_2__) && defined(__x86_64__)
  { CityHash128_test,    128, 0x6531F54E, "City128",     "Google CityHash128WithSeed (old)", GOOD },
  { CityHashCrc128_test, 128, 0xD4389C97, "CityCrc128",  "Google CityHashCrc128WithSeed SSE4.2 (old)", GOOD },
#endif

#ifdef __FreeBSD__
#  define FARM64_VERIF        0x0
#  define FARM128_VERIF       0x0
#else
#  define FARM64_VERIF        0xEBC4A679
#  define FARM128_VERIF       0x305C0D9A
#endif
  { FarmHash32_test,      32, 0/*0x2E226C14*/,   "FarmHash32",  "Google FarmHash32WithSeed", GOOD },
  { FarmHash64_test,      64, FARM64_VERIF, "FarmHash64",  "Google FarmHash64WithSeed", GOOD },
 //{ FarmHash64noSeed_test,64, 0xA5B9146C,  "Farm64noSeed","Google FarmHash64 without seed (default, misses on final avalanche)", POOR },
  { FarmHash128_test,    128, FARM128_VERIF,"FarmHash128", "Google FarmHash128WithSeed", GOOD },
#if defined(__SSE4_2__) && defined(__x86_64__)
  { farmhash32_c_test,    32, 0/*0xA2E45238*/,   "farmhash32_c", "farmhash32_with_seed (C99)", GOOD },
  { farmhash64_c_test,    64, FARM64_VERIF, "farmhash64_c",  "farmhash64_with_seed (C99)", GOOD },
  { farmhash128_c_test,  128, FARM128_VERIF,"farmhash128_c", "farmhash128_with_seed (C99)", GOOD },
#endif

  { xxHash64_test,        64, 0x024B7CF4, "xxHash64",    "xxHash, 64-bit", GOOD },
#if 0
  { xxhash256_test,       64, 0x024B7CF4, "xxhash256",   "xxhash256, 64-bit unportable", GOOD },
#endif
  { SpookyHash32_test,    32, 0x3F798BBB, "Spooky32",    "Bob Jenkins' SpookyHash, 32-bit result", GOOD },
  { SpookyHash64_test,    64, 0xA7F955F1, "Spooky64",    "Bob Jenkins' SpookyHash, 64-bit result", GOOD },
  { SpookyHash128_test,  128, 0x8D263080, "Spooky128",   "Bob Jenkins' SpookyHash, 128-bit result", GOOD },
  { t1ha2_atonce_test,           64, 0x8F16C948, "t1ha2_atonce",    "Fast Positive Hash (portable, aims 64-bit, little-endian)", GOOD },
  { t1ha2_stream_test,           64, 0xDED9B580, "t1ha2_stream",    "Fast Positive Hash (portable, aims 64-bit, little-endian)", POOR },
  { t1ha2_atonce128_test,       128, 0xB44C43A1, "t1ha2_atonce128", "Fast Positive Hash (portable, aims 64-bit, little-endian)", GOOD },
  { t1ha2_stream128_test,       128, 0xE929E756, "t1ha2_stream128", "Fast Positive Hash (portable, aims 64-bit, little-endian)", POOR },
#if T1HA0_AESNI_AVAILABLE
#  ifndef _MSC_VER
  { t1ha0_ia32aes_noavx_test,  64, 0xF07C4DA5, "t1ha0_aes_noavx", "Fast Positive Hash (machine-specific, requires AES-NI)", GOOD },
#  endif
#  if defined(__AVX__)
  { t1ha0_ia32aes_avx1_test,   64, 0xF07C4DA5, "t1ha0_aes_avx1",  "Fast Positive Hash (machine-specific, requires AES-NI & AVX)", GOOD },
#  endif /* __AVX__ */
#  if defined(__AVX2__)
  { t1ha0_ia32aes_avx2_test,   64, 0x8B38C599, "t1ha0_aes_avx2",  "Fast Positive Hash (machine-specific, requires AES-NI & AVX2)", GOOD },
#  endif /* __AVX2__ */
#endif /* T1HA0_AESNI_AVAILABLE */
#ifdef HAVE_BIT32
  { wyhash32_test,         32, 0x09DE8066, "wyhash32",       "wyhash (32-bit)", GOOD },
#else
  { wyhash32low,           32, 0xDCFB4E8F, "wyhash32low",    "wyhash lower 32bit", GOOD },
#endif
#ifdef HAVE_INT64
  { wyhash_test,           64, 0x8323EB7E, "wyhash",         "wyhash (64-bit)", GOOD },
#endif

};

HashInfo * findHash ( const char * name )
{
  for(size_t i = 0; i < sizeof(g_hashes) / sizeof(HashInfo); i++)
  {
    if(_stricmp(name,g_hashes[i].name) == 0)
      return &g_hashes[i];
  }

  return NULL;
}

// optional hash state initializers
void Hash_init (HashInfo* info) {
  if (info->hash == sha2_224_64)
    sha224_init(&ltc_state);
  else if (info->hash == rmd128)
    rmd128_init(&ltc_state);
#if defined(__SSE4_2__) && defined(__x86_64__)
  else if(info->hash == clhash_test)
    clhash_init();
#endif
  else if (info->hash == VHASH_32 || info->hash == VHASH_64)
    VHASH_init();
#ifdef HAVE_HIGHWAYHASH
  else if(info->hash == HighwayHash64_test)
    HighwayHash_init();
#endif
#ifndef _MSC_VER
  else if(info->hash == tsip_test)
    tsip_init();
#endif
  else if(info->hash == chaskey_test)
    chaskey_init();
}

//-----------------------------------------------------------------------------
// Self-test on startup - verify that all installed hashes work correctly.

void SelfTest(bool verbose) {
  bool pass = true;
  for (size_t i = 0; i < sizeof(g_hashes) / sizeof(HashInfo); i++) {
    HashInfo *info = &g_hashes[i];
    if (verbose)
      printf("%20s - ", info->name);
    pass &= VerificationTest(info->hash, info->hashbits, info->verification,
                             verbose);
  }

  if (!pass) {
    printf("Self-test FAILED!\n");
    if (!verbose) {
      for (size_t i = 0; i < sizeof(g_hashes) / sizeof(HashInfo); i++) {
        HashInfo *info = &g_hashes[i];
        printf("%20s - ", info->name);
        pass &= VerificationTest(info->hash, info->hashbits, info->verification,
                                 true);
      }
    }
    exit(1);
  }
}

//----------------------------------------------------------------------------

template < typename hashtype >
void test ( hashfunc<hashtype> hash, HashInfo* info )
{
  const int hashbits = sizeof(hashtype) * 8;

  if (g_testAll) {
    printf("-------------------------------------------------------------------------------\n");
  }

  // eventual initializers
  Hash_init (info);

  //-----------------------------------------------------------------------------
  // Sanity tests

  if(g_testVerifyAll)
  {
    printf("[[[ VerifyAll Tests ]]]\n\n"); fflush(NULL);
    SelfTest(g_drawDiagram);
    printf("PASS\n\n"); fflush(NULL); // if not it does exit(1)
  }

  if (g_testAll || g_testSpeed || g_testHashmap) {
    printf("--- Testing %s \"%s\" %s\n\n", info->name, info->desc, quality_str[info->quality]);
  } else {
    fprintf(stderr, "--- Testing %s \"%s\" %s\n\n", info->name, info->desc, quality_str[info->quality]);
  }
  fflush(NULL);

  // sha1_32a runs 30s
  if(g_testSanity || g_testAll)
  {
    printf("[[[ Sanity Tests ]]]\n\n");
    fflush(NULL);

    VerificationTest(hash,hashbits,info->verification,true);
    SanityTest(hash,hashbits);
    AppendedZeroesTest(hash,hashbits);
    printf("\n");
    fflush(NULL);
  }

  //-----------------------------------------------------------------------------
  // Speed tests

  if(g_testSpeed || g_testAll)
  {
    double sum = 0.0;
    printf("[[[ Speed Tests ]]]\n\n");
    fflush(NULL);

    BulkSpeedTest(info->hash,info->verification);
    printf("\n");
    fflush(NULL);

    for(int i = 1; i < 32; i++)
    {
      sum += TinySpeedTest(hashfunc<hashtype>(info->hash),sizeof(hashtype),i,info->verification,true);
    }
    g_speed = sum = sum / 31.0;
    printf("Average                                    %6.3f cycles/hash\n",sum);
    printf("\n");
    fflush(NULL);
  } else {
    // known slow hashes (> 500), cycle/hash
    const struct { pfHash h; double cycles; } speeds[] =
    {{ multiply_shift,    50.50 },
     { pair_multiply_shift,31.71},
     { md5_32,           670.99 },
     { md5_128,          730.30 },
     { sha1_32a,        1385.80 },
     { sha1_160,        1470.55 },
     { sha2_224,        1354.81 },
     { sha2_224_64,     1360.10 },
     { sha2_256,        1374.90 },
     { sha2_256_64,     1376.34 },
     { rmd128,           672.35 },
     { rmd160,          1045.79 },
     { rmd256,           638.30 },
     { blake2s128_test,  698.09 },
     { blake2s160_test, 1026.74 },
     { blake2s224_test, 1063.86 },
     { blake2s256_test, 1014.88 },
     { blake2s256_64,   1014.88 },
     { blake2b160_test, 1236.84 },
     { blake2b224_test, 1228.50 },
     { blake2b256_test, 1232.22 },
     { blake2b256_64,   1236.84 },
     { sha3_256,        3877.18 },
     { sha3_256_64,     3909.00 },
     { tifuhash_64,     1679.52 }
    };
    for (int i=0; i<sizeof(speeds)/sizeof(speeds[0]); i++) {
      if (speeds[i].h == hash)
        {
          g_speed = speeds[i].cycles; break;
        }
    }
  }

  // sha1_32a runs 30s
  if(g_testHashmap || g_testAll)
  {
    printf("[[[ 'Hashmap' Speed Tests ]]]\n\n");
    fflush(NULL);
    int trials = 50;
    if ((g_speed > 500 /*|| hash == multiply_shift || hash == pair_multiply_shift*/ )
         && !g_testExtra)
      trials = 5;
    bool result = true;
    if (info->quality == SKIP) {
      result = false;
    } else {
      std::vector<std::string> words = HashMapInit(g_drawDiagram);
      result &= HashMapTest(hash,info->hashbits,words,trials,g_drawDiagram);
    }
    if(!result) printf("*********FAIL*********\n");
    printf("\n");
    fflush(NULL);
  }

  //-----------------------------------------------------------------------------
  // Avalanche tests
  // 1m30 for xxh3
  // 13m  for xxh3 with --extra
  // 3m   for farmhash128_c (was 7m with 512,1024)

  if(g_testAvalanche || g_testAll)
  {
    printf("[[[ Avalanche Tests ]]]\n\n");
    fflush(NULL);

    bool result = true;
    bool verbose = g_drawDiagram; //.......... progress dots

    result &= AvalancheTest< Blob< 24>, hashtype > (hash,300000,verbose);
    result &= AvalancheTest< Blob< 32>, hashtype > (hash,300000,verbose);
    result &= AvalancheTest< Blob< 40>, hashtype > (hash,300000,verbose);
    result &= AvalancheTest< Blob< 48>, hashtype > (hash,300000,verbose);
    result &= AvalancheTest< Blob< 56>, hashtype > (hash,300000,verbose);
    result &= AvalancheTest< Blob< 64>, hashtype > (hash,300000,verbose);
    result &= AvalancheTest< Blob< 72>, hashtype > (hash,300000,verbose);
    result &= AvalancheTest< Blob< 80>, hashtype > (hash,300000,verbose);

    result &= AvalancheTest< Blob< 96>, hashtype > (hash,300000,verbose);
    result &= AvalancheTest< Blob<112>, hashtype > (hash,300000,verbose);
    result &= AvalancheTest< Blob<128>, hashtype > (hash,300000,verbose);
    result &= AvalancheTest< Blob<160>, hashtype > (hash,300000,verbose);

    if(g_testExtra) {
      result &= AvalancheTest< Blob<192>, hashtype > (hash,300000,verbose);
      result &= AvalancheTest< Blob<224>, hashtype > (hash,300000,verbose);
      result &= AvalancheTest< Blob<256>, hashtype > (hash,300000,verbose);

      result &= AvalancheTest< Blob<320>, hashtype > (hash,300000,verbose);
      result &= AvalancheTest< Blob<384>, hashtype > (hash,300000,verbose);
      result &= AvalancheTest< Blob<448>, hashtype > (hash,300000,verbose);
    }
    if (g_testExtra || info->hashbits <= 64) {
      result &= AvalancheTest< Blob<512>, hashtype > (hash,300000,verbose);
    }
    if(g_testExtra) {
      result &= AvalancheTest< Blob<640>, hashtype > (hash,300000,verbose);
      result &= AvalancheTest< Blob<768>, hashtype > (hash,300000,verbose);
      result &= AvalancheTest< Blob<896>, hashtype > (hash,300000,verbose);
    }
    if (g_testExtra || info->hashbits <= 64) {
      result &= AvalancheTest< Blob<1024>,hashtype > (hash,300000,verbose);
    }
    if(g_testExtra) {
      result &= AvalancheTest< Blob<1280>,hashtype > (hash,300000,verbose);
      result &= AvalancheTest< Blob<1536>,hashtype > (hash,300000,verbose);
    }

    if(!result) printf("*********FAIL*********\n");
    printf("\n");
    fflush(NULL);
  }

  //-----------------------------------------------------------------------------
  // Keyset 'Sparse' - keys with all bits 0 except a few
  // 3m30 for xxh3
  // 14m  for xxh3 with --extra
  // 6m30 for farmhash128_c (was too much with >= 512)

  if(g_testSparse || g_testAll)
  {
    printf("[[[ Keyset 'Sparse' Tests ]]]\n\n");
    fflush(NULL);

    bool result = true;

      result &= SparseKeyTest<  16,hashtype>(hash,9,true,true,true, g_drawDiagram);
      result &= SparseKeyTest<  24,hashtype>(hash,8,true,true,true, g_drawDiagram);
      result &= SparseKeyTest<  32,hashtype>(hash,7,true,true,true, g_drawDiagram);
      result &= SparseKeyTest<  40,hashtype>(hash,6,true,true,true, g_drawDiagram);
      result &= SparseKeyTest<  48,hashtype>(hash,6,true,true,true, g_drawDiagram);
      result &= SparseKeyTest<  56,hashtype>(hash,5,true,true,true, g_drawDiagram);
      result &= SparseKeyTest<  64,hashtype>(hash,5,true,true,true, g_drawDiagram);
      result &= SparseKeyTest<  72,hashtype>(hash,5,true,true,true, g_drawDiagram);
      result &= SparseKeyTest<  96,hashtype>(hash,4,true,true,true, g_drawDiagram);
    if (g_testExtra) {
      result &= SparseKeyTest< 112,hashtype>(hash,4,true,true,true, g_drawDiagram);
      result &= SparseKeyTest< 128,hashtype>(hash,4,true,true,true, g_drawDiagram);
      result &= SparseKeyTest< 144,hashtype>(hash,4,true,true,true, g_drawDiagram);
    }
      result &= SparseKeyTest< 160,hashtype>(hash,4,true,true,true, g_drawDiagram);
    if (g_testExtra) {
      result &= SparseKeyTest< 192,hashtype>(hash,4,true,true,true, g_drawDiagram);
    }
      result &= SparseKeyTest< 256,hashtype>(hash,3,true,true,true, g_drawDiagram);
    if (g_testExtra) {
      result &= SparseKeyTest< 288,hashtype>(hash,3,true,true,true, g_drawDiagram);
      result &= SparseKeyTest< 320,hashtype>(hash,3,true,true,true, g_drawDiagram);
      result &= SparseKeyTest< 384,hashtype>(hash,3,true,true,true, g_drawDiagram);
      result &= SparseKeyTest< 448,hashtype>(hash,3,true,true,true, g_drawDiagram);
    } else {
      if (info->hashbits > 64) //too long
        goto END_Sparse;
    }
      result &= SparseKeyTest< 512,hashtype>(hash,3,true,true,true, g_drawDiagram);
    if (g_testExtra) {
      result &= SparseKeyTest< 640,hashtype>(hash,3,true,true,true, g_drawDiagram);
      result &= SparseKeyTest< 768,hashtype>(hash,3,true,true,true, g_drawDiagram);
      result &= SparseKeyTest< 896,hashtype>(hash,2,true,true,true, g_drawDiagram);
    }
      result &= SparseKeyTest<1024,hashtype>(hash,2,true,true,true, g_drawDiagram);
    if (g_testExtra) {
      result &= SparseKeyTest<1280,hashtype>(hash,2,true,true,true, g_drawDiagram);
      result &= SparseKeyTest<1536,hashtype>(hash,2,true,true,true, g_drawDiagram);
    }
      result &= SparseKeyTest<2048,hashtype>(hash,2,true,true,true, g_drawDiagram);
    if (g_testExtra) {
      result &= SparseKeyTest<3072,hashtype>(hash,2,true,true,true, g_drawDiagram);
      result &= SparseKeyTest<4096,hashtype>(hash,2,true,true,true, g_drawDiagram);
      result &= SparseKeyTest<6144,hashtype>(hash,2,true,true,true, g_drawDiagram);
      result &= SparseKeyTest<8192,hashtype>(hash,2,true,true,true, g_drawDiagram);
      result &= SparseKeyTest<9992,hashtype>(hash,2,true,true,true, g_drawDiagram);
    }
  END_Sparse:
    if(!result) printf("*********FAIL*********\n");
    printf("\n");
    fflush(NULL);
  }

  //-----------------------------------------------------------------------------
  // Keyset 'Permutation' - all possible combinations of a set of blocks
  // 9m with xxh3 and maxlen=23, 4m15 with maxlen=22
  // 120m for farmhash128_c with maxlen=18, 1m20 FAIL with maxlen=12
  //                                        1m20 PASS with maxlen=14,16,17

  if(g_testPermutation || g_testAll)
  {
    const int maxlen = g_testExtra
      ? 23
      : info->hashbits > 64
         ? 17
         : 22;

    {
      // This one breaks lookup3, surprisingly
      printf("[[[ Keyset 'Permutation' Tests ]]]\n\n");
      printf("Combination Lowbits Tests:\n");
      fflush(NULL);

      bool result = true;

      uint32_t blocks[] = { 0, 1, 2, 3, 4, 5, 6, 7 };

      result &= CombinationKeyTest<hashtype>(hash,7,blocks,
                                             sizeof(blocks) / sizeof(uint32_t),
                                             true,true, g_drawDiagram);

      if(!result) printf("*********FAIL*********\n");
      printf("\n");
      fflush(NULL);
    }

    {
      printf("Combination Highbits Tests\n");
      fflush(NULL);

      bool result = true;

      uint32_t blocks[] =
      {
        0x00000000,
        0x20000000, 0x40000000, 0x60000000, 0x80000000, 0xA0000000, 0xC0000000, 0xE0000000
      };

      result &= CombinationKeyTest(hash,7,blocks,sizeof(blocks) / sizeof(uint32_t),
                                   true,true, g_drawDiagram);

      if(!result) printf("*********FAIL*********\n");
      printf("\n");
      fflush(NULL);
    }

    {
      printf("Combination Hi-Lo Tests:\n");

      bool result = true;

      uint32_t blocks[] =
      {
        0x00000000,
        0x00000001, 0x00000002, 0x00000003, 0x00000004, 0x00000005, 0x00000006, 0x00000007,
        0x80000000, 0x40000000, 0xC0000000, 0x20000000, 0xA0000000, 0x60000000, 0xE0000000
      };

      result &= CombinationKeyTest<hashtype>(hash,6,blocks,sizeof(blocks) / sizeof(uint32_t),
                                             true,true, g_drawDiagram);

      if(!result) printf("*********FAIL*********\n");
      printf("\n");
      fflush(NULL);
    }

    {
      printf("Combination 0x8000000 Tests:\n");
      fflush(NULL);

      bool result = true;

      uint32_t blocks[] =
      {
        0x00000000,
        0x80000000,
      };

      result &= CombinationKeyTest(hash, maxlen, blocks, sizeof(blocks) / sizeof(uint32_t),
                                   true,true, g_drawDiagram);

      if(!result) printf("*********FAIL*********\n");
      printf("\n");
      fflush(NULL);
    }

    {
      printf("Combination 0x0000001 Tests:\n");

      bool result = true;

      uint32_t blocks[] =
      {
        0x00000000,
        0x00000001,
      };

      result &= CombinationKeyTest(hash, maxlen, blocks, sizeof(blocks) / sizeof(uint32_t),
                                   true, true, g_drawDiagram);

      if(!result) printf("*********FAIL*********\n");
      printf("\n");
      fflush(NULL);
    }

    {
      printf("Combination 0x800000000000000 Tests:\n");
      fflush(NULL);

      bool result = true;

      uint64_t blocks[] =
      {
        0x0000000000000000ULL,
        0x8000000000000000ULL,
      };

      result &= CombinationKeyTest(hash, maxlen, blocks, sizeof(blocks) / sizeof(uint64_t),
                                   true, true, g_drawDiagram);

      if(!result) printf("*********FAIL*********\n");
      printf("\n");
      fflush(NULL);
    }

    {
      printf("Combination 0x000000000000001 Tests:\n");
      fflush(NULL);

      bool result = true;

      uint64_t blocks[] =
      {
        0x0000000000000000ULL,
        0x0000000000000001ULL,
      };

      result &= CombinationKeyTest(hash, maxlen, blocks, sizeof(blocks) / sizeof(uint64_t),
                                   true, true, g_drawDiagram);

      if(!result) printf("*********FAIL*********\n");
      printf("\n");
      fflush(NULL);
    }

    {
      printf("Combination 16-bytes [0-1] Tests:\n");
      fflush(NULL);

      bool result = true;

      block16 blocks[2];
      memset(blocks, 0, sizeof(blocks));
      blocks[0].c[0] = 1;   // presumes little endian

      result &= CombinationKeyTest(hash, maxlen, blocks, 2, true, true, g_drawDiagram);

      if(!result) printf("*********FAIL*********\n");
      printf("\n");
      fflush(NULL);
    }

    {
      printf("Combination 16-bytes [0-last] Tests:\n");
      fflush(NULL);

      bool result = true;

      size_t const nbElts = 2;
      block16 blocks[nbElts];
      memset(blocks, 0, sizeof(blocks));
      blocks[0].c[sizeof(blocks[0].c)-1] = 0x80;   // presumes little endian

      result &= CombinationKeyTest(hash, maxlen, blocks, nbElts, true, true, g_drawDiagram);

      if(!result) printf("*********FAIL*********\n");
      printf("\n");
      fflush(NULL);
    }

    {
      printf("Combination 32-bytes [0-1] Tests:\n");
      fflush(NULL);

      bool result = true;

      block32 blocks[2];
      memset(blocks, 0, sizeof(blocks));
      blocks[0].c[0] = 1;   // presumes little endian

      result &= CombinationKeyTest(hash, maxlen, blocks, 2, true, true, g_drawDiagram);

      if(!result) printf("*********FAIL*********\n");
      printf("\n");
      fflush(NULL);
    }

    {
      printf("Combination 32-bytes [0-last] Tests:\n");
      fflush(NULL);

      bool result = true;

      size_t const nbElts = 2;
      block32 blocks[nbElts];
      memset(blocks, 0, sizeof(blocks));
      blocks[0].c[sizeof(blocks[0].c)-1] = 0x80;   // presumes little endian

      result &= CombinationKeyTest(hash, maxlen, blocks, nbElts, true, true, g_drawDiagram);

      if(!result) printf("*********FAIL*********\n");
      printf("\n");
      fflush(NULL);
    }

    {
      printf("Combination 64-bytes [0-1] Tests:\n");
      fflush(NULL);

      bool result = true;

      block64 blocks[2];
      memset(blocks, 0, sizeof(blocks));
      blocks[0].c[0] = 1;   // presumes little endian

      result &= CombinationKeyTest(hash, maxlen, blocks, 2, true, true, g_drawDiagram);

      if(!result) printf("*********FAIL*********\n");
      printf("\n");
      fflush(NULL);
    }

    {
      printf("Combination 64-bytes [0-last] Tests:\n");
      fflush(NULL);

      bool result = true;

      size_t const nbElts = 2;
      block64 blocks[nbElts];
      memset(blocks, 0, sizeof(blocks));
      blocks[0].c[sizeof(blocks[0].c)-1] = 0x80;   // presumes little endian

      result &= CombinationKeyTest(hash, maxlen, blocks, nbElts, true, true, g_drawDiagram);

      if(!result) printf("*********FAIL*********\n");
      printf("\n");
      fflush(NULL);
    }

    {
      printf("Combination 128-bytes [0-1] Tests:\n");
      fflush(NULL);

      bool result = true;

      block128 blocks[2];
      memset(blocks, 0, sizeof(blocks));
      blocks[0].c[0] = 1;   // presumes little endian

      result &= CombinationKeyTest(hash, maxlen, blocks, 2, true, true, g_drawDiagram);

      if(!result) printf("*********FAIL*********\n");
      printf("\n");
      fflush(NULL);
    }

    {
      printf("Combination 128-bytes [0-last] Tests:\n");
      fflush(NULL);

      bool result = true;

      size_t const nbElts = 2;
      block128 blocks[nbElts];
      memset(blocks, 0, sizeof(blocks));
      blocks[0].c[sizeof(blocks[0].c)-1] = 0x80;   // presumes little endian

      result &= CombinationKeyTest(hash, maxlen, blocks, nbElts, true, true, g_drawDiagram);

      if(!result) printf("*********FAIL*********\n");
      printf("\n");
      fflush(NULL);
    }

  }

  //-----------------------------------------------------------------------------
  // Keyset 'Window'

  // Skip distribution test for these - they're too easy to distribute well,
  // and it generates a _lot_ of testing.
  // 11s for crc32_hw, 28s for xxh3
  // 51s for crc32_hw --extra
  // 180m for farmhash128_c with 20 windowbits,
  //      0.19s with windowbits=10, 2s for 14, 9s for 16, 37s for 18
  // 7m for FNV64 with windowbits=27 / 32bit keys
  // 5m35 for hasshe2 with windowbits=25 / 32bit keys

  if(g_testWindow || g_testAll)
  {
    printf("[[[ Keyset 'Window' Tests ]]]\n\n");

    bool result = true;
    bool testCollision = true;
    bool testDistribution = g_testExtra;
    // This value is now adjusted to generate at least 0.5 collisions per window,
    // except for 64++bit where it unrealistic. There use smaller but more keys,
    // to get a higher collision percentage.
    int windowbits = 20;
    const int keybits = (hashbits >= 64) ? 32 : hashbits*2+2;

    result &= WindowedKeyTest< Blob<keybits>, hashtype >
      ( hash, windowbits, testCollision, testDistribution, g_drawDiagram );

    if(!result) printf("*********FAIL*********\n");
    printf("\n");
    fflush(NULL);
  }

  //-----------------------------------------------------------------------------
  // Keyset 'Cyclic' - keys of the form "abcdabcdabcd..."
  // 5s for crc32_hw
  // 18s for farmhash128_c

  if(g_testCyclic || g_testAll)
  {
    printf("[[[ Keyset 'Cyclic' Tests ]]]\n\n");
    fflush(NULL);
#ifdef DEBUG
    const int reps = 2;
#else
    const int reps = g_speed > 500.0 ? 100000 : 1000000;
#endif
    bool result = true;

    result &= CyclicKeyTest<hashtype>(hash,sizeof(hashtype)+0,8,reps, g_drawDiagram);
    result &= CyclicKeyTest<hashtype>(hash,sizeof(hashtype)+1,8,reps, g_drawDiagram);
    result &= CyclicKeyTest<hashtype>(hash,sizeof(hashtype)+2,8,reps, g_drawDiagram);
    result &= CyclicKeyTest<hashtype>(hash,sizeof(hashtype)+3,8,reps, g_drawDiagram);
    result &= CyclicKeyTest<hashtype>(hash,sizeof(hashtype)+4,8,reps, g_drawDiagram);
    result &= CyclicKeyTest<hashtype>(hash,sizeof(hashtype)+8,8,reps, g_drawDiagram);

    if(!result) printf("*********FAIL*********\n");
    printf("\n");
    fflush(NULL);
  }

  //-----------------------------------------------------------------------------
  // Keyset 'TwoBytes' - all keys up to N bytes containing two non-zero bytes
  // 3m40 for crc32_hw (32bit), 8m30 for xxh3 --extra (64bit)
  // 4m16 for xxh3
  // 4m50 for metrohash128crc_1
  // 260m for farmhash128_c with maxlen=16, 31s with maxlen=10, 2m with 12,14,15

  // With --extra this generates some huge keysets,
  // 128-bit tests will take ~1.3 gigs of RAM.

  if(g_testTwoBytes || g_testAll)
  {
    printf("[[[ Keyset 'TwoBytes' Tests ]]]\n\n");
    fflush(NULL);

    bool result = true;
    int maxlen = 24;
    if (!g_testExtra && (info->hashbits > 32)) {
      maxlen = (info->hashbits < 128) ? 20 : 15;
      if (g_speed > 500.0)
        maxlen = 8;
    }

    for(int len = 4; len <= maxlen; len += 4)
    {
      result &= TwoBytesTest2<hashtype>(hash, len, g_drawDiagram);
    }

    if(!result) printf("*********FAIL*********\n");
    printf("\n");
    fflush(NULL);
  }

  //-----------------------------------------------------------------------------
  // Keyset 'Text'

  if(g_testText || g_testAll)
  {
    printf("[[[ Keyset 'Text' Tests ]]]\n\n");

    bool result = true;

    const char * alnum = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789";

    result &= TextKeyTest( hash, "Foo",    alnum, 4, "Bar",    g_drawDiagram );
    result &= TextKeyTest( hash, "FooBar", alnum, 4, "",       g_drawDiagram );
    result &= TextKeyTest( hash, "",       alnum, 4, "FooBar", g_drawDiagram );

    if(!result) printf("*********FAIL*********\n");
    printf("\n");
    fflush(NULL);
  }

  //-----------------------------------------------------------------------------
  // Keyset 'Zeroes'

  if(g_testZeroes || g_testAll)
  {
    printf("[[[ Keyset 'Zeroes' Tests ]]]\n\n");

    bool result = true;

    result &= ZeroKeyTest<hashtype>( hash, g_drawDiagram );

    if(!result) printf("*********FAIL*********\n");
    printf("\n");
    fflush(NULL);
  }

  //-----------------------------------------------------------------------------
  // Keyset 'Seed'

  if(g_testSeed || g_testAll)
  {
    printf("[[[ Keyset 'Seed' Tests ]]]\n\n");

    bool result = true;

    result &= SeedTest<hashtype>( hash, 5000000, g_drawDiagram );

    if(!result) printf("*********FAIL*********\n");
    printf("\n");
    fflush(NULL);
  }

  //-----------------------------------------------------------------------------
  // Keyset 'PerlinNoise'

  if(g_testPerlinNoise || g_testAll)
  {
    printf("[[[ Keyset 'PerlinNoise' Tests ]]]\n\n");

    bool testCollision = true;
    bool testDistribution = g_testExtra;

    bool result = true;
    result &= PerlinNoise<hashtype>( hash, 2, testCollision, testDistribution, g_drawDiagram );
    if (g_testExtra) {
        result &= PerlinNoise<hashtype>( hash, 4, testCollision, testDistribution, g_drawDiagram );
        result &= PerlinNoise<hashtype>( hash, 8, testCollision, testDistribution, g_drawDiagram );
        result &= PerlinNoise<hashtype>( hash,16, testCollision, testDistribution, g_drawDiagram );
    }

    if(!result) printf("*********FAIL*********\n");
    printf("\n");
    fflush(NULL);
  }


  //-----------------------------------------------------------------------------
  // Differential tests
  // 5m30 with xxh3
  // less reps with slow or very bad hashes
  // md5: 1h38m with 1000 reps!

  if(g_testDiff || g_testAll)
  {
    printf("[[[ Diff 'Differential' Tests ]]]\n\n");
    fflush(NULL);

    bool result = true;
    bool dumpCollisions = g_drawDiagram; // from --verbose
    int reps = 1000;
    if ((g_speed > 500.0 || info->hashbits > 128 ||
         hash == multiply_shift || hash == pair_multiply_shift ||
         hash == o1hash_test) && !g_testExtra)
      reps = 100; // sha1: 7m, md5: 4m53

    result &= DiffTest< Blob<64>,  hashtype >(hash,5,reps,dumpCollisions);
    result &= DiffTest< Blob<128>, hashtype >(hash,4,reps,dumpCollisions);
    result &= DiffTest< Blob<256>, hashtype >(hash,3,reps,dumpCollisions);

    if(!result) printf("*********FAIL*********\n");
    printf("\n");
    fflush(NULL);
  }

  //-----------------------------------------------------------------------------
  // Differential-distribution tests
  // 2m40 with xxh3

  if (g_testDiffDist || g_testAll)
  {
    printf("[[[ DiffDist 'Differential Distribution' Tests ]]]\n\n");
    fflush(NULL);

    bool result = true;

    result &= DiffDistTest2<uint64_t,hashtype>(hash, g_drawDiagram);

    if(!result) printf("*********FAIL*********\n");
    printf("\n");
    fflush(NULL);
  }

  // Moment Chi-Square test, measuring the probability of the
  // lowest 32 bits set over the whole key space. Not where the bits are, but how many.
  // See e.g. https://www.statlect.com/fundamentals-of-probability/moment-generating-function
  // 10s (16 step interval until 0x7ffffff)
  // 20s (16 step interval until 0xcffffff)
  //   step  time
  //   1     300s
  //   2     150s
  //   3     90s
  //   7     35s
  //   13    20s
  //   16    12s
  if (g_testMomentChi2 || g_testAll)
  {
    printf("[[[ MomentChi2 Tests ]]]\n\n");

    bool result = true;
    result &= MomentChi2Test(info, 4);
    if (g_testExtra) {
        result &= MomentChi2Test(info, 8);
        result &= MomentChi2Test(info, 16);
    }

    if(!result) printf("\n*********FAIL*********\n");
    printf("\n");
    fflush(NULL);
  }


  if (g_testPrng || (g_testAll && g_testExtra))
  {
    printf("[[[ Prng Tests ]]]\n\n");

    bool testCollision = true;
    bool testDistribution = g_testExtra;

    bool result = true;
    result &= PrngTest<hashtype>( hash, testCollision, testDistribution, g_drawDiagram );

    if(!result) printf("\n*********FAIL*********\n");
    printf("\n");
    fflush(NULL);
  }


  //-----------------------------------------------------------------------------
  // LongNeighbors - collisions between long messages of low Hamming distance
  // esp. for testing separate word and then byte-wise processing of unaligned
  // rest parts. Only with --test=LongNeighbors or --extra
  // 10s for fasthash32
  // 7m with xxh3 (64bit)
  // 10m30s with farmhash128_c

  // Not yet included for licensing reasons
#if 0
  if(g_testLongNeighbors || (g_testAll && g_testExtra))
  {
    printf("[[[ LongNeighbors Tests ]]]\n\n");

    bool result = true;

    result &= testLongNeighbors(info->hash, info->hashbits, g_drawDiagram);

    if(!result) printf("*********FAIL*********\n");
    printf("\n");
    fflush(NULL);
  }
#endif

  //-----------------------------------------------------------------------------
  // Bit Independence Criteria. Interesting, but doesn't tell us much about
  // collision or distribution. For 128bit hashes only with --extra
  // 4m with xxh3
  // 152m with farmhash128_c with reps=1000000, => 8m with 100000

  if(g_testBIC || (g_testAll && info->hashbits > 64 && g_testExtra))
  {
    printf("[[[ BIC 'Bit Independence Criteria' Tests ]]]\n\n");
    fflush(NULL);

    bool result = true;
    if (info->hashbits > 64 || g_speed > 500.0) {
      result &= BicTest3<Blob<128>,hashtype>(hash,100000,g_drawDiagram);
    } else {
      const long reps = 64000000/info->hashbits;
      //result &= BicTest<uint64_t,hashtype>(hash,2000000);
      result &= BicTest3<Blob<88>,hashtype>(hash,(int)reps,g_drawDiagram);
    }

    if(!result) printf("\n*********FAIL*********\n");
    printf("\n");
    fflush(NULL);
  }

}

//-----------------------------------------------------------------------------

uint32_t g_inputVCode = 1;
uint32_t g_outputVCode = 1;
uint32_t g_resultVCode = 1;

HashInfo * g_hashUnderTest = NULL;

void VerifyHash ( const void * key, int len, uint32_t seed, void * out )
{
  g_inputVCode = MurmurOAAT((const char *)key, len, g_inputVCode);
  g_inputVCode = MurmurOAAT((const char *)&seed, sizeof(uint32_t), g_inputVCode);

  g_hashUnderTest->hash(key, len, seed, out);

  g_outputVCode = MurmurOAAT((const char *)out, g_hashUnderTest->hashbits/8, g_outputVCode);
}

// sha1_32a: 23m with step 3
bool MomentChi2Test ( struct HashInfo *info, int inputSize)
{
  pfHash const hash = info->hash;
  const int step = ((g_speed > 500 || info->hashbits > 128)
                    && !g_testExtra) ? 6 : 3;
  const unsigned mx = 0xfffffff0;
  assert(inputSize >= 4);
  long double const n = mx/step;
#define INPUT_SIZE_MAX 256
  assert(inputSize <= INPUT_SIZE_MAX);
  char key[INPUT_SIZE_MAX] = {0};
#define HASH_SIZE_MAX 64
  int hbits = info->hashbits;
  assert(hbits <= HASH_SIZE_MAX*8);
  char hbuff[HASH_SIZE_MAX] = {0};

  assert(inputSize > 0);
  printf("Analyze hashes produced from a serie of linearly increasing numbers "
         "of %i-bit, using a step of %d ... \n", inputSize*8, step);
  fflush(NULL);

  /* Notes on the ranking system.
   * Ideally, this test should report and sum all popcount values
   * and compare the resulting distribution to an ideal distribution.
   *
   * What happens here is quite simplified :
   * the test gives "points" for each popcount, and sum them all.
   * The metric (using N^5) is heavily influenced by the largest outliers.
   * For example, a 64-bit hash should have a popcount close to 32.
   * But a popcount==40 will tilt the metric upward
   * more than popcount==24 will tilt the metric downward.
   * In reality, both situations should be ranked similarly.
   *
   * To compensate, we measure both popcount1 and popcount0,
   * and compare to some pre-calculated "optimal" sums for the hash size.
   *
   * Another limitation of this test is that it only popcounts the first 64-bit.
   * For large hashes, bits beyond this limit are ignored.
   */

  if (hbits > 64) hbits = 64;   // limited due to popcount8
  long double srefh, srefl;
  switch (hbits/8) {
      case 8:
          srefh = 38918200.;
          srefl = 410450.;
          break;
      case 4:
          srefh = 1391290.;
          srefl = 1030.9;
          break;
      default:
          printf("hash size not covered \n");
          abort();
  }
  printf("Target values to approximate : %Lf - %Lf \n", srefh, srefl);

  unsigned const seed = 0;
  uint64_t previous = 0;
  long double b1h = 0. , b1l = 0., db1h = 0., db1l = 0.;
  long double b0h = 0. , b0l = 0., db0h = 0., db0l = 0.;
  for (unsigned i=1; i<=mx; i+=step) {
    assert(sizeof(i) <= inputSize);
    memcpy(key, &i, sizeof(i));
    hash(key, inputSize, seed, hbuff);

    uint64_t h; memcpy(&h, hbuff, 8);
    // popcount8 assumed to work on 64-bit
    // note : ideally, one should rather popcount the whole hash
    {   uint64_t const bits1 = popcount8(h);
        uint64_t const bits0 = hbits - bits1;
        uint64_t const b1_exp5 = bits1 * bits1 * bits1 * bits1 * bits1;
        uint64_t const b0_exp5 = bits0 * bits0 * bits0 * bits0 * bits0;
        b1h+=b1_exp5; b1l+=b1_exp5*b1_exp5;
        b0h+=b0_exp5; b0l+=b0_exp5*b0_exp5;
    }
    // derivative
    {   uint64_t const bits1 = popcount8(previous^h);
        uint64_t const bits0 = hbits - bits1;
        uint64_t const b1_exp5 = bits1 * bits1 * bits1 * bits1 * bits1;
        uint64_t const b0_exp5 = bits0 * bits0 * bits0 * bits0 * bits0;
        db1h+=b1_exp5; db1l+=b1_exp5*b1_exp5;
        db0h+=b0_exp5; db0l+=b0_exp5*b0_exp5;
    }
    previous=h;
  }

  b1h/=n; b1l=(b1l/n-b1h*b1h)/n;
  db1h/=n; db1l=(db1l/n-db1h*db1h)/n;
  b0h/=n; b0l=(b0l/n-b0h*b0h)/n;
  db0h/=n; db0l=(db0l/n-db0h*db0h)/n;

  printf("Popcount 1 stats : %Lf - %Lf\n", b1h, b1l);
  printf("Popcount 0 stats : %Lf - %Lf\n", b0h, b0l);
  double worsec2 = 0;
  {   double chi2 = (b1h-srefh) * (b1h-srefh) / (b1l+srefl);
      printf("MomentChi2 for bits 1 :  %8.6g \n", chi2);
      if (chi2 > worsec2) worsec2 = chi2;
  }
  {   double chi2 = (b0h-srefh) * (b0h-srefh) / (b0l+srefl);
      printf("MomentChi2 for bits 0 :  %8.6g \n", chi2);
      if (chi2 > worsec2) worsec2 = chi2;
  }

  /* Derivative :
   * In this scenario, 2 consecutive hashes are xored,
   * and the outcome of this xor operation is then popcount controlled.
   * Obviously, the _order_ in which the hash values are generated becomes critical.
   *
   * This scenario comes from the prng world,
   * where derivative of the generated suite of random numbers is analyzed
   * to ensure the suite is truly "random".
   *
   * However, in almost all prng, the seed of next random number is the previous random number.
   *
   * This scenario is quite different: it introduces a fixed distance between 2 consecutive "seeds".
   * This is especially detrimental to algorithms relying on linear operations, such as multiplications.
   *
   * This scenario is relevant if the hash is used as a prng and generates values from a linearly increasing counter as a seed.
   * It is not relevant for scenarios employing the hash as a prng
   * with the more classical method of using the previous random number as a seed for the next one.
   * This scenario has no relevance for classical usages of hash algorithms,
   * such as hash tables, bloom filters and such, were only the raw values are ever used.
   */
  printf("\nDerivative stats (transition from 2 consecutive values) : \n");
  printf("Popcount 1 stats : %Lf - %Lf\n", db1h, db1l);
  printf("Popcount 0 stats : %Lf - %Lf\n", db0h, db0l);
  {   double chi2 = (db1h-srefh) * (db1h-srefh) / (db1l+srefl);
      printf("MomentChi2 for deriv b1 :  %8.6g \n", chi2);
      if (chi2 > worsec2) worsec2 = chi2;
  }
  {   double chi2 = (db0h-srefh) * (db0h-srefh) / (db0l+srefl);
      printf("MomentChi2 for deriv b0 :  %8.6g \n", chi2);
      if (chi2 > worsec2) worsec2 = chi2;
  }

  // note : previous threshold : 3.84145882069413
  int const rank = (worsec2 < 500.) + (worsec2 < 50.) + (worsec2 < 5.);
  assert(0 <= rank && rank <= 3);

  const char* rankstr[4] = { "FAIL !!!!", "pass", "Good !", "Great !!" };
  printf("\n  %s \n\n", rankstr[rank]);
  fflush(NULL);

  return (rank > 0);
}


//-----------------------------------------------------------------------------

void testHash ( const char * name )
{
  HashInfo * pInfo = findHash(name);

  if(pInfo == NULL)
  {
    printf("Invalid hash '%s' specified\n", name);
    return;
  }
  else
  {
    g_hashUnderTest = pInfo;

    if(pInfo->hashbits == 32)
    {
      test<uint32_t>( pInfo->hash, pInfo );
    }
    else if(pInfo->hashbits == 64)
    {
      test<uint64_t>( pInfo->hash, pInfo );
    }
    else if(pInfo->hashbits == 128)
    {
      test<uint128_t>( pInfo->hash, pInfo );
    }
    else if(pInfo->hashbits == 160)
    {
      test<Blob<160>>( pInfo->hash, pInfo );
    }
    else if(pInfo->hashbits == 224)
    {
      test<Blob<224>>( pInfo->hash, pInfo );
    }
    else if(pInfo->hashbits == 256)
    {
      test<uint256_t>( pInfo->hash, pInfo );
    }
    else
    {
      printf("Invalid hash bit width %d for hash '%s'",
             pInfo->hashbits, pInfo->name);
    }
  }
}
//-----------------------------------------------------------------------------

#ifdef _MSC_VER
static char* strndup(char const *s, size_t n)
{
  size_t const len = strnlen(s, n);
  char *p = (char*) malloc(len + 1);
  if (p == NULL) return NULL;
  memcpy(p, s, len);
  p[len] = '\0';
  return p;
}
#endif


int main ( int argc, const char ** argv )
{
#ifdef DEBUG
  const char * defaulthash = "wysha";
#elif defined(__x86_64__) || defined(_M_X64) || defined(_X86_64_)
  const char * defaulthash = "xxh3";
#else
  const char * defaulthash = "wyhash";
#endif
  const char * hashToTest = defaulthash;

  if (argc < 2) {
    printf("No test hash given on command line, testing %s.\n", hashToTest);
    printf("Usage: SMHasher [--list][--listnames][--tests] [--verbose][--extra]\n"
           "       [--test=Speed,...] hash\n");
  }

  for (int argnb = 1; argnb < argc; argnb++) {
    const char* const arg = argv[argnb];
    if (strncmp(arg,"--", 2) == 0) {
      // This is a command
      if (strcmp(arg,"--help") == 0) {
        printf("Usage: SMHasher [--list][--listnames][--tests] [--verbose][--extra]\n"
               "       [--test=Speed,...] hash\n");
        exit(0);
      }
      if (strcmp(arg,"--list") == 0) {
        for(size_t i = 0; i < sizeof(g_hashes) / sizeof(HashInfo); i++) {
          printf("%-16s\t\"%s\" %s\n", g_hashes[i].name, g_hashes[i].desc, quality_str[g_hashes[i].quality]);
        }
        exit(0);
      }
      if (strcmp(arg,"--listnames") == 0) {
        for(size_t i = 0; i < sizeof(g_hashes) / sizeof(HashInfo); i++) {
          printf("%s\n", g_hashes[i].name);
        }
        exit(0);
      }
      if (strcmp(arg,"--tests") == 0) {
        printf("Valid tests:\n");
        for(size_t i = 0; i < sizeof(g_testopts) / sizeof(TestOpts); i++) {
          printf("  %s\n", g_testopts[i].name);
        }
        exit(0);
      }
      if (strcmp(arg,"--verbose") == 0) {
        g_drawDiagram = true;
        continue;
      }
      if (strcmp(arg,"--extra") == 0) {
        g_testExtra = true;
        continue;
      }
      /* default: --test=All. comma seperated list of options */
      if (strncmp(arg,"--test=", 6) == 0) {
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
          for (size_t i = 0; i < sizeof(g_testopts) / sizeof(TestOpts); i++) {
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
            printf(" \n");
            exit(1);
          }
        } while (p);
        continue;
      }
      // invalid command
      printf("Invalid command \n");
      printf("Usage: SMHasher [--list][--listnames][--tests] [--verbose][--extra]\n"
             "       [--test=Speed,...] hash\n");
      exit(1);
    }
    // Not a command ? => interprested as hash name
    hashToTest = arg;
  }

  // Code runs on the 3rd CPU by default? only for speed tests
  //SetAffinity((1 << 2));
  //SelfTest();

  int timeBegin = clock();

  testHash(hashToTest);

  int timeEnd = clock();

  printf("\n");
  fflush(NULL);
  if (g_testAll) {
    printf("Input vcode 0x%08x, Output vcode 0x%08x, Result vcode 0x%08x\n", g_inputVCode, g_outputVCode, g_resultVCode);
    printf("Verification value is 0x%08x - Testing took %f seconds\n", g_verify, double(timeEnd-timeBegin)/double(CLOCKS_PER_SEC));
    printf("-------------------------------------------------------------------------------\n");
  } else {
    fprintf(stderr, "Input vcode 0x%08x, Output vcode 0x%08x, Result vcode 0x%08x\n", g_inputVCode, g_outputVCode, g_resultVCode);
    fprintf(stderr, "Verification value is 0x%08x - Testing took %f seconds\n", g_verify, double(timeEnd-timeBegin)/double(CLOCKS_PER_SEC));
  }
    fflush(NULL);
  return 0;
}
