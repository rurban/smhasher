#pragma once

#include "Types.h"
#include "HashFunc.h"
#include "SimpleStats.h"
#include "TAP.h"
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
bool TestDistribution ( std::vector<hashtype> & hashes, double confidence, bool drawDiagram, const char *name )
{
  //printf("# Testing distribution - ");

  //drawDiagram = true;
  if(drawDiagram) printf("\n"); // nl ok

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
  bool result = !worst;

  if (worst)
    printf("# worst bias is the %3d-bit window at bit %3d - score %5.3f (from %d scores)\n",
      worstWidth, worstStart, worst, scores);
  return okf(result, "Distribution Bias Check for %s", name);
}

//----------------------------------------------------------------------------

template < typename hashtype >
bool TestHashList ( std::vector<hashtype> & hashes, std::vector<hashtype> & collisions, double confidence , bool drawDiagram, const char *name )
{
  bool result = true;

  size_t count = hashes.size();

  double expected = (double(count) * double(count-1)) / pow(2.0,double(sizeof(hashtype) * 8 + 1));

  printf("# Testing collisions   - Expected %8.2f, ",expected);

  HashSet<hashtype> my_collisions;
  double collcount = FindCollisions(hashes,my_collisions,1000);

  if( sizeof(hashtype) == sizeof(uint32_t) )
  {
      // 2x expected collisions = fail

      // #TODO - collision failure cutoff needs to be expressed as a standard deviation instead
      // of a scale factor, otherwise we fail erroneously if there are a small expected number
      // of collisions

      if(double(collcount) / double(expected) > 2.0 &&
         fabs(double(collcount) - double(expected)) > 1)
      {
        result = false;
      }
  }
  else
  if(collcount > 0) {
    // For all hashes larger than 32 bits, _any_ collisions are a failure.
    // not sure this is right really. but for now we will live with it
      result = false;
  }

  if (collcount / expected < 1000) {
    printf("actual %8.0f (%5.2fx) - %s\n",collcount, collcount / expected,
        result ? "passed" : "failed");
  } else if (collcount == count - 1) {
    printf("actual %8.0f - ALL keys were in collision - %s\n",collcount,
        result ? "passed" : "failed" );
  } else {
    printf("actual %8.0f - Excessive keys in collision - %s\n",collcount,
        result ? "passed" : "failed");
  }
  okf(result,"Collision Rate for %s",name);
  //----------

  if(confidence)
    result &= TestDistribution<hashtype>(hashes,confidence,drawDiagram, name);

  return result;
}

//----------

template < typename hashtype >
bool TestHashList ( std::vector<hashtype> & hashes, bool /*testColl*/, double confidence, bool drawDiagram, const char *name )
{
  std::vector<hashtype> collisions;

  return TestHashList<hashtype>(hashes,collisions,confidence,drawDiagram,name);
}

//-----------------------------------------------------------------------------
/* vim: set sts=2 sw=2 et: */
