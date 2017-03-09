#include "KeysetTest.h"

#include "Platform.h"
#include "Random.h"

#include <map>
#include <set>


//-----------------------------------------------------------------------------
// Generate all keys of up to N bytes containing two non-zero bytes

void TwoBytesKeygen ( int maxlen, KeyCallback & c, char *name )
{
  //----------
  // Compute # of keys

  int keycount = 0;

  for(int i = 2; i <= maxlen; i++) keycount += (int)chooseK(i,2);

  keycount *= 255*255;

  for(int i = 2; i <= maxlen; i++) keycount += i*255;

  if (name)
    snprintf(name,1024,"Keyset 'TwoBytes' - up-to-%d-byte keys, %d total keys",
        maxlen, keycount);

  c.reserve(keycount);

  //----------
  // Add all keys with one non-zero byte

  uint8_t key[256];

  memset(key,0,256);

  for(int keylen = 2; keylen <= maxlen; keylen++)
  for(int byteA = 0; byteA < keylen; byteA++)
  {
    for(int valA = 1; valA <= 255; valA++)
    {
      key[byteA] = (uint8_t)valA;

      c(key,keylen);
    }

    key[byteA] = 0;
  }

  //----------
  // Add all keys with two non-zero bytes

  for(int keylen = 2; keylen <= maxlen; keylen++)
  for(int byteA = 0; byteA < keylen-1; byteA++)
  for(int byteB = byteA+1; byteB < keylen; byteB++)
  {
    for(int valA = 1; valA <= 255; valA++)
    {
      key[byteA] = (uint8_t)valA;

      for(int valB = 1; valB <= 255; valB++)
      {
        key[byteB] = (uint8_t)valB;
        c(key,keylen);
      }

      key[byteB] = 0;
    }

    key[byteA] = 0;
  }
}

//-----------------------------------------------------------------------------

template< typename hashtype >
void DumpCollisionMap ( CollisionMap<hashtype,ByteVec> & cmap )
{
  typedef CollisionMap<hashtype,ByteVec> cmap_t;

  for(typename cmap_t::iterator it = cmap.begin(); it != cmap.end(); ++it)
  {
    const hashtype & hash = (*it).first;

    printf("### Hash - ");
    printbytes(&hash,sizeof(hashtype));
    printf("\n"); // nl ok

    std::vector<ByteVec> & keys = (*it).second;

    for(int i = 0; i < (int)keys.size(); i++)
    {
      ByteVec & key = keys[i];

      printf("#   Key  - ");
      printbytes(&key[0],(int)key.size());
      printf("\n"); // nl ok
    }
  }

}

// test code
template<typename hashtype>
void ReportCollisions ( hashfunc<hashtype> hash )
{
  printf("# Hashing keyset\n");

  std::vector<uint128_t> hashes;

  HashCallback<uint128_t> c(hash,hashes);

  TwoBytesKeygen(20,c,NULL);

  printf("# %d hashes\n",(int)hashes.size());

  printf("# Finding collisions\n");

  HashSet<uint128_t> collisions;

  FindCollisions(hashes,collisions,1000);

  printf("# %d collisions\n",(int)collisions.size());

  printf("# Mapping collisions\n");

  CollisionMap<uint128_t,ByteVec> cmap;

  CollisionCallback<uint128_t> c2(hash,collisions,cmap);

  TwoBytesKeygen(20,c2,NULL);

  printf("# Dumping collisions\n");

  DumpCollisionMap(cmap);
}
/* vim: set sts=2 sw=2 et: */
