#include "yhash.h"
/* lived 75 generations */
#define MIXV0(v,v0,v1) STMT_START {     \
    WARN3("v0=%016llx v1=%016llx v=%016llx - MIXV0\n", v0, v1, v); \
    v1 += v;                            \
    v0 += v1;                           \ 
    v1 = ROTL(v1, 32) ^ v0;             \               
    v0 += v1;                           \ 
    v0 = ROTL(v0, 15) ^ v1;             \               
    v1 -= v0;                           \
} STMT_END

/* lived 66 generations */

#define MIXV1(v,v0,v1) STMT_START {     \
    WARN3("v0=%016llx v1=%016llx v=%016llx - MIXV0\n", v0, v1, v); \
    v1 += v;                            \
    v0 -= v1;                           \ 
    v1 = ROTL(v1, 33) ^ v0;             \               
    v0 += v1;                           \ 
    v0 = ROTL(v0, 16) ^ v1;             \               
    v1 -= v0;                           \
} STMT_END

PERL_STATIC_INLINE U64 stoner_hash(
    U8 *seed_ch,
    U8 *key,
    STRLEN key_len
) {
    HASH_UINT *seed= (HASH_UINT *)seed_ch;
    HASH_UINT v0= seed[0] ^ 0x736f6d6570736575ull;
    HASH_UINT v1= seed[0];
    HASH_UINT v;

    while (key_len >= 16) {
        v = U8TO64_LE(key);
        MIXV0(v,v0,v1);
        key += 8;       
        v = U8TO64_LE(key);
        MIXV1(v,v0,v1);
        key_len -= 16;
        key += 8;
    }

    if (key_len >= 8) {
        v = U8TO64_LE(key);
        MIXV0(v,v0,v1);
        key += 8;       
        key_len -= 8;
    }

    v= ((U64)key_len+1) << 56;
    switch (tail & 7) {
        case 7: v |= (U64)key[6] << 48;
        case 6: v |= (U64)key[5] << 40;
        case 5: v |= (U64)key[4] << 32;
        case 4: v |= (U64)key[3] << 24;
        case 3: v |= (U64)key[2] << 16;
        case 2: v |= (U64)key[1] << 8;
        case 1: v |= (U64)key[0];
        default: break;
    }
    MIXV0(v,v0,v1);

    WARN2("v0=%016llx v1=%016llx   %016s - ENTER FINALIZE\n", v0, v1);
    v1 ^= v0;                             
    v0 = ROTL(v0, 20) + v1;                                
    v0 = ROTL(v0, 35);
    v1 -= v0;                          
    v0 = ROTL(v0, 46);
    v1 ^= v0;                             
    v0 += v1;                             
    v0 = ROTL(v0,  6);
    v1 += v0;                             

    WARN2("v0=%016llx v1=%016llx   %16s - MID FINALIZE\n", v0, v1);
    v0 += v1;                             
    v1 = ROTL(v1, 53);
    v0 ^= v1;                             
    v1 += v0;                             
    v0 = ROTL(v0, 50) - v1;                             
    v1 = ROTL(v1, 36) ^ v0;                             
    v1 = ROTL(v1, 31);     
    v0 += v1;                             
    v1 = ROTL(v1, 30) + v0;                             
    v1 = ROTL(v1, 56);
    
    v = v0 ^ v1;

    WARN3("v0=%016llx v1=%016llx h=%016llx - FINAL\n\n", v0, v1, v);

    return v;
}

void yhash_test(const void *key, STRLEN len, U32 seed, void *out) {
    U32 seed[2]= { seed_base, seed_base ^ 0xDEADBEEF };
    *((U64 *)out)= stoner_hash(seed, (U8 *)key, len);
}

