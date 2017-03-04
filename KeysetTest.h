//-----------------------------------------------------------------------------
// Keyset tests generate various sorts of difficult-to-hash keysets and compare
// the distribution and collision frequency of the hash results against an
// ideal random distribution

// The sanity checks are also in this cpp/h

#pragma once

#include "Types.h"
#include "Stats.h"
#include "Random.h"   // for rand_p
#include "HashFunc.h"

#include <algorithm>  // for std::swap
#include <assert.h>

//-----------------------------------------------------------------------------
// Sanity tests

bool VerificationTest   ( pfHash hash, const int hashbits, uint32_t expected, int verbose,
        const char *name);
bool SanityTest         ( pfHash hash, const int hashbits );
bool AppendedZeroesTest ( pfHash hash, const int hashbits );

//-----------------------------------------------------------------------------
// Keyset 'Combination' - all possible combinations of input blocks
// used by the "permutation" tests in main.cpp via CombinationKeyTest
template< typename hashtype >
void CombinationKeygenRecurse ( uint32_t * key, int len, int maxlen, 
                  uint32_t * blocks, int blockcount, 
                pfHash hash, std::vector<hashtype> & hashes )
{
  if(len == maxlen) return;

  for(int i = 0; i < blockcount; i++)
  {
    key[len] = blocks[i];
  
    //if(len == maxlen-1)
    {
      hashtype h;
      hash(key,(len+1) * sizeof(uint32_t),0,&h);
      hashes.push_back(h);
    }

    //else
    {
      CombinationKeygenRecurse(key,len+1,maxlen,blocks,blockcount,hash,hashes);
    }
  }
}

// used for the Permutation tests in main.cpp
template< typename hashtype >
bool CombinationKeyTest ( hashfunc<hashtype> hash, int maxlen, uint32_t * blocks, int blockcount, bool testColl, double confidence, bool drawDiagram )
{
  printf("Keyset 'Combination' - up to %d blocks from a set of %d - ",maxlen,blockcount);

  //----------

  std::vector<hashtype> hashes;

  uint32_t * key = new uint32_t[maxlen];

  CombinationKeygenRecurse<hashtype>(key,0,maxlen,blocks,blockcount,hash,hashes);

  delete [] key;

  printf("%d keys\n",(int)hashes.size());

  //----------

  bool result = true;

  result &= TestHashList<hashtype>(hashes,testColl,confidence,drawDiagram);
  
  printf("\n");

  return result;
}

//-----------------------------------------------------------------------------
// Keyset 'Sparse' - generate all possible N-bit keys with up to K bits set
// used by the Sparse tests in main.cpp

template < typename keytype, typename hashtype >
void SparseKeygenRecurse ( pfHash hash, int start, int bitsleft, bool inclusive, keytype & k, std::vector<hashtype> & hashes )
{
  const int nbytes = sizeof(keytype);
  const int nbits = nbytes * 8;

  hashtype h;

  for(int i = start; i < nbits; i++)
  {
    flipbit(&k,nbytes,i);

    if(inclusive || (bitsleft == 1))
    {
      hash(&k,sizeof(keytype),0,&h);
      hashes.push_back(h);
    }

    if(bitsleft > 1)
    {
      SparseKeygenRecurse(hash,i+1,bitsleft-1,inclusive,k,hashes);
    }

    flipbit(&k,nbytes,i);
  }
}

//----------
// used by the Sparse tests in main.cpp
template < int keybits, typename hashtype >
bool SparseKeyTest ( hashfunc<hashtype> hash, const int setbits, bool inclusive, bool testColl, double confidence, bool drawDiagram  )
{
  printf("Keyset 'Sparse' - %d-bit keys with %s %d bits set - ",keybits, inclusive ? "up to" : "exactly", setbits);

  typedef Blob<keybits> keytype;

  std::vector<hashtype> hashes;

  keytype k;
  memset(&k,0,sizeof(k));

  if(inclusive)
  {
    hashtype h;

    hash(&k,sizeof(keytype),uint32_t(0),&h);

    hashes.push_back(h);
  }

  SparseKeygenRecurse(hash,0,setbits,inclusive,k,hashes);

  printf("%d keys\n",(int)hashes.size());

  bool result = true;
  
  result &= TestHashList<hashtype>(hashes,testColl,confidence,drawDiagram);

  printf("\n");

  return result;
}

//-----------------------------------------------------------------------------
// Keyset 'Windows' - for all possible N-bit windows of a K-bit key, generate
// all possible keys with bits set in that window
// this is used by the Windowed tests in main.cpp, which are disabled by default
// and documented as less than useful.
template < typename keytype, typename hashtype >
bool WindowedKeyTest ( hashfunc<hashtype> hash, const int windowbits, bool testCollision, double confidence, bool drawDiagram )
{
  const int keybits = sizeof(keytype) * 8;
  const int keycount = 1 << windowbits;

  std::vector<hashtype> hashes;
  hashes.resize(keycount);

  bool result = true;

  int testcount = keybits;

  printf("Keyset 'Windowed' - %3d-bit key, %3d-bit window - %d tests, %d keys per test\n",
          keybits,windowbits,testcount,keycount);

  for(int j = 0; j <= testcount; j++)
  {
    int minbit = j;

    keytype key;

    for(int i = 0; i < keycount; i++)
    {
      key = i;
      //key = key << minbit;

      lrot(&key,sizeof(keytype),minbit);

      hash(&key,sizeof(keytype),uint32_t(0),&hashes[i]);
    }

    printf("Window at %3d - ",j);

    result &= TestHashList(hashes,testCollision,confidence,drawDiagram);

    //printf("\n");
  }

  return result;
}

//-----------------------------------------------------------------------------
// Keyset 'Cyclic' - generate keys that consist solely of N repetitions of M
// bytes.

// (This keyset type is designed to make MurmurHash2 fail)

template < typename hashtype >
bool CyclicKeyTest ( pfHash hash, int cycleLen, int cycleReps, const int keycount, double confidence, bool drawDiagram )
{
  printf("Keyset 'Cyclic' - %d cycles of %d bytes - %d keys\n",cycleReps,cycleLen,keycount);

  Rand r(483723);

  std::vector<hashtype> hashes;
  hashes.resize(keycount);

  int keyLen = cycleLen * cycleReps;

  uint8_t * cycle = new uint8_t[cycleLen + 16];
  uint8_t * key = new uint8_t[keyLen];

  //----------

  for(int i = 0; i < keycount; i++)
  {
    r.rand_p(cycle,cycleLen);

    *(uint32_t*)cycle = f3mix(i ^ 0x746a94f1);

    for(int j = 0; j < keyLen; j++)
    {
      key[j] = cycle[j % cycleLen];
    }

    hash(key,keyLen,0,&hashes[i]);
  }

  //----------
  
  bool result = true;

  result &= TestHashList(hashes,true,confidence,drawDiagram);
  printf("\n");

  delete [] cycle;
  delete [] key;

  return result;
}

//-----------------------------------------------------------------------------
// Keyset 'TwoBytes' - generate all keys up to length N with two non-zero bytes

void TwoBytesKeygen ( int maxlen, KeyCallback & c );

template < typename hashtype >
bool TwoBytesTest2 ( pfHash hash, int maxlen, double confidence, bool drawDiagram )
{
  std::vector<hashtype> hashes;

  HashCallback<hashtype> c(hash,hashes);

  TwoBytesKeygen(maxlen,c);

  bool result = true;

  result &= TestHashList(hashes,true,confidence,drawDiagram);
  printf("\n");

  return result;
}

//-----------------------------------------------------------------------------
// Keyset 'Text' - generate all keys of the form "prefix"+"core"+"suffix",
// where "core" consists of all possible combinations of the given character
// set of length N.

template < typename hashtype >
bool TextKeyTest ( hashfunc<hashtype> hash, const char * prefix, const char * coreset, const int corelen, const char * suffix, double confidence, bool drawDiagram )
{
  const int prefixlen = (int)strlen(prefix);
  const int suffixlen = (int)strlen(suffix);
  const int corecount = (int)strlen(coreset);

  const int keybytes = prefixlen + corelen + suffixlen;
  const int keycount = (int)pow(double(corecount),double(corelen));

  printf("Keyset 'Text' - keys of form \"%s[",prefix);
  for(int i = 0; i < corelen; i++) printf("X");		
  printf("]%s\" - %d keys\n",suffix,keycount);

  uint8_t * key = new uint8_t[keybytes+1];

  key[keybytes] = 0;

  memcpy(key,prefix,prefixlen);
  memcpy(key+prefixlen+corelen,suffix,suffixlen);

  //----------

  std::vector<hashtype> hashes;
  hashes.resize(keycount);

  for(int i = 0; i < keycount; i++)
  {
    int t = i;

    for(int j = 0; j < corelen; j++)
    {
      key[prefixlen+j] = coreset[t % corecount]; t /= corecount;
    }

    hash(key,keybytes,uint32_t(0),&hashes[i]);
  }

  //----------

  bool result = true;

  result &= TestHashList(hashes,true,confidence,drawDiagram);

  printf("\n");

  delete [] key;

  return result;
}

//-----------------------------------------------------------------------------
// Keyset 'RepeatedCharkeyTest' - keys consisting of all the same char,
// differing only in length

// We reuse one block of empty bytes, otherwise the RAM cost is enormous.

template < typename hashtype >
bool RepeatedCharKeyTest ( pfHash hash, const char *name, unsigned char c, int keycount, double confidence, bool drawDiagram )
{

  printf("Keyset '%s' - %d keys\n",name,keycount);

  unsigned char * block = new unsigned char[keycount];
  memset(block,c,keycount);

  //----------

  std::vector<hashtype> hashes;

  hashes.resize(keycount);

  for(int i = 0; i < keycount; i++)
  {
    hash(block,i,0,&hashes[i]);
  }

  bool result = true;

  result &= TestHashList(hashes,true,confidence,drawDiagram);

  printf("\n");

  delete [] block;

  return result;
}

template <typename T>
struct IndirectComparator
{
  const std::vector<T> & value_vector;

  IndirectComparator(const std::vector<T> & val_vec): value_vector(val_vec) {}

  bool operator()(int a, int b)
  {
    return value_vector[a] <  value_vector[b] ? true :
           value_vector[a] == value_vector[b] ? a < b : false;
  }
};
//-----------------------------------------------------------------------------
// Keyset 'Seed' - hash "the quick brown fox..." using different seeds

template < typename seedtype, typename hashtype >
bool SeedTest ( hashfunc<hashtype> hash, int count, double confidence, bool drawDiagram,
    Rand &seed_r, const char * text )
{
  printf("Keyset 'Seed' - %d seeds, key length %d\n  Key \"%s\"\n",
          count, (int)strlen(text), text);

  const int len = (int)strlen(text);
  //----------
  // All of this palaver is to dedupe the seeds.
  // We keep track of the seeds we used for each hash. Then
  // we sort the index array by the seeds, and then use that
  // to transcribe out the hashes in seed-order, while skipping
  // any dupes that are due to dupe seeds.

  std::vector<hashtype> hashes;
  std::vector<hashtype> sorted_hashes;
  std::vector<seedtype> seeds;
  std::vector<int>      indexes;

  sorted_hashes.resize(count);
  hashes.resize(count);
  seeds.resize(count);
  indexes.resize(count);

  for(int i = 0; i < count; i++)
  {
    seed_r.rand_p(&seeds[i],sizeof(seedtype));
    hash(text, len, &seeds[i], &hashes[i]);
    indexes[i]= i;
  }

  // sort the indexes by their seed
  std::sort(indexes.begin(),indexes.end(),IndirectComparator<seedtype>(seeds));

  // now loop through
  int dupes = 0;
  int unique = 0;
  sorted_hashes[unique++] = hashes[indexes[0]];
  for(size_t i = 1; i < indexes.size(); i++) {
    if(seeds[indexes[i]] == seeds[indexes[i-1]]) {
      dupes++;
    } else {
      sorted_hashes[unique++]= hashes[indexes[i]];
    }
  }
  sorted_hashes.resize(unique);

  bool result = true;

  result &= TestHashList(sorted_hashes,true,confidence,drawDiagram);

  printf("\n");

  return result;
}

//-----------------------------------------------------------------------------
/* vim: set sts=2 sw=2 et: */
