#ifndef CRC32C_H
#define CRC32C_H

#ifndef CRC32C_STATIC
#ifdef CRC32C_EXPORTS
#define CRC32C_API __declspec(dllexport)
#else
#define CRC32C_API __declspec(dllimport)
#endif
#else
#define CRC32C_API
#endif

#include <stdint.h>

/*
    Computes CRC-32C (Castagnoli) checksum. Uses Intel's CRC32 instruction if it is available.
    Otherwise it uses a very fast software fallback.
*/
extern "C" CRC32C_API uint32_t crc32c_append(
    uint32_t crc,               // Initial CRC value. Typically it's 0.
                                // You can supply non-trivial initial value here.
                                // Initial value can be used to chain CRC from multiple buffers.
    const uint8_t *input,       // Data to be put through the CRC algorithm.
    size_t length);             // Length of the data in the input buffer.

extern "C" CRC32C_API void crc32c_unittest();

#endif
