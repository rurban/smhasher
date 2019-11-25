SMhasher
========

| Hash function                                 |      MiB/sec |cycl./hash|cycl./map   | size| Quality problems               |
|:----------------------------------------------|-------------:|---------:|-----------:|----:|--------------------------------|
| [donothing32](doc/donothing32)                |  14924689.47 |     6.00 |     -      |  13 | test NOP                       |
| [donothing64](doc/donothing64)                |  14942317.78 |     6.00 |     -      |  13 | test NOP                       |
| [donothing128](doc/donothing128)              |  14943678.61 |     6.00 |     -      |  13 | test NOP                       |
| [NOP_OAAT_read64](doc/NOP_OAAT_read64)        |     29794.83 |    35.50 |     -      |  47 | test NOP                       |
| [BadHash](doc/BadHash)                        |       522.75 |    96.39 |     -      |  47 | test FAIL                      |
| [sumhash](doc/sumhash)                        |      7135.41 |    31.14 |     -      | 363 | test FAIL                      |
| [sumhash32](doc/sumhash32)                    |     23873.87 |    22.49 |     -      | 863 | test FAIL                      |
| [multiply_shift](doc/multiply_shift)          |      4909.57 |    45.28 |  too slow  | 345 | fails all tests                |
| [pair_multiply_shift](doc/pair_multiply_shift)|     13604.46 |    31.71 |  too slow  | 609 | fails all tests                |
| --------------------------                    |              |          |            |     |                                |
| [crc32](doc/crc32)                            |       392.05 |   130.08 | 199.87 (3) | 422 | insecure, 8590x collisions, distrib |
| [md5_32a](doc/md5_32a)                        |       352.19 |   668.69 |1053.14(149)|4419 | 8590x collisions, distrib |
| [sha1_32a](doc/sha1_32a)                      |       364.78 |  1514.25 |1967.21 (14)|5126 | collisions, 36.6% distrib |
| [sha2-224](doc/sha2-224)                      |       147.13 |  1354.81 |1589.92 (12)|     | collisions++  |
| [sha2-224_64](doc/sha2-224_64)                |       147.60 |  1360.10 |1620.93 (13)|     | collisions++  |
| [sha2-256](doc/sha2-256)                      |       147.80 |  1374.90 |1606.06 (16)|     | collisions++, Moment Chi2 4  |
| [sha2-256_64](doc/sha2-256_64)                |       148.01 |  1376.34 |1624.71 (16)|     | Moment Chi2 7 |
| [rmd128](doc/rmd128)                          |       332.78 |   672.35 | 897.29 (8) |     | collisions++  |
| [rmd160](doc/rmd160)                          |       202.16 |  1045.79 |1287.74 (16)|     | collisions++  |
| [rmd256](doc/rmd256)                          |       356.57 |   638.30 | 833.96 (10)|     | collisions++  |
| [blake2s-128](doc/blake2s-128)                |       295.30 |   698.09 |1059.24 (51)|     | collisions++  |
| [blake2s-160](doc/blake2s-160)                |       215.01 |  1026.74 |1239.54 (11)|     | collisions++  |
| [blake2s-224](doc/blake2s-224)                |       207.06 |  1063.86 |1236.50 (20)|     | collisions++  |
| [blake2s-256](doc/blake2s-256)                |       215.28 |  1014.88 |1230.38 (28)|     | collisions++  |
| [blake2s-256_64](doc/blake2s-256_64)          |       211.52 |  1044.22 |1228.43 (8) |     |               |
| [blake2b-160](doc/blake2b-160)                |       356.08 |  1236.84 |1458.15 (12)|     | collisions++  |
| [blake2b-224](doc/blake2b-224)                |       356.59 |  1228.50 |1425.87 (16)|     | collisions++  |
| [blake2b-256](doc/blake2b-256)                |       355.97 |  1232.22 |1443.31 (19)|     | collisions++  |
| [blake2b-256_64](doc/blake2b-256_64)          |       356.97 |  1222.76 |1435.03 (9) |     |               |
| [sha3-256](doc/sha3-256)                      |       100.58 |  3877.18 |4159.79 (37)|     | collisions++  |
| [sha3-256_64](doc/sha3-256_64)                |       100.57 |  3909.00 |4174.63 (16)|     | collisions++  |
| [hasshe2](doc/hasshe2)                        |      2357.32 |    76.10 | 348.10 (6) | 445 | insecure, fails all tests    |
| [crc32_hw](doc/crc32_hw)                      |      6292.63 |    30.38 | 204.19 (18)| 653 | insecure, 100% bias, collisions, distrib, machine-specific (x86 SSE4.2) |
| [crc32_hw1](doc/crc32_hw1)                    |     23382.53 |    36.84 | 197.39 (20)| 671 | insecure, 100% bias, collisions, distrib, machine-specific (x86 SSE4.2) |
| [crc64_hw](doc/crc64_hw)                      |      8387.01 |    29.96 | 161.44 (13)| 652 | insecure, 100% bias, collisions, distrib, machine-specific (x64 SSE4.2) |
| [fibonacci](doc/fibonacci)                    |      9462.17 |    33.52 | 774.87 (14)|1692 | zeros, fails all tests     |
| [FNV1a](doc/FNV1a)                            |       780.53 |    69.65 | 197.06 (20)| 204 | zeros, fails all tests       |
| [FNV1A_Totenschiff](doc/FNV1A_Totenschiff)    |      6235.86 |    36.56 | 214.61 (18)| 270 | zeros, fails all tests       |
| [FNV1A_Pippip_Yurii](doc/FNV1A_Pippip_Yurii)  |      6238.08 |    37.22 | 218.06 (25)| 147 | sanity, fails all tests      |
| [FNV1a_YT](doc/FNV1a_YT)                      |      9618.28 |    28.26 | 192.33 (18)| 321 | fails all tests              |
| [FNV2](doc/FNV2)                              |      6244.26 |    38.25 | 168.46 (15)| 278 | fails all tests              |
| [FNV64](doc/FNV64)                            |       791.82 |    69.82 | 184.87 (19)|  79 | fails all tests              |
| [fletcher2](doc/fletcher2)                    |     11809.88 |    26.85 | 321.91 (13)| 248 | fails all tests              |
| [fletcher4](doc/fletcher4)                    |     11973.55 |    26.94 | 324.53 (12)| 371 | fails all tests              |
| [bernstein](doc/bernstein)                    |       791.83 |    67.15 | 200.11 (17)|  41 | fails all tests              |
| [sdbm](doc/sdbm)                              |       785.84 |    67.02 | 209.44 (17)|  41 | fails all tests              |
| [x17](doc/x17)                                |       522.46 |    97.51 | 214.03 (18)|  79 | 99.98% bias, fails all tests |
| [JenkinsOOAT](doc/JenkinsOOAT)                |       446.57 |   142.43 | 245.34 (20)| 153 | 53.5% bias, fails all tests  |
| [JenkinsOOAT_pl](doc/JenkinsOOAT_perl)        |       442.05 |   120.53 | 336.30 (4) |  65 | 1.5-11.5% bias, 7.2x collisions  |
| [HalfSipHash](doc/HalfSipHash)                |       741.59 |   122.25 | 256.22 (20)| 700 | zeroes                     |
| [SipHash13](doc/SipHash13)                    |      1762.44 |   104.61 | 304.84 (17)| 778 | 0.9% bias                  |
| [MicroOAAT](doc/MicroOAAT)                    |       950.84 |    62.23 | 233.94 (19)|  68 | 100% bias, distrib           |
| [jodyhash32](doc/jodyhash32)                  |      1386.76 |    46.13 | 232.12 (21)| 102 | bias, collisions, distr      |
| [jodyhash64](doc/jodyhash64)                  |      2765.84 |    40.89 | 222.41 (20)| 118 | bias, collisions, distr      |
| [lookup3](doc/lookup3)                        |      1702.13 |    48.25 | 243.90 (19)| 341 | 28% bias, collisions, 30% distr  |
| [superfast](doc/superfast)                    |      1899.70 |    54.19 | 240.49 (19)| 210 | 91% bias, 5273.01x collisions, 37% distr |
| [MurmurOAAT](doc/MurmurOAAT)                  |       437.03 |   117.00 | 236.62 (19)|  47 | collisions, 99.998% distr          |
| [Crap8](doc/Crap8)                            |      3060.17 |    35.25 | 242.59 (20)| 342 | 2.42% bias, collisions, 2% distrib |
| [Murmur2](doc/Murmur2)                        |      3032.69 |    41.97 | 233.68 (19)| 358 | 1.7% bias, 81x coll, 1.7% distrib  |
| [Murmur2A](doc/Murmur2A)                      |      3033.56 |    46.63 | 227.70 (20)| 407 | 12.7% bias                         |
| [Murmur2B](doc/Murmur2B)                      |      4728.94 |    47.90 | 192.82 (18)| 433 | 1.8% bias, collisions, 3.4% distrib|
| [Murmur2C](doc/Murmur2C)                      |      3817.99 |    47.67 | 207.53 (18)| 444 | 91% bias, collisions, distr        |
| [Murmur3C](doc/Murmur3C)                      |      3093.46 |    68.53 | 256.49 (19)| 859 | LongNeighbors              |
| [PMurHash32](doc/PMurHash32)                  |      2281.97 |    58.65 | 259.59 (20)|1862 | Moment Chi2 69             |
| [Murmur3A](doc/Murmur3A)                      |      2347.90 |    51.38 | 237.58 (19)| 351 | Moment Chi2 69             |
| [Murmur3F](doc/Murmur3F)                      |      5076.20 |    51.66 | 222.39 (18)| 699 | Sparse for low 32bit       |
| [xxHash32](doc/xxHash32)                      |      5868.40 |    49.08 | 222.09 (21)| 738 | LongNeighbors, collisions with 4bit diff |
| [metrohash64_1](doc/metrohash64_1)            |      9274.60 |    50.85 | 201.37 (19)| 624 | LongNeighbors              |
| [metrohash64_2](doc/metrohash64_2)            |      9281.99 |    51.04 | 191.44 (20)| 627 | LongNeighbors              |
| [metrohash64crc_1](doc/metrohash64crc_1)      |     13641.79 |    56.19 | 193.28 (19)| 632 | cyclic collisions 8 byte, machine-specific (x64 SSE4.2) |
| [metrohash64crc_2](doc/metrohash64crc_2)      |     13550.77 |    56.17 | 204.84 (18)| 632 | cyclic collisions 8 byte, machine-specific (x64 SSE4.2) |
| [metrohash128crc_1](doc/metrohash128crc_1)    |     13668.29 |    85.94 | 249.99 (18)| 723 | low 32bit coll. machine-specific (x64 SSE4.2) |
| [metrohash128crc_2](doc/metrohash128crc_2)    |     13600.83 |    85.92 | 239.28 (20)| 723 | low 32bit coll. machine-specific (x64 SSE4.2) |
| [metrohash128_1](doc/metrohash128_1)          |      9263.64 |    79.71 | 240.70 (20)| 773 | LongNeighbors              |
| [metrohash128_2](doc/metrohash128_2)          |      9060.39 |    79.74 | 240.46 (20)| 773 | LongNeighbors              |
| [cmetrohash64_1o](doc/cmetrohash64_1o)        |      8851.32 |    50.59 | 193.19 (18)|3506 | LongNeighbors              |
| [cmetrohash64_1](doc/cmetrohash64_1)          |      9074.37 |    50.92 | 201.33 (18)| 652 | LongNeighbors              |
| [cmetrohash64_2](doc/cmetrohash64_2)          |      9302.98 |    51.09 | 194.04 (18)| 655 | LongNeighbors              |
| [City64noSeed](doc/City64noSeed)              |      8873.07 |    38.70 | 171.57 (4) |1038 |                            |
| [City64](doc/City64)                          |      9213.05 |    55.52 | 200.24 (2) |1120 | 2 minor collisions         |
| [falkhash](doc/falkhash)                      |     20374.98 |   169.84 | 328.42 (5) | 264 | LongNeighbors, machine-specific (x86_64 AES-NI) |
| [FarmHash128](doc/FarmHash128)                |      9814.32 |    82.06 | 229.38 (4) | 163 | low 32bit coll.            |
| [farmhash128_c](doc/farmhash128_c)            |     10158.20 |    98.80 | 232.00 (2) |1890 | low 32bit coll.            |
| [t1ha2_atonce128](doc/t1ha2_atonce128)        |      8382.53 |    64.15 | 199.63 (4) | 613 | LongNeighbors              |
| [t1ha2_stream128](doc/t1ha2_stream128)        |      8846.52 |   117.19 | 270.95 (2) |1665 | LongNeighbors              |
| [Spooky128](doc/Spooky128)                    |      9865.67 |    70.84 | 185.64 (2) |2221 | low 32bit coll.            |
| [xxh3](doc/xxh3)                              |     11694.36 |    37.07 | 169.43 (2) | 744 | Moment Chi2 14974 ! low 32bit coll |
| [xxh3low](doc/xxh3low)                        |     10664.58 |    36.96 | 196.75 (3) | 756 | Moment Chi2 1.8e+9 ! low 32bit coll|
| [xxh128](doc/xxh128)                          |     11553.50 |    41.68 | 184.25 (2) |1012 | low 32bit coll.            |
| [xxh128low](doc/xxh128low)                    |     11482.39 |    37.60 | 168.17 (4) |1012 | Moment Chi2 14974 ! low 32bit coll |
| --------------------------------------        |              |          |            |     |                            |
| [SipHash](doc/SipHash)                        |       958.78 |   141.84 | 278.15 (3) |1071 |                            |
| [HighwayHash64](doc/HighwayHash64)            |      6239.64 |    98.65 | 263.94 (4) |2546 |                            |
| [GoodOAAT](doc/GoodOAAT)                      |      1052.90 |    70.80 | 208.00 (3) | 237 |                            |
| [TSip](doc/TSip)                              |      3346.72 |    60.02 | 203.82 (5) | 519 | !msvc                      |
| [seahash](doc/seahash)                        |      4517.87 |    65.03 | 212.61 (3) | 871 | !msvc                      |
| [seahash32low](doc/seahash32low)              |      4589.52 |    64.53 | 212.74 (3) | 871 | !msvc                      |
| [fasthash32](doc/fasthash32)                  |      4658.24 |    50.50 | 181.96 (2) | 566 |                            |
| [fasthash64](doc/fasthash64)                  |      4657.64 |    47.71 | 168.22 (3) | 509 | Moment Chi2 5159 !         |
| [MUM](doc/MUM)                                |      6890.75 |    39.90 | 174.94 (3) |1912 | machine-specific (32/64 differs) |
| [MUMlow](doc/MUMlow)                          |      6893.98 |    46.02 | 191.55 (3) |1912 |                            |
| [mirhash](doc/mirhash)                        |      5453.50 |    42.31 | 163.35 (2) |1112 | LongNeighbors, machine-specific (32/64 differs) |
| [mirhash32low](doc/mirhash32low)              |      5452.48 |    42.31 | 190.92 (2) |1112 | LongNeighbors, machine-specific (32/64 differs) |
| [mirhashstrict](doc/mirhashstrict)            |      2217.70 |    65.39 | 175.38 (3) |1112 |                            |
| [mirhashstrict32low](doc/mirhashstrict32low)  |      2217.87 |    64.72 | 188.44 (4) |1112 |                            |
| [City32](doc/City32)                          |      3833.52 |    52.97 | 211.69 (4) |1319 |                            |
| [City64low](doc/City64low)                    |      9209.88 |    63.55 | 274.62 (17) |1120|                           |
| [City128](doc/City128)                        |      9747.33 |    77.22 | 269.02 (14) |1841 |                            |
| [CityCrc128](doc/CityCrc128)                  |     12958.29 |    77.31 | 267.14 (17) | 295 |                            |
| [FarmHash32](doc/FarmHash32)                  |     16663.96 |    63.67 | 265.35 (19) |11489| machine-specific (x86_64 SSE4/AVX) |
| [FarmHash64](doc/FarmHash64)                  |      8476.56 |    65.05 | 251.81 (17) |3758 |                            |
| [farmhash32_c](doc/farmhash32_c)              |     17335.51 |    63.77 | 277.75 (18) | 762 | machine-specific (x86_64 SSE4/AVX) |
| [farmhash64_c](doc/farmhash64_c)              |      8335.98 |    77.01 | 266.15 (18) |3688 |                            |
| [xxHash64](doc/xxHash64)                      |      8719.19 |    59.22 | 210.40 (16) |1999 |                            |
| [Spooky32](doc/Spooky32)                      |      9570.40 |    70.27 | 250.23 (18) |2221|                            |
| [Spooky64](doc/Spooky64)                      |      9603.18 |    70.19 | 231.73 (17) |2221 |                            |
| [clhash](doc/clhash)                          |      4405.28 |    85.35 | 288.20 (14) |1809 | machine-specific (x64 SSE4.2) |
| [t1ha2_atonce](doc/t1ha2_atonce)              |      8192.03 |    48.48 | 230.46 (14) | 541 |                            |
| [t1ha2_stream](doc/t1ha2_stream)              |      8558.73 |    93.45 | 305.03 (16) |1665 |                            |
| [t1ha1_64le](doc/t1ha1_64le)                  |      9541.33 |    39.33 | 239.04 (16) | 517 |                            |
| [t1ha1_64be](doc/t1ha1_64be)                  |      6848.76 |    41.08 | 230.22 (17) | 555 |                            |
| [t1ha0_32le](doc/t1ha0_32le)                  |      4913.74 |    55.00 | 233.39 (18) | 509 | LongNeighbors              |
| [t1ha0_32be](doc/t1ha0_32be)                  |      4241.68 |    55.12 | 236.50 (18) | 533 | LongNeighbors              |
| [t1ha0_aes_noavx](doc/t1ha0_aes_noavx)        |     21782.86 |    48.17 | 238.14 (18) | 925 | LN, machine-specific (x86 AES-NI) |
| [t1ha0_aes_avx1](doc/t1ha0_aes_avx1)          |     22714.85 |    48.12 | 226.52 (16) | 843 | LN, machine-specific (x64 AVX)    |
| [t1ha0_aes_avx2](doc/t1ha0_aes_avx2)          |     36436.51 |    36.31 | ?           | 792 | LN, machine-specific (x64 AVX2)   |
| [wyhash](doc/wyhash)                          |     10423.42 |    31.12 | 219.26 (19) |1373 |                            |
| [wyhash32low](doc/wyhash32low)                |     10407.09 |    31.14 | 239.93 (18) |1373 | Moment Chi2 4.2            |

Summary
-------

I added some SSE assisted hashes and fast intel/arm CRC32-C and AES HW variants, but not the fastest
[crcutil](https://code.google.com/p/crcutil/) yet. See [our crcutil results](https://github.com/rurban/smhasher/blob/master/doc/crcutil).
See also the old [https://code.google.com/p/smhasher/w/list](https://code.google.com/p/smhasher/w/list).

So the fastest hash functions on x86_64 without quality problems are:

- wyhash v3
- t1ha2_atonce, t1ha1_64le
- metrohash64crc
- FarmHash (_not portable, too machine specific: 64 vs 32bit, old gcc, ..._)
- Spooky32
- fasthash
- MUM/mir (_different results on 32/64-bit archs_)

Hash functions for symbol tables or hash tables typically use 32 bit
hashes, for databases, file systems and file checksums typically 64 or
128bit, for crypto now starting with 256 bit.

Typical median key size in perl5 is 20, the most common 4. Similar for all other dynamic languages.
See [github.com/rurban/perl-hash-stats](https://github.com/rurban/perl-hash-stats)

When used in a hash table the instruction cache will usually beat the
CPU and throughput measured here. In my tests the smallest `FNV1A`
beats the fastest `crc32_hw1` with [Perl 5 hash tables](https://github.com/rurban/perl-hash-stats). 
Even if those worse hash functions will lead to more collisions, the
overall speed advantage and inline-ability beats the slightly worse quality.
See e.g. [A Seven-Dimensional Analysis of Hashing Methods and its Implications on Query Processing](https://infosys.cs.uni-saarland.de/publications/p249-richter.pdf)
for a concise overview of the best hash table strategies, confirming that the
simplest Mult hashing (bernstein, FNV*, x17, sdbm) always beat "better" hash
functions (Tabulation, Murmur, Farm, ...) when used in a hash table.

The fast hash functions tested here are recommendable as fast for file
digests and maybe bigger databases, but not for 32bit hash tables.  The
"Quality problems" lead to less uniform distribution, i.e.  more collisions
and worse performance, but are rarely related to real security attacks, just
the 2nd sanity zeroes test against `\0` invariance is security relevant.

Columns
-------

MiB/sec: The average of the Bulk key speed test for alignments 0-7 with 262144-byte keys.
The higher the better.

cycl./hash: The average of the Small key speed test for 1-31 byte keys.
The smaller the better.

cycl./map: The result of the Hashmap test for /usr/dict/words with
std::unordered_map get queries, with the standard deviation in
brackets. This tests the inlinability of the hash function (see size).
The smaller the better.

size: The object size in byte on AMD64. This affects the inlinability in e.g. hash tables.
The smaller the better.

Quality problems: See the failures in the linked doc.
The less the better.

Other
-----

* [http://www.strchr.com/hash_functions](http://www.strchr.com/hash_functions) lists other benchmarks and quality of most simple and fast hash functions.
* [http://bench.cr.yp.to/primitives-hash.html](http://bench.cr.yp.to/primitives-hash.html) lists the benchmarks of all currently tested secure hashes.
* The [Hash Function Lounge](http://www.larc.usp.br/~pbarreto/hflounge.html) overviews the known weaknesses and attacks.


SECURITY
--------

The hash table attacks described in [SipHash](https://131002.net/siphash/)
against City, Murmur or Perl JenkinsOAAT or at
[Hash Function Lounge](http://www.larc.usp.br/~pbarreto/hflounge.html)
are not included here.

Such an attack avoidance cannot be the problem of the hash
function, but only the hash table collision resolution scheme.  You can
attack every single hash function, even the best and most secure if
you detect the seed, e.g. from language (mis-)features, side-channel
attacks, collision timings and independly the sort-order, so you need
to protect your collision handling scheme from the worst-case O(n),
i.e. separate chaining with linked lists. Linked lists chaining allows
high load factors, but is very cache-unfriendly.  The only
recommendable linked list scheme is inlining the key or hash into the
array. Nowadays everybody uses fast open addressing, even if the load
factor needs to be ~50%, unless you use Cuckoo Hashing.

I.e. the usage of SipHash for their hash table in Python 3.4, ruby,
rust, systemd, OpenDNS, Haskell and OpenBSD is pure security theatre.
SipHash is not secure enough for security purposes and not fast enough
for general usage. Brute-force generation of ~32k collisions need 2-4m
for all these hashes. siphash being the slowest needs max 4m, other
typically max 2m30s, with <10s for practical 16k collision attacks
with all hash functions.  Using Murmur is usually slower than a simple
Mult, even in the worst case.  Provable secure is only uniform
hashing, i.e. 2-5 independent Mult or Tabulation, or using a
guaranteed logarithmic collision scheme (a tree) or a linear collision
scheme, such as Robin Hood or Cockoo hashing with collision counting.

One more note regarding security: Nowadays even SHA1 can be solved in
a solver, like Z3 (or faster ones) for practical hash table collision
attacks (i.e. 14-20 bits). So all hash functions with less than 256
bits tested here cannot be considered "secure" at all.

The '\0' vulnerability attack with binary keys is tested in the 2nd
Sanity Zero test.
