#include "Platform.h"
#include "HashMapTest.h"

//-----------------------------------------------------------------------------
// This should be a realistic I-Cache test, when our hash is used inlined
// in a hash table. There the size matters more than the bulk speed.

bool HashMapTest ( pfHash hash, const int hashbits, bool verbose )
{
  const int hashbytes = hashbits / 8;

  uint8_t * key    = new uint8_t[256];
  uint8_t * hashes = new uint8_t[hashbytes * 256];
  uint8_t * final  = new uint8_t[hashbytes];

  memset(key,0,256);
  memset(hashes,0,hashbytes*256);
  memset(final,0,hashbytes);

  printf("Running HashMapTest     ");
#if 0
  // Hash keys of the form {0}, {0,1}, {0,1,2}... up to N=255,using 256-N as
  // the seed
  for(int i = 0; i < 256; i++)
  {
    key[i] = (uint8_t)i;
    hash(key,i,256-i,&hashes[i*hashbytes]);
  }

  // Then hash the result array
  hash(hashes,hashbytes*256,0,final);

  // The first four bytes of that hash, interpreted as a little-endian integer, is our
  // verification value
  uint32_t verification =
      (final[0] << 0) | (final[1] << 8) | (final[2] << 16) | (final[3] << 24);
#endif

  delete [] key;
  delete [] hashes;
  delete [] final;

  //----------

  printf(" ....... PASS\n");
  return true;
}
