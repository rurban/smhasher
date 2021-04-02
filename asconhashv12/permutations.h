#ifndef PERMUTATIONS_H_
#define PERMUTATIONS_H_

#include <stdint.h>

#include "api.h"
#include "ascon.h"
#include "config.h"
#include "printstate.h"
#include "round.h"

#define ASCON_128_KEYBYTES 16
#define ASCON_128A_KEYBYTES 16
#define ASCON_80PQ_KEYBYTES 20

#define ASCON_128_RATE 8
#define ASCON_128A_RATE 16

#define ASCON_128_PA_ROUNDS 12
#define ASCON_128_PB_ROUNDS 6
#define ASCON_128A_PB_ROUNDS 8

#define ASCON_HASH_BYTES 32

#define ASCON_128_IV WORD_T(0x80400c0600000000)
#define ASCON_128A_IV WORD_T(0x80800c0800000000)
#define ASCON_80PQ_IV WORD_T(0xa0400c0600000000)
#define ASCON_HASH_IV WORD_T(0x00400c0000000100)
#define ASCON_XOF_IV WORD_T(0x00400c0000000000)

#define ASCON_HASH_IV0 WORD_T(0xee9398aadb67f03dull)
#define ASCON_HASH_IV1 WORD_T(0x8bb21831c60f1002ull)
#define ASCON_HASH_IV2 WORD_T(0xb48a92db98d5da62ull)
#define ASCON_HASH_IV3 WORD_T(0x43189921b8f8e3e8ull)
#define ASCON_HASH_IV4 WORD_T(0x348fa5c9d525e140ull)

#define ASCON_XOF_IV0 WORD_T(0xb57e273b814cd416ull)
#define ASCON_XOF_IV1 WORD_T(0x2b51042562ae2420ull)
#define ASCON_XOF_IV2 WORD_T(0x66a3a7768ddf2218ull)
#define ASCON_XOF_IV3 WORD_T(0x5aad0a7a8153650cull)
#define ASCON_XOF_IV4 WORD_T(0x4f3e0e32539493b6ull)

#define START(n) ((3 + (n)) << 4 | (12 - (n)))
#define RC(c) WORD_T(c)

forceinline void P12ROUNDS(state_t* s) {
  ROUND(s, RC(0xf0));
  ROUND(s, RC(0xe1));
  ROUND(s, RC(0xd2));
  ROUND(s, RC(0xc3));
  ROUND(s, RC(0xb4));
  ROUND(s, RC(0xa5));
  ROUND(s, RC(0x96));
  ROUND(s, RC(0x87));
  ROUND(s, RC(0x78));
  ROUND(s, RC(0x69));
  ROUND(s, RC(0x5a));
  ROUND(s, RC(0x4b));
}

forceinline void P8ROUNDS(state_t* s) {
  ROUND(s, RC(0xb4));
  ROUND(s, RC(0xa5));
  ROUND(s, RC(0x96));
  ROUND(s, RC(0x87));
  ROUND(s, RC(0x78));
  ROUND(s, RC(0x69));
  ROUND(s, RC(0x5a));
  ROUND(s, RC(0x4b));
}

forceinline void P6ROUNDS(state_t* s) {
  ROUND(s, RC(0x96));
  ROUND(s, RC(0x87));
  ROUND(s, RC(0x78));
  ROUND(s, RC(0x69));
  ROUND(s, RC(0x5a));
  ROUND(s, RC(0x4b));
}

forceinline void PROUNDS(state_t* s, int nr) {
  for (int i = START(nr); i > 0x4a; i -= 0x0f) ROUND(s, RC(i));
}

#if ASCON_INLINE_PERM && ASCON_UNROLL_LOOPS

forceinline void P(state_t* s, int nr) {
  if (nr == 12) P12ROUNDS(s);
  if (nr == 8) P8ROUNDS(s);
  if (nr == 6) P6ROUNDS(s);
}

#elif !ASCON_INLINE_PERM && ASCON_UNROLL_LOOPS

void P12(state_t* s);
void P8(state_t* s);
void P6(state_t* s);

forceinline void P(state_t* s, int nr) {
  if (nr == 12) P12(s);
  if (nr == 8) P8(s);
  if (nr == 6) P6(s);
}

#elif ASCON_INLINE_PERM && !ASCON_UNROLL_LOOPS

forceinline void P(state_t* s, int nr) { PROUNDS(s, nr); }

#else /* !ASCON_INLINE_PERM && !ASCON_UNROLL_LOOPS */

void P(state_t* s, int nr);

#endif

#endif /* PERMUTATIONS_H_ */
