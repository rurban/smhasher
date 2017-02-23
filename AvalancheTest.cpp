#include "AvalancheTest.h"

//-----------------------------------------------------------------------------
// Print out a diagram in a hacked up base-51 representation.
// We are printing out counts of bit changes, where we expect to see the bits
// change half the time. This lends itself naturally to a base-51 representation
// which means we can use one character per bit/count.
// The 'scale' parameter controls how magnified the data should be. So setting it
// to 1 shows 2% per digit, with the absolute worst being "#". Setting it to 2
// shows roughly 1% per digit with anything higher than 0.5 being shown as "#",
// setting to 10 shows 0.2% per character, with anything higher than 0.1 being
// shown as "#", etc.
// A good hash function should show all "." for scale 2 at least.

const char * const symbols = ".1234567890ABCDEFGHIJKLMNOPQRSTUVWXYZ[]{}()<>*&%$@#";
void PrintAvalancheDiagram ( av_statst *stats, int mode, int scale, double confidence )
{
  int rows = (stats->seedbits + stats->keybits);
  /*                                1         2         3         4         5    */
  /*                      0        90        90        90        90        90    */
  if(!scale) scale = 1;
  double expect = double(stats->reps) / 4;
  int failed = 0;
  double *cursor= mode ? stats->gtests : stats->pcts;

  printf("%12s |0         1         2         3         4         5|\n","");
  printf("%12s |012345678901234567890123456789012345678901234567890|\n","");
  printf("%12s +---------------------------------------------------+\n","");
  printf("%-12s |%-51s|\n","Scale:",symbols);
  printf("%12s +---------------------------------------------------+\n","");
  if (mode) {
    printf("%12s |%-51s|\n","",
               "scaled p-value above confidence level (zero is ok)");
  } else {
    printf("%12s |%-51s|\n","",
               "pct diff from 50%: abs(0.5-(changed/reps)) * 100");
  }
  int words;
  int width;
  if (stats->hashbits == 32) {
    printf("%12s +--------------------------------+\n","");
    printf("%12s |0         1         2         3 |\n","");
    printf("%12s |01234567890123456789012345678901|\n","");
    printf("%12s +--------------------------------+\n","");
    width= 32;
  } else {
    printf("%12s +----------------------------------------------------------------+\n","");
    printf("%12s |0         1         2         3         4         5         6   |\n","");
    printf("%12s |0123456789012345678901234567890123456789012345678901234567890123|\n","");
    printf("%12s +----------------------------------------------------------------+\n","");
    width = 64;
  }
  words = stats->hashbits / width;

  char buf[words*(width+1)*rows];
  char *outcursor=buf;
  char *outend= buf + (words*(width+1)*rows);
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
          digit = (int)floor( val * (50.0 * scale));   /*interval: [0, 50] */
        } else {
          double prob_scale= 50.0 / (1.0 - confidence);
          digit = (int)floor(val * prob_scale);
          digit -= confidence * prob_scale;
        }
        if (digit < 0) { digit = 0; }
        else if (digit >= 50) { digit = 50; }
        if (digit) failed ++;
        *outcursor++ = symbols[ digit ];
      }
      *outcursor++ = 0;
    }
  }
  char *lastrow[words];
  outcursor= buf;
  for (int i= 0; i < rows; i++) {
    for (int w= 0; w < words; w++, outcursor += (width+1)) {
      if (i && outcursor < lastrow[w])
        continue;
      char *scancursor= outcursor + ((width+1) * words);
      int count = 1;
      int nextrow= i+1;
      while (
          scancursor < outend
          && nextrow != stats->seedbits
          && strcmp(outcursor,scancursor) == 0)
      {
        count++;
        scancursor += ((width+1) * words);
        nextrow++;
      }
      lastrow[w]= scancursor;
      printf("%-4s  %4d.%d |%s|", i < stats->seedbits ? "seed" : "key",
          i < stats->seedbits ? i : i - stats->seedbits, w, outcursor);
      if (count>1) {
        printf(" x %d\n", count);
      } else{
        printf("\n");
      }
    }
  }

  printf("%12s +%.*s+\n","",
        stats->hashbits,
        "-------------------------------------------------------------------"
  );
  if (mode) {
      printf( "%d of %d bits failed (%.2f%%) failed at %.6f confidence\n",
        failed,
        stats->num_bits,
        100 * ( failed / double(stats->num_bits) ),
        100 * confidence
      );
  }
  printf("\n");
}

//----------------------------------------------------------------------------
#define GTEST_PROB(bins,gval) \
  ( 1.0 - gsl_sf_gamma_inc_Q( ( double(bins) - 1.0) / 2.0, (gval) ) )

double calcBiasStats ( std::vector<int> & counts, int reps, av_statst *stats, double confidence )
{
  double worst = 0.0;
  double dreps= double(reps);
  double expected= dreps / 2.0;

  stats->err= 0.0;
  stats->gtest_prob= 0.0;
  stats->worst_pct= 0.0;
  std::vector<int> col_bins(stats->hashbits * 2,0);
  std::vector<int> row_bins(stats->num_rows * 2,0);

  for(int i = 0; i < (int)counts.size(); i += 4 )
  {
    double changed= double(counts[i + 1] + counts[i + 2]);
    double stayed=  double(counts[i + 0] + counts[i + 3]);
    double ratio = changed / dreps;
    double diff = 0.5 - ratio;
    double d_pct = fabs(diff * 2);
    stats->err += (diff * diff);

    int row= (i / 4) / stats->hashbits;
    int col= (i / 4) % stats->hashbits;
    col_bins[(col * 2) + 0] += stayed;
    col_bins[(col * 2) + 1] += changed;
    row_bins[(row * 2) + 0] += stayed;
    row_bins[(row * 2) + 1] += changed;

    double this_gtest = 0.0;
    if ( changed ) {
        double adj= changed * log( changed / expected );
        this_gtest += adj;
        stats->gtest_prob += adj;
    }
    if ( stayed ) {
        double adj= stayed * log( stayed / expected );
        this_gtest += adj;
        stats->gtest_prob += adj;
    }

    stats->gtests[i/4] = GTEST_PROB( 2.0, this_gtest );
    stats->pcts[i/4] = d_pct;

    if(d_pct > worst)
    {
        stats->worst_pct= d_pct;
        worst = d_pct;
        stats->worst_x = col;
        stats->worst_y = row;
    }
  }
  for (int i= 0; i < stats->hashbits * 2; i+=2) {
    double gtest= 0.0;
    double l= double(col_bins[i + 0]);
    double r= double(col_bins[i + 1]);
    double m= (l + r) / 2;
    if (l) gtest += l * log( l / m );
    if (r) gtest += r * log( r / m );

    gtest = GTEST_PROB( 2.0, gtest );
    stats->col_gtests[i/2] = gtest;
    if (gtest >= confidence)
      stats->col_errors++;
  }
  for (int i= 0; i < stats->num_rows * 2; i+=2) {
    double gtest= 0.0;
    double l= double(row_bins[i + 0]);
    double r= double(row_bins[i + 1]);
    double m= (l + r) / 2;
    if (l) gtest += l * log( l / m );
    if (r) gtest += r * log( r / m );

    gtest = GTEST_PROB( 2.0, gtest );
    stats->row_gtests[i/2] = gtest;
    if (gtest >= confidence)
      stats->row_errors++;
  }

  stats->err_scaled = stats->err / ( double(stats->num_bits) / 1024.0 );
  stats->expected_err_scaled = (0.00256 / (stats->reps / 100000.0));
  stats->err_scaled_ratio= stats->err_scaled / stats->expected_err_scaled;
  stats->gtest_prob = GTEST_PROB( stats->num_bins / 2, stats->gtest_prob );
  return worst;
}

//-----------------------------------------------------------------------------
/* vim: set sts=2 sw=2 et: */
