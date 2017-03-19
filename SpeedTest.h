#pragma once
#include "Random.h"

#include <stdio.h>   // for printf
#include <memory.h>  // for memset
#include <math.h>    // for sqrt
#include <algorithm> // for sort

#include "Types.h"
#include "HashFunc.h"

double CalcMean ( std::vector<double> & v );
void FilterOutliers ( std::vector<double> & v );
//-----------------------------------------------------------------------------
// We really want the rdtsc() calls to bracket the function call as tightly
// as possible, but that's hard to do portably. We'll try and get as close as
// possible by marking the function as NEVER_INLINE (to keep the optimizer from
// moving it) and marking the timing variables as "volatile register".
// NOTE THIS CHANGES THE KEY ON EACH CALL!

//-----------------------------------------------------------------------------
// Specialized procedure for small lengths. Serialize invocations of the hash
// function, make sure they would not be computed in parallel on an out-of-order CPU.

template < typename hashtype >
NEVER_INLINE int64_t timehash ( hashfunc<hashtype> hash, void * key, int len, int seed, int trials)
{
  volatile unsigned long long int begin, end;

  /* initialize the seed */
  if (!trials)
    trials = len < 200 ? 200 - len : 1;
  if (trials <= 0)
    trials = 1;

  int n= trials;

  begin = rdtsc();
  do {
    hash(key,len,key);
  } while (--n);
  end = rdtsc();

  return (int64_t)((end - begin) / (double)trials);
}
//-----------------------------------------------------------------------------

template < typename hashtype >
double runSpeedTest ( hashfunc<hashtype> hash, const int trials, const int blocksize, const int align, Rand &r )
{
  uint8_t * buf = new uint8_t[blocksize + 512];

  uint64_t t1 = reinterpret_cast<uint64_t>(buf);

  t1 = (t1 + 255) & BIG_CONSTANT(0xFFFFFFFFFFFFFF00);
  t1 += align;

  uint8_t * block = reinterpret_cast<uint8_t*>(t1);

  r.rand_p(block,blocksize);


  //----------
  // NOTE - All invocation of timehash and thus this function
  // modify their key argument by writing their output hash into the key.
  // Add dependency between invocations of hash-function to prevent parallel
  // evaluation of them. However this way the invocations still would not be
  // fully serialized. Another option is to use lfence instruction (load-from-memory
  // serialization instruction) or mfence (load-from-memory AND store-to-memory
  // serialization instruction):
  //   __asm volatile ("lfence");
  // It's hard to say which one is the most realistic and sensible approach.

  std::vector<double> times;
  times.reserve(trials);

  uint32_t seed = 0;

  hash.seed_state_rand(r);

  /* warm up */
  for(int itrial = 0; itrial < 2; itrial++)
  {
    r.rand_p(block,blocksize);

    double t = (double)timehash(hash, block, blocksize, seed, 0);
  }

  for(int itrial = 0; itrial < trials; itrial++)
  {
    r.rand_p(block,blocksize);

    double t = (double)timehash(hash, block, blocksize, seed, 0);

    /* why do we ignore 0? That makes no sense... */
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

template < typename hashtype >
void BulkSpeedTest ( hashfunc<hashtype> hash, Rand &r )
{
  const int trials = 4999;
  const int blocksize = 256 * 1024;

  printf("## Bulk speed test - %d-byte keys\n",blocksize);
  double sumbpc = 0.0;

  volatile double warmup_cycles = runSpeedTest(hash,trials,blocksize,0,r);

  for(int align = 7; align >= 0; align--)
  {
    double cycles = runSpeedTest(hash,trials,blocksize,align,r);

    double bestbpc = double(blocksize)/cycles;

    double bestbps = (bestbpc * 3000000000.0 / 1048576.0);
    printf("# Alignment %2d - %6.3f bytes/cycle - %7.2f MiB/sec @ 3 ghz\n",align,bestbpc,bestbps);
    sumbpc += bestbpc;
  }
  sumbpc = sumbpc / 8.0;
  printf("# Average      - %6.3f bytes/cycle - %7.2f MiB/sec @ 3 ghz\n",
          sumbpc,(sumbpc * 3000000000.0 / 1048576.0));
}

//-----------------------------------------------------------------------------

template < typename hashtype >
double TinySpeedTest ( hashfunc<hashtype> hash, int keysize, int trials, Rand &r, bool verbose)
{

  double cycles = runSpeedTest(hash,trials,keysize,0,r);

  if (verbose) {
    if (keysize) {
      double ks= keysize;
      printf("# %-20s %6d byte keys %12.3f c/h %12.3f c/b %12.3f b/c\n",
          hash.name(), keysize, cycles, cycles/ks, ks/cycles);
    } else {
      printf("# %-20s %6d byte keys %12.3f c/h\n",
          hash.name(), keysize, cycles);
    }
  }
  return cycles;
}

template < typename hashtype >
bool RunKeySpeedTests(hashfunc<hashtype> hash, Rand & r)
{
  int count = 0;
  double sum = 0.0;
  double sum_key_len = 0.0;

  printf("## KeySpeed tests\n");
  for(int key_len = 0; key_len < 32; key_len++)
  {
    sum += TinySpeedTest(hash, key_len, 199999, r, true);
    sum_key_len += double(key_len);
    count++;
  }
  printf("# %-20s %16s %12.3f c/h %12.3f c/b %12.3f b/c\n",
      "", "Average < 32",
      sum / double(count), sum / sum_key_len, sum_key_len / sum );
  for(int key_len = 32; key_len < 128; key_len+=4)
  {
    sum += TinySpeedTest(hash, key_len, 99999, r, true);
    sum_key_len += double(key_len);
    count++;
  }
  printf("# %-20s %16s %12.3f c/h %12.3f c/b %12.3f b/c\n",
      "", "Average < 128",
      sum / double(count), sum / sum_key_len, sum_key_len / sum );
  for(int key_len = 128; key_len <= 1 << 16; key_len *= 2)
  {
    sum += TinySpeedTest(hash, key_len, 59999, r, true);
    sum_key_len += double(key_len);
    count++;
  }
  printf("# %-20s %16s %12.3f c/h %12.3f c/b %12.3f b/c\n",
      "", "Overall Average",
      sum / double(count), sum / sum_key_len, sum_key_len / sum );
  return true;
}

//-----------------------------------------------------------------------------
/* vim: set sts=2 sw=2 et: */
