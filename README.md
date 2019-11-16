SMhasher
========

| Hash function                                 |      MiB/sec |cycl./hash|cycl./map | size| Quality problems               |
|:----------------------------------------------|-------------:|---------:|---------:|----:|--------------------------------|
| [donothing32](doc/donothing32)                |  15050802.99 |     6.00 |    -     |  13 | test NOP                       |
| [donothing64](doc/donothing64)                |  15047620.56 |     6.00 |    -     |  13 | test NOP                       |
| [donothing128](doc/donothing128)              |  15101395.05 |     6.00 |    -     |  13 | test NOP                       |
| [NOP_OAAT_read64](doc/NOP_OAAT_read64)        |     30700.99 |    35.25 |    -     |  47 | test NOP                       |
| [BadHash](doc/BadHash)                        |       523.63 |    95.52 |    -     |  47 | test FAIL                      |
| [sumhash](doc/sumhash)                        |      7162.83 |    31.18 |    -     | 363 | test FAIL                      |
| [sumhash32](doc/sumhash32)                    |     21659.67 |    25.09 |    -     | 863 | test FAIL                      |
| [multiply_shift](doc/multiply_shift)          |      4910.24 |    45.32 | too slow | 345 | fails all tests                |
| [pair_multiply_shift](doc/pair_multiply_shift)|     12473.90 |    36.24 | too slow | 609 | fails all tests                |
| --------------------------                    |              |          |          |     |                                |
| [crc32](doc/crc32)                            |       353.40 |   142.74 | 408.60 (18)| 422 | insecure, 8590x collisions, distrib |
| [md5_32a](doc/md5_32a)                        |       352.19 |   668.69 |1053.14 (149)|4419| 8590x collisions, distrib |
| [sha1_32a](doc/sha1_32a)                      |       364.78 |  1514.25 |1967.21 (14)|5126 | collisions, 36.6% distrib   |
| [hasshe2](doc/hasshe2)                        |      2370.81 |    75.81 | 348.10 (6) | 445 | insecure, fails all tests    |
| [crc32_hw](doc/crc32_hw)                      |      6331.24 |    30.12 | 317.47 (13)| 653 | insecure, 100% bias, collisions, distrib, machine-specific (x86 SSE4.2) |
| [crc32_hw1](doc/crc32_hw1)                    |     23206.77 |    36.20 | 319.12 (4) | 671 | insecure, 100% bias, collisions, distrib, machine-specific (x86 SSE4.2) |
| [crc64_hw](doc/crc64_hw)                      |      8439.83 |    29.75 | 302.02 (8) | 652 | insecure, 100% bias, collisions, distrib, machine-specific (x86_64 SSE4.2) |
| [fibonacci](doc/fibonacci)                    |      9481.62 |    32.72 | 1366.07 (16)|1692| zeros, fails all tests     |
| [FNV1a](doc/FNV1a)                            |       791.82 |    69.32 | 326.71 (5) | 204 | zeros, fails all tests       |
| [FNV1A_Totenschiff](doc/FNV1A_Totenschiff)    |      6259.03 |    36.45 | 315.96 (3) | 270 | zeros, fails all tests       |
| [FNV1A_Pippip_Yurii](doc/FNV1A_Pippip_Yurii)  |      6258.47 |    36.98 | 311.08 (3) | 147 | sanity, fails all tests      |
| [FNV1a_YT](doc/FNV1a_YT)                      |      8960.87 |    28.00 | 314.20 (4) | 321 | fails all tests              |
| [FNV2](doc/FNV2)                              |      6258.87 |    38.01 | 300.60 (5) | 278 | fails all tests              |
| [FNV64](doc/FNV64)                            |       791.82 |    69.31 | 314.65 (5) |  79 | fails all tests              |
| [fletcher2](doc/fletcher2)                    |     11889.76 |    25.84 | 551.83 (8) | 248 | fails all tests              |
| [fletcher4](doc/fletcher4)                    |      5199.45 |    24.86 | 316.19 (16)| 371 | fails all tests              |
| [bernstein](doc/bernstein)                    |       791.81 |    66.63 | 320.65 (13)|  41 | fails all tests              |
| [sdbm](doc/sdbm)                              |       715.14 |    73.96 | 341.33 (3) |  41 | fails all tests              |
| [x17](doc/x17)                                |       527.90 |    96.71 | 323.50 (2) |  79 | 99.98% bias, fails all tests |
| [JenkinsOOAT](doc/JenkinsOOAT)                |       451.04 |   141.01 | 358.53 (4) | 153 | 53.5% bias, fails all tests  |
| [JenkinsOOAT_pl](doc/JenkinsOOAT_perl)        |       442.05 |   120.53 | 336.30 (4) |  65 | 1.5-11.5% bias, 7.2x collisions  |
| [MicroOAAT](doc/MicroOAAT)                    |       978.60 |    59.75 | 347.08 (7) |  68 | 100% bias, distrib           |
| [jodyhash32](doc/jodyhash32)                  |      1428.43 |    44.32 | 312.05 (4) | 102 | bias, collisions, distr      |
| [jodyhash64](doc/jodyhash64)                  |      2848.09 |    39.48 | 306.57 (4) | 118 | bias, collisions, distr      |
| [lookup3](doc/lookup3)                        |      1754.08 |    46.85 | 320.00 (4) | 341 | 28% bias, collisions, 30% distr  |
| [superfast](doc/superfast)                    |      1580.94 |    57.69 | 319.81 (3) | 210 | 91% bias, 5273.01x collisions, 37% distr |
| [MurmurOAAT](doc/MurmurOAAT)                  |       451.66 |   114.34 | 338.72 (3) |  47 | collisions, 99.998% distr          |
| [Crap8](doc/Crap8)                            |      2845.72 |    37.66 | 334.59 (4) | 342 | 2.42% bias, collisions, 2% distrib |
| [Murmur2](doc/Murmur2)                        |      3125.31 |    40.66 | 324.43 (18)| 358 | 1.7% bias, 81x coll, 1.7% distrib  |
| [Murmur2A](doc/Murmur2A)                      |      3126.02 |    45.27 | 320.96 (5) | 407 | 12.7% bias                         |
| [Murmur2B](doc/Murmur2B)                      |      4866.96 |    46.47 | 313.25 (5) | 433 | 1.8% bias, collisions, 3.4% distrib|
| [Murmur2C](doc/Murmur2C)                      |      3933.68 |    46.34 | 337.43 (5) | 444 | 91% bias, collisions, distr        |
| [Murmur3C](doc/Murmur3C)                      |      3144.69 |    66.54 | 348.81 (4) | 859 | LongNeighbors              |
| [xxHash32](doc/xxHash32)                      |      6043.80 |    47.58 | 315.95 (4) | 738 | LongNeighbors, collisions with 4bit diff |
| [metrohash64_1](doc/metrohash64_1)            |      9554.13 |    49.34 | 311.23 (4) | 624 | LongNeighbors              |
| [metrohash64_2](doc/metrohash64_2)            |      9344.42 |    49.53 | 309.96 (4) | 627 | LongNeighbors              |
| [metrohash64crc_1](doc/metrohash64crc_1)      |     13983.88 |    54.55 | 306.74 (6) | 632 | cyclic collisions 8 byte, machine-specific (x64 SSE4.2) |
| [metrohash64crc_2](doc/metrohash64crc_2)      |     13833.18 |    54.52 | 312.60 (5) | 632 | cyclic collisions 8 byte, machine-specific (x64  SSE4.2) |
| [metrohash128_1](doc/metrohash128_1)          |      9567.09 |    77.39 | 352.65 (10)| 773 | LongNeighbors              |
| [metrohash128_2](doc/metrohash128_2)          |      9542.49 |    77.35 | 340.90 (4) | 773 | LongNeighbors              |
| [cmetrohash64_1o](doc/cmetrohash64_1o)        |      9124.84 |    49.25 | 313.42 (7) |3506 | LongNeighbors              |
| [cmetrohash64_1](doc/cmetrohash64_1)          |      9579.01 |    49.32 | 313.12 (6) | 652 | LongNeighbors              |
| [cmetrohash64_2](doc/cmetrohash64_2)          |      9560.78 |    49.62 | 305.45 (4) | 655 | LongNeighbors              |
| [City64noSeed](doc/City64noSeed)              |      9181.88 |    38.61 | 315.06 (8) |1038 |                            |
| [City64](doc/City64)                          |      8225.28 |    61.77 | 354.65 (6) |1120 | 2 minor collisions         |
| [falkhash](doc/falkhash)                      |     20026.92 |   169.65 | 461.36 (4) | 264 | LongNeighbors, machine-specific (x86_64 AES-NI) |
| [t1ha2_atonce128](doc/t1ha2_atonce128)        |      8442.97 |    63.97 | 352.21 (4) | 613 | LongNeighbors              |
| [t1ha2_stream128](doc/t1ha2_stream128)        |      8902.68 |   116.08 | 420.07 (4) |1665 | LongNeighbors              |
| [HalfSipHash](doc/HalfSipHash)                |       744.80 |   122.87 | 381.83 (4) | 700 | zeroes                     |
| [SipHash13](doc/SipHash13)                    |      1591.58 |   113.47 | 412.03 (6) | 778 | 0.9% bias                  |
| --------------------------------------        |              |          |            |     |                            |
| [SipHash](doc/SipHash)                        |       868.00 |   156.95 | 450.46 (3) |1071 |                            |
| [HighwayHash64](doc/HighwayHash64)            |      5644.02 |   109.23 | 429.07 (4) |2546 |                            |
| [GoodOAAT](doc/GoodOAAT)                      |       950.12 |    78.83 | 357.45 (3) | 237 |                            |
| [TSip](doc/TSip)                              |      3023.80 |    66.43 | 370.48 (5) | 519 | !msvc                      |
| [seahash](doc/seahash)                        |      3827.79 |    75.22 | 370.83 (4) | 871 | !msvc                      |
| [seahash32low](doc/seahash32low)              |      4234.67 |    64.47 | 417.73 (71)| 871 | !msvc                      |
| [PMurHash32](doc/PMurHash32)                  |      2123.02 |    63.02 | 366.26 (2) |1862 | Moment Chi2 69             |
| [Murmur3A](doc/Murmur3A)                      |      2183.98 |    55.40 | 348.50 (4) | 351 | Moment Chi2 69             |
| [Murmur3F](doc/Murmur3F)                      |      4724.06 |    55.17 | 362.62 (4) | 699 |                            |
| [fasthash32](doc/fasthash32)                  |      4209.33 |    56.07 | 347.88 (4) | 566 |                            |
| [fasthash64](doc/fasthash64)                  |      4209.36 |    52.76 | 345.63 (4) | 509 | Moment Chi2 5159 !         |
| [MUM](doc/MUM)                                |      6269.69 |    44.17 | 350.07 (4) |1912 | machine-specific (32/64 differs) |
| [MUMlow](doc/MUMlow)                          |      6892.86 |    46.35 | 331.66 (3) |1912 |           |                            |
| [mirhash](doc/mirhash)                        |      4926.94 |    46.72 | 344.40 (4) | 1112 | machine-specific (32/64 differs) |
| [mirhash32low](doc/mirhash32low)              |      5454.01 |    42.33 | 316.46 (3) | 1112 | machine-specific (32/64 differs) |
| [mirhashstrict](doc/mirhashstrict)            |      2003.63 |    71.66 | 348.81 (5) |1112 |                            |
| [mirhashstrict32low](doc/mirhashstrict32low)  |      2016.85 |    70.89 | 302.77 (4) |1112 |                            |
| [City32](doc/City32)                          |      3505.99 |    58.60 | 361.98 (4) |1319 |                            |
| [City64low](doc/City64low)                    |      8249.14 |    68.44 | 454.65 (104)|1120 |                           |
| [City128](doc/City128)                        |      9050.33 |    82.70 | 380.35 (4) |1841 |                            |
| [CityCrc128](doc/CityCrc128)                  |     12011.76 |    83.02 | 381.41 (10)| 295 |                            |
| [FarmHash32](doc/FarmHash32)                  |     15288.62 |    68.36 | 376.03 (4) |11489| machine-specific (x86_64 SSE4/AVX) |
| [FarmHash64](doc/FarmHash64)                  |      7866.95 |    69.71 | 369.97 (9) |3758 |                            |
| [FarmHash128](doc/FarmHash128)                |      8881.00 |    90.84 | 402.15 (4) | 163 |                            |
| [farmhash32_c](doc/farmhash32_c)              |     16592.05 |    68.40 | 376.51 (4) | 762 | machine-specific (x86_64 SSE4/AVX) |
| [farmhash64_c](doc/farmhash64_c)              |      7809.77 |    82.50 | 379.30 (5) |3688 |                            |
| [farmhash128_c](doc/farmhash128_c)            |      9161.54 |   109.29 | 421.26 (3) |1890 |                            |
| [xxHash64](doc/xxHash64)                      |      8149.18 |    63.14 | 337.70 (5) |1999 |                            |
| [xxh3](doc/xxh3)                              |     12410.56 |    41.29 | 343.44 (4) | 744 | Moment Chi2 14974 !        |
| [xxh3low](doc/xxh3low)                        |     12781.94 |    41.03 | 344.55 (4) | 756 | Moment Chi2 1.8e+9 !       |
| [xxh128](doc/xxh128)                          |     12346.93 |    45.75 | 340.66 (4) |1012 |                            |
| [xxh128low](doc/xxh128low)                    |     12403.53 |    41.41 | 328.56 (5) |1012 | Moment Chi2 14974 !        |
| [Spooky32](doc/Spooky32)                      |      8891.87 |    75.24 | 437.09 (158)|2221 |                            |
| [Spooky64](doc/Spooky64)                      |      8917.09 |    75.15 | 364.59 (5) |2221 |                            |
| [Spooky128](doc/Spooky128)                    |      8919.58 |    78.24 | 372.23 (4) |2221 |                            |
| [metrohash128crc_1](doc/metrohash128crc_1)    |     12806.19 |    92.48 | 365.92 (11)| 723 | machine-specific (x64 SSE4.2) |
| [metrohash128crc_2](doc/metrohash128crc_2)    |     12578.00 |    92.46 | 368.34 (6) | 723 | machine-specific (x64 SSE4.2) |
| [clhash](doc/clhash)                          |      3668.68 |    90.64 | 416.01 (5) |1809 | machine-specific (x64 SSE4.2) |
| [t1ha2_atonce](doc/t1ha2_atonce)              |      7633.86 |    51.92 | 346.17 (6) | 541 |                            |
| [t1ha2_stream](doc/t1ha2_stream)              |      8010.07 |    99.57 | 415.86 (6) |1665 |                            |
| [t1ha1_64le](doc/t1ha1_64le)                  |      8599.49 |    41.83 | 341.04 (14)| 517 |                            |
| [t1ha1_64be](doc/t1ha1_64be)                  |      6532.71 |    44.00 | 352.59 (3) | 555 |                            |
| [t1ha0_32le](doc/t1ha0_32le)                  |      4580.62 |    59.09 | 348.05 (13)| 509 |                            |
| [t1ha0_32be](doc/t1ha0_32be)                  |      3937.01 |    59.19 | 355.43 (5) | 533 |                            |
| [t1ha0_aes_noavx](doc/t1ha0_aes_noavx)        |     18221.09 |    51.46 | 337.99 (3) | 925 | machine-specific (x86 AES-NI) |
| [t1ha0_aes_avx1](doc/t1ha0_aes_avx1)          |     19583.35 |    51.65 | 336.56 (5) | 843 | machine-specific (x64 AVX)    |
| [t1ha0_aes_avx2](doc/t1ha0_aes_avx2)          |     36436.51 |    36.31 | ?          | 792 | machine-specific (x64 AVX2)   |
| [wyhash](doc/wyhash)                          |     11285.57 |    31.34 | 341.74 (5) |2546 |                            |
| [wyhash32low](doc/wyhash32low)                |     11322.31 |    30.41 | 429.22 (104)|2546 | Moment Chi2 4.2            |

Summary
-------

I added some SSE assisted hashes and fast intel/arm CRC32-C and AES HW variants, but not the fastest
[crcutil](https://code.google.com/p/crcutil/) yet. See [our crcutil results](https://github.com/rurban/smhasher/blob/master/doc/crcutil).
See also the old [https://code.google.com/p/smhasher/w/list](https://code.google.com/p/smhasher/w/list).

So the fastest hash functions on x86_64 without quality problems are:

- wyhash v3
- xxh3 (faster with random len)
- t1ha2_atonce, t1ha1_64le
- metrohash64crc
- FarmHash (_not portable, too machine specific: 64 vs 32bit, old gcc, ..._)
- Spooky32
- fasthash
- MUM (_machine specific, mum: different results on 32/64-bit archs_)

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

TODO
----

Some slower cryptographic hashes or more secure hashes are still
missing. BLAKE2, SHA-2, SHA-3 (Keccak), Grøstl, JH, Skein, ...
They will pass all tests, and are way too slow compared to our
candidates here.

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
