#pragma once

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
/* vim: set sts=2 sw=2 et: */
