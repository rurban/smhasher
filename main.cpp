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

bool g_testAll = true;

bool g_testSanity      = false;
bool g_testSpeed       = false;
bool g_testDiff        = false;
bool g_testDiffDist    = false;
bool g_testAvalanche   = false;
bool g_testBIC         = false;
bool g_testCyclic      = false;
bool g_testTwoBytes    = false;
bool g_testSparse      = false;
bool g_testPermutation = false;
bool g_testWindow      = false;
bool g_testText        = false;
bool g_testZeroes      = false;
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
  { g_testSeed,		"Seed" }
};

//-----------------------------------------------------------------------------
// This is the list of all hashes that SMHasher can test.

struct HashInfo
{
  pfHash hash;
  int hashbits;
  uint32_t verification;
  const char * name;
  const char * desc;
};

// sorted by quality and speed
HashInfo g_hashes[] =
{
  // first the bad hash funcs, failing tests:
  { DoNothingHash,        32, 0x00000000, "donothing32", "Do-Nothing function (only valid for measuring call overhead)" },
  { DoNothingHash,        64, 0x00000000, "donothing64", "Do-Nothing function (only valid for measuring call overhead)" },
  { DoNothingHash,       128, 0x00000000, "donothing128", "Do-Nothing function (only valid for measuring call overhead)" },
  { NoopOAATReadHash,     64, 0x00000000, "NOP_OAAT_read64", "Noop function (only valid for measuring call + OAAT reading overhead)" },
  { BadHash,     	  32, 0xAB432E23, "BadHash", 	 "very simple XOR shift" },
  { sumhash,     	  32, 0x0000A9AC, "sumhash", 	 "sum all bytes" },
  { sumhash32,     	  32, 0xF5562C80, "sumhash32",   "sum all 32bit words" },

  // here start the real hashes. the problematic ones:
  { crc32,                32, 0x3719DB20, "crc32",       "CRC-32 soft" },

  { md5_32,               32, 0xF7192210, "md5_32a",     "MD5, first 32 bits of result" },
#ifdef _MSC_VER /* truncated long to 32 */
#  define SHA1_SEED           0xDCB02360
#else
#  define SHA1_SEED           0x7FE8C80E
#endif
  { sha1_32a,             32, SHA1_SEED,  "sha1_32a",    "SHA1, first 32 bits of result" },
#if 0
  { sha1_64a,             32, 0x00000000, "sha1_64a",    "SHA1 64-bit, first 64 bits of result" },
  { sha2_32a,             32, 0x00000000, "sha2_32a",    "SHA2, first 32 bits of result" },
  { sha2_64a,             64, 0x00000000, "sha2_64a",    "SHA2, first 64 bits of result" },
  { sha3_32a,             32, 0x00000000, "sha3_32a",    "SHA3, first 32 bits of result" },
  { sha3_64a,             64, 0x00000000, "sha3_64a",    "SHA3, first 64 bits of result" },
  { BLAKE2_32a,           32, 0x00000000, "blake2_32a",  "BLAKE2, first 32 bits of result" },
  { BLAKE2_64a,           64, 0x00000000, "blake2_64a",  "BLAKE2, first 64 bits of result" },
  { bcrypt_64a,           64, 0x00000000, "bcrypt_64a",  "bcrypt, first 64 bits of result" },
  { scrypt_64a,           64, 0x00000000, "scrypt_64a",  "scrypt, first 64 bits of result" },
#endif

#ifdef __SSE2__
  { hasshe2_test,        256, 0xF5D39DFE, "hasshe2",     "SSE2 hasshe2, 256-bit" },
#endif
#if defined(__SSE4_2__) && defined(__x86_64__)
  /* Even 32 uses crc32q, quad only */
  { crc32c_hw_test,       32, 0x0C7346F0, "crc32_hw",    "SSE4.2 crc32 in HW" },
  { crc32c_hw1_test,      32, 0x0C7346F0, "crc32_hw1",   "Faster Adler SSE4.2 crc32 in HW" },
  { crc64c_hw_test,       64, 0xE7C3FD0E, "crc64_hw",    "SSE4.2 crc64 in HW" },
#endif
#if 0 && defined(__x86_64__) && (defined(__linux__) || defined(__APPLE__))
  // elf64 or macho64 only
  { fhtw_test,            64, 0x0,        "fhtw",        "fhtw asm" },
#endif
  { FNV32a,               32, 0xE3CBBE91, "FNV1a",       "Fowler-Noll-Vo hash, 32-bit" },
  { FNV32a_YoshimitsuTRIAD,32,0xD8AFFD71, "FNV1a_YT",    "FNV1a-YoshimitsuTRIAD 32-bit sanmayce" },
  { FNV64a,               64, 0x103455FC, "FNV64",       "Fowler-Noll-Vo hash, 64-bit" },
#if 0 /* TODO */
  { fletcher2,            64, 0x0, "fletcher2",  "fletcher2 ZFS"} //TODO
  { fletcher4,            64, 0x0, "fletcher4",  "fletcher4 ZFS"} //TODO
  { Jesteress,            32, 0x0, "Jesteress",  "FNV1a-Jesteress 32-bit sanmayce" },
  { Meiyan,       	  32, 0x0, "Meiyan",     "FNV1a-Meiyan 32-bit sanmayce" },
#endif
  { Bernstein,            32, 0xBDB4B640, "bernstein",   "Bernstein, 32-bit" },
  { sdbm,                 32, 0x582AF769, "sdbm",        "sdbm as in perl5" },
  { x17_test,             32, 0x8128E14C, "x17",         "x17" },
  // also called jhash:
  { JenkinsOOAT,          32, 0x83E133DA, "JenkinsOOAT", "Bob Jenkins' OOAT as in perl 5.18" },
  { JenkinsOOAT_perl,     32, 0xEE05869B, "JenkinsOOAT_perl", "Bob Jenkins' OOAT as in old perl5" },
  { MicroOAAT,            32, 0x16F1BA97, "MicroOAAT", "Small non-multiplicative OAAT that passes all collision checks (by funny-falcon)" },
  { jodyhash32_test,      32, 0xFB47D60D, "jodyhash32",  "jodyhash, 32-bit (v5)" },
  { jodyhash64_test,      64, 0x9F09E57F, "jodyhash64",  "jodyhash, 64-bit (v5)" },
  { lookup3_test,         32, 0x3D83917A, "lookup3",     "Bob Jenkins' lookup3" },
  { SuperFastHash,        32, 0x980ACD1D, "superfast",   "Paul Hsieh's SuperFastHash" },
  { MurmurOAAT_test,      32, 0x5363BD98, "MurmurOAAT",  "Murmur one-at-a-time" },
  { Crap8_test,           32, 0x743E97A1, "Crap8",       "Crap8" },
  { MurmurHash2_test,     32, 0x27864C1E, "Murmur2",     "MurmurHash2 for x86, 32-bit" },
  { MurmurHash2A_test,    32, 0x7FBD4396, "Murmur2A",    "MurmurHash2A for x86, 32-bit" },
#if __WORDSIZE >= 64
  { MurmurHash64A_test,   64, 0x1F0D3804, "Murmur2B",    "MurmurHash2 for x64, 64-bit" },
#endif
  { MurmurHash64B_test,   64, 0xDD537C05, "Murmur2C",    "MurmurHash2 for x86, 64-bit" },
  { halfsiphash_test,     32, 0xA7A05F72, "HalfSipHash", "HalfSipHash 2-4, 32bit" },
  // as in rust and swift
  { siphash13_test,       64, 0x29C010BF, "SipHash13",   "SipHash 1-3 - SSSE3 optimized" },

  // and now the quality hash funcs
  { siphash_test,         64, 0xC58D7F9C, "SipHash",     "SipHash 2-4 - SSSE3 optimized" },
  { GoodOAAT,             32, 0x7B14EEE5, "GoodOAAT",    "Small non-multiplicative OAAT" },
  // TODO: Google HighwayHash
  { PMurHash32_test,      32, 0xB0F57EE3, "PMurHash32",  "Shane Day's portable-ized MurmurHash3 for x86, 32-bit." },
  { MurmurHash3_x86_32,   32, 0xB0F57EE3, "Murmur3A",    "MurmurHash3 for x86, 32-bit" },
  { MurmurHash3_x86_128, 128, 0xB3ECE62A, "Murmur3C",    "MurmurHash3 for x86, 128-bit" },
#if __WORDSIZE >= 64
  { MurmurHash3_x64_128, 128, 0x6384BA69, "Murmur3F",    "MurmurHash3 for x64, 128-bit" },
#endif
  { fasthash32_test,      32, 0xE9481AFC, "fasthash32",  "fast-hash 32bit" },
  { fasthash64_test,      64, 0xA16231A7, "fasthash64",  "fast-hash 64bit" },
#if defined(__GNUC__) && UINT_MAX != ULONG_MAX
 #define MUM_SEED             0x3EEAE2D4
#else
 #define MUM_SEED             0xA973C6C0
#endif
  { mum_hash_test,        64, MUM_SEED,   "MUM",         "github.com/vnmakarov/mum-hash" },
  { CityHash32_test,      32, 0x5C28AD62, "City32",      "Google CityHash32WithSeed (old)" },
  { CityHash64_test,      64, 0x25A20825, "City64",      "Google CityHash64WithSeed (old)" },
#if defined(__SSE4_2__) && defined(__x86_64__)
  { CityHash128_test,    128, 0x6531F54E, "City128",     "Google CityHash128WithSeed (old)" },
  { CityHashCrc128_test, 128, 0xD4389C97, "CityCrc128",  "Google CityHashCrc128WithSeed SSE4.2 (old)" },
#endif
# ifdef _MSC_VER /* truncated long to 32 */
#  define FARM64_SEED         0xEBC4A679
#  define FARM128_SEED        0x305C0D9A
# else
#  define FARM64_SEED         0x35F84A93
#  define FARM128_SEED        0x9E636AAE
# endif
  { FarmHash64_test,      64, FARM64_SEED, "FarmHash64",  "Google FarmHash64WithSeed" },
  { FarmHash128_test,    128, FARM128_SEED,"FarmHash128", "Google FarmHash128WithSeed" },
#if defined(__SSE4_2__) && defined(__x86_64__)
  { farmhash64_c_test,    64, FARM64_SEED, "farmhash64_c",  "farmhash64_with_seed (C99)" },
  { farmhash128_c_test,  128, FARM128_SEED,"farmhash128_c", "farmhash128_with_seed (C99)" },
#endif
  { xxHash32_test,        32, 0xBA88B743, "xxHash32",    "xxHash, 32-bit for x64" },
  { xxHash64_test,        64, 0x024B7CF4, "xxHash64",    "xxHash, 64-bit" },
#if 0
  { xxhash256_test,       64, 0x024B7CF4, "xxhash256",   "xxhash256, 64-bit unportable" },
#endif
  { SpookyHash32_test,    32, 0x3F798BBB, "Spooky32",    "Bob Jenkins' SpookyHash, 32-bit result" },
  { SpookyHash64_test,    64, 0xA7F955F1, "Spooky64",    "Bob Jenkins' SpookyHash, 64-bit result" },
  { SpookyHash128_test,  128, 0x8D263080, "Spooky128",   "Bob Jenkins' SpookyHash, 128-bit result" },
  { metrohash64_1_test,       64, 0xEE88F7D2, "metrohash64_1",     "MetroHash64_1 for 64-bit" },
  { metrohash64_2_test,       64, 0xE1FC7C6E, "metrohash64_2",     "MetroHash64_2 for 64-bit" },
  { metrohash128_1_test,     128, 0x20E8A1D7, "metrohash128_1",    "MetroHash128_1 for 64-bit" },
  { metrohash128_2_test,     128, 0x5437C684, "metrohash128_2",    "MetroHash128_2 for 64-bit" },
#if defined(__SSE4_2__) && defined(__x86_64__)
  { metrohash64crc_1_test,    64, 0x29C68A50, "metrohash64crc_1",  "MetroHash64crc_1 for x64" },
  { metrohash64crc_2_test,    64, 0x2C00BD9F, "metrohash64crc_2",  "MetroHash64crc_2 for x64" },
  { metrohash128crc_1_test,  128, 0x5E75144E, "metrohash128crc_1", "MetroHash128crc_1 for x64" },
  { metrohash128crc_2_test,  128, 0x1ACF3E77, "metrohash128crc_2", "MetroHash128crc_2 for x64" },
#endif
#if defined(__SSE4_2__)
  { cmetrohash64_1_optshort_test, 64, 0xEE88F7D2, "cmetrohash64_1o", "cmetrohash64_1 (shorter key optimized) , 64-bit for x64" },
  { cmetrohash64_1_test,        64, 0xEE88F7D2, "cmetrohash64_1",    "cmetrohash64_1, 64-bit for x64" },
  { cmetrohash64_2_test,        64, 0xE1FC7C6E, "cmetrohash64_2",    "cmetrohash64_2, 64-bit for x64" },
#endif
#if defined(__SSE4_2__) && defined(__x86_64__)
  { falkhash_test_cxx,          64, 0x2F99B071, "falkhash",          "falkhash.asm with aesenc, 64-bit for x64" },
#endif
  { t1ha2_atonce_test,           64, 0x8F16C948, "t1ha2_atonce",     "Fast Positive Hash (portable, aims 64-bit, little-endian)" },
  { t1ha2_stream_test,           64, 0xDED9B580, "t1ha2_stream",     "Fast Positive Hash (portable, aims 64-bit, little-endian)" },
  { t1ha2_atonce128_test,       128, 0xB44C43A1, "t1ha2_atonce128",  "Fast Positive Hash (portable, aims 64-bit, little-endian)" },
  { t1ha2_stream128_test,       128, 0xE929E756, "t1ha2_stream128",  "Fast Positive Hash (portable, aims 64-bit, little-endian)" },
  { t1ha1_64le_test,             64, 0xD6836381, "t1ha1_64le",       "Fast Positive Hash (portable, aims 64-bit, little-endian)" },
  { t1ha1_64be_test,             64, 0x93F864DE, "t1ha1_64be",       "Fast Positive Hash (portable, aims 64-bit, big-engian)" },
  { t1ha0_32le_test,             64, 0x7F7D7B29, "t1ha0_32le",       "Fast Positive Hash (portable, aims 32-bit, little-endian)" },
  { t1ha0_32be_test,             64, 0xDA6A4061, "t1ha0_32be",       "Fast Positive Hash (portable, aims 32-bit, big-endian)" },
#if T1HA0_AESNI_AVAILABLE
  { t1ha0_ia32aes_noavx_test,    64, 0xF07C4DA5, "t1ha0_aes_noavx",  "Fast Positive Hash (machine-specific, requires AES-NI)" },
#if defined(__AVX__)
  { t1ha0_ia32aes_avx1_test,     64, 0xF07C4DA5, "t1ha0_aes_avx1",   "Fast Positive Hash (machine-specific, requires AES-NI & AVX)" },
#endif /* __AVX__ */
#if defined(__AVX2__)
  { t1ha0_ia32aes_avx2_test,     64, 0x8B38C599, "t1ha0_aes_avx2",   "Fast Positive Hash (machine-specific, requires AES-NI & AVX)" },
#endif /* __AVX2__ */
#endif /* T1HA0_AESNI_AVAILABLE */
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

    pass &= VerificationTest(info->hash,info->hashbits,info->verification,false);
  }

  if(!pass)
  {
    printf("Self-test FAILED!\n");

    for(size_t i = 0; i < sizeof(g_hashes) / sizeof(HashInfo); i++)
    {
      HashInfo * info = & g_hashes[i];

      printf("%16s - ",info->name);
      pass &= VerificationTest(info->hash,info->hashbits,info->verification,true);
    }

    exit(1);
  }
}

//----------------------------------------------------------------------------

template < typename hashtype >
void test ( hashfunc<hashtype> hash, HashInfo * info )
{
  const int hashbits = sizeof(hashtype) * 8;

  printf("-------------------------------------------------------------------------------\n");
  printf("--- Testing %s \"%s\"\n\n",info->name,info->desc);
  fflush(NULL);

  //-----------------------------------------------------------------------------
  // Sanity tests

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
  // Differential tests

  if(g_testDiff || g_testAll)
  {
    printf("[[[ Differential Tests ]]]\n\n");
    fflush(NULL);

    bool result = true;
    bool dumpCollisions = false;

    result &= DiffTest< Blob<64>,  hashtype >(hash,5,1000,dumpCollisions);
    result &= DiffTest< Blob<128>, hashtype >(hash,4,1000,dumpCollisions);
    result &= DiffTest< Blob<256>, hashtype >(hash,3,1000,dumpCollisions);

    if(!result) printf("*********FAIL*********\n");
    printf("\n");
    fflush(NULL);
  }

  //-----------------------------------------------------------------------------
  // Differential-distribution tests

  if(g_testDiffDist /*|| g_testAll*/)
  {
    printf("[[[ Differential Distribution Tests ]]]\n\n");
    fflush(NULL);

    bool result = true;

    result &= DiffDistTest2<uint64_t,hashtype>(hash);

    printf("\n");
    fflush(NULL);
  }

  //-----------------------------------------------------------------------------
  // Avalanche tests

  if(g_testAvalanche || g_testAll)
  {
    printf("[[[ Avalanche Tests ]]]\n\n");
    fflush(NULL);

    bool result = true;

    result &= AvalancheTest< Blob< 32>, hashtype > (hash,300000);
    result &= AvalancheTest< Blob< 40>, hashtype > (hash,300000);
    result &= AvalancheTest< Blob< 48>, hashtype > (hash,300000);
    result &= AvalancheTest< Blob< 56>, hashtype > (hash,300000);

    result &= AvalancheTest< Blob< 64>, hashtype > (hash,300000);
    result &= AvalancheTest< Blob< 72>, hashtype > (hash,300000);
    result &= AvalancheTest< Blob< 80>, hashtype > (hash,300000);
    result &= AvalancheTest< Blob< 88>, hashtype > (hash,300000);

    result &= AvalancheTest< Blob< 96>, hashtype > (hash,300000);
    result &= AvalancheTest< Blob<104>, hashtype > (hash,300000);
    result &= AvalancheTest< Blob<112>, hashtype > (hash,300000);
    result &= AvalancheTest< Blob<120>, hashtype > (hash,300000);

    result &= AvalancheTest< Blob<128>, hashtype > (hash,300000);
    result &= AvalancheTest< Blob<136>, hashtype > (hash,300000);
    result &= AvalancheTest< Blob<144>, hashtype > (hash,300000);
    result &= AvalancheTest< Blob<152>, hashtype > (hash,300000);

    if(!result) printf("*********FAIL*********\n");
    printf("\n");
    fflush(NULL);
  }

  //-----------------------------------------------------------------------------
  // Bit Independence Criteria. Interesting, but doesn't tell us much about
  // collision or distribution.

  if(g_testBIC)
  {
    printf("[[[ Bit Independence Criteria ]]]\n\n");
    fflush(NULL);

    bool result = true;

    //result &= BicTest<uint64_t,hashtype>(hash,2000000);
    BicTest3<Blob<88>,hashtype>(hash,2000000);

    if(!result) printf("*********FAIL*********\n");
    printf("\n");
    fflush(NULL);
  }

  //-----------------------------------------------------------------------------
  // Keyset 'Cyclic' - keys of the form "abcdabcdabcd..."

  if(g_testCyclic || g_testAll)
  {
    printf("[[[ Keyset 'Cyclic' Tests ]]]\n\n");
    fflush(NULL);

    bool result = true;
    bool drawDiagram = false;

#if 0
    result &= CyclicKeyTest<hashtype>(hash,sizeof(hashtype)+0,8,100000,drawDiagram);
#else
    result &= CyclicKeyTest<hashtype>(hash,sizeof(hashtype)+0,8,10000000,drawDiagram);
    result &= CyclicKeyTest<hashtype>(hash,sizeof(hashtype)+1,8,10000000,drawDiagram);
    result &= CyclicKeyTest<hashtype>(hash,sizeof(hashtype)+2,8,10000000,drawDiagram);
    result &= CyclicKeyTest<hashtype>(hash,sizeof(hashtype)+3,8,10000000,drawDiagram);
    result &= CyclicKeyTest<hashtype>(hash,sizeof(hashtype)+4,8,10000000,drawDiagram);
#endif
    if(!result) printf("*********FAIL*********\n");
    printf("\n");
    fflush(NULL);
  }

  //-----------------------------------------------------------------------------
  // Keyset 'TwoBytes' - all keys up to N bytes containing two non-zero bytes

  // This generates some huge keysets, 128-bit tests will take ~1.3 gigs of RAM.

  if(g_testTwoBytes || g_testAll)
  {
    printf("[[[ Keyset 'TwoBytes' Tests ]]]\n\n");
    fflush(NULL);

    bool result = true;
    bool drawDiagram = false;

    for(int i = 4; i <= 20; i += 4)
    {
      result &= TwoBytesTest2<hashtype>(hash,i,drawDiagram);
    }

    if(!result) printf("*********FAIL*********\n");
    printf("\n");
    fflush(NULL);
  }

  //-----------------------------------------------------------------------------
  // Keyset 'Sparse' - keys with all bits 0 except a few

  if(g_testSparse || g_testAll)
  {
    printf("[[[ Keyset 'Sparse' Tests ]]]\n\n");
    fflush(NULL);

    bool result = true;
    bool drawDiagram = false;

    result &= SparseKeyTest<  32,hashtype>(hash,6,true,true,true,drawDiagram);
    result &= SparseKeyTest<  40,hashtype>(hash,6,true,true,true,drawDiagram);
    result &= SparseKeyTest<  48,hashtype>(hash,5,true,true,true,drawDiagram);
    result &= SparseKeyTest<  56,hashtype>(hash,5,true,true,true,drawDiagram);
    result &= SparseKeyTest<  64,hashtype>(hash,5,true,true,true,drawDiagram);
    result &= SparseKeyTest<  96,hashtype>(hash,4,true,true,true,drawDiagram);
    result &= SparseKeyTest< 256,hashtype>(hash,3,true,true,true,drawDiagram);
    result &= SparseKeyTest<2048,hashtype>(hash,2,true,true,true,drawDiagram);

    if(!result) printf("*********FAIL*********\n");
    printf("\n");
    fflush(NULL);
  }

  //-----------------------------------------------------------------------------
  // Keyset 'Permutation' - all possible combinations of a set of blocks

  if(g_testPermutation || g_testAll)
  {
    {
      // This one breaks lookup3, surprisingly

      printf("[[[ Keyset 'Combination Lowbits' Tests ]]]\n\n");
      fflush(NULL);

      bool result = true;
      bool drawDiagram = false;

      uint32_t blocks[] =
      {
        0x00000000,

        0x00000001, 0x00000002, 0x00000003, 0x00000004, 0x00000005, 0x00000006, 0x00000007,
      };

      result &= CombinationKeyTest<hashtype>(hash,8,blocks,sizeof(blocks) / sizeof(uint32_t),true,true,drawDiagram);

      if(!result) printf("*********FAIL*********\n");
      printf("\n");
      fflush(NULL);
    }

    {
      printf("[[[ Keyset 'Combination Highbits' Tests ]]]\n\n");
      fflush(NULL);

      bool result = true;
      bool drawDiagram = false;

      uint32_t blocks[] =
      {
        0x00000000,

        0x20000000, 0x40000000, 0x60000000, 0x80000000, 0xA0000000, 0xC0000000, 0xE0000000
      };

      result &= CombinationKeyTest<hashtype>(hash,8,blocks,sizeof(blocks) / sizeof(uint32_t),true,true,drawDiagram);

      if(!result) printf("*********FAIL*********\n");
      printf("\n");
      fflush(NULL);
    }

    {
      printf("[[[ Keyset 'Combination 0x8000000' Tests ]]]\n\n");
      fflush(NULL);

      bool result = true;
      bool drawDiagram = false;

      uint32_t blocks[] =
      {
        0x00000000,

        0x80000000,
      };

      result &= CombinationKeyTest<hashtype>(hash,20,blocks,sizeof(blocks) / sizeof(uint32_t),true,true,drawDiagram);

      if(!result) printf("*********FAIL*********\n");
      printf("\n");
      fflush(NULL);
    }

    {
      printf("[[[ Keyset 'Combination 0x0000001' Tests ]]]\n\n");

      bool result = true;
      bool drawDiagram = false;

      uint32_t blocks[] =
      {
        0x00000000,

        0x00000001,
      };

      result &= CombinationKeyTest<hashtype>(hash,20,blocks,sizeof(blocks) / sizeof(uint32_t),true,true,drawDiagram);

      if(!result) printf("*********FAIL*********\n");
      printf("\n");
      fflush(NULL);
    }

    {
      printf("[[[ Keyset 'Combination Hi-Lo' Tests ]]]\n\n");

      bool result = true;
      bool drawDiagram = false;

      uint32_t blocks[] =
      {
        0x00000000,

        0x00000001, 0x00000002, 0x00000003, 0x00000004, 0x00000005, 0x00000006, 0x00000007,

        0x80000000, 0x40000000, 0xC0000000, 0x20000000, 0xA0000000, 0x60000000, 0xE0000000
      };

      result &= CombinationKeyTest<hashtype>(hash,6,blocks,sizeof(blocks) / sizeof(uint32_t),true,true,drawDiagram);

      if(!result) printf("*********FAIL*********\n");
      printf("\n");
      fflush(NULL);
    }
  }

  //-----------------------------------------------------------------------------
  // Keyset 'Window'

  // Skip distribution test for these - they're too easy to distribute well,
  // and it generates a _lot_ of testing

  if(g_testWindow || g_testAll)
  {
    printf("[[[ Keyset 'Window' Tests ]]]\n\n");

    bool result = true;
    bool testCollision = true;
    bool testDistribution = false;
    bool drawDiagram = false;

    result &= WindowedKeyTest< Blob<hashbits*2>, hashtype > ( hash, 20, testCollision, testDistribution, drawDiagram );

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
    bool drawDiagram = false;

    const char * alnum = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789";

    result &= TextKeyTest( hash, "Foo",    alnum,4, "Bar",    drawDiagram );
    result &= TextKeyTest( hash, "FooBar", alnum,4, "",       drawDiagram );
    result &= TextKeyTest( hash, "",       alnum,4, "FooBar", drawDiagram );

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
    bool drawDiagram = false;

    result &= ZeroKeyTest<hashtype>( hash, drawDiagram );

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
    bool drawDiagram = false;

    result &= SeedTest<hashtype>( hash, 1000000, drawDiagram );

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
  g_inputVCode = MurmurOAAT(key,len,g_inputVCode);
  g_inputVCode = MurmurOAAT(&seed,sizeof(uint32_t),g_inputVCode);

  g_hashUnderTest->hash(key,len,seed,out);

  g_outputVCode = MurmurOAAT(out,g_hashUnderTest->hashbits/8,g_outputVCode);
}

//-----------------------------------------------------------------------------

void testHash ( const char * name )
{
  HashInfo * pInfo = findHash(name);

  if(pInfo == NULL)
  {
    printf("Invalid hash '%s' specified\n",name);
    return;
  }
  else
  {
    g_hashUnderTest = pInfo;

    if(pInfo->hashbits == 32)
    {
      test<uint32_t>( VerifyHash, pInfo );
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
      printf("Invalid hash bit width %d for hash '%s'",pInfo->hashbits,pInfo->name);
    }
  }
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

int main ( int argc, char ** argv )
{
#if (defined(__x86_64__) && __SSE4_2__) || defined(_M_X64) || defined(_X86_64_)
  const char * defaulthash = "t1ha2_atonce"; /* "murmur3a"; */
#else
  const char * defaulthash = "t1ha0_32le";
#endif
  const char * hashToTest = defaulthash;

  if(argc < 2) {
    printf("No test hash given on command line, testing %s.\n", hashToTest);
    printf("Usage: SMHasher --list or --test=Speed,... hash\n");
  }
  else {
    hashToTest = argv[1];

    if (strncmp(hashToTest,"--", 2) == 0) {
      if (strcmp(hashToTest,"--list") == 0) {
        for(size_t i = 0; i < sizeof(g_hashes) / sizeof(HashInfo); i++) {
          printf("%-16s\t(%s)\n", g_hashes[i].name, g_hashes[i].desc);
        }
        exit(0);
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
      if (argc > 2)
        hashToTest = argv[2];
      else
        hashToTest = defaulthash;
    }
  }

  // Code runs on the 3rd CPU by default
  //SetAffinity((1 << 2));
  SelfTest();

  int timeBegin = clock();

  testHash(hashToTest);

  int timeEnd = clock();

  printf("\n");
  fflush(NULL);
  printf("Input vcode 0x%08x, Output vcode 0x%08x, Result vcode 0x%08x\n",g_inputVCode,g_outputVCode,g_resultVCode);
  printf("Verification value is 0x%08x - Testing took %f seconds\n",g_verify,double(timeEnd-timeBegin)/double(CLOCKS_PER_SEC));
  printf("-------------------------------------------------------------------------------\n");
  fflush(NULL);
  return 0;
}
