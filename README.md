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
| [crc32](doc/crc32)                            |       353.40 |   142.74 | 408.60 (18) | 422 | insecure, 8590x collisions, distrib |
| [md5_32a](doc/md5_32a)                        |       352.19 |   668.69 |1053.14 (149)|4419| 8590x collisions, distrib |
| [sha1_32a](doc/sha1_32a)                      |       364.78 |  1514.25 |1967.21 (14)|5126 | collisions, 36.6% distrib   |
| [hasshe2](doc/hasshe2)                        |      2370.81 |    75.81 | 348.10 (6) | 445 | insecure, fails all tests    |
| [crc32_hw](doc/crc32_hw)                      |      6331.24 |    30.12 | 317.47 (13) | 653 | insecure, 100% bias, collisions, distrib, machine-specific (x86 SSE4.2) |
| [crc32_hw1](doc/crc32_hw1)                    |     23206.77 |    36.20 | 184.95 (2) | 671 | insecure, 100% bias, collisions, distrib, machine-specific (x86 SSE4.2) |
| [crc64_hw](doc/crc64_hw)                      |      8439.83 |    29.75 | 150.62 (2) | 652 | insecure, 100% bias, collisions, distrib, machine-specific (x86_64 SSE4.2) |
| [fibonacci](doc/fibonacci)                    |      9481.62 |    32.72 | 755.13 (4) |1692| zeros, fails all tests     |
| [FNV1a](doc/FNV1a)                            |       791.82 |    69.32 | 178.74 (2) | 204 | zeros, fails all tests       |
| [FNV1A_Totenschiff](doc/FNV1A_Totenschiff)    |      6259.03 |    36.45 | 188.19 (2) | 270 | zeros, fails all tests       |
| [FNV1A_Pippip_Yurii](doc/FNV1A_Pippip_Yurii)  |      6258.47 |    36.98 | 185.94 (2) | 147 | sanity, fails all tests      |
| [FNV1a_YT](doc/FNV1a_YT)                      |      8960.87 |    28.00 | 207.81 (32) | 321 | fails all tests              |
| [FNV2](doc/FNV2)                              |      6258.87 |    38.01 | 157.38 (4) | 278 | fails all tests              |
| [FNV64](doc/FNV64)                            |       791.82 |    69.31 | 167.14 (3) |  79 | fails all tests              |
| [fletcher2](doc/fletcher2)                    |     11889.76 |    25.84 | 303.71 (5) | 248 | fails all tests              |
| [fletcher4](doc/fletcher4)                    |      5199.45 |    24.86 | 148.68 (2) | 371 | fails all tests              |
| [bernstein](doc/bernstein)                    |       791.81 |    66.63 | 206.74 (8) |  41 | fails all tests              |
| [sdbm](doc/sdbm)                              |       715.14 |    73.96 | 205.15 (11) |  41 | fails all tests              |
| [x17](doc/x17)                                |       527.90 |    96.71 | 207.18 (12) |  79 | 99.98% bias, fails all tests |
| [JenkinsOOAT](doc/JenkinsOOAT)                |       451.04 |   141.01 | 226.88 (10) | 153 | 53.5% bias, fails all tests  |
| [JenkinsOOAT_pl](doc/JenkinsOOAT_perl)        |       442.05 |   120.53 | 336.30 (4) |  65 | 1.5-11.5% bias, 7.2x collisions  |
| [HalfSipHash](doc/HalfSipHash)                |       744.80 |   122.87 | 250.74 (3) | 700 | zeroes                     |
| [SipHash13](doc/SipHash13)                    |      1591.58 |   113.47 | 267.62 (26) | 778 | 0.9% bias                  |
| [MicroOAAT](doc/MicroOAAT)                    |       978.60 |    59.75 | 198.06 (4) |  68 | 100% bias, distrib           |
| [jodyhash32](doc/jodyhash32)                  |      1428.43 |    44.32 | 197.12 (2) | 102 | bias, collisions, distr      |
| [jodyhash64](doc/jodyhash64)                  |      2848.09 |    39.48 | 167.69 (3) | 118 | bias, collisions, distr      |
| [lookup3](doc/lookup3)                        |      1754.08 |    46.85 | 188.51 (4) | 341 | 28% bias, collisions, 30% distr  |
| [superfast](doc/superfast)                    |      1580.94 |    57.69 | 186.18 (4) | 210 | 91% bias, 5273.01x collisions, 37% distr |
| [MurmurOAAT](doc/MurmurOAAT)                  |       451.66 |   114.34 | 193.55 (3) |  47 | collisions, 99.998% distr          |
| [Crap8](doc/Crap8)                            |      2845.72 |    37.66 | 200.40 (4) | 342 | 2.42% bias, collisions, 2% distrib |
| [Murmur2](doc/Murmur2)                        |      3125.31 |    40.66 | 180.19 (4) | 358 | 1.7% bias, 81x coll, 1.7% distrib  |
| [Murmur2A](doc/Murmur2A)                      |      3126.02 |    45.27 | 195.24 (3) | 407 | 12.7% bias                         |
| [Murmur2B](doc/Murmur2B)                      |      4866.96 |    46.47 | 167.83 (2) | 433 | 1.8% bias, collisions, 3.4% distrib|
| [Murmur2C](doc/Murmur2C)                      |      3933.68 |    46.34 | 162.75 (2) | 444 | 91% bias, collisions, distr        |
| [Murmur3C](doc/Murmur3C)                      |      3144.69 |    66.54 | 212.12 (3) | 859 | LongNeighbors              |
| [PMurHash32](doc/PMurHash32)                  |      2123.02 |    63.02 | 200.56 (2) |1862 | Moment Chi2 69             |
| [Murmur3A](doc/Murmur3A)                      |      2183.98 |    55.40 | 203.40 (6) | 351 | Moment Chi2 69             |
| [Murmur3F](doc/Murmur3F)                      |      4724.06 |    55.17 | 184.74 (12) | 699 | Sparse for low 32bit       |
| [xxHash32](doc/xxHash32)                      |      6043.80 |    47.58 | 190.14 (4) | 738 | LongNeighbors, collisions with 4bit diff |
| [metrohash64_1](doc/metrohash64_1)            |      9554.13 |    49.34 | 171.06 (2) | 624 | LongNeighbors              |
| [metrohash64_2](doc/metrohash64_2)            |      9344.42 |    49.53 | 169.46 (4) | 627 | LongNeighbors              |
| [metrohash64crc_1](doc/metrohash64crc_1)      |     13983.88 |    54.55 | 173.34 (6) | 632 | cyclic collisions 8 byte, machine-specific (x64 SSE4.2) |
| [metrohash64crc_2](doc/metrohash64crc_2)      |     13833.18 |    54.52 | 179.84 (1) | 632 | cyclic collisions 8 byte, machine-specific (x64  SSE4.2) |
| [metrohash128crc_1](doc/metrohash128crc_1)    |     12806.19 |    92.48 | 227.09 (7) | 723 | low 32bit coll. machine-specific (x64 SSE4.2) |
| [metrohash128crc_2](doc/metrohash128crc_2)    |     12578.00 |    92.46 | 216.47 (7) | 723 | low 32bit coll. machine-specific (x64 SSE4.2) |
| [metrohash128_1](doc/metrohash128_1)          |      9567.09 |    77.39 | 224.06 (16) | 773 | LongNeighbors              |
| [metrohash128_2](doc/metrohash128_2)          |      9542.49 |    77.35 | 211.07 (2) | 773 | LongNeighbors              |
| [cmetrohash64_1o](doc/cmetrohash64_1o)        |      9124.84 |    49.25 | 174.25 (2) |3506 | LongNeighbors              |
| [cmetrohash64_1](doc/cmetrohash64_1)          |      9579.01 |    49.32 | 174.25 (2) | 652 | LongNeighbors              |
| [cmetrohash64_2](doc/cmetrohash64_2)          |      9560.78 |    49.62 | 175.39 (2) | 655 | LongNeighbors              |
| [City64noSeed](doc/City64noSeed)              |      9181.88 |    38.61 | 198.95 (3) |1038 |                            |
| [City64](doc/City64)                          |      8225.28 |    61.77 | 211.81 (6) |1120 | 2 minor collisions         |
| [falkhash](doc/falkhash)                      |     20026.92 |   169.65 | 363.27 (4) | 264 | LongNeighbors, machine-specific (x86_64 AES-NI) |
| [FarmHash128](doc/FarmHash128)                |      8881.00 |    90.84 | 222.32 (3) | 163 | low 32bit coll.            |
| [farmhash128_c](doc/farmhash128_c)            |      9161.54 |   109.29 | 258.95 (3) |1890 | low 32bit coll.            |
| [t1ha2_atonce128](doc/t1ha2_atonce128)        |      8442.97 |    63.97 | 201.80 (4) | 613 | LongNeighbors              |
| [t1ha2_stream128](doc/t1ha2_stream128)        |      8902.68 |   116.08 | 295.90 (7) |1665 | LongNeighbors              |
| [Spooky128](doc/Spooky128)                    |      8919.58 |    78.24 | 216.43 (8) |2221 | low 32bit coll.            |
| [xxh3](doc/xxh3)                              |     12410.56 |    41.29 | 211.22 (12) | 744 | Moment Chi2 14974 ! low 32bit coll |
| [xxh3low](doc/xxh3low)                        |     12781.94 |    41.03 | 205.65 (4) | 756 | Moment Chi2 1.8e+9 ! low 32bit coll|
| [xxh128](doc/xxh128)                          |     12346.93 |    45.75 | 196.60 (4) |1012 | low 32bit coll.            |
| [xxh128low](doc/xxh128low)                    |     12403.53 |    41.41 | 197.96 (4) |1012 | Moment Chi2 14974 ! low 32bit coll |
| --------------------------------------        |              |          |            |     |                            |
| [SipHash](doc/SipHash)                        |       868.00 |   156.95 | 300.59 (2) |1071 |                            |
| [HighwayHash64](doc/HighwayHash64)            |      5644.02 |   109.23 | 261.60 (2) |2546 |                            |
| [GoodOAAT](doc/GoodOAAT)                      |       950.12 |    78.83 | 223.57 (4) | 237 |                            |
| [TSip](doc/TSip)                              |      3023.80 |    66.43 | 204.56 (2) | 519 | !msvc                      |
| [seahash](doc/seahash)                        |      3827.79 |    75.22 | 204.84 (3) | 871 | !msvc                      |
| [seahash32low](doc/seahash32low)              |      4234.67 |    64.47 | 242.36 (8) | 871 | !msvc                      |
| [fasthash32](doc/fasthash32)                  |      4209.33 |    56.07 | 214.53 (3) | 566 |                            |
| [fasthash64](doc/fasthash64)                  |      4209.36 |    52.76 | 190.23 (4) | 509 | Moment Chi2 5159 !         |
| [MUM](doc/MUM)                                |      6269.69 |    44.17 | 182.93 (2) |1912 | machine-specific (32/64 differs) |
| [MUMlow](doc/MUMlow)                          |      6892.86 |    46.35 | 221.42 (3) |1912 |                            |
| [mirhash](doc/mirhash)                        |      4926.94 |    46.72 | 176.80 (7) |1112 | machine-specific (32/64 differs) |
| [mirhash32low](doc/mirhash32low)              |      5454.01 |    42.33 | 200.93 (2) |1112 | machine-specific (32/64 differs) |
| [mirhashstrict](doc/mirhashstrict)            |      2003.63 |    71.66 | 190.70 (3) |1112 |                            |
| [mirhashstrict32low](doc/mirhashstrict32low)  |      2003.82 |    70.55 | 218.75 (2) |1112 |                            |
| [City32](doc/City32)                          |      3505.99 |    58.60 | 231.31 (2) |1319 |                            |
| [City64low](doc/City64low)                    |      8249.14 |    68.44 | 237.24 (8) |1120|                           |
| [City128](doc/City128)                        |      9050.33 |    82.70 | 231.10 (2) |1841 |                            |
| [CityCrc128](doc/CityCrc128)                  |     12011.76 |    83.02 | 249.24 (4) | 295 |                            |
| [FarmHash32](doc/FarmHash32)                  |     15288.62 |    68.36 | 236.95 (9) |11489| machine-specific (x86_64 SSE4/AVX) |
| [FarmHash64](doc/FarmHash64)                  |      7866.95 |    69.71 | 219.02 (3) |3758 |                            |
| [farmhash32_c](doc/farmhash32_c)              |     16592.05 |    68.40 | 234.15 (2) | 762 | machine-specific (x86_64 SSE4/AVX) |
| [farmhash64_c](doc/farmhash64_c)              |      7809.77 |    82.50 | 248.89 (2) |3688 |                            |
| [xxHash64](doc/xxHash64)                      |      8149.18 |    63.14 | 182.94 (1) |1999 |                            |
| [Spooky32](doc/Spooky32)                      |      8891.87 |    75.24 | 222.96 (6) |2221|                            |
| [Spooky64](doc/Spooky64)                      |      8917.09 |    75.15 | 214.23 (4) |2221 |                            |
| [clhash](doc/clhash)                          |      3668.68 |    90.64 | 248.11 (6) |1809 | machine-specific (x64 SSE4.2) |
| [t1ha2_atonce](doc/t1ha2_atonce)              |      7633.86 |    51.92 | 205.95 (8) | 541 |                            |
| [t1ha2_stream](doc/t1ha2_stream)              |      8010.07 |    99.57 | 267.50 (17) |1665 |                            |
| [t1ha1_64le](doc/t1ha1_64le)                  |      8599.49 |    41.83 | 202.74 (8) | 517 |                            |
| [t1ha1_64be](doc/t1ha1_64be)                  |      6532.71 |    44.00 | 183.92 (3) | 555 |                            |
| [t1ha0_32le](doc/t1ha0_32le)                  |      4580.62 |    59.09 | 193.81 (2) | 509 |                            |
| [t1ha0_32be](doc/t1ha0_32be)                  |      3937.01 |    59.19 | 197.00 (4) | 533 |                            |
| [t1ha0_aes_noavx](doc/t1ha0_aes_noavx)        |     18221.09 |    51.46 | 180.06 (2) | 925 | machine-specific (x86 AES-NI) |
| [t1ha0_aes_avx1](doc/t1ha0_aes_avx1)          |     19583.35 |    51.65 | 182.17 (4) | 843 | machine-specific (x64 AVX)    |
| [t1ha0_aes_avx2](doc/t1ha0_aes_avx2)          |     36436.51 |    36.31 | ?          | 792 | machine-specific (x64 AVX2)   |
| [wyhash](doc/wyhash)                          |     10839.95 |    30.19 | 171.64 (2) |1373 |                            |
| [wyhash32low](doc/wyhash32low)                |     10715.27 |    30.15 | 189.52 (2) |1373 | Moment Chi2 4.2            |

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
