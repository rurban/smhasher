/*
 * Rainbow hash function - 256-bit internal state, 128-bit input chunks, up to 256-bit output
 * Stream based
 * Can also be utilized as an eXtensible Output Function (XOF).
 *
 * Copyright (C) 2023 Cris Stringfellow (and DOSYAGO)
 *
 * Rainstorm hash is licensed under Apache-2.0
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#include "Platform.h"
#ifdef smhasher3
#include "Hashlib.h"
#endif

// P to W are primes chosen for their excellent avalanche properties
static const uint64_t P = UINT64_C(  0xFFFFFFFFFFFFFFFF) - 58;
static const uint64_t Q = UINT64_C(13166748625691186689);
static const uint64_t R = UINT64_C( 1573836600196043749);
static const uint64_t S = UINT64_C( 1478582680485693857);
static const uint64_t T = UINT64_C( 1584163446043636637);
static const uint64_t U = UINT64_C( 1358537349836140151);
static const uint64_t V = UINT64_C( 2849285319520710901);
static const uint64_t W = UINT64_C( 2366157163652459183);

static inline void mixA( uint64_t * s ) {
    uint64_t a = s[0], b = s[1], c = s[2], d = s[3];

    a *= P;
    a  = ROTR64(a, 23);
    a *= Q;

    b ^= a;

    b *= R;
    b  = ROTR64(b, 29);
    b *= S;

    c *= T;
    c  = ROTR64(c, 31);
    c *= U;

    d ^= c;

    d *= V;
    d  = ROTR64(d, 37);
    d *= W;

    s[0] = a; s[1] = b; s[2] = c; s[3] = d;
}

static inline void mixB( uint64_t * s, uint64_t iv ) {
    uint64_t a = s[1], b = s[2];

    a *= V;
    a  = ROTR64(a, 23);
    a *= W;

    b ^= a + iv;

    b *= R;
    b  = ROTR64(b, 23);
    b *= S;

    s[1] = a; s[2] = b;
}

template <uint32_t hashsize>
static void rainbow( const void * in, const size_t olen, const seed_t seed, void * out ) {
    const uint8_t * data  = (const uint8_t *)in;
    uint64_t        h[4]  = { seed + olen + 1, seed + olen + 3, seed + olen + 5, seed + olen + 7 };
    size_t          len   = olen;
    uint64_t        g     = 0;
    bool            inner = 0;

    while (len >= 16) {
        g     = GET_U64(data, 0);

        h[0] -= g;
        h[1] += g;

        data += 8;

        g     = GET_U64(data, 0);

        h[2] += g;
        h[3] -= g;

        if (inner) {
            mixB(h, seed);
        } else {
            mixA(h);
        }
        inner ^= 1;

        data  += 8;
        len   -= 16;
    }

    mixB(h, seed);

    switch (len) {
    case 15: h[0] += (uint64_t)data[14] << 56; // FALLTHROUGH
    case 14: h[1] += (uint64_t)data[13] << 48; // FALLTHROUGH
    case 13: h[2] += (uint64_t)data[12] << 40; // FALLTHROUGH
    case 12: h[3] += (uint64_t)data[11] << 32; // FALLTHROUGH
    case 11: h[0] += (uint64_t)data[10] << 24; // FALLTHROUGH
    case 10: h[1] += (uint64_t)data[ 9] << 16; // FALLTHROUGH
    case  9: h[2] += (uint64_t)data[ 8] <<  8; // FALLTHROUGH
    case  8: h[3] +=           data[ 7];       // FALLTHROUGH
    case  7: h[0] += (uint64_t)data[ 6] << 48; // FALLTHROUGH
    case  6: h[1] += (uint64_t)data[ 5] << 40; // FALLTHROUGH
    case  5: h[2] += (uint64_t)data[ 4] << 32; // FALLTHROUGH
    case  4: h[3] += (uint64_t)data[ 3] << 24; // FALLTHROUGH
    case  3: h[0] += (uint64_t)data[ 2] << 16; // FALLTHROUGH
    case  2: h[1] += (uint64_t)data[ 1] <<  8; // FALLTHROUGH
    case  1: h[2] += (uint64_t)data[ 0];
    }

    mixA(h);
    mixB(h, seed);
    mixA(h);

    g  = 0;
    g -= h[2];
    g -= h[3];

    PUT_U64(g, (uint8_t *)out, 0);
    if (hashsize == 128) {
        mixA(h);
        g  = 0;
        g -= h[3];
        g -= h[2];
        PUT_U64(g, (uint8_t *)out, 8);
    } else if (hashsize == 256) {
        mixA(h);
        g  = 0;
        g -= h[3];
        g -= h[2];
        PUT_U64(g, (uint8_t *)out, 8);
        mixA(h);
        mixB(h, seed);
        mixA(h);
        g  = 0;
        g -= h[3];
        g -= h[2];
        PUT_U64(g, (uint8_t *)out, 16);
        mixA(h);
        g  = 0;
        g -= h[3];
        g -= h[2];
        PUT_U64(g, (uint8_t *)out, 24);
    }
}

#ifdef smhasher3
REGISTER_FAMILY(rainbow,
   $.src_url    = "https://github.com/dosyago/rain",
   $.src_status = HashFamilyInfo::SRC_STABLEISH
);

REGISTER_HASH(rainbow,
   $.desc       = "Rainbow v1.0.6 (aka Newhash)",
   $.hash_flags = 0,
   $.impl_flags =
        FLAG_IMPL_MULTIPLY_64_64   |
        FLAG_IMPL_ROTATE           |
        FLAG_IMPL_LICENSE_APACHE2,
   $.bits = 64,
   $.verification_LE = 0x2972C52E,
   $.verification_BE = 0x217A79E2,
   $.hashfn_native   = rainbow<64, false>,
   $.hashfn_bswap    = rainbow<64, true>
);

REGISTER_HASH(rainbow_128,
   $.desc       = "Rainbow 128-bit v1.0.6 (aka Newhash 128-bit)",
   $.hash_flags = 0,
   $.impl_flags =
        FLAG_IMPL_MULTIPLY_64_64   |
        FLAG_IMPL_ROTATE           |
        FLAG_IMPL_LICENSE_APACHE2,
   $.bits = 128,
   $.verification_LE = 0xFD9BCADD,
   $.verification_BE = 0xAF215EDF,
   $.hashfn_native   = rainbow<128, false>,
   $.hashfn_bswap    = rainbow<128, true>
);

REGISTER_HASH(rainbow_256,
   $.desc       = "Rainbow 256-bit v1.0.6 (aka Newhash 256-bit)",
   $.hash_flags = 0,
   $.impl_flags =
        FLAG_IMPL_MULTIPLY_64_64   |
        FLAG_IMPL_ROTATE           |
        FLAG_IMPL_LICENSE_APACHE2,
   $.bits = 256,
   $.verification_LE = 0xB327563D,
   $.verification_BE = 0x10C46364,
   $.hashfn_native   = rainbow<256, false>,
   $.hashfn_bswap    = rainbow<256, true>
);
#endif
