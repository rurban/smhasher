#pragma once

#include "Types.h"
#include "Random.h"

void BulkSpeedTest ( pfHash hash, uint32_t seed );
double TinySpeedTest ( pfHash hash, int hashsize, int keysize, uint32_t seed, bool verbose );

double CalcMean ( std::vector<double> & v );
double CalcStdv ( std::vector<double> & v );
bool ContainsOutlier ( std::vector<double> & v, size_t len );
void FilterOutliers ( std::vector<double> & v );

#include <algorithm> // for sort, min
#include <string>
#include <unordered_map>
#include <parallel_hashmap/phmap.h>
#include <functional>

typedef std::unordered_map<std::string, int,
  std::function<size_t (const std::string &key)>> std_hashmap;
typedef phmap::flat_hash_map<std::string, int,
  std::function<size_t (const std::string &key)>> fast_hashmap;

template< typename hashtype >
double HashMapSpeedTest ( pfHash pfhash, const int hashbits,
                          std::vector<std::string> words,
                          const int trials, bool verbose )
{
  //using phmap::flat_node_hash_map;
  Rand r(82762);
  const uint32_t seed = r.rand_u32();
  std_hashmap hashmap(words.size(), [=](const std::string &key)
                  {
                    // 256 needed for hasshe2, but only size_t used
                    static hashtype out;
                    pfhash(key.c_str(), key.length(), seed, &out);
                    return *(size_t*)out;
                  });
  fast_hashmap phashmap(words.size(), [=](const std::string &key)
                  {
                    static hashtype out;
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
  { // hash inserts and 1% deletes
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
