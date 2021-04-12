/* Compile with gcc -O3 -msse4.2 ... */

#include <stdint.h>
#if defined(__aarch64__)
#include "sse2neon.h"
#else
#include <smmintrin.h>
#endif

// Byte-boundary alignment issues
#define ALIGN_SIZE      0x08UL
#define ALIGN_MASK      (ALIGN_SIZE - 1)
#define CALC_CRC(op, crc, type, buf, len)                               \
  do {                                                                  \
    for (; (len) >= sizeof (type); (len) -= sizeof(type), buf += sizeof (type)) { \
      (crc) = op((crc), *(type *) (buf));                               \
    }                                                                   \
  } while(0)


/* Compute CRC-32C using the Intel hardware instruction. */
/* for better parallelization with bigger buffers see 
   http://www.drdobbs.com/parallel/fast-parallelized-crc-computation-using/229401411 */
uint32_t crc32c_hw(const void *input, int len, uint32_t crc)
{
    const char* buf = (const char*)input;

    // XOR the initial CRC with INT_MAX
    crc ^= 0xFFFFFFFF;

    // Align the input to the word boundary
    for (; (len > 0) && ((size_t)buf & ALIGN_MASK); len--, buf++) {
        crc = _mm_crc32_u8(crc, *buf);
    }

    // Blast off the CRC32 calculation
#if defined(__x86_64__) || defined(__aarch64__)
    CALC_CRC(_mm_crc32_u64, crc, uint64_t, buf, len);
#endif
    CALC_CRC(_mm_crc32_u32, crc, uint32_t, buf, len);
    CALC_CRC(_mm_crc32_u16, crc, uint16_t, buf, len);
    CALC_CRC(_mm_crc32_u8, crc, uint8_t, buf, len);

    // Post-process the crc
    return (crc ^ 0xFFFFFFFF);
}

uint64_t crc64c_hw(const void *input, int len, uint32_t seed)
{
    const char* buf = (const char*)input;
    uint64_t crc = (uint64_t)seed;

    // Align the input to the word boundary
    for (; (len > 0) && ((size_t)buf & ALIGN_MASK); len--, buf++) {
        crc = _mm_crc32_u8(crc, *buf);
    }

    // Blast off the CRC32 calculation
#if defined(__x86_64__) || defined(__aarch64__)
    CALC_CRC(_mm_crc32_u64, crc, uint64_t, buf, len);
#endif
    CALC_CRC(_mm_crc32_u32, crc, uint32_t, buf, len);
    CALC_CRC(_mm_crc32_u16, crc, uint16_t, buf, len);
    CALC_CRC(_mm_crc32_u8, crc, uint8_t, buf, len);

    // Post-process the crc
    return crc;
}

