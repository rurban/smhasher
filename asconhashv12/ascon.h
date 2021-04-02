#ifndef ASCON_H_
#define ASCON_H_

#include <stdint.h>

#include "word.h"

typedef struct {
  word_t x0, x1, x2, x3, x4;
} state_t;

void ascon_init(state_t* s);
void ascon_absorb(state_t* s, const uint8_t* in, uint64_t inlen);
void ascon_squeeze(state_t* s, uint8_t* out, uint64_t outlen);

#endif /* ASCON_H */
