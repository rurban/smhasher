#include "AvalancheTest.h"

//-----------------------------------------------------------------------------

void PrintAvalancheDiagram ( int x, int y, int reps, double scale, int * bins )
{
  const char * symbols = ".123456789X";

  for(int i = 0; i < y; i++)
  {
    printf("[");
    for(int j = 0; j < x; j++)
    {
      int k = (y - i) -1;

      int bin = bins[k + (j*y)];

      double b = double(bin) / double(reps);
      b = fabs(b*2 - 1);

      b *= scale;

      int s = (int)floor(b*10);

      if(s > 10) s = 10;
      if(s < 0) s = 0;

      printf("%c",symbols[s]);
    }

    printf("]\n");
    fflush(NULL);
  }
}

//----------------------------------------------------------------------------

double maxBias ( const std::vector<int> & counts, int reps, double & average )
{
  double worst = 0;
  double sum = 0;

  for(int i = 0; i < (int)counts.size(); i++)
  {
    double c = double(counts[i]) / double(reps);

    double d = fabs(c * 2 - 1);
    sum += d;

    if(d > worst)
    {
      worst = d;
    }
  }
  average = counts.empty() ? 0 : sum / counts.size();


  return worst;
}

//-----------------------------------------------------------------------------
