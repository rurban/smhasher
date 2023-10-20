/*
 * SPDX-FileCopyrightText: 2023 Dmitrii Lebed <lebed.dmitry@gmail.com>
 * SPDX-License-Identifier: BSD-2-Clause
 */

/*
 * XMSX (XOR - Multiply - Shift - XOR) Hash
 * Inspired by MUM and Murmur hashes
 *
 * Design inputs:
 *   - be faster than SW CRC32 on modern 32-bit CPUs (and microcontrollers)
 *      (supporting HW 32bx32b->64b multiplication)
 *   - be as simple as possible (small code size)
 *   - try to reuse the same round function (xor-mul-shift-xor)
 *   - provide reasonable hashing quality (pass SMHasher tests)
 * XMSX32 passes all SMHasher tests (2 bad seeds)
 */

#ifndef XMSX_H
#define XMSX_H

#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif


uint32_t xmsx32(const void *buf, size_t len, uint32_t seed);


#ifdef __cplusplus
};
#endif

#endif /* XMSX_H */
