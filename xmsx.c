/*
 * SPDX-FileCopyrightText: 2023 Dmitrii Lebed <lebed.dmitry@gmail.com>
 * SPDX-License-Identifier: BSD-2-Clause
 */

#include "xmsx.h"
#include <string.h>

static uint64_t xmsx32_round(uint64_t h, uint32_t d)
{
    const uint64_t p = 0xcdb32970830fcaa1ULL;

    h = (h ^ d) * p;
    h ^= h >> 32;

    return h;
}

uint32_t xmsx32(const void *buf, size_t len, uint32_t seed)
{
    const unsigned char *data = (const unsigned char *)buf;
    uint64_t h = ((uint64_t)seed << 32) | seed;

    h = xmsx32_round(h, len);

    while (len) {
        uint32_t d;
        const size_t word_size = sizeof(d);

        memcpy(&d, data, sizeof(d));

        if (len < word_size) {
            const size_t bits_to_clear = 8 * (word_size - len);

            d <<= bits_to_clear;
            d >>= bits_to_clear;
            len = word_size;
        }

        h = xmsx32_round(h, d);

        len -= word_size;
        data += word_size;
    }

    return xmsx32_round(h, h >> 47);
}
