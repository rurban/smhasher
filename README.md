SMhasher
========

| Hash function         |      MiB/sec  |cycles/hash| Quality problems              |
|:----------------------|--------------:|---------:|--------------------------------|
| donothing32           |   9637878.60  |     5.48 | overall bad                    |
| donothing64           |   8211016.32	|     5.15 | overall bad                    |
| donothing128          |   7676888.89	|     5.83 | overall bad                    |
| NOP_OAAT_read64       |      2414.92  |    45.03 | 100% bias, 2.17x collisions    |
| crc32                 |       569.71	|    97.67 | insecure, 8589.93x collisions, distrib   |
| md5_32a               |       414.75	|   533.59 | 8589.93x collisions, distrib   |
| sha1_32a              |       657.77	|   971.34 | collisions, 36.6% distrib      |
| hasshe2               |      2292.04	|    72.23 | insecure,100% bias, collisions, distrib |
| crc32_hw              |      7733.44  |    43.48 | insecure,100% bias, collisions, distrib |
| crc64_hw              |     10937.53  |    23.33 | insecure,100% bias, collisions, distrib |
| crc32_hw1             |     30703.38  |    27.08 | insecure,100% bias, collisions, distrib |
| FNV1a                 |      1026.17  |    52.90 | zeros,100% bias, collisions, distrib |
| FNV1a_YoshimitsuTRIAD |     15700.04  |    20.62 | 100% bias, collisions, distrib |
| FNV64                 |      1015.60  |    53.55 | 100% bias, collisions, distrib |
| bernstein             |      1026.17  |    51.13 | 100% bias, collisions, distrib |
| sdbm                  |       977.48  |    51.75 | 100% bias, collisions, distrib |
| x17                   |       893.28  |    62.36 | 99.98% bias, collisions, distrib |
| JenkinsOOAT           |       563.23  |   104.31 | 53.5% bias, collisions, distrib |
| JenkinsOOAT_perl      |       676.91  |    87.82 | 1.5-11.5% bias, 7.2x collisions |
| lookup3               |      3303.80  |    31.66 | 28% bias, collisions, 30% distr |
| superfast             |      3158.88  |    35.44 | 91% bias, 5273.01x collisions, 37% distr |
| MurmurOAAT            |       719.76  |    76.48 | collisions, 99.998% distr      |
| Crap8                 |      4069.57  |    26.83 | 2.42% bias, collisions, 2% distrib |
| Murmur2               |      4049.53  |    31.64 | 1.7% bias, 81x coll, 1.7% distrib  |
| Murmur2A              |      4031.16  |    34.56 | 12.7% bias                     |
| Murmur2B              |      8062.20  |    33.62 | 1.8% bias, collisions, 3.4% distrib |
| Murmur2C              |      5381.36  |    35.33 | 91% bias, collisions, distr    |
| ----------------------|               |          |                            |
| PMurHash32            |      3034.35  |    45.11 |                            |
| Murmur3A              |      3263.93  |    37.86 |                            |
| Murmur3C              |      3733.90  |    48.66 |                            |
| Murmur3F              |      7136.07  |    36.33 |                            |
| fasthash32            |      7198.92  |    33.05 |                            |
| fasthash64            |      6911.62  |    33.57 |                            |
| City32                |      6054.93  |    41.96 | 2 minor collisions         |
| City64                |     14116.86  |    46.76 |                            |
| City128               |     13512.87  |    47.20 |                            |
| CityCrc128            |     21009.15  |    51.60 |                            |
| FarmHash64        	|     12795.07  |    49.34 | machine-specific           |
| FarmHash128       	|     13514.39  |    59.63 | machine-specific           |
| FarmHash32        	|     24831.45  |    24.99 | disabled. too machine-specific |
| farmhash32_c       	|     24647.21  |    25.36 |                            |
| farmhash64_c    	    |     14976.48  |    41.88 |                            |
| farmhash128_c     	|     15856.60  |    56.52 |                            |
| SipHash               |      1264.31  |   117.20 |                            |
| Spooky32              |     15352.40  |    43.98 |                            |
| Spooky64              |     16411.41  |    43.69 |                            |
| Spooky128             |     16358.37  |    45.04 | collisions with 4bit diff  |
| xxHash32              |      4494.48  |    47.90 |                            |
| xxHash64              |     15580.86  |    44.80 |                            |
| metrohash64_1         |     16555.13  |    37.22 |                            |
| metrohash64_2         |     16287.46  |    39.82 |                            |
| metrohash128_1        |     15802.90  |    47.65 |                            |
| metrohash128_2        |     15500.74  |    48.16 | cyclic collisions 8 byte   |
| metrohash64crc_1      |     23946.99  |    45.67 | cyclic collisions 8 byte   |
| metrohash64crc_2      |     25105.50  |    38.75 |                            |
| metrohash128crc_1     |     27411.55  |    46.06 |                            |
| metrohash128crc_2     |     27790.85  |    45.67 |                            |
| cmetrohash64_1_o      |     17237.75  |    36.95 |                            |
| cmetrohash64_1        |     16463.21  |    36.30 |                            |
| cmetrohash64_2        |     17188.28  |    35.40 |                            |
| falkhash              |     39817.46  |   124.81 |                            |


Summary
-------

I added some SSE assisted hashes and fast intel/arm CRC32-C and AES HW variants, but not the fastest
[crcutil](https://code.google.com/p/crcutil/) yet. See [our crcutil results](https://github.com/rurban/smhasher/blob/master/doc/crcutil).
See also the old [https://code.google.com/p/smhasher/w/list](https://code.google.com/p/smhasher/w/list).

So the fastest hash functions on x86_64 without quality problems are:

* falkhash (_macho64 and elf64 nasm only, with HW AES extension_)
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
beats the fastest `crc32_hw1` with
[Perl 5 hash tables](https://github.com/rurban/perl-hash-stats). 
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

The hash table attacks described in
[SipHash](https://131002.net/siphash/) against City, Murmur or Perl
JenkinsOAAT or at
[Hash Function Lounge](http://www.larc.usp.br/~pbarreto/hflounge.html)
are not included here.

Such an attack avoidance cannot not be the problem of the hash
function, but the hash table collision resolution scheme.  You can
attack every single hash function, even the best and most secure if you detect the
seed, e.g. from the sort-order, so you need to protect your collision
handling scheme from the worst-case O(n), i.e. separate chaining with
linked lists. Linked lists chaining allows high load factors, but is very
cache-unfriendly.
The only recommendable linked list scheme is inlining the key or hash into
the array. Nowadays everybody uses fast open addressing, even if the load factor
needs to be ~50%, unless you use Cuckoo Hashing.

I.e. the usage of siphash for their hash table in Python 3.4, ruby,
rust, systemd, OpenDNS, Haskell and OpenBSD is pure security theatre.
siphash is not secure enough for security purposes and not fast
enough for general usage.
Using Murmur is usually slower than a simple Mult, even in the worst case.
Provable secure is only uniform hashing, i.e. 2-5 independent Mult or Tabulation,
or using a guaranteed logarithmic or linear collision scheme, such as Robin Hood
or Cockoo hashing.

One more note regarding security: Nowadays even SHA1 can be solved in
a solver, like Z3 (or faster ones). So all hash functions with less
than 256 bits tested here cannot be considered "secure" at all.

The '\0' vulnerability attack with binary keys is tested in the 2nd Sanity test.

