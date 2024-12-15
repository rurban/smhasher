/*
 * aesnihash from the PageBloomFilter project
 * Copyright (C) 2023  Frank J. T. Wojcik
 * Copyright (C) 2023  Ruan Kunliang
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 * notice, this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above
 * copyright notice, this list of conditions and the following
 * disclaimer in the documentation and/or other materials provided
 * with the distribution.
 *
 * 3. Neither the name of the copyright holder nor the names of its
 * contributors may be used to endorse or promote products derived from
 * this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 * COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT,
 * INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
 * OF THE POSSIBILITY OF SUCH DAMAGE.
 */
#include "Platform.h"

#if defined(HAVE_AESNI)
#include <immintrin.h>

void aesnihash_peterrk(const void * in, int len0, uint32_t seed, void * out) {
    auto a = _mm_set1_epi32((uint32_t)seed);
    auto b = _mm_set1_epi32((uint32_t)len0);
    auto m = _mm_set_epi32(0xdeadbeef, 0xffff0000, 0x01234567, 0x89abcdef);
    auto s = _mm_set_epi8(3, 7, 11, 15, 2, 6, 10, 14, 1, 5, 9, 13, 0, 4, 8, 12);
    const uint8_t * msg = (const uint8_t *)in;
    uint32_t        len = (uint32_t)len0;

    bool greed = (((uintptr_t)msg + (len - 1)) & UINT64_C(0xfff)) >= 15; // do not cross page

    if (len > 80) {
        auto c = _mm_aesenc_si128(b, m);
        auto d = _mm_aesdec_si128(a, m);
        a = _mm_aesenc_si128(a, m);
        b = _mm_aesdec_si128(b, m);
        do {
            a    = _mm_xor_si128(a, _mm_lddqu_si128((const __m128i *)msg));
            b    = _mm_xor_si128(b, _mm_lddqu_si128((const __m128i *)(msg + 16)));
            c    = _mm_xor_si128(c, _mm_lddqu_si128((const __m128i *)(msg + 32)));
            d    = _mm_xor_si128(d, _mm_lddqu_si128((const __m128i *)(msg + 48)));
            a    = _mm_shuffle_epi8(_mm_aesenc_si128(a, m), s);
            b    = _mm_shuffle_epi8(_mm_aesdec_si128(b, m), s);
            c    = _mm_shuffle_epi8(_mm_aesenc_si128(c, m), s);
            d    = _mm_shuffle_epi8(_mm_aesdec_si128(d, m), s);
            msg += 64;
            len -= 64;
        } while (len > 80);
        c = _mm_aesenc_si128(a, c);
        d = _mm_aesdec_si128(b, d);
        a = _mm_aesenc_si128(c, d);
        b = _mm_aesdec_si128(d, c);
    }

    auto mix = [&a, &b, m, s]( __m128i x ) {
                a = _mm_aesenc_si128(x, a);
                a = _mm_aesenc_si128(a, m);
                b = _mm_shuffle_epi8(_mm_xor_si128(x, b)   , s);
                b = _mm_shuffle_epi8(_mm_aesdec_si128(b, m), s);
            };

    while (len >= 16) {
        mix(_mm_lddqu_si128((const __m128i *)msg));
        msg += 16;
        len -= 16;
    }

    if (greed) {
#define GREEDILY_READ(n, addr) \
        _mm_bsrli_si128(_mm_bslli_si128(_mm_lddqu_si128((const __m128i*)addr), (16-(n))), (16-(n)))

        switch (len) {
        case 15: mix(GREEDILY_READ(15, msg)); break;
        case 14: mix(GREEDILY_READ(14, msg)); break;
        case 13: mix(GREEDILY_READ(13, msg)); break;
        case 12: mix(GREEDILY_READ(12, msg)); break;
        case 11: mix(GREEDILY_READ(11, msg)); break;
        case 10: mix(GREEDILY_READ(10, msg)); break;
        case  9: mix(GREEDILY_READ(9, msg)); break;
#ifndef _MSC_VER
        case  8: mix((__m128i)_mm_load_sd((const double *)msg)); break;
#else
        // MSVC cannot copy __m128d -> __m128i
        case  8: mix(GREEDILY_READ(7, msg)); break;
#endif
        case  7: mix(GREEDILY_READ(7, msg)); break;
        case  6: mix(GREEDILY_READ(6, msg)); break;
        case  5: mix(GREEDILY_READ(5, msg)); break;
#ifndef _MSC_VER
        case  4: mix((__m128i)_mm_load_ss((const float *)msg)); break;
#else
        // MSVC cannot copy __m128 -> __m128i
        case  4: mix(GREEDILY_READ(4, msg)); break;
#endif
        case  3: mix(GREEDILY_READ(3, msg)); break;
        case  2: mix(GREEDILY_READ(2, msg)); break;
        case  1: mix(GREEDILY_READ(1, msg)); break;
        case  0:
        default:     // try to keep m & s from register spilling
                 a = _mm_add_epi8(a, s);
                 b = _mm_add_epi8(b, m);
        }
  #undef GREEDILY_READ
    } else {
        uint64_t x = 0;
        switch (len) {
        case 15:
                 x |= ((uint64_t)msg[14]) << 48U; //FALLTHROUGH
        case 14:
                 x |= ((uint64_t)msg[13]) << 40U; //FALLTHROUGH
        case 13:
                 x |= ((uint64_t)msg[12]) << 32U; //FALLTHROUGH
        case 12:
                 x |= *(const uint32_t *)(msg + 8);
                 mix(_mm_set_epi64x(x, *(const uint64_t *)msg));
                 break;
        case 11:
                 x |= ((uint32_t)msg[10]) << 16U; //FALLTHROUGH
        case 10:
                 x |= ((uint32_t)msg[ 9]) <<  8U; //FALLTHROUGH
        case  9:
                 x |= msg[8];                     //FALLTHROUGH
        case  8:
                 mix(_mm_set_epi64x(x, *(const uint64_t *)msg));
                 break;
        case  7:
                 x |= ((uint64_t)msg[6]) << 48U; //FALLTHROUGH
        case  6:
                 x |= ((uint64_t)msg[5]) << 40U; //FALLTHROUGH
        case  5:
                 x |= ((uint64_t)msg[4]) << 32U; //FALLTHROUGH
        case  4:
                 x |= *(const uint32_t *)msg;
                 mix(_mm_set_epi64x(0, x));
                 break;
        case  3:
                 x |= ((uint32_t)msg[2]) << 16U; //FALLTHROUGH
        case  2:
                 x |= ((uint32_t)msg[1]) <<  8U; //FALLTHROUGH
        case  1:
                 x |= msg[0];
                 mix(_mm_set_epi64x(0, x));
                 break;
        case  0:
        default: // try to keep m & s from register spilling
                 a = _mm_add_epi8(a, s);
                 b = _mm_add_epi8(b, m);
        }
    }

    _mm_storeu_si128((__m128i *)out, _mm_aesenc_si128(a, b));
}

#endif

#ifdef smhasher3
//------------------------------------------------------------
REGISTER_FAMILY(aesnihash_peterrk,
   $.src_url    = "https://github.com/PeterRK/PageBloomFilter/blob/main/src/aesni-hash.h",
   $.src_status = HashFamilyInfo::SRC_ACTIVE
 );

REGISTER_HASH(aesnihash_peterrk,
   $.desc            = "AESNI Hash 128-bit for x64-only by PeterRK",
   $.impl            = "aesni",
   $.hash_flags      =
        FLAG_HASH_AES_BASED           |
        FLAG_HASH_SMALL_SEED          |
        FLAG_HASH_ENDIAN_INDEPENDENT,
   $.impl_flags      =
        FLAG_IMPL_READ_PAST_EOB       |
        FLAG_IMPL_CANONICAL_BOTH      |
        FLAG_IMPL_LICENSE_BSD,
   $.bits            = 128,
   $.verification_LE = 0xF06DA1B1,
   $.verification_BE = 0xF06DA1B1,
   $.hashfn_native   = aesnihash_peterrk,
   $.hashfn_bswap    = aesnihash_peterrk
 );
#endif
