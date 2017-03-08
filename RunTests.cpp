#include <stdio.h>
#include <time.h>
#include "Types.h"
#include "Platform.h"
#include "KeysetTest.h"
#include "SpeedTest.h"
#include "AvalancheTest.h"
#include "DifferentialTest.h"
#include "StreamTest.h"
#include "RunTests.h"

extern HashInfo * g_hashUnderTest;
extern bool g_runCtrStream;
extern bool g_testAll;
extern bool g_testReallyAll;
extern bool g_testSanity;
extern bool g_testSpeed;
extern bool g_testBulkSpeed;
extern bool g_testKeySpeed;
extern bool g_testDiff;
extern bool g_testDiffDist;   /* only ReallyAll */
extern bool g_testAvalanche;
extern bool g_testBIC;        /* only ReallyAll */
extern bool g_testCyclic;
extern bool g_testTwoBytes;
extern bool g_testSparse;
extern bool g_testPermutation;
extern bool g_testWindow;     /* only ReallyAll */
extern bool g_testText;
extern bool g_testZeroes;
extern bool g_testEffs;
extern bool g_testSeed;
extern HashInfo g_hashes[];
extern int g_hashes_sizeof;
extern uint64_t g_rngSeed;
extern uint64_t g_streamKeyLen;

//----------------------------------------------------------------------------
template < typename hashtype, typename seedtype >
bool testHashWithSeed ( HashInfo * info, int self_test, double confidence )
{
  const int hashbits = sizeof(hashtype) * 8;
  bool pass= true;
  // see: https://en.wikipedia.org/wiki/Standard_deviation#Rules_for_normally_distributed_data

  hashfunc<hashtype> hash(
      info->seed_state,
      info->hash_with_state,
      info->seedbits,
      info->statebits,
      info->name
  );
  if (!self_test and g_runCtrStream) {
    CtrStream<hashtype>(hash, g_rngSeed, g_streamKeyLen);
  }
  setvbuf(stdout, NULL, _IONBF, 0); /* autoflush stdout */

  if (!self_test) {
    printf("###################################################################\n");
    printf("### Testing %s (%s)\n",info->name,info->desc);
    printf("###################################################################\n");
  }
  //-----------------------------------------------------------------------------
  // Sanity tests

  if(self_test || g_testSanity || g_testAll)
  {
    if(!self_test)
      printf("### Sanity Tests ###\n");

    bool result = VerificationTest<hashtype>(hash, info->verification,
        !self_test ? 2 : self_test > 1,info->name);
    if (self_test) return result;

    result &= SanityTest<hashtype>(hash);
    result &= AppendedZeroesTest<hashtype>(hash);

    pass &= ok(result, "Sanity Test", info->name);
  }

  //-----------------------------------------------------------------------------
  // Speed tests

  if(g_testSpeed || g_testBulkSpeed || g_testKeySpeed || g_testAll)
  {
    printf("### Speed Tests ###\n");

    bool result = true;
    Rand r(info->verification);

    if (g_testSpeed || g_testBulkSpeed || g_testAll) {
      BulkSpeedTest(hash, r);
    }

    if (g_testSpeed || g_testKeySpeed || g_testAll) {
      result &= RunKeySpeedTests(hash,r);
    }
    pass &= ok(result, "Speed (always passes)", info->name);
  }

  //-----------------------------------------------------------------------------
  // Differential tests

  if(g_testDiff || g_testAll)
  {
    printf("### Differential Tests ###\n");

    bool result = true;
    bool dumpCollisions = false;
    Rand r(100);

    result &= DiffTest< Blob<64>,  hashtype >(hash,5,1000,dumpCollisions, r);
    result &= DiffTest< Blob<128>, hashtype >(hash,4,1000,dumpCollisions, r);
    result &= DiffTest< Blob<256>, hashtype >(hash,3,1000,dumpCollisions, r);

    pass &= ok(result, "Differential", info->name);
  }

  //-----------------------------------------------------------------------------
  // Differential-distribution tests

  if(g_testDiffDist || g_testReallyAll)
  {
    printf("### Differential Distribution Tests ###\n");

    bool result = true;

    result &= DiffDistTest2<uint64_t,hashtype>(hash,confidence);

    pass &= ok(result, "Differential Distribution", info->name);
  }

  //-----------------------------------------------------------------------------
  // Avalanche tests

  if(g_testAvalanche || g_testAll)
  {
    printf("### Avalanche Tests ### - seed-bits: %d hash-bits: %d\n",
        info->seedbits, info->hashbits);

    bool result = true;
    const int reps = 32000000 / info->hashbits;
    double max_pct_error = 1.0 / 100.00;
    double max_error_ratio = 1.5;
    int size = 0;
    Rand r(923145681);
    printf("# Samples %d, expected error %.8f, confidence level %.8f%%\n",
        reps, 0.00256 / ( (double)reps / 100000.0 ), confidence * 100);
    /* this is very ugly - but we cant use a variable for the bob size.
     * I think maybe there are ways to get rid of this. We have type explosion
     * going on here big time. */
    if (!size || size == 0)
    result &= AvalancheTest< seedtype, Blob< 0>, hashtype > (hash, reps, r, confidence, max_pct_error, max_error_ratio);
    if (!size || size == 8)
    result &= AvalancheTest< seedtype, Blob< 8>, hashtype > (hash, reps, r, confidence, max_pct_error, max_error_ratio);
    if (!size || size == 16)
    result &= AvalancheTest< seedtype, Blob< 16>, hashtype > (hash, reps, r, confidence, max_pct_error, max_error_ratio);
    if (!size || size == 24)
    result &= AvalancheTest< seedtype, Blob< 24>, hashtype > (hash, reps, r, confidence, max_pct_error, max_error_ratio);
    if (!size || size == 32)
    result &= AvalancheTest< seedtype, Blob< 32>, hashtype > (hash, reps, r, confidence, max_pct_error, max_error_ratio);
    if (!size || size == 40)
    result &= AvalancheTest< seedtype, Blob< 40>, hashtype > (hash, reps, r, confidence, max_pct_error, max_error_ratio);
    if (!size || size == 48)
    result &= AvalancheTest< seedtype, Blob< 48>, hashtype > (hash, reps, r, confidence, max_pct_error, max_error_ratio);
    if (!size || size == 56)
    result &= AvalancheTest< seedtype, Blob< 56>, hashtype > (hash, reps, r, confidence, max_pct_error, max_error_ratio);
    if (!size || size == 64)
    result &= AvalancheTest< seedtype, Blob< 64>, hashtype > (hash, reps, r, confidence, max_pct_error, max_error_ratio);
    if (!size || size == 72)
    result &= AvalancheTest< seedtype, Blob< 72>, hashtype > (hash, reps, r, confidence, max_pct_error, max_error_ratio);
    if (!size || size == 80)
    result &= AvalancheTest< seedtype, Blob< 80>, hashtype > (hash, reps, r, confidence, max_pct_error, max_error_ratio);
    if (!size || size == 88)
    result &= AvalancheTest< seedtype, Blob< 88>, hashtype > (hash, reps, r, confidence, max_pct_error, max_error_ratio);
    if (!size || size == 96)
    result &= AvalancheTest< seedtype, Blob< 96>, hashtype > (hash, reps, r, confidence, max_pct_error, max_error_ratio);
    if (!size || size == 104)
    result &= AvalancheTest< seedtype, Blob<104>, hashtype > (hash, reps, r, confidence, max_pct_error, max_error_ratio);
    if (!size || size == 112)
    result &= AvalancheTest< seedtype, Blob<112>, hashtype > (hash, reps, r, confidence, max_pct_error, max_error_ratio);
    if (!size || size == 120)
    result &= AvalancheTest< seedtype, Blob<120>, hashtype > (hash, reps, r, confidence, max_pct_error, max_error_ratio);
    if (!size || size == 128)
    result &= AvalancheTest< seedtype, Blob<128>, hashtype > (hash, reps, r, confidence, max_pct_error, max_error_ratio);
    if (!size || size == 136)
    result &= AvalancheTest< seedtype, Blob<136>, hashtype > (hash, reps, r, confidence, max_pct_error, max_error_ratio);
    if (!size || size == 144)
    result &= AvalancheTest< seedtype, Blob<144>, hashtype > (hash, reps, r, confidence, max_pct_error, max_error_ratio);
    if (!size || size == 152)
    result &= AvalancheTest< seedtype, Blob<152>, hashtype > (hash, reps, r, confidence, max_pct_error, max_error_ratio);

    pass &= ok(result, "Strict Avalanche Criteria", info->name);
  }

  //-----------------------------------------------------------------------------
  // Bit Independence Criteria. Interesting, but doesn't tell us much about
  // collision or distribution.

  if(g_testBIC || g_testReallyAll)
  {
    printf("### Bit Independence Criteria ###\n");

    bool result = true;

    //result &= BicTest<uint64_t,hashtype>(hash,2000000);
    result &= BicTest3<Blob<88>,hashtype>(hash,2000000);

    pass &= ok(result, "Bit Independence Criteria (BicTest3)", info->name);
  }

  //-----------------------------------------------------------------------------
  // Keyset 'Cyclic' - keys of the form "abcdabcdabcd..."

  if(g_testCyclic || g_testAll)
  {
    printf("### Keyset 'Cyclic' Tests ###\n");

    bool result = true;
    bool drawDiagram = false;
    Rand r(910203);
    int size = sizeof(hashtype);

    result &= CyclicKeyTest<hashtype>(hash,size+0,8,10000000, confidence, drawDiagram, r);
    result &= CyclicKeyTest<hashtype>(hash,size+1,8,10000000, confidence, drawDiagram, r);
    result &= CyclicKeyTest<hashtype>(hash,size+2,8,10000000, confidence, drawDiagram, r);
    result &= CyclicKeyTest<hashtype>(hash,size+3,8,10000000, confidence, drawDiagram, r);
    result &= CyclicKeyTest<hashtype>(hash,size+4,8,10000000, confidence, drawDiagram, r);

    pass &= ok(result, "Keyset 'Cyclic'", info->name);
  }

  //-----------------------------------------------------------------------------
  // Keyset 'TwoBytes' - all keys up to N bytes containing two non-zero bytes

  // This generates some huge keysets, 128-bit tests will take ~1.3 gigs of RAM.

  if(g_testTwoBytes || g_testAll)
  {
    printf("### Keyset 'TwoBytes' Tests ###\n");

    bool result = true;
    bool drawDiagram = false;

    for(int i = 4; i <= 20; i += 4)
    {
      result &= TwoBytesTest2<hashtype>(hash,i, confidence, drawDiagram);
    }

    pass &= ok(result, "Keyset 'TwoBytes'", info->name);
  }

  //-----------------------------------------------------------------------------
  // Keyset 'Sparse' - keys with all bits 0 except a few

  if(g_testSparse || g_testAll)
  {
    printf("### Keyset 'Sparse' Tests ###\n");

    bool result = true;
    bool drawDiagram = false;
    Rand r(8075093);

    result &= SparseKeyTest<  32,hashtype>(hash,6,true,true,confidence, drawDiagram, r);
    result &= SparseKeyTest<  40,hashtype>(hash,6,true,true,confidence, drawDiagram, r);
    result &= SparseKeyTest<  48,hashtype>(hash,5,true,true,confidence, drawDiagram, r);
    result &= SparseKeyTest<  56,hashtype>(hash,5,true,true,confidence, drawDiagram, r);
    result &= SparseKeyTest<  64,hashtype>(hash,5,true,true,confidence, drawDiagram, r);
    result &= SparseKeyTest<  96,hashtype>(hash,4,true,true,confidence, drawDiagram, r);
    result &= SparseKeyTest< 256,hashtype>(hash,3,true,true,confidence, drawDiagram, r);
    result &= SparseKeyTest<2048,hashtype>(hash,2,true,true,confidence, drawDiagram, r);

    pass &= ok(result, "Keyset 'Sparse'", info->name);
  }

  //-----------------------------------------------------------------------------
  // Keyset 'Permutation' - all possible combinations of a set of blocks

  if(g_testPermutation || g_testAll)
  {
    {
      // This one breaks lookup3, surprisingly

      char name[]= "Keyset 'Combination Lowbits'";
      printf("### %s Tests ###\n", name);

      bool drawDiagram = false;
      Rand r(4810489);

      uint32_t blocks[] =
      {
        0x00000000, 0x00000001, 0x00000002, 0x00000003,
        0x00000004, 0x00000005, 0x00000006, 0x00000007,
      };

      bool result = CombinationKeyTest<hashtype>(
        hash,8,blocks,sizeof(blocks) / sizeof(uint32_t),true,confidence, drawDiagram, r, name);

      pass &= ok(result, name, info->name);
    }

    {
      char name[]= "Keyset 'Combination Highbits'";
      printf("### %s Tests ###\n", name);

      bool result = true;
      bool drawDiagram = false;
      Rand r(9104174);

      uint32_t blocks[] =
      {
        0x00000000, 0x20000000, 0x40000000, 0x60000000,
        0x80000000, 0xA0000000, 0xC0000000, 0xE0000000
      };

      result &= CombinationKeyTest<hashtype>(
          hash,8,blocks,sizeof(blocks) / sizeof(uint32_t),true,confidence, drawDiagram, r, name);

      pass &= ok(result, name, info->name);
    }
    {
      char name[]= "Keyset 'Combination Highbits2'";
      printf("### %s Tests ###\n", name);

      bool result = true;
      bool drawDiagram = false;
      Rand r(9104174);

      uint32_t blocks[] =
      {
        0x80000000, 0x90000000, 0xA0000000, 0xB0000000,
        0xC0000000, 0xD0000000, 0xE0000000, 0xF0000000
      };

      result &= CombinationKeyTest<hashtype>(
          hash,8,blocks,sizeof(blocks) / sizeof(uint32_t),true,confidence, drawDiagram, r, name);

      pass &= ok(result, name, info->name);
    }

    {
      char name[]= "Keyset 'Combination 0x80000000'";
      printf("### %s Tests ###\n", name);

      bool drawDiagram = false;
      Rand r(183235);

      uint32_t blocks[] =
      {
        0x00000000,
        0x80000000,
      };

      bool result = CombinationKeyTest<hashtype>(
        hash,20,blocks,sizeof(blocks) / sizeof(uint32_t), true, confidence, drawDiagram, r,name);

      pass &= ok(result, name, info->name);
    }

    {
      char name[]= "Keyset 'Combination 0x00000001'";
      printf("### %s Tests ###\n", name);

      bool drawDiagram = false;
      Rand r(831951);

      uint32_t blocks[] =
      {
        0x00000000,
        0x00000001,
      };

      bool result = CombinationKeyTest<hashtype>(
        hash,20,blocks,sizeof(blocks) / sizeof(uint32_t),true,confidence, drawDiagram, r,name);

      pass &= ok(result, name, info->name);
    }

    {
      char name[]= "Keyset 'Combination Hi-Lo'";
      printf("### %s Tests ###\n", name);

      bool drawDiagram = false;
      Rand r(47831);

      uint32_t blocks[] =
      {
        0x00000000,
        0x00000001, 0x00000002, 0x00000003, 0x00000004, 0x00000005, 0x00000006, 0x00000007,
        0x80000000, 0x40000000, 0xC0000000, 0x20000000, 0xA0000000, 0x60000000, 0xE0000000
      };

      bool result = CombinationKeyTest<hashtype>(
        hash,6,blocks,sizeof(blocks) / sizeof(uint32_t),true,confidence, drawDiagram, r, name);

      pass &= ok(result, name, info->name);
    }
  }

  //-----------------------------------------------------------------------------
  // Keyset 'Window'

  // Skip distribution test for these - they're too easy to distribute well,
  // and it generates a _lot_ of testing

  if(g_testWindow || g_testReallyAll)
  {
    printf("### Keyset 'Window' Tests ###\n");

    bool testCollision = true;
    bool drawDiagram = false;
    Rand r(77589);

    bool result = WindowedKeyTest< Blob<hashbits*2>, hashtype >(
        hash, 20, testCollision, confidence, drawDiagram, r );

    pass &= ok(result, "Keyset 'Window'", info->name);
  }

  //-----------------------------------------------------------------------------
  // Keyset 'Text'

  if(g_testText || g_testAll)
  {
    printf("### Keyset 'Text' Tests ###\n");

    bool result = true;
    bool drawDiagram = false;
    Rand r(543823);

    const char * alnum = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789";

    result &= TextKeyTest( hash, "Foo",    alnum,4, "Bar",    confidence, drawDiagram, r );
    result &= TextKeyTest( hash, "FooBar", alnum,4, "",       confidence, drawDiagram, r );
    result &= TextKeyTest( hash, "",       alnum,4, "FooBar", confidence, drawDiagram, r );

    pass &= ok(result, "Keyset 'Text'", info->name);
  }

  //-----------------------------------------------------------------------------
  // Keyset 'Zeroes'

  if(g_testZeroes || g_testAll)
  {
    printf("### Keyset 'Zeroes' Tests ###\n");

    bool drawDiagram = false;
    int keycount = 256 * 1024;
    Rand r(834192);

    bool result = RepeatedCharKeyTest<hashtype>(
        hash, "Zeroes", 0, keycount, confidence, drawDiagram, r );

    pass &= ok(result, "Keyset 'Zeroes'", info->name);
  }

  //-----------------------------------------------------------------------------
  // Keyset 'Seed'

  if(g_testSeed || g_testAll)
  {
    printf("### Keyset 'Seed' Tests ###\n");

    bool result = true;
    bool drawDiagram = false;
    Rand seed_r(392612);
    // these tests suck. the avalanche tests probably do a better job
    // we should test way more keys than this.

    result &= SeedTest<seedtype,hashtype>( hash, 2000000, confidence, drawDiagram,
        seed_r, "The quick brown fox jumps over the lazy dog");
    result &= SeedTest<seedtype,hashtype>( hash, 2000000, confidence, drawDiagram,
        seed_r, "");
    result &= SeedTest<seedtype,hashtype>( hash, 2000000, confidence, drawDiagram,
        seed_r, "00101100110101101");
    result &= SeedTest<seedtype,hashtype>( hash, 2000000, confidence, drawDiagram,
        seed_r, "abcbcddbdebdcaaabaaababaaabacbeedbabseeeeeeeesssssseeeewwwww");

    pass &= ok(result, "Keyset 'Seed'", info->name);
  }

  //-----------------------------------------------------------------------------
  // Keyset 'Effs'

  if(g_testEffs || g_testAll)
  {
    printf("### Keyset 'Effs' Tests ###\n");

    bool drawDiagram = false;
    int keycount = 256 * 1024;
    Rand r(4139126);

    bool result = RepeatedCharKeyTest<hashtype>(
        hash, "Effs", 0xFF, keycount, confidence, drawDiagram, r );

    pass &= ok(result, "Keyset 'Effs'", info->name);
  }

  return pass;
}

template < typename hashtype >
bool testHash ( HashInfo * info, int self_test, double confidence )
{
  if (info->seedbits == 32) {
    return testHashWithSeed< hashtype, Blob<32> >(info, self_test, confidence);
  } else if (info->seedbits == 64) {
    return testHashWithSeed< hashtype, Blob<64> >(info, self_test, confidence);
  } else if (info->seedbits == 95) {
    return testHashWithSeed< hashtype, Blob<95> >(info, self_test, confidence);
  } else if (info->seedbits == 96) {
    return testHashWithSeed< hashtype, Blob<96> >(info, self_test, confidence);
  } else if (info->seedbits == 112) {
    return testHashWithSeed< hashtype, Blob<112> >(info, self_test, confidence);
  } else if (info->seedbits == 127) {
    return testHashWithSeed< hashtype, Blob<127> >(info, self_test, confidence);
  } else if (info->seedbits == 128) {
    return testHashWithSeed< hashtype, Blob<128> >(info, self_test, confidence);
  } else if (info->seedbits == 191) {
    return testHashWithSeed< hashtype, Blob<191> >(info, self_test, confidence);
  } else if (info->seedbits == 256) {
    return testHashWithSeed< hashtype, Blob<256> >(info, self_test, confidence);
  } else {
    printf("Invalid seed width %d for hash '%s'",
      info->seedbits, info->name);
    exit(1);
  }
  return false;
}

//-----------------------------------------------------------------------------

bool testHashByInfo ( HashInfo * pInfo, int self_test, double confidence )
{
  const char * name = pInfo->name;

  g_hashUnderTest = pInfo;
  if(pInfo->hashbits == 32)
  {
    return testHash<uint32_t>( pInfo, self_test, confidence );
  }
  else if(pInfo->hashbits == 64)
  {
    return testHash<uint64_t>( pInfo, self_test, confidence );
  }
  else if(pInfo->hashbits == 128)
  {
    return testHash<uint128_t>( pInfo, self_test, confidence );
  }
  else if(pInfo->hashbits == 256)
  {
    return testHash<uint256_t>( pInfo, self_test, confidence );
  }
  else
  {
    printf("Invalid hash bit width %d for hash '%s'",pInfo->hashbits,pInfo->name);
    exit(1);
  }
  return false;
}

/* vim: set sts=2 sw=2 et: */
