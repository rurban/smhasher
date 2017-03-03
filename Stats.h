#pragma once

#include "Types.h"
#include "HashFunc.h"
#include <math.h>
#include <vector>
#include <map>
#include <algorithm>   // for std::sort
#include <string.h>    // for memset
#include <stdio.h>     // for printf
#include <gsl/gsl_sf.h>
#include <gsl/gsl_sys.h>

// Calculate the log of the ratio o/e without
// rounding/cancelling issues by using log1p when
// necessary. If o is slightly bigger than e, and e
// is big enough then log(o/e) will be 0 when it
// should not be, whereas log1p() won't have this issue.
inline double _logoe(double o, double e) {
  double ret;
  if (o == e) {
    return 0.0;
  } else if ( o > e ) {
    return log1p((o-e)/e);
  }
  return log(o/e);
}

#define GTEST_PROB(bins,gval) \
  ( 1.0 - gsl_sf_gamma_inc_Q( ( double(bins) - 1.0) / 2.0, (gval) ) )
#define GTEST_ADD(gtest,observed,expected) \
    if (observed) gtest += ( double(observed) * _logoe( double(observed), double(expected) ) )

inline double sigmasToProb ( double sigmas ) {
  return erf(sigmas/sqrt(2));
}

double calcScore_old ( const int * vals, const int count, const int sum);
double calcScore ( const int * vals, const int count, const int sum, double confidence );
double calcGTestProbability ( const int * vals, const int count, const int sum );
double calcGTestProbability ( const int * vals, const int count);
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
bool TestDistribution ( std::vector<hashtype> & hashes, double confidence, bool drawDiagram )
{
  printf("Testing distribution - ");

  //drawDiagram = true;
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
  int scores = 0;

  for(int start = 0; start < hashbits; start++)
  {
    int width = maxwidth;
    int bincount = (1 << width);

    memset(&bins[0],0,sizeof(int)*bincount);

    for(size_t j = 0; j < hashes.size(); j++)
    {
      hashtype & hash = hashes[j];

      uint32_t index = window(&hash, sizeof(hash), start, width);

      bins[index]++;
    }

    // Test the distribution, then fold the bins in half,
    // repeat until we're down to 256 bins

    if(drawDiagram) printf("[");

    while(bincount >= 256)
    {
      double n = calcScore(&bins[0],bincount,(int)hashes.size(), confidence);
      scores++;
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

  if (worst) {
    printf("not ok! (%.6f confidence) - worst bias is the %3d-bit window at bit %3d - %5.3f%% (%d)\n",
      confidence * 100, worstWidth, worstStart, worst * 100, scores);
    return false;
  } else {
    printf("ok. (%.6f confidence)\n",confidence * 100);
    return true;
  }
}

//----------------------------------------------------------------------------

template < typename hashtype >
bool TestHashList ( std::vector<hashtype> & hashes, std::vector<hashtype> & collisions, double confidence , bool drawDiagram )
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

  if(confidence)
  {
    result &= TestDistribution(hashes,confidence,drawDiagram);
  }

  return result;
}

//----------

template < typename hashtype >
bool TestHashList ( std::vector<hashtype> & hashes, bool /*testColl*/, double confidence, bool drawDiagram )
{
  std::vector<hashtype> collisions;

  return TestHashList(hashes,collisions,confidence,drawDiagram);
}

//-----------------------------------------------------------------------------


//-----------------------------------------------------------------------------
/* vim: set sts=2 sw=2 et: */
