#include "Birthday.h"
#include <math.h>

double Birthday::collisionProbability(unsigned hashbits, double tests, double extras) {
  double hashes = exp2((double)hashbits);

  // First compute the probability of avoiding a collision among
  // the tests. The formula is
  //   p = h! / (h-t)! / h^t
  // which through Stirling's approximation and a page or so of
  // algebra works out as
  //  log(p) = -log(1-t/h)(h-t+1/2) - t
  // but we need to be careful about numeric effects.
  double frac = tests / hashes;
  double mlog;       // becomes -log(1-frac)
  double logsuccess; // becomes log(p)
  if( tests >= hashes || frac >= 1.0 ) {
    // Pigeonhole principle says there will always be a collision
    return 1.0;
  } else if( tests < 2.5 ) {
    mlog = -log1p(-frac);
    if( tests < 1.5 )
      logsuccess = 0;
    else
      logsuccess = -mlog;
  } else if( frac > 1e-8 ) {
    // In this range the main numeric risk is that mlog*h is nearly
    // canceled by t, so we need to be sure to subtract those before
    // adding the other terms in.
    // In principle the log1p function loses some precision if frac
    // is close to 1, but that would be lost anyway because the end
    // result rounds up to 1.0 no matter what.
    mlog = -log1p(-frac);
    logsuccess = (mlog-frac) * hashes - mlog * (tests - 0.5);
  } else {
    // Here is frac too small for the difference mlog-frac to be visible
    // compared to mlog. Instead of log1p we will manually approximate
    // log(1+x)-x ~~ -x^2/2
    double diff = frac*frac*0.5 ;
    mlog = frac + diff;
    logsuccess = diff * hashes - mlog * (tests - 0.5);
    // The threshold of 1e-8 was chosen as the largest order of
    // magnitude before a jump in the value between the strategies
    // begins to be visible in the tests below. (The small-frac
    // algorithm generally gives smoother output, so we'll want to
    // use it until the higher-order terms in the logarithm series
    // begin to be visible).
  }

  // Now multiply with the probability of the additional tries succeeding:
  //   p = (1-b/h)^m
  logsuccess -= mlog * extras ;

  // Since we want the probability of failure, compute 1 - e^logsucces
  return -expm1(logsuccess);
}

// --------------------------------------------------------------------------

#ifdef INCLUDE_TEST_MAIN
#include <stdio.h>

static void checkContinuity(int bits, double threshold, double span) {
  double hashes = exp2(bits);
  double prev = 0;
  double prevdelta = 0;
  double step = 2*span / 17 ;
  for( double frac = threshold-span ; frac <= threshold+span ; frac += step ) {
    double tests = ceil(hashes*frac);
    frac = tests/hashes ;
    double prob = Birthday::collisionProbability(bits, tests, 0);
    double delta = prob-prev;
    printf("%d bits, p=%.9g for %g tests, frac %g (+ %g + %g)\n",
           bits, prob, tests, frac, delta, delta-prevdelta);
    prev = prob;
    prevdelta = delta;
  }
  printf("\n");
}

int main(int argc, const char **argv) {
  checkContinuity(3, 0.5, 0.5);
  checkContinuity(6, 0.5, 0.1);
  checkContinuity(7, 0.5, 0.06);
  checkContinuity(8, 0.5, 0.03);

  checkContinuity(128, 1e-29, 0.5e-29);

  checkContinuity(48, 1e-8, 0.0005e-8);
  checkContinuity(56, 1e-8, 0.0005e-8);
  checkContinuity(64, 1e-8, 0.0005e-8);
  return 0;
}
#endif
