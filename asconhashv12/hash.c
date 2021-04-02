#include "api.h"
#include "ascon.h"
#include "crypto_hash.h"
#include "permutations.h"
#include "printstate.h"

#if !ASCON_INLINE_MODE
#undef forceinline
#define forceinline
#endif

forceinline void ascon_init(state_t* s) {
  /* initialize */
#ifdef ASCON_HASH
  s->x0 = ASCON_HASH_IV0;
  s->x1 = ASCON_HASH_IV1;
  s->x2 = ASCON_HASH_IV2;
  s->x3 = ASCON_HASH_IV3;
  s->x4 = ASCON_HASH_IV4;
#endif
#ifdef ASCON_XOF
  s->x0 = ASCON_XOF_IV0;
  s->x1 = ASCON_XOF_IV1;
  s->x2 = ASCON_XOF_IV2;
  s->x3 = ASCON_XOF_IV3;
  s->x4 = ASCON_XOF_IV4;
#endif
  printstate("initialization", s);
}

forceinline void ascon_absorb(state_t* s, const uint8_t* in, uint64_t inlen) {
  /* absorb full plaintext blocks */
  while (inlen >= ASCON_RATE) {
    s->x0 = XOR(s->x0, LOAD(in, 8));
    P(s, 12);
    in += ASCON_RATE;
    inlen -= ASCON_RATE;
  }
  /* absorb final plaintext block */
  if (inlen) s->x0 = XOR(s->x0, LOAD(in, inlen));
  s->x0 = XOR(s->x0, PAD(inlen));
  P(s, 12);
  printstate("absorb plaintext", s);
}

forceinline void ascon_squeeze(state_t* s, uint8_t* out, uint64_t outlen) {
  /* squeeze full output blocks */
  while (outlen > ASCON_RATE) {
    STORE(out, s->x0, 8);
    P(s, 12);
    out += ASCON_RATE;
    outlen -= ASCON_RATE;
  }
  /* squeeze final output block */
  STORE(out, s->x0, outlen);
  printstate("squeeze output", s);
}

int crypto_hash(unsigned char* out, const unsigned char* in,
                unsigned long long inlen) {
  state_t s;
  ascon_init(&s);
  ascon_absorb(&s, in, inlen);
  ascon_squeeze(&s, out, CRYPTO_BYTES);
  return 0;
}

/// smhasher interfaces
void asconhashv12_64 ( const void * key, int len, uint32_t seed, void * out ) {
  state_t s;
  ascon_init(&s);
  s.x0 ^= seed;
  ascon_absorb(&s, key, (unsigned long long)len);
  ascon_squeeze(&s, (unsigned char*)out, 8 /*CRYPTO_BYTES*/);
}
void asconhashv12_256 ( const void * key, int len, uint32_t seed, void * out ) {
  state_t s;
  ascon_init(&s);
  s.x0 ^= seed;
  ascon_absorb(&s, key, (unsigned long long)len);
  ascon_squeeze(&s, (unsigned char*)out, CRYPTO_BYTES);
}
