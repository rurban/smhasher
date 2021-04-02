#include "permutations.h"

#if !ASCON_INLINE_PERM && ASCON_UNROLL_LOOPS

void P12(state_t* s) { P12ROUNDS(s); }
void P8(state_t* s) { P8ROUNDS(s); }
void P6(state_t* s) { P6ROUNDS(s); }

#endif

#if !ASCON_INLINE_PERM && !ASCON_UNROLL_LOOPS

void P(state_t* s, int nr) { PROUNDS(s, nr); }

#endif
