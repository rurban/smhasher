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
#include "md5.h"
#include "siphash.h"
#include <stdio.h>
#include <time.h>

//-----------------------------------------------------------------------------
// Configuration. This are defaults.

HashInfo * g_hashUnderTest = NULL;

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
bool g_testCrcCollision= false;

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
  { g_testSeed,		"Seed" },
  { g_testCrcCollision, "CRC" },
};

//-----------------------------------------------------------------------------
// This is the list of all hashes that SMHasher can test.
#define bitsizeof(x) (sizeof(x) * 8)

HashInfo g_hashes[] =
{
  // -- No-op hashes
  { "donothing32", "Do-Nothing function (only valid for measuring call overhead)",
    32, 32, 32, 0x00000000,
    DoNothingHash_seed_state, DoNothingHash_with_state },
  { "donothing64", "Do-Nothing function (only valid for measuring call overhead)",
    64, 64, 64, 0x00000000,
    DoNothingHash_seed_state, DoNothingHash_with_state },
  { "donothing128", "Do-Nothing function (only valid for measuring call overhead)",
    128, 128, 128, 0x00000000,
    DoNothingHash_seed_state, DoNothingHash_with_state },
  { "NOP_OAAT_read64", "Noop function (only valid for measuring call + OAAT reading overhead)",
    32, 32, 64, 0x00000000,
    NULL, NoopOAATReadHash_with_state },
  // -- Crap hashes
  { "BadHash", "very simple XOR shift",
    32, 32, 32, 0x3C2BE229,
    NULL, BadHash_with_state },
  { "crc32", "CRC-32",
    32, 32, 32, 0x240C9875,
    NULL, crc32_with_state_test },
#if defined(__SSE4_2__) && defined(__x86_64__)
  /* Even 32 uses crc32q, quad only */
  { "crc32_hw", "SSE4.2 crc32 in HW",
    32, 32, 32, 0xD706EE10,
    NULL, crc32c_hw_with_state_test },
  { "crc32_hw1", "Faster Adler SSE4.2 crc32 in HW",
    32, 32, 32, 0xD706EE10,
    NULL, crc32c_hw1_with_state_test },
  { "crc64_hw", "SSE4.2 crc64 in HW",
    64, 64, 64, 0x6B9CBBED,
    NULL, crc64c_hw_with_state_test },
#endif

  // -- Message Digests/NIST Hash Functions.
  { "md5_128a", "MD5, with a 64 bit seed of the start state",
    64, bitsizeof(md5_context), 128, 0x92912D2E,
    md5_seed_state, md5_with_state },
  { "md5_32a", "MD5, first 32 bits, with a 64 bit seed of the start start",
    64, bitsizeof(md5_context), 32, 0x4B2DAA7D,
    md5_seed_state, md5_32_with_state },
  { "sha1_32a", "SHA1, 32 bit seed, returning first 32 bits",
    32, 32, 32, 0xED4CCEA0,
    NULL, sha1_32a_with_state },
  { "sha1_64a", "SHA1, 32 bit seed, returning first 64 bits",
    32, 32, 64, 0xBE59879F,
    NULL, sha1_64a_with_state },
  { "sha1_32b", "SHA1, 32 bit seed, first 32 bits xored with last 32 bits",
    32, 32, 32, 0xA2D9EDEF,
    NULL, sha1_32b_with_state },

  // -- Others
  { "FNV1a", "Fowler-Noll-Vo hash, 32-bit",
    32, 32, 32, 0x03A9E27D,
    NULL, FNV32a_with_state_test },
  { "FNV1a_YT", "FNV1a-YoshimitsuTRIAD 32-bit sanmayce",
    32, 32, 32, 0xAB461C28,
    NULL, FNV32a_YoshimitsuTRIAD_with_state_test },
  { "FNV64", "Fowler-Noll-Vo hash, 64-bit",
    64, 64, 64, 0x28C0C9FE,
    NULL, FNV64a_with_state_test },
  { "bernstein", "Bernstein, 32-bit",
    32, 32, 32, 0xE956A8EA,
    NULL, Bernstein_with_state },
  { "lookup3", "Bob Jenkins' lookup3",
    32, 32, 32, 0x74F2AE38,
    NULL, lookup3_with_state_test },
  { "superfast", "Paul Hsieh's SuperFastHash",
    32, 32, 32, 0x4FC5C2FD,
    NULL, SuperFastHash },
  { "MurmurOAAT", "Murmur one-at-a-time",
    32, 32, 32, 0x9C2391D8,
    NULL, MurmurOAAT_with_state_test },
  { "Crap8", "Crap8",
    32, 32, 32, 0x23D7DE52,
    NULL, Crap8_test },

  // SpoookyHash
  { "Spooky32", "Bob Jenkins' SpookyHash, 32-bit seed, 32-bit result",
    32, 128, 32, 0x66857857,
    SpookyHash_seed_state_test, SpookyHash32_with_state_test },
  { "Spooky64", "Bob Jenkins' SpookyHash, 64-bit seed, 64-bit result",
    64, 128, 64, 0x461EC794,
    SpookyHash_seed_state_test, SpookyHash64_with_state_test },
  { "Spooky128", "Bob Jenkins' SpookyHash, 128-bit seed, 128-bit result",
    128, 128, 128, 0x5DF3C836,
    SpookyHash_seed_state_test, SpookyHash128_with_state_test },

  // MurmurHash2
  { "Murmur2", "MurmurHash2 for x86, 32-bit",
    32, 32, 32, 0xBD317868,
    NULL, MurmurHash2_with_state_test },
  { "Murmur2A", "MurmurHash2A for x86, 32-bit",
    32, 32, 32, 0x510C93BE,
    NULL, MurmurHash2A_with_state_test },
  { "Murmur2B", "MurmurHash2 for x64, 64-bit",
    64, 64, 64, 0x5393B6C8,
    NULL, MurmurHash64A_with_state_test },
  { "Murmur2C", "MurmurHash2 for x86, 64-bit",
    64, 64, 64, 0x89EAA4DF,
    NULL, MurmurHash64B_with_state_test },

  // MurmurHash3
  { "Murmur3A", "MurmurHash3 for x86, 32-bit",
    32, 32, 32, 0x6D6CCA26,
    NULL, MurmurHash3_x86_32_with_state },
  { "Murmur3C", "MurmurHash3 for x86, 128-bit",
    32, 32, 128, 0xC70CC981,
    NULL, MurmurHash3_x86_128_with_state },
#if defined(__x86_64__)
  { "Murmur3F", "MurmurHash3 for x64, 128-bit",
    64, 64, 128, 0x8731131B,
    NULL, MurmurHash3_x64_128_with_state },
#endif

  { "PMurHash32", "Shane Day's portable-ized MurmurHash3 for x86, 32-bit.",
    32, 32, 32, 0x6D6CCA26,
    NULL, PMurHash32_with_state_test },

  // BeagleHash_32_xx
  { "BeagleHash_32_32", "Yves Orton's hash for 64-bit in 32-bit mode (32-bit seed).",
    32, 128, 32, 0xCE2B0329,
    beagle_seed_state_128_a_smhasher, beagle_hash_with_state_32_128_a_smhasher },
  { "BeagleHash_32_64", "Yves Orton's hash for 64-bit in 32-bit mode (64-bit seed).",
    64, 128, 32, 0xF47C019F,
    beagle_seed_state_128_a_smhasher, beagle_hash_with_state_32_128_a_smhasher },
  { "BeagleHash_32_96", "Yves Orton's hash for 64-bit in 32-bit mode (96-bit seed).",
    96, 128, 32, 0x5A0F7E13,
    beagle_seed_state_128_a_smhasher, beagle_hash_with_state_32_128_a_smhasher },
  { "BeagleHash_32_112", "Yves Orton's hash for 64-bit in 32-bit mode (112-bit seed).",
    112, 128, 32, 0x293F1FFD,
    beagle_seed_state_128_a_smhasher, beagle_hash_with_state_32_128_a_smhasher },
  { "BeagleHash_32_127", "Yves Orton's hash for 64-bit in 32-bit mode (127-bit seed).",
    127, 128, 32, 0x556FEDF5,
    beagle_seed_state_128_a_smhasher, beagle_hash_with_state_32_128_a_smhasher },

  // BeagleHash_64_xx
  { "BeagleHash_64_32", "Yves Orton's hash for 64-bit. (32 bit seed)",
    32, 128, 64, 0x61CA1652,
    beagle_seed_state_128_a_smhasher, beagle_hash_with_state_64_128_a_smhasher },
  { "BeagleHash_64_64", "Yves Orton's hash for 64-bit. (64 bit seed)",
    64, 128, 64, 0xFDBD2E90,
    beagle_seed_state_128_a_smhasher, beagle_hash_with_state_64_128_a_smhasher },
  { "BeagleHash_64_96", "Yves Orton's hash for 64-bit (96 bit seed).",
    96, 128, 64, 0x5C10352B,
    beagle_seed_state_128_a_smhasher, beagle_hash_with_state_64_128_a_smhasher },
  { "BeagleHash_64_112", "Yves Orton's hash for 64-bit (112 bit seed).",
    112, 128, 64, 0x92CBB964,
    beagle_seed_state_128_a_smhasher, beagle_hash_with_state_64_128_a_smhasher },
  { "BeagleHash_64_127", "Yves Orton's hash for 64-bit (127 bit seed).",
    127, 128, 64, 0x32F92681,
    beagle_seed_state_128_a_smhasher, beagle_hash_with_state_64_128_a_smhasher },
#if 0
  // very very slow seeding process, huge state, etc. not suitable for day-to-day testing
  { "SBOX", "Yves Orton's 64 bit substitution box hash for up to 32 char strings",
    128, (32 * 256 + 3) * 64, 64, 0xEF32DD9E,
    sbox_seed_state_smhasher_test, sbox_hash_with_state_smhasher_test },
#endif
  //{ "Marsaglia64", "Yves Orton's 64 bit hash with 128 bit seed",
  //  128, 256, 64, 0x0,
  //  stadt64_seed_state_smhasher_test, stadt64_hash_with_state_smhasher_test },
  { "Zaphod64", "Yves Orton's 64 bit hash with 191 bit seed",
    191, 192, 64, 0xC0CA33C4,
    zaphod64_seed_state_smhasher_test, zaphod64_hash_with_state_smhasher_test },
  { "Zaphod32", "Yves Orton's 32 bit hash with 95 bit seed",
    95, 96, 32, 0x39CA924F,
    zaphod32_seed_state_smhasher_test, zaphod32_hash_with_state_smhasher_test },
  { "Phat4", "Yves Orton's 32 bit hash with 96 bit seed",
    96, 96, 32, 0xE1C39C99,
    NULL, phat4_hash_with_state_smhasher_test },
  { "Lua53oaat", "Hash function from Lua53, pure one-at-a-time",
    32, 32, 32, 0x072086C1,
    NULL, lua_v53_string_hash_oaat },
  { "Lua53", "Hash function from Lua53, (skip forward)",
    32, 32, 32, 0x8BF8D02F,
    NULL, lua_v53_string_hash },

#ifdef __SSE2__
  { "hasshe2", "SSE2 hasshe2, 256-bit",
    128, 128, 256, 0x5A1E57A1,
    NULL, hasshe2_test },
#endif


  { "sdbm", "sdbm - with seeding (as in perl5)",
    32, 32, 32, 0x558B55E5,
    NULL, sdbm },
  { "x17", "x17",
    32, 32, 32, 0x1A7639D5,
    NULL, x17_test },
  // also called jhash:
  { "JenkinsOAATH", "Bob Jenkins' one-at-a-time with hardening (as in perl 5.18)",
    64, 64, 32, 0x89132BD1,
    NULL, JenkinsOAATH_with_state },
  { "JenkinsOAAT", "Bob Jenkins' one-at-a-time as in old perl5",
    32, 32, 32, 0xCE081D30,
    NULL, JenkinsOAAT_with_state },
  { "MicroOAAT", "Small non-mul OAAT that passes collision checks (by funny-falcon)",
    32, 32, 32, 0x9735B83E,
    NULL, MicroOAAT },
  { "HalfSipHash", "HalfSipHash 2-4, 32bit",
    64, 128, 32, 0xC8C3FAB5,
    halfsiphash_seed_state_test, halfsiphash_with_state_test },

  // and now the quality hash funcs, which mostly work
  // GoodOOAT passes whole SMHasher (by funny-falcon)
  { "GoodOAAT", "Small non-multiplicative OAAT",
    32, 32, 32, 0xF447CE02,
    NULL, GoodOAAT },
  { "SipHash", "SipHash 2-4",
    128, 256, 64, 0x5F4173E0,
    siphash_seed_state_test, siphash_with_state_test },
  // as in rust and swift
  { "SipHash13", "SipHash 1-3",
    128, 256, 64, 0xD03C3A2C,
    siphash_seed_state_test, siphash13_with_state_test },
#if defined(__x86_64__)
  { "fasthash32", "fast-hash 32bit",
    32, 32, 32, 0x4651528E,
    NULL, fasthash32_test },
  { "fasthash64", "fast-hash 64bit",
    64, 64, 64, 0xACB02426,
    NULL, fasthash64_test },
#endif

  // CityHash
  { "City32", "Google CityHash32WithSeed (old)",
    32, 32, 32, 0x1CC93AA2,
    NULL, CityHash32_with_state_test },
  { "City64", "Google CityHash64WithSeed (old)",
    64, 64, 64, 0xFEF9D67E,
    NULL, CityHash64_with_state_test },
#if defined(__SSE4_2__) && defined(__x86_64__)
  { "City128", "Google CityHash128WithSeed (old)",
    128, 128, 128, 0x9D806278,
    NULL, CityHash128_with_state_test },
  { "CityCrc128", "Google CityHashCrc128WithSeed SSE4.2 (old)",
    128, 128, 128, 0x10AC3095,
    NULL, CityHashCrc128_with_state_test },
#endif
#if defined(__x86_64__)
  { "FarmHash64", "Google FarmHash64WithSeed",
    64, 64, 64, 0x96461B3D,
    NULL, FarmHash64_with_state_test },
  { "FarmHash128", "Google FarmHash128WithSeed",
    128, 128, 128, 0x2F80D32B,
    NULL, FarmHash128_with_state_test },
  { "farmhash64_c", "farmhash64_with_seed (C99)",
    64, 64, 64, 0x96461B3D,
    NULL, farmhash64_c_with_state_test },
  { "farmhash128_c", "farmhash128_with_seed (C99)",
    128, 128, 128, 0x2F80D32B,
    NULL, farmhash128_c_with_state_test },
#endif
#if defined(__x86_64__)
  { "xxHash32", "xxHash, 32-bit for x64",
    32, 32, 32, 0x6058CA7F,
    NULL, xxHash32_with_state_test },
  { "xxHash64", "xxHash, 64-bit",
    64, 64, 64, 0x22B287C9,
    NULL, xxHash64_with_state_test },
#endif
#if defined(__x86_64__)
  { "metrohash64_1", "MetroHash64_1 for 64-bit",
    64, 64, 64, 0x65BF19EE,
    NULL, metrohash64_1_with_state_test },
  { "metrohash64_2", "MetroHash64_2 for 64-bit",
    64, 64, 64, 0xF942721F,
    NULL, metrohash64_2_with_state_test },
  { "metrohash128_1", "MetroHash128_1 for 64-bit",
    64, 64, 128, 0x9BBD5CE7,
    NULL, metrohash128_1_with_state_test },
  { "metrohash128_2", "MetroHash128_2 for 64-bit",
    64, 64, 128, 0x2EFAD245,
    NULL, metrohash128_2_with_state_test },
#if defined(__SSE4_2__) && defined(__x86_64__)
  { "metrohash64crc_1", "MetroHash64crc_1 for x64",
    64, 64, 64, 0x2B7B5BD7,
    NULL, metrohash64crc_1_with_state_test },
  { "metrohash64crc_2", "MetroHash64crc_2 for x64",
    64, 64, 64, 0x54FA2B08,
    NULL, metrohash64crc_2_with_state_test },
  { "metrohash128crc_1", "MetroHash128crc_1 for x64",
    64, 64, 128, 0x0B2C1E37,
    NULL, metrohash128crc_1_with_state_test },
  { "metrohash128crc_2", "MetroHash128crc_2 for x64",
    64, 64, 128, 0xA3773542,
    NULL, metrohash128crc_2_with_state_test },
#endif
#endif
#if defined(__x86_64__)
  { "cmetrohash64_1o", "cmetrohash64_1 (shorter key optimized) , 64-bit for x64",
    64, 64, 64, 0x807349A0,
    NULL, cmetrohash64_1_optshort_with_state_test },
  { "cmetrohash64_1", "cmetrohash64_1, 64-bit for x64",
    64, 64, 64, 0x807349A0,
    NULL, cmetrohash64_1_with_state_test },
  { "cmetrohash64_2", "cmetrohash64_2, 64-bit for x64",
    64, 64, 64, 0x048847C2,
    NULL, cmetrohash64_2_with_state_test },
#endif
#if defined(__SSE4_2__) && defined(__x86_64__)
  { "falkhash", "falkhash.asm with aesenc, 64-bit for x64",
    64, 64, 64, 0x211CE75E,
    NULL, falkhash_with_state_test_cxx },
#endif
  { "t1ha", "Fast Positive Hash (portable, best for: 64-bit, little-endian)",
    64, 64, 64, 0xA18A4E0E,
    NULL, t1ha_with_state_test },
  { "t1ha_64be", "Fast Positive Hash (portable, best for: 64-bit, big-endian)",
    64, 64, 64, 0xF8FFDED6,
    NULL, t1ha_64be_with_state_test },
  { "t1ha_32le", "Fast Positive Hash (portable, best for: 32-bit, little-endian)",
    64, 64, 64, 0xE42B362F,
    NULL, t1ha_32le_with_state_test },
  { "t1ha_32be", "Fast Positive Hash (portable, best for: 32-bit, big-endian)",
    64, 64, 64, 0xF0C501EB,
    NULL, t1ha_32be_with_state_test },
#if (defined(__SSE4_2__) && defined(__x86_64__)) || defined(_M_X64)
  { "t1ha_crc", "Fast Positive Hash (machine-specific, requires: SSE4.2 CRC32C)",
    64, 64, 64, 0x20BD585A,
    NULL, t1ha_crc_with_state_test },
#endif
#if defined(__AES__) || defined(_M_X64) || defined(_M_IX86)
  { "t1ha_aes", "Fast Positive Hash (machine-specific, requires: AES-NI)",
    64, 64, 64, 0x62ACCBC1,
    NULL, t1ha_aes_with_state_test },
#endif
#if defined(__GNUC__) && UINT_MAX != ULONG_MAX
#define MUM_VERIFY 0xE9816A4F
#else
/* this is almost for sure wrong. if necessary just fix it */
#define MUM_VERIFY 0xA973C6C0
#endif
  { "MUM", "github.com/vnmakarov/mum-hash",
    64, 64, 64, 0x73F6BFD7,
    NULL, mum_hash_with_state_test },
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

//-----------------------------------------------------------------------------
// Self-test on startup - verify that all installed hashes work correctly.

void SelfTest ( bool validate )
{
  bool pass = true;
  char name[1024];
  snprintf(name,1024,"Self Test - Verify %d Hashes",(int)(g_hashes_sizeof/sizeof(HashInfo)));

  for(int i = 0; i < g_hashes_sizeof / sizeof(HashInfo); i++)
  {
    HashInfo * info = & g_hashes[i];

    pass &= testHashByInfo( info, 1, g_confidence );
  }
  ok(pass,name);

  if(!pass || validate)
  {
    for(size_t i = 0; i < g_hashes_sizeof / sizeof(HashInfo); i++)
    {
      HashInfo * info = & g_hashes[i];

      pass &= testHashByInfo( info, 2, g_confidence );
    }
    done_testing();

  }
}

int main ( int argc, char ** argv )
{
#if (defined(__x86_64__) && __SSE4_2__) || defined(_M_X64) || defined(_X86_64_)
  const char * defaulthash = "metrohash64crc_1"; /* "murmur3a"; */
#else
  const char * defaulthash = "t1ha_32le";
#endif
  const char * hashToTest = defaulthash;
  bool opt_validate = false;
  
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
          printf("\n"); // nl ok
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
      printf("\n"); // nl ok
      exit(1);
    }
  }

  // Code runs on the 3rd CPU by default
  SetAffinity((1 << 2));

  SelfTest(opt_validate);

  //----------
  HashInfo * pInfo = findHash(hashToTest);
  if (ok(pInfo!=NULL,"Found Hash",pInfo->name))
    ok(testHashByInfo(pInfo,0,g_confidence),"all tests passed",pInfo->name);

  done_testing();

  //----------

  return 0;
}
/* vim: set sts=2 sw=2 et: */
