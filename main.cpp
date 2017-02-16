#include "Platform.h"
#include "Hashes.h"
#include "KeysetTest.h"
#include "SpeedTest.h"
#include "AvalancheTest.h"
#include "DifferentialTest.h"
#include "PMurHash.h"
#include "beagle_hash.h"
#include "Marvin32.h"

#include <stdio.h>
#include <time.h>

//-----------------------------------------------------------------------------
// Configuration. TODO - move these to command-line flags

bool g_testAll = true;
bool g_testReallyAll = false;

bool g_testSanity      = false;
bool g_testSpeed       = false;
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
  { DoNothingHash, NULL, NULL, NULL, 32,  32,  32, 0x00000000, "donothing32",
      "Do-Nothing function (only valid for measuring call overhead)", NULL },
  { DoNothingHash, NULL, NULL, NULL, 32,  32,  64, 0x00000000, "donothing64",
      "Do-Nothing function (only valid for measuring call overhead)", NULL },
  { DoNothingHash, NULL, NULL, NULL, 32,  32, 128, 0x00000000, "donothing128",
      "Do-Nothing function (only valid for measuring call overhead)", NULL },
  { NoopOAATReadHash, NULL, NULL, NULL, 32, 32, 64, 0x00000000, "NOP_OAAT_read64",
      "Noop function (only valid for measuring call + OAAT reading overhead)", NULL },
  // -- Crap hashes
  { BadHash, NULL, NULL, NULL, 32, 32, 32, 0xAB432E23, "BadHash",
      "very simple XOR shift", NULL },
  { sumhash, NULL, NULL, NULL, 32, 32, 32, 0x0000A9AC, "sumhash",
      "sum all bytes", NULL },
  { sumhash32, NULL, NULL, NULL, 32, 32, 32, 0xF5562C80, "sumhash32",
      "sum all 32bit words", NULL },
  { crc32, NULL, NULL, NULL, 32,  32,  32, 0x3719DB20, "crc32",
      "CRC-32", NULL },
#if defined(__SSE4_2__) && defined(__x86_64__)
  /* Even 32 uses crc32q, quad only */
  { crc32c_hw_test, NULL, NULL, NULL, 32, 32, 32, 0x0C7346F0, "crc32_hw",
      "SSE4.2 crc32 in HW", NULL },
  { crc32c_hw1_test, NULL, NULL, NULL, 32, 32, 32, 0x0C7346F0, "crc32_hw1",
      "Faster Adler SSE4.2 crc32 in HW", NULL },
  { crc64c_hw_test, NULL, NULL, NULL, 32, 32, 64, 0xE7C3FD0E, "crc64_hw",
      "SSE4.2 crc64 in HW", NULL },
#endif

  // -- Message Digests
  { md5_32, NULL, NULL, NULL, 32,  32,  32, 0x0, "md5_32a",
      "MD5, first 32 bits", NULL },
  { sha1_32a, NULL, NULL, NULL, 32,  32,  32, 0x0, "sha1_32a",
      "SHA1, first 32 bits", NULL },
#if 0
  { sha1_64a, NULL, NULL, NULL, 32, 32, 32, 0x00000000, "sha1_64a",
      "SHA1 64-bit, first 64 bits of result", NULL },
  { sha2_32a, NULL, NULL, NULL, 32, 32, 32, 0x00000000, "sha2_32a",
      "SHA2, first 32 bits of result", NULL },
  { sha2_64a, NULL, NULL, NULL, 32, 32, 64, 0x00000000, "sha2_64a",
      "SHA2, first 64 bits of result", NULL },
  { sha3_32a, NULL, NULL, NULL, 32, 32, 32, 0x00000000, "sha3_32a",
      "SHA3, first 32 bits of result", NULL },
  { sha3_64a, NULL, NULL, NULL, 32, 32, 64, 0x00000000, "sha3_64a",
      "SHA3, first 64 bits of result", NULL },
  { BLAKE2_32a, NULL, NULL, NULL, 32, 32, 32, 0x00000000, "blake2_32a",
      "BLAKE2, first 32 bits of result", NULL },
  { BLAKE2_64a, NULL, NULL, NULL, 32, 32, 64, 0x00000000, "blake2_64a",
      "BLAKE2, first 64 bits of result", NULL },
  { bcrypt_64a, NULL, NULL, NULL, 32, 32, 64, 0x00000000, "bcrypt_64a",
      "bcrypt, first 64 bits of result", NULL },
  { scrypt_64a, NULL, NULL, NULL, 32, 32, 64, 0x00000000, "scrypt_64a",
      "scrypt, first 64 bits of result", NULL },
#endif

  // -- Others
#if 0 && defined(__x86_64__) && (defined(__linux__) || defined(__APPLE__))
  // elf64 or macho64 only
  { fhtw_test, NULL, NULL, NULL, 32, 32, 64, 0x0,        "fhtw",
      "fhtw asm", NULL },
#endif
  { FNV32a, NULL, NULL, NULL, 32, 32, 32, 0xE3CBBE91, "FNV1a",
      "Fowler-Noll-Vo hash, 32-bit", NULL },
  { FNV32a_YoshimitsuTRIAD, NULL, NULL, NULL, 32, 32, 32,0xD8AFFD71, "FNV1a_YT",
      "FNV1a-YoshimitsuTRIAD 32-bit sanmayce", NULL },
  { FNV64a, NULL, NULL, NULL, 32, 32, 64, 0x103455FC, "FNV64",
      "Fowler-Noll-Vo hash, 64-bit", NULL },
  { Bernstein, NULL, NULL, NULL, 32, 32, 32, 0xBDB4B640, "bernstein",
      "Bernstein, 32-bit", NULL },
  { lookup3_test, NULL, NULL, NULL, 32, 32, 32, 0x3D83917A, "lookup3",
      "Bob Jenkins' lookup3", NULL },
  { SuperFastHash, NULL, NULL, NULL, 32, 32, 32, 0x980ACD1D, "superfast",
      "Paul Hsieh's SuperFastHash", NULL },
  { MurmurOAAT_test, NULL, NULL, NULL, 32, 32, 32, 0x5363BD98, "MurmurOAAT",
      "Murmur one-at-a-time", NULL },
  { Crap8_test, NULL, NULL, NULL, 32, 32, 32, 0x743E97A1, "Crap8",
      "Crap8", NULL },
  // CityHash
  { CityHash64_test, NULL, NULL, NULL, 32, 32, 64, 0x25A20825, "City64",
      "Google CityHash64WithSeed", NULL },
  { CityHash128_test, NULL, NULL, NULL, 32, 32,128, 0x6531F54E, "City128",
      "Google CityHash128WithSeed", NULL },

  // SpoookyHash
  { SpookyHash32_test, NULL, NULL, NULL, 32, 32,   32, 0x3F798BBB, "Spooky32",
      "Bob Jenkins' SpookyHash, 32-bit result", NULL },
  { SpookyHash64_test, NULL, NULL, NULL, 32, 32,   64, 0xA7F955F1, "Spooky64",
      "Bob Jenkins' SpookyHash, 64-bit result", NULL },
  { SpookyHash128_test, NULL, NULL, NULL, 32, 32, 128, 0x8D263080, "Spooky128",
      "Bob Jenkins' SpookyHash, 128-bit result", NULL },

  // MurmurHash2
  { MurmurHash2_test, NULL, NULL, NULL, 32, 32, 32, 0x27864C1E, "Murmur2",
    "MurmurHash2 for x86, 32-bit", NULL },
  { MurmurHash2A_test, NULL, NULL, NULL, 32, 32, 32, 0x7FBD4396, "Murmur2A",
      "MurmurHash2A for x86, 32-bit", NULL },
  { MurmurHash64A_test, NULL, NULL, NULL, 32, 32, 64, 0x1F0D3804, "Murmur2B",
      "MurmurHash2 for x64, 64-bit", NULL },
  { MurmurHash64B_test, NULL, NULL, NULL, 32, 32, 64, 0xDD537C05, "Murmur2C",
      "MurmurHash2 for x86, 64-bit", NULL },

  // MurmurHash3
  { MurmurHash3_x86_32, NULL, NULL, NULL, 32, 32,  32, 0xB0F57EE3, "Murmur3A",
      "MurmurHash3 for x86, 32-bit", NULL },
  { MurmurHash3_x86_128, NULL, NULL, NULL, 32, 32, 128, 0xB3ECE62A, "Murmur3C",
      "MurmurHash3 for x86, 128-bit", NULL },
#if defined(__x86_64__)
  { MurmurHash3_x64_128, NULL, NULL, NULL, 32, 32, 128, 0x6384BA69, "Murmur3F",
      "MurmurHash3 for x64, 128-bit", NULL },
#endif

  { PMurHash32_test, NULL, NULL, NULL, 32, 32,  32, 0xB0F57EE3, "PMurHash32",
      "Shane Day's portable-ized MurmurHash3 for x86, 32-bit.", NULL },

  // BeagleHash_32_xx
  { beagle_hash_32_32_a_smhasher_test, NULL, NULL, NULL, 32, 32,  32, 0x47CECE27, "BeagleHash_32_32",
      "Yves Orton's hash for 64-bit reduced to 32 (32 bit seed).", NULL },
  { beagle_hash_32_64_a_smhasher_test, NULL, NULL, NULL, 32, 32,  32, 0xC7CD22FA, "BeagleHash_32_64",
      "Yves Orton's hash for 64-bit reduced to 32 (64 bit seed).", NULL },
  { beagle_hash_32_96_a_smhasher_test, NULL, NULL, NULL, 32, 32,  32, 0xCE38DE69, "BeagleHash_32_96",
      "Yves Orton's hash for 64-bit reduced to 32 (96 bit seed).", NULL },
  { beagle_hash_32_112_a_smhasher_test, NULL, NULL, NULL, 32, 32,  32, 0x5CCE6AC4, "BeagleHash_32_112",
      "Yves Orton's hash for 64-bit reduced to 32 (112 bit seed).", NULL },
  { beagle_hash_32_127_a_smhasher_test, NULL, NULL, NULL, 32, 32,  32, 0xC9134969, "BeagleHash_32_127",
      "Yves Orton's hash for 64-bit reduced to 32 (127 bit seed).", NULL },

  // BeagleHash_64_xx
  { beagle_hash_64_32_a_smhasher_test, NULL, NULL, NULL, 32, 32,   64, 0xDE789E78, "BeagleHash_64_32",
      "Yves Orton's hash for 64-bit. (32 bit seed)", NULL },
  { beagle_hash_64_64_a_smhasher_test, NULL, NULL, NULL, 32, 32,   64, 0x3CDD6E7C, "BeagleHash_64_64",
      "Yves Orton's hash for 64-bit. (64 bit seed)", NULL },
  { beagle_hash_64_96_a_smhasher_test, NULL, NULL, NULL, 32, 32,   64, 0x40EBE522, "BeagleHash_64_96",
      "Yves Orton's hash for 64-bit (96 bit seed).", NULL },
  { beagle_hash_64_112_a_smhasher_test, NULL, NULL, NULL, 32, 32,  64, 0xF91596C5, "BeagleHash_64_112",
      "Yves Orton's hash for 64-bit (112 bit seed).", NULL },
  { beagle_hash_64_127_a_smhasher_test, NULL, NULL, NULL, 32, 32,  64, 0x575C6DA6, "BeagleHash_64_127",
      "Yves Orton's hash for 64-bit (127 bit seed).", NULL },

  // ZaphodHash, Marvin32, Phat, Phat4
  { zaphod_hash_smhasher_test, NULL, NULL, NULL, 32, 32, 32, 0x0549DFCF, "Zaphod32",
      "Marvin32 like hash", NULL },
  { phat_hash_smhasher_test, NULL, NULL, NULL, 32, 32, 32, 0xF7C8FFE3, "Phat",
      "Phat Hash", NULL },
  { phat4_hash_smhasher_test, NULL, NULL, NULL, 32, 32, 32, 1 ? 0x0 : 0xB3A5E4A4, "Phat4",
      "Phat4 Hash", NULL },
  { marvin_32_smhasher_test, NULL, NULL, NULL, 32, 32, 32, 0xE6711235, "Marvin32",
      "Marvin32 from MicroSoft", NULL },
#ifdef __SSE2__
  { hasshe2_test, NULL, NULL, NULL, 32, 32, 256, 0xF5D39DFE, "hasshe2",     "SSE2 hasshe2, 256-bit", NULL },
#endif


#if 0
  { fletcher2, NULL, NULL, NULL, 32, 32, 64, 0x0, "fletcher2",
      "fletcher2 ZFS"} //TODO
  { fletcher4, NULL, NULL, NULL, 32, 32, 64, 0x0, "fletcher4",
      "fletcher4 ZFS"} //TODO
  { Jesteress, NULL, NULL, NULL, 32, 32, 32, 0x0, "Jesteress",
      "FNV1a-Jesteress 32-bit sanmayce", NULL },
  { Meiyan, NULL, NULL, NULL, 32, 32, 32, 0x0, "Meiyan",
      "FNV1a-Meiyan 32-bit sanmayce", NULL },
#endif
  { sdbm, NULL, NULL, NULL, 32, 32, 32, 0x582AF769, "sdbm",
      "sdbm as in perl5", NULL },
  { x17_test, NULL, NULL, NULL, 32, 32, 32, 0x8128E14C, "x17",
      "x17", NULL },
  // also called jhash:
  { JenkinsOOAT, NULL, NULL, NULL, 32, 32, 32, 0x83E133DA, "JenkinsOOAT",
      "Bob Jenkins' OOAT as in perl 5.18", NULL },
  { JenkinsOOAT_perl, NULL, NULL, NULL, 32, 32, 32, 0xEE05869B, "JenkinsOOAT_perl",
      "Bob Jenkins' OOAT as in old perl5", NULL },
  { MicroOAAT, NULL, NULL, NULL, 32, 32, 32, 0x16F1BA97, "MicroOAAT",
      "Small non-multiplicative OAAT that passes all collision checks (by funny-falcon)", NULL },
  { halfsiphash_test, NULL, NULL, NULL, 32, 32, 32, 0xA7A05F72, "HalfSipHash",
      "HalfSipHash 2-4, 32bit", NULL },

  // and now the quality hash funcs, which mostly work
  // GoodOOAT passes whole SMHasher (by funny-falcon)
  { GoodOAAT, NULL, NULL, NULL, 32, 32, 32, 0x7B14EEE5, "GoodOAAT",
      "Small non-multiplicative OAAT", NULL },
  { siphash_test, NULL, NULL, NULL, 32, 32, 64, 0xC58D7F9C, "SipHash",
      "SipHash 2-4 - SSSE3 optimized", NULL },
  // as in rust and swift
  { siphash13_test, NULL, NULL, NULL, 32, 32, 64, 0x29C010BF, "SipHash13",
      "SipHash 1-3 - SSSE3 optimized", NULL },
#if defined(__x86_64__)
  { fasthash32_test, NULL, NULL, NULL, 32, 32, 32, 0xE9481AFC, "fasthash32",
      "fast-hash 32bit", NULL },
  { fasthash64_test, NULL, NULL, NULL, 32, 32, 64, 0xA16231A7, "fasthash64",
      "fast-hash 64bit", NULL },
#endif
  { CityHash32_test, NULL, NULL, NULL, 32, 32, 32, 0x5C28AD62, "City32",
      "Google CityHash32WithSeed (old)", NULL },
  { CityHash64_test, NULL, NULL, NULL, 32, 32, 64, 0x25A20825, "City64",
      "Google CityHash64WithSeed (old)", NULL },
#if defined(__SSE4_2__) && defined(__x86_64__)
  { CityHash128_test, NULL, NULL, NULL, 32, 32, 128, 0x6531F54E, "City128",
      "Google CityHash128WithSeed (old)", NULL },
  { CityHashCrc128_test, NULL, NULL, NULL, 32, 32, 128, 0xD4389C97, "CityCrc128",
      "Google CityHashCrc128WithSeed SSE4.2 (old)", NULL },
#endif
#if defined(__x86_64__)
  { FarmHash64_test, NULL, NULL, NULL, 32, 32, 64, 0x35F84A93, "FarmHash64",
      "Google FarmHash64WithSeed", NULL },
  { FarmHash128_test, NULL, NULL, NULL, 32, 32, 128, 0x9E636AAE, "FarmHash128",
      "Google FarmHash128WithSeed", NULL },
  { farmhash64_c_test, NULL, NULL, NULL, 32, 32, 64, 0x35F84A93, "farmhash64_c",
      "farmhash64_with_seed (C99)", NULL },
  { farmhash128_c_test, NULL, NULL, NULL, 32, 32, 128, 0x9E636AAE, "farmhash128_c",
      "farmhash128_with_seed (C99)", NULL },
#endif
#if defined(__x86_64__)
  { xxHash32_test, NULL, NULL, NULL, 32, 32, 32, 0xBA88B743, "xxHash32",
      "xxHash, 32-bit for x64", NULL },
  { xxHash64_test, NULL, NULL, NULL, 32, 32, 64, 0x024B7CF4, "xxHash64",
      "xxHash, 64-bit", NULL },
#if 0
  { xxhash256_test, NULL, NULL, NULL, 32, 32, 64, 0x024B7CF4, "xxhash256",
      "xxhash256, 64-bit unportable", NULL },
#endif
#endif
#if defined(__x86_64__)
  { metrohash64_1_test, NULL, NULL, NULL, 32, 32, 64, 0xEE88F7D2, "metrohash64_1",
      "MetroHash64_1 for 64-bit", NULL },
  { metrohash64_2_test, NULL, NULL, NULL, 32, 32, 64, 0xE1FC7C6E, "metrohash64_2",
      "MetroHash64_2 for 64-bit", NULL },
  { metrohash128_1_test, NULL, NULL, NULL, 32, 32, 128, 0x20E8A1D7, "metrohash128_1",
      "MetroHash128_1 for 64-bit", NULL },
  { metrohash128_2_test, NULL, NULL, NULL, 32, 32, 128, 0x5437C684, "metrohash128_2",
      "MetroHash128_2 for 64-bit", NULL },
#if defined(__SSE4_2__) && defined(__x86_64__)
  { metrohash64crc_1_test, NULL, NULL, NULL, 32, 32, 64, 0x29C68A50, "metrohash64crc_1",
      "MetroHash64crc_1 for x64", NULL },
  { metrohash64crc_2_test, NULL, NULL, NULL, 32, 32, 64, 0x2C00BD9F, "metrohash64crc_2",
      "MetroHash64crc_2 for x64", NULL },
  { metrohash128crc_1_test, NULL, NULL, NULL, 32, 32, 128, 0x5E75144E, "metrohash128crc_1",
      "MetroHash128crc_1 for x64", NULL },
  { metrohash128crc_2_test, NULL, NULL, NULL, 32, 32, 128, 0x1ACF3E77, "metrohash128crc_2",
      "MetroHash128crc_2 for x64", NULL },
#endif
#endif
#if defined(__x86_64__)
  { cmetrohash64_1_optshort_test, NULL, NULL, NULL, 32, 32, 64, 0xEE88F7D2, "cmetrohash64_1o",
      "cmetrohash64_1 (shorter key optimized) , 64-bit for x64", NULL },
  { cmetrohash64_1_test, NULL, NULL, NULL, 32, 32, 64, 0xEE88F7D2, "cmetrohash64_1",
      "cmetrohash64_1, 64-bit for x64", NULL },
  { cmetrohash64_2_test, NULL, NULL, NULL, 32, 32, 64, 0xE1FC7C6E, "cmetrohash64_2",
      "cmetrohash64_2, 64-bit for x64", NULL },
#endif
#if defined(__SSE4_2__) && defined(__x86_64__)
  { falkhash_test_cxx, NULL, NULL, NULL, 32, 32, 64, 0x2F99B071, "falkhash",
      "falkhash.asm with aesenc, 64-bit for x64", NULL },
#endif
  { t1ha_test, NULL, NULL, NULL, 32, 32, 64, 0xD6836381, "t1ha",
      "Fast Positive Hash (portable, best for: 64-bit, little-endian)", NULL },
  { t1ha_64be_test, NULL, NULL, NULL, 32, 32, 64, 0x93F864DE, "t1ha_64be",
      "Fast Positive Hash (portable, best for: 64-bit, big-endian)", NULL },
  { t1ha_32le_test, NULL, NULL, NULL, 32, 32, 64, 0xE489F366, "t1ha_32le",
      "Fast Positive Hash (portable, best for: 32-bit, little-endian)", NULL },
  { t1ha_32be_test, NULL, NULL, NULL, 32, 32, 64, 0x71F649A9, "t1ha_32be",
      "Fast Positive Hash (portable, best for: 32-bit, big-endian)", NULL },
#if (defined(__SSE4_2__) && defined(__x86_64__)) || defined(_M_X64)
  { t1ha_crc_test, NULL, NULL, NULL, 32, 32, 64, 0xA57ACE7D, "t1ha_crc",
      "Fast Positive Hash (machine-specific, requires: SSE4.2 CRC32C)", NULL },
#endif
#if defined(__AES__) || defined(_M_X64) || defined(_M_IX86)
  { t1ha_aes_test, NULL, NULL, NULL, 32, 32, 64, 0x54BBFF21, "t1ha_aes",
      "Fast Positive Hash (machine-specific, requires: AES-NI)", NULL },
#endif
#if defined(__GNUC__) && UINT_MAX != ULONG_MAX
#define MUM_VERIFY 0x3EEAE2D4
#else
#define MUM_VERIFY 0xA973C6C0
#endif
  { mum_hash_test, NULL, NULL, NULL, 32, 32, 64, MUM_VERIFY, "MUM",
    "github.com/vnmakarov/mum-hash", NULL },
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

    pass &= VerificationTest(info->hash,info->hashbits,info->verification,false,info->name);
  }

  if(!pass)
  {
    printf("Self-test FAILED!\n");

    for(size_t i = 0; i < sizeof(g_hashes) / sizeof(HashInfo); i++)
    {
      HashInfo * info = & g_hashes[i];

      printf("%24s - ",info->name);
      pass &= VerificationTest(info->hash,info->hashbits,info->verification,true,info->name);
    }

    exit(1);
  }
}

//----------------------------------------------------------------------------

template < typename hashtype >
void test ( hashfunc<hashtype> hash, HashInfo * info )
{
  const int hashbits = sizeof(hashtype) * 8;
  bool pass= true;

  printf("-------------------------------------------------------------------------------\n");
  printf("--- Testing %s (%s)\n\n",info->name,info->desc);

  //-----------------------------------------------------------------------------
  // Sanity tests

  if(g_testSanity || g_testAll)
  {
    printf("[[[ Sanity Tests ]]] - %s\n\n",info->name);

    pass &= VerificationTest(hash,hashbits,info->verification,true,info->name);
    pass &= SanityTest(hash,hashbits);
    pass &= AppendedZeroesTest(hash,hashbits);

    if(!pass) printf("********* %s - FAIL *********\n",info->name);
    printf("\n");
  }

  //-----------------------------------------------------------------------------
  // Speed tests

  if(g_testSpeed || g_testAll)
  {
    double sum = 0.0;
    double t0  = 0.0;
    printf("[[[ Speed Tests ]]] - %s\n\n",info->name);

    BulkSpeedTest(info->hash,info->verification);
    printf("\n");

    for(int i = 0; i <= 31; i++)
    {
      sum += TinySpeedTest(hashfunc<hashtype>(info->hash),
              sizeof(hashtype), i, info->verification, true, t0);
      if(!i) t0 = sum;
    }
    sum = sum / 32.0; /* includes the empty string! */

    printf("                            Average - %6.3f cycles/hash\n",sum);
    printf("\n");
  }

  //-----------------------------------------------------------------------------
  // Differential tests

  if(g_testDiff || g_testAll)
  {
    printf("[[[ Differential Tests ]]] - %s\n\n",info->name);

    bool result = true;
    bool dumpCollisions = false;

    result &= DiffTest< Blob<64>,  hashtype >(hash,5,1000,dumpCollisions);
    result &= DiffTest< Blob<128>, hashtype >(hash,4,1000,dumpCollisions);
    result &= DiffTest< Blob<256>, hashtype >(hash,3,1000,dumpCollisions);

    if(!result) printf("********* %s - FAIL *********\n",info->name);
    printf("\n");
    pass &= result;
  }

  //-----------------------------------------------------------------------------
  // Differential-distribution tests

  if(g_testDiffDist || g_testReallyAll)
  {
    printf("[[[ Differential Distribution Tests ]]] - %s\n\n",info->name);

    bool result = true;

    result &= DiffDistTest2<uint64_t,hashtype>(hash);

    if(!result) printf("********* %s - FAIL *********\n",info->name);
    printf("\n");
    pass &= result;
  }

  //-----------------------------------------------------------------------------
  // Avalanche tests

  if(g_testAvalanche || g_testAll)
  {
    printf("[[[ Avalanche Tests ]]] - %s\n\n",info->name);

    bool result = true;
    const int reps = 1000000;
    printf("Expected error: %.5e\n\n", 0.00256 / ( (double)reps / 100000.0 ));

    result &= AvalancheTest< Blob< 32>, hashtype > (hash,reps);
    result &= AvalancheTest< Blob< 40>, hashtype > (hash,reps);
    result &= AvalancheTest< Blob< 48>, hashtype > (hash,reps);
    result &= AvalancheTest< Blob< 56>, hashtype > (hash,reps);

    result &= AvalancheTest< Blob< 64>, hashtype > (hash,reps);
    result &= AvalancheTest< Blob< 72>, hashtype > (hash,reps);
    result &= AvalancheTest< Blob< 80>, hashtype > (hash,reps);
    result &= AvalancheTest< Blob< 88>, hashtype > (hash,reps);

    result &= AvalancheTest< Blob< 96>, hashtype > (hash,reps);
    result &= AvalancheTest< Blob<104>, hashtype > (hash,reps);
    result &= AvalancheTest< Blob<112>, hashtype > (hash,reps);
    result &= AvalancheTest< Blob<120>, hashtype > (hash,reps);

    result &= AvalancheTest< Blob<128>, hashtype > (hash,reps);
    result &= AvalancheTest< Blob<136>, hashtype > (hash,reps);
    result &= AvalancheTest< Blob<144>, hashtype > (hash,reps);
    result &= AvalancheTest< Blob<152>, hashtype > (hash,reps);

    if(!result) printf("********* %s - FAIL *********\n",info->name);
    printf("\n");
    pass &= result;
  }

  //-----------------------------------------------------------------------------
  // Bit Independence Criteria. Interesting, but doesn't tell us much about
  // collision or distribution.

  if(g_testBIC || g_testReallyAll)
  {
    printf("[[[ Bit Independence Criteria ]]] - %s\n\n",info->name);

    bool result = true;

    //result &= BicTest<uint64_t,hashtype>(hash,2000000);
    result &= BicTest3<Blob<88>,hashtype>(hash,2000000);

    if(!result) printf("********* %s - FAIL *********\n",info->name);
    printf("\n");
    pass &= result;
  }

  //-----------------------------------------------------------------------------
  // Keyset 'Cyclic' - keys of the form "abcdabcdabcd..."

  if(g_testCyclic || g_testAll)
  {
    printf("[[[ Keyset 'Cyclic' Tests ]]] - %s\n\n",info->name);

    bool result = true;
    bool drawDiagram = false;

    result &= CyclicKeyTest<hashtype>(hash,sizeof(hashtype)+0,8,10000000,drawDiagram);
    result &= CyclicKeyTest<hashtype>(hash,sizeof(hashtype)+1,8,10000000,drawDiagram);
    result &= CyclicKeyTest<hashtype>(hash,sizeof(hashtype)+2,8,10000000,drawDiagram);
    result &= CyclicKeyTest<hashtype>(hash,sizeof(hashtype)+3,8,10000000,drawDiagram);
    result &= CyclicKeyTest<hashtype>(hash,sizeof(hashtype)+4,8,10000000,drawDiagram);

    if(!result) printf("********* %s - FAIL *********\n",info->name);
    printf("\n");
    pass &= result;
  }

  //-----------------------------------------------------------------------------
  // Keyset 'TwoBytes' - all keys up to N bytes containing two non-zero bytes

  // This generates some huge keysets, 128-bit tests will take ~1.3 gigs of RAM.

  if(g_testTwoBytes || g_testAll)
  {
    printf("[[[ Keyset 'TwoBytes' Tests ]]] - %s\n\n",info->name);

    bool result = true;
    bool drawDiagram = false;

    for(int i = 4; i <= 20; i += 4)
    {
      result &= TwoBytesTest2<hashtype>(hash,i,drawDiagram);
    }

    if(!result) printf("********* %s - FAIL *********\n",info->name);
    printf("\n");
    pass &= result;
  }

  //-----------------------------------------------------------------------------
  // Keyset 'Sparse' - keys with all bits 0 except a few

  if(g_testSparse || g_testAll)
  {
    printf("[[[ Keyset 'Sparse' Tests ]]] - %s\n\n",info->name);

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

    if(!result) printf("********* %s - FAIL *********\n",info->name);
    printf("\n");
    pass &= result;
  }

  //-----------------------------------------------------------------------------
  // Keyset 'Permutation' - all possible combinations of a set of blocks

  if(g_testPermutation || g_testAll)
  {
    {
      // This one breaks lookup3, surprisingly

      printf("[[[ Keyset 'Combination Lowbits' Tests ]]] - %s\n\n",info->name);

      bool result = true;
      bool drawDiagram = false;

      uint32_t blocks[] =
      {
        0x00000000,

        0x00000001, 0x00000002, 0x00000003, 0x00000004, 0x00000005, 0x00000006, 0x00000007,
      };

      result &= CombinationKeyTest<hashtype>(hash,8,blocks,sizeof(blocks) / sizeof(uint32_t),true,true,drawDiagram);

    if(!result) printf("********* %s - FAIL *********\n",info->name);
      printf("\n");
      pass &= result;
    }

    {
      printf("[[[ Keyset 'Combination Highbits' Tests ]]] - %s\n\n",info->name);

      bool result = true;
      bool drawDiagram = false;

      uint32_t blocks[] =
      {
        0x00000000,

        0x20000000, 0x40000000, 0x60000000, 0x80000000, 0xA0000000, 0xC0000000, 0xE0000000
      };

      result &= CombinationKeyTest<hashtype>(hash,8,blocks,sizeof(blocks) / sizeof(uint32_t),true,true,drawDiagram);

      if(!result) printf("********* %s - FAIL *********\n",info->name);
      printf("\n");
      pass &= result;
    }

    {
      printf("[[[ Keyset 'Combination 0x8000000' Tests ]]] - %s\n\n",info->name);

      bool result = true;
      bool drawDiagram = false;

      uint32_t blocks[] =
      {
        0x00000000,

        0x80000000,
      };

      result &= CombinationKeyTest<hashtype>(hash,20,blocks,sizeof(blocks) / sizeof(uint32_t),true,true,drawDiagram);

      if(!result) printf("********* %s - FAIL *********\n",info->name);
      printf("\n");
      pass &= result;
    }

    {
      printf("[[[ Keyset 'Combination 0x0000001' Tests ]]] - %s\n\n",info->name);

      bool result = true;
      bool drawDiagram = false;

      uint32_t blocks[] =
      {
        0x00000000,

        0x00000001,
      };

      result &= CombinationKeyTest<hashtype>(hash,20,blocks,sizeof(blocks) / sizeof(uint32_t),true,true,drawDiagram);

      if(!result) printf("********* %s - FAIL *********\n",info->name);
      printf("\n");
      pass &= result;
    }

    {
      printf("[[[ Keyset 'Combination Hi-Lo' Tests ]]] - %s\n\n",info->name);

      bool result = true;
      bool drawDiagram = false;

      uint32_t blocks[] =
      {
        0x00000000,

        0x00000001, 0x00000002, 0x00000003, 0x00000004, 0x00000005, 0x00000006, 0x00000007,

        0x80000000, 0x40000000, 0xC0000000, 0x20000000, 0xA0000000, 0x60000000, 0xE0000000
      };

      result &= CombinationKeyTest<hashtype>(hash,6,blocks,sizeof(blocks) / sizeof(uint32_t),true,true,drawDiagram);

      if(!result) printf("********* %s - FAIL *********\n",info->name);
      printf("\n");
      pass &= result;
    }
  }

  //-----------------------------------------------------------------------------
  // Keyset 'Window'

  // Skip distribution test for these - they're too easy to distribute well,
  // and it generates a _lot_ of testing

  if(g_testWindow || g_testAll)
  {
    printf("[[[ Keyset 'Window' Tests ]]] - %s\n\n",info->name);

    bool result = true;
    bool testCollision = true;
    bool testDistribution = false;
    bool drawDiagram = false;

    result &= WindowedKeyTest< Blob<hashbits*2>, hashtype > ( hash, 20, testCollision, testDistribution, drawDiagram );

    if(!result) printf("********* %s - FAIL *********\n",info->name);
    printf("\n");
    pass &= result;
  }

  //-----------------------------------------------------------------------------
  // Keyset 'Text'

  if(g_testText || g_testAll)
  {
    printf("[[[ Keyset 'Text' Tests ]]] - %s\n\n",info->name);

    bool result = true;
    bool drawDiagram = false;

    const char * alnum = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789";

    result &= TextKeyTest( hash, "Foo",    alnum,4, "Bar",    drawDiagram );
    result &= TextKeyTest( hash, "FooBar", alnum,4, "",       drawDiagram );
    result &= TextKeyTest( hash, "",       alnum,4, "FooBar", drawDiagram );

    if(!result) printf("********* %s - FAIL *********\n",info->name);
    printf("\n");
    pass &= result;
  }

  //-----------------------------------------------------------------------------
  // Keyset 'Zeroes'

  if(g_testZeroes || g_testAll)
  {
    printf("[[[ Keyset 'Zeroes' Tests ]]] - %s\n\n",info->name);

    bool result = true;
    bool drawDiagram = false;

    result &= ZeroKeyTest<hashtype>( hash, drawDiagram );

    if(!result) printf("********* %s - FAIL *********\n",info->name);
    printf("\n");
    pass &= result;
  }

  //-----------------------------------------------------------------------------
  // Keyset 'Seed'

  if(g_testSeed || g_testAll)
  {
    printf("[[[ Keyset 'Seed' Tests ]]] - %s\n\n",info->name);

    bool result = true;
    bool drawDiagram = false;

    result &= SeedTest<hashtype>( hash, 2000000, drawDiagram,
        "The quick brown fox jumps over the lazy dog");
    result &= SeedTest<hashtype>( hash, 2000000, drawDiagram,
        "");
    result &= SeedTest<hashtype>( hash, 2000000, drawDiagram,
        "00101100110101101");
    result &= SeedTest<hashtype>( hash, 2000000, drawDiagram,
        "abcbcddbdebdcaaabaaababaaabacbeedbabseeeeeeeesssssseeeewwwww");

    if(!result) printf("********* %s - FAIL *********\n",info->name);
    printf("\n");
    pass &= result;
  }

  //-----------------------------------------------------------------------------
  // Keyset 'Effs'

  if(g_testEffs || g_testAll)
  {
    printf("[[[ Keyset 'Effs' Tests ]]] - %s\n\n",info->name);
    
    bool result = true;
    bool drawDiagram = false;

    result &= EffsKeyTest<hashtype>( hash, drawDiagram );
    
    if(!result) printf("********* %s - FAIL *********\n",info->name);
    printf("\n");
    pass &= result;
  }

  if (pass) {
      printf("####### %s - ALL TESTS PASSED. #######",info->name);
  } else {
      printf("******* %s - TESTS FAILED *******",info->name);
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
  const char * defaulthash = "metrohash64crc_1"; /* "murmur3a"; */
#else
  const char * defaulthash = "t1ha_32le";
#endif
  const char * hashToTest = defaulthash;
  
  setvbuf(stdout, NULL, _IONBF, 0); /* autoflush stdout */
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

  SetAffinity((1 << 2));

  SelfTest();

  clock_t timeBegin = clock();

  //g_testAll = false;

  //g_testSanity = true;
  //g_testSpeed = true;
  //g_testAvalanche = true;
  //g_testBIC = true;
  //g_testCyclic = true;
  //g_testTwoBytes = true;
  //g_testDiff = true;
  //g_testDiffDist = true;
  //g_testSparse = true;
  //g_testPermutation = true;
  //g_testWindow = true;
  //g_testZeroes = true;
  //g_testEffs = true;
  //g_testSeed = true;

  testHash(hashToTest);

  //----------

  clock_t timeEnd = clock();

  printf("\n");
  printf("Input vcode 0x%08x, Output vcode 0x%08x, Result vcode 0x%08x\n",g_inputVCode,g_outputVCode,g_resultVCode);
  printf( "Verification value is 0x%08x - Testing took %f seconds\n",g_verify,
          double(timeEnd - timeBegin) / double(CLOCKS_PER_SEC) );
  printf("-------------------------------------------------------------------------------\n");
  return 0;
}
