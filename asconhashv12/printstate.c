#ifdef ASCON_PRINTSTATE

#include "printstate.h"

#include <inttypes.h>
#include <stdio.h>

void printword(const char* text, const word_t x) {
  printf("%s=%016" PRIx64 "\n", text, WORDTOU64(x));
}

void printstate(const char* text, const state_t* s) {
  printf("%s:\n", text);
  printword("  x0", s->x0);
  printword("  x1", s->x1);
  printword("  x2", s->x2);
  printword("  x3", s->x3);
  printword("  x4", s->x4);
}

#endif
