SMhasher
========

| Hash function     |MiB/sec @ Haswell 3ghz | cycles/hash | Quality problems    |
|:------------------|--------------:|---------:|--------------------------------|
| donothing32     	|  34142468.23	|    24.25 | overall bad                    |
| donothing64     	|  33509179.27	|    24.28 | overall bad                    |
| donothing128    	|  33854199.39	|    24.19 | overall bad                    |
| crc32           	|       438.42	|    64.95 | 100% bias, 2.17x collisions    |
| md5_32a         	|       553.41	|   393.37 | 8589.93x collisions, distrib   |
| sha1_32a        	|       539.68	|   733.05 | 8589.93x collisions, distrib   |
| hasshe2         	|      2808.13	|    42.98 | collisions, 36.6% distrib      |
| crc32_hw        	|      7162.71	|    26.55 | 100% bias, collisions, distrib |
| crc64_hw        	|      9585.10	|    26.18 | 100% bias, collisions, distrib |
| crc32_hw1       	|     26871.56	|    31.27 | 100% bias, collisions, distrib |
| FNV1a          	|       898.41	|    33.19 | 100% bias, collisions, distrib |
| FNV1a_YoshimitsuTRIAD|  18145.89	|    18.27 | 100% bias, collisions, distrib |
| FNV64           	|       982.23	|    30.38 | 100% bias, collisions, distrib |
| bernstein       	|      1355.42	|    33.98 | 100% bias, collisions, distrib |
| sdbm            	|       992.55	|    29.23 | 100% bias, collisions, distrib |
| x17             	|       803.49	|    36.55 | 100% bias, collisions, distrib |
| JenkinsOOAT     	|       804.33	|    61.03 | 99.98% bias, collisions, distrib |
| JenkinsOOAT_perl	|       804.80	|    50.83 | 53.5% bias, collisions, distrib |
| lookup3         	|      3031.05	|    25.37 | 1.5-11.5% bias, 7.2x collisions |
| superfast       	|      2699.37	|    27.75 | 28% bias, collisions, 30% distr |
| MurmurOAAT      	|       635.29	|    49.37 | 91% bias, 5273.01x collisions, 37% distr |
| Crap8           	|      3925.29	|    23.61 | collisions, 99.998% distr      |
| Murmur2         	|      4006.44	|    24.73 | 2.42% bias, collisions, 2% distrib |
| Murmur2A        	|      3704.47	|    28.80 | 1.7% bias, 81x coll, 1.7% distrib  |
| Murmur2B        	|      7534.67	|    27.48 | 12.7% bias                     |
| Murmur2C        	|      4543.07	|    29.72 | 1.8% bias, collisions, 3.4% distrib |
| PMurHash32      	|      2927.84	|    37.16 | 91% bias, collisions, distr    |
| ------------------|               |          |                            |
| Murmur3A        	|      2911.94	|    29.54 |                            |
| Murmur3C        	|      4582.45	|    40.56 |                            |
| Murmur3F        	|      7037.66	|    37.38 |                            |
| City32          	|      6346.17	|    31.20 |                            |
| City64          	|     13460.57	|    26.14 | 2 minor collisions         |
| City128         	|     14991.24	|    38.59 |                            |
| CityCrc128      	|     19300.94	|    43.58 |                            |
| FarmHash32      	|     12815.77	|    27.31 |                            |
| FarmHash64      	|     13597.07	|    29.48 |                            |
| FarmHash128     	|     14820.56	|    46.96 |                            |
| SipHash         	|      1060.13	|   114.48 |                            |
| Spooky32        	|     13288.56	|    38.45 |                            |
| Spooky64        	|     13886.73	|    39.80 |                            |
| Spooky128       	|     13176.36	|    38.66 |                            |
| xxHash32        	|      7004.33	|    27.37 | collisions with 4bit diff  |
| xxHash64        	|     13965.82	|    31.50 |                            |
| metrohash64_1   	|     14624.06	|    26.78 |                            |
| metrohash64_2   	|     14639.30	|    23.61 |                            |
| metrohash128_1  	|     14706.25	|    29.63 |                            |
| metrohash128_2  	|     15246.65	|    26.99 |                            |
| metrohash64crc_1	|     28287.41	|    23.27 | cyclic collisions 8 byte   |
| metrohash64crc_2	|     28552.97	|    23.09 | cyclic collisions 8 byte   |
| metrohash128crc_1	|     27701.39	|    25.58 |                            |
| metrohash128crc_2	|     25736.80	|    25.84 |                            |


Summary
-----

I added some SSE assisted hashes and fast intel/arm CRC32-C HW variants, but not the fastest
[crcutil](https://code.google.com/p/crcutil/) yet. See [our crcutil results](https://github.com/rurban/smhasher/blob/master/doc/crcutil).
See also the old [https://code.google.com/p/smhasher/w/list](https://code.google.com/p/smhasher/w/list).

So the fastest hash functions on x86_64 without quality problems are:

* FarmHash (_C++ only, see cfarmhash on github for a 64bit variant in C_)
* Metro (_but not 64crc yet, WIP_)
* Spooky32
* xxHash64
* City (_deprecated_)

Hash functions for symbol tables or hash tables typically use 32 bit hashes,
for databases and file systems typically 64 or 128bit, for crypto now starting with 256 bit.

Typical median key size in perl5 is 20, the most common 4.
See [github.com/rurban/perl-hash-stats](https://github.com/rurban/perl-hash-stats)

When used in a hash table the instruction cache will usually beat the
CPU and throughput measured here. In my tests the smallest
`FNV1A` beats the fastest `crc32_hw1`.

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

The hash table attacks described in
[SipHash](https://131002.net/siphash/) against City, Murmur or Perl
JenkinsOAAT or at
[Hash Function Lounge](http://www.larc.usp.br/~pbarreto/hflounge.html)
are not included here.

Such an attack avoidance cannot not be the problem of the hash
function, but the hash table collision resolution scheme.  You can
attack every single hash function, even the best, if you detect the
seed, e.g. from the sort-order, so you need to protect your collision
handling scheme from the worst-case O(n), i.e. separate chaining with
linked lists. Linked lists chaining is also very cache-unfriendly.

The '\0' vulnerability attack with binary keys is tested in the 2nd Sanity test.
