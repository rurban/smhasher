#include "SpeedTest.h"
#include "Random.h"
#include "vmac.h"
#include "Hashes.h"

#include <stdio.h>   // for printf
#include <memory.h>  // for memset
#include <math.h>    // for sqrt
#include <algorithm> // for sort, min
#include <string>
#include <sstream>

#include <unordered_map>
#include <parallel_hashmap/phmap.h>
#include <functional>

#define COUNT_OF(x) ((sizeof(x)/sizeof(0[x])) / ((size_t)(!(sizeof(x) % sizeof(0[x])))))
#define ARRAY_END(x) (&(x)[COUNT_OF(x)])

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

  return timer_sub(end, begin);
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

  return timer_sub(end, begin) / NUM_TRIALS;
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

    if(blocksize <= TIMEHASH_SMALL_LEN_MAX)
    {
      t = (double)timehash_small(hash,block,blocksize,itrial);
    }
    else
    {
      SampleCpuFreq();
      t = (double)timehash(hash,block,blocksize,itrial);
    }

    if(t > 0) times.push_back(t);
  }

  if (blocksize > TIMEHASH_SMALL_LEN_MAX)
    SampleCpuFreq();

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

  const double MHz2MiBps = (1000.0 * 1000.0) / (1024.0 * 1024.0);
  for(int align = 7; align >= 0; align--)
  {
    double cycles = SpeedTest(hash,seed,trials,blocksize,align);

    double bestbpc = double(blocksize)/cycles;
    const unsigned cpuMHz = GetCpuFreqMHz();
    double bestbps = (bestbpc * cpuMHz * MHz2MiBps);
    printf("Alignment %2d - %6.3f bytes/cycle - %7.2f MiB/sec @ %u MHz\n",align,bestbpc,bestbps,cpuMHz);
    sumbpc += bestbpc;
  }
  const unsigned cpuMHz = GetCpuFreqMHz();
  sumbpc = sumbpc / 8.0;
  printf("Average      - %6.3f bytes/cycle - %7.2f MiB/sec @ %u MHz\n",sumbpc,sumbpc*cpuMHz*MHz2MiBps,cpuMHz);
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

static void ReportAverage ( const std::vector<double>& cph, int minkey, int maxkey )
{
  double sum = 0;
  for (int i = minkey; i <= maxkey; i++)
    sum += cph[i];
  sum /= (maxkey - minkey + 1);
  printf("Average                                 %9.3f cycles/hash\n",sum);
}

static void ReportWeighted ( const std::vector<double>& cph, const std::vector<double>& weights, int minkey, int maxkey, const char *name )
{
  assert(0 <= minkey && minkey <= maxkey && maxkey <= cph.size() + 1);
  if (weights.size() < cph.size()) {
    printf("Average, weighted by key length, SKIP %s dataset, need %lu more weights\n",
        name, cph.size() - weights.size());
    return;
  }
  double tot = 0.0, use = 0.0, sum = 0.0;
  for (int i = 0; i < minkey; i++)
    tot += weights[i];
  for (int i = minkey; i <= maxkey; i++) {
    sum += weights[i] * cph[i];
    use += weights[i];
    tot += weights[i];
  }
  for (int i = maxkey + 1; i < weights.size(); i++)
    tot += weights[i];
  printf("Average, weighted by key length freq.   %9.3f cycles/hash (using %.1f%% of %s dataset)\n",
      sum / use, 100. * use / tot, name);
}

// These are lengths of top 7,073,200 domain names from Tranco. The list represents "popular" domain
// names.  The dataset was downloaded from https://tranco-list.eu/list/LJ5W4/1000000 on 2024-Sep-05
// SHA256(tranco_LJ5W4.csv) = 4593f2a162697946f36ef7bbe7c8b434eec42e0e93c4298517c4a3966b08c054
//
// Victor Le Pochat, Tom Van Goethem, Samaneh Tajalizadehkhoob, Maciej Korczyński, and Wouter
// Joosen. 2019. "Tranco: A Research-Oriented Top Sites Ranking Hardened Against Manipulation",
// Proceedings of the 26th Annual Network and Distributed System Security Symposium (NDSS 2019).
// https://doi.org/10.14722/ndss.2019.23386
//
static const unsigned TrancoDNSNameLengths[] = { 0, 0, 5, 0, 326, 5568, 41632, 88175, 151138, 253649,
  386024, 416786, 458718, 482490, 491891, 473417, 450606, 413517, 371676, 327361, 288868, 251641,
  213514, 178542, 159986, 132611, 113222, 101498, 82455, 67296, 67906, 56843, 53731, 49744, 36404,
  32346, 30329, 26978, 24359, 24345, 19161, 16914, 16370, 13708, 13714, 10832, 13548, 9635, 8125,
  15536, 6273, 8207, 7490, 5196, 7330, 6202, 3801, 4455, 3756, 3709, 4142, 3989, 3593, 4783, 5052,
  1403, 1580, 2072, 1998, 1420, 1836, 1872, 1135, 2664, 1172, 837, 998, 1063, 685, 566, 2020, 627,
  2146, 1144, 635, 618, 569, 756, 411, 361, 362, 1138, 218, 278, 182, 185, 175, 220, 3205, 143, 353,
  131, 132, 199, 134, 139, 130, 168, 135, 169, 630, 155, 137, 129, 229, 154, 166, 205, 204, 203, 208,
  201, 211, 141, 157, 147, 172, 183, 134, 155, 123, 159, 148, 165, 145, 143, 112, 111, 112, 115, 128,
  120, 116, 119, 137, 123, 106, 118, 105, 125, 126, 106, 99, 124, 102, 94, 95, 113, 105, 103, 118, 81,
  103, 86, 78, 80, 82, 70, 72, 74, 52, 58, 71, 46, 67, 65, 70, 74, 75, 66, 59, 81, 110, 97, 107, 116,
  109, 72, 67, 89, 82, 79, 73, 82, 83, 73, 71, 89, 98, 103, 90, 118, 120, 67, 63, 50, 71, 57, 67, 64,
  54, 55, 65, 53, 73, 65, 63, 60, 83, 80, 61, 87, 82, 55, 74, 66, 38, 41, 22, 47, 27, 36, 30, 38, 33,
  46, 33, 36, 58, 50, 61, 71, 99, 46, 50, 54, 38, 17, 15, 4, 3, 0, 0, 116, 0, 0 };

// These are lengths of 1,000,000 calls to umash_full() during the batch hash table phase.
// It's arguably with an off-by-one, since NUL terminators are included in the hashed data.
//
// All the lengths are clamped to 256 bytes per TIMEHASH_SMALL_LEN_MAX.
// The last bin UmashStartupLengths[256] is essentially the long tail that is never used.
//
// startup-1M.2020-08-28.trace.bz2 @ https://github.com/backtrace-labs/umash/wiki/Execution-traces
// SHA256(trace.bz2) = 02bae7f0e07880bf24fdd67b6d5fc2a675c6ca05b534081925a16f06c11659c0
//
static const unsigned UmashStartupLengths[] = { 0, 7, 51, 396, 1312, 3110, 5616, 7887, 11145, 68172,
  14618, 16670, 9502, 8275, 7444, 8088, 105451, 246, 100, 117, 116, 487, 367, 179, 293, 58, 56, 124,
  191, 340, 323, 333, 303, 274, 238, 202, 246, 409961, 235, 10119, 239, 171, 128, 100, 5217, 51, 62,
  53, 42, 69, 63, 89, 38, 52, 102, 84, 90, 75, 61, 90, 55, 57, 60, 71, 106, 92520, 54, 57, 101, 316,
  961, 1873, 1714, 290, 88, 185, 600, 1038, 1762, 3228, 3174, 284, 266, 292, 752, 1381, 1331, 145,
  161, 177, 1517, 304, 176, 9464, 342, 1809, 286, 962, 116, 390, 383, 244, 50, 54, 46, 88, 191, 74,
  54, 91, 110, 11347, 4310, 5021, 51, 189, 902, 60, 3476, 44543, 275, 5960, 58, 1705, 84, 15, 34, 68,
  1113, 43, 55, 27, 126, 15, 33, 1512, 14, 359, 13, 43, 7604, 78108, 43, 27, 7, 23, 140, 5, 3, 0, 13,
  6, 8, 33, 54, 3, 0, 0, 13, 10, 13, 0, 6, 5, 11, 0, 11, 25, 11, 9, 0, 12, 13, 0, 0, 41, 3, 4, 8, 49,
  29, 25, 17, 10, 3, 29, 7, 9, 2, 20, 17, 17, 5, 35, 3, 5, 0, 13, 0, 149, 17, 6, 8, 3, 11, 17, 0, 1,
  780, 0, 0, 14, 29, 10, 3, 14, 20, 9, 12, 29, 11, 6, 10, 6, 12, 0, 10, 7, 22, 13, 6, 10, 14, 167, 0,
  3, 0, 11, 7, 5, 9, 35, 4, 5, 7, 2, 14, 6, 7, 2, 16, 5, 6, 8, 0, 4, 1022 };

// Weighted average exist under assumption that hash speed does not depend on input,
// which is not true due to multiplication instruction having certain amount of variance.
void ReportTinySpeedTest ( const std::vector<double>& cycles_per_hash, int minkey, int maxkey )
{
  ReportAverage(cycles_per_hash, minkey, maxkey);

  std::vector<double> w(TrancoDNSNameLengths, ARRAY_END(TrancoDNSNameLengths));
  ReportWeighted(cycles_per_hash, w, minkey, maxkey, "top-7m Tranco DNS names");
  w.clear();

  w.insert(w.begin(), UmashStartupLengths, ARRAY_END(UmashStartupLengths));
  ReportWeighted(cycles_per_hash, w, minkey, maxkey, "startup-1M UMASH trace");
  w.clear();

  if (const char *ew = getenv("SMHASHER_SMALLKEY_WEIGHTS"))
  {
    std::istringstream ssws(ew);
    for (double flt; ssws >> flt; )
      w.push_back(flt);
    ReportWeighted(cycles_per_hash, w, minkey, maxkey, "${SMHASHER_SMALLKEY_WEIGHTS}");
    w.clear();
  }
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
    t1 = double(timer_sub(end, begin)) / words.size();
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
      t = double(timer_sub(end, begin)) / words.size();
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
    t1 = double(timer_sub(end,  begin)) / words.size();
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
      t = double(timer_sub(end, begin)) / words.size();
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

