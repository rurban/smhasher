//-----------------------------------------------------------------------------
// Flipping a single bit of a key should cause an "avalanche" of changes in
// the hash function's output. Ideally, each output bits should flip 50% of
// the time - if the probability of an output bit flipping is not 50%, that bit
// is "biased". Too much bias means that patterns applied to the input will
// cause "echoes" of the patterns in the output, which in turn can cause the
// hash function to fail to create an even, random distribution of hash values.


#pragma once

#include "Types.h"
#include "Random.h"

#include <vector>
#include <stdio.h>
#include <math.h>
#include <gsl/gsl_sf.h>

// Avalanche fails if a bit is biased by more than 1%

/* no bit may be more than 1% from the expected 50% changed rate */
#define AVALANCHE_FAIL          0.01
/* g-test probability of the table (virtual) must be below 0.95 */
#define G_TEST_P_FAIL           0.95
/* Fail if our standard error is more than this many times the actual error.
 * The expected error varies with seed/key/hash bitsize and reps, so
 * we normalize the error we receive to that expected for a 32x32 square
 * with the given repetions. We then compare our normalized error rate to
 * the expected to get a ratio. A good hash function should average this to
 * 1 over many reps, and should probably never be further than twice the
 * expected value. Being under occasionally is fine. */
#define ERROR_SCALED_RATIO_FAIL 2.0
typedef struct av_stats {
  int     *bins;
  double  *gtests;
  double  *pcts;
  double  *col_gtests;
  double  *row_gtests;
  int     reps;
  int     seedbits;
  int     keybits;
  int     hashbits;
  int     num_bits;
  int     num_bins;
  int     num_rows;
  int     worst_y;
  int     worst_x;
  double  gtest_prob;
  double  worst_pct;
  double  err;
  double  err_scaled;
  double  expected_err_scaled;
  double  err_scaled_ratio;
} av_statst;

double calcBiasStats ( std::vector<int> & counts, int reps, av_statst *stats, double confidence );
void PrintAvalancheDiagram ( av_statst *stats, int mode, int scale, double confidence );

//-----------------------------------------------------------------------------

template < typename keytype, typename hashtype >
void calcBiasWithSeed ( pfHash hash, std::vector<int> & counts, int reps, Rand & r )
{
  const int keybytes = sizeof(keytype);
  const int hashbytes = sizeof(hashtype);

  const int seedbits = 32;
  const int keybits = keybytes * 8;
  const int hashbits = hashbytes * 8;
  uint32_t seed = 0;
  keytype K;
  hashtype A,B;

  for(int irep = 0; irep < reps; irep++)
  {
    if(irep % (reps/10) == 0) printf(".");

    r.rand_p(&K,keybytes);
    seed = r.rand_u32();

    hash(&K,keybytes,seed,&A);

    int * cursor = &counts[0];

    for(uint32_t mask= 1; mask; mask = mask << 1)
    {
      seed ^= mask;
      hash(&K,keybytes,seed,&B);
      seed ^= mask;

      for(int iOut = 0; iOut < hashbits; iOut++)
      {
        int bitA = getbit(&A,hashbytes,iOut) ? 2 : 0;
        int bitB = getbit(&B,hashbytes,iOut) ? 1 : 0;

        cursor[ bitA + bitB ]++;
        cursor += 4;
      }
    }

    for(int iBit = 0; iBit < keybits; iBit++)
    {
      flipbit(&K,keybytes,iBit);
      hash(&K,keybytes,seed,&B);
      flipbit(&K,keybytes,iBit);

      for(int iOut = 0; iOut < hashbits; iOut++)
      {
        int bitA = getbit(&A,hashbytes,iOut) ? 2 : 0;
        int bitB = getbit(&B,hashbytes,iOut) ? 1 : 0;

        cursor[ bitA + bitB ]++;
        cursor += 4;
      }
    }
  }
}

//-----------------------------------------------------------------------------

template < typename keytype, typename hashtype >
bool AvalancheTest ( pfHash hash, const int reps, double confidence )
{
  Rand r(923145681);
  int seedbits = 32;
  int keybits = sizeof(keytype) * 8;
  int hashbits = sizeof(hashtype) * 8;
  int num_rows = keybits + seedbits;
  int num_bits = ( keybits + seedbits ) * hashbits;
  int num_bins = num_bits * 4;
  std::vector<int> bins(num_bins,0);
  std::vector<double> gtests(num_bits,0);
  std::vector<double> pcts(num_bits,0);
  std::vector<double> col_gtest(hashbits,0);
  std::vector<double> row_gtest(num_rows,0);
  
  av_statst stats = {
    &bins[0],
    &gtests[0],
    &pcts[0],
    &col_gtest[0],
    &row_gtest[0],
    reps,
    seedbits,
    keybits,
    hashbits,
    num_bits,
    num_bins,
    num_rows,
  };

  printf("Testing %3d-bit seeds, %3d-bit keys -> %3d-bit hashes, %8d reps",
          stats.seedbits, stats.keybits, stats.hashbits, reps);

  //----------

  calcBiasWithSeed<keytype,hashtype>(hash, bins, reps, r);
  
  //----------

  bool result = true;

  double worst_bit_error = calcBiasStats( bins, reps, &stats, confidence );

  int failed= 0;
  int score_size= hashbits;
  std::vector<double> scores(score_size,0);
  std::vector<int> scores_incr(score_size,0);
  std::vector<int> scores_count(score_size,0);
  std::vector<int> scores_bad(score_size,0);
  std::vector<int> scores_failed(score_size,0);
  for (int start_p= 0; start_p < score_size; start_p++) {
    for (int incr= hashbits - 7; incr < hashbits + 7 ; incr++) {
      int cnt = 0;
      int bad = 0;
      for (int p= start_p; p < num_bits; p += incr) {
        cnt++;
        if (gtests[p] >= confidence) {
          bad++;
        }
      }
      double score= double(bad) / double(cnt);
      if ( score > scores[start_p] && score >= 0.4 ){
        scores[start_p]= score;
        scores_incr[start_p]= incr;
        scores_count[start_p]= cnt;
        scores_bad[start_p]= bad;
        scores_failed[start_p]= 1;
        failed++;
        if (0)
            printf("\nfailed at start pos %d (%d,%d), increment %d, count %d bad %d score %f\n",
              start_p, start_p/hashbits, start_p % hashbits, incr,
              scores_count[start_p], scores_bad[start_p], scores[start_p]);
      }
    }
  }

  if(
    worst_bit_error        >= AVALANCHE_FAIL          ||
    stats.err_scaled_ratio >= ERROR_SCALED_RATIO_FAIL ||
    stats.gtest_prob       >= G_TEST_P_FAIL           ||
    failed                                            ||
    !result
  ) {
    printf(" not ok!\n");
    result = false;
  } else {
    printf(" ok\n");
  }

  if (!result)
    PrintAvalancheDiagram(&stats, 3, 1, confidence);

  printf(
    "    worst bit: %5.3f%% error: %.8f error ratio: %.4f\n"
    "    probability not-random (g-test): %12.5f%%\n",
    worst_bit_error * 100.0,
    stats.err_scaled,
    stats.err_scaled_ratio,
    stats.gtest_prob * 100.0
  );
  for (int i= 0; i < score_size; i++) {
    if (scores_failed[i]) {
      printf("    Striping error at bit %d increment %d, %.2f%% in error! %d/%d\n",
        i, scores_incr[i], scores[i] * 100, scores_bad[i], scores_count[i]);
    }
  }
  if (!result)
    printf("FAILED\n");
  return result;
}

//----------------------------------------------------------------------------
// Tests the Bit Independence Criteron. Stricter than Avalanche, but slow and
// not really all that useful.

template< typename keytype, typename hashtype >
void BicTest ( pfHash hash, const int keybit, const int reps, double & maxBias, int & maxA, int & maxB, bool verbose )
{
  Rand r(11938);
  
  const int keybytes = sizeof(keytype);
  const int hashbytes = sizeof(hashtype);
  const int hashbits = hashbytes * 8;

  std::vector<int> bins(hashbits*hashbits*4,0);

  keytype key;
  hashtype h1,h2;

  for(int irep = 0; irep < reps; irep++)
  {
    if(verbose)
    {
      if(irep % (reps/10) == 0) printf(".");
    }

    r.rand_p(&key,keybytes);
    hash(&key,keybytes,0,&h1);

    flipbit(key,keybit);
    hash(&key,keybytes,0,&h2);

    hashtype d = h1 ^ h2;

    for(int out1 = 0; out1 < hashbits; out1++)
    for(int out2 = 0; out2 < hashbits; out2++)
    {
      if(out1 == out2) continue;

      uint32_t b = getbit(d,out1) | (getbit(d,out2) << 1);

      bins[(out1 * hashbits + out2) * 4 + b]++;
    }
  }

  if(verbose) printf("\n");

  maxBias = 0;

  for(int out1 = 0; out1 < hashbits; out1++)
  {
    for(int out2 = 0; out2 < hashbits; out2++)
    {
      if(out1 == out2)
      {
        if(verbose) printf("\\");
        continue;
      }

      double bias = 0;

      for(int b = 0; b < 4; b++)
      {
        double b2 = double(bins[(out1 * hashbits + out2) * 4 + b]) / double(reps / 2);
        b2 = fabs(b2 * 2 - 1);

        if(b2 > bias) bias = b2;
      }

      if(bias > maxBias)
      {
        maxBias = bias;
        maxA = out1;
        maxB = out2;
      }

      if(verbose) 
      {
        if     (bias < 0.01) printf(".");
        else if(bias < 0.05) printf("o");
        else if(bias < 0.33) printf("O");
        else                 printf("X");
      }
    }

    if(verbose) printf("\n");
  }
}

//----------

template< typename keytype, typename hashtype >
bool BicTest ( pfHash hash, const int reps )
{
  const int keybytes = sizeof(keytype);
  const int keybits = keybytes * 8;

  double maxBias = 0;
  int maxK = 0;
  int maxA = 0;
  int maxB = 0;

  for(int i = 0; i < keybits; i++)
  {
    if(i % (keybits/10) == 0) printf(".");

    double bias;
    int a,b;
    
    BicTest<keytype,hashtype>(hash,i,reps,bias,a,b,true);

    if(bias > maxBias)
    {
      maxBias = bias;
      maxK = i;
      maxA = a;
      maxB = b;
    }
  }

  printf("Max bias %f - (%3d : %3d,%3d)\n",maxBias,maxK,maxA,maxB);

  // Bit independence is harder to pass than avalanche, so we're a bit more lax here.

  bool result = (maxBias < 0.05);

  return result;
}

//-----------------------------------------------------------------------------
// BIC test variant - store all intermediate data in a table, draw diagram
// afterwards (much faster)

template< typename keytype, typename hashtype >
bool BicTest3 ( pfHash hash, const int reps, bool verbose = true )
{
  const int keybytes = sizeof(keytype);
  const int keybits = keybytes * 8;
  const int hashbytes = sizeof(hashtype);
  const int hashbits = hashbytes * 8;
  const int pagesize = hashbits*hashbits*4;

  Rand r(11938);

  double maxBias = 0;
  int maxK = 0;
  int maxA = 0;
  int maxB = 0;

  keytype key;
  hashtype h1,h2;

  std::vector<int> bins(keybits*pagesize,0);

  for(int keybit = 0; keybit < keybits; keybit++)
  {
    if(keybit % (keybits/10) == 0) printf(".");

    int * page = &bins[keybit*pagesize];

    for(int irep = 0; irep < reps; irep++)
    {
      r.rand_p(&key,keybytes);
      hash(&key,keybytes,0,&h1);
      flipbit(key,keybit);
      hash(&key,keybytes,0,&h2);

      hashtype d = h1 ^ h2;

      for(int out1 = 0; out1 < hashbits-1; out1++)
      for(int out2 = out1+1; out2 < hashbits; out2++)
      {
        int * b = &page[(out1*hashbits+out2)*4];

        uint32_t x = getbit(d,out1) | (getbit(d,out2) << 1);

        b[x]++;
      }
    }
  }

  printf("\n");

  for(int out1 = 0; out1 < hashbits-1; out1++)
  {
    for(int out2 = out1+1; out2 < hashbits; out2++)
    {
      if(verbose) printf("(%3d,%3d) - ",out1,out2);

      for(int keybit = 0; keybit < keybits; keybit++)
      {
        int * page = &bins[keybit*pagesize];
        int * bins = &page[(out1*hashbits+out2)*4];

        double bias = 0;

        for(int b = 0; b < 4; b++)
        {
          double b2 = double(bins[b]) / double(reps / 2);
          b2 = fabs(b2 * 2 - 1);

          if(b2 > bias) bias = b2;
        }

        if(bias > maxBias)
        {
          maxBias = bias;
          maxK = keybit;
          maxA = out1;
          maxB = out2;
        }

        if(verbose) 
        {
          if     (bias < 0.01) printf(".");
          else if(bias < 0.05) printf("o");
          else if(bias < 0.33) printf("O");
          else                 printf("X");
        }
      }

      // Finished keybit

      if(verbose) printf("\n");
    }

    if(verbose)
    {
      for(int i = 0; i < keybits+12; i++) printf("-");
      printf("\n");
    }
  }

  printf("Max bias %f - (%3d : %3d,%3d)\n",maxBias,maxK,maxA,maxB);
  return true;
}


//-----------------------------------------------------------------------------
// BIC test variant - iterate over output bits, then key bits. No temp storage,
// but slooooow

template< typename keytype, typename hashtype >
void BicTest2 ( pfHash hash, const int reps, bool verbose = true )
{
  const int keybytes = sizeof(keytype);
  const int keybits = keybytes * 8;
  const int hashbytes = sizeof(hashtype);
  const int hashbits = hashbytes * 8;

  Rand r(11938);

  double maxBias = 0;
  int maxK = 0;
  int maxA = 0;
  int maxB = 0;

  keytype key;
  hashtype h1,h2;

  for(int out1 = 0; out1 < hashbits-1; out1++)
  for(int out2 = out1+1; out2 < hashbits; out2++)
  {
    if(verbose) printf("(%3d,%3d) - ",out1,out2);

    for(int keybit = 0; keybit < keybits; keybit++)
    {
      int bins[4] = { 0, 0, 0, 0 };

      for(int irep = 0; irep < reps; irep++)
      {
        r.rand_p(&key,keybytes);
        hash(&key,keybytes,0,&h1);
        flipbit(key,keybit);
        hash(&key,keybytes,0,&h2);

        hashtype d = h1 ^ h2;

        uint32_t b = getbit(d,out1) | (getbit(d,out2) << 1);

        bins[b]++;
      }

      double bias = 0;

      for(int b = 0; b < 4; b++)
      {
        double b2 = double(bins[b]) / double(reps / 2);
        b2 = fabs(b2 * 2 - 1);

        if(b2 > bias) bias = b2;
      }

      if(bias > maxBias)
      {
        maxBias = bias;
        maxK = keybit;
        maxA = out1;
        maxB = out2;
      }

      if(verbose) 
      {
        if     (bias < 0.05) printf(".");
        else if(bias < 0.10) printf("o");
        else if(bias < 0.50) printf("O");
        else                 printf("X");
      }
    }

    // Finished keybit

    if(verbose) printf("\n");
  }

  printf("Max bias %f - (%3d : %3d,%3d)\n",maxBias,maxK,maxA,maxB);
}

//-----------------------------------------------------------------------------
/* vim: set sts=2 sw=2 et: */
