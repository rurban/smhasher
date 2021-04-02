#ifndef ROUND_H_
#define ROUND_H_

#include "ascon.h"
#include "printstate.h"

forceinline void KINIT(word_t* K0, word_t* K1, word_t* K2) {
  *K0 = WORD_T(0);
  *K1 = WORD_T(0);
  *K2 = WORD_T(0);
}

forceinline void PINIT(state_t* s) {
  s->x0 = WORD_T(0);
  s->x1 = WORD_T(0);
  s->x2 = WORD_T(0);
  s->x3 = WORD_T(0);
  s->x4 = WORD_T(0);
}

forceinline void ROUND(state_t* s, word_t C) {
  state_t t;
  /* round constant */
  s->x2 = XOR(s->x2, C);
  /* s-box layer */
  s->x0 = XOR(s->x0, s->x4);
  s->x4 = XOR(s->x4, s->x3);
  s->x2 = XOR(s->x2, s->x1);
  t.x0 = XOR(s->x0, AND(NOT(s->x1), s->x2));
  t.x2 = XOR(s->x2, AND(NOT(s->x3), s->x4));
  t.x4 = XOR(s->x4, AND(NOT(s->x0), s->x1));
  t.x1 = XOR(s->x1, AND(NOT(s->x2), s->x3));
  t.x3 = XOR(s->x3, AND(NOT(s->x4), s->x0));
  t.x1 = XOR(t.x1, t.x0);
  t.x3 = XOR(t.x3, t.x2);
  t.x0 = XOR(t.x0, t.x4);
  /* linear layer */
  s->x2 = XOR(t.x2, ROR(t.x2, 6 - 1));
  s->x3 = XOR(t.x3, ROR(t.x3, 17 - 10));
  s->x4 = XOR(t.x4, ROR(t.x4, 41 - 7));
  s->x0 = XOR(t.x0, ROR(t.x0, 28 - 19));
  s->x1 = XOR(t.x1, ROR(t.x1, 61 - 39));
  s->x2 = XOR(t.x2, ROR(s->x2, 1));
  s->x3 = XOR(t.x3, ROR(s->x3, 10));
  s->x4 = XOR(t.x4, ROR(s->x4, 7));
  s->x0 = XOR(t.x0, ROR(s->x0, 19));
  s->x1 = XOR(t.x1, ROR(s->x1, 39));
  s->x2 = NOT(s->x2);
  printstate(" round output", s);
}

#endif /* ROUND_H_ */
