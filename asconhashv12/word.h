#ifndef WORD_H_
#define WORD_H_

#include <stdint.h>

#include "endian.h"
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

forceinline uint64_t MASK(int n) {
  /* undefined for n == 0 */
  return ~0ull >> (64 - 8 * n);
}

forceinline word_t LOAD(const uint8_t* bytes, int n) {
  uint64_t x = *(uint64_t*)bytes & MASK(n);
  return U64BIG(x);
}

forceinline void STORE(uint8_t* bytes, word_t w, int n) {
  *(uint64_t*)bytes &= ~MASK(n);
  *(uint64_t*)bytes |= U64BIG(w);
}

forceinline word_t LOADBYTES(const uint8_t* bytes, int n) {
  uint64_t x = 0;
  for (int i = 0; i < n; ++i) ((uint8_t*)&x)[7 - i] = bytes[i];
  return x;
}

forceinline void STOREBYTES(uint8_t* bytes, word_t w, int n) {
  for (int i = 0; i < n; ++i) bytes[i] = ((uint8_t*)&w)[7 - i];
}

#endif /* WORD_H_ */
