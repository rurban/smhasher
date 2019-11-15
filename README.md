SMhasher
========

| Hash function                         |      MiB/sec  |cycl./hash|cycl./map | size| Quality problems                |
|:--------------------------------------|--------------:|---------:|---------:|----:|--------------------------------|
| [donothing32](doc/donothing32)        |  36889069.95  |     5.20 |    -     |  13 | test NOP                       |
| [donothing64](doc/donothing64)        |  26608963.56  |     5.20 |    -     |  13 | test NOP                       |
| [donothing128](doc/donothing128)      |  36577798.02  |     5.24 |    -     |  13 | test NOP                       |
| [NOP_OAAT_read64](doc/NOP_OAAT_read64)|     28724.57  |    34.72 |    -     |  47 | test NOP                       |
| [BadHash](doc/BadHash)                |       524.50  |    95.36 |    -     |  47 | test FAIL                      |
| [sumhash](doc/sumhash)                |      7162.98  |    31.10 |    -     | 363 | test FAIL                      |
| [sumhash32](doc/sumhash32)            |     23472.74  |    22.88 |    -     | 863 | test FAIL                      |
| [multiply_shift](doc/multiply_shift)  |      4818.75  |    45.99 | skipped  | 345 | fails all tests              |
| [pair_multiply_shift](doc/pair_multiply_shift)| 13635.23|  32.96 | skipped  | 609 | fails all tests              |
| --------------------------            |               |          |          |     |                                |
| [crc32](doc/crc32)                    |       392.06  |   129.92 | 342.08 (15)| 422 | insecure, 8590x collisions, distrib |
| [md5_32a](doc/md5_32a)                |       352.19  |   668.69 |1053.14 (149)|4419 | 8590x collisions, distrib |
| [sha1_32a](doc/sha1_32a)              |       364.78  |  1514.25 |1967.21 (14)|5126 | collisions, 36.6% distrib   |
| [hasshe2](doc/hasshe2)                |      2325.03  |    77.15 | 349.99 (6)| 445 | insecure, fails all tests    |
| [crc32_hw](doc/crc32_hw)              |      6216.21  |    30.45 | 308.09 (5)| 653 | insecure, 100% bias, collisions, distrib, machine-specific (x86 SSE4.2) |
| [crc32_hw1](doc/crc32_hw1)            |     22324.27  |    36.62 | 324.70 (5)| 671 | insecure, 100% bias, collisions, distrib, machine-specific (x86 SSE4.2) |
| [crc64_hw](doc/crc64_hw)              |      8282.08  |    30.02 | 293.76 (5)| 652 | insecure, 100% bias, collisions, distrib, machine-specific (x86_64 SSE4.2) |
| [fibonacci](doc/fibonacci)            |      9292.44  |    33.24 | 1377.87 (6) |1692 | zeros, fails all tests     |
| [FNV1a](doc/FNV1a)                    |        774.91 |    70.52 | 316.55 (4)| 204 | zeros, fails all tests       |
| [FNV1A_Totenschiff](doc/FNV1A_Totenschiff)|   6142.04 |    36.84 | 326.69 (7)| 270 | zeros, fails all tests       |
| [FNV1A_Pippip_Yurii](doc/FNV1A_Pippip_Yurii)| 6260.14 |    36.87 | 316.83 (6)| 147 | sanity, fails all tests      |
| [FNV1a_YT](doc/FNV1a_YT)              |       8783.51 |    28.35 | 306.86 (3)| 321 | fails all tests              |
| [FNV2](doc/FNV2)                      |       6143.11 |    38.91 | 314.15 (4)| 278 | fails all tests              |
| [FNV64](doc/FNV64)                    |        774.56 |    70.52 | 301.66 (4)|  79 | fails all tests              |
| [fletcher2](doc/fletcher2)            |      11773.15 |    26.06 | 595.88 (3)| 248 | fails all tests              |
| [fletcher4](doc/fletcher4)            |       5104.48 |    24.77 | 296.67 (4)| 371 | fails all tests              |
| [bernstein](doc/bernstein)            |        774.87 |    67.74 | 347.97 (3)|  41 | fails all tests              |
| [sdbm](doc/sdbm)                      |        774.46 |    67.74 | 338.64 (5)|  41 | fails all tests              |
| [x17](doc/x17)                        |        515.66 |    98.30 | 350.15 (3)|  79 | 99.98% bias, fails all tests |
| [JenkinsOOAT](doc/JenkinsOOAT)        |        442.12 |   143.56 | 384.73 (2)| 153 | 53.5% bias, fails all tests  |
| [JenkinsOOAT_pl](doc/JenkinsOOAT_perl)|        442.05 |   120.53 | 336.30 (4)|  65 | 1.5-11.5% bias, 7.2x collisions  |
| [MicroOAAT](doc/MicroOAAT)            |        960.78 |    60.71 | 319.82 (4)|  68 | 100% bias, distrib           |
| [jodyhash32](doc/jodyhash32)          |       1400.31 |    44.98 | 333.17 (4)| 102 | bias, collisions, distr      |
| [jodyhash64](doc/jodyhash64)          |       2794.20 |    40.30 | 311.01 (13) | 118 | bias, collisions, distr    |
| [lookup3](doc/lookup3)                |       1718.05 |    47.46 | 314.46 (4)| 341 | 28% bias, collisions, 30% distr  |
| [superfast](doc/superfast)            |       1549.70 |    58.59 | 325.88 (4)| 210 | 91% bias, 5273.01x collisions, 37% distr |
| [MurmurOAAT](doc/MurmurOAAT)          |        445.94 |   114.28 | 336.35 (4)|  47 | collisions, 99.998% distr          |
| [Crap8](doc/Crap8)                    |       3121.92 |    34.33 | 319.94 (5)| 342 | 2.42% bias, collisions, 2% distrib |
| [Murmur2](doc/Murmur2)                |       3111.73 |    40.97 | 321.58 (5)| 358 | 1.7% bias, 81x coll, 1.7% distrib  |
| [Murmur2A](doc/Murmur2A)              |       3112.94 |    45.53 | 311.23 (3)| 407 | 12.7% bias                         |
| [Murmur2B](doc/Murmur2B)              |       4824.52 |    46.78 | 323.74 (6)| 433 | 1.8% bias, collisions, 3.4% distrib|
| [Murmur2C](doc/Murmur2C)              |       3902.16 |    46.76 | 314.43 (5)| 444 | 91% bias, collisions, distr        |
| [Murmur3C](doc/Murmur3C)              |       3120.97 |    66.80 | 318.68 (6)| 859 | LongNeighbors              |
| [xxHash32](doc/xxHash32)              |       6051.56 |    47.59 | 354.30 (5)| 738 | LongNeighbors, collisions with 4bit diff |
| [metrohash64_1](doc/metrohash64_1)    |       9328.47 |    49.28 | 310.21 (4)| 624 | LongNeighbors              |
| [metrohash64_2](doc/metrohash64_2)    |       9574.39 |    49.45 | 310.10 (6)| 627 | LongNeighbors              |
| [metrohash64crc_1](doc/metrohash64crc_1)|    13970.56 |    54.34 | 344.17 (14)| 632 | cyclic collisions 8 byte, machine-specific (x64 SSE4.2) |
| [metrohash64crc_2](doc/metrohash64crc_2)|    13881.83 |    54.34 | 346.52 (5)| 632 | cyclic collisions 8 byte, machine-specific (x64  SSE4.2) |
| [metrohash128_1](doc/metrohash128_1)  |       9557.24 |    77.21 | 315.67 (14)| 773 | LongNeighbors              |
| [metrohash128_2](doc/metrohash128_2)  |       9559.81 |    77.18 | 309.57 (4)| 773 | LongNeighbors              |
| [cmetrohash64_1o](doc/cmetrohash64_1o)|       9135.34 |    49.01 | 305.10 (4)|3506 | LongNeighbors              |
| [cmetrohash64_1](doc/cmetrohash64_1)  |       9574.61 |    49.24 | 308.42 (5)| 652 | LongNeighbors              |
| [cmetrohash64_2](doc/cmetrohash64_2)  |       9567.94 |    49.51 | 313.38 (6)| 655 | LongNeighbors              |
| [City64noSeed](doc/City64noSeed)      |       9207.73 |    38.69 | 308.08 (15)|1038 |                            |
| [City64](doc/City64)                  |       9086.82 |    55.99 | 324.46 (4)|1120 | 2 minor collisions         |
| [falkhash](doc/falkhash)              |      20355.06 |   169.62 | 466.78 (6)| 264 | LongNeighbors, machine-specific (x86_64 AES-NI) |
| [t1ha2_atonce128](doc/t1ha2_atonce128)|       8469.18 |    63.76 | 346.96 (5)| 613 | LongNeighbors              |
| [t1ha2_stream128](doc/t1ha2_stream128)|       8905.72 |   115.38 | 416.44 (5)|1665 | LongNeighbors              |
| [HalfSipHash](doc/HalfSipHash)        |        747.09 |   123.52 | 386.21 (4)| 700 | zeroes                     |
| [SipHash13](doc/SipHash13)            |       1748.73 |   103.22 | 380.05 (14)| 778 | 0.9% bias                 |
| --------------------------------------|               |          |           |     |                            |
| [SipHash](doc/SipHash)                |       953.74  |   142.65 | 406.63 (3)|1071 |                            |
| [HighwayHash64](doc/HighwayHash64)    |      6194.33  |    99.06 | 397.56 (6)|2546 |                            |
| [GoodOAAT](doc/GoodOAAT)              |      1049.83  |    71.56 | 324.00 (4)| 237 |                            |
| [TSip](doc/TSip)                      |      3284.28  |    61.10 | 347.05 (9)| 519 | !msvc                      |
| [seahash](doc/seahash)                |      4236.59  |    64.37 | 336.60 (5)| 871 | !msvc                      |
| [seahash32low](doc/seahash32low)      |      4234.97  |    64.37 | 415.25 (74)|871 | !msvc                      |
| [PMurHash32](doc/PMurHash32)          |      2329.73  |    57.27 | 342.57 (4)|1862 | Moment Chi2 69             |
| [Murmur3A](doc/Murmur3A)              |      2402.80  |    50.29 | 327.60 (5)| 351 | Moment Chi2 69             |
| [Murmur3F](doc/Murmur3F)              |      5179.38  |    49.98 | 357.37 (5)| 699 |                            |
| [fasthash32](doc/fasthash32)          |      4621.08  |    50.70 | 323.96 (5)| 566 |                            |
| [fasthash64](doc/fasthash64)          |      4621.33  |    47.89 | 315.76 (17)| 509 | Moment Chi2 5159 !         |
| [MUM](doc/MUM)                        |      6902.42  |    40.16 | 316.02 (4)|1912 | machine-specific (32/64 differs) |
| [MUMlow](doc/MUMlow)                  |      6840.70  |    46.22 | 328.42 (2)|1912 |           |                            |
| [mirhash](doc/mirhash)                |      5310.60	|    43.51 | 296.56 (3)| 1112 | machine-specific (32/64 differs) |
| [mirhash32low](doc/mirhash32low)      |      5337.43	|    42.95 | 312.09 (3)| 1112 | machine-specific (32/64 differs) |
| [mirhashstrict](doc/mirhashstrict)    |      2006.89  |    71.61 | 322.50 (10)|1112 |                            |
| [mirhashstrict32low](doc/mirhashstrict32low)|2016.85	|    70.89 | 302.77 (4) |1112 |                            |
| [City32](doc/City32)                  |      3800.14  |    53.12 | 331.86 (15)|1319 |                            |
| [City64low](doc/City64low)            |      9104.84  |    62.12 | 406.76 (146)|1120 |                           |
| [City128](doc/City128)                |      9917.86  |    75.12 | 343.65 (5) |1841 |                            |
| [CityCrc128](doc/CityCrc128)          |     13218.72  |    75.27 | 346.39 (3) | 295 |                            |
| [FarmHash32](doc/FarmHash32)         	|     24831.45  |    24.99 | 343.12 (14)|11489| machine-specific (x86_64 SSE4/AVX) |
| [FarmHash64](doc/FarmHash64)         	|      8756.99  |    63.40 | 331.99 (4) |3758 |                            |
| [FarmHash128](doc/FarmHash128)        |      9772.55  |    82.70 | 364.23 (5) | 163 |                            |
| [farmhash32_c](doc/farmhash32_c)      |     24647.21  |    25.36 | 342.08 (3) | 762 | machine-specific (x86_64 SSE4/AVX) |
| [farmhash64_c](doc/farmhash64_c)     	|      8620.92  |    75.04 | 373.54 (4) |3688 |                            |
| [farmhash128_c](doc/farmhash128_c)    |     10071.77  |    97.70 | 415.70 (5) |1890 |                            |
| [xxHash64](doc/xxHash64)              |      9031.87  |    57.10 | 336.10 (4) |1999 |                            |
| [xxh3](doc/xxh3)                      |     14169.25  |    37.01 | 325.67 (4) | 744 | Moment Chi2 14974 !        |
| [xxh3low](doc/xxh3low)                |     13865.81  |    36.89 | 307.56 (4) | 756 | Moment Chi2 1.8e+9 !       |
| [xxh128](doc/xxh128)                  |     14151.13  |    41.32 | 363.44 (26)|1012 |                            |
| [xxh128low](doc/xxh128low)            |     13811.03  |    38.57 | 329.66 (5) |1012 | Moment Chi2 14974 !        |
| [Spooky32](doc/Spooky32)              |      9563.88  |    70.22 | 426.41 (162)|2221 |                            |
| [Spooky64](doc/Spooky64)              |      9548.01  |    69.33 | 357.40 (5) |2221 |                            |
| [Spooky128](doc/Spooky128)            |      9898.77  |    70.72 | 342.44 (16)|2221 |                            |
| [metrohash128crc_1](doc/metrohash128crc_1)| 13867.67  |    83.29 | 338.66 (3) | 723 | machine-specific (x64 SSE4.2) |
| [metrohash128crc_2](doc/metrohash128crc_2)| 14224.59  |    83.26 | 365.25 (9) | 723 | machine-specific (x64 SSE4.2) |
| [clhash](doc/clhash)                  |      4070.68  |    81.56 | 373.09 (5) |1809 | machine-specific (x64 SSE4.2) |
| [t1ha2_atonce](doc/t1ha2_atonce)      |      8459.94  |    46.74 | 315.28 (4) | 541 |                            |
| [t1ha2_stream](doc/t1ha2_stream)      |      8884.03  |    91.16 | 414.67 (5) |1665 |                            |
| [t1ha1_64le](doc/t1ha1_64le)          |      9526.17  |    37.65 | 313.70 (5) | 517 |                            |
| [t1ha1_64be](doc/t1ha1_64be)          |      7240.97  |    39.77 | 318.98 (8) | 555 |                            |
| [t1ha0_32le](doc/t1ha0_32le)          |      5074.19  |    53.07 | 318.20 (4) | 509 |                            |
| [t1ha0_32be](doc/t1ha0_32be)          |      4359.59  |    53.20 | 322.56 (4) | 533 |                            |
| [t1ha0_aes_noavx](doc/t1ha0_aes_noavx)|     20547.70  |    46.60 | 325.61 (7) | 925 | machine-specific (x86 AES-NI) |
| [t1ha0_aes_avx1](doc/t1ha0_aes_avx1)  |     22278.29  |    46.58 | 319.81 (6) | 843 | machine-specific (x64 AVX)    |
| [t1ha0_aes_avx2](doc/t1ha0_aes_avx2)  |     36436.51	|    36.31 | ?          | 792 | machine-specific (x64 AVX2)   |
| [wyhash](doc/wyhash)                  |     12327.50  |    34.98 | 347.00 (5)  |2546 |                            |
| [wyhash32low](doc/wyhash32low)        |     12295.53  |    35.01 | 447.85 (107)|2546 | Moment Chi2 4.2            |

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
