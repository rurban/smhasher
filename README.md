SMhasher
========

| Hash function                         |      MiB/sec  |cycles/hash| Quality problems              |
|:--------------------------------------|--------------:|---------:|--------------------------------|
| [donothing32](doc/donothing32)        |  23271327.59  |    5.00  | test NOP                       |
| [donothing64](doc/donothing64)        |  22524919.33  |    5.00  | test NOP                       |
| [donothing128](doc/donothing128)      |  35531868.85  |    5.00  | test NOP                       |
| [NOP_OAAT_read64](doc/NOP_OAAT_read64)|      2098.56  |   33.66  | test NOP                       |
| [BadHash](doc/BadHash)                |       452.47  |  108.78  | test FAIL                      |
| [sumhash](doc/sumhash)                |      7220.74  |   33.77  | test FAIL                      |
| [sumhash32](doc/sumhash32)            |     21605.70  |   14.16  | test FAIL                      |
| --------------------------------------|               |          |                                |
| [crc32](doc/crc32)                    |      392.09	|   135.20 | insecure, 8589.93x collisions, distrib  |
| [md5_32a](doc/md5_32a)                |      352.25	|   674.76 | 8589.93x collisions, distrib   |
| [sha1_32a](doc/sha1_32a)              |      373.41	|  1492.19 | collisions, 36.6% distrib      |
| [hasshe2](doc/hasshe2)                |     3139.96	|    70.18 | insecure, 100% bias, collisions, distrib |
| [crc32_hw](doc/crc32_hw)              |     6331.24	|    29.89 | insecure, 100% bias, collisions, distrib, machine-specific (x86 SSE4.2) |
| [crc32_hw1](doc/crc32_hw1)            |    23011.78	|    35.72 | insecure, 100% bias, collisions, distrib, machine-specific (x86 SSE4.2) |
| [crc64_hw](doc/crc64_hw)              |     8423.86	|    29.36 | insecure, 100% bias, collisions, distrib, machine-specific (x86_64 SSE4.2) |
| [FNV1a](doc/FNV1a)                    |      790.45	|    69.32 | zeros, 100% bias, collisions, distrib    |
| [FNV1a_YT](doc/FNV1a_YT)              |     8949.71	|    27.97 | 100% bias, collisions, distrib   |
| [FNV64](doc/FNV64)                    |      791.85	|    69.31 | 100% bias, collisions, distrib   |
| [bernstein](doc/bernstein)            |      791.84	|    67.09 | 100% bias, collisions, distrib   |
| [sdbm](doc/sdbm)                      |      790.50	|    66.69 | 100% bias, collisions, distrib   |
| [x17](doc/x17)                        |      527.91	|    96.67 | 99.98% bias, collisions, distrib |
| [JenkinsOOAT](doc/JenkinsOOAT)        |      452.49	|   141.18 | 53.5% bias, collisions, distrib  |
| [JenkinsOOAT_pl](doc/JenkinsOOAT_perl)|      452.49	|   118.65 | 1.5-11.5% bias, 7.2x collisions  |
| [MicroOAAT](doc/MicroOAAT)            |      972.14	|    59.82 | 100% bias, distrib               |
| [jodyhash32](doc/jodyhash32)          |     1428.46	|    44.25 | bias, collisions, distr    |
| [jodyhash64](doc/jodyhash64)          |     2843.60	|    39.53 | bias, collisions, distr    |
| [lookup3](doc/lookup3)                |     1744.87	|    47.23 | 28% bias, collisions, 30% distr  |
| [superfast](doc/superfast)            |     1570.59	|    57.55 | 91% bias, 5273.01x collisions, 37% distr |
| [MurmurOAAT](doc/MurmurOAAT)          |      451.66	|   114.34 | collisions, 99.998% distr          |
| [Crap8](doc/Crap8)                    |     3149.87	|    34.14 | 2.42% bias, collisions, 2% distrib |
| [Murmur2](doc/Murmur2)                |     3139.49	|    40.63 | 1.7% bias, 81x coll, 1.7% distrib  |
| [Murmur2A](doc/Murmur2A)              |     3139.50	|    45.14 | 12.7% bias                         |
| [Murmur2B](doc/Murmur2B)              |     4867.23	|    46.49 | 1.8% bias, collisions, 3.4% distrib|
| [Murmur2C](doc/Murmur2C)              |     3919.19	|    46.27 | 91% bias, collisions, distr        |
| [HalfSipHash](doc/HalfSipHash)        |      747.06	|   121.13 | zeroes                     |
| [SipHash13](doc/SipHash13)            |     1865.13	|   100.55 | 0.9% bias                  |
| --------------------------------------|               |          |                            |
| [SipHash](doc/SipHash)                |       978.37	|   139.56 |                            |
| [GoodOAAT](doc/GoodOAAT)              |      1047.85	|    71.39 |                            |
| [PMurHash32](doc/PMurHash32)          |      2348.59	|    57.56 |                            |
| [Murmur3A](doc/Murmur3A)              |      2329.88	|    50.22 |                            |
| [Murmur3C](doc/Murmur3C)              |      3186.78	|    66.49 |                            |
| [Murmur3F](doc/Murmur3F)              |      5256.15	|    50.23 |                            |
| [fasthash32](doc/fasthash32)          |      4661.31	|    50.81 |                            |
| [fasthash64](doc/fasthash64)          |      4612.00	|    48.14 |                            |
| [MUM](doc/MUM)                        |      6564.38	|    39.85 | machine-specific (32/64 differs) |
| [City32](doc/City32)                  |      3818.33	|    53.02 |                            |
| [City64](doc/City64)                  |      9200.87	|    55.70 | 2 minor collisions         |
| [City128](doc/City128)                |     10105.87	|    89.41 |                            |
| [CityCrc128](doc/CityCrc128)          |     12638.87	|    90.65 |                            |
| [FarmHash64](doc/FarmHash64)         	|      9187.94	|    62.93 |                            |
| [FarmHash128](doc/FarmHash128)        |      9877.60	|    82.89 |                            |
| [FarmHash32](doc/FarmHash32)         	|     24831.45  |    24.99 | machine-specific (x86_64 SSE4/AVX) |
| [farmhash32_c](doc/farmhash32_c)      |     24647.21  |    25.36 | machine-specific (x86_64 SSE4/AVX) |
| [farmhash64_c](doc/farmhash64_c)     	|      9149.44	|    74.39 |                            |
| [farmhash128_c](doc/farmhash128_c)    |      9959.95	|    99.01 |                            |
| [xxHash32](doc/xxHash32)              |      5414.57	|    46.85 | collisions with 4bit diff  |
| [xxHash64](doc/xxHash64)              |     10288.36	|    58.79 |                            |
| [Spooky32](doc/Spooky32)              |      9899.74	|    68.10 |                            |
| [Spooky64](doc/Spooky64)              |      9885.49	|    68.00 |                            |
| [Spooky128](doc/Spooky128)            |      9901.48	|    68.56 |                            |
| [metrohash64_1](doc/metrohash64_1)    |      9541.68	|    50.85 |                            |
| [metrohash64_2](doc/metrohash64_2)    |      9569.20	|    50.86 |                            |
| [metrohash128_1](doc/metrohash128_1)  |      9908.72	|    81.39 |                            |
| [metrohash128_2](doc/metrohash128_2)  |      9943.95	|    81.31 |                            |
| [metrohash64crc_1](doc/metrohash64crc_1)  | 14007.26	|    55.84 | cyclic collisions 8 byte, machine-specific (x86_64 SSE4.2) |
| [metrohash64crc_2](doc/metrohash64crc_2)  | 13932.90	|    55.90 | cyclic collisions 8 byte, machine-specific (x86_64  SSE4.2) |
| [metrohash128crc_1](doc/metrohash128crc_1)| 13993.55	|    86.92 | machine-specific (x86_64 SSE4.2) |
| [metrohash128crc_2](doc/metrohash128crc_2)| 13929.50	|    86.90 | machine-specific (x86_64 SSE4.2) |
| [cmetrohash64_1o](doc/cmetrohash64_1o)    |  8665.09	|    50.82 |                            |
| [cmetrohash64_1](doc/cmetrohash64_1)      |  9522.76	|    51.04 |                            |
| [cmetrohash64_2](doc/cmetrohash64_2)      |  9470.74	|    50.80 |                            |
| [falkhash](doc/falkhash)              |     19984.13	|   173.46 | machine-specific (x86_64 AES-NI) |
| [t1ha_64be](doc/t1ha_64be)            |      7146.84	|    39.78 |                            |
| [t1ha_32le](doc/t1ha_32le)            |      5577.12	|    42.53 |                            |
| [t1ha_32be](doc/t1ha_32be)            |      4266.89	|    41.72 |                            |
| [t1ha](doc/t1ha)                      |      9590.96	|    36.51 |                            |
| [t1ha_crc](doc/t1ha_crc)              |     13775.04	|    35.87 | machine-specific (x86 SSE4.2) |
| [t1ha_aes](doc/t1ha_aes)              |     19927.77	|    36.02 | machine-specific (x86 AES-NI) |

Summary
-------

I added some SSE assisted hashes and fast intel/arm CRC32-C and AES HW variants, but not the fastest
[crcutil](https://code.google.com/p/crcutil/) yet. See [our crcutil results](https://github.com/rurban/smhasher/blob/master/doc/crcutil).
See also the old [https://code.google.com/p/smhasher/w/list](https://code.google.com/p/smhasher/w/list).

So the fastest hash functions on x86_64 without quality problems are:

- t1ha
- falkhash (_macho64 and elf64 nasm only, with HW AES extension_)
- Metro (_but not 64crc yet, WIP_)
- FarmHash (_not portable, too machine specific: 64 vs 32bit, old gcc, ..._)
- Spooky32
- xxHash64
- fasthash
- City (_deprecated_)
- mum (_machine specific, mum: different results on 32/64-bit archs_)

Hash functions for symbol tables or hash tables typically use 32 bit
hashes, for databases, file systems and file checksums typically 64 or
128bit, for crypto now starting with 256 bit.

Typical median key size in perl5 is 20, the most common 4. Similar for all other dynamic languages.
See [github.com/rurban/perl-hash-stats](https://github.com/rurban/perl-hash-stats)

When used in a hash table the instruction cache will usually beat the
CPU and throughput measured here. In my tests the smallest `FNV1A`
beats the fastest `crc32_hw1` with [Perl 5 hash tables](https://github.com/rurban/perl-hash-stats). 
Even if those worse hash functions will lead to more collisions, the
overall speed advantage beats the slightly worse quality.
See e.g. [A Seven-Dimensional Analysis of Hashing Methods and its Implications on Query Processing](https://infosys.cs.uni-saarland.de/publications/p249-richter.pdf)
for a concise overview of the best hash table strategies, confirming that the
simplest Mult hashing (bernstein, FNV*, x17, sdbm) always beat "better" hash
functions (Tabulation, Murmur, Farm, ...) when used in a hash table.

The fast hash functions tested here are recommendable as fast for file
digests and maybe bigger databases, but not for 32bit hash tables.  The
"Quality problems" lead to less uniform distribution, i.e.  more collisions
and worse performance, but are rarely related to real security attacks, just
the 2nd sanity test against `\0` invariance is security relevant.

Other
-----

* [http://www.strchr.com/hash_functions](http://www.strchr.com/hash_functions) lists other benchmarks and quality of most simple and fast hash functions.
* [http://bench.cr.yp.to/primitives-hash.html](http://bench.cr.yp.to/primitives-hash.html) lists the benchmarks of all currently tested secure hashes.
* The [Hash Function Lounge](http://www.larc.usp.br/~pbarreto/hflounge.html) overviews the known weaknesses and attacks.

TODO
----

Some popular SSE-improved FNV1 (_sanmayce_) variants, fletcher (_ZFS_), ...
and slower cryptographic hashes or more secure hashes are still
missing. BLAKE2, SHA-2, SHA-3 (Keccak), Grøstl, JH, Skein, ...

SECURITY
--------

The hash table attacks described in [SipHash](https://131002.net/siphash/)
against City, Murmur or Perl JenkinsOAAT or at
[Hash Function Lounge](http://www.larc.usp.br/~pbarreto/hflounge.html)
are not included here.

Such an attack avoidance cannot be the problem of the hash
function, but the hash table collision resolution scheme.  You can
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

The '\0' vulnerability attack with binary keys is tested in the 2nd Sanity test.
