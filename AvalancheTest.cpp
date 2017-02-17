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

void PrintAvalancheDiagram ( av_statst *stats, int mode, int scale, double confidence )
{
  int rows = (stats->seedbits + stats->keybits);
  /*                                1         2         3         4         5    */
  /*                      0        90        90        90        90        90    */
  const char * symbols = ".1234567890ABCDEFGHIJKLMNOPQRSTUVWXYZ[]{}()<>*&%$@#";
  if(!scale) scale = 1;
  double expect = double(stats->reps) / 4;
  int failed = 0;
  double *cursor= mode == 0 ? stats->pcts :
                  mode == 1 ? stats->chisqs :
                  stats->gtests;

  if (stats->hashbits == 32) {
    printf("%12s |0         1         2         3 |\n","");
    printf("%12s |01234567890123456789012345678901|\n","");
    printf("%12s +--------------------------------+\n","");
  } else {
    printf("%12s |0         1         2         3         4         5         6   |\n","");
    printf("%12s |0123456789012345678901234567890123456789012345678901234567890123|\n","");
    printf("%12s +----------------------------------------------------------------+\n","");
  }

  for(int i = 0; i < rows; i++)
  {
    if (i == stats->seedbits)
        printf("%12s +%.*s+\n","",
                stats->hashbits,
                "-------------------------------------------------------------------"
        );
    printf("%-4s bit %3d |", i < stats->seedbits ? "seed" : "key",
            i < stats->seedbits ? i : i - stats->seedbits);
    for( int j = 0; j < stats->hashbits; j++, cursor++ )
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
      printf("%c", symbols[ digit ]);
    }
    printf("|\n");
  }
  printf("%12s +%.*s+\n","",
        stats->hashbits,
        "-------------------------------------------------------------------"
  );
  if (mode) {
      double expect = double(stats->num_bits) * (1.0 - confidence);
      printf("At %.2f confidence, %d failed, %.2f expected (%.2f%%)\n",
              confidence, failed, expect, 100 * fabs(failed/expect));
  }
  printf("\n");
}

//----------------------------------------------------------------------------

double calcBiasStats ( std::vector<int> & counts, int reps, av_statst *stats )
{
  double worst = 0.0;
  double dreps= double(reps);
  double expect_changed= dreps / 2.0;
  double expect= dreps / 4.0;

  stats->err= 0.0;
  stats->chisq_prob= 0.0;
  stats->gtest_prob= 0.0;
  stats->worst_pct= 0.0;

  for(int i = 0; i < (int)counts.size(); i += 4 )
  {
    double changed= double(counts[i + 1] + counts[i + 2]);

    double ratio = changed / dreps;
    double diff = 0.5 - ratio;
    double d_pct = fabs(diff * 2);
    stats->err += (diff * diff);

    double this_chisq = 0.0;
    double this_gtest = 0.0;

    for (int j = 0; j < 4; j++) {
        this_chisq += pow(counts[ i + j ] - expect, 2) / expect;
        stats->chisq_prob += pow(counts[ i + j ] - expect, 2) / expect;
        if ( counts[ i + j ] > 0 ) {
            this_gtest += counts[ i + j ] * log( counts[ i + j ] / expect );
            stats->gtest_prob += counts[ i + j ] * log( counts[ i + j ] / expect );
        }
    }
    stats->chisqs[i/4] = (1.0 - gsl_sf_gamma_inc_Q( 3.0 / 2.0, this_chisq / 2.0));
    stats->gtests[i/4] = 1.0 - gsl_sf_gamma_inc_Q( 3.0 / 2.0, this_gtest);
    stats->pcts[i/4] = d_pct;
      
    if(d_pct > worst)
    {
        stats->worst_pct= d_pct;
        stats->worst_i= i/4;
        worst = d_pct;
    }
  }
  stats->bins_dof = double(stats->num_bins) - 1.0;
  stats->err_scaled = stats->err / ( double(stats->num_bits) / 1024.0 );
  stats->expected_err_scaled = (0.00256 / (stats->reps / 100000.0));
  stats->err_scaled_ratio= stats->err_scaled / stats->expected_err_scaled;
  stats->chisq_prob = 1.0 - gsl_sf_gamma_inc_Q(stats->bins_dof/2.0, stats->chisq_prob/2.0);
  stats->gtest_prob = 1.0 - gsl_sf_gamma_inc_Q(stats->bins_dof/2.0, stats->gtest_prob);
  stats->worst_x = stats->worst_i % stats->hashbits;
  stats->worst_y = stats->worst_i / stats->hashbits;
  return worst;
}

//-----------------------------------------------------------------------------
/* vim: set sts=2 sw=2 et: */
