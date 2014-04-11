SMhasher
========

| Hash function   |MiB/sec @ 3 ghz | cycles/hash 10-byte keys | Quality problem |
|:----------------|------------:|-------------:|--------------------------------|
| donothing32     | 20620706.92 |        25.82 | overall bad                    |
| donothing64     | 20489811.27 |        25.43 | overall bad                    |
| donothing128    | 28096086.10 |        25.19 | overall bad                    |
| crc32           |      333.70 |        46.80 | 100% bias, 2.17x collisions    |
| md5_32a         |      331.67 |       670.46 | 8589.93x collisions, distrib   |
| sha1_32a        |      262.58 |      1638.29 | 8589.93x collisions, distrib   |
| hasshe2         |     2078.01 |        54.48 | collisions, 36.6% distrib      |
| crc32_hw        |     6127.57 |        29.78 | 100% bias, collisions, distrib |
| crc64_hw        |     8059.75 |        30.87 | 100% bias, collisions, distrib |
| crc32_hw1       |    22584.94 |        34.69 | 100% bias, collisions, distrib |
| FNV             |      740.34 |        41.87 | 100% bias, collisions, distrib |
| FNV64           |      744.22 |        42.51 | 100% bias, collisions, distrib |
| bernstein       |      753.62 |        44.73 | 100% bias, collisions, distrib |
| sdbm            |      737.83 |        30.57 | 100% bias, collisions, distrib |
| x17             |      497.34 |        36.09 | 100% bias, collisions, distrib |
| JenkinsOOAT     |      425.83 |        83.75 | 99.98% bias, collisions, distrib |
| JenkinsOOAT_perl|      452.49 |        43.62 | 53.5% bias, collisions, distrib |
| lookup3         |     1685.01 |        31.54 | 1.5-11.5% bias, 7.2x collisions |
| superfast       |     1532.52 |        34.72 | 28% bias, collisions, 30% distr |
| MurmurOAAT      |      431.89 |        39.36 | 91% bias, 5273.01x collisions, 37% distr |
| Crap8           |     3064.24 |        23.01 | collisions, 99.998% distr      |
| City64          |     9678.77 |        30.13 | 2 minor collisions             |
| City128         |     9750.69 |        46.78 |                                |
| CityCrc128      |    12871.94 |        56.12 |                                |
| SipHash         |      779.74 |       154.68 |                                |
| Spooky32        |     9223.07 |        51.10 |                                |
| Spooky64        |     9189.83 |        52.43 |                                |
| Spooky128       |     8976.22 |        51.62 |                                |
| Murmur2         |     3024.36 |        25.93 | 2.42% bias, collisions, 2% distrib |
| Murmur2A        |     3063.94 |        36.69 | 1.7% bias, 81x coll, 1.7% distrib  |
| Murmur2B        |     5946.77 |        30.34 | 12.7% bias                     |
| Murmur2C        |     3883.56 |        33.94 | 1.8% bias, collisions, 3.4% distrib |
| Murmur3A        |     2343.75 |        34.03 |                                |
| Murmur3C        |     3046.89 |        63.66 |                                |
| Murmur3F        |     4952.79 |        43.52 |                                |
| PMurHash32      |     1651.11 |        47.38 |                                |

I added some SSE assisted hashes.
See [https://code.google.com/p/smhasher/w/list](https://code.google.com/p/smhasher/w/list).

* [http://bench.cr.yp.to/primitives-hash.html](http://bench.cr.yp.to/primitives-hash.html) lists the benchmarks of all currently tested secure hashes.
* The [Hash Function Lounge](http://www.larc.usp.br/~pbarreto/hflounge.html) overviews the known weaknesses and attacks.

Hash functions for symbol tables or hash tables typically use 32 bit hashes,
for databases typically 64 or 128bit, for crypto more than 256 bit.

TODO
----
Some popular slower cryptographic hashes or more secure hashes are still missing.
BLAKE2, SHA-2, SHA-3.

SECURITY
--------

The attacks described in [SipHash](https://131002.net/siphash/) against City, Murmur or Perl
JenkinsOAAT or at [Hash Function Lounge](http://www.larc.usp.br/~pbarreto/hflounge.html) are not included here.

Such an attack avoidance cannot not be the problem of the hash function, but the collision resolution scheme.
You can attack every single hash function, even the best, if you detect the seed, so you need to protect
your collision handling scheme from the worst-case O(n), i.e. separate chaining with linked lists.
