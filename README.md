SMhasher
========

| Hash function     |MiB/sec @ Haswell 3ghz | cycles/hash | Quality problems    |
|:------------------|--------------:|---------:|--------------------------------|
| donothing32           |  28295961.78  |    24.66 | overall bad                    |
| donothing64           |  30859408.29  |    26.16 | overall bad                    |
| donothing128          |  40564679.38  |    25.48 | overall bad                    |
| NOP_OAAT_read64       |      2414.92  |    45.03 | 100% bias, 2.17x collisions    |
| crc32                 |       403.80  |    74.33 | insecure, 8589.93x collisions, distrib   |
| md5_32a               |       497.78  |   386.72 | 8589.93x collisions, distrib   |
| sha1_32a              |       513.43  |   657.10 | collisions, 36.6% distrib      |
| hasshe2               |      3210.42  |    41.31 | insecure,100% bias, collisions, distrib |
| crc32_hw              |      8204.61  |    23.09 | insecure,100% bias, collisions, distrib |
| crc64_hw              |     10917.00  |    21.50 | insecure,100% bias, collisions, distrib |
| crc32_hw1             |     30703.38  |    27.08 | insecure,100% bias, collisions, distrib |
| FNV1a                 |      1011.45  |    32.92 | zeros,100% bias, collisions, distrib |
| FNV1a_YoshimitsuTRIAD |     18204.42  |    20.74 | 100% bias, collisions, distrib |
| FNV64                 |       990.31  |    33.12 | 100% bias, collisions, distrib |
| bernstein             |      1348.66  |    33.99 | 100% bias, collisions, distrib |
| sdbm                  |      1026.19  |    30.06 | 100% bias, collisions, distrib |
| x17                   |       794.05  |    35.24 | 99.98% bias, collisions, distrib |
| JenkinsOOAT           |       810.29  |    60.17 | 53.5% bias, collisions, distrib |
| JenkinsOOAT_perl      |       820.96  |    54.22 | 1.5-11.5% bias, 7.2x collisions |
| lookup3               |      3030.99  |    23.64 | 28% bias, collisions, 30% distr |
| superfast             |      2620.25  |    28.64 | 91% bias, 5273.01x collisions, 37% distr |
| MurmurOAAT            |       684.14  |    48.19 | collisions, 99.998% distr      |
| Crap8                 |      4080.28  |    21.59 | 2.42% bias, collisions, 2% distrib |
| Murmur2               |      3956.34  |    22.75 | 1.7% bias, 81x coll, 1.7% distrib  |
| Murmur2A              |      4027.33  |    28.19 | 12.7% bias                     |
| Murmur2B              |      7648.97  |    21.26 | 1.8% bias, collisions, 3.4% distrib |
| Murmur2C              |      5398.54  |    26.74 | 91% bias, collisions, distr    |
| ----------------------|               |          |                            |
| PMurHash32            |      3242.25  |    31.52 |                            |
| Murmur3A              |      3188.52  |    24.50 |                            |
| Murmur3C              |      5400.86  |    35.01 |                            |
| Murmur3F              |      7845.17  |    35.23 |                            |
| fasthash32            |      8048.25  |    23.77 |                            |
| fasthash64            |      8015.22  |    22.04 |                            |
| City32                |      7152.39  |    24.00 | 2 minor collisions         |
| City64                |     15335.21  |    23.99 |                            |
| City128               |     16383.46  |    34.13 |                            |
| CityCrc128            |     21221.23  |    37.03 |                            |
| FarmHash32            |     14837.70  |    23.68 |                            |
| FarmHash64            |     15795.36  |    24.53 |                            |
| FarmHash128           |     17426.60  |    42.90 |                            |
| SipHash               |      1268.37  |   105.28 |                            |
| Spooky32              |     15989.78  |    30.95 |                            |
| Spooky64              |     15994.90  |    34.74 |                            |
| Spooky128             |     16017.34  |    34.97 | collisions with 4bit diff  |
| xxHash32              |      7547.52  |    25.05 |                            |
| xxHash64              |     15789.29  |    30.01 |                            |
| metrohash64_1         |     17287.69  |    27.17 |                            |
| metrohash64_2         |     16554.01  |    22.76 |                            |
| metrohash128_1        |     16673.07  |    24.31 |                            |
| metrohash128_2        |     16542.33  |    28.02 | cyclic collisions 8 byte   |
| metrohash64crc_1      |     29181.89  |    22.60 | cyclic collisions 8 byte   |
| metrohash64crc_2      |     29596.02  |    21.55 |                            |
| metrohash128crc_1     |     28870.28  |    24.89 |                            |
| metrohash128crc_2     |     27789.30  |    27.23 |                            |
| cmetrohash64_1_optshort |   16862.58  |    23.11 |                            |
| cmetrohash64_1        |     15799.52  |    25.44 |                            |
| cmetrohash64_2        |     15040.97  |    28.59 |                            |
| falkhash              |     36785.50  |    65.55 |                            |


Summary
-------

I added some SSE assisted hashes and fast intel/arm CRC32-C and AES HW variants, but not the fastest
[crcutil](https://code.google.com/p/crcutil/) yet. See [our crcutil results](https://github.com/rurban/smhasher/blob/master/doc/crcutil).
See also the old [https://code.google.com/p/smhasher/w/list](https://code.google.com/p/smhasher/w/list).

So the fastest hash functions on x86_64 without quality problems are:

* falkhash (_macho64 and elf64 nasm only, with HW AES extension_)
* FarmHash (_C++ only, see cfarmhash on github for a 64bit variant in C_)
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
beats the fastest `crc32_hw1` with
[Perl 5 hash tables](https://github.com/rurban/perl-hash-stats).  Even
if those worse hash functions will lead to more collisions, the
overall speed advantage beats the worse quality.

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
