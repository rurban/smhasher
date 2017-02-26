#pragma once

#include "Types.h"
#include "HashFunc.h"
#include <math.h>
#include <vector>
#include <map>
#include <algorithm>   // for std::sort
#include <string.h>    // for memset
#include <stdio.h>     // for printf
#include <gsl/gsl_sys.h>
double calcScore ( const int * bins, const int bincount, const int ballcount );

void plot ( double n );

inline double ExpectedCollisions ( double balls, double bins )
{
  return balls - bins + bins * pow(1.0 - 1.0/bins, balls);
}

inline double BirthdayCount(double probability, int bits )
{
    return sqrt( 2.0 * pow(2.0,double(bits)) * ( 0 - gsl_log1p( 0 - probability ) ) );
}

// used by DiffTest in DifferentialTest.h
double chooseK ( int b, int k );
double chooseUpToK ( int n, int k );

//-----------------------------------------------------------------------------
// used by the CyclicKeyTest
inline uint32_t f3mix ( uint32_t k )
{
  k ^= k >> 16;
  k *= 0x85ebca6b;
  k ^= k >> 13;
  k *= 0xc2b2ae35;
  k ^= k >> 16;

  return k;
}

//-----------------------------------------------------------------------------
// Sort the hash list, count the total number of collisions and return
// the first N collisions for further processing
//
// Used by TestHashList which is widely used elsewhere.

template< typename hashtype >
int FindCollisions ( std::vector<hashtype> & hashes, 
                     HashSet<hashtype> & collisions,
                     int maxCollisions )
{
  int collcount = 0;

  std::sort(hashes.begin(),hashes.end());

  for(size_t i = 1; i < hashes.size(); i++)
  {
    if(hashes[i] == hashes[i-1])
    {
      collcount++;

      if((int)collisions.size() < maxCollisions)
      {
        collisions.insert(hashes[i]);
      }
    }
  }

  return collcount;
}

//----------------------------------------------------------------------------
// Measure the distribution "score" for each possible N-bit span up to 20 bits
//
// Used by TestHashList, which is widely used elsewhere.

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

  printf("Worst bias is the %3d-bit window at bit %3d - %5.3f%%",worstWidth,worstStart,pct);
  if(pct >= 1.0) printf(" !!!!! ");
  printf("\n");

  return pct < 1.0;
}

//----------------------------------------------------------------------------

template < typename hashtype >
bool TestHashList ( std::vector<hashtype> & hashes, std::vector<hashtype> & collisions, bool testDist, bool drawDiagram )
{
  bool result = true;

  {
    size_t count = hashes.size();

    double expected = (double(count) * double(count-1)) / pow(2.0,double(sizeof(hashtype) * 8 + 1));

    printf("Testing collisions   - Expected %8.2f, ",expected);

    double collcount = 0;

    HashSet<hashtype> collisions;

    collcount = FindCollisions(hashes,collisions,1000);

    printf("actual %8.2f (%5.2fx)",collcount, collcount / expected);

    if(sizeof(hashtype) == sizeof(uint32_t))
    {
        // 2x expected collisions = fail

        // #TODO - collision failure cutoff needs to be expressed as a standard deviation instead
        // of a scale factor, otherwise we fail erroneously if there are a small expected number
        // of collisions

        if(double(collcount) / double(expected) > 2.0 &&
           abs(double(collcount) - double(expected)) > 1)
        {
          printf(" !!!!! ");
          result = false;
        }
    }
    else
    {
      // For all hashes larger than 32 bits, _any_ collisions are a failure.
      
      if(collcount > 0)
      {
        printf(" !!!!! ");
        result = false;
      }
    }

    printf("\n");
  }

  //----------

  if(testDist)
  {
    result &= TestDistribution(hashes,drawDiagram);
  }

  return result;
}

//----------

template < typename hashtype >
bool TestHashList ( std::vector<hashtype> & hashes, bool /*testColl*/, bool testDist, bool drawDiagram )
{
  std::vector<hashtype> collisions;

  return TestHashList(hashes,collisions,testDist,drawDiagram);
}

//-----------------------------------------------------------------------------


//-----------------------------------------------------------------------------
/* vim: set sts=2 sw=2 et: */
