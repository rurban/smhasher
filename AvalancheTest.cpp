#include "AvalancheTest.h"

// This code implements the avalanche test. For this test we do the following:
//
//  For a given length of key (including the empty key)
//    For T iterations:
//      choose a random seed and random key
//      computer hash as h1
//      for each bit K in seed and key
//        flip bit
//        compute hash as h2
//        flip bit back
//        for each bit H in h2
//          if the bit is different from the corresponding bit in h1
//            increment changed[K][H]
//
// We then can compute various goodness of fit statistics from the ratio of
// the changed count and the number of iterations, with an ideal function having
// each bit combination changing %50 of the time.
//
//  gtest-cell: goodness of fit estimate using the g-test for a single cell
//  gtest-all: goodness of fit estimate using the g-test for all cells in the matrix.
//  error: sum of the square of the error
//  error-ratio: percentage of the expected error for this number of iterations
//  column/row: goodness of fit estimate using the gtest for each input bit (row),
//  and for each output bit (column).
//  worst-bit: the input/output bit that is the furthest from the expected %50
//
// NB: we actually do a 3 dimensional count matrix, with four cells per input/output
// bit combination, and count the pairs of input bit/output bit by value. We currently
// do not use this potential fully in testing and notationally reduce the 4 cell
// representation (00,01,10,11) to a 2 cell representation (unchanged,changed)
// during post-processing. Once I sort out the math I plan to use this data for
// additional tests.

//-----------------------------------------------------------------------------
// Print out a diagram in a hacked up base-101 representation.
// We are printing out counts of bit changes, where we expect to see the bits
// change half the time and we want to see percentage difference, so
// having a 100 unit scale - actually 101, as we have to deal with the closed
// interval [0,1] and not the half-open interval [0,1).
// The 'scale' parameter controls how magnified the data should be.
// A good hash function should show all "." for scale 1 at least.
const uint8_t digits1[101]= { 46, 49, 50, 51, 52, 53, 54, 55, 56, 57, 48, 97, 98,
  99, 100, 101, 102, 103, 104, 105, 106, 107, 108, 109, 110, 111, 112, 113,
  114, 115, 116, 117, 118, 119, 120, 121, 122, 195, 195, 195, 195, 195,
  195, 195, 195, 195, 195, 195, 195, 195, 195, 195, 65, 66, 67, 68, 69, 70,
  71, 72, 73, 74, 75, 76, 77, 78, 79, 80, 81, 82, 83, 84, 85, 86, 87, 88,
  89, 90, 195, 195, 195, 195, 195, 195, 195, 195, 195, 195, 195, 195, 195,
  195, 194, 42, 194, 194, 194, 38, 37, 64, 35 };
const uint8_t digits2[101]= { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 163, 164,
  165, 170, 171, 174, 175, 176, 177, 180, 181, 182, 187, 188, 191, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 130,
  131, 132, 133, 138, 139, 145, 148, 149, 150, 155, 156, 157, 184, 164, 0,
  169, 174, 182, 0, 0, 0, 0 };

void PrintAvalancheDiagram ( av_statst *stats, int mode, int scale, double confidence )
{
  int rows = (stats->seedbits + stats->keybits);
  /*                                1         2         3         4         5    */
  /*                      0        90        90        90        90        90    */
  if(!scale) scale = 1;
  double expect = double(stats->reps) / 4;
  int failed = 0;
  double *cursor= mode ? stats->gtests : stats->pcts;

  printf("# %12s +---------------------------------------------------+\n","");
  printf("# %12s |012345678901234567890123456789012345678901234567890|\n","");
  printf("# %12s +---------------------------------------------------+\n","");
  printf("# %-12s |","Scale:");
  for (int i= 0; i <=50; i++) {
    printf("%c",digits1[i]);
    if(digits2[i])
    printf("%c",digits2[i]);
  }
  printf("|\n");
  printf("# %-12s |","");
  for (int i= 50; i <=100; i++) {
    printf("%c",digits1[i]);
    if(digits2[i])
    printf("%c",digits2[i]);
  }
  printf("|\n");
  printf("# %12s +---------------------------------------------------+\n","");
  if (mode) {
    printf("# %12s |%-51s|\n","",
               "scaled p-value above confidence level (zero is ok)");
  } else {
    printf("# %12s |%-48s%3d|\n","",
               "pct diff from 50%: abs((0.5-(changed/reps))*2) *", scale * 100);
  }
  int words;
  int width;
  if (stats->hashbits == 32) {
    printf("# %12s +--------------------------------+\n","");
    printf("# %12s |0         1         2         3 |\n","");
    printf("# %12s |01234567890123456789012345678901|\n","");
    printf("# %12s +--------------------------------+\n","");
    width= 32;
  } else {
    printf("# %12s +----------------------------------------------------------------+\n","");
    printf("# %12s |0         1         2         3         4         5         6   |\n","");
    printf("# %12s |0123456789012345678901234567890123456789012345678901234567890123|\n","");
    printf("# %12s +----------------------------------------------------------------+\n","");
    width = 64;
  }
  words = stats->hashbits / width;

  unsigned char buf[words*(width+1)*rows];
  unsigned char *outcursor=buf;
  unsigned char *outend= buf + (words*(width+1)*rows);
  int dupe= 0;
  int i;
  int j;
  int w;
  for( i = 0; i < rows; i++)
  {
    for( w = 0; w < words; w++)
    {
      for( j = 0; j < width; j++, cursor++ )
      {
        int digit;
        double val = *cursor;
        if (mode == 0) {
          digit = (int)floor( 0.5 + val * (100.0 * double(scale)));   /*interval: [0, 50] */
        } else {
          if (val < confidence) {
            digit= 0;
          } else {
            double dscale = 100.0 / (1.0 - confidence);
            digit= uint8_t((val - confidence) * dscale);
          }
        }
        if (digit < 0) { digit = 0; }
        else if (digit > 100) { digit = 100; }
        if (digit)
          failed ++;

        *outcursor = uint8_t(1 + digit);
        outcursor++;
      }
      *outcursor = 0;
      outcursor++;
    }
  }
  unsigned char *lastrow[words];
  outcursor= buf;
  for (int i= 0; i < rows; i++) {
    for (int w= 0; w < words; w++, outcursor += (width+1)) {
      if (i && outcursor < lastrow[w])
        continue;
      unsigned char *scancursor= outcursor + ((width+1) * words);
      int count = 1;
      int nextrow= i+1;
      while (
          scancursor < outend
          && nextrow != stats->seedbits
          && strcmp((char *)outcursor,(char *)scancursor) == 0)
      {
        count++;
        scancursor += ((width+1) * words);
        nextrow++;
      }
      lastrow[w]= scancursor;
      printf("# %-4s  %4d.%d |", i < stats->seedbits ? "seed" : "key",
          i < stats->seedbits ? i : i - stats->seedbits, w);
      for (int i= 0; i < width; i++) {
        int digit = outcursor[i] - 1;
        printf("%c", digits1[digit]);
        if (digits2[digit])
          printf("%c", digits2[digit]);
      }

      if (count>1) {
        printf("| x %d\n", count);
      } else{
        printf("|\n");
      }
    }
  }

  printf("# %12s +%.*s+\n","",
        stats->hashbits,
        "-------------------------------------------------------------------"
  );
  if (mode) {
      printf( "# %d of %d bits failed (%.2f%%) failed at %.6f confidence\n",
        failed,
        stats->num_bits,
        100 * ( failed / double(stats->num_bits) ),
        100 * confidence
      );
  }
}

//----------------------------------------------------------------------------

void
calcBiasStats ( std::vector<int> & counts, int reps, av_statst *stats, double confidence )
{
  double dreps= double(reps);
  double expected= dreps / 2.0;

  stats->err= 0.0;
  stats->gtest_prob= 0.0;
  stats->worst_pct= 0.0;

  for(int i = 0; i < (int)counts.size(); i += 4 )
  {
    double diff = double(counts[i + 1] + counts[i + 2]);
    double same = double(counts[i + 0] + counts[i + 3]);
    double ratio = diff / double(dreps);
    double delta = 0.5 - ratio;
    stats->err += pow(delta, 2.0);
    stats->pcts[i/4] = fabs(delta * 2);

    if ( stats->worst_pct < stats->pcts[i/4] )
      stats->worst_pct = stats->pcts[i/4];

    int row= (i / 4) / stats->hashbits;
    int col= (i / 4) % stats->hashbits;

    stats->col_bins[(col * 2) + 0] += diff;
    stats->col_bins[(col * 2) + 1] += same;
    stats->row_bins[(row * 2) + 0] += diff;
    stats->row_bins[(row * 2) + 1] += same;

    double this_gtest = 0.0;
    if ( diff ) {
        double adj= 0.0;
        GTEST_ADD(adj, diff, expected );
        this_gtest += adj;
        stats->gtest_prob += adj;
    }
    if ( same ) {
        double adj= 0.0;
        GTEST_ADD(adj, same, expected );
        this_gtest += adj;
        stats->gtest_prob += adj;
    }
    stats->gtests[i/4] = GTEST_PROB( 2.0, this_gtest );
  }

  for (int i= 0; i < stats->hashbits; i++) {
    double gtest= 0.0;
    double diff= stats->col_bins[(i * 2) + 0];
    double same= stats->col_bins[(i * 2) + 1];
    double avg= (same + diff) / 2;
    GTEST_ADD(gtest,same,avg);
    GTEST_ADD(gtest,diff,avg);
    gtest = GTEST_PROB( 2.0, gtest );

    stats->col_gtests[i] = gtest;
    if (gtest >= confidence) {
      stats->col_errors++;
    }
  }
  for (int i= 0; i < stats->num_rows; i++) {
    double gtest= 0.0;
    double diff= stats->row_bins[(i * 2) + 0];
    double same= stats->row_bins[(i * 2) + 1];
    double avg= (same + diff) / 2;
    GTEST_ADD(gtest,same,avg);
    GTEST_ADD(gtest,diff,avg);
    gtest = GTEST_PROB( 2.0, gtest );

    stats->row_gtests[i] = gtest;
    if (gtest >= confidence) {
      //printf("row-error %d: %.0f/%.0f => %.20f\n", i/2, l, r, gtest);
      stats->row_errors++;
    }
  }

  stats->err_scaled = stats->err / ( double(stats->num_bits) / 1024.0 );
  stats->expected_err_scaled = (0.00256 / (stats->reps / 100000.0));
  stats->err_scaled_ratio= stats->err_scaled / stats->expected_err_scaled;
  stats->gtest_prob = GTEST_PROB( stats->num_bins / 2, stats->gtest_prob );
}

//-----------------------------------------------------------------------------
/* vim: set sts=2 sw=2 et: */
