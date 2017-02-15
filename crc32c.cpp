/*
  Copyright (c) 2013 - 2014 Mark Adler, Robert Vazan

  This software is provided 'as-is', without any express or implied
  warranty.  In no event will the author be held liable for any damages
  arising from the use of this software.

  Permission is granted to anyone to use this software for any purpose,
  including commercial applications, and to alter it and redistribute it
  freely, subject to the following restrictions:

  1. The origin of this software must not be misrepresented; you must not
  claim that you wrote the original software. If you use this software
  in a product, an acknowledgment in the product documentation would be
  appreciated but is not required.
  2. Altered source versions must be plainly marked as such, and must not be
  misrepresented as being the original software.
  3. This notice may not be removed or altered from any source distribution.
*/

#ifndef _CRT_SECURE_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS
#endif

#include "crc32c.h"

#define NOMINMAX
#include <windows.h>

#include <nmmintrin.h>
#include <stdio.h>

#include <random>
#include <algorithm>


typedef const uint8_t *buffer;


#include "generated-constants.cpp"


static uint32_t append_trivial(uint32_t crc, buffer input, size_t length)
{
    for (size_t i = 0; i < length; ++i)
    {
        crc = crc ^ input[i];
        for (int j = 0; j < 8; j++)
            crc = (crc >> 1) ^ 0x80000000 ^ ((~crc & 1) * POLY);
    }
    return crc;
}

/* Table-driven software version as a fall-back.  This is about 15 times slower
   than using the hardware instructions.  This assumes little-endian integers,
   as is the case on Intel processors that the assembler code here is for. */
static uint32_t append_adler_table(uint32_t crci, buffer input, size_t length)
{
    buffer next = input;
    uint64_t crc;

    crc = crci ^ 0xffffffff;
    while (length && ((uintptr_t)next & 7) != 0)
    {
        crc = table[0][(crc ^ *next++) & 0xff] ^ (crc >> 8);
        --length;
    }
    while (length >= 8)
    {
        crc ^= *(uint64_t *)next;
        crc = table[7][crc & 0xff]
            ^ table[6][(crc >> 8) & 0xff]
            ^ table[5][(crc >> 16) & 0xff]
            ^ table[4][(crc >> 24) & 0xff]
            ^ table[3][(crc >> 32) & 0xff]
            ^ table[2][(crc >> 40) & 0xff]
            ^ table[1][(crc >> 48) & 0xff]
            ^ table[0][crc >> 56];
        next += 8;
        length -= 8;
    }
    while (length)
    {
        crc = table[0][(crc ^ *next++) & 0xff] ^ (crc >> 8);
        --length;
    }
    return (uint32_t)crc ^ 0xffffffff;
}

/* Table-driven software version as a fall-back.  This is about 15 times slower
   than using the hardware instructions.  This assumes little-endian integers,
   as is the case on Intel processors that the assembler code here is for. */
static uint32_t append_table(uint32_t crci, buffer input, size_t length)
{
    buffer next = input;
#ifdef _M_X64
    uint64_t crc;
#else
    uint32_t crc;
#endif

    crc = crci ^ 0xffffffff;
#ifdef _M_X64
    while (length && ((uintptr_t)next & 7) != 0)
    {
        crc = table[0][(crc ^ *next++) & 0xff] ^ (crc >> 8);
        --length;
    }
    while (length >= 16)
    {
        crc ^= *(uint64_t *)next;
        uint64_t high = *(uint64_t *)(next + 8);
        crc = table[15][crc & 0xff]
            ^ table[14][(crc >> 8) & 0xff]
            ^ table[13][(crc >> 16) & 0xff]
            ^ table[12][(crc >> 24) & 0xff]
            ^ table[11][(crc >> 32) & 0xff]
            ^ table[10][(crc >> 40) & 0xff]
            ^ table[9][(crc >> 48) & 0xff]
            ^ table[8][crc >> 56]
            ^ table[7][high & 0xff]
            ^ table[6][(high >> 8) & 0xff]
            ^ table[5][(high >> 16) & 0xff]
            ^ table[4][(high >> 24) & 0xff]
            ^ table[3][(high >> 32) & 0xff]
            ^ table[2][(high >> 40) & 0xff]
            ^ table[1][(high >> 48) & 0xff]
            ^ table[0][high >> 56];
        next += 16;
        length -= 16;
    }
#else
    while (length && ((uintptr_t)next & 3) != 0)
    {
        crc = table[0][(crc ^ *next++) & 0xff] ^ (crc >> 8);
        --length;
    }
    while (length >= 12)
    {
        crc ^= *(uint32_t *)next;
        uint32_t high = *(uint32_t *)(next + 4);
        uint32_t high2 = *(uint32_t *)(next + 8);
        crc = table[11][crc & 0xff]
            ^ table[10][(crc >> 8) & 0xff]
            ^ table[9][(crc >> 16) & 0xff]
            ^ table[8][crc >> 24]
            ^ table[7][high & 0xff]
            ^ table[6][(high >> 8) & 0xff]
            ^ table[5][(high >> 16) & 0xff]
            ^ table[4][high >> 24]
            ^ table[3][high2 & 0xff]
            ^ table[2][(high2 >> 8) & 0xff]
            ^ table[1][(high2 >> 16) & 0xff]
            ^ table[0][high2 >> 24];
        next += 12;
        length -= 12;
    }
#endif
    while (length)
    {
        crc = table[0][(crc ^ *next++) & 0xff] ^ (crc >> 8);
        --length;
    }
    return (uint32_t)crc ^ 0xffffffff;
}

/* Apply the zeros operator table to crc. */
static inline uint32_t shift_crc(uint32_t shift_table[][256], uint32_t crc)
{
    return shift_table[0][crc & 0xff]
        ^ shift_table[1][(crc >> 8) & 0xff]
        ^ shift_table[2][(crc >> 16) & 0xff]
        ^ shift_table[3][crc >> 24];
}

/* Compute CRC-32C using the Intel hardware instruction. */
static uint32_t append_hw(uint32_t crc, buffer buf, size_t len)
{
    buffer next = buf;
    buffer end;
#ifdef _M_X64
    uint64_t crc0, crc1, crc2;      /* need to be 64 bits for crc32q */
#else
    uint32_t crc0, crc1, crc2;
#endif

    /* pre-process the crc */
    crc0 = crc ^ 0xffffffff;

    /* compute the crc for up to seven leading bytes to bring the data pointer
       to an eight-byte boundary */
    while (len && ((uintptr_t)next & 7) != 0)
    {
        crc0 = _mm_crc32_u8(static_cast<uint32_t>(crc0), *next);
        ++next;
        --len;
    }

#ifdef _M_X64
    /* compute the crc on sets of LONG_SHIFT*3 bytes, executing three independent crc
       instructions, each on LONG_SHIFT bytes -- this is optimized for the Nehalem,
       Westmere, Sandy Bridge, and Ivy Bridge architectures, which have a
       throughput of one crc per cycle, but a latency of three cycles */
    while (len >= 3 * LONG_SHIFT)
    {
        crc1 = 0;
        crc2 = 0;
        end = next + LONG_SHIFT;
        do
        {
            crc0 = _mm_crc32_u64(crc0, *reinterpret_cast<const uint64_t *>(next));
            crc1 = _mm_crc32_u64(crc1, *reinterpret_cast<const uint64_t *>(next + LONG_SHIFT));
            crc2 = _mm_crc32_u64(crc2, *reinterpret_cast<const uint64_t *>(next + 2 * LONG_SHIFT));
            next += 8;
        } while (next < end);
        crc0 = shift_crc(long_shifts, static_cast<uint32_t>(crc0)) ^ crc1;
        crc0 = shift_crc(long_shifts, static_cast<uint32_t>(crc0)) ^ crc2;
        next += 2 * LONG_SHIFT;
        len -= 3 * LONG_SHIFT;
    }

    /* do the same thing, but now on SHORT_SHIFT*3 blocks for the remaining data less
       than a LONG_SHIFT*3 block */
    while (len >= 3 * SHORT_SHIFT)
    {
        crc1 = 0;
        crc2 = 0;
        end = next + SHORT_SHIFT;
        do
        {
            crc0 = _mm_crc32_u64(crc0, *reinterpret_cast<const uint64_t *>(next));
            crc1 = _mm_crc32_u64(crc1, *reinterpret_cast<const uint64_t *>(next + SHORT_SHIFT));
            crc2 = _mm_crc32_u64(crc2, *reinterpret_cast<const uint64_t *>(next + 2 * SHORT_SHIFT));
            next += 8;
        } while (next < end);
        crc0 = shift_crc(short_shifts, static_cast<uint32_t>(crc0)) ^ crc1;
        crc0 = shift_crc(short_shifts, static_cast<uint32_t>(crc0)) ^ crc2;
        next += 2 * SHORT_SHIFT;
        len -= 3 * SHORT_SHIFT;
    }

    /* compute the crc on the remaining eight-byte units less than a SHORT_SHIFT*3
    block */
    end = next + (len - (len & 7));
    while (next < end)
    {
        crc0 = _mm_crc32_u64(crc0, *reinterpret_cast<const uint64_t *>(next));
        next += 8;
    }
#else
    /* compute the crc on sets of LONG_SHIFT*3 bytes, executing three independent crc
    instructions, each on LONG_SHIFT bytes -- this is optimized for the Nehalem,
    Westmere, Sandy Bridge, and Ivy Bridge architectures, which have a
    throughput of one crc per cycle, but a latency of three cycles */
    while (len >= 3 * LONG_SHIFT)
    {
        crc1 = 0;
        crc2 = 0;
        end = next + LONG_SHIFT;
        do
        {
            crc0 = _mm_crc32_u32(crc0, *reinterpret_cast<const uint32_t *>(next));
            crc1 = _mm_crc32_u32(crc1, *reinterpret_cast<const uint32_t *>(next + LONG_SHIFT));
            crc2 = _mm_crc32_u32(crc2, *reinterpret_cast<const uint32_t *>(next + 2 * LONG_SHIFT));
            next += 4;
        } while (next < end);
        crc0 = shift_crc(long_shifts, static_cast<uint32_t>(crc0)) ^ crc1;
        crc0 = shift_crc(long_shifts, static_cast<uint32_t>(crc0)) ^ crc2;
        next += 2 * LONG_SHIFT;
        len -= 3 * LONG_SHIFT;
    }

    /* do the same thing, but now on SHORT_SHIFT*3 blocks for the remaining data less
    than a LONG_SHIFT*3 block */
    while (len >= 3 * SHORT_SHIFT)
    {
        crc1 = 0;
        crc2 = 0;
        end = next + SHORT_SHIFT;
        do
        {
            crc0 = _mm_crc32_u32(crc0, *reinterpret_cast<const uint32_t *>(next));
            crc1 = _mm_crc32_u32(crc1, *reinterpret_cast<const uint32_t *>(next + SHORT_SHIFT));
            crc2 = _mm_crc32_u32(crc2, *reinterpret_cast<const uint32_t *>(next + 2 * SHORT_SHIFT));
            next += 4;
        } while (next < end);
        crc0 = shift_crc(short_shifts, static_cast<uint32_t>(crc0)) ^ crc1;
        crc0 = shift_crc(short_shifts, static_cast<uint32_t>(crc0)) ^ crc2;
        next += 2 * SHORT_SHIFT;
        len -= 3 * SHORT_SHIFT;
    }

    /* compute the crc on the remaining eight-byte units less than a SHORT_SHIFT*3
    block */
    end = next + (len - (len & 7));
    while (next < end)
    {
        crc0 = _mm_crc32_u32(crc0, *reinterpret_cast<const uint32_t *>(next));
        next += 4;
    }
#endif
    len &= 7;

    /* compute the crc for up to seven trailing bytes */
    while (len)
    {
        crc0 = _mm_crc32_u8(static_cast<uint32_t>(crc0), *next);
        ++next;
        --len;
    }

    /* return a post-processed crc */
    return static_cast<uint32_t>(crc0) ^ 0xffffffff;
}

static bool detect_hw()
{
    int info[4];
    __cpuid(info, 1);
    return (info[2] & (1 << 20)) != 0;
}

static bool hw_available = detect_hw();

extern "C" CRC32C_API uint32_t crc32c_append(uint32_t crc, buffer input, size_t length)
{
    if (hw_available)
        return append_hw(crc, input, length);
    else
        return append_table(crc, input, length);
}

#define TEST_BUFFER 65536
#define TEST_SLICES 1000000

static int benchmark(const char *name, uint32_t(*function)(uint32_t, buffer, size_t), buffer input, int *offsets, int *lengths, uint32_t *crcs)
{
    uint64_t startTime = GetTickCount64();
    int slice = 0;
    uint64_t totalBytes = 0;
    bool first = true;
    int iterations = 0;
    uint32_t crc = 0;
    while (GetTickCount64() - startTime < 1000)
    {
        crc = function(crc, input + offsets[slice], lengths[slice]);
        totalBytes += lengths[slice];
        if (first)
            crcs[slice] = crc;
        ++slice;
        ++iterations;
        if (slice == TEST_SLICES)
        {
            slice = 0;
            first = false;
        }
    }
    int time = static_cast<int>(GetTickCount64() - startTime);
    double throughput = totalBytes * 1000.0 / time;
    printf("%s: ", name);
    if (throughput > 1024.0 * 1024.0 * 1024.0)
        printf("%.1f GB/s\n", throughput / 1024 / 1024 / 1024);
    else
        printf("%.0f MB/s\n", throughput / 1024 / 1024);
    return std::min(TEST_SLICES, iterations);
}

static void compare_crcs(const char *leftName, uint32_t *left, const char *rightName, uint32_t *right, int count)
{
    for (int i = 0; i < count; ++i)
        if (left[i] != right[i])
        {
            printf("CRC mismatch between algorithms %s and %s at offset %d: %x vs %x\n", leftName, rightName, i, left[i], right[i]);
            exit(1);
        }
}

extern "C" CRC32C_API void crc32c_unittest()
{
    std::random_device rd;
    std::uniform_int_distribution<int> byteDist(0, 255);
    uint8_t *input = new uint8_t[TEST_BUFFER];
    for (int i = 0; i < TEST_BUFFER; ++i)
        input[i] = byteDist(rd);
    int *offsets = new int[TEST_SLICES];
    int *lengths = new int[TEST_SLICES];
    std::uniform_int_distribution<int> lengthDist(0, TEST_BUFFER);
    for (int i = 0; i < TEST_SLICES; ++i)
    {
        lengths[i] = lengthDist(rd);
        std::uniform_int_distribution<int> offsetDist(0, TEST_BUFFER - lengths[i]);
        offsets[i] = offsetDist(rd);
    }
    uint32_t *crcsTrivial = new uint32_t[TEST_SLICES];
    uint32_t *crcsAdlerTable = new uint32_t[TEST_SLICES];
    uint32_t *crcsTable = new uint32_t[TEST_SLICES];
    uint32_t *crcsHw = new uint32_t[TEST_SLICES];
    int iterationsTrivial = benchmark("trivial", append_trivial, input, offsets, lengths, crcsTrivial);
    int iterationsAdlerTable = benchmark("adler_table", append_adler_table, input, offsets, lengths, crcsAdlerTable);
    compare_crcs("trivial", crcsTrivial, "adler_table", crcsAdlerTable, std::min(iterationsTrivial, iterationsAdlerTable));
    int iterationsTable = benchmark("table", append_table, input, offsets, lengths, crcsTable);
    compare_crcs("adler_table", crcsAdlerTable, "table", crcsTable, std::min(iterationsAdlerTable, iterationsTable));
    if (hw_available)
    {
        int iterationsHw = benchmark("hw", append_hw, input, offsets, lengths, crcsHw);
        compare_crcs("table", crcsTable, "hw", crcsHw, std::min(iterationsTable, iterationsHw));
    }
    else
        printf("HW doesn't have crc instruction\n");
    benchmark("auto", crc32c_append, input, offsets, lengths, crcsHw);
}
