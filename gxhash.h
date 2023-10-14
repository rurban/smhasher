#ifndef GXHASH_H
#define GXHASH_H

#include <stdint.h>

#if defined (__cplusplus)
extern "C" {
#endif
uint32_t gxhash32(const uint8_t* input, int len, uint32_t seed);
uint64_t gxhash64(const uint8_t* input, int len, uint32_t seed);
#if defined (__cplusplus)
}
#endif

#endif // GXHASH_H
