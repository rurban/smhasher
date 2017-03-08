//-----------------------------------------------------------------------------
// Differential collision & distribution tests - generate a bunch of random keys,
// see what happens to the hash value when we flip a few bits of the key.

#pragma once

#include "Types.h"
#include "Stats.h"      // for chooseUpToK
#include "KeysetTest.h" // for SparseKeygenRecurse
#include "Random.h"

#include <vector>
#include <algorithm>
#include <stdio.h>

//-----------------------------------------------------------------------------
// Sort through the differentials, ignoring collisions that only occured once 
// (these could be false positives). If we find collisions of 3 or more, the
// differential test fails.
//
// used by DiffTest - which is widely used

template < class keytype >
bool ProcessDifferentials ( std::vector<keytype> & diffs, int reps, bool dumpCollisions, int maxerrors )
{
  std::sort(diffs.begin(), diffs.end());

  int count = 1;
  int ignore = 0;

  bool result = true;

  if(diffs.size())
  {
    keytype kp = diffs[0];

    for(int i = 1; i < (int)diffs.size(); i++)
    {
      if(diffs[i] == kp)
      {
        count++;
        continue;
      }
      else
      {
        if(count > 1)
        {
          result = false;

          double pct = 100 * (double(count) / double(reps));

          if(dumpCollisions)
          {
            printbits((unsigned char*)&kp,sizeof(kp));
            printf("# - %4.2f%%\n", pct );
          }
        }
        else 
        {
          ignore++;
        }

        kp = diffs[i];
        count = 1;
      }
    }

    if(count > 1)
    {
      double pct = 100 * (double(count) / double(reps));

      if(dumpCollisions)
      {
        printbits((unsigned char*)&kp,sizeof(kp));
        printf("# - %4.2f%%\n", pct );
      }
    }
    else 
    {
      ignore++;
    }
  }

  printf("# %d total collisions, of which %d single collisions were ignored\n",
      (int)diffs.size(),ignore);

  return ok(result,"ProcessDifferentials");
}

//-----------------------------------------------------------------------------
// Check all possible keybits-choose-N differentials for collisions, report
// ones that occur significantly more often than expected.

// Random collisions can happen with probability 1 in 2^32 - if we do more than
// 2^32 tests, we'll probably see some spurious random collisions, so don't report
// them.
// used by DiffTest - which is widely used.
template < typename keytype, typename hashtype >
uint64_t DiffTestRecurse ( hashfunc<hashtype> hash, keytype & k1, keytype & k2, hashtype & h1, hashtype & h2, int start, int bitsleft, std::vector<keytype> & diffs, int maxerrors)
{
  const int bits = sizeof(keytype)*8;
  uint64_t skipped = 0;

  for(int i = start; i < bits; i++)
  {
    flipbit(&k2,sizeof(k2),i);
    bitsleft--;

    hash(&k2,sizeof(k2),&h2);
    // printf("hash2: %08x\n", *((uint32_t *)&h2));

    if(h1 == h2)
    {
      /* stop collecting data at this point */
      if (diffs.size() < maxerrors) {
        diffs.push_back(k1 ^ k2);
      } else {
        skipped++;
      }
    }

    if(bitsleft)
    {
      skipped += DiffTestRecurse(hash,k1,k2,h1,h2,i+1,bitsleft,diffs,maxerrors);
    }

    flipbit(&k2,sizeof(k2),i);
    bitsleft++;
  }
  return skipped;
}

//----------
// widely used
template < typename keytype, typename hashtype >
bool DiffTest ( hashfunc<hashtype> hash, int diffbits, int reps, bool dumpCollisions, Rand &r )
{
  const int keybits = sizeof(keytype) * 8;
  const int hashbits = sizeof(hashtype) * 8;

  double diffcount = chooseUpToK(keybits,diffbits);
  double testcount = (diffcount * double(reps));
  double expected  = testcount / pow(2.0,double(hashbits));

  std::vector<keytype> diffs;

  keytype k1,k2;
  hashtype h1,h2;
  memset(&h1,0,sizeof(hashtype));
  memset(&h2,0,sizeof(hashtype));

  printf("# Testing %0.f up-to-%d-bit differentials in %d-bit keys -> %d bit hashes.\n",
          diffcount,diffbits,keybits,hashbits);
  printf("# %d reps, %0.f total tests, expecting %2.2f random collisions",
          reps,testcount,expected);
  int maxerrors= 100000; /* we need a limit or we could run out of memory */
  uint64_t skipped= 0;

  for(int i = 0; i < reps; i++)
  {
    if(i % (reps/10) == 0) printf("%c",skipped ? '!' : '.');

    r.rand_p(&k1,sizeof(keytype));
    k2 = k1;
    hash.seed_state_rand(r);
    hash(&k1,sizeof(k1),(uint32_t*)&h1);
    //printf("hash1: %08x\n", *((uint32_t *)&h1));

    skipped += DiffTestRecurse<keytype,hashtype>(hash,k1,k2,h1,h2,0,diffbits,diffs,maxerrors);
  }
  printf("\n"); // nl ok
  bool result = ok(diffs.size() < maxerrors,"Differential collisions",hash.name());
  result &= ok(skipped==0,"Nothing skipped during differential collision check",hash.name());
  return result && ProcessDifferentials(diffs,reps,dumpCollisions,maxerrors);
}

//-----------------------------------------------------------------------------
// Simpler differential-distribution test - for all 1-bit differentials,
// generate random key pairs and run full distribution/collision tests on the
// hash differentials

template < typename keytype, typename hashtype >
bool DiffDistTest2 ( hashfunc<hashtype> hash, double confidence  )
{
  Rand r(857374);

  int keybits = sizeof(keytype) * 8;
  const int keycount = 256*256*32;
  keytype k;
  
  std::vector<hashtype> hashes(keycount);
  hashtype h1,h2;
  memset(&h1,0,sizeof(hashtype));
  memset(&h2,0,sizeof(hashtype));

  bool result = true;

  hash.seed_state_rand(r);

  for(int keybit = 0; keybit < keybits; keybit++)
  {
    char name[1024];
    snprintf(name,1024,"differential distribution - bit %d\n",keybit);

    for(int i = 0; i < keycount; i++)
    {
      r.rand_p(&k,sizeof(keytype));
      
      hash(&k,sizeof(keytype),&h1);
      flipbit(&k,sizeof(keytype),keybit);
      hash(&k,sizeof(keytype),&h2);

      hashes[i] = h1 ^ h2;
    }

    result &= TestHashList<hashtype>(hashes,true,confidence,true,name);
  }

  return result;
}

//----------------------------------------------------------------------------
/* vim: set sts=2 sw=2 et: */
