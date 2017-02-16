#!/bin/bash
make
rm doc/*.tmp
./SMHasher Phat4 2>&1 | tee doc/Phat4_32.tmp; mv doc/Phat4_32.tmp doc/Phat4_32.out
./SMHasher Zaphod32 2>&1 | tee doc/Zaphod_32.tmp; mv doc/Zaphod_32.tmp doc/Zaphod_32.out
./SMHasher Phat 2>&1 | tee doc/Phat_32.tmp; mv doc/Phat_32.tmp doc/Phat_32.out 

./SMHasher BeagleHash_32_32 2>&1 | tee doc/BeagleHash_S32_32.tmp doc/BeagleHash_S32_32.out; mv doc/BeagleHash_S32_32.out doc/BeagleHash_S32_32.tmp
./SMHasher BeagleHash_64_32 2>&1 | tee doc/BeagleHash_S32_64.tmp mv doc/BeagleHash_S32_64.tmp doc/BeagleHash_S32_64.out

./SMHasher BeagleHash_32_64 2>&1 | tee doc/BeagleHash_S64_32.tmp; mv doc/BeagleHash_S64_32.tmp doc/BeagleHash_S64_32.out
./SMHasher BeagleHash_64_64 2>&1 | tee doc/BeagleHash_S64_64.tmp; mv doc/BeagleHash_S64_64.tmp doc/BeagleHash_S64_64.out

./SMHasher BeagleHash_32_96 2>&1 | tee doc/BeagleHash_S96_32.tmp; mv doc/BeagleHash_S96_32.tmp doc/BeagleHash_S96_32.out
./SMHasher BeagleHash_64_96 2>&1 | tee doc/BeagleHash_S96_64.tmp; mv doc/BeagleHash_S96_64.tmp doc/BeagleHash_S96_64.out

./SMHasher BeagleHash_32_112 2>&1 | tee doc/BeagleHash_S112_32.tmp; mv doc/BeagleHash_S112_32.tmp doc/BeagleHash_S112_32.out
./SMHasher BeagleHash_64_112 2>&1 | tee doc/BeagleHash_S112_64.tmp; mv doc/BeagleHash_S112_64.tmp doc/BeagleHash_S112_64.out

./SMHasher BeagleHash_32_127 2>&1 | tee doc/BeagleHash_S127_32.tmp; mv doc/BeagleHash_S127_32.tmp doc/BeagleHash_S127_32.out
./SMHasher BeagleHash_64_127 2>&1 | tee doc/BeagleHash_S127_64.tmp; mv doc/BeagleHash_S127_64.tmp doc/BeagleHash_S127_64.out

./SMHasher Spooky32 2>&1 | tee doc/Spooky_32.tmp; mv doc/Spooky_32.tmp doc/Spooky_32.out
./SMHasher Spooky64 2>&1 | tee doc/Spooky_64.tmp; mv doc/Spooky_64.tmp doc/Spooky_64.out
./SMHasher Spooky128 2>&1 | tee doc/Spooky_128.tmp; mv doc/Spooky_128.tmp doc/Spooky_128.out

./SMHasher City64 2>&1 | tee doc/City_64.tmp; mv doc/City_64.tmp doc/City_64.out
./SMHasher City128 2>&1 | tee doc/City_128.tmp; mv doc/City_128.tmp doc/City_128.out

./SMHasher Murmur2 2>&1 | tee doc/Murmur2_32.tmp; mv doc/Murmur2_32.tmp doc/Murmur2_32.out
./SMHasher Murmur2A 2>&1 | tee doc/Murmur2A_32.tmp; mv doc/Murmur2A_32.tmp doc/Murmur2A_32.out
./SMHasher Murmur2B 2>&1 | tee doc/Murmur2B_64.tmp; mv doc/Murmur2B_64.tmp doc/Murmur2B_64.out
./SMHasher Murmur3C 2>&1 | tee doc/Murmur2C_64.tmp; mv doc/Murmur2C_64.tmp doc/Murmur2C_64.out

./SMHasher PMurHash32 2>&1 | tee doc/PMurHash32.tmp; mv doc/PMurHash32.tmp doc/PMurHash32.out
./SMHasher Murmur3A 2>&1 | tee doc/Murmur3A_32.tmp; mv doc/Murmur3A_32.tmp doc/Murmur3A_32.out
./SMHasher Murmur3C 2>&1 | tee doc/Murmur3C_128.tmp; mv doc/Murmur3C_128.tmp doc/Murmur3C_128.out
./SMHasher Murmur3F 2>&1 | tee doc/Murmur3F_128.tmp; mv doc/Murmur3F_128.tmp doc/Murmur3F_128.out

./SMHasher superfast 2>&1 | tee doc/SuperFast_32.tmp; mv doc/SuperFast_32.tmp doc/SuperFast_32.out

./SMHasher FNV 2>&1 | tee doc/FNV_32.tmp; mv doc/FNV_32.tmp doc/FNV_32.out

./SMHasher bernstein 2>&1 | tee doc/bernstein_32.tmp; mv doc/bernstein_32.tmp doc/bernstein_32.out

./SMHasher lookup3 2>&1 | tee doc/lookup3_32.tmp; mv doc/lookup3_32.tmp doc/lookup3_32.out


