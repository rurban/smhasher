// This is free and unencumbered software released into the public domain.


#include "pearsonb.h"


// Christopher Wellons' triple32 from https://github.com/skeeto/hash-prospector
// published under The Unlicense
#define permute32(in) \
    in ^= in >> 17;   \
    in *= 0xed5ad4bb; \
    in ^= in >> 11;   \
    in *= 0xac4c1b51; \
    in ^= in >> 15;   \
    in *= 0x31848bab; \
    in ^= in >> 14


// David Stafford's Mix13 from http://zimbry.blogspot.com/2011/09/better-bit-mixing-improving-on.html
// the author clarified via eMail that this of his work is released to the public domain
#define permute64(in)         \
    in ^= (in >> 30);         \
    in *= 0xbf58476d1ce4e5b9; \
    in ^= (in >> 27);         \
    in *= 0x94d049bb133111eb; \
    in ^= (in >> 31)


#define dec1(in) \
    in--

#define dec2(in) \
    dec1(in);    \
    dec1(in)

#define dec3(in) \
    dec2(in);    \
    dec1(in)

#define dec4(in) \
    dec3(in);    \
    dec1(in)

#define hash_round(hash, in, part) \
    hash##part ^= in;              \
    dec##part(hash##part);         \
    permute64(hash##part)


void pearsonb_hash_256 (uint8_t *out, const uint8_t *in, size_t len, uint64_t seed) {

    uint64_t *current;
    current = (uint64_t*)in;
    uint64_t org_len = len;
    uint64_t hash1 = seed;

    permute64(hash1);

    uint64_t hash2 = hash1;
    uint64_t hash3 = hash1;
    uint64_t hash4 = hash1;

    while (len > 7) {
        // digest words little endian first
        hash_round(hash, le64toh(*current), 1);
        hash_round(hash, le64toh(*current), 2);
        hash_round(hash, le64toh(*current), 3);
        hash_round(hash, le64toh(*current), 4);

        current++;
        len-=8;
    }

    // handle the rest
    hash1 = ~hash1;
    hash2 = ~hash2;
    hash3 = ~hash3;
    hash4 = ~hash4;

    while(len) {
        // byte-wise, no endianess
        hash_round(hash, *(uint8_t*)current, 1);
        hash_round(hash, *(uint8_t*)current, 2);
        hash_round(hash, *(uint8_t*)current, 3);
        hash_round(hash, *(uint8_t*)current, 4);

        current = (uint64_t*)((uint8_t*)current + 1);
        len--;
    }

    // digest length
    hash1 = ~hash1;
    hash2 = ~hash2;
    hash3 = ~hash3;
    hash4 = ~hash4;

    hash_round(hash, org_len, 1);
    hash_round(hash, org_len, 2);
    hash_round(hash, org_len, 3);
    hash_round(hash, org_len, 4);

    // hash string is stored big endian, the natural way to read
    uint64_t *o;
    o = (uint64_t*)out;
    *o = htobe64(hash4);
    o++;
    *o = htobe64(hash3);
    o++;
    *o = htobe64(hash2);
    o++;
    *o = htobe64(hash1);
}


void pearsonb_hash_128 (uint8_t *out, const uint8_t *in, size_t len, uint64_t seed) {

    uint64_t *current;
    current = (uint64_t*)in;
    uint64_t org_len = len;
    uint64_t hash1 = seed;

    permute64(hash1);

    uint64_t hash2 = hash1;

    while (len > 7) {
        // digest words little endian first
        hash_round(hash, le64toh(*current), 1);
        hash_round(hash, le64toh(*current), 2);

        current++;
        len-=8;
    }

    // handle the rest
    hash1 = ~hash1;
    hash2 = ~hash2;

    while(len) {
        // byte-wise, no endianess
        hash_round(hash, *(uint8_t*)current, 1);
        hash_round(hash, *(uint8_t*)current, 2);

        current = (uint64_t*)((uint8_t*)current + 1);
        len--;
    }

    // digest length
    hash1 = ~hash1;
    hash2 = ~hash2;

    hash_round(hash, org_len, 1);
    hash_round(hash, org_len, 2);

    // hash string is stored big endian, the natural way to read
    uint64_t *o;
    o = (uint64_t*)out;
    *o = htobe64(hash2);
    o++;
    *o = htobe64(hash1);
}


uint64_t pearsonb_hash_64 (const uint8_t *in, size_t len, uint64_t seed) {

    uint64_t *current;
    current = (uint64_t*)in;
    uint64_t org_len = len;
    uint64_t hash1 = seed;

    permute64(hash1);

    while(len > 7) {
        // digest words little endian first
        hash_round(hash, le64toh(*current), 1);

        current++;
        len-=8;
    }

    // handle the rest
    hash1 = ~hash1;
    while(len) {
        // byte-wise, no endianess
        hash_round(hash, *(uint8_t*)current, 1);

        current = (uint64_t*)((uint8_t*)current + 1);
        len--;
    }

    // digest length
    hash1 = ~hash1;
    hash_round(hash, org_len, 1);

    // caller is responsible for storing it the big endian way to memory (if ever)
    return hash1;
}


uint32_t pearsonb_hash_32 (const uint8_t *in, size_t len, uint64_t seed) {

    return pearsonb_hash_64(in, len, seed);
}


uint16_t pearsonb_hash_16 (const uint8_t *in, size_t len, uint64_t seed) {

    return pearsonb_hash_64(in, len, seed);
}


void pearsonb_hash_init (void) {

}
