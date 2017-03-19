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
#include "HashFunc.h"
#include "Stats.h"

#include <vector>
#include <stdio.h>
#include <math.h>

// Avalanche fails if a bit is biased by more than 1%

typedef struct av_stats {
  int     *bins;
  double  *gtests;
  double  *pcts;
  double  *col_gtests;
  double  *row_gtests;
  double  *col_bins;
  double  *row_bins;
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
  int     col_errors;
  int     row_errors;
} av_statst;

void calcBiasStats ( std::vector<int> & counts, int reps, av_statst *stats, double confidence );
void PrintAvalancheDiagram ( av_statst *stats, int mode, int scale, double confidence );

//-----------------------------------------------------------------------------

template < typename keytype, typename hashtype >
void calcBiasWithSeed ( hashfunc<hashtype> hash, std::vector<int> & counts, int reps, Rand & r )
{
  const int seedbytes = hash.seedbytes();
  const int keybytes = sizeof(keytype);
  const int hashbytes = sizeof(hashtype);

  const int seedbits = hash.seedbits();
  const int keybits = keybytes * 8;
  const int hashbits = hashbytes * 8;
  std::vector<uint8_t> seed(seedbytes);
  keytype K;
  hashtype A,B;

  for(int irep = 0; irep < reps; irep++)
  {
    if(irep % (reps/10) == 0) printf(".");

    r.rand_p(&K,keybytes);
    r.rand_p(&seed[0],seedbytes);

    hash(&K,keybytes,&seed[0],&A);

    int * cursor = &counts[0];

    for(int iBit = 0; iBit < seedbits; iBit++)
    {
      flipbit(&seed[0],seedbytes,iBit);
      hash.seed_state(&seed[0]);
      hash(&K,keybytes,&B);
      flipbit(&seed[0],seedbytes,iBit);

      for(int iOut = 0; iOut < hashbits; iOut++)
      {
        int bitA = getbit(&A,hashbytes,iOut) ? 2 : 0;
        int bitB = getbit(&B,hashbytes,iOut) ? 1 : 0;

        cursor[ bitA + bitB ]++;
        cursor += 4;
      }
    }

    hash.seed_state(&seed[0]);
    for(int iBit = 0; iBit < keybits; iBit++)
    {
      flipbit(&K,keybytes,iBit);
      hash(&K,keybytes,&B);
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
/* no bit may be more than 1% from the expected 50% changed rate */

template < typename keytype, typename hashtype >
bool AvalancheTest (
  hashfunc<hashtype> hash,
  const int reps,
  Rand &r,
  double confidence,
  double max_pct_error,
  double max_error_ratio
) {
  int seedbits = hash.seedbits();
  int keybits = sizeof(keytype) * 8;
  int hashbits = sizeof(hashtype) * 8;
  int num_rows = keybits + seedbits;
  int num_bits = ( keybits + seedbits ) * hashbits;
  int num_bins = num_bits * 4;
  char name[1024];
  snprintf(name,1024,"Strict Avalanche Criteria - %d bit/%d byte keys # %s",
      keybits, int(sizeof(keytype)), hash.name());

  std::vector<int> bins(num_bins, 0);
  std::vector<double> gtests(num_bits, 0);
  std::vector<double> pcts(num_bits, 0);
  std::vector<double> col_gtest(hashbits, 0);
  std::vector<double> row_gtest(num_rows, 0);
  std::vector<double> col_bins(hashbits * 2, 0);
  std::vector<double> row_bins(num_rows * 2, 0);
  
  av_statst stats = {
    &bins[0],
    &gtests[0],
    &pcts[0],
    &col_gtest[0],
    &row_gtest[0],
    &col_bins[0],
    &row_bins[0],
    reps,
    seedbits,
    keybits,
    hashbits,
    num_bits,
    num_bins,
    num_rows,
  };

  printf("# Testing %3d-bit keys", keybits);

  //----------

  calcBiasWithSeed<keytype,hashtype>(hash, bins, reps, r);
  calcBiasStats( bins, reps, &stats, confidence );
  
  //----------


  bool result = true;
  if(
    stats.gtest_prob       >= confidence      ||
    stats.worst_pct        >= max_pct_error   ||
    stats.err_scaled_ratio >= max_error_ratio ||
    stats.col_errors                          ||
    stats.row_errors                          ||
    0
  ) {
    printf(" not ok! ");
    result = false;
  } else {
    printf(" ok.     ");
  }


  printf("# worst-bit: %7.3f%% error-ratio: %e\n",
      stats.worst_pct * 100.0, stats.err_scaled_ratio);
  if (result)
    return ok(result,name);


  PrintAvalancheDiagram(&stats, 0, 1, confidence);
  PrintAvalancheDiagram(&stats, 1, 1, confidence);

  printf(
    "#     g-test: %7.6f%%\n"
    "#     sum-error-square: %.8f\n",
    stats.gtest_prob * 100.0, stats.err_scaled );

  if (stats.row_errors)
    printf( "#     key/seed errors: %d/%d\n",
      stats.row_errors, stats.num_rows);

  if (stats.row_errors) {
    int seed_reported = 0;
    int key_reported = 0;
    int seed_skipped = 0;
    int key_skipped = 0;
    for (int i=0; i < stats.num_rows; i++) {
      if (stats.row_gtests[i] < confidence)
        continue;
      if (i<stats.seedbits) {
        if (seed_reported < 2) {
          printf("#     - seed bit %d gtest probability not random: %.4f (%.0f/%.0f)\n",
            i, stats.row_gtests[i] * 100,
            stats.row_bins[(i * 2) + 0], stats.row_bins[(i * 2) + 1]);
          seed_reported++;
        } else {
          seed_skipped++;
        }
      } else {
        if(key_reported < 2) {
          printf("#     - key bit %d gtest probability not random: %.4f (%.0f/%.0f)\n",
            i - stats.seedbits, stats.row_gtests[i] * 100,
            stats.row_bins[(i * 2) + 0], stats.row_bins[(i * 2) + 1]);
          key_reported++;
        } else {
          key_skipped++;
        }
      }
    }
    if (seed_skipped && key_skipped)
      printf("#     - with %d more seed errors and %d more key errors not described above.\n",
          seed_skipped, key_skipped);
    else if (seed_skipped)
      printf("#     - with %d more seed errors not described above.\n", seed_skipped);
    else if (key_skipped)
      printf("#     - with %d more key errors not described above.\n", key_skipped);
  }

  if (stats.col_errors)
    printf( "#     hash bit-level errors: %d/%d\n",
      stats.col_errors, stats.hashbits );

  if (stats.col_errors) {
    int reported = 0;
    int skipped = 0;
    for (int i = 0; i < stats.hashbits; i++) {
      if (stats.col_gtests[i] < confidence)
        continue;
      if (reported < 3) {
        printf("#     - hash bit %d gtest-prob not-random: %.4f (%.0f/%.0f)\n",
          i, stats.col_gtests[i] * 100,
          col_bins[(i * 2) + 0], col_bins[(i * 2) + 1]);
        reported++;
      } else {
        skipped++;
      }
    }
    if (skipped)
      printf("#     - with %d more hash bit errors not described above.\n", skipped);
  }

  return ok(result,name);
}

//----------------------------------------------------------------------------
// Tests the Bit Independence Criteron. Stricter than Avalanche, but slow and
// not really all that useful.

template< typename keytype, typename hashtype >
void BicTest ( hashfunc<hashtype> hash, const int keybit, const int reps, double & maxBias, int & maxA, int & maxB, bool verbose )
{
  Rand r(11938);
  
  const int keybytes = sizeof(keytype);
  const int hashbytes = sizeof(hashtype);
  const int hashbits = hashbytes * 8;

  std::vector<int> bins(hashbits*hashbits*4,0);

  keytype key;
  hashtype h1,h2;

  hash.seed_state_rand(r);

  for(int irep = 0; irep < reps; irep++)
  {
    if(verbose)
    {
      if(irep % (reps/10) == 0) printf(".");
    }

    r.rand_p(&key,keybytes);
    hash(&key,keybytes,&h1);

    flipbit(key,keybit);
    hash(&key,keybytes,&h2);

    hashtype d = h1 ^ h2;

    for(int out1 = 0; out1 < hashbits; out1++)
    for(int out2 = 0; out2 < hashbits; out2++)
    {
      if(out1 == out2) continue;

      uint32_t b = getbit(d,out1) | (getbit(d,out2) << 1);

      bins[(out1 * hashbits + out2) * 4 + b]++;
    }
  }

  if(verbose) printf("\n"); // nl maybe

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

    if(verbose) printf("\n"); // nl ok
  }
}

//----------

template< typename keytype, typename hashtype >
bool BicTest ( hashfunc<hashtype> hash, const int reps )
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
bool BicTest3 ( hashfunc<hashtype> hash, const int reps, bool verbose = true )
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

  hash.seed_state_rand(r);

  std::vector<int> bins(keybits*pagesize,0);

  for(int keybit = 0; keybit < keybits; keybit++)
  {
    if(keybit % (keybits/10) == 0) printf(".");

    int * page = &bins[keybit*pagesize];

    for(int irep = 0; irep < reps; irep++)
    {
      r.rand_p(&key,keybytes);
      hash(&key,keybytes,&h1);
      flipbit(key,keybit);
      hash(&key,keybytes,&h2);

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

  printf("\n"); // nl maybe

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

      if(verbose) printf("\n"); // nl ok
    }

    if(verbose)
    {
      for(int i = 0; i < keybits+12; i++) printf("-");
      printf("\n"); // nl ok
    }
  }

  printf("# Max bias %f - (%3d : %3d,%3d)\n",maxBias,maxK,maxA,maxB);
  return true;
}


//-----------------------------------------------------------------------------
// BIC test variant - iterate over output bits, then key bits. No temp storage,
// but slooooow

template< typename keytype, typename hashtype >
void BicTest2 ( hashfunc<hashtype> hash, const int reps, bool verbose = true )
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

  hash.seed_state_rand(r);

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
        hash(&key,keybytes,&h1);
        flipbit(key,keybit);
        hash(&key,keybytes,&h2);

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

    if(verbose) printf("\n"); // nl ok
  }

  printf("Max bias %f - (%3d : %3d,%3d)\n",maxBias,maxK,maxA,maxB);
}

//-----------------------------------------------------------------------------
/* vim: set sts=2 sw=2 et: */
