SMhasher
========

[![Linux Build status](https://travis-ci.org/rurban/smhasher.svg?branch=master)](https://travis-ci.org/rurban/smhasher/) [![Windows Build status](https://ci.appveyor.com/api/projects/status/tb6ckfcrhqi6pwn9/branch/master?svg=true)](https://ci.appveyor.com/project/rurban/smhasher) [![FreeBSD Build status](https://api.cirrus-ci.com/github/rurban/smhasher.svg?branch=master)](https://cirrus-ci.com/github/rurban/smhasher)

| Hash function                                 |      MiB/sec |cycl./hash|cycl./map   | size| Quality problems               |
|:----------------------------------------------|-------------:|---------:|-----------:|----:|--------------------------------|
| [donothing32](doc/donothing32.txt)            |  15363872.79 |     6.00 | -|  13 | test NOP                       |
| [donothing64](doc/donothing64.txt)            |  15401240.14 |     6.00 | -|  13 | test NOP                       |
| [donothing128](doc/donothing128.txt)          |  15377357.10 |     6.00 | -|  13 | test NOP                       |
| [NOP_OAAT_read64](doc/NOP_OAAT_read64.txt)    |     29111.46 |    18.17 | -|  47 | test NOP                       |
| [BadHash](doc/BadHash.txt)                    |       524.81 |    95.70 | -|  47 | test FAIL                      |
| [sumhash](doc/sumhash.txt)                    |      7168.98 |    31.10 | -| 363 | test FAIL                      |
| [sumhash32](doc/sumhash32.txt)                |     23537.84 |    21.86 | -| 863 | UB, test FAIL                  |
| [multiply_shift](doc/multiply_shift.txt)      |      4909.58 |    45.35 | too slow| 345 | fails all tests                |
| [pair_multiply_shift](doc/pair_multiply_shift)|     13604.46 |    31.71 | too slow   | 609 | UB, fails all tests            |
| --------------------------                    |              |          |            |     |                                |
| [crc32](doc/crc32.txt)                        |       392.07 |   129.91 | 201.59 (2) | 422 | insecure, 8590x collisions, distrib, PerlinNoise |
| [md5_32a](doc/md5_32a.txt)                    |       353.74 |   630.50 | 794.71 (14)|4419 | 8590x collisions, distrib |
| [sha1_32a](doc/sha1_32a.txt)                  |       353.03 |  1385.80 |1759.94 (5) |5126 | Cyclic low32, 36.6% distrib |
| [md5-128](doc/md5-128.txt)                    |       353.76 |   637.87 | 805.13 (10) |4419 |                    |
| [sha1-160](doc/sha1-160.txt)                  |       364.95 |  1470.55 |1794.16 (13)|5126 | Comb/Cyclic low32  |
| [sha2-224](doc/sha2-224.txt)                  |       147.13 |  1354.81 |1589.92 (12)|     | Comb low32    |
| [sha2-224_64](doc/sha2-224_64.txt)            |       147.60 |  1360.10 |1620.93 (13)|     | Cyclic low32  |
| [sha2-256](doc/sha2-256.txt)                  |       147.80 |  1374.90 |1606.06 (16)|     | Moment Chi2 4 |
| [sha2-256_64](doc/sha2-256_64.txt)            |       148.01 |  1376.34 |1624.71 (16)|     | Moment Chi2 7 |
| [sha1ni](doc/sha1ni.txt)                      |      2019.96 |   135.84 | 564.40 (6) | 989 | insecure,sanity, Permutation, Zeroes, machine-specific |
| [sha1ni_32](doc/sha1ni_32.txt)                |      2019.94 |   136.82 | 589.46 (1) | 989 | machine-specific |
| [sha2ni-256](doc/sha2ni-256.txt)              |      1906.77 |   145.47 | 603.08 (22)|4241 | insecure,sanity, Permutation, Zeroes, machine-specific |
| [sha2ni-256_64](doc/sha2ni-256_64.txt)        |      1910.34 |   146.06 | 595.16 (6) |4241 | Zeroes, machine-specific |
| [blake3_c](doc/blake3_c.txt)                  |      1282.00 |   349.97 | 508.50 (2) |     | Moment Chi2, Seed, PerlinNoise, no 32bit portability |
| [rmd128](doc/rmd128.txt)                      |       334.05 |   652.25 | 838.30 (7) |     |               |
| [rmd160](doc/rmd160.txt)                      |       202.16 |  1045.79 |1287.74 (16)|     | Cyclic hi32   |
| [rmd256](doc/rmd256.txt)                      |       362.46 |   614.40 | 804.67 (8) |     |               |
| [blake2s-128](doc/blake2s-128.txt)            |       295.30 |   698.09 |1059.24 (51)|     |               |
| [blake2s-160](doc/blake2s-160.txt)            |       215.01 |  1026.74 |1239.54 (11)|     |               |
| [blake2s-224](doc/blake2s-224.txt)            |       207.06 |  1063.86 |1236.50 (20)|     |               |
| [blake2s-256](doc/blake2s-256.txt)            |       215.28 |  1014.88 |1230.38 (28)|     |               |
| [blake2s-256_64](doc/blake2s-256_64.txt)      |       211.52 |  1044.22 |1228.43 (8) |     |               |
| [blake2b-160](doc/blake2b-160.txt)            |       356.08 |  1236.84 |1458.15 (12)|     |               |
| [blake2b-224](doc/blake2b-224.txt)            |       356.59 |  1228.50 |1425.87 (16)|     |               |
| [blake2b-256](doc/blake2b-256.txt)            |       355.97 |  1232.22 |1443.31 (19)|     | Sparse high 32-bit |
| [blake2b-256_64](doc/blake2b-256_64.txt)      |       356.97 |  1222.76 |1435.03 (9) |     |               |
| [sha3-256](doc/sha3-256.txt)                  |       100.58 |  3877.18 |4159.79 (37)|     | PerlinNoise   |
| [sha3-256_64](doc/sha3-256_64.txt)            |       100.57 |  3909.00 |4174.63 (16)|     | PerlinNoise   |
| [hasshe2](doc/hasshe2.txt)                    |      2374.20 |    75.71 | 224.24 (3) | 445 | insecure, Permutation,TwoBytes,Zeroes,Seed  |
| [crc32_hw](doc/crc32_hw.txt)                  |      6330.28 |    29.68 | 173.40 (5) | 653 | insecure, 100% bias, collisions, distrib, BIC, machine-specific (x86 SSE4.2) |
| [crc32_hw1](doc/crc32_hw1.txt)                |     23028.02 |    35.36 | 179.06 (2) | 671 | insecure, 100% bias, collisions, distrib, BIC, machine-specific (x86 SSE4.2) |
| [crc64_hw](doc/crc64_hw.txt)                  |      8439.94 |    29.25 | 140.24 (2) | 652 | insecure, 100% bias, collisions, distrib, BIC, machine-specific (x64 SSE4.2) |
| [crc32_pclmul](doc/crc32_pclmul.txt)          |   1993401.62 |     8.01 |    -       | 481 | insecure, 100% bias, collisions, distrib, BIC, machine-specific (x86 SSE4.2+PCLMUL) |
| [o1hash](doc/o1hash.txt)                      |  13646953.39 |    17.35 | 153.63 (1) | 101 | insecure, zeros, fails all tests |
| [fibonacci](doc/fibonacci.txt)                |     11344.83 |    27.01 | 745.72 (2) |1692 | UB, zeros, fails all tests       |
| [FNV1a](doc/FNV1a.txt)                        |       791.82 |    69.29 | 190.41 (2) | 204 | zeros, fails all tests       |
| [FNV1A_Totenschiff](doc/FNV1A_Totenschiff.txt)|      6258.32 |    36.37 | 187.63 (4) | 270 | UB, zeros, fails all tests   |
| [FNV1A_Pippip_Yurii](doc/FNV1A_Pippip_Yurii.txt)|    6258.64 |    36.87 | 184.57 (1) | 147 | UB, sanity, fails all tests  |
| [FNV1a_YT](doc/FNV1a_YT.txt)                  |      9627.47 |    27.99 | 188.26 (3) | 321 | UB, fails all tests          |
| [FNV2](doc/FNV2.txt)                          |      6330.75 |    37.95 | 162.60 (3) | 278 | fails all tests              |
| [FNV64](doc/FNV64.txt)                        |       791.82 |    69.30 | 166.63 (2) |  79 | fails all tests              |
| [fletcher2](doc/fletcher2.txt)                |     11996.71 |    26.28 | 308.36 (2) | 248 | UB, fails all tests          |
| [fletcher4](doc/fletcher4.txt)                |     11982.16 |    26.29 | 307.61 (2) | 371 | UB, fails all tests          |
| [bernstein](doc/bernstein.txt)                |       791.81 |    66.61 | 193.13 (1) |  41 | fails all tests              |
| [sdbm](doc/sdbm.txt)                          |       791.83 |    66.59 | 179.03 (3) |  41 | fails all tests              |
| [x17](doc/x17.txt)                            |       527.90 |    96.64 | 185.01 (1) |  79 | 99.98% bias, fails all tests |
| [JenkinsOOAT](doc/JenkinsOOAT.txt)            |       452.49 |   141.09 | 225.02 (2) | 153 | 53.5% bias, fails all tests  |
| [JenkinsOOAT_perl](doc/JenkinsOOAT_perl.txt)  |       452.49 |   118.47 | 205.72 (2) |  65 | 1.5-11.5% bias, 7.2x collisions, BIC, LongNeighbors |
| [MicroOAAT](doc/MicroOAAT.txt)                |       977.99 |    60.01 | 196.69 (2) |  68 | 100% bias, distrib, BIC      |
| [VHASH_32](doc/VHASH_32.txt)                  |      9365.29 |    81.72 | 251.89 (3) |1231 | sanity, Seed, MomentChi2     |
| [VHASH_64](doc/VHASH_64.txt)                  |      9379.00 |    81.73 | 227.94 (5) |1231 | sanity, Seed, Sparse         |
| [farsh32](doc/farsh32.txt)                    |     14053.09 |    74.29 | 245.33 (3) | 944 | insecure: AppendedZeroes, collisions+bias, MomentChi2, LongNeighbors |
| [farsh64](doc/farsh64.txt)                    |      7216.29 |   130.30 | 302.44 (3) | 944 | insecure: AppendedZeroes, collisions+bias, MomentChi2, LongNeighbors |
| [jodyhash32](doc/jodyhash32.txt)              |      1428.42 |    43.90 | 183.40 (2) | 102 | bias, collisions, distr, BIC LongNeighbors |
| [jodyhash64](doc/jodyhash64.txt)              |      2848.24 |    39.45 | 166.13 (1) | 118 | bias, collisions, distr, BIC, LongNeighbors |
| [lookup3](doc/lookup3.txt)                    |      1658.24 |    47.41 | 183.32 (2) | 341 | UB, 28% bias, collisions, 30% distr, BIC  |
| [superfast](doc/superfast.txt)                |      1956.27 |    52.57 | 180.58 (2) | 210 | UB, 91% bias, 5273.01x collisions, 37% distr, BIC |
| [MurmurOAAT](doc/MurmurOAAT.txt)              |       452.49 |   112.37 | 198.94 (4) |  47 | collisions, 99.998% distr., BIC, LongNeighbors |
| [Crap8](doc/Crap8.txt)                        |      3149.35 |    34.12 | 198.21 (2) | 342 | UB, 2.42% bias, collisions, 2% distrib |
| [Murmur2](doc/Murmur2.txt)                    |      3146.96 |    40.59 | 177.57 (2) | 358 | UB, 1.7% bias, 81x coll, 1.7% distrib, BIC |
| [Murmur2A](doc/Murmur2A.txt)                  |      3146.80 |    45.13 | 181.27 (2) | 407 | UB, 12.7% bias, LongNeighbors          |
| [Murmur2B](doc/Murmur2B.txt)                  |      4883.71 |    46.41 | 152.22 (2) | 433 | UB, 1.8% bias, collisions, 3.4% distrib, BIC |
| [Murmur2C](doc/Murmur2C.txt)                  |      4091.10 |    46.02 | 167.25 (2) | 444 | UB, 91% bias, collisions, distr, BIC, LongNeighbors |
| [Murmur3A](doc/Murmur3A.txt)                  |      2413.88 |    50.67 | 195.02 (2) | 351 | UB, Moment Chi2 69             |
| [PMurHash32](doc/PMurHash32.txt)              |      2344.47 |    51.16 | 204.90 (3) |1862 | Moment Chi2 69             |
| [Murmur3C](doc/Murmur3C.txt)                  |      3198.90 |    66.11 | 208.67 (2) | 859 | UB, LongNeighbors, Text, DiffDist  |
| [PMPML_32](doc/PMPML_32.txt)                  |      6835.62 |    50.73 | 188.80 (2) |1084 | Avalanche >512, unseeded: Seed, BIC, MomentChi2, PerlinNoise |
| [PMPML_64](doc/PMPML_64.txt)                  |      8266.88 |    58.75 | 171.93 (3) |1305 | unseeded: Seed, MomentChi2, BIC        |
| [xxHash32](doc/xxHash32.txt)                  |      6041.37 |    47.06 | 179.55 (2) | 738 | LongNeighbors, collisions with 4bit diff, MomentChi2 220 |
| [metrohash64](doc/metrohash64.txt)            |      9667.77 |    49.57 | 153.05 (2) | 624 | UB, LongNeighbors, BIC         |
| [metrohash64_1](doc/metrohash64_1.txt)        |      9676.40 |    49.61 | 161.54 (1) | 624 | UB, LongNeighbors, BIC, MomentChi2         |
| [metrohash64crc_1](doc/metrohash64crc_1.txt)  |     13920.67 |    54.71 | 154.81 (1) | 632 | UB, Cyclic 8/8 byte, DiffDist, BIC, MomentChi2, machine-specific (x64 SSE4.2) |
| [metrohash64crc_2](doc/metrohash64crc_2.txt)  |     13978.81 |    54.69 | 154.65 (2) | 632 | UB, Cyclic 8/8 byte, DiffDist, BIC, machine-specific (x64 SSE4.2) |
| [cmetrohash64_1o](doc/cmetrohash64_1o.txt)    |      9674.20 |    49.01 | 160.53 (2) |3506 | UB, LongNeighbors, BIC, MomentChi2  |
| [cmetrohash64_1](doc/cmetrohash64_1.txt)      |      9688.52 |    49.41 | 155.12 (1) | 652 | UB, LongNeighbors, BIC, MomentChi2 |
| [City64noSeed](doc/City64noSeed.txt)          |      9090.85 |    38.50 | 172.55 (2) |1038 | Avalanche, Sparse, TwoBytes, MomentChi2, Seed |
| [City64](doc/City64.txt)                      |      9094.53 |    53.98 | 187.77 (2) |1120 | Sparse, TwoBytes           |
| [t1ha1_64le](doc/t1ha1_64le.txt)              |      9713.65 |    37.84 | 190.33 (2) | 517 | Avalanche                  |
| [t1ha1_64be](doc/t1ha1_64be.txt)              |      7466.93 |    39.74 | 181.94 (2) | 555 | Avalanche                  |
| [t1ha0_32le](doc/t1ha0_32le.txt)              |      5144.99 |    51.14 | 194.40 (2) | 509 | Sparse, LongNeighbors      |
| [t1ha0_32be](doc/t1ha0_32be.txt)              |      4582.17 |    51.14 | 184.72 (2) | 533 | Sparse, LongNeighbors      |
| [t1ha2_stream](doc/t1ha2_stream.txt)          |      9066.36 |    76.29 | 226.95 (3) |1665 | Sparse, Permutation, LongNeighbors |
| [t1ha2_stream128](doc/t1ha2_stream128.txt)    |      9065.72 |    94.17 | 258.51 (4) |1665 | Sparse, Permutation, LongNeighbors |
| [aesnihash](doc/aesnihash.txt)                |      2963.39 |    71.24 | 217.73 (3) |     | fails most tests, machine-specific (x64 AES-NI) |
| [falkhash](doc/falkhash.txt)                  |     20184.26 |   169.44 | 320.82 (5) | 264 | Sparse, LongNeighbors, machine-specific (x64 AES-NI) |
| [MeowHash](doc/MeowHash.txt)                  |     36695.65 |    58.09 | 233.81 (3) |1764 | Sparse low32, machine-specific (x64 AES-NI)  |
| [MeowHash32low](doc/MeowHash32low.txt)        |     17247.34 |    87.32 | 245.98 (3) |1764 | Sparse, machine-specific (x64 AES-NI.txt)    |
| --------------------------------------        |              |          |            |     |                            |
| [tifuhash_64](doc/tifuhash_64.txt)            |        35.60 |  1679.52 |1212.75 (15)| 276 | Cyclic low32               |
| [beamsplitter](doc/beamsplitter.txt)          |       789.22 |   682.45 |1150.33 (26)|4203 | UB                         |
| [floppsyhash](doc/floppsyhash.txt)            |        35.72 |  1868.92 |1411.07 (7) | 623 |                           |
| [chaskey](doc/chaskey.txt)                    |       753.28 |   151.11 | 289.32 (3) |1609 | PerlinNoise                |
| [SipHash](doc/SipHash.txt)                    |       980.83 |   133.39 | 236.15 (3) |1071 |                            |
| [HalfSipHash](doc/HalfSipHash.txt)            |       755.69 |   114.06 | 232.89 (3) | 700 | zeroes                     |
| [GoodOAAT](doc/GoodOAAT.txt)                  |      1052.30 |    71.32 | 204.96 (4) | 237 |                            |
| [prvhash42_32](doc/prvhash42_32.txt)          |      1384.36 |    62.35 | 205.48 (4) | 476 |                            |
| [prvhash42_64](doc/prvhash42_64.txt)          |      1392.04 |    90.99 | 231.19 (3) | 960 |                            |
| [prvhash42_128](doc/prvhash42_128.txt)        |      1394.88 |   117.48 | 252.39 (4) | 432 |                            |
| [prvhash42s_32](doc/prvhash42s_32.txt)        |      6588.60 |   152.18 | 318.57 (6) |2574 |                            |
| [prvhash42s_64](doc/prvhash42s_64.txt)        |      6539.39 |   187.85 | 348.30 (2) |2637 |                            |
| [prvhash42s_128](doc/prvhash42s_128.txt)      |      6546.06 |   305.70 | 476.15 (4) |2653 |                            |
| [SipHash13](doc/SipHash13.txt)                |      1888.99 |    93.94 | 207.27 (2) | 778 | 0.9% bias                  |
| [BEBB4185](doc/BEBB4185.txt)                  |      2951.01 |   213.86 | 346.87 (4) |1294 | msvc-specific verification |
| [TSip](doc/TSip.txt)                          |      3228.59 |    58.31 | 201.86 (2) | 519 | !msvc                      |
| [seahash](doc/seahash.txt)                    |      4830.94 |    64.23 | 203.77 (3) | 871 | PerlinNoise, !msvc         |
| [seahash32low](doc/seahash32low.txt)          |      4828.56 |    64.23 | 224.75 (2) | 871 | PerlinNoise 32, !msvc      |
| [clhash](doc/clhash.txt)                      |      4012.36 |    83.37 | 241.30 (3) |1809 | PerlinNoise, machine-specific (x64 SSE4.2) |
| [HighwayHash64](doc/HighwayHash64.txt)        |      6245.28 |    98.68 | 250.67 (4) |2546 |                            |
| [Murmur3F](doc/Murmur3F.txt)                  |      5225.91 |    50.16 | 180.28 (3) | 699 | UB                         |
| [fasthash32](doc/fasthash32.txt)              |      4736.91 |    50.48 | 182.25 (3) | 566 | UB                         |
| [fasthash64](doc/fasthash64.txt)              |      4737.49 |    47.81 | 158.17 (1) | 509 | UB, Moment Chi2 5159 !     |
| [MUM](doc/MUM.txt)                            |      7127.75 |    39.79 | 176.29 (2) |1912 | UB, machine-specific (32/64 differs) |
| [MUMlow](doc/MUMlow.txt)                      |      7123.97 |    39.77 | 186.61 (3) |1912 | UB                         |
| [mirhash](doc/mirhash.txt)                    |      5412.29 |    42.77 | 162.87 (2) |1112 | UB, LongNeighbors, machine-specific (32/64 differs) |
| [mirhash32low](doc/mirhash32low.txt)          |      5415.48 |    42.73 | 180.08 (1) |1112 | UB, Cyclic, LongNeighbors, machine-specific (32/64 differs) |
| [mirhashstrict](doc/mirhashstrict.txt)        |      2217.39 |    64.91 | 178.74 (2) |1112 |                            |
| [mirhashstrict32low](doc/mirhashstrict32low.txt)|    2219.08 |    64.91 | 198.50 (1) |1112 | MomentChi2 9               |
| [mx3](doc/mx3.txt)                            |      6152.73 |    55.32 | 178.00 (3) | 734 | UB                         |
| [pengyhash](doc/pengyhash.txt)                |      7146.53 |    92.22 | 243.13 (4) | 421 |                            |
| [City32](doc/City32.txt)                      |      3676.34 |    53.75 | 199.72 (2) |1319 |                            |
| [City64low](doc/City64low.txt)                |      9165.70 |    54.14 | 202.28 (2) |1120|                             |
| [City128](doc/City128.txt)                    |      9675.32 |    88.87 | 235.59 (2) |1841 |                            |
| [CityCrc128](doc/CityCrc128.txt)              |     12358.42 |    75.02 | 213.07 (3) | 295 |                            |
| [FarmHash32](doc/FarmHash32.txt)              |     17311.94 |    54.87 | 204.48 (3) |11489| machine-specific (x64 SSE4/AVX) |
| [FarmHash64](doc/FarmHash64.txt)              |      8729.80 |    54.23 | 188.47 (2) |3758 |                            |
| [FarmHash128](doc/FarmHash128.txt)            |      9414.56 |    74.28 | 221.84 (4) | 163 |                            |
| [farmhash32_c](doc/farmhash32_c.txt)          |     16361.97 |    54.93 | 214.82 (3) | 762 | machine-specific (x64 SSE4/AVX) |
| [farmhash64_c](doc/farmhash64_c.txt)          |      8704.51 |    54.29 | 187.58 (2) |3688 |                            |
| [farmhash128_c](doc/farmhash128_c.txt)        |      9254.42 |    74.27 | 209.51 (4) |1890 |                            |
| [metrohash64_2](doc/metrohash64_2.txt)        |      9650.51 |    49.63 | 163.62 (1) | 627 | UB, LongNeighbors          |
| [cmetrohash64_2](doc/cmetrohash64_2.txt)      |      9666.90 |    49.63 | 150.14 (2) | 655 | LongNeighbors              |
| [metrohash128](doc/metrohash128.txt)          |      9550.64 |    62.60 | 177.25 (2) | 773 | UB, LongNeighbors          |
| [metrohash128_1](doc/metrohash128_1.txt)      |      9569.53 |    62.81 | 176.30 (1) | 773 | UB, LongNeighbors          |
| [metrohash128_2](doc/metrohash128_2.txt)      |      9341.15 |    62.90 | 175.80 (2) | 773 | UB, LongNeighbors          |
| [metrohash128crc_1](doc/metrohash128crc_1.txt)|     14020.49 |    69.06 | 170.53 (2) | 723 | UB, machine-specific (x64 SSE4.2) |
| [metrohash128crc_2](doc/metrohash128crc_2.txt)|     13846.10 |    69.02 | 167.42 (2) | 723 | UB, machine-specific (x64 SSE4.2) |
| [xxHash64](doc/xxHash64.txt)                  |      8935.34 |    57.20 | 168.89 (2) |1999 |                            |
| [Spooky32](doc/Spooky32.txt)                  |      9709.28 |    67.83 | 205.63 (2) |2221 | UB                         |
| [Spooky64](doc/Spooky64.txt)                  |      9718.08 |    67.82 | 194.79 (2) |2221 | UB                         |
| [Spooky128](doc/Spooky128.txt)                |      9720.68 |    69.79 | 183.69 (2) |2221 | UB                         |
| [xxh3](doc/xxh3.txt)                          |     16553.27 |    37.10 | 176.43 (2) | 744 | DiffDist bit 7 w. 36 bits, BIC |
| [xxh3low](doc/xxh3low.txt)                    |     16465.30 |    37.11 | 202.98 (2) | 756 |                            |
| [xxh128](doc/xxh128.txt)                      |     15180.48 |    44.71 | 195.81 (2) |1012 |                            |
| [xxh128low](doc/xxh128low.txt)                |     15139.22 |    37.97 | 187.54 (3) |1012 |                            |
| [t1ha2_atonce](doc/t1ha2_atonce.txt)          |      9244.14 |    47.52 | 191.56 (2) | 541 | Zeroes low3                |
| [t1ha2_atonce128](doc/t1ha2_atonce128.txt)    |      8349.04 |    62.79 | 199.41 (2) | 613 | LongNeighbors              |
| [t1ha0_aes_noavx](doc/t1ha0_aes_noavx.txt)    |     22596.49 |    47.45 | 180.48 (3) | 925 | LongNeighbors, machine-specific (x86 AES-NI) |
| [t1ha0_aes_avx1](doc/t1ha0_aes_avx1)          |     22714.85 |    48.12 | 226.52 (16)| 843 | LongNeighbors, machine-specific (x64 AVX.txt)|
| [t1ha0_aes_avx2](doc/t1ha0_aes_avx2.txt)      |     22345.33 |    44.38 | 556.47 (89)| 792 | LongNeighbors, machine-specific (x64 AVX2)   |
| [wyhash32](doc/wyhash32.txt)                  |      2532.89 |    48.40 | 484.57 (1) | 410 | 32-bit                     |
| [wyhash32low](doc/wyhash32low.txt)            |     11920.73 |    32.92 | 183.61 (2) | 922 |                            |
| [wyhash](doc/wyhash.txt)                      |     12195.62 |    32.91 | 178.14 (2) | 938 |                            |

The [sortable table variant](http://rurban.github.io/smhasher/doc/table.html)

smhasher is being sponsored by the following tool; please help to support us
by taking a look and signing up to a free trial.
<a href="https://tracking.gitads.io/?repo=smhasher">
<img src="https://images.gitads.io/smhasher" alt="GitAds"/></a>

Summary
-------

I added some SSE assisted hashes and fast intel/arm CRC32-C, AES and SHA HW variants.
See also the old [https://code.google.com/p/smhasher/w/list](https://code.google.com/p/smhasher/w/list) and the improved, but unmaintained fork [https://github.com/demerphq/smhasher](https://github.com/demerphq/smhasher).

So the fastest hash functions on x86_64 without quality problems are:

- xxh3low
- wyhash
- t1ha2_atonce
- FarmHash (_not portable, too machine specific: 64 vs 32bit, old gcc, ..._)
- Spooky32
- pengyhash
- mx3
- MUM/mir (_different results on 32/64-bit archs_)
- fasthash32

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
the 2nd sanity AppendZeroes test against `\0` invariance is security relevant.

Columns
-------

MiB/sec: The average of the Bulk key speed test for alignments 0-7 with 262144-byte keys.
The higher the better.

cycl./hash: The average of the Small key speed test for 1-31 byte keys.
The smaller the better.

cycl./map: The result of the Hashmap test for /usr/dict/words with
fast C++ hashmap get queries, with the standard deviation in
brackets. This tests the inlinability of the hash function in practise (see size).
The smaller the better.

size: The object size in byte on AMD64. This affects the inlinability in e.g. hash tables.
The smaller the better.

Quality problems: See the failures in the linked doc.
The less the better.

Other
-----

* http://nohatcoder.dk/2019-05-19-1.html gives a new, useful hash level classification 1-5.
* [http://www.strchr.com/hash_functions](http://www.strchr.com/hash_functions) lists other benchmarks and quality of most simple and fast hash functions.
* [http://bench.cr.yp.to/primitives-hash.html](http://bench.cr.yp.to/primitives-hash.html) lists the benchmarks of all currently tested secure hashes.
* http://valerieaurora.org/hash.html Lifetimes of cryptographic hash functions

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

CRYPTO
------

Our crypto hashes are hardened with added seed, mixed into the initial
state, and the versions which require zero-padding are hardened by
adding the len also, to prevent from collisions with AppendedZeroes
for the padding. The libtomcrypt implementations already provide for
that, but others might not. Without, such crypto hash functions are
unsuitable for normal tasks, as it's trivial to create collisions by
padding.

The official NIST hash function testsuite does not do such extensive
statistical tests, to search for weak ranges in the bits. Also crypto
does not change the initial state, which we do here for our random
32bit seed. Crypto mostly cares about unreversable key -> hash
functions without changing the initial fixed state and
timings/sidechannel attacks.

The NIST "Cryptographic Algorithm Validation Program" (CAVP) involves
the testing of the implementations of FIPS-approved and
NIST-recommended cryptographic algorithms. During the NIST SHA-3
competition, the testing methodology was borrowed from the "CAVP",
as the KATs and MCTs of the [SHA-3 Competition Test Suite](https://csrc.nist.gov/CSRC/media/Projects/Cryptographic-Algorithm-Validation-Program/documents/sha3/sha3vs.pdf)
were based on the CAVP tests for SHA-2. In addition to this, the “Extremely Long
Message Test,” not present in the CAVP for SHA-2, required the
submitters to generate the hash value corresponding to a message with
a length of 1 GiB. “NIST - Cryptographic Algorithm Validation Program (CAVP),”
June 2017. Available: http://csrc.nist.gov/groups/STM/cavp
(No testing source code provided, just high-level descriptions)

Two other independent third party testsuites found an extensive number
of bugs and weaknesses in the SHA3 candidates.
"Finding Bugs in Cryptographic Hash Function Implementations",
Nicky Mouha, Mohammad S Raunak, D. Richard Kuhn, and Raghu Kacker, 2017.
https://eprint.iacr.org/2017/891.pdf

Maybe independent researchers should come together to do a better
public SHA-4 round, based on better and more testing methods, open
source code for the tests, and using standard industry practices, such
as valgrind, address-sanitizer and ubsan to detect obvious bugs.

PROBLEMS
--------

Typical undefined behaviour (**UB**) problems:

* Misaligned

  Many word-wise hashes (in opposite to safe byte-wise processing)
  don't check the input buffer for proper word alignment, which will
  fail with ubsan or Sparc. word being `int32_t` or `int64_t` or even
  more.  On some old RISC hardware this will be a BUS error, you can
  even let Intel HW generate such a bus error by setting some CPU
  flag. But generally using misaligned accesses is fine.

  These are: mx3, Spooky, mirhash (_but not strict_), MUM, fasthash,
  Murmur3*, Murmur2*, metrohash* (all but cmetro*), Crap8, discohash,
  beamsplitter, lookup3, fletcher4, fletcher2, all sanmayce
  FNV1a\_ variants (FNV1a\_YT, FNV1A\_Pippip\_Yurii,
  FNV1A\_Totenschiff, ...), fibonacci.

  The usual mitigation is to check the buffer alignment either in the
  caller, provide a pre-processing loop for the misaligned prefix, or
  copy the whole buffer into a fresh aligned area.

* oob - Out of bounds

  Some hash function assume a padded input buffer which can be
  accessed past its length up to the word size. This allows for faster
  loop processing, as no 2nd loop or switch table for the rest is
  needed, but it requires a cooperative calling enviroment and is as
  such considered cheating.

* Signed integer overflow

  A simple type error, this hash needs to use unsigned integer types
  internally, to avoid undefined and inconsistent behaviour.
  i.e. SuperFastHash: signed integer overflow: -2147483641 + -113 cannot be represented in type 'int'

* shift exponent overflow

  With: FNV1A\_Pippip\_Yurii, FNV1A_Totenschiff, pair\_multiply\_shift, sumhash32
  shift exponent 64 is too large for 64-bit type 'long unsigned int'
