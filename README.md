SMhasher
========

| Hash function                         |      MiB/sec  |cycles/hash| Quality problems              |
|:--------------------------------------|--------------:|---------:|--------------------------------|
| [donothing32](doc/donothing32)        |  12954965.31  |     4.64 | test NOP                       |
| [donothing64](doc/donothing64)        |   9224792.69  |     4.72 | test NOP                       |
| [donothing128](doc/donothing128)      |   9074269.36  |     4.47 | test NOP                       |
| [NOP_OAAT_read64](doc/NOP_OAAT_read64)|      3909.83  |    16.34 | test NOP                       |
| [BadHash](doc/BadHash)                |       900.52  |    58.13 | test FAIL                      |
| [sumhash](doc/sumhash)                |     20379.60  |    21.60 | test FAIL                      |
| [sumhash32](doc/sumhash32)            |     74930.18  |    11.33 | test FAIL                      |
| --------------------------------------|               |          |                                |
| [crc32](doc/crc32)                    |       544.84  |    92.65 | insecure, 8589.93x collisions, distrib  |
| [md5_32a](doc/md5_32a)                |       399.30  |   550.28 | 8589.93x collisions, distrib   |
| [sha1_32a](doc/sha1_32a)              |       648.72  |   850.86 | collisions, 36.6% distrib      |
| [hasshe2](doc/hasshe2)                |      2054.39  |    76.60 | insecure, fails all tests      |
| [crc32_hw](doc/crc32_hw)              |      9310.60  |    24.63 | insecure, 100% bias, collisions, distrib, machine-specific (x86 SSE4.2) |
| [crc32_hw1](doc/crc32_hw1)            |     30145.28  |    30.98 | insecure, 100% bias, collisions, distrib, machine-specific (x86 SSE4.2) |
| [crc64_hw](doc/crc64_hw)              |     10181.36  |    23.46 | insecure, 100% bias, collisions, distrib, machine-specific (x86_64 SSE4.2) |
| [fibonacci](doc/fibonacci)            |     20706.41  |    19.38 | zeros, fails all tests       |
| [multiply_shift](doc/multiply_shift)  |      5266.99  |    42.03 | fails all tests              |
| [pair_multiply_shift](doc/pair_multiply_shift) | 21763.98  |    22.70 | fails all tests         |
| [FNV1a](doc/FNV1a)                    |       937.85  |    56.63 | zeros, fails all tests       |
| [FNV1a_YT](doc/FNV1a_YT)              |     15425.31  |    21.51 | fails all tests              |
| [FNV64](doc/FNV64)                    |       893.70  |    56.94 | fails all tests              |
| [FNV2](doc/FNV2)                      |      7885.43  |    28.71 | fails all tests              |
| [fletcher2](doc/fletcher2)            |     26241.42  |    19.01 | fails all tests              |
| [fletcher4](doc/fletcher4)            |      9526.54  |    19.53 | fails all tests              |
| [bernstein](doc/bernstein)            |      1261.71  |    46.20 | fails all tests              |
| [sdbm](doc/sdbm)                      |       929.94  |    54.35 | fails all tests              |
| [x17](doc/x17)                        |      1174.99  |    53.62 | 99.98% bias, fails all tests |
| [JenkinsOOAT](doc/JenkinsOOAT)        |       736.83  |    86.43 | 53.5% bias, fails all tests  |
| [JenkinsOOAT_pl](doc/JenkinsOOAT_perl)|       741.41  |    71.78 | 1.5-11.5% bias, 7.2x collisions  |
| [MicroOAAT](doc/MicroOAAT)            |       819.52  |    63.24 | 100% bias, distrib           |
| [jodyhash32](doc/jodyhash32)          |      1864.24  |    34.43 | bias, collisions, distr      |
| [jodyhash64](doc/jodyhash64)          |      3764.44  |    30.13 | bias, collisions, distr      |
| [lookup3](doc/lookup3)                |      3133.07  |    30.65 | 28% bias, collisions, 30% distr  |
| [superfast](doc/superfast)            |      2918.47  |    34.62 | 91% bias, 5273.01x collisions, 37% distr |
| [MurmurOAAT](doc/MurmurOAAT)          |       669.46  |    74.54 | collisions, 99.998% distr          |
| [Crap8](doc/Crap8)                    |      3962.47  |    26.98 | 2.42% bias, collisions, 2% distrib |
| [Murmur2](doc/Murmur2)                |      3712.67  |    31.32 | 1.7% bias, 81x coll, 1.7% distrib  |
| [Murmur2A](doc/Murmur2A)              |      3875.69  |    34.80 | 12.7% bias                         |
| [Murmur2B](doc/Murmur2B)              |      7336.97  |    34.43 | 1.8% bias, collisions, 3.4% distrib|
| [Murmur2C](doc/Murmur2C)              |      5050.30  |    34.10 | 91% bias, collisions, distr        |
| [Murmur3C](doc/Murmur3C)              |      3404.34  |    48.65 | LongNeighbors              |
| [metrohash64_1](doc/metrohash64_1)    |     15491.63  |    37.56 | LongNeighbors              |
| [metrohash64_2](doc/metrohash64_2)    |     16316.71  |    37.82 | LongNeighbors              |
| [metrohash128_1](doc/metrohash128_1)  |     15902.48  |    43.79 | LongNeighbors              |
| [metrohash128_2](doc/metrohash128_2)  |     16374.31  |    46.21 | LongNeighbors              |
| [cmetrohash64_1o](doc/cmetrohash64_1o)    | 15997.88	|    37.07 | LongNeighbors              |
| [cmetrohash64_1](doc/cmetrohash64_1)      | 17094.64	|    36.85 | LongNeighbors              |
| [cmetrohash64_2](doc/cmetrohash64_2)      | 15952.55	|    38.87 | LongNeighbors              |
| [falkhash](doc/falkhash)              |     37742.94	|   108.59 | LongNeighbors, machine-specific (x86_64 AES-NI) |
| [xxHash32](doc/xxHash32)              |      7297.39	|    36.91 | LongNeighbors, collisions with 4bit diff |
| [t1ha2_atonce128](doc/t1ha2_atonce128)|     14007.67	|    50.51 | LongNeighbors              |
| [t1ha2_stream128](doc/t1ha2_stream128)|      7274.53	|    93.11 | LongNeighbors              |
| [HalfSipHash](doc/HalfSipHash)        |      1105.93  |    79.31 | zeroes                     |
| [SipHash13](doc/SipHash13)            |      2124.26  |    84.20 | 0.9% bias                  |
| --------------------------------------|               |          |                            |
| [SipHash](doc/SipHash)                |      1124.24  |   115.72 |                            |
| [HighwayHash64](doc/HighwayHash64)    |     14997.76  |    78.50 |                            |
| [GoodOAAT](doc/GoodOAAT)              |      1237.86  |    52.75 |                            |
| [PMurHash32](doc/PMurHash32)          |      3070.82  |    44.13 | Moment Chi2 69             |
| [Murmur3A](doc/Murmur3A)              |      3166.17  |    38.70 | Moment Chi2 69             |
| [Murmur3F](doc/Murmur3F)              |      6853.24  |    38.15 |                            |
| [fasthash32](doc/fasthash32)          |      6693.98  |    36.67 |                            |
| [fasthash64](doc/fasthash64)          |      7020.23  |    35.51 | Moment Chi2 5159 !         |
| [MUM](doc/MUM)                        |     12790.65  |    29.63 | machine-specific (32/64 differs) |
| [MUMlow](doc/MUMlow)                  |     12973.97  |    34.67 |                            |
| [MUMhigh](doc/MUMhigh)                |     12465.58  |    35.64 |                            |
| [City32](doc/City32)                  |      5802.91  |    40.73 |                            |
| [City64noSeed](doc/City64noSeed)      |     14124.43  |    29.41 |                            |
| [City64](doc/City64)                  |     13964.74  |    39.84 | 2 minor collisions         |
| [City64low](doc/City64low)            |     13888.28  |    45.66 |                            |
| [City64high](doc/City64high)          |     14340.83  |    45.69 |                            |
| [City128](doc/City128)                |     16000.61  |    45.74 |                            |
| [CityCrc128](doc/CityCrc128)          |     19348.29  |    48.34 |                            |
| [FarmHash64](doc/FarmHash64)         	|     14899.89  |    42.41 |                            |
| [FarmHash128](doc/FarmHash128)        |     15998.86  |    58.12 |                            |
| [FarmHash32](doc/FarmHash32)         	|     24831.45  |    24.99 | machine-specific (x86_64 SSE4/AVX) |
| [farmhash32_c](doc/farmhash32_c)      |     24647.21  |    25.36 | machine-specific (x86_64 SSE4/AVX) |
| [farmhash64_c](doc/farmhash64_c)     	|     14967.76  |    42.01 |                            |
| [farmhash128_c](doc/farmhash128_c)    |     15097.31  |    61.00 |                            |
| [xxHash64](doc/xxHash64)              |     14879.09	|    44.11 |                            |
| [xxh3](doc/xxh3)                      |     43021.12	|    26.00 | Moment Chi2 14974 !        |
| [xxh3low](doc/xxh3low)                |     37670.92	|    26.39 | Moment Chi2 1.8e+9 !       |
| [xxh3high](doc/xxh3high)              |     45932.74	|    27.29 |      - " -                 |
| [xxh128](doc/xxh128)                  |     44407.83	|    26.89 |                            |
| [xxh128low](doc/xxh128low)            |     43661.23	|    25.86 | Moment Chi2 14974 !        |
| [xxh128high](doc/xxh128high)          |     41762.05	|    28.81 |      - " -                 |
| [Spooky32](doc/Spooky32)              |     14213.99	|    48.03 |                            |
| [Spooky64](doc/Spooky64)              |     14839.81	|    46.62 |                            |
| [Spooky128](doc/Spooky128)            |     14833.63	|    47.50 |                            |
| [metrohash64crc_1](doc/metrohash64crc_1)  | 25856.64	|    41.24 | cyclic collisions 8 byte, machine-specific (x64 SSE4.2) |
| [metrohash64crc_2](doc/metrohash64crc_2)  | 26450.83	|    39.74 | cyclic collisions 8 byte, machine-specific (x64  SSE4.2) |
| [metrohash128crc_1](doc/metrohash128crc_1)| 25404.51	|    49.07 | machine-specific (x64 SSE4.2) |
| [metrohash128crc_2](doc/metrohash128crc_2)| 25248.70	|    49.57 | machine-specific (x64 SSE4.2) |
| [clhash](doc/clhash)                  |     19322.99	|    57.17 | machine-specific (x64 SSE4.2) |
| [t1ha2_atonce](doc/t1ha2_atonce)      |     14747.01	|    36.09 |                            |
| [t1ha2_stream](doc/t1ha2_stream)      |      6376.62	|    82.41 |                            |
| [t1ha1_64le](doc/t1ha1_64le)          |     16255.08	|    27.63 |                            |
| [t1ha1_64be](doc/t1ha1_64be)          |     12285.15	|    28.84 |                            |
| [t1ha0_32le](doc/t1ha0_32le)          |      8866.04	|    36.95 |                            |
| [t1ha0_32be](doc/t1ha0_32be)          |      7859.07	|    38.63 |                            |
| [t1ha0_aes_noavx](doc/t1ha0_aes_noavx)|     21264.27	|    35.63 | machine-specific (x86 AES-NI) |
| [t1ha0_aes_avx1](doc/t1ha0_aes_avx1)  |     20443.32	|    36.05 | machine-specific (x64 AVX)    |
| [t1ha0_aes_avx2](doc/t1ha0_aes_avx2)  |     36436.51	|    36.31 | machine-specific (x64 AVX2)   |
| [wyhash](doc/wyhash)                  |     16528.58	|    17.67 |                            |
| [wyhash32lo](doc/wyhash32lo)          |     15933.47	|    17.77 |                            |

Summary
-------

I added some SSE assisted hashes and fast intel/arm CRC32-C and AES HW variants, but not the fastest
[crcutil](https://code.google.com/p/crcutil/) yet. See [our crcutil results](https://github.com/rurban/smhasher/blob/master/doc/crcutil).
See also the old [https://code.google.com/p/smhasher/w/list](https://code.google.com/p/smhasher/w/list).

So the fastest hash functions on x86_64 without quality problems are:

- wyhash
- xxh3
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

Other
-----

* [http://www.strchr.com/hash_functions](http://www.strchr.com/hash_functions) lists other benchmarks and quality of most simple and fast hash functions.
* [http://bench.cr.yp.to/primitives-hash.html](http://bench.cr.yp.to/primitives-hash.html) lists the benchmarks of all currently tested secure hashes.
* The [Hash Function Lounge](http://www.larc.usp.br/~pbarreto/hflounge.html) overviews the known weaknesses and attacks.

TODO
----

Some popular SSE-improved FNV1 (_sanmayce_) variants
and slower cryptographic hashes or more secure hashes are still
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

The '\0' vulnerability attack with binary keys is tested in the 2nd Sanity test.
