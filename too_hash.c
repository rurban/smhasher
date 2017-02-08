#include "too_hash.h"

#define TOO_MIXV(v,v0,v1,s0,s1,s2,s3,s4) \
STMT_START {                        \
    WARN3("v0=%08x v1=%08x v=%08x - MIXV\n", v0, v1, v);\
    v0 ^= v;                        \
    v1 += v0;                       \
    v0 = ROTL(v0, s0);              \
    v0 ^= v1;                       \
    v1 = ROTL(v1, s1);              \
                                    \
    v1 ^= ~v;                       \
    v0 += v1;                       \
    v1 = ROTL(v1, s2);              \
    v1 ^= v0;                       \
    v0 = ROTL(v0, s3);              \
    v0 ^= v1;                       \
    v1 = ROTL(v1, s4);              \
} STMT_END

#define TOO_MIXV_SHIFT(v,v0,v1) \
    TOO_MIXV(v,v0,v1,13,20, 4,14,17)

/*    TOO_MIXV(v,v0,v1,25,17,10,13,19) *
/*    TOO_MIXV(v,v0,v1,30,7,21,12,26)*/

PERL_STATIC_INLINE U32 too_hash(
    U32 *seed,
    U8 *key,
    STRLEN key_len
) {
    U32 tail= key_len & 3;
    U8 *end= key + key_len - tail;
    U32 v0= seed[0];
    U32 v1= seed[1];
    U32 v= (key_len | (1 << 31)) & ~(1<<30);

    TOO_MIXV_SHIFT(v,v0,v1);

    for (;key < end; key += 4) {
        v = U8TO32_LE(key);
        TOO_MIXV_SHIFT(v,v0,v1);
    }

    v= ~tail << 24;
    switch (tail & 3) {
        case 3: v |= (key[2] << 16);
        case 2: v |= ((key[1] << 8) | key[0]);
                break;
        case 1: v |= key[0];
                break;
                /* fallthrough */
        default: break;
    }
    TOO_MIXV_SHIFT(v,v0,v1);
    TOO_MIXV_SHIFT(0xFF,v0,v1);
    
    v = v0 ^ v1;

    WARN3("v0=%08x v1=%08x H=%08x - FINAL\n", v0, v1, v);

    return v;
}


void too_hash_test(const void *key, STRLEN len, U32 seed_base, void *out) {
    U32 seed[2]= { seed_base, seed_base }; /* ^ 0xaed548f1 }; */
    *((U32 *)out)= too_hash(seed, (U8 *)key, len);
}

