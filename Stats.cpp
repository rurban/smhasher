#include "Stats.h"

//-----------------------------------------------------------------------------

double chooseK ( int n, int k )
{
  if(k > (n - k)) k = n - k;

  double c = 1;

  for(int i = 0; i < k; i++)
  {
    double t = double(n-i) / double(i+1);

    c *= t;
  }

    return c;
}

double chooseUpToK ( int n, int k )
{
  double c = 0;

  for(int i = 1; i <= k; i++)
  {
    c += chooseK(n,i);
  }

  return c;
}
//-----------------------------------------------------------------------------
// Calculate a modified version of the RMS ratio for a distribution.
//
// This function is the "old" calcScore, which was not well explained.
//
// We know that the equation:
//
// r = ( sum * sum ) / sum_squares / count
//
// results in r=1 when the distribution is "perfect", and produces
// slightly smaller
//
// And the further away from a perfect distribution the smaller the
// ratio.
//
// This function modifies that to
//
// r = ( sum * sum - 1 ) / (sum_squares - sum) / count
//
// which means a perfect mapping gets a score of above 1.
//
// Honestly I don't understand this really. If I compare the g-test probability
// on something this check would say is "bad", the g-test may be conclusive
// and vice versa. I have a lot more trust in the g-test than in this code.

double calcScore_old ( const int * vals, const int count, const int sum)
{
  double sum_squares = 0.0;
  for(int i = 0; i < count; i++)
  {
    sum_squares += pow(double(vals[i]),2.0);
  }
  double square_sum= pow(double(sum), 2.0);
  return 1.0 - ( ( square_sum - 1) / (sum_squares - double(sum)) / count );
}
//-----------------------------------------------------------------------------
// Calculate the likelihood that a given distribution is random, and if it is
// return the RMS ratio for it.
double calcScore ( const int * vals, const int icount, const int isum, double confidence )
{
  double sum= double(isum);
  double count= double(icount);
  double expected = sum / count;
  double gtest = 0.0;
  double quality_score = 0.0;
  double old_score = 0.0;
  double stddev = 0.0;
  int min = vals[0];
  int max = vals[0];
  std::vector<int> count_counts(expected * 4, 0);

  for(int i = 0; i < count; i++)
  {
    double iv= vals[i];
    double v= double(iv);
    if (iv >= count_counts.size())
      count_counts.resize(iv * 2,0);
    count_counts[iv]++;
    if (min > iv) min = iv;
    if (max < iv) max = iv;

    GTEST_ADD(gtest, v, expected);
    quality_score += ( v * ( v + 1.0 ) ) / 2.0;
    old_score += pow( v, 2.0 );
    stddev += pow( v - expected , 2.0 );
  }
  gtest = GTEST_PROB(count, gtest);

  stddev = sqrt( stddev / count );
  old_score = 1.0 - ( ( pow( sum, 2.0 ) - 1.0 ) / ( old_score - sum ) / count );
  quality_score = quality_score / ( ( sum / ( 2.0 * count ) ) *
                                    ( sum + ( 2.0 * count ) - 1.0 ) );
  double score = fabs( 1.0 - quality_score );


  if (gtest >= confidence && score > 0.01) {
    if (g_verbose) {
      printf(
          "Failed gtest with %.6f prob\n"
          "  old_score = %.6f%% score = %.6f (%.6f)\n"
          "  bins=%.0f sum=%.0f mean=%.0f stddev=%.6f\n",
          gtest * 100, old_score * 100, score, quality_score,
          count, sum, expected, stddev);
      printf("  Bucket Count Frequencies:\n");
      for ( int i = min ; i <= max ; i++ )
        printf("    %3d = %10d (%6.2f)\n",
            i, count_counts[i], count_counts[i] / sum * 100.0 );
    }
    return score;
  } else {
    if (g_verbose > 1) {
      printf(
          "Passed gtest with %.6f prob\n"
          "  old_score = %.6f%% score = %.6f (%.6f)\n"
          "  bins=%.0f sum=%.0f mean=%.0f stddev=%.6f\n",
          gtest * 100, old_score * 100, score, quality_score,
          count, sum, expected, stddev);
    }
    return 0;
  }
}
//-----------------------------------------------------------------------------
// Calculate the likelihood that a given distribution is random.
//

double calcGTestProbability ( const int * vals, const int count, const int sum )
{
  double expected = double(sum) / double(count);
  double gtest = 0.0;

  for(int i = 0; i < count; i++)
  {
    GTEST_ADD(gtest,vals[i],expected);
  }
  return GTEST_PROB(count, gtest);
}

double calcGTestProbability ( const int * vals, const int count)
{
  int sum = 0;
  for (int i = 0; i < count; i++) {
    sum += vals[i];
  }
  return calcGTestProbability(vals, count, sum);
}


//----------------------------------------------------------------------------

void plot ( double n )
{
  double n2 = n * 1;

  if(n2 < 0) n2 = 0;

  n2 *= 100;

  if(n2 > 64) n2 = 64;

  int n3 = (int)n2;

  if(n3 == 0)
    printf(".");
  else
  {
    char x = '0' + char(n3);

    if(x > '9') x = 'X';

    printf("%c",x);
  }
}

//-----------------------------------------------------------------------------
/* vim: set sts=2 sw=2 et: */
