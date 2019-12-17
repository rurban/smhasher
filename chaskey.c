/*
   Chaskey reference C implementation (speed optimized)

   Written in 2014 by Nicky Mouha, based on SipHash

   To the extent possible under law, the author has dedicated all copyright
   and related and neighboring rights to this software to the public domain
   worldwide. This software is distributed without any warranty.

   You should have received a copy of the CC0 Public Domain Dedication along with
   this software. If not, see <http://creativecommons.org/publicdomain/zero/1.0/>.
   
   NOTE: This implementation assumes a little-endian architecture
         that does not require aligned memory accesses.
   http://mouha.be/chaskey/
*/
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>

static uint32_t k[4] = { 0x833D3433, 0x009F389F, 0x2398E64F, 0x417ACF39 };
static uint32_t k1[4], k2[4];
const volatile uint32_t C[2] = { 0x00, 0x87 };

#define ROTL(x,b) (uint32_t)( ((x) >> (32 - (b))) | ( (x) << (b)) )

#define ROUND \
  do { \
    v[0] += v[1]; v[1]=ROTL(v[1], 5); v[1] ^= v[0]; v[0]=ROTL(v[0],16); \
    v[2] += v[3]; v[3]=ROTL(v[3], 8); v[3] ^= v[2]; \
    v[0] += v[3]; v[3]=ROTL(v[3],13); v[3] ^= v[0]; \
    v[2] += v[1]; v[1]=ROTL(v[1], 7); v[1] ^= v[2]; v[2]=ROTL(v[2],16); \
  } while(0)
  
#define PERMUTE \
  ROUND; \
  ROUND; \
  ROUND; \
  ROUND; \
  ROUND; \
  ROUND; \
  ROUND; \
  ROUND;

#define TIMESTWO(out,in) \
  do { \
    out[0] = (in[0] << 1) ^ C[in[3] >> 31]; \
    out[1] = (in[1] << 1) | (in[0] >> 31); \
    out[2] = (in[2] << 1) | (in[1] >> 31); \
    out[3] = (in[3] << 1) | (in[2] >> 31); \
  } while(0)
    
void subkeys(uint32_t k1[4], uint32_t k2[4], const uint32_t k[4]) {
  TIMESTWO(k1,k);
  TIMESTWO(k2,k1);
}

void chaskey(uint32_t *tag, uint32_t taglen, const uint8_t *m, const uint32_t mlen,
             const uint32_t k[4], const uint32_t k1[4], const uint32_t k2[4])
{
  const uint32_t *M = (uint32_t*) m;
  const uint32_t *end = M + (((mlen-1)>>4)<<2); /* pointer to last message block */

  const uint32_t *l;
  uint8_t lb[16];
  const uint32_t *lastblock;
  uint32_t v[4];
  
  int i;
  uint8_t *p;
  
  assert(taglen <= 16);

  v[0] = k[0];
  v[1] = k[1];
  v[2] = k[2];
  v[3] = k[3];
  /* rurban: added seeding support */
  if (taglen >= 16)
    v[3] ^= tag[3];
  if (taglen >= 12)
    v[2] ^= tag[2];
  if (taglen >= 8)
    v[1] ^= tag[1];
  if (taglen >= 4)
    v[0] ^= tag[0];

  if (mlen != 0) {
    for ( ; M != end; M += 4 ) {
#ifdef CHASKEY_DEBUG
      printf("(%3d) v[0] %08X\n", mlen, v[0]);
      printf("(%3d) v[1] %08X\n", mlen, v[1]);
      printf("(%3d) v[2] %08X\n", mlen, v[2]);
      printf("(%3d) v[3] %08X\n", mlen, v[3]);
      printf("(%3d) compress %08X %08X %08X %08X\n", mlen, m[0], m[1], m[2], m[3]);
#endif
      v[0] ^= M[0];
      v[1] ^= M[1];
      v[2] ^= M[2];
      v[3] ^= M[3];
      PERMUTE;
    }
  }

  if ((mlen != 0) && ((mlen & 0xF) == 0)) {
    l = k1;
    lastblock = M;
  } else {
    l = k2;
    p = (uint8_t*) M;
    i = 0;
    for ( ; p != m + mlen; p++,i++) {
      lb[i] = *p;
    }
    lb[i++] = 0x01; /* padding bit */
    for ( ; i != 16; i++) {
      lb[i] = 0;
    }
    lastblock = (uint32_t*) lb;
  }

#ifdef CHASKEY_DEBUG
  printf("(%3d) v[0] %08X\n", mlen, v[0]);
  printf("(%3d) v[1] %08X\n", mlen, v[1]);
  printf("(%3d) v[2] %08X\n", mlen, v[2]);
  printf("(%3d) v[3] %08X\n", mlen, v[3]);
  printf("(%3d) last block %08X %08X %08X %08X\n", mlen, lastblock[0], lastblock[1], lastblock[2], lastblock[3]);
#endif
  v[0] ^= lastblock[0];
  v[1] ^= lastblock[1];
  v[2] ^= lastblock[2];
  v[3] ^= lastblock[3];

  v[0] ^= l[0];
  v[1] ^= l[1];
  v[2] ^= l[2];
  v[3] ^= l[3];

  PERMUTE;

#ifdef CHASKEY_DEBUG
  printf("(%3d) v[0] %08X\n", mlen, v[0]);
  printf("(%3d) v[1] %08X\n", mlen, v[1]);
  printf("(%3d) v[2] %08X\n", mlen, v[2]);
  printf("(%3d) v[3] %08X\n", mlen, v[3]);
#endif

  v[0] ^= l[0];
  v[1] ^= l[1];
  v[2] ^= l[2];
  v[3] ^= l[3];

  memcpy(tag,v,taglen);

}

void
chaskey_init()
{
  /* key schedule */
  subkeys(k1,k2,k);
}

void
chaskey_c(const void *input, int len, uint64_t seed, void *out)
{
#if 0
  if (!len) {
    *(uint64_t *) out = 0;
    return;
  }
#endif
  chaskey((uint32_t *)&seed, 8, input, len, k, k1, k2);
  *(uint64_t *) out = seed;
}

#if 0
const uint32_t vectors[64][4] =
{
  { 0x792E8FE5, 0x75CE87AA, 0x2D1450B5, 0x1191970B },
  { 0x13A9307B, 0x50E62C89, 0x4577BD88, 0xC0BBDC18 },
  { 0x55DF8922, 0x2C7FF577, 0x73809EF4, 0x4E5084C0 },
  { 0x1BDBB264, 0xA07680D8, 0x8E5B2AB8, 0x20660413 },
  { 0x30B2D171, 0xE38532FB, 0x16707C16, 0x73ED45F0 },
  { 0xBC983D0C, 0x31B14064, 0x234CD7A2, 0x0C92BBF9 },
  { 0x0DD0688A, 0xE131756C, 0x94C5E6DE, 0x84942131 },
  { 0x7F670454, 0xF25B03E0, 0x19D68362, 0x9F4D24D8 },
  { 0x09330F69, 0x62B5DCE0, 0xA4FBA462, 0xF20D3C12 },
  { 0x89B3B1BE, 0x95B97392, 0xF8444ABF, 0x755DADFE },
  { 0xAC5B9DAE, 0x6CF8C0AC, 0x56E7B945, 0xD7ECF8F0 },
  { 0xD5B0DBEC, 0xC1692530, 0xD13B368A, 0xC0AE6A59 },
  { 0xFC2C3391, 0x285C8CD5, 0x456508EE, 0xC789E206 },
  { 0x29496F33, 0xAC62D558, 0xE0BAD605, 0xC5A538C6 },
  { 0xBF668497, 0x275217A1, 0x40C17AD4, 0x2ED877C0 },
  { 0x51B94DA4, 0xEFCC4DE8, 0x192412EA, 0xBBC170DD },
  { 0x79271CA9, 0xD66A1C71, 0x81CA474E, 0x49831CAD },
  { 0x048DA968, 0x4E25D096, 0x2D6CF897, 0xBC3959CA },
  { 0x0C45D380, 0x2FD09996, 0x31F42F3B, 0x8F7FD0BF },
  { 0xD8153472, 0x10C37B1E, 0xEEBDD61D, 0x7E3DB1EE },
  { 0xFA4CA543, 0x0D75D71E, 0xAF61E0CC, 0x0D650C45 },
  { 0x808B1BCA, 0x7E034DE0, 0x6C8B597F, 0x3FACA725 },
  { 0xC7AFA441, 0x95A4EFED, 0xC9A9664E, 0xA2309431 },
  { 0x36200641, 0x2F8C1F4A, 0x27F6A5DE, 0x469D29F9 },
  { 0x37BA1E35, 0x43451A62, 0xE6865591, 0x19AF78EE },
  { 0x86B4F697, 0x93A4F64F, 0xCBCBD086, 0xB476BB28 },
  { 0xBE7D2AFA, 0xAC513DE7, 0xFC599337, 0x5EA03E3A },
  { 0xC56D7F54, 0x3E286A58, 0x79675A22, 0x099C7599 },
  { 0x3D0F08ED, 0xF32E3FDE, 0xBB8A1A8C, 0xC3A3FEC4 },
  { 0x2EC171F8, 0x33698309, 0x78EFD172, 0xD764B98C },
  { 0x5CECEEAC, 0xA174084C, 0x95C3A400, 0x98BEE220 },
  { 0xBBDD0C2D, 0xFAB6FCD9, 0xDCCC080E, 0x9F04B41F },
  { 0x60B3F7AF, 0x37EEE7C8, 0x836CFD98, 0x782CA060 },
  { 0xDF44EA33, 0xB0B2C398, 0x0583CE6F, 0x846D823E },
  { 0xC7E31175, 0x6DB4E34D, 0xDAD60CA1, 0xE95ABA60 },
  { 0xE0DC6938, 0x84A0A7E3, 0xB7F695B5, 0xB46A010B },
  { 0x1CEB6C66, 0x3535F274, 0x839DBC27, 0x80B4599C },
  { 0xBBA106F4, 0xD49B697C, 0xB454B5D9, 0x2B69E58B },
  { 0x5AD58A39, 0xDFD52844, 0x34973366, 0x8F467DDC },
  { 0x67A67B1F, 0x3575ECB3, 0x1C71B19D, 0xA885C92B },
  { 0xD5ABCC27, 0x9114EFF5, 0xA094340E, 0xA457374B },
  { 0xB559DF49, 0xDEC9B2CF, 0x0F97FE2B, 0x5FA054D7 },
  { 0x2ACA7229, 0x99FF1B77, 0x156D66E0, 0xF7A55486 },
  { 0x565996FD, 0x8F988CEF, 0x27DC2CE2, 0x2F8AE186 },
  { 0xBE473747, 0x2590827B, 0xDC852399, 0x2DE46519 },
  { 0xF860AB7D, 0x00F48C88, 0x0ABFBB33, 0x91EA1838 },
  { 0xDE15C7E1, 0x1D90EFF8, 0xABC70129, 0xD9B2F0B4 },
  { 0xB3F0A2C3, 0x775539A7, 0x6CAA3BC1, 0xD5A6FC7E },
  { 0x127C6E21, 0x6C07A459, 0xAD851388, 0x22E8BF5B },
  { 0x08F3F132, 0x57B587E3, 0x087AD505, 0xFA070C27 },
  { 0xA826E824, 0x3F851E6A, 0x9D1F2276, 0x7962AD37 },
  { 0x14A6A13A, 0x469962FD, 0x914DB278, 0x3A9E8EC2 },
  { 0xFE20DDF7, 0x06505229, 0xF9C9F394, 0x4361A98D },
  { 0x1DE7A33C, 0x37F81C96, 0xD9B967BE, 0xC00FA4FA },
  { 0x5FD01E9A, 0x9F2E486D, 0x93205409, 0x814D7CC2 },
  { 0xE17F5CA5, 0x37D4BDD0, 0x1F408335, 0x43B6B603 },
  { 0x817CEEAE, 0x796C9EC0, 0x1BB3DED7, 0xBAC7263B },
  { 0xB7827E63, 0x0988FEA0, 0x3800BD91, 0xCF876B00 },
  { 0xF0248D4B, 0xACA7BDC8, 0x739E30F3, 0xE0C469C2 },
  { 0x67363EB6, 0xFAE8E047, 0xF0C1C8E5, 0x828CCD47 },
  { 0x3DBD1D15, 0x05092D7B, 0x216FC6E3, 0x446860FB },
  { 0xEBF39102, 0x8F4C1708, 0x519D2F36, 0xC67C5437 },
  { 0x89A0D454, 0x9201A282, 0xEA1B1E50, 0x1771BEDC },
  { 0x9047FAD7, 0x88136D8C, 0xA488286B, 0x7FE9352C }
};

int test_vectors() {
  uint8_t m[64];
  uint32_t tag[4];
  uint32_t k[4] = { 0x833D3433, 0x009F389F, 0x2398E64F, 0x417ACF39 };
  uint32_t k1[4], k2[4];
  int i;
  int ok = 1;
  uint32_t taglen = 16;

  /* key schedule */
  subkeys(k1,k2,k);
#ifdef CHASKEY_DEBUG
  printf("K0 %08X %08X %08X %08X\n", k[0], k[1], k[2], k[3]);
  printf("K1 %08X %08X %08X %08X\n", k1[0], k1[1], k1[2], k1[3]);
  printf("K2 %08X %08X %08X %08X\n", k2[0], k2[1], k2[2], k2[3]);  
#endif
  
  /* mac */
  for (i = 0; i < 64; i++) {
    m[i] = i;
    
    chaskey(tag, taglen, m, i, k, k1, k2);

    if (memcmp( tag, vectors[i], taglen )) {
      printf("test vector failed for %d-byte message\n", i);
      ok = 0;
    }
  }

  return ok;
}

int main() {
  if (test_vectors()) printf("test vectors ok\n");

  return 0;
}
#endif
