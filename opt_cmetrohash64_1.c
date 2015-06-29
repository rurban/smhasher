
// Original: metrohash64.cpp
// Port to C: Frank Denis (jedisct1@github) 
// Optimized variants: Paul G (paulie-g@github, paulg@perforge.net)
//
//
// The MIT License (MIT)
//
// Copyright (c) 2015 J. Andrew Rogers
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.
//

#include "cmetrohash.h"
#include "opt_cmetrohash.h"

#define DO_16 {\
	register uint64_t v0 = hash + (cread_u64(ptr) * k0); ptr += 8; v0 = crotate_right(v0,33) * k1;\
	register uint64_t v1 = hash + (cread_u64(ptr) * k1); ptr += 8; v1 = crotate_right(v1,33) * k2;\
	v0 ^= crotate_right(v0 * k0, 35) + v1;\
	v1 ^= crotate_right(v1 * k3, 35) + v0;\
	hash += v1;\
}

#define DO_8 {\
    hash += cread_u64(ptr) * k3; ptr += 8;\
    hash ^= crotate_right(hash, 33) * k1;\
}

#define DO_4 {\
    hash += cread_u32(ptr) * k3; ptr += 4;\
    hash ^= crotate_right(hash, 15) * k1;\
}

#define DO_2 {\
    hash += cread_u16(ptr) * k3; ptr += 2;\
    hash ^= crotate_right(hash, 13) * k1;\
}

#define DO_1 {\
	hash += cread_u8 (ptr) * k3;\
	hash ^= crotate_right(hash, 25) * k1;\
}

#define OUT {\
    hash ^= crotate_right(hash, 33);\
    hash *= k0;\
    hash ^= crotate_right(hash, 33);\
    memcpy(out, &hash, 8);\
	return;\
}


void cmetrohash64_1_optshort(const uint8_t * key, uint64_t len, uint32_t seed, uint8_t * out)
{
    static const uint64_t k0 = 0xC83A91E1;
    static const uint64_t k1 = 0x8648DBDB;
    static const uint64_t k2 = 0x7BDEC03B;
    static const uint64_t k3 = 0x2F5870A5;

	static const void *branches[] = {
									&&out,
									&&len1,
									&&len2,
									&&len3,
									&&len4,
									&&len5,
									&&len6,
									&&len7,
									&&len8,
									&&len9,
									&&len10,
									&&len11,
									&&len12,
									&&len13,
									&&len14,
									&&len15,
									&&len16,
									&&len17,
									&&len18,
									&&len19,
									&&len20,
									&&len21,
									&&len22,
									&&len23,
									&&len24,
									&&len25,
									&&len26,
									&&len27,
									&&len28,
									&&len29,
									&&len30,
									&&len31,
									&&lenall,
	};

    const uint8_t * ptr = key;
    const uint8_t * const end = ptr + len;
    
    uint64_t hash = ((((uint64_t) seed) + k2) * k0) + len;

    uint64_t branch_index = len > BRANCH_THRESHHOLD(branches) ? BRANCH_MAXINDEX(branches) : len;
    goto *branches[branch_index];


lenall:    
    if (len >= 32)
    {
        uint64_t v[4];
        v[0] = hash;
        v[1] = hash;
        v[2] = hash;
        v[3] = hash;
        
        do
        {
            v[0] += cread_u64(ptr) * k0; ptr += 8; v[0] = crotate_right(v[0],29) + v[2];
            v[1] += cread_u64(ptr) * k1; ptr += 8; v[1] = crotate_right(v[1],29) + v[3];
            v[2] += cread_u64(ptr) * k2; ptr += 8; v[2] = crotate_right(v[2],29) + v[0];
            v[3] += cread_u64(ptr) * k3; ptr += 8; v[3] = crotate_right(v[3],29) + v[1];
        }
        while (ptr <= (end - 32));

        v[2] ^= crotate_right(((v[0] + v[3]) * k0) + v[1], 33) * k1;
        v[3] ^= crotate_right(((v[1] + v[2]) * k1) + v[0], 33) * k0;
        v[0] ^= crotate_right(((v[0] + v[2]) * k0) + v[3], 33) * k1;
        v[1] ^= crotate_right(((v[1] + v[3]) * k1) + v[2], 33) * k0;
        hash += v[0] ^ v[1];
    }
    
    if ((end - ptr) >= 16)
    {
        uint64_t v0 = hash + (cread_u64(ptr) * k0); ptr += 8; v0 = crotate_right(v0,33) * k1;
        uint64_t v1 = hash + (cread_u64(ptr) * k1); ptr += 8; v1 = crotate_right(v1,33) * k2;
        v0 ^= crotate_right(v0 * k0, 35) + v1;
        v1 ^= crotate_right(v1 * k3, 35) + v0;
        hash += v1;
    }
    
    if ((end - ptr) >= 8)
    {
        hash += cread_u64(ptr) * k3; ptr += 8;
        hash ^= crotate_right(hash, 33) * k1;
        
    }
    
    if ((end - ptr) >= 4)
    {
        hash += cread_u32(ptr) * k3; ptr += 4;
        hash ^= crotate_right(hash, 15) * k1;
    }
    
    if ((end - ptr) >= 2)
    {
        hash += cread_u16(ptr) * k3; ptr += 2;
        hash ^= crotate_right(hash, 13) * k1;
    }
    
    if ((end - ptr) >= 1)
    {
        hash += cread_u8 (ptr) * k3;
        hash ^= crotate_right(hash, 25) * k1;
    }
    
    hash ^= crotate_right(hash, 33);
    hash *= k0;
    hash ^= crotate_right(hash, 33);

    memcpy(out, &hash, 8);
    return;

len31:
	DO_16
	DO_8
	DO_4
	DO_2
	DO_1
	OUT

len30:
	DO_16
	DO_8
	DO_4
	DO_2
	OUT

len29:
	DO_16
	DO_8
	DO_4
	DO_1
	OUT

len28:
	DO_16
	DO_8
	DO_4
	OUT

len27:
	DO_16
	DO_8
	DO_2
	DO_1
	OUT

len26:
	DO_16
	DO_8
	DO_2
	OUT

len25:
	DO_16
	DO_8
	DO_1
	OUT

len24:
	DO_16
	DO_8
	OUT

len23:
	DO_16
	DO_4
	DO_2
	DO_1
	OUT

len22:
	DO_16
	DO_4
	DO_2
	OUT


len21:
	DO_16
	DO_4
	DO_1
	OUT

len20:
	DO_16
	DO_4
	OUT

len19:
	DO_16
	DO_2
	DO_1
	OUT

len18:
	DO_16
	DO_2
	OUT

len17:
	DO_16
	DO_1
	OUT

len16:
	DO_16
	OUT

len15:
	DO_8
	DO_4
	DO_2
	DO_1
	OUT

len14:
	DO_8
	DO_4
	DO_2
	OUT

len13:
	DO_8
	DO_4
	DO_1
	OUT

len12:
	DO_8
	DO_4
	OUT

len11:
	DO_8
	DO_2
	DO_1
	OUT

len10:
	DO_8
	DO_2
	OUT

len9:
	DO_8
	DO_1
	OUT

len8:
	DO_8
	OUT
len7:
	DO_4
	DO_2
	DO_1
	OUT
len6:
	DO_4
	DO_2
	OUT
len5:
	DO_4
	DO_1
	OUT
len4:
	DO_4
	OUT
len3:
	DO_2
	DO_1
	OUT
len2:
	DO_2
	OUT
len1:
	DO_1

out:
	OUT

}
