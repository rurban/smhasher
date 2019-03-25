#include "Platform.h"
#include "Hashes.h"
#include "KeysetTest.h"
#include "SpeedTest.h"
#include "AvalancheTest.h"
#include "DifferentialTest.h"

#include "PMurHash.h"

#include <stdio.h>
#include <stdint.h>
#include <time.h>

//-----------------------------------------------------------------------------
// Configuration. TODO - move these to command-line flags

bool g_drawDiagram     = false;
bool g_testAll         = true;
bool g_testExtra       = false; // excessive torture tests: Sparse, Avalanche, DiffDist
bool g_testVerifyAll   = false;

bool g_testSanity      = false;
bool g_testSpeed       = false;
bool g_testAvalanche   = false;
bool g_testSparse      = false;
bool g_testPermutation = false;
bool g_testWindow      = false;
bool g_testCyclic      = false;
bool g_testTwoBytes    = false;
bool g_testMomentChi2  = false;
//bool g_testLongNeighbors = false;
bool g_testText        = false;
bool g_testZeroes      = false;
bool g_testSeed        = false;
bool g_testDiff        = false;
bool g_testDiffDist    = false;
bool g_testBIC         = false;

struct TestOpts {
  bool         &var;
  const char*  name;
};
TestOpts g_testopts[] =
{
  { g_testAll, 		"All" },
  { g_testVerifyAll,    "VerifyAll" },
  { g_testSanity, 	"Sanity" },
  { g_testSpeed, 	"Speed" },
  { g_testAvalanche, 	"Avalanche" },
  { g_testSparse,	"Sparse" },
  { g_testPermutation,	"Permutation" },
  { g_testWindow,	"Window" },
  { g_testCyclic,	"Cyclic" },
  { g_testTwoBytes,	"TwoBytes" },
  { g_testMomentChi2,   "MomentChi2" },
  //{ g_testLongNeighbors,"LongNeighbors" },
  { g_testText,		"Text" },
  { g_testZeroes,	"Zeroes" },
  { g_testSeed,		"Seed" },
  { g_testDiff, 	"Diff" },
  { g_testDiffDist, 	"DiffDist" },
  { g_testBIC, 		"BIC" }
};

bool MomentChi2Test ( struct HashInfo *info );

//-----------------------------------------------------------------------------
// This is the list of all hashes that SMHasher can test.

const char* quality_str[3] = { "SKIP", "POOR", "GOOD" };
enum HashQuality             {  SKIP,   POOR,   GOOD };
struct HashInfo
{
  pfHash hash;
  int hashbits;
  uint32_t verification;
  const char * name;
  const char * desc;
  enum HashQuality quality;
};

// sorted by quality and speed
HashInfo g_hashes[] =
{
  // first the bad hash funcs, failing tests:
 { DoNothingHash,        32, 0x00000000, "donothing32", "Do-Nothing function (only valid for measuring call overhead)", SKIP },
  { DoNothingHash,        64, 0x00000000, "donothing64", "Do-Nothing function (only valid for measuring call overhead)", SKIP },
  { DoNothingHash,       128, 0x00000000, "donothing128", "Do-Nothing function (only valid for measuring call overhead)", SKIP },
  { NoopOAATReadHash,     64, 0x00000000, "NOP_OAAT_read64", "Noop function (only valid for measuring call + OAAT reading overhead)", SKIP },
  { BadHash,     	  32, 0xAB432E23, "BadHash", 	 "very simple XOR shift", SKIP },
  { sumhash,     	  32, 0x0000A9AC, "sumhash", 	 "sum all bytes", SKIP },
  { sumhash32,     	  32, 0x3D6DC280, "sumhash32",   "sum all 32bit words", SKIP },

  // here start the real hashes. the problematic ones:
  { crc32,                32, 0x3719DB20, "crc32",       "CRC-32 soft", POOR },
  { md5_32,               32, 0xF7192210, "md5_32a",     "MD5, first 32 bits of result", POOR },
#ifdef _MSC_VER /* truncated long to 32 */
#  define SHA1_VERIF          0xDCB02360
#else
#  define SHA1_VERIF          0x7FE8C80E
#endif
  { sha1_32a,             32, SHA1_VERIF, "sha1_32a",    "SHA1, first 32 bits of result", POOR},
#if 0
  { sha1_64a,             32, 0x00000000, "sha1_64a",    "SHA1 64-bit, first 64 bits of result", POOR },
  { sha2_32a,             32, 0x00000000, "sha2_32a",    "SHA2, first 32 bits of result", POOR },
  { sha2_64a,             64, 0x00000000, "sha2_64a",    "SHA2, first 64 bits of result", POOR },
  { sha3_32a,             32, 0x00000000, "sha3_32a",    "SHA3, first 32 bits of result", POOR },
  { sha3_64a,             64, 0x00000000, "sha3_64a",    "SHA3, first 64 bits of result", POOR },
  { BLAKE2_32a,           32, 0x00000000, "blake2_32a",  "BLAKE2, first 32 bits of result", POOR },
  { BLAKE2_64a,           64, 0x00000000, "blake2_64a",  "BLAKE2, first 64 bits of result", POOR },
  { bcrypt_64a,           64, 0x00000000, "bcrypt_64a",  "bcrypt, first 64 bits of result", POOR },
  { scrypt_64a,           64, 0x00000000, "scrypt_64a",  "scrypt, first 64 bits of result", POOR },
#endif

#ifdef __SSE2__
  { hasshe2_test,        256, 0xF5D39DFE, "hasshe2",     "SSE2 hasshe2, 256-bit", POOR },
#endif
#if defined(__SSE4_2__) && defined(__x86_64__)
  /* Even 32 uses crc32q, quad only */
  { crc32c_hw_test,       32, 0x0C7346F0, "crc32_hw",    "SSE4.2 crc32 in HW", POOR },
  { crc32c_hw1_test,      32, 0x0C7346F0, "crc32_hw1",   "Faster Adler SSE4.2 crc32 in HW", POOR },
  { crc64c_hw_test,       64, 0xE7C3FD0E, "crc64_hw",    "SSE4.2 crc64 in HW", POOR },
#endif
#if 0 && defined(__x86_64__) && (defined(__linux__) || defined(__APPLE__))
  // elf64 or macho64 only
  { fhtw_test,            64, 0x0,        "fhtw",        "fhtw asm", POOR },
#endif
#ifdef HAVE_BIT32
 #define FIBONACCI_VERIF      0x09952480
 #define FNV2_VERIF           0x739801C5
 #define MULTSHIFT_VERIF      0x90FA041A
 #define PAIRMULTSHIFT_VERIF  0x0C211550
#else
 #define FIBONACCI_VERIF      0xFE3BD380
 #define FNV2_VERIF           0x1967C625
 #define MULTSHIFT_VERIF      0xF15F3D1E
 #define PAIRMULTSHIFT_VERIF  0xB070283D
#endif
  { fibonacci,    __WORDSIZE, FIBONACCI_VERIF, "fibonacci",   "wordwise Fibonacci", POOR },
  // M. Dietzfelbinger, T. Hagerup, J. Katajainen, and M. Penttonen. A reliable randomized
  // algorithm for the closest-pair problem. J. Algorithms, 25:19–51, 1997.
  { multiply_shift, __WORDSIZE,MULTSHIFT_VERIF, "multiply_shift", "Dietzfelbinger Multiply-shift on strings", POOR },
  { pair_multiply_shift, __WORDSIZE, PAIRMULTSHIFT_VERIF, "pair_multiply_shift", "Pair-multiply-shift", POOR },

  { FNV32a,               32, 0xE3CBBE91, "FNV1a",       "Fowler-Noll-Vo hash, 32-bit", POOR },
  { FNV32a_YoshimitsuTRIAD,32,0xD8AFFD71, "FNV1a_YT",    "FNV1a-YoshimitsuTRIAD 32-bit sanmayce", POOR },
#if 0 /* TODO */
  { Jesteress,            32, 0x0, "FNV1a_Jesteress",  "FNV1a-Jesteress 32-bit sanmayce", POOR },
  { Meiyan,       	  32, 0x0, "FNV1a_Meiyan",     "FNV1a-Meiyan 32-bit sanmayce", POOR },
#endif
  { FNV64a,               64, 0x103455FC, "FNV64",       "Fowler-Noll-Vo hash, 64-bit", POOR },
  { FNV2,         __WORDSIZE, FNV2_VERIF, "FNV2",        "wordwise FNV", POOR },
  { fletcher2,            64, 0x890767C0, "fletcher2",   "fletcher2 ZFS", POOR},
  { fletcher4,            64, 0x47660EB7, "fletcher4",   "fletcher4 ZFS", POOR},
  { Bernstein,            32, 0xBDB4B640, "bernstein",   "Bernstein, 32-bit", POOR },
  { sdbm,                 32, 0x582AF769, "sdbm",        "sdbm as in perl5", POOR },
  { x17_test,             32, 0x8128E14C, "x17",         "x17", POOR },
  // also called jhash:
  { JenkinsOOAT,          32, 0x83E133DA, "JenkinsOOAT", "Bob Jenkins' OOAT as in perl 5.18", POOR },
  { JenkinsOOAT_perl,     32, 0xEE05869B, "JenkinsOOAT_perl", "Bob Jenkins' OOAT as in old perl5", POOR },
  { MicroOAAT,            32, 0x16F1BA97, "MicroOAAT", "Small non-multiplicative OAAT that passes all collision checks (by funny-falcon)", POOR },
  { jodyhash32_test,      32, 0xFB47D60D, "jodyhash32",  "jodyhash, 32-bit (v5)", POOR },
  { jodyhash64_test,      64, 0x9F09E57F, "jodyhash64",  "jodyhash, 64-bit (v5)", POOR },
  { lookup3_test,         32, 0x3D83917A, "lookup3",     "Bob Jenkins' lookup3", POOR },
  { SuperFastHash,        32, 0x980ACD1D, "superfast",   "Paul Hsieh's SuperFastHash", POOR },
  { MurmurOAAT_test,      32, 0x5363BD98, "MurmurOAAT",  "Murmur one-at-a-time", POOR },
  { Crap8_test,           32, 0x743E97A1, "Crap8",       "Crap8", POOR },
  { MurmurHash2_test,     32, 0x27864C1E, "Murmur2",     "MurmurHash2 for x86, 32-bit", POOR },
  { MurmurHash2A_test,    32, 0x7FBD4396, "Murmur2A",    "MurmurHash2A for x86, 32-bit", POOR },
#if __WORDSIZE >= 64
  { MurmurHash64A_test,   64, 0x1F0D3804, "Murmur2B",    "MurmurHash2 for x64, 64-bit", POOR },
#endif
  { MurmurHash64B_test,   64, 0xDD537C05, "Murmur2C",    "MurmurHash2 for x86, 64-bit", POOR },
  { MurmurHash3_x86_128, 128, 0xB3ECE62A, "Murmur3C",    "MurmurHash3 for x86, 128-bit", POOR },
  { halfsiphash_test,     32, 0xA7A05F72, "HalfSipHash", "HalfSipHash 2-4, 32bit", POOR },
  // as in rust and swift
  { siphash13_test,       64, 0x29C010BF, "SipHash13",   "SipHash 1-3 - SSSE3 optimized", POOR },

  // and now the quality hash funcs
  { siphash_test,         64, 0xC58D7F9C, "SipHash",     "SipHash 2-4 - SSSE3 optimized", GOOD },
#ifdef HAVE_HIGHWAYHASH
  { HighwayHash64_test,   64, 0x53BE18F1, "HighwayHash64", "Google HighwayHash (portable with overhead from the lib)", GOOD },
#endif
  { GoodOAAT,             32, 0x7B14EEE5, "GoodOAAT",    "Small non-multiplicative OAAT", GOOD },
  { PMurHash32_test,      32, 0xB0F57EE3, "PMurHash32",  "Shane Day's portable-ized MurmurHash3 for x86, 32-bit", GOOD },
  { MurmurHash3_x86_32,   32, 0xB0F57EE3, "Murmur3A",    "MurmurHash3 for x86, 32-bit", GOOD },
#if __WORDSIZE >= 64
  { MurmurHash3_x64_128, 128, 0x6384BA69, "Murmur3F",    "MurmurHash3 for x64, 128-bit", GOOD },
#endif
  { fasthash32_test,      32, 0xE9481AFC, "fasthash32",  "fast-hash 32bit", GOOD },
  { fasthash64_test,      64, 0xA16231A7, "fasthash64",  "fast-hash 64bit", GOOD },
#if defined(__GNUC__) && UINT_MAX != ULONG_MAX
 #define MUM_VERIF            0x3EEAE2D4
 #define MUMLOW_VERIF         0x520263F5
#else
 #define MUM_VERIF            0xA973C6C0
 #define MUMLOW_VERIF         0x7F898826
#endif
  { mum_hash_test,        64, MUM_VERIF,  "MUM",         "github.com/vnmakarov/mum-hash", GOOD },
  { mum_low_test,         32, MUMLOW_VERIF,"MUMlow",     "github.com/vnmakarov/mum-hash", GOOD },

  { CityHash32_test,      32, 0x5C28AD62, "City32",      "Google CityHash32WithSeed (old)", GOOD },
  { CityHash64noSeed_test,64, 0x63FC6063, "City64noSeed","Google CityHash64 without seed (default version, misses one final avalanche)", POOR },
  { CityHash64_test,      64, 0x25A20825, "City64",      "Google CityHash64WithSeed (old)", POOR },
  { CityHash64_low_test,  32, 0xCC5BC861, "City64low",   "Google CityHash64WithSeed (low 32-bits)", GOOD },
#if defined(__SSE4_2__) && defined(__x86_64__)
  { CityHash128_test,    128, 0x6531F54E, "City128",     "Google CityHash128WithSeed (old)", GOOD },
  { CityHashCrc128_test, 128, 0xD4389C97, "CityCrc128",  "Google CityHashCrc128WithSeed SSE4.2 (old)", GOOD },
#endif

#ifdef _MSC_VER /* truncated long to 32 */
#  define FARM64_VERIF        0xEBC4A679
#  define FARM128_VERIF       0x305C0D9A
#else
#  define FARM64_VERIF        0x35F84A93
#  define FARM128_VERIF       0x9E636AAE
#endif
  { FarmHash64_test,      64, FARM64_VERIF, "FarmHash64",  "Google FarmHash64WithSeed", GOOD },
 //{ FarmHash64noSeed_test,64, 0xA5B9146C,  "Farm64noSeed","Google FarmHash64 without seed (default, misses on final avalanche)", POOR },
  { FarmHash128_test,    128, FARM128_VERIF,"FarmHash128", "Google FarmHash128WithSeed", GOOD },
#if defined(__SSE4_2__) && defined(__x86_64__)
  { farmhash64_c_test,    64, FARM64_VERIF, "farmhash64_c",  "farmhash64_with_seed (C99)", GOOD },
  { farmhash128_c_test,  128, FARM128_VERIF,"farmhash128_c", "farmhash128_with_seed (C99)", GOOD },
#endif

  { xxHash32_test,        32, 0xBA88B743, "xxHash32",    "xxHash, 32-bit for x64", POOR },
  { xxHash64_test,        64, 0x024B7CF4, "xxHash64",    "xxHash, 64-bit", GOOD },
  { xxh3_test,            64, 0x5921E69E, "xxh3",        "xxHash v3, 64-bit", GOOD },
  { xxh3low_test,         32, 0xAC902311, "xxh3low",     "xxHash v3, 64-bit, low 32-bits part", GOOD },
  { xxh128_test,         128, 0x80E5D1DF, "xxh128",      "xxHash v3, 128-bit", GOOD },
  { xxh128low_test,       64, 0xB1BB6A50, "xxh128low",   "xxHash v3, 128-bit, low 64-bits part", GOOD },
#if 0
  { xxhash256_test,       64, 0x024B7CF4, "xxhash256",   "xxhash256, 64-bit unportable", GOOD },
#endif

  { SpookyHash32_test,    32, 0x3F798BBB, "Spooky32",    "Bob Jenkins' SpookyHash, 32-bit result", GOOD },
  { SpookyHash64_test,    64, 0xA7F955F1, "Spooky64",    "Bob Jenkins' SpookyHash, 64-bit result", GOOD },
  { SpookyHash128_test,  128, 0x8D263080, "Spooky128",   "Bob Jenkins' SpookyHash, 128-bit result", GOOD },
  { metrohash64_1_test,       64, 0xEE88F7D2, "metrohash64_1",     "MetroHash64_1 for 64-bit", POOR },
  { metrohash64_2_test,       64, 0xE1FC7C6E, "metrohash64_2",     "MetroHash64_2 for 64-bit", POOR },
  { metrohash128_1_test,     128, 0x20E8A1D7, "metrohash128_1",    "MetroHash128_1 for 64-bit", POOR },
  { metrohash128_2_test,     128, 0x5437C684, "metrohash128_2",    "MetroHash128_2 for 64-bit", POOR },
#if defined(__SSE4_2__) && defined(__x86_64__)
  { metrohash64crc_1_test,    64, 0x29C68A50, "metrohash64crc_1",  "MetroHash64crc_1 for x64", POOR },
  { metrohash64crc_2_test,    64, 0x2C00BD9F, "metrohash64crc_2",  "MetroHash64crc_2 for x64", POOR },
  { metrohash128crc_1_test,  128, 0x5E75144E, "metrohash128crc_1", "MetroHash128crc_1 for x64", GOOD },
  { metrohash128crc_2_test,  128, 0x1ACF3E77, "metrohash128crc_2", "MetroHash128crc_2 for x64", GOOD },
#endif
#if defined(__SSE4_2__)
  { cmetrohash64_1_optshort_test,64, 0xEE88F7D2, "cmetrohash64_1o", "cmetrohash64_1 (shorter key optimized) , 64-bit for x64", POOR },
  { cmetrohash64_1_test,         64, 0xEE88F7D2, "cmetrohash64_1",  "cmetrohash64_1, 64-bit for x64", POOR },
  { cmetrohash64_2_test,         64, 0xE1FC7C6E, "cmetrohash64_2",  "cmetrohash64_2, 64-bit for x64", POOR },
#endif
#if defined(__SSE4_2__) && defined(__x86_64__)
  { falkhash_test_cxx,           64, 0x2F99B071, "falkhash",        "falkhash.asm with aesenc, 64-bit for x64", POOR },
  { clhash_test,                 64, 0xFF27B919, "clhash",          "carry-less mult. hash -DBITMIX (64-bit for x64, SSE4.2)", GOOD },
#endif
  { t1ha2_atonce_test,           64, 0x8F16C948, "t1ha2_atonce",    "Fast Positive Hash (portable, aims 64-bit, little-endian)", GOOD },
  { t1ha2_stream_test,           64, 0xDED9B580, "t1ha2_stream",    "Fast Positive Hash (portable, aims 64-bit, little-endian)", GOOD },
  { t1ha2_atonce128_test,       128, 0xB44C43A1, "t1ha2_atonce128", "Fast Positive Hash (portable, aims 64-bit, little-endian)", POOR },
  { t1ha2_stream128_test,       128, 0xE929E756, "t1ha2_stream128", "Fast Positive Hash (portable, aims 64-bit, little-endian)", POOR },
#if 0
  { t1ha1_64le_test,             64, 0xD6836381, "t1ha1_64le",      "Fast Positive Hash (portable, aims 64-bit, little-endian)", GOOD },
  { t1ha1_64be_test,             64, 0x93F864DE, "t1ha1_64be",      "Fast Positive Hash (portable, aims 64-bit, big-engian)", GOOD },
  { t1ha0_32le_test,             64, 0x7F7D7B29, "t1ha0_32le",      "Fast Positive Hash (portable, aims 32-bit, little-endian)", GOOD },
  { t1ha0_32be_test,             64, 0xDA6A4061, "t1ha0_32be",      "Fast Positive Hash (portable, aims 32-bit, big-endian)", GOOD },
# if T1HA0_AESNI_AVAILABLE
  { t1ha0_ia32aes_noavx_test,    64, 0xF07C4DA5, "t1ha0_aes_noavx", "Fast Positive Hash (machine-specific, requires AES-NI)", GOOD },
#  if defined(__AVX__)
  { t1ha0_ia32aes_avx1_test,     64, 0xF07C4DA5, "t1ha0_aes_avx1",  "Fast Positive Hash (machine-specific, requires AES-NI & AVX)", GOOD },
#  endif /* __AVX__ */
#  if defined(__AVX2__)
  { t1ha0_ia32aes_avx2_test,     64, 0x8B38C599, "t1ha0_aes_avx2",  "Fast Positive Hash (machine-specific, requires AES-NI & AVX)", GOOD },
#  endif /* __AVX2__ */
# endif /* T1HA0_AESNI_AVAILABLE */
#endif /* older t1ha */
  { wyhash_test,                 64, 0x1196BB84, "wyhash",          "wyhash (portable, 64-bit, little-endian)", GOOD },
  { wyhash32low,                 32, 0x2199CEDD, "wyhash32low",     "wyhash - lower 32bit", GOOD }
};

HashInfo * findHash ( const char * name )
{
  for(size_t i = 0; i < sizeof(g_hashes) / sizeof(HashInfo); i++)
  {
    if(_stricmp(name,g_hashes[i].name) == 0) return &g_hashes[i];
  }

  return NULL;
}

//-----------------------------------------------------------------------------
// Self-test on startup - verify that all installed hashes work correctly.

void SelfTest ( void )
{
  bool pass = true;

  for(size_t i = 0; i < sizeof(g_hashes) / sizeof(HashInfo); i++)
  {
    HashInfo * info = & g_hashes[i];

    pass &= VerificationTest(info->hash,info->hashbits,info->verification,
                             false);
  }

  if(!pass)
  {
    printf("Self-test FAILED!\n");

    for(size_t i = 0; i < sizeof(g_hashes) / sizeof(HashInfo); i++)
    {
      HashInfo * info = & g_hashes[i];

      printf("%20s - ",info->name);
      pass &= VerificationTest(info->hash,info->hashbits,info->verification,
                               true);
    }

    exit(1);
  }
}

//----------------------------------------------------------------------------

template < typename hashtype >
void test ( hashfunc<hashtype> hash, HashInfo* info )
{
  const int hashbits = sizeof(hashtype) * 8;

  printf("-------------------------------------------------------------------------------\n");

  // eventual initializers
#if defined(__SSE4_2__) && defined(__x86_64__)
  if(info->hash == clhash_test)
    clhash_init();
#endif
#ifdef HAVE_HIGHWAYHASH
  if(info->hash == HighwayHash64_test)
    HighwayHash_init();
#endif

  //-----------------------------------------------------------------------------
  // Sanity tests

  if(g_testVerifyAll)
  {
    printf("[[[ VerifyAll Tests ]]]\n\n"); fflush(NULL);
    SelfTest();
    printf("PASS\n\n"); fflush(NULL); // if not it does exit(1)
  }

  printf("--- Testing %s \"%s\" %s\n\n", info->name, info->desc, quality_str[info->quality]);
  fflush(NULL);

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
    sum = sum / 31.0;
    printf("Average                                    %6.3f cycles/hash\n",sum);
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

  if(g_testWindow || g_testAll)
  {
    printf("[[[ Keyset 'Window' Tests ]]]\n\n");

    bool result = true;
    bool testCollision = true;
    bool testDistribution = g_testExtra;
    const int windowbits = info->hashbits > 64 ? 18 : 20;

    result &= WindowedKeyTest< Blob<hashbits*2+2>, hashtype >
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

    bool result = true;

#if 0
    result &= CyclicKeyTest<hashtype>(hash,sizeof(hashtype)+0,8,100000, g_drawDiagram);
#else
    result &= CyclicKeyTest<hashtype>(hash,sizeof(hashtype)+0,8,1000000, g_drawDiagram);
    result &= CyclicKeyTest<hashtype>(hash,sizeof(hashtype)+1,8,1000000, g_drawDiagram);
    result &= CyclicKeyTest<hashtype>(hash,sizeof(hashtype)+2,8,1000000, g_drawDiagram);
    result &= CyclicKeyTest<hashtype>(hash,sizeof(hashtype)+3,8,1000000, g_drawDiagram);
    result &= CyclicKeyTest<hashtype>(hash,sizeof(hashtype)+4,8,1000000, g_drawDiagram);
    result &= CyclicKeyTest<hashtype>(hash,sizeof(hashtype)+8,8,1000000, g_drawDiagram);
#endif
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
    }

    for(int i = 4; i <= maxlen; i += 4)
    {
      result &= TwoBytesTest2<hashtype>(hash, i, g_drawDiagram);
    }

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
  if(g_testMomentChi2 || g_testAll)
  {
    printf("[[[ 'MomentChi2' Tests ]]]\n\n");

    bool result = true;

    result &= MomentChi2Test(info);

    if(!result) printf("*********FAIL*********\n");
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
    printf("[[[ 'LongNeighbors' Tests ]]]\n\n");

    bool result = true;

    result &= testLongNeighbors(info->hash, info->hashbits, g_drawDiagram);

    if(!result) printf("*********FAIL*********\n");
    printf("\n");
    fflush(NULL);
  }
#endif

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
  // Differential tests
  // 5m30 with xxh3

  if(g_testDiff || g_testAll)
  {
    printf("[[[ Diff 'Differential' Tests ]]]\n\n");
    fflush(NULL);

    bool result = true;
    bool dumpCollisions = g_drawDiagram; // from --verbose

    result &= DiffTest< Blob<64>,  hashtype >(hash,5,1000,dumpCollisions);
    result &= DiffTest< Blob<128>, hashtype >(hash,4,1000,dumpCollisions);
    result &= DiffTest< Blob<256>, hashtype >(hash,3,1000,dumpCollisions);

    if(!result) printf("*********FAIL*********\n");
    printf("\n");
    fflush(NULL);
  }

  //-----------------------------------------------------------------------------
  // Differential-distribution tests
  // 2m40 with xxh3

  if(g_testDiffDist || g_testAll)
  {
    printf("[[[ DiffDist 'Differential Distribution' Tests ]]]\n\n");
    fflush(NULL);

    bool result = true;

    result &= DiffDistTest2<uint64_t,hashtype>(hash, g_drawDiagram);

    if(!result) printf("*********FAIL*********\n");
    printf("\n");
    fflush(NULL);
  }

  //-----------------------------------------------------------------------------
  // Bit Independence Criteria. Interesting, but doesn't tell us much about
  // collision or distribution. For 128bit hashes only with --extra
  // 4m with xxh3
  // 152m with farmhash128_c with reps=1000000, => 8m with 100000

  if(g_testBIC || (info->hashbits > 64 && g_testExtra))
  {
    printf("[[[ 'BIC' (Bit Independence Criteria) Tests ]]]\n\n");
    fflush(NULL);

    bool result = true;
    if (info->hashbits > 64) {
      result &= BicTest3<Blob<128>,hashtype>(hash,100000,g_drawDiagram);
    } else {
      const long reps = 64000000/info->hashbits;
      //result &= BicTest<uint64_t,hashtype>(hash,2000000);
      result &= BicTest3<Blob<88>,hashtype>(hash,(int)reps,g_drawDiagram);
    }

    if(!result) printf("*********FAIL*********\n");
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
  g_inputVCode = MurmurOAAT(key, len, g_inputVCode);
  g_inputVCode = MurmurOAAT(&seed, sizeof(uint32_t), g_inputVCode);

  g_hashUnderTest->hash(key, len, seed, out);

  g_outputVCode = MurmurOAAT(out, g_hashUnderTest->hashbits/8, g_outputVCode);
}

bool MomentChi2Test ( struct HashInfo *info )
{
  pfHash hash = info->hash;
  const int size = info->hashbits / 8;
  const int step = 3;
  unsigned k = 0, s = 0;
  unsigned long l, h, x;
  unsigned mx = 0xfffffff0;
  long double sa=0, saa=0, sb=0, sbb=0,	n = mx/step;
  hash(&k,sizeof(k),s,&l);
  printf("Running 1st unseeded MomentChi2 for the low 32bits/step %d ... ", step);
  fflush(NULL);
  for(unsigned i=1; i<=mx; i+=step){
    hash(&i,sizeof(i),s,&h);
    x = popcount8(l^h); // check the lower 32bits only
    x = x*x*x*x*x;
    sa+=x; saa+=x*x; l=h;
  }
  sa/=n; saa=(saa/n-sa*sa)/n;
  printf("%Lf - %Lf\n", sa, saa);
  printf("Running 2nd   seeded MomentChi2 for the low 32bits/step %d ... ", step);
  fflush(NULL);
  hash(&k,sizeof(k),s,&l);
  for(unsigned i=1; i<=mx; i+=step){
    hash(&k,sizeof(k),i,&h);
    x = popcount8(l^h);
    x = x*x*x*x*x;
    sb+=x; sbb+=x*x; l=h;
  }
  sb/=n; sbb=(sbb/n-sb*sb)/n;

  double chi2=(sa-sb)*(sa-sb)/(saa+sbb);
  printf("%Lf - %Lf\nKeySeedMomentChi2:\t%g\t", sb, sbb, chi2);
  fflush(NULL);
  if (chi2 > 3.84145882069413)
  {
    printf("FAIL!!!!\n");
    fflush(NULL);
    return false;
  }
  else
  {
    printf("PASS\n");
    fflush(NULL);
    return true;
  }
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
#if (defined(__x86_64__) && __SSE4_2__) || defined(_M_X64) || defined(_X86_64_)
  const char * defaulthash = "xxh3"; // because it fails some tests
#else
  const char * defaulthash = "wyhash";
#endif
  const char * hashToTest = defaulthash;

  if(argc < 2) {
    printf("No test hash given on command line, testing %s.\n", hashToTest);
    printf("Usage: SMHasher [--list][--tests] [--verbose][--extra]\n"
           " or --test=Speed,... hash\n");
  }
  else {
    int i = 1;
    hashToTest = argv[i];

    if (strncmp(hashToTest,"--", 2) == 0) {
      if (strcmp(hashToTest,"--help") == 0) {
        printf("Usage: SMHasher [--list][--tests] [--verbose][--extra]\n"
               " or --test=Speed,... hash\n");
        exit(0);
      }
      if (strcmp(hashToTest,"--list") == 0) {
        for(size_t i = 0; i < sizeof(g_hashes) / sizeof(HashInfo); i++) {
          printf("%-16s\t\"%s\" %s\n", g_hashes[i].name, g_hashes[i].desc, quality_str[g_hashes[i].quality]);
        }
        exit(0);
      }
      if (strcmp(hashToTest,"--tests") == 0) {
        printf("Valid tests:\n");
        for(size_t i = 0; i < sizeof(g_testopts) / sizeof(TestOpts); i++) {
          printf("  %s\n", g_testopts[i].name);
        }
        exit(0);
      }
      if (strcmp(hashToTest,"--verbose") == 0) {
        g_drawDiagram = true;
        i++;
        if (argc > i)
          hashToTest = argv[i];
        else
          hashToTest = defaulthash;
      }
      if (strcmp(hashToTest,"--extra") == 0) {
        g_testExtra = true;
        i++;
        if (argc > i)
          hashToTest = argv[i];
        else
          hashToTest = defaulthash;
      }
      /* default: --test=All. comma seperated list of options */
      if (strncmp(hashToTest,"--test=", 6) == 0) {
        char *opt = (char *)&hashToTest[7];
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
      i++;
      if (argc > i)
        hashToTest = argv[i];
      else
        hashToTest = defaulthash;
    }
  }

  // Code runs on the 3rd CPU by default
  //SetAffinity((1 << 2));
  //SelfTest();

  int timeBegin = clock();

  testHash(hashToTest);

  int timeEnd = clock();

  printf("\n");
  fflush(NULL);
  printf("Input vcode 0x%08x, Output vcode 0x%08x, Result vcode 0x%08x\n", g_inputVCode, g_outputVCode, g_resultVCode);
  printf("Verification value is 0x%08x - Testing took %f seconds\n", g_verify, double(timeEnd-timeBegin)/double(CLOCKS_PER_SEC));
  printf("-------------------------------------------------------------------------------\n");
  fflush(NULL);
  return 0;
}
