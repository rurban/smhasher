#include "SpeedTest.h"


//-----------------------------------------------------------------------------
// We view our timing values as a series of random variables V that has been
// contaminated with occasional outliers due to cache misses, thread
// preemption, etcetera. To filter out the outliers, we search for the largest
// subset of V such that all its values are within three standard deviations
// of the mean.

double CalcMean ( std::vector<double> & v )
{
  double mean = 0;
  
  for(int i = 0; i < (int)v.size(); i++)
  {
    mean += v[i];
  }
  
  mean /= double(v.size());
  
  return mean;
}

double CalcMean ( std::vector<double> & v, int a, int b )
{
  double mean = 0;
  
  for(int i = a; i <= b; i++)
  {
    mean += v[i];
  }
  
  mean /= (b-a+1);
  
  return mean;
}

double CalcStdv ( std::vector<double> & v, int a, int b )
{
  double mean = CalcMean(v,a,b);

  double stdv = 0;
  
  for(int i = a; i <= b; i++)
  {
    double x = v[i] - mean;
    
    stdv += x*x;
  }
  
  stdv = sqrt(stdv / (b-a+1));
  
  return stdv;
}

// Return true if the largest value in v[0,len) is more than three
// standard deviations from the mean

bool ContainsOutlier ( std::vector<double> & v, size_t len )
{
  double mean = 0;
  
  for(size_t i = 0; i < len; i++)
  {
    mean += v[i];
  }
  
  mean /= double(len);
  
  double stdv = 0;
  
  for(size_t i = 0; i < len; i++)
  {
    double x = v[i] - mean;
    stdv += x*x;
  }
  
  stdv = sqrt(stdv / double(len));

  double cutoff = mean + stdv*3;
  
  return v[len-1] > cutoff;  
}

// Do a binary search to find the largest subset of v that does not contain
// outliers.

void FilterOutliers ( std::vector<double> & v )
{
  std::sort(v.begin(),v.end());
  
  size_t len = 0;
  
  for(size_t x = 0x40000000; x; x = x >> 1 )
  {
    if((len | x) >= v.size()) continue;
    
    if(!ContainsOutlier(v,len | x))
    {
      len |= x;
    }
  }
  
  v.resize(len);
}

// Iteratively tighten the set to find a subset that does not contain
// outliers. I'm not positive this works correctly in all cases.

void FilterOutliers2 ( std::vector<double> & v )
{
  std::sort(v.begin(),v.end());
  
  int a = 0;
  int b = (int)(v.size() - 1);
  
  for(int i = 0; i < 10; i++)
  {
    //printf("%d %d\n",a,b);
  
    double mean = CalcMean(v,a,b);
    double stdv = CalcStdv(v,a,b);
    
    double cutA = mean - stdv*3;  
    double cutB = mean + stdv*3;
    
    while((a < b) && (v[a] < cutA)) a++;
    while((b > a) && (v[b] > cutB)) b--;
  }
  
  std::vector<double> v2;
  
  v2.insert(v2.begin(),v.begin()+a,v.begin()+b+1);
  
  v.swap(v2);
}



//-----------------------------------------------------------------------------
/* vim: set sts=2 sw=2 et: */
