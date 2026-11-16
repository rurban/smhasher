#ifndef WORD_H_
#define WORD_H_

#include <stdint.h>

#include "forceinline.h"

typedef uint64_t word_t;

#define WORD_T
#define UINT64_T
#define U64TOWORD
#define WORDTOU64

forceinline word_t ROR(word_t x, int n) { return x >> n | x << (64 - n); }

forceinline word_t NOT(word_t a) { return ~a; }

forceinline word_t XOR(word_t a, word_t b) { return a ^ b; }

forceinline word_t AND(word_t a, word_t b) { return a & b; }

forceinline word_t KEYROT(word_t lo2hi, word_t hi2lo) {
  return lo2hi << 32 | hi2lo >> 32;
}

forceinline int NOTZERO(word_t a, word_t b) {
  uint64_t result = a | b;
  result |= result >> 32;
  result |= result >> 16;
  result |= result >> 8;
  return ((((int)(result & 0xff) - 1) >> 8) & 1) - 1;
}

forceinline word_t PAD(int i) { return 0x80ull << (56 - 8 * i); }

forceinline word_t CLEAR(word_t w, int n) {
  /* undefined for n == 0 */
  uint64_t mask = 0x00ffffffffffffffull >> (n * 8 - 8);
  return w & mask;
}

/* Ascon v1.2 absorbs and emits the most significant byte first. */
forceinline word_t LOAD(const uint8_t* bytes, int n) {
  word_t x = 0;
  for (int i = 0; i < n; ++i)
    x |= (word_t)bytes[i] << (56 - 8 * i);
  return x;
}

forceinline void STORE(uint8_t* bytes, word_t w, int n) {
  for (int i = 0; i < n; ++i)
    bytes[i] = (uint8_t)(w >> (56 - 8 * i));
}

forceinline word_t LOADBYTES(const uint8_t* bytes, int n) {
  return LOAD(bytes, n);
}

forceinline void STOREBYTES(uint8_t* bytes, word_t w, int n) {
  STORE(bytes, w, n);
}

#endif /* WORD_H_ */
