// lookup3 by Bob Jekins, code is public domain.

#include "Platform.h"

#define rot(x,k) (((x)<<(k)) | ((x)>>(32-(k))))

#define mix(a,b,c) \
{ \
  a -= c;  a ^= rot(c, 4);  c += b; \
  b -= a;  b ^= rot(a, 6);  a += c; \
  c -= b;  c ^= rot(b, 8);  b += a; \
  a -= c;  a ^= rot(c,16);  c += b; \
  b -= a;  b ^= rot(a,19);  a += c; \
  c -= b;  c ^= rot(b, 4);  b += a; \
}

#define final(a,b,c) \
{ \
  c ^= b; c -= rot(b,14); \
  a ^= c; a -= rot(c,11); \
  b ^= a; b -= rot(a,25); \
  c ^= b; c -= rot(b,16); \
  a ^= c; a -= rot(c,4);  \
  b ^= a; b -= rot(a,14); \
  c ^= b; c -= rot(b,24); \
}

// objsize: 0x0-0x155: 341
uint32_t lookup3 ( const char * key, int length, uint32_t initval )
{
  uint32_t a,b,c;                                          /* internal state */

  a = b = c = 0xdeadbeef + ((uint32_t)length) + initval;

  const unsigned char *k = (const unsigned char *)key;

  /* Read complete words as little-endian bytes, regardless of alignment. */
  while (length > 12)
  {
    a += (uint32_t)k[0] | (uint32_t)k[1] << 8 | (uint32_t)k[2] << 16 | (uint32_t)k[3] << 24;
    b += (uint32_t)k[4] | (uint32_t)k[5] << 8 | (uint32_t)k[6] << 16 | (uint32_t)k[7] << 24;
    c += (uint32_t)k[8] | (uint32_t)k[9] << 8 | (uint32_t)k[10] << 16 | (uint32_t)k[11] << 24;
    mix(a,b,c);
    length -= 12;
    k += 12;
  }

  switch(length)
  {
    case 12: c += (uint32_t)k[11] << 24;
    case 11: c += (uint32_t)k[10] << 16;
    case 10: c += (uint32_t)k[9] << 8;
    case 9 : c += k[8];
    case 8 : b += (uint32_t)k[7] << 24;
    case 7 : b += (uint32_t)k[6] << 16;
    case 6 : b += (uint32_t)k[5] << 8;
    case 5 : b += k[4];
    case 4 : a += (uint32_t)k[3] << 24;
    case 3 : a += (uint32_t)k[2] << 16;
    case 2 : a += (uint32_t)k[1] << 8;
    case 1 : a += k[0]; break;
    case 0 : { return c; }              /* zero length strings require no mixing */
  }

  final(a,b,c);

  return c;
}
