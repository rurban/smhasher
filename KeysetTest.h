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
#include "TAP.h"

#include <algorithm>  // for std::swap
#include <assert.h>

//-----------------------------------------------------------------------------
// Sanity tests

//-----------------------------------------------------------------------------
// This should hopefully be a thorough and uambiguous test of whether a hash
// is correctly implemented on a given platform
template <typename hashtype>
bool VerificationTest ( hashfunc<hashtype> hash, uint32_t expected, int verbose, const char *name )
{
  const int hashbytes = sizeof(hashtype);

  uint8_t * key    = new uint8_t[256];
  uint8_t * hashes = new uint8_t[hashbytes * 256];
  uint8_t * final  = new uint8_t[hashbytes];
  Rand r(1);

  memset(key,0,256);
  memset(hashes,0,hashbytes*256);
  memset(final,0,hashbytes);

  // Hash keys of the form {0}, {0,1}, {0,1,2}... up to N=255,
  // using the RNG for seeding.

  for(int i = 0; i < 256; i++)
  {
    key[i] = (uint8_t)i;
    hash.seed_state_rand(r);
    hash(key,i,&hashes[i*hashbytes]);
  }

  // Then hash the result array
  hash.seed_state_rand(r);
  hash(hashes,hashbytes*256,final);

  // The first four bytes of that hash, interpreted as a little-endian integer, is our
  // verification value

  uint32_t verification = (final[0] << 0) | (final[1] << 8) | (final[2] << 16) | (final[3] << 24);

  delete [] key;
  delete [] hashes;
  delete [] final;
  bool different = expected != verification;
  bool result = !different || !verification;

  if (verbose>1)
    ok(result, "Verification code", name);

  //----------
#define _NAMEFMT "# %-20s"
  if (verbose) {
    if (different) {
      if (result) {
        printf(_NAMEFMT " - Verification value 0x%08X : Failed! (Dev-Mode)\n", name, verification);
      } else {
        printf(_NAMEFMT " - Verification value 0x%08X : Failed! (Expected 0x%08X)\n",
          name, verification, expected);
      }
    } else if (verbose > 1) {
      printf(_NAMEFMT " - Verification value 0x%08X : Passed.\n",
        name, verification);
    }
  }
  return result;
}

//----------------------------------------------------------------------------
// Basic sanity checks -

// A hash function should not be reading outside the bounds of the key.

// Flipping a bit of a key should, with overwhelmingly high probability,
// result in a different hash.

// Hashing the same key twice should always produce the same result.

// The memory alignment of the key should not affect the hash result.

template <typename hashtype>
bool SanityTest ( hashfunc<hashtype> hash )
{

  Rand r(883741);

  bool result = true;

  const int hashbytes = sizeof(hashtype);
  const int reps = 10;
  const int keymax = 256;
  const int pad = 16;
  const int buflen = keymax + pad*3;

  uint8_t * buffer1 = new uint8_t[buflen];
  uint8_t * buffer2 = new uint8_t[buflen];

  uint8_t * hash1 = new uint8_t[hashbytes];
  uint8_t * hash2 = new uint8_t[hashbytes];
  uint32_t count_inconsistent = 0;
  uint32_t count_same = 0;
  uint32_t count = 0;
  //----------

  printf("# Sanity check simple key bit flips and consistency");

  for(int irep = 0; irep < reps; irep++)
  {
    if(irep % (reps/10) == 0) printf(".");
    hash.seed_state_rand(r);

    for(int len = 4; len <= keymax; len++)
    {
      for(int offset = pad; offset < pad*2; offset++)
      {
        uint8_t * key1 = &buffer1[pad];
        uint8_t * key2 = &buffer2[pad+offset];

        r.rand_p(buffer1,buflen);
        r.rand_p(buffer2,buflen);

        memcpy(key2,key1,len);

        hash(key1,len,hash1);

        for(int bit = 0; bit < (len * 8); bit++)
        {
          // Flip a bit, hash the key -> we should get a different result.

          flipbit(key2,len,bit);
          hash(key2,len,hash2);
          count++;

          if(memcmp(hash1,hash2,hashbytes) == 0)
          {
            result = false;
            count_same ++;
          }

          // Flip it back, hash again -> we should get the original result.

          flipbit(key2,len,bit);
          hash(key2,len,hash2);

          if(memcmp(hash1,hash2,hashbytes) != 0)
          {
            count_inconsistent ++;
            result = false;
          }
        }
      }
    }
  }
  printf("\n"); // nl ok
  if (!ok(result,"SanityTest", hash.name()))
    printf("# from %d hashes there were %d inconsistent of %d collisions.\n",
        count, count_inconsistent, count_same);

  delete [] buffer1;
  delete [] buffer2;

  delete [] hash1;
  delete [] hash2;

  return result;
}

//----------------------------------------------------------------------------
// Appending zero bytes to a key should always cause it to produce a different
// hash value

template <typename hashtype>
bool AppendedZeroesTest ( hashfunc<hashtype> hash )
{
  printf("# Sanity check null suffixes change the hash (simple)");

  Rand r(173994);

  const int hashbytes = sizeof(hashtype);
  int failed = 0;

  for(int rep = 0; rep < 1000; rep++)
  {
    if(rep % 100 == 0) printf(".");

    /* Very crude test - check that a 32 byte random string
     * has a different hash compared to the same string suffixed
     * with 1 to 32 null bytes. */
    unsigned char key[256];

    memset(key,0,sizeof(key));

    hash.seed_state_rand(r);

    uint32_t h1[16];
    uint32_t h2[16];

    memset(h1,0,hashbytes);
    memset(h2,0,hashbytes);

    for(int i = 0; i < 32; i++)
    {
      hash(key,32+i,h1);

      if(i && memcmp(h1,h2,hashbytes) == 0)
      {
        failed++;
      }

      memcpy(h2,h1,hashbytes);
    }
    r.rand_p(key,32);
  }
  printf("\n"); // nl ok
  ok(failed == 0,"Appended Zeroes Test",hash.name());
  if (failed) printf("# There were %d collisions\n", failed);

  return failed == 0;
}

//-----------------------------------------------------------------------------
// Keyset 'Combination' - all possible combinations of input blocks
// used by the "permutation" tests in main.cpp via CombinationKeyTest
template< typename hashtype >
void CombinationKeygenRecurse ( uint32_t * key, int len, int maxlen, 
                  uint32_t * blocks, int blockcount, 
                hashfunc<hashtype> hash, std::vector<hashtype> & hashes)
{
  if(len == maxlen) return;

  for(int i = 0; i < blockcount; i++)
  {
    key[len] = blocks[i];
  
    //if(len == maxlen-1)
    {
      hashtype h;
      hash(key,(len+1) * sizeof(uint32_t),&h);
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
bool CombinationKeyTest ( hashfunc<hashtype> hash, int maxlen, uint32_t * blocks, int blockcount, bool testColl, double confidence, bool drawDiagram, Rand &r, const char *name )
{
  printf("# Keyset 'Combination' - up to %d blocks from a set of %d - ",maxlen,blockcount);

  //----------

  std::vector<hashtype> hashes;

  uint32_t * key = new uint32_t[maxlen];
  hash.seed_state_rand(r);

  CombinationKeygenRecurse<hashtype>(key,0,maxlen,blocks,blockcount,hash,hashes);

  delete [] key;

  printf("%d keys\n",(int)hashes.size());

  //----------

  bool result = true;

  result &= TestHashList<hashtype>(hashes,testColl,confidence,drawDiagram,name);
  
  return result;
}

//-----------------------------------------------------------------------------
// Keyset 'Sparse' - generate all possible N-bit keys with up to K bits set
// used by the Sparse tests in main.cpp

template < typename keytype, typename hashtype >
void SparseKeygenRecurse ( hashfunc<hashtype> hash, int start, int bitsleft, bool inclusive, keytype & k, std::vector<hashtype> & hashes, Rand &r )
{
  const int nbytes = sizeof(keytype);
  const int nbits = nbytes * 8;

  hashtype h;
  hash.seed_state_rand(r);

  for(int i = start; i < nbits; i++)
  {
    flipbit(&k,nbytes,i);

    if(inclusive || (bitsleft == 1))
    {
      hash(&k,sizeof(keytype),&h);
      hashes.push_back(h);
    }

    if(bitsleft > 1)
    {
      SparseKeygenRecurse(hash,i+1,bitsleft-1,inclusive,k,hashes,r);
    }

    flipbit(&k,nbytes,i);
  }
}

//----------
// used by the Sparse tests in main.cpp
template < int keybits, typename hashtype >
bool SparseKeyTest ( hashfunc<hashtype> hash, const int setbits, bool inclusive, bool testColl, double confidence, bool drawDiagram, Rand &r )
{
  char name[1024];
  snprintf(name,1024,"Keyset 'Sparse' - %d-bit keys with %s %d bits set",
      keybits, inclusive ? "up to" : "exactly", setbits);
  printf("# %s - ", name);

  typedef Blob<keybits> keytype;

  std::vector<hashtype> hashes;

  keytype k;
  memset(&k,0,sizeof(k));

  hash.seed_state_rand(r);

  if(inclusive)
  {
    hashtype h;

    hash(&k,sizeof(keytype),&h);

    hashes.push_back(h);
  }

  SparseKeygenRecurse(hash,0,setbits,inclusive,k,hashes,r);

  printf("%d keys\n",(int)hashes.size());

  bool result = true;
  
  result &= TestHashList<hashtype>(hashes,testColl,confidence,drawDiagram,name);

  return result;
}

//----------
// The purpose of this test is break hash functions which use the
// intel crc sse4 intrinsic _mm_crc32_u64() by generating an arbitrary
// set of keys composed of blocks which have the same CRC value. Any
// decent hash function should have no problem here, but one that tries
// to "optimize" by using crc will hit a wall. The blocks were found by
// brute forcing a set of colliding blocks, and just to make life interesting
// I used 4 different blocks, but two would do just as well. (You can brute
// force a collision after inspecting about 200k randomly selected blocks.)
// We then compose an arbitrary set of unique keys by permuting a set of the
// colliding blocks. The resulting keys have the same length, and will be
// unique, but when fed to a naive CRC based hash function will all produce
// the same hash value.
template < typename hashtype >
bool CrcCollisionKeyTest ( hashfunc<hashtype> hash, Rand &r )
{
  const int seeds= 10;
  const int num_key_blocks= 16;
  int count = 100 * 1000;
  int key_bytes= num_key_blocks * sizeof(uint64_t);
  uint64_t key[num_key_blocks];
  char name[1024];
  snprintf(name,1024,"Keyset 'CRC-MultiCollision' - %d x %d block keys - %d-bytes long",
      count, num_key_blocks, key_bytes);
  printf("# %s - %d seeds\n", name, seeds);
  int name_len= strlen(name);

  std::vector<hashtype> hashes;
  // The following blocks all have a crc of 57c58437
  const int num_src_blocks= 4;
  const int shift_bits= 2;
  const int mask= num_src_blocks - 1;
  const uint64_t blocks[num_src_blocks]= {
    0x4bb53c935d7bc565UL,
    0x53fa1f51857fa7f4UL,
    0x6caeaca38c4a8764UL,
    0xf9e0603f18749bf3UL,
  };
  // Alternatively we could use:
  // 5a476a7f 020d080728338f41 9a36026fdc10f1e0 c5e5a331ecf163dc c9f6d7755f81beca

  hashes.resize(count);

  bool result= true;

  uint32_t block_template = 1;

  for (int i= 0; i < seeds; i++) {
    hash.seed_state_rand(r);
    for(int j=0; j < count; j++) {
      uint64_t *cursor= &key[0];
      uint32_t template_bits = block_template;
      for (int c = 0; c < (sizeof(uint32_t)*8)/shift_bits; c++) {
        *cursor = blocks[template_bits & mask];
        //printf("%016lx\n",*cursor);
        cursor++;
        template_bits >>= shift_bits;
      }
      // marsaglia 32-bit permutation - we could use simple increment
      // also, but this makes the keys look "random" at a block level.
      block_template ^= block_template << 13;
      block_template ^= block_template >> 3;
      block_template ^= block_template << 17;

      // and then hash the result
      hash(&key[0],key_bytes,&hashes[j]);
      //printf("hash=%016lx",*((uint64_t*)&hashes[j]));
    }
    snprintf(name,1024,"Keyset 'CRC-MultiCollision' - seed %d # %s", i+1,hash.name());
    result &= TestHashList<hashtype>(hashes,true,false,false,name);
  }
  return result;
}

//-----------------------------------------------------------------------------
// Keyset 'Windows' - for all possible N-bit windows of a K-bit key, generate
// all possible keys with bits set in that window
// this is used by the Windowed tests in main.cpp, which are disabled by default
// and documented as less than useful.
template < typename keytype, typename hashtype >
bool WindowedKeyTest ( hashfunc<hashtype> hash, const int windowbits, bool testCollision, double confidence, bool drawDiagram, Rand &r )
{
  const int keybits = sizeof(keytype) * 8;
  const int keycount = 1 << windowbits;

  std::vector<hashtype> hashes;
  hashes.resize(keycount);

  bool result = true;

  int testcount = keybits;

  hash.seed_state_rand(r);

  printf("# Keyset 'Windowed' - %3d-bit key, %3d-bit window - %d tests, %d keys per test\n",
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

      hash(&key,sizeof(keytype),&hashes[i]);
    }

    char name[1024];
    snprintf(name,1024,"Window at %3d",j);

    result &= TestHashList(hashes,testCollision,confidence,drawDiagram,name);

  }

  return result;
}

//-----------------------------------------------------------------------------
// Keyset 'Cyclic' - generate keys that consist solely of N repetitions of M
// bytes.

// (This keyset type is designed to make MurmurHash2 fail)

template < typename hashtype >
bool CyclicKeyTest ( hashfunc<hashtype> hash, int cycleLen, int cycleReps, const int keycount, double confidence, bool drawDiagram, Rand &r )
{
  char name[1024];
  snprintf(name,1024,"Keyset 'Cyclic' - %d cycles of %d bytes - %d keys",
      cycleReps,cycleLen,keycount);
  printf("# %s\n",name);

  std::vector<hashtype> hashes;
  hashes.resize(keycount);

  int keyLen = cycleLen * cycleReps;

  uint8_t * cycle = new uint8_t[cycleLen + 16];
  uint8_t * key = new uint8_t[keyLen];

  hash.seed_state_rand(r);
  //----------

  for(int i = 0; i < keycount; i++)
  {
    r.rand_p(cycle,cycleLen);

    *(uint32_t*)cycle = f3mix(i ^ 0x746a94f1);

    for(int j = 0; j < keyLen; j++)
    {
      key[j] = cycle[j % cycleLen];
    }

    hash(key,keyLen,&hashes[i]);
  }

  //----------
  
  bool result = true;

  result &= TestHashList(hashes,true,confidence,drawDiagram,name);

  delete [] cycle;
  delete [] key;

  return result;
}

//-----------------------------------------------------------------------------
// Keyset 'TwoBytes' - generate all keys up to length N with two non-zero bytes

void TwoBytesKeygen ( int maxlen, KeyCallback & c, char *name );

template < typename hashtype >
bool TwoBytesTest2 ( hashfunc<hashtype> hash, int maxlen, double confidence, bool drawDiagram )
{
  char name[1024];
  std::vector<hashtype> hashes;

  HashCallback<hashtype> c(hash,hashes);

  TwoBytesKeygen(maxlen,c,name);

  bool result = true;

  result &= TestHashList(hashes,true,confidence,drawDiagram,name);

  return result;
}

//-----------------------------------------------------------------------------
// Keyset 'Text' - generate all keys of the form "prefix"+"core"+"suffix",
// where "core" consists of all possible combinations of the given character
// set of length N.

template < typename hashtype >
bool TextKeyTest ( hashfunc<hashtype> hash, const char * prefix, const char * coreset, const int corelen, const char * suffix, double confidence, bool drawDiagram, Rand &r )
{
  const int prefixlen = (int)strlen(prefix);
  const int suffixlen = (int)strlen(suffix);
  const int corecount = (int)strlen(coreset);

  const int keybytes = prefixlen + corelen + suffixlen;
  const int keycount = (int)pow(double(corecount),double(corelen));

  char name[1024];
  snprintf(name,1024,"Keyset 'Text' - keys of form \"%s[%.*s]%s\" - %d keys",
      prefix,corelen,"XXXXXXXXXXXXXXXX",suffix,keycount);
  printf("# %s\n",name);

  uint8_t * key = new uint8_t[keybytes+1];

  key[keybytes] = 0;

  memcpy(key,prefix,prefixlen);
  memcpy(key+prefixlen+corelen,suffix,suffixlen);

  //----------

  std::vector<hashtype> hashes;
  hashes.resize(keycount);

  hash.seed_state_rand(r);

  for(int i = 0; i < keycount; i++)
  {
    int t = i;

    for(int j = 0; j < corelen; j++)
    {
      key[prefixlen+j] = coreset[t % corecount]; t /= corecount;
    }

    hash(key,keybytes,&hashes[i]);
  }

  //----------

  bool result = true;

  result &= TestHashList(hashes,true,confidence,drawDiagram,name);

  delete [] key;

  return result;
}

//-----------------------------------------------------------------------------
// Keyset 'RepeatedCharkeyTest' - keys consisting of all the same char,
// differing only in length

// We reuse one block of empty bytes, otherwise the RAM cost is enormous.

template < typename hashtype >
bool RepeatedCharKeyTest ( hashfunc<hashtype> hash, const char *name, unsigned char c, int keycount, double confidence, bool drawDiagram, Rand &r )
{
  char fullname[1024];
  snprintf(fullname,1024,"Keyset '%s' - %d keys",name,keycount);
  printf("# %s\n", fullname);

  unsigned char * block = new unsigned char[keycount];
  memset(block,c,keycount);

  //----------

  std::vector<hashtype> hashes;

  hashes.resize(keycount);

  if (hash.seedbits() == 64) {
    uint64_t s= 0xffffffff9d66d03f;
    hash.seed_state(&s);
  } else {
    hash.seed_state_rand(r);
  }

  for(int i = 0; i < keycount; i++)
  {
    hash(block,i,&hashes[i]);
  }

  bool result = true;

  result &= TestHashList(hashes,true,confidence,drawDiagram,fullname);

  delete [] block;

  return result;
}

/* so we can sort an array of indexes into another array */
struct IndirectComparator
{
  const std::vector<uint8_t> & value_vector;
  int seedbytes;

  IndirectComparator(const std::vector<uint8_t> & val_vec, int sb):
    seedbytes(sb), value_vector(val_vec) {}

  bool operator()(int a, int b)
  {
    int c = memcmp(&value_vector[a * seedbytes],&value_vector[b * seedbytes], seedbytes);
    return c < 0 ? true : c == 0 ? a < b : false;
  }
};
//-----------------------------------------------------------------------------
// Keyset 'Seed' - hash "the quick brown fox..." using different seeds

template < typename hashtype >
bool SeedTest ( hashfunc<hashtype> hash, int count, double confidence, bool drawDiagram,
    Rand &seed_r, const char * text )
{
  char name[1024];
  snprintf(name,1024,"Keyset 'Seed' - %d seeds, Key \"%.20s\"%s",
          count, text, strlen(text)>20 ? "..." : "");
  printf("# %s\n",name);

  const int len = (int)strlen(text);
  //----------
  // All of this palaver is to dedupe the seeds.
  // We keep track of the seeds we used for each hash. Then
  // we sort the index array by the seeds, and then use that
  // to transcribe out the hashes in seed-order, while skipping
  // any dupes that are due to dupe seeds. Otherwise we end up
  // with ~ twice the expected error just because the RNG will
  // produce its own collisions. In most cases this does not matter
  // as the expected collisions are so small (far below zero for
  // a 64 bit seed/64 bit hash). But for small seeds and/or small
  // hashes the problem compounds.

  std::vector<hashtype> hashes;
  std::vector<hashtype> sorted_hashes;
  std::vector<uint8_t>  seeds;
  std::vector<int>      indexes;

  seeds.resize( count * hash.seedbytes() );
  sorted_hashes.resize(count);
  hashes.resize(count);
  indexes.resize(count);
  int seedbytes = hash.seedbytes();

  for(int i = 0; i < count; i++)
  {
    seed_r.rand_p(&seeds[ i * seedbytes ], seedbytes);
    hash(text, len, &seeds[i * seedbytes ], &hashes[i]);
    indexes[i]= i;
  }

  // sort the indexes by their seed
  std::sort(indexes.begin(),indexes.end(),IndirectComparator(seeds,seedbytes));

  // now loop through
  int dupes = 0;
  int unique = 0;
  sorted_hashes[unique++] = hashes[indexes[0]];
  for(size_t i = 1; i < indexes.size(); i++) {
    int c = memcmp(&seeds[indexes[i] * seedbytes],&seeds[indexes[i-1] * seedbytes],seedbytes);
    if ( c == 0 ) {
      dupes++;
    } else {
      sorted_hashes[unique++]= hashes[indexes[i]];
    }
  }
  sorted_hashes.resize(unique);

  bool result = true;

  result &= TestHashList<hashtype>(sorted_hashes,true,confidence,drawDiagram,name);

  return result;
}

//-----------------------------------------------------------------------------
/* vim: set sts=2 sw=2 et: */
