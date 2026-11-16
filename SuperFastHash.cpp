#include "Platform.h"

/* By Paul Hsieh (C) 2004, 2005.  Covered under the Paul Hsieh derivative 
   license. See: 
   http://www.azillionmonkeys.com/qed/weblicense.html for license details.

   http://www.azillionmonkeys.com/qed/hash.html */

FORCE_INLINE uint16_t get16bits ( const void * p )
{
  const uint8_t *bytes = (const uint8_t *)p;
  return (uint16_t)((uint16_t)bytes[0] | (uint16_t)bytes[1] << 8);
}

// objsize: 0-d2: 210
uint32_t SuperFastHash (const char * data, int len, uint32_t hash) {
  uint32_t tmp;
  int rem;

  if (len <= 0 || !data) return 0;

  rem = len & 3;
  len >>= 2;

  /* Main loop */
  for (;len > 0; len--) {
    hash  += get16bits (data);
    tmp    = (get16bits (data+2) << 11) ^ hash;
    hash   = (hash << 16) ^ tmp;
    data  += 2*sizeof (uint16_t);
    hash  += hash >> 11;
  }

  /* Legacy tails sign-extend the final byte, regardless of char signedness. */
  switch (rem) {
    case 3:	hash += get16bits (data);
        hash ^= hash << 16;
        hash ^= (((uint32_t)(uint8_t)data[2] ^ 0x80U) - 0x80U) << 18;
        hash += hash >> 11;
        break;
    case 2:	hash += get16bits (data);
        hash ^= hash << 11;
        hash += hash >> 17;
        break;
    case 1: hash += ((uint32_t)(uint8_t)*data ^ 0x80U) - 0x80U;
        hash ^= hash << 10;
        hash += hash >> 1;
  }

  /* Force "avalanching" of final 127 bits */
  hash ^= hash << 3;
  hash += hash >> 5;
  hash ^= hash << 4;
  hash += hash >> 17;
  hash ^= hash << 25;
  hash += hash >> 6;

  return hash;
}
