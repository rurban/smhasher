#pragma once
#include <stdio.h>
#include <iostream>
template <typename hashtype>
void CtrStream(hashfunc<hashtype>hash, uint64_t rng_seed, int key_len) {
    uint8_t key[key_len];
    Rand r(rng_seed);
    memset(key,0,key_len);
    hash.seed_state_rand(r);
    while (1) {
        hashtype h;
        hash(key,key_len,&h);
        std::cout.write((const char*)&h,sizeof(h));
        int i= 0;
        while (1) {
            if ( ++key[i] ) {
                break;
            } else if (i < key_len) {
                i++;
            } else {
                /* roll-over, new seed. presumably this won't happend often */
                hash.seed_state_rand(r);
                break;
            }
        }
    }
    /* Not-Reached */
}
