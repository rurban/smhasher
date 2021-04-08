#include "SpeedTest.h"
#include "Random.h"
#include "vmac.h"
#include "Hashes.h"

#include <stdio.h>   // for printf
#include <memory.h>  // for memset
#include <math.h>    // for sqrt
#include <algorithm> // for sort, min
#include <string>

#include <unordered_map>
#include <parallel_hashmap/phmap.h>
#include <functional>

typedef std::unordered_map<std::string, int,
  std::function<size_t (const std::string &key)>> std_hashmap;
typedef phmap::flat_hash_map<std::string, int,
  std::function<size_t (const std::string &key)>> fast_hashmap;

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

double CalcStdv ( std::vector<double> & v )
{
  return CalcStdv(v, 0, v.size());
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
// We really want the rdtsc() calls to bracket the function call as tightly
// as possible, but that's hard to do portably. We'll try and get as close as
// possible by marking the function as NEVER_INLINE (to keep the optimizer from
// moving it) and marking the timing variables as "volatile register".

NEVER_INLINE int64_t timehash ( pfHash hash, const void * key, int len, int seed )
{
  volatile int64_t begin, end;
  uint32_t temp[16];

  begin = timer_start();
  
  hash(key,len,seed,temp);
  
  end = timer_end();
  
  return end - begin;
}

//-----------------------------------------------------------------------------
// Specialized procedure for small lengths. Serialize invocations of the hash
// function. Make sure they would not be computed in parallel on an out-of-order CPU.

NEVER_INLINE int64_t timehash_small ( pfHash hash, const void * key, int len, int seed )
{
  const int NUM_TRIALS = 200;
  volatile unsigned long long int begin, end;
  uint32_t hash_temp[16] = {0};
  uint32_t *buf;
  if (!need_minlen64_align16(hash)) {
    buf = new uint32_t[1 + (len + 3) / 4]();
  } else {
    assert(len < 64);
    buf = new uint32_t[64/4]();
  }
  memcpy(buf,key,len);

  begin = timer_start();

  for(int i = 0; i < NUM_TRIALS; i++) {
    hash(buf + (hash_temp[0] & 1),len,seed,hash_temp);
    // XXX Add dependency between invocations of hash-function to prevent parallel
    // evaluation of them. However this way the invocations still would not be
    // fully serialized. Another option is to use lfence instruction (load-from-memory
    // serialization instruction) or mfence (load-from-memory AND store-to-memory
    // serialization instruction):
    //   __asm volatile ("lfence");
    // It's hard to say which one is the most realistic and sensible approach.
    seed += hash_temp[0];
  }

  end = timer_end();
  delete[] buf;

  return (int64_t)((end - begin) / (double)NUM_TRIALS);
}

//-----------------------------------------------------------------------------

double SpeedTest ( pfHash hash, uint32_t seed, const int trials, const int blocksize, const int align )
{
  Rand r(seed);
  uint8_t *buf = new uint8_t[blocksize + 512];
  uint64_t t1 = reinterpret_cast<uint64_t>(buf);
  
  t1 = (t1 + 255) & UINT64_C(0xFFFFFFFFFFFFFF00);
  t1 += align;
  
  uint8_t * block = reinterpret_cast<uint8_t*>(t1);

  r.rand_p(block,blocksize);

  //----------

  std::vector<double> times;
  times.reserve(trials);

  for(int itrial = 0; itrial < trials; itrial++)
  {
    r.rand_p(block,blocksize);

    double t;

    if(blocksize < 100)
    {
      t = (double)timehash_small(hash,block,blocksize,itrial);
    }
    else
    {
      t = (double)timehash(hash,block,blocksize,itrial);
    }

    if(t > 0) times.push_back(t);
  }

  //----------
  
  std::sort(times.begin(),times.end());
  
  FilterOutliers(times);
  
  delete [] buf;
  
  return CalcMean(times);
}

//-----------------------------------------------------------------------------
// 256k blocks seem to give the best results.

void BulkSpeedTest ( pfHash hash, uint32_t seed )
{
  const int trials = 2999;
  const int blocksize = 256 * 1024;

  printf("Bulk speed test - %d-byte keys\n",blocksize);
  double sumbpc = 0.0;

  volatile double warmup_cycles = SpeedTest(hash,seed,trials,blocksize,0);

  for(int align = 7; align >= 0; align--)
  {
    double cycles = SpeedTest(hash,seed,trials,blocksize,align);

    double bestbpc = double(blocksize)/cycles;

    double bestbps = (bestbpc * 3000000000.0 / 1048576.0);
    printf("Alignment %2d - %6.3f bytes/cycle - %7.2f MiB/sec @ 3 ghz\n",align,bestbpc,bestbps);
    sumbpc += bestbpc;
  }
  sumbpc = sumbpc / 8.0;
  printf("Average      - %6.3f bytes/cycle - %7.2f MiB/sec @ 3 ghz\n",sumbpc,(sumbpc * 3000000000.0 / 1048576.0));
  fflush(NULL);
}

//-----------------------------------------------------------------------------

double TinySpeedTest ( pfHash hash, int hashsize, int keysize, uint32_t seed, bool verbose )
{
  const int trials = 99999;

  if(verbose) printf("Small key speed test - %4d-byte keys - ",keysize);
  
  double cycles = SpeedTest(hash,seed,trials,keysize,0);
  
  printf("%8.2f cycles/hash\n",cycles);
  return cycles;
}

double HashMapSpeedTest ( pfHash pfhash, const int hashbits,
                          std::vector<std::string> words,
                          const uint32_t seed, const int trials, bool verbose )
{
  //using phmap::flat_node_hash_map;
  Rand r(82762);
  std_hashmap hashmap(words.size(), [=](const std::string &key)
                  {
                    // 256 needed for hasshe2, but only size_t used
                    static char out[256] = { 0 };
                    pfhash(key.c_str(), key.length(), seed, &out);
                    return *(size_t*)out;
                  });
  fast_hashmap phashmap(words.size(), [=](const std::string &key)
                  {
                    static char out[256] = { 0 }; // 256 for hasshe2, but stripped to 64/32
                    pfhash(key.c_str(), key.length(), seed, &out);
                    return *(size_t*)out;
                  });
  
  std::vector<std::string>::iterator it;
  std::vector<double> times;
  double t1;

  printf("std::unordered_map\n");
  printf("Init std HashMapTest:     ");
  fflush(NULL);
  times.reserve(trials);
  if (need_minlen64_align16(pfhash)) {
    for (it = words.begin(); it != words.end(); it++) {
      // requires min len 64, and 16byte key alignment
      (*it).resize(64);
    }
  }
  {
    // hash inserts plus 1% deletes
    volatile int64_t begin, end;
    int i = 0;
    begin = timer_start();
    for (it = words.begin(); it != words.end(); it++, i++) {
      std::string line = *it;
      hashmap[line] = 1;
      if (i % 100 == 0)
        hashmap.erase(line);
    }
    end = timer_end();
    t1 = (double)(end - begin) / (double)words.size();
  }
  fflush(NULL);
  printf("%0.3f cycles/op (%zu inserts, 1%% deletions)\n",
         t1, words.size());
  printf("Running std HashMapTest:  ");
  if (t1 > 10000.) { // e.g. multiply_shift 459271.700
    printf("SKIP");
    return 0.;
  }
  fflush(NULL);

  for(int itrial = 0; itrial < trials; itrial++)
    { // hash query
      volatile int64_t begin, end;
      int i = 0, found = 0;
      double t;
      begin = timer_start();
      for ( it = words.begin(); it != words.end(); it++, i++ )
        {
          std::string line = *it;
          if (hashmap[line])
            found++;
        }
      end = timer_end();
      t = (double)(end - begin) / (double)words.size();
      if(found > 0 && t > 0) times.push_back(t);
    }
  hashmap.clear();

  std::sort(times.begin(),times.end());
  FilterOutliers(times);
  double mean = CalcMean(times);
  double stdv = CalcStdv(times);
  printf("%0.3f cycles/op", mean);
  printf(" (%0.1f stdv)\n", stdv);

  times.clear();

  printf("\ngreg7mdp/parallel-hashmap\n");
  printf("Init fast HashMapTest:    ");
#ifndef NDEBUG
  if ((pfhash == VHASH_32 || pfhash == VHASH_64) && !verbose)
    {
      printf("SKIP");
      return 0.;
    }
#endif
  fflush(NULL);
  times.reserve(trials);
  { // hash inserts and 1% deletes
    volatile int64_t begin, end;
    int i = 0;
    begin = timer_start();
    for (it = words.begin(); it != words.end(); it++, i++) {
      std::string line = *it;
      phashmap[line] = 1;
      if (i % 100 == 0)
        phashmap.erase(line);
    }
    end = timer_end();
    t1 = (double)(end - begin) / (double)words.size();
  }
  fflush(NULL);
  printf("%0.3f cycles/op (%zu inserts, 1%% deletions)\n",
         t1, words.size());
  printf("Running fast HashMapTest: ");
  if (t1 > 10000.) { // e.g. multiply_shift 459271.700
    printf("SKIP");
    return 0.;
  }
  fflush(NULL);  
  for(int itrial = 0; itrial < trials; itrial++)
    { // hash query
      volatile int64_t begin, end;
      int i = 0, found = 0;
      double t;
      begin = timer_start();
      for ( it = words.begin(); it != words.end(); it++, i++ )
        {
          std::string line = *it;
          if (phashmap[line])
            found++;
        }
      end = timer_end();
      t = (double)(end - begin) / (double)words.size();
      if(found > 0 && t > 0) times.push_back(t);
    }
  phashmap.clear();
  fflush(NULL);

  std::sort(times.begin(),times.end());
  FilterOutliers(times);
  double mean1 = CalcMean(times);
  double stdv1 = CalcStdv(times);
  printf("%0.3f cycles/op", mean1);
  printf(" (%0.1f stdv) ", stdv1);
  fflush(NULL);

  return mean;
}

//-----------------------------------------------------------------------------
