//-----------------------------------------------------------------------------
// Keyset tests generate various sorts of difficult-to-hash keysets and compare
// the distribution and collision frequency of the hash results against an
// ideal random distribution

// The sanity checks are also in this cpp/h

#pragma once

#include "Types.h"
#include "Stats.h"
#include "Random.h"   // for rand_p

#include <algorithm>  // for std::swap
#include <assert.h>

//-----------------------------------------------------------------------------
// Sanity tests

bool VerificationTest   ( pfHash hash, const int hashbits, uint32_t expected, bool verbose );
bool SanityTest         ( pfHash hash, const int hashbits );
void AppendedZeroesTest ( pfHash hash, const int hashbits );

//-----------------------------------------------------------------------------
// Keyset 'Combination' - all possible combinations of input blocks

static void printKey(const void* key, size_t len)
{
    const unsigned char* const p = (const unsigned char*)key;
    size_t s;
    printf("\n0x");
    for (s=0; s<len; s++) printf("%02X", p[s]);
    printf("\n  ");
    for (s=0; s<len; s+=8) printf("%-16zu", s);
}

template< typename hashtype, class blocktype >
void CombinationKeygenRecurse ( blocktype * key, int len, int maxlen,
                  blocktype * blocks, int blockcount,
                  pfHash hash, std::vector<hashtype> & hashes )
{
  if(len == maxlen) return;  // end recursion

  for(int i = 0; i < blockcount; i++)
  {
    key[len] = blocks[i];

    //if(len == maxlen-1)
    {
      hashtype h;
      hash(key, (len+1) * sizeof(blocktype), 0, &h);
      hashes.push_back(h);
    }

    //else
    {
      CombinationKeygenRecurse(key,len+1,maxlen,blocks,blockcount,hash,hashes);
    }
  }
}

typedef struct { char c[16]; } block16;
typedef struct { char c[32]; } block32;
typedef struct { char c[64]; } block64;
typedef struct { char c[128]; } block128;

template< typename hashtype, typename blocktype >
bool CombinationKeyTest ( hashfunc<hashtype> hash, int maxlen, blocktype* blocks,
                          int blockcount, bool testColl, bool testDist, bool drawDiagram )
{
  printf("Keyset 'Combination' - up to %d blocks from a set of %d - ",maxlen,blockcount);

  //----------

  std::vector<hashtype> hashes;

  blocktype * key = new blocktype[maxlen];

  CombinationKeygenRecurse(key,0,maxlen,blocks,blockcount,hash,hashes);

  delete [] key;

  printf("%d keys\n",(int)hashes.size());

  //----------

  bool result = true;

  result &= TestHashList(hashes,drawDiagram,testColl,testDist);

  printf("\n");

  return result;
}



//----------------------------------------------------------------------------
// Keyset 'Permutation' - given a set of 32-bit blocks, generate keys
// consisting of all possible permutations of those blocks

template< typename hashtype >
void PermutationKeygenRecurse ( pfHash hash, uint32_t * blocks, int blockcount, int k, std::vector<hashtype> & hashes )
{
  if(k == blockcount-1)
  {
    hashtype h;

    hash(blocks,blockcount * sizeof(uint32_t),0,&h);

    hashes.push_back(h);

    return;
  }

  for(int i = k; i < blockcount; i++)
  {
    std::swap(blocks[k],blocks[i]);

    PermutationKeygenRecurse(hash,blocks,blockcount,k+1,hashes);

    std::swap(blocks[k],blocks[i]);
  }
}

template< typename hashtype >
bool PermutationKeyTest ( hashfunc<hashtype> hash, uint32_t * blocks, int blockcount, bool testColl, bool testDist, bool drawDiagram )
{
  printf("Keyset 'Permutation' - %d blocks - ",blockcount);

  //----------

  std::vector<hashtype> hashes;

  PermutationKeygenRecurse<hashtype>(hash,blocks,blockcount,0,hashes);

  printf("%d keys\n",(int)hashes.size());

  //----------

  bool result = true;

  result &= TestHashList<hashtype>(hashes,drawDiagram,testColl,testDist);

  printf("\n");

  return result;
}

//-----------------------------------------------------------------------------
// Keyset 'Sparse' - generate all possible N-bit keys with up to K bits set

static void printSparseKey(const void* buffer, size_t size)
{
    const char* const p = (const char*)buffer;
    size_t s;
    printf("bits:");
    for (s=0; s<size; s++) {
        int b;
        for (b=0; b<8; b++) {
            if ((p[s] >> b) & 1)
                printf(" %2u.%2i,", (unsigned)s, b);
        }
    }
}

template < typename keytype, typename hashtype >
void SparseKeygenRecurse ( pfHash hash, int start, int bitsleft, bool inclusive, keytype & k, std::vector<hashtype> & hashes )
{
  const int nbytes = sizeof(keytype);
  const int nbits = nbytes * 8;

  hashtype h;

  for(int i = start; i < nbits; i++)
  {
    flipbit(&k, nbytes, i);

    if(inclusive || (bitsleft == 1))
    {
      hash(&k, sizeof(keytype), 0, &h);
      hashes.push_back(h);
    }

    if(bitsleft > 1)
    {
      SparseKeygenRecurse(hash, i+1, bitsleft-1, inclusive, k, hashes);
    }

    flipbit(&k, nbytes, i);
  }
}

//----------

template < int keybits, typename hashtype >
bool SparseKeyTest ( hashfunc<hashtype> hash, const int setbits, bool inclusive, bool testColl, bool testDist, bool drawDiagram  )
{
  printf("Keyset 'Sparse' - %d-bit keys with %s %d bits set - ",keybits, inclusive ? "up to" : "exactly", setbits);

  typedef Blob<keybits> keytype;

  std::vector<hashtype> hashes;

  keytype k;
  memset(&k,0,sizeof(k));

  if(inclusive)
  {
    hashtype h;

    hash(&k,sizeof(keytype),0,&h);

    hashes.push_back(h);
  }

  SparseKeygenRecurse(hash,0,setbits,inclusive,k,hashes);

  printf("%d keys\n",(int)hashes.size());

  bool result = true;

  result &= TestHashList<hashtype>(hashes,drawDiagram,testColl,testDist);

  printf("\n");

  return result;
}

//-----------------------------------------------------------------------------
// Keyset 'Window' - for all possible N-bit windows of a K-bit key, generate
// all possible keys with bits set in that window

template < typename keytype, typename hashtype >
bool WindowedKeyTest ( hashfunc<hashtype> hash, const int windowbits,
                       bool testCollision, bool testDistribution, bool drawDiagram )
{
  const int keybits = sizeof(keytype) * 8;
  const int keycount = 1 << windowbits;

  std::vector<hashtype> hashes;
  hashes.resize(keycount);

  bool result = true;

  int testcount = keybits;

  printf("Keyset 'Window' - %3d-bit key, %3d-bit window - %d tests, %d keys per test\n",
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

      hash(&key,sizeof(keytype),0,&hashes[i]);
    }

    printf("Window at %3d - ",j);

    result &= TestHashList(hashes, drawDiagram, testCollision, testDistribution,
                           /* do not test high/low bits (to not clobber the screen) */
                           false, false);

    //printf("\n");
  }

  return result;
}

//-----------------------------------------------------------------------------
// Keyset 'Cyclic' - generate keys that consist solely of N repetitions of M
// bytes.

// (This keyset type is designed to make MurmurHash2 fail)

template < typename hashtype >
bool CyclicKeyTest ( pfHash hash, int cycleLen, int cycleReps, const int keycount, bool drawDiagram )
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

  result &= TestHashList(hashes,drawDiagram);
  printf("\n");

  delete [] cycle;
  delete [] key;

  return result;
}

//-----------------------------------------------------------------------------
// Keyset 'TwoBytes' - generate all keys up to length N with two non-zero bytes

void TwoBytesKeygen ( int maxlen, KeyCallback & c );

template < typename hashtype >
bool TwoBytesTest2 ( pfHash hash, int maxlen, bool drawDiagram )
{
  std::vector<hashtype> hashes;

  HashCallback<hashtype> c(hash,hashes);

  TwoBytesKeygen(maxlen,c);

  bool result = true;

  result &= TestHashList(hashes,drawDiagram);
  printf("\n");

  return result;
}

//-----------------------------------------------------------------------------
// Keyset 'Text' - generate all keys of the form "prefix"+"core"+"suffix",
// where "core" consists of all possible combinations of the given character
// set of length N.

template < typename hashtype >
bool TextKeyTest ( hashfunc<hashtype> hash, const char * prefix, const char * coreset, const int corelen, const char * suffix, bool drawDiagram )
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

    hash(key,keybytes,0,&hashes[i]);
  }

  //----------

  bool result = true;

  result &= TestHashList(hashes,drawDiagram);

  printf("\n");

  delete [] key;

  return result;
}

//-----------------------------------------------------------------------------
// Keyset 'Zeroes' - keys consisting of all zeroes, differing only in length

// We reuse one block of empty bytes, otherwise the RAM cost is enormous.

template < typename hashtype >
bool ZeroKeyTest ( pfHash hash, bool drawDiagram )
{
  int keycount = 200*1024;

  printf("Keyset 'Zeroes' - %d keys\n",keycount);

  unsigned char * nullblock = new unsigned char[keycount];
  memset(nullblock,0,keycount);

  //----------

  std::vector<hashtype> hashes;

  hashes.resize(keycount);

  for(int i = 0; i < keycount; i++)
  {
    hash(nullblock,i,0,&hashes[i]);
  }

  bool result = true;

  result &= TestHashList(hashes,drawDiagram);

  printf("\n");

  delete [] nullblock;

  return result;
}

//-----------------------------------------------------------------------------
// Keyset 'Seed' - hash "the quick brown fox..." using different seeds

template < typename hashtype >
bool SeedTest ( pfHash hash, int keycount, bool drawDiagram )
{
  printf("Keyset 'Seed' - %d keys\n",keycount);

  const char * text = "The quick brown fox jumps over the lazy dog";
  const int len = (int)strlen(text);

  //----------

  std::vector<hashtype> hashes;

  hashes.resize(keycount);

  for(int i = 0; i < keycount; i++)
  {
    hash(text,len,i,&hashes[i]);
  }

  bool result = true;

  result &= TestHashList(hashes,drawDiagram);

  printf("\n");

  return result;
}

//-----------------------------------------------------------------------------
