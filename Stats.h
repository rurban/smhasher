#pragma once

#include "Types.h"

#include <math.h>
#include <vector>
#include <map>
#include <limits>
#include <climits>
#include <algorithm>   // for std::sort
#include <string.h>    // for memset
#include <stdio.h>     // for printf
#include <assert.h>

void Seed_init (HashInfo *info, size_t seed);
void Hash_Seed_init (pfHash hash, size_t seed);
double calcScore ( const int * bins, const int bincount, const int ballcount );

void plot ( double n );

double chooseK ( int b, int k );
double chooseUpToK ( int n, int k );

//-----------------------------------------------------------------------------

inline uint32_t f3mix ( uint32_t k )
{
  k ^= k >> 16;
  k *= 0x85ebca6b;
  k ^= k >> 13;
  k *= 0xc2b2ae35;
  k ^= k >> 16;

  return k;
}

static void printHash(const void* key, size_t len)
{
    const unsigned char* const p = (const unsigned char*)key;
    assert(len < INT_MAX);
    for (int i=(int)len-1; i >= 0 ; i--) printf("%02x", p[i]);
    printf("  ");
}

//-----------------------------------------------------------------------------
// Sort the hash list, count the total number of collisions and return
// the first N collisions for further processing

template< typename hashtype >
unsigned int FindCollisions ( std::vector<hashtype> & hashes,
                              HashSet<hashtype> & collisions,
                              int maxCollisions = 1000,
                              bool drawDiagram = false)
{
  unsigned int collcount = 0;
#if 0
  // sort indices instead
  std::vector< std::pair<hashtype, size_t>> pairs;
  pairs.resize (hashes.size());
  for(size_t i = 0; i < hashes.size(); i++)
    {
      pairs[i] = std::make_pair(hashes[i], i);
    }
  std::sort(pairs.begin(),pairs.end());
  for(size_t hnb = 1; hnb < pairs.size(); hnb++)
    {
      hashtype h1 = pairs[hnb].first;
      hashtype prev = pairs[hnb-1].first;
      if(h1 == prev)
        {
          collcount++;
          if((int)collisions.size() < maxCollisions)
            {
#ifdef DEBUG
              printf ("\n%zu <=> %zu: ", pairs[hnb-1].second, pairs[hnb].second);
              printHash(&h1, sizeof(hashtype));
#endif
              collisions.insert(h1);
            }
        }
    }
#else
    std::sort(hashes.begin(),hashes.end());

    for(size_t hnb = 1; hnb < hashes.size(); hnb++)
      {
        if(hashes[hnb] == hashes[hnb-1])
          {
            collcount++;
            if(collcount < maxCollisions)
              {
#ifdef DEBUG
                printf ("\n%zu: ", hnb);
                printHash(&hashes[hnb], sizeof(hashtype));
#endif
                if (drawDiagram)
                  collisions.insert(hashes[hnb]);
              }
          }
      }
#endif

#ifdef DEBUG
    if (collcount)
      printf ("\n");
#endif
  return collcount;
}

// Note: with 32bit 77163 keys will get a 50% probability of one collision.

// Naive multiplication, no accuracy at all
static double ExpectedNBCollisions_Slow ( const double nbH, const double nbBits )
{
  long balls = nbH;
  long double bins = nbBits;
  long double result = 1.0;
  for (long i = 1; i < balls / 2; i++) {
    // take a pair from the front and the end to minimize errors
    result *= ((bins - i) / bins) * ((bins - (nbH - i)) / bins);
  }
  return (double)(nbH * result);
}

// TODO This only works for a low number of collisions
static inline double ExpectedCollisions ( const double balls, const double bins )
{
  return balls - (bins * (1 - pow((bins - 1)/bins, balls)));
}

// Still too inaccurate: https://preshing.com/20110504/hash-collision-probabilities/
static double EstimateNbCollisions_Taylor(const double nbH, const double nbBits)
{
  const long double k = nbH;
  const long double b = nbBits;
  return (double)(k * (1.0 - expl(-0.5 * k * (k - 1.0) / b)));
}

// demerphq: (double(count) * double(count-1)) / pow(2.0,double(sizeof(hashtype) * 8 + 1));
// the very same as our calc. pow 2 vs exp2. Just the high cutoff is missing here.
static double EstimateNbCollisions_Demerphq(const double nbH, const double nbBits)
{
  return (nbH * (nbH - 1)) / pow(2.0, nbBits + 1);
}

// The currently best calculation, highly prone to inaccuracies with low results (1.0 - 10.0)
// TODO: return also the error.
static double EstimateNbCollisions(const int nbH, const int nbBits)
{
#if 0
  //return ExpectedNBCollisions_Slow((const double)nbH, (const double)nbBits);
  return EstimateNbCollisions_Demerphq((const double)nbH, (const double)nbBits);
  //return EstimateNbCollisions_Taylor((const double)nbH, (const double)nbBits);
  //return ExpectedCollisions((const double)nbH, (const double)nbBits);
#else
  double exp = exp2((double)nbBits); // 2 ^ bits
  double result = (double(nbH) * double(nbH-1)) / (2.0 * exp);
  if (result > (double)nbH)
    result = (double)nbH;
  // improved floating point accuracy
  if (result <= exp || nbBits > 32)
    return result;
  return result - exp;
#endif
}

template< typename hashtype >
bool CountLowbitsCollisions ( std::vector<hashtype> & revhashes, int nbLBits)
{
  const int origBits = sizeof(hashtype) * 8;
  int shiftBy = origBits - nbLBits;

  if (shiftBy <= 0) return true;

  size_t const nbH = revhashes.size();
  double expected = EstimateNbCollisions(nbH, nbLBits);
  printf("Testing collisions (low  %2i-bit) - Expected %12.1f, ", nbLBits, expected);
  int collcount = 0;

  for (size_t hnb = 1; hnb < nbH; hnb++)
  {
#ifdef DEBUG
    hashtype const h1x = revhashes[hnb-1];
    hashtype const h2x = revhashes[hnb];
#endif
    hashtype const h1 = revhashes[hnb-1] >> shiftBy;
    hashtype const h2 = revhashes[hnb]   >> shiftBy;
    if(h1 == h2)
      collcount++;
  }

  double ratio = double(collcount) / expected;
  printf("actual %6i (%.2fx)", collcount, expected > 0.0 ? ratio : (double)collcount);
  if (ratio > 0.98 && collcount != (int)expected)
    printf(" (%i)", collcount - (int)expected);

  // low estimation values are too inaccurate
  if (expected >= 0.1 && expected <= 10.0)
    {
      if (ratio > 4.0)
        {
          printf(" !!!!!\n");
          return false;
        }
      else if (ratio > 2.0)
        printf(" !");
    }
  // allow expected 0.3 and actual 1
  else if (ratio > 2.0 && collcount > 1)
  {
    printf(" !!!!!\n");
    return false;
  }

  printf("\n");
  return true;
}

template< typename hashtype >
bool CountHighbitsCollisions ( std::vector<hashtype> & hashes, int nbHBits)
{
  int origBits = sizeof(hashtype) * 8;
  int shiftBy = origBits - nbHBits;

  if (shiftBy <= 0) return true;

  size_t const nbH = hashes.size();
  double expected = EstimateNbCollisions(nbH, nbHBits);
  printf("Testing collisions (high %2i-bit) - Expected %12.1f, ", nbHBits, expected);
  int collcount = 0;

  for (size_t hnb = 1; hnb < nbH; hnb++)
  {
#ifdef DEBUG
    hashtype const h1x = hashes[hnb-1];
    hashtype const h2x = hashes[hnb];
#endif
    hashtype const h1 = hashes[hnb-1] >> shiftBy;
    hashtype const h2 = hashes[hnb]   >> shiftBy;
    if(h1 == h2)
      collcount++;
  }

  double ratio = double(collcount) / expected;
  printf("actual %6i (%.2fx)", collcount, expected > 0.0 ? ratio : (double)collcount);
  if (ratio > 0.98 && collcount != (int)expected)
    printf(" (%i)", collcount - (int)expected);

  // low estimation values are too inaccurate
  if (expected >= 0.1 && expected <= 10.0)
    {
      if (ratio > 4.0)
        {
          printf(" !!!!!\n");
          return false;
        }
      else if (ratio > 2.0)
        printf(" !");
    }
  // allow expected 0.3 and actual 1
  else if (ratio > 2.0 && collcount > 1)
  {
    printf(" !!!!!\n");
    return false;
  }

  printf("\n");
  return true;
}

static int FindMinBits_TargetCollisionShare(int nbHashes, double share)
{
    int nb;
    for (nb=2; nb<64; nb++) {
        double const maxColls = (double)(1ULL << nb) * share;
        double const nbColls = EstimateNbCollisions(nbHashes, nb);
        if (nbColls < maxColls) return nb;
    }
    assert(0);
    return nb;
}

static int FindMaxBits_TargetCollisionNb(int nbHashes, int minCollisions)
{
    int nb;
    for (nb=63; nb>2; nb--) {
        double const nbColls = EstimateNbCollisions(nbHashes, nb);
        if (nbColls > minCollisions) return nb;
    }
    //assert(0);
    return nb;
}

template< typename hashtype >
int CountNbCollisions ( std::vector<hashtype> & hashes, int nbHBits)
{
  const int origBits = sizeof(hashtype) * 8;
  const int shiftBy = origBits - nbHBits;

  assert(shiftBy > 0);

  size_t const nbH = hashes.size();
  int collcount = 0;

  for (size_t hnb = 1; hnb < nbH; hnb++)
  {
    hashtype const h1 = hashes[hnb-1] >> shiftBy;
    hashtype const h2 = hashes[hnb]   >> shiftBy;
    if(h1 == h2)
    {
      collcount++;
    }
  }

  return collcount;
}

template< typename hashtype >
bool TestLowbitsCollisions ( std::vector<hashtype> & revhashes)
{
  int origBits = sizeof(hashtype) * 8;

  size_t const nbH = revhashes.size();
  int const minBits = FindMinBits_TargetCollisionShare(nbH, 0.01);
  int const maxBits = FindMaxBits_TargetCollisionNb(nbH, 20);
  if (maxBits <= 0 || maxBits >= origBits) return true;

  printf("Testing collisions (low  %2i-%2i bits) - ", minBits, maxBits);
  double maxCollDev = 0.0;
  int maxCollDevBits = 0;
  int maxCollDevNb = 0;
  double maxCollDevExp = 1.0;

  for (int b = minBits; b <= maxBits; b++) {
      int    const nbColls = CountNbCollisions(revhashes, b);
      double const expected = EstimateNbCollisions(nbH, b);
      assert(expected > 0.0);
      double const dev = (double)nbColls / expected;
      if (dev > maxCollDev) {
          maxCollDev = dev;
          maxCollDevBits = b;
          maxCollDevNb = nbColls;
          maxCollDevExp = expected;
      }
  }

  printf("Worst is %2i bits: %2i/%2i (%.2fx)",
        maxCollDevBits, maxCollDevNb, (int)maxCollDevExp, maxCollDev);

  if (maxCollDev > 2.0) {
    printf(" !!!!!\n");
    return false;
  }

  printf("\n");
  return true;
}

template< typename hashtype >
bool TestHighbitsCollisions ( std::vector<hashtype> & hashes)
{
  int origBits = sizeof(hashtype) * 8;

  size_t const nbH = hashes.size();
  int const minBits = FindMinBits_TargetCollisionShare(nbH, 0.01);
  int const maxBits = FindMaxBits_TargetCollisionNb(nbH, 20);
  if (maxBits >= origBits) return true;

  printf("Testing collisions (high %2i-%2i bits) - ", minBits, maxBits);
  double maxCollDev = 0.0;
  int maxCollDevBits = 0;
  int maxCollDevNb = 0;
  double maxCollDevExp = 1.0;

  for (int b = minBits; b <= maxBits; b++) {
      int    const nbColls = CountNbCollisions(hashes, b);
      double const expected = EstimateNbCollisions(nbH, b);
      assert(expected > 0.0);
      double const dev = (double)nbColls / expected;
      if (dev > maxCollDev) {
          maxCollDev = dev;
          maxCollDevBits = b;
          maxCollDevNb = nbColls;
          maxCollDevExp = expected;
      }
  }

  printf("Worst is %2i bits: %2i/%2i (%.2fx)",
        maxCollDevBits, maxCollDevNb, (int)maxCollDevExp, maxCollDev);

  if (maxCollDev > 2.0) {
    printf(" !!!!!\n");
    return false;
  }

  printf("\n");
  return true;
}

//-----------------------------------------------------------------------------

template < typename hashtype >
int PrintCollisions ( HashSet<hashtype> & collisions )
{
  printf("\nCollisions:\n");
  for (typename HashSet<hashtype>::iterator it = collisions.begin();
       it != collisions.end(); ++it)
  {
    const hashtype &hash = *it;
    printhex(&hash, sizeof(hashtype));
    printf("\n");
  }
  return 0;
}

//----------------------------------------------------------------------------
// Measure the distribution "score" for each possible N-bit span up to 20 bits

template< typename hashtype >
bool TestDistribution ( std::vector<hashtype> & hashes, bool drawDiagram )
{
  printf("Testing distribution - ");

  if(drawDiagram) printf("\n");

  const int hashbits = sizeof(hashtype) * 8;

  int maxwidth = 20;

  // We need at least 5 keys per bin to reliably test distribution biases
  // down to 1%, so don't bother to test sparser distributions than that

  while(double(hashes.size()) / double(1 << maxwidth) < 5.0)
  {
    if (0 == maxwidth) break;
    maxwidth--;
  }

  std::vector<int> bins;
  bins.resize(1 << maxwidth);

  double worst = 0;
  int worstStart = -1;
  int worstWidth = -1;

  for(int start = 0; start < hashbits; start++)
  {
    int width = maxwidth;
    int bincount = (1 << width);

    memset(&bins[0],0,sizeof(int)*bincount);

    for(size_t j = 0; j < hashes.size(); j++)
    {
      hashtype & hash = hashes[j];

      uint32_t index = window(&hash,sizeof(hash),start,width);

      bins[index]++;
    }

    // Test the distribution, then fold the bins in half,
    // repeat until we're down to 256 bins

    if(drawDiagram) printf("[");

    while(bincount >= 256)
    {
      double n = calcScore(&bins[0],bincount,(int)hashes.size());

      if(drawDiagram) plot(n);

      if(n > worst)
      {
        worst = n;
        worstStart = start;
        worstWidth = width;
      }

      width--;
      bincount /= 2;

      if(width < 8) break;

      for(int i = 0; i < bincount; i++)
      {
        bins[i] += bins[i+bincount];
      }
    }

    if(drawDiagram) printf("]\n");
  }

  double pct = worst * 100.0;

  printf("Worst bias is the %2d-bit window at bit %2d - %.3f%%",
         worstWidth, worstStart, pct);
  if(pct >= 1.0) {
    printf(" !!!!!\n");
    return false;
  }
  else {
    printf("\n");
    return true;
  }
}

//----------------------------------------------------------------------------

static int FindNbBitsForCollisionTarget(int targetNbCollisions, int nbHashes)
{
    int nb;
    double const target = (double)targetNbCollisions;
    for (nb=2; nb<64; nb++) {
        double nbColls = EstimateNbCollisions(nbHashes, nb);
        if (nbColls < target) break;
    }

    if ((EstimateNbCollisions(nbHashes, nb)) > targetNbCollisions/5)
        return nb;

    return nb-1;
}

// 0xf00f1001 => 0x8008f00f
template <typename hashtype>
hashtype bitreverse(hashtype n, size_t b = sizeof(hashtype) * 8)
{
    assert(b <= std::numeric_limits<hashtype>::digits);
    hashtype rv = 0;
    for (size_t i = 0; i < b; i += 8) {
        rv <<= 8;
        rv |= bitrev(n & 0xff); // ensure overloaded |= op for Blob not underflowing
        n >>= 8;
    }
    return rv;
}

template < typename hashtype >
bool TestHashList ( std::vector<hashtype> & hashes, bool drawDiagram,
                    bool testCollision = true, bool testDist = true,
                    bool testHighBits = true, bool testLowBits = true,
                    bool verbose = true)
{
  bool result = true;

  if (testCollision)
  {
    size_t const count = hashes.size();
    double const expected = EstimateNbCollisions(count, sizeof(hashtype) * 8);
    if (verbose)
      printf("Testing collisions (%3i-bit) - Expected %6.1f, ",
             (int)sizeof(hashtype)*8, expected);
    const int i_expected = (int)expected;

    int collcount = 0;
    HashSet<hashtype> collisions;
    collcount = FindCollisions(hashes, collisions, 1000, drawDiagram);
    double ratio = double(collcount) / expected;
    if (verbose) {
      printf("actual %6i (%.2fx)", (int)collcount, expected > 0.0 ? ratio : (double)collcount);
      if (ratio > 0.98 && collcount != i_expected)
        printf(" (%i)", collcount - i_expected);
    }

    if (sizeof(hashtype) <= sizeof(uint32_t))
    {
      // fail with >= 2x expected collisions

      // TODO - collision failure cutoff needs to be expressed as a standard deviation instead
      // of a scale factor, otherwise we fail erroneously if there are a small expected number
      // of collisions

      // low estimation values are too inaccurate
      if (expected >= 0.1 && expected <= 10.0)
        {
          ratio = ceil(ratio);
          if (ceil(ratio) > 4.0)
            {
              printf(" !!!!!\n");
              return false;
            }
          else if (ceil(ratio) > 2.0)
            printf(" !");
        }
      // allow expected 0.3 and actual 1
      else if (ceil(ratio) > 2.0 && collcount > 1)
        {
          printf(" !!!!!\n");
          return false;
        }
    }
    else
    {
      // For all hashes larger than 32 bits, _any_ collisions are a failure.
      if (collcount > 0 && expected < 1.0)
      {
        printf(" !!!!!");
        result = false;
        if(drawDiagram)
          {
            PrintCollisions(collisions);
            //printf("Mapping collisions\n");
            //CollisionMap<uint128_t,ByteVec> cmap;
            //CollisionCallback<uint128_t> c2(hash,collisions,cmap);
            ////TwoBytesKeygen(20,c2);
            //printf("Dumping collisions\n");
            //DumpCollisionMap(cmap);
          }
      }
    }

    if (verbose) {
      printf("\n");
    }
    fflush(NULL);

    if (testHighBits) {
      result &= CountHighbitsCollisions(hashes, 224);
      result &= CountHighbitsCollisions(hashes, 160);
      result &= CountHighbitsCollisions(hashes, 128);
      result &= CountHighbitsCollisions(hashes,  64);
      result &= CountHighbitsCollisions(hashes,  32);

      /*
        int const optimalNbBits = FindNbBitsForCollisionTarget(100, count);
        result &= CountHighbitsCollisions(hashes, optimalNbBits);
      */

      result &= TestHighbitsCollisions(hashes);

      /* Following tests are too small : tables are necessarily saturated.
       * It would be better to count the nb of collisions per Cell,
       * and compared the distribution of values against a random source.
       * But this is a different test */
      result &= CountHighbitsCollisions(hashes,   12);
      result &= CountHighbitsCollisions(hashes,   8);
    }
    if (testLowBits) {
      // reverse: bitwise flip the hashes. lowest bits first
      std::vector<hashtype> revhashes = hashes;
      for (size_t i = 0; i < revhashes.size(); i++) {
        revhashes[i] = bitreverse(hashes[i]);
      }
      std::sort(revhashes.begin(), revhashes.end());

      result &= CountLowbitsCollisions(revhashes, 224);
      result &= CountLowbitsCollisions(revhashes, 160);
      result &= CountLowbitsCollisions(revhashes, 128);
      result &= CountLowbitsCollisions(revhashes,  64);
      result &= CountLowbitsCollisions(revhashes,  32);

      /*
        int const optimalNbBits = FindNbBitsForCollisionTarget(100, count);
        result &= CountLowbitsCollisions(hashes, optimalNbBits);
      */

      result &= TestLowbitsCollisions(revhashes);
      /* Following tests are too small : tables are necessarily saturated.
       * It would be better to count the nb of collisions per Cell,
       * and compared the distribution of values against a random source.
       * But this is a different test */
      /* rurban: No, these tests are for non-prime hash tables, using only
         the lower 5-10 bits */
      result &= CountLowbitsCollisions(revhashes,   12);
      result &= CountLowbitsCollisions(revhashes,   8);

      //std::vector<hashtype>().swap(revhashes);
      //revhashes.clear();
    }
  }


  //----------

  if(testDist)
  {
    result &= TestDistribution(hashes,drawDiagram);
  }

  return result;
}

//-----------------------------------------------------------------------------

template < class keytype, typename hashtype >
bool TestKeyList ( hashfunc<hashtype> hash, std::vector<keytype> & keys,
                   bool drawDiagram, bool testColl, bool testDist )
{
  int keycount = (int)keys.size();

  std::vector<hashtype> hashes;
  hashes.resize(keycount);

  printf("Hashing");
  for(int i = 0; i < keycount; i++)
  {
    if(i % (keycount / 10) == 0) printf(".");

    keytype & k = keys[i];

    hash(&k,sizeof(k),0,&hashes[i]);
  }
  printf("\n");

  bool result = TestHashList(hashes,drawDiagram,testColl,testDist);
  printf("\n");

  return result;
}

//-----------------------------------------------------------------------------
// Bytepair test - generate 16-bit indices from all possible non-overlapping
// 8-bit sections of the hash value, check distribution on all of them.

// This is a very good test for catching weak intercorrelations between bits -
// much harder to pass than the normal distribution test. However, it doesn't
// really model the normal usage of hash functions in hash table lookup, so
// I'm not sure it's that useful (and hash functions that fail this test but
// pass the normal distribution test still work well in practice)

template < typename hashtype >
double TestDistributionBytepairs ( std::vector<hashtype> & hashes, bool drawDiagram )
{
  const int nbytes = sizeof(hashtype);
  const int hashbits = nbytes * 8;

  const int nbins = 65536;

  std::vector<int> bins(nbins,0);

  double worst = 0;

  for(int a = 0; a < hashbits; a++)
  {
    if(drawDiagram) if((a % 8 == 0) && (a > 0)) printf("\n");

    if(drawDiagram) printf("[");

    for(int b = 0; b < hashbits; b++)
    {
      if(drawDiagram) if((b % 8 == 0) && (b > 0)) printf(" ");

      bins.clear();
      bins.resize(nbins,0);

      for(size_t i = 0; i < hashes.size(); i++)
      {
        hashtype & hash = hashes[i];

        uint32_t pa = window(&hash,sizeof(hash),a,8);
        uint32_t pb = window(&hash,sizeof(hash),b,8);

        bins[pa | (pb << 8)]++;
      }

      double s = calcScore(bins,bins.size(),hashes.size());

      if(drawDiagram) plot(s);

      if(s > worst)
      {
        worst = s;
      }
    }

    if(drawDiagram) printf("]\n");
  }

  return worst;
}

//-----------------------------------------------------------------------------
// Simplified test - only check 64k distributions, and only on byte boundaries

template < typename hashtype >
void TestDistributionFast ( std::vector<hashtype> & hashes, double & dworst, double & davg )
{
  const int hashbits = sizeof(hashtype) * 8;
  const int nbins = 65536;

  std::vector<int> bins(nbins,0);

  dworst = -1.0e90;
  davg = 0;

  for(int start = 0; start < hashbits; start += 8)
  {
    bins.clear();
    bins.resize(nbins,0);

    for(size_t j = 0; j < hashes.size(); j++)
    {
      hashtype & hash = hashes[j];

      uint32_t index = window(&hash,sizeof(hash),start,16);

      bins[index]++;
    }

    double n = calcScore(&bins.front(),(int)bins.size(),(int)hashes.size());

    davg += n;

    if(n > dworst) dworst = n;
  }

  davg /= double(hashbits/8);
}

//-----------------------------------------------------------------------------
