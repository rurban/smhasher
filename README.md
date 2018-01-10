SMhasher
========

| Hash function                         |      MiB/sec  |cycles/hash| Quality problems              |
|:--------------------------------------|--------------:|---------:|--------------------------------|
| [donothing32](doc/donothing32)        |  26868545.30  |     6.11 | test NOP                       |
| [donothing64](doc/donothing64)        |  18810836.72  |     6.09 | test NOP                       |
| [donothing128](doc/donothing128)      |  24694622.36  |     5.11 | test NOP                       |
| [NOP_OAAT_read64](doc/NOP_OAAT_read64)|      2791.35  |    21.28 | test NOP                       |
| [BadHash](doc/BadHash)                |       468.31  |   105.42 | test FAIL                      |
| [sumhash](doc/sumhash)                |     11058.06  |    25.41 | test FAIL                      |
| [sumhash32](doc/sumhash32)            |     26601.48  |    15.61 | test FAIL                      |
| --------------------------------------|              |          |                                |
| [crc32](doc/crc32)                    |       352.29  |   143.32 | insecure, 8589.93x collisions, distrib  |
| [md5_32a](doc/md5_32a)                |       277.56  |   716.47 | 8589.93x collisions, distrib   |
| [sha1_32a](doc/sha1_32a)              |       324.16  |  1427.58 | collisions, 36.6% distrib      |
| [hasshe2](doc/hasshe2)                |      1639.48  |   103.19 | insecure,100% bias, collisions, distrib |
| [crc32_hw](doc/crc32_hw)              |      8103.92  |    32.36 | insecure,100% bias, collisions, distrib |
| [crc64_hw](doc/crc64_hw)              |      7624.95  |    35.12 | insecure,100% bias, collisions, distrib |
| [crc32_hw1](doc/crc32_hw1)            |     22610.60  |    36.80 | insecure,100% bias, collisions, distrib |
| [FNV1a](doc/FNV1a)                    |       791.84	|    69.21 | zeros,100% bias, collisions, distrib    |
| [FNV1a_YT](doc/FNV1a_YoshimitsuTRIAD) |      8328.60	|    27.78 | 100% bias, collisions, distrib   |
| [FNV64](doc/FNV64)                    |       791.84	|    70.17 | 100% bias, collisions, distrib   |
| [bernstein](doc/bernstein)            |       715.21	|    74.00 | 100% bias, collisions, distrib   |
| [sdbm](doc/sdbm)                      |       791.84	|    66.51 | 100% bias, collisions, distrib   |
| [x17](doc/x17)                        |       637.20	|    87.19 | 99.98% bias, collisions, distrib |
| [JenkinsOOAT](doc/JenkinsOOAT)        |       412.39	|   154.66 | 53.5% bias, collisions, distrib  |
| [JenkinsOOAT_pl](doc/JenkinsOOAT_perl)|       452.50	|   118.25 | 1.5-11.5% bias, 7.2x collisions  |
| [MicroOAAT](doc/MicroOAAT)            |       689.16	|    83.45 | 100% bias, distrib               |
| [jodyhash32](doc/jodyhash32)          |      1428.46	|    44.25 | bias, collisions, distr    |
| [jodyhash64](doc/jodyhash64)          |      2843.93	|    39.53 | bias, collisions, distr    |
| [lookup3](doc/lookup3)                |      1735.25	|    48.78 | 28% bias, collisions, 30% distr  |
| [superfast](doc/superfast)            |      2045.98	|    52.16 | 91% bias, 5273.01x collisions, 37% distr |
| [MurmurOAAT](doc/MurmurOAAT)          |       465.93	|   110.78 | collisions, 99.998% distr          |
| [Crap8](doc/Crap8)                    |      2844.03	|    37.48 | 2.42% bias, collisions, 2% distrib |
| [Murmur2](doc/Murmur2)                |      3147.50	|    40.85 | 1.7% bias, 81x coll, 1.7% distrib  |
| [Murmur2A](doc/Murmur2A)              |      2843.03	|    47.78 | 12.7% bias                         |
| [Murmur2B](doc/Murmur2B)              |      6155.72	|    57.48 | 1.8% bias, collisions, 3.4% distrib|
| [Murmur2C](doc/Murmur2C)              |      3633.24	|    46.88 | 91% bias, collisions, distr        |
| [HalfSipHash](doc/HalfSipHash)        |       587.04  |   145.03 | zeroes                     |
| --------------------------------------|               |          |                            |
| [GoodOAAT](doc/GoodOAAT)              |       929.59	|    79.06 |                            |
| [SipHash](doc/SipHash)                |       951.02	|   145.97 |                            |
| [SipHash13](doc/SipHash13)            |      1678.76  |   115.11 | 0.9% bias                  |
| [PMurHash32](doc/PMurHash32)          |      2436.65	|    60.15 |                            |
| [Murmur3A](doc/Murmur3A)              |      2364.40	|    52.83 |                            |
| [Murmur3C](doc/Murmur3C)              |      2468.70	|    79.44 |                            |
| [Murmur3F](doc/Murmur3F)              |      4376.29	|    55.03 |                            |
| [fasthash32](doc/fasthash32)          |      4881.26	|    55.11 |                            |
| [fasthash64](doc/fasthash64)          |      5404.17	|    46.73 |                            |
| [MUM](doc/MUM)                        |      7763.68	|    41.28 |  machine-specific          |
| [City32](doc/City32)                  |      3523.77	|    53.76 |                            |
| [City64](doc/City64)                  |      8728.22	|    54.72 | 2 minor collisions         |
| [City128](doc/City128)                |      9769.19	|    63.38 |                            |
| [CityCrc128](doc/CityCrc128)          |     13730.97	|    85.88 |                            |
| [FarmHash64](doc/FarmHash64)         	|      8711.25	|    58.16 | machine-specific           |
| [FarmHash128](doc/FarmHash128)        |      9738.29	|    79.91 | machine-specific           |
| [FarmHash32](doc/FarmHash32)         	|     24831.45  |    24.99 | disabled. too machine-specific |
| [farmhash32_c](doc/farmhash32_c)      |     24647.21  |    25.36 |                            |
| [farmhash64_c](doc/farmhash64_c)     	|      7886.79	|    64.48 |                            |
| [farmhash128_c](doc/farmhash128_c)    |      9770.40	|    79.59 |                            |
| [Spooky32](doc/Spooky32)              |      9944.88	|    60.12 |                            |
| [Spooky64](doc/Spooky64)              |      9943.72	|    60.16 |                            |
| [Spooky128](doc/Spooky128)            |      9936.26	|    60.10 |                            |
| [xxHash32](doc/xxHash32)              |      4914.62	|    64.17 | collisions with 4bit diff  |
| [xxHash64](doc/xxHash64)              |      8474.87	|    61.57 |                            |
| [metrohash64_1](doc/metrohash64_1)    |      8177.73	|    56.05 |                            |
| [metrohash64_2](doc/metrohash64_2)    |      9064.45	|    50.83 |                            |
| [metrohash128_1](doc/metrohash128_1)  |      7931.50	|    65.88 |                            |
| [metrohash128_2](doc/metrohash128_2)  |      8779.11	|    59.36 |                            |
| [metrohash64crc_1](doc/metrohash64crc_1)  | 15827.71	|    55.72 | cyclic collisions 8 byte   |
| [metrohash64crc_2](doc/metrohash64crc_2)  | 16072.41	|    56.79 | cyclic collisions 8 byte   |
| [metrohash128crc_1](doc/metrohash128crc_1)| 15468.70	|    66.62 |                            |
| [metrohash128crc_2](doc/metrohash128crc_2)| 14100.80	|    71.89 |                            |
| [cmetrohash64_1_o](doc/cmetrohash64_1_o)  |  9054.06	|    50.66 |                            |
| [cmetrohash64_1](doc/cmetrohash64_1)      |  8135.24	|    55.96 |                            |
| [cmetrohash64_2](doc/cmetrohash64_2)      |  9046.35	|    50.95 |                            |
| [falkhash](doc/falkhash)              |     19888.45	|   173.92 |                            |
| [t1ha](doc/t1ha)                      |     15480.28	|    26.41 |                            |
| [t1ha_64be](doc/t1ha_64be)            |      5203.00	|    53.69 |                            |
| [t1ha_32le](doc/t1ha_32le)            |      8930.90	|    29.79 |                            |
| [t1ha_32be](doc/t1ha_32be)            |      6931.84	|    34.17 |                            |
| [t1ha_crc](doc/t1ha_crc)              |     16757.73	|    28.69 |                            |
| [t1ha_aes](doc/t1ha_aes)              |     37299.66	|    25.68 |  machine-specific          |

Summary
-------

I added some SSE assisted hashes and fast intel/arm CRC32-C and AES HW variants, but not the fastest
[crcutil](https://code.google.com/p/crcutil/) yet. See [our crcutil results](https://github.com/rurban/smhasher/blob/master/doc/crcutil).
See also the old [https://code.google.com/p/smhasher/w/list](https://code.google.com/p/smhasher/w/list).

So the fastest hash functions on x86_64 without quality problems are:

* falkhash (_macho64 and elf64 nasm only, with HW AES extension_)
* t1ha + mum (_machine specific, mum: different arch results_)
* FarmHash (_not portable, too machine specific: 64 vs 32bit, old gcc, ..._)
* Metro (_but not 64crc yet, WIP_)
* Spooky32
* xxHash64
* fasthash
* City (_deprecated_)

Hash functions for symbol tables or hash tables typically use 32 bit
hashes, for databases, file systems and file checksums typically 64 or
128bit, for crypto now starting with 256 bit.

Typical median key size in perl5 is 20, the most common 4.
See [github.com/rurban/perl-hash-stats](https://github.com/rurban/perl-hash-stats)

When used in a hash table the instruction cache will usually beat the
CPU and throughput measured here. In my tests the smallest `FNV1A`
beats the fastest `crc32_hw1` with [Perl 5 hash tables](https://github.com/rurban/perl-hash-stats). 
Even if those worse hash functions will lead to more collisions, the
overall speed advantage beats the slightly worse quality.
See e.g. [A Seven-Dimensional Analysis of Hashing Methods and its Implications on Query Processing](https://infosys.cs.uni-saarland.de/publications/p249-richter.pdf)
for a concise overview of the best hash table strategies, confirming that the
simpliest Mult hashing (bernstein, FNV*, x17, sdbm) always beat "better" hash
functions (Tabulation, Murmur, Farm, ...) when used in a hash table.

The fast hash functions tested here are recommendable as fast for file
digests and maybe bigger databases, but not for 32bit hash tables.  The
"Quality problems" lead to less uniform distribution, i.e.  more collisions
and worse performance, but are rarely related to real security attacks, just
the 2nd sanity test against \0 invariance is security relevant.

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

Such an attack avoidance cannot not be the problem of the hash
function, but the hash table collision resolution scheme.  You can
attack every single hash function, even the best and most secure if
you detect the seed, e.g. from collision timings and independly the
sort-order, so you need to protect your collision handling scheme from
the worst-case O(n), i.e. separate chaining with linked lists. Linked
lists chaining allows high load factors, but is very cache-unfriendly.
The only recommendable linked list scheme is inlining the key or hash
into the array. Nowadays everybody uses fast open addressing, even if
the load factor needs to be ~50%, unless you use Cuckoo Hashing.

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
