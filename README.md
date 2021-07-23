SMhasher
========
[![SMHasher support vis xs:code](doc/SMhasher-banner.png)](https://xscode.com/rurban/smhasher)

[![Linux Build status](https://travis-ci.org/rurban/smhasher.svg?branch=master)](https://travis-ci.org/rurban/smhasher/) [![Windows Build status](https://ci.appveyor.com/api/projects/status/tb6ckfcrhqi6pwn9/branch/master?svg=true)](https://ci.appveyor.com/project/rurban/smhasher) [![FreeBSD Build status](https://api.cirrus-ci.com/github/rurban/smhasher.svg?branch=master)](https://cirrus-ci.com/github/rurban/smhasher)


| Hash function                                 |      MiB/sec |cycl./hash|cycl./map   | size| Quality problems               |
|:----------------------------------------------|-------------:|---------:|-----------:|----:|--------------------------------|
| [donothing32](doc/donothing32.txt)            |  15316474.36 |     6.00 |     -      |  13 | bad seed 0, test NOP           |
| [donothing64](doc/donothing64.txt)            |  15330019.19 |     6.00 |     -      |  13 | bad seed 0, test NOP           |
| [donothing128](doc/donothing128.txt)          |  15278983.09 |     6.00 |     -      |  13 | bad seed 0, test NOP           |
| [NOP_OAAT_read64](doc/NOP_OAAT_read64.txt)    |     28467.50 |    18.48 |     -      |  47 | test NOP                       |
| [BadHash](doc/BadHash.txt)                    |       524.81 |    96.20 |     -      |  47 | bad seed 0, test FAIL          |
| [sumhash](doc/sumhash.txt)                    |      7169.08 |    27.12 |     -      | 363 | bad seed 0, test FAIL          |
| [sumhash32](doc/sumhash32.txt)                |     22556.18 |    22.98 |     -      | 863 | UB, test FAIL                  |
| [multiply_shift](doc/multiply_shift.txt)      |      5418.36 |    28.69 | 157.11 (3) | 345 | bad seeds & 0xfffffff0, fails most tests |
| [pair_multiply_shift](doc/pair_multiply_shift)|      3716.95 |    40.22 | 186.34 (3) | 609 | fails most tests |
| --------------------------                    |              |          |            |     |                      |
| [crc32](doc/crc32.txt)                        |       392.10 |   131.62 | 204.58 (4) | 422 | insecure, 8590x collisions, distrib, PerlinNoise |
| [md5_32a](doc/md5_32a.txt)                    |       353.64 |   629.85 | 799.56 (9) |4419 | 8590x collisions, distrib |
| [sha1_32a](doc/sha1_32a.txt)                  |       353.03 |  1385.80 |1759.94 (5) |5126 | Cyclic low32, 36.6% distrib |
| [md5-128](doc/md5-128.txt)                    |       353.76 |   638.29 | 803.39 (9) |4419 |                    |
| [sha1-160](doc/sha1-160.txt)                  |       364.95 |  1470.55 |1794.16 (13)|5126 | Comb/Cyclic low32  |
| [sha2-224](doc/sha2-224.txt)                  |       147.13 |  1354.81 |1589.92 (12)|     | Bad seeds, Comb low32    |
| [sha2-224_64](doc/sha2-224_64.txt)            |       147.60 |  1360.10 |1620.93 (13)|     | Bad seeds, Cyclic low32  |
| [sha2-256](doc/sha2-256.txt)                  |       147.80 |  1374.90 |1606.06 (16)|     | Bad seeds, Moment Chi2 4 |
| [sha2-256_64](doc/sha2-256_64.txt)            |       148.01 |  1376.34 |1624.71 (16)|     | Bad seeds, Moment Chi2 7 |
| [sha1ni](doc/sha1ni.txt)                      |      2019.96 |   135.84 | 564.40 (6) | 989 | insecure,sanity, Permutation, Zeroes, machine-specific |
| [sha1ni_32](doc/sha1ni_32.txt)                |      2019.94 |   136.82 | 589.46 (1) | 989 | machine-specific |
| [sha2ni-256](doc/sha2ni-256.txt)              |      1906.77 |   145.47 | 603.08 (22)|4241 | insecure,sanity, Permutation, Zeroes, machine-specific |
| [sha2ni-256_64](doc/sha2ni-256_64.txt)        |      1910.34 |   146.06 | 595.16 (6) |4241 | Zeroes, machine-specific |
| [blake3_c](doc/blake3_c.txt)                  |      1285.91 |   340.01 | 552.63 (3) |     | no 32bit portability     |
| [rmd128](doc/rmd128.txt)                      |       334.36 |   659.03 | 838.32 (9) |     | Bad seeds                |
| [rmd160](doc/rmd160.txt)                      |       202.16 |  1045.79 |1287.74 (16)|     | Bad seeds, Cyclic hi32   |
| [rmd256](doc/rmd256.txt)                      |       362.49 |   617.02 | 815.44 (8) |     | Bad seeds     |
| [blake2s-128](doc/blake2s-128.txt)            |       295.30 |   698.09 |1059.24 (51)|     |               |
| [blake2s-160](doc/blake2s-160.txt)            |       215.01 |  1026.74 |1239.54 (11)|     |               |
| [blake2s-224](doc/blake2s-224.txt)            |       207.06 |  1063.86 |1236.50 (20)|     |               |
| [blake2s-256](doc/blake2s-256.txt)            |       215.28 |  1014.88 |1230.38 (28)|     |               |
| [blake2s-256_64](doc/blake2s-256_64.txt)      |       211.52 |  1044.22 |1228.43 (8) |     |               |
| [blake2b-160](doc/blake2b-160.txt)            |       356.08 |  1236.84 |1458.15 (12)|     |               |
| [blake2b-224](doc/blake2b-224.txt)            |       356.59 |  1228.50 |1425.87 (16)|     |               |
| [blake2b-256](doc/blake2b-256.txt)            |       355.97 |  1232.22 |1443.31 (19)|     | Sparse high 32-bit |
| [blake2b-256_64](doc/blake2b-256_64.txt)      |       356.97 |  1222.76 |1435.03 (9) |     |               |
| [asconhashv12](doc/asconhashv12.txt)          |       144.98 |   885.02 |1324.23 (38)|4341 |               |
| [asconhashv12_64](doc/asconhashv12_64.txt)    |        86.73 |   684.02 | 606.93 (6) |6490 |               |
| [sha3-256](doc/sha3-256.txt)                  |       100.58 |  3877.18 |4159.79 (37)|     | PerlinNoise   |
| [sha3-256_64](doc/sha3-256_64.txt)            |       100.57 |  3909.00 |4174.63 (16)|     | PerlinNoise   |
| [hasshe2](doc/hasshe2.txt)                    |      2372.52 |    68.64 | 216.74 (5) | 445 | Permutation,TwoBytes,Zeroes,Seed|
| [poly_1_mersenne](doc/poly_1_mersenne.txt)    |      1431.65 |    54.49 | 189.52 (3) | 479 | fails most tests |
| [poly_2_mersenne](doc/poly_2_mersenne.txt)    |      1323.69 |    66.93 | 190.88 (2) | 479 |                  |
| [poly_3_mersenne](doc/poly_3_mersenne.txt)    |      1323.59 |    74.86 | 206.77 (2) | 479 |                  |
| [poly_4_mersenne](doc/poly_4_mersenne.txt)    |      1323.57 |    82.67 | 200.36 (4) | 479 |                  |
| [tabulation32](doc/tabulation32.txt)          |      4317.34 |    35.45 | 197.20 (2) | 848 | collisions       |
| [tabulation](doc/tabulation.txt)              |      7621.75 |    42.19 | 179.93 (2) | 554 |                  |
| [crc32_hw](doc/crc32_hw.txt)                  |      6330.42 |    35.55 | 170.16 (1) | 653 | insecure, 100% bias, collisions, distrib, BIC, machine-specific (SSE4.2/NEON) |
| [crc32_hw1](doc/crc32_hw1.txt)                |     23208.73 |    46.74 | 179.70 (2) | 671 | insecure, 100% bias, collisions, distrib, BIC, machine-specific (x86 SSE4.2)  |
| [crc64_hw](doc/crc64_hw.txt)                  |      8440.13 |    34.94 | 141.15 (2) | 652 | insecure, 100% bias, collisions, distrib, BIC, machine-specific (SSE4.2/NEON) |
| [crc32_pclmul](doc/crc32_pclmul.txt)          |           -  |     -    |     -      |     | insecure, 100% bias, collisions, distrib, BIC, machine-specific (x86 SSE4.2+PCLMUL) |
| [o1hash](doc/o1hash.txt)                      |  12439661.09 |    16.77 | 166.13 (1) | 101 | insecure, no seed, zeros, fails all tests |
| [fibonacci](doc/fibonacci.txt)                |     11339.87 |    26.33 | 705.64 (2) |1692 | UB, zeros, fails all tests       |
| [FNV1a](doc/FNV1a.txt)                        |       791.84 |    69.69 | 177.84 (2) | 204 | bad seed, zeros, fails all tests       |
| [FNV1A_Totenschiff](doc/FNV1A_Totenschiff.txt)|      6258.23 |    27.99 | 198.20 (1) | 270 | UB, zeros, fails all tests   |
| [FNV1A_Pippip_Yurii](doc/FNV1A_Pippip_Yurii.txt)|    6258.46 |    28.19 | 184.41 (2) | 147 | UB, sanity, fails all tests  |
| [FNV1a_YT](doc/FNV1a_YT.txt)                  |      9643.42 |    32.06 | 175.19 (2) | 321 | bad seed, UB, fails all tests          |
| [FNV2](doc/FNV2.txt)                          |      6258.84 |    33.25 | 142.89 (1) | 278 | fails all tests              |
| [FNV64](doc/FNV64.txt)                        |       791.82 |    70.24 | 159.29 (1) |  79 | fails all tests              |
| [fletcher2](doc/fletcher2.txt)                |     12011.15 |    25.29 | 298.60 (1) | 248 | bad seed 0, UB, fails all tests          |
| [fletcher4](doc/fletcher4.txt)                |     11928.55 |    25.27 | 293.49 (2) | 371 | bad seed 0, UB, fails all tests          |
| [bernstein](doc/bernstein.txt)                |       791.82 |    68.63 | 180.71 (2) |  41 | bad seed 0, fails all tests              |
| [sdbm](doc/sdbm.txt)                          |       791.84 |    67.69 | 177.06 (2) |  41 | bad seed 0, fails all tests              |
| [x17](doc/x17.txt)                            |       527.90 |    98.78 | 184.09 (2) |  79 | 99.98% bias, fails all tests |
| [JenkinsOOAT](doc/JenkinsOOAT.txt)            |       452.48 |   142.85 | 213.93 (2) | 153 | bad seed 0, 53.5% bias, fails all tests  |
| [JenkinsOOAT_perl](doc/JenkinsOOAT_perl.txt)  |       452.49 |   118.78 | 194.78 (1) |  65 | bad seed 0, 1.5-11.5% bias, 7.2x collisions, BIC, LongNeighbors |
| [MicroOAAT](doc/MicroOAAT.txt)                |       977.60 |    59.61 | 185.06 (2) |  68 | 100% bias, distrib, BIC      |
| [beamsplitter](doc/beamsplitter.txt)          |       789.22 |   682.45 |1150.33 (26)|4203 | UB, too many bad seeds       |
| [BEBB4185](doc/BEBB4185.txt)                  |      2951.62 |   222.03 | 343.63 (4) |1294 | UB, too many bad seeds, msvc-specific |
| [pearsonhash64](doc/pearsonhash64.txt)        |       287.95 |   174.11 | 196.50 (1) |     | Avalanche, Seed, SSSE3 only. broken MSVC     |
| [pearsonhash128](doc/pearsonhash128.txt)      |       287.95 |   171.72 | 194.61 (1) |     | Avalanche, Seed, SSSE3 only. broken MSVC     |
| [pearsonhash256](doc/pearsonhash256.txt)      |       264.51 |   184.87 | 218.79 (0) |     | Avalanche, Seed, SSSE3 only. broken MSVC     |
| [VHASH_32](doc/VHASH_32.txt)                  |      9404.99 |    77.01 | 250.57 (2) |1231 | sanity, Seed, MomentChi2     |
| [VHASH_64](doc/VHASH_64.txt)                  |      9392.39 |    74.72 | 227.92 (2) |1231 | sanity, Seed, Sparse         |
| [farsh32](doc/farsh32.txt)                    |     14053.09 |    74.29 | 245.33 (3) | 944 | insecure: AppendedZeroes, collisions+bias, MomentChi2, LongNeighbors |
| [farsh64](doc/farsh64.txt)                    |      7216.29 |   130.30 | 302.44 (3) | 944 | insecure: AppendedZeroes, collisions+bias, MomentChi2, LongNeighbors |
| [farsh128](doc/farsh128.txt)                  |      3776.92 |   232.48 | 398.67 (6) | 944 | ?? |
| [farsh256](doc/farsh256.txt)                  |      1895.77 |   459.86 | 575.95 (3) | 944 | ?? |
| [jodyhash32](doc/jodyhash32.txt)              |      1428.37 |    44.36 | 185.85 (3) | 102 | bias, collisions, distr, BIC LongNeighbors |
| [jodyhash64](doc/jodyhash64.txt)              |      2848.42 |    29.99 | 164.36 (1) | 118 | bias, collisions, distr, BIC, LongNeighbors |
| [lookup3](doc/lookup3.txt)                    |      1658.31 |    48.84 | 194.15 (2) | 341 | UB, 28% bias, collisions, 30% distr, BIC  |
| [superfast](doc/superfast.txt)                |      1956.25 |    53.61 | 180.10 (3) | 210 | UB, bad seed 0, 91% bias, 5273.01x collisions, 37% distr, BIC |
| [MurmurOAAT](doc/MurmurOAAT.txt)              |       452.49 |   113.07 | 197.83 (2) |  47 | bad seed 0, collisions, 99.998% distr., BIC, LongNeighbors |
| [Crap8](doc/Crap8.txt)                        |      3149.63 |    36.23 | 195.11 (1) | 342 | UB, 2.42% bias, collisions, 2% distrib |
| [Murmur1](doc/Murmur1.txt)                    |      1804.67 |    51.51 | 188.41 (1) | 358 | UB, 1 bad seed, 511x collisions, Diff, BIC |
| [Murmur2](doc/Murmur2.txt)                    |      3146.91 |    41.87 | 187.89 (2) | 358 | UB, 1 bad seed, 1.7% bias, 81x coll, 1.7% distrib, BIC |
| [Murmur2A](doc/Murmur2A.txt)                  |      3146.79 |    46.87 | 191.96 (4) | 407 | UB, 1 bad seed, 12.7% bias, LongNeighbors          |
| [Murmur2B](doc/Murmur2B.txt)                  |      4882.95 |    39.72 | 149.43 (2) | 433 | UB, 1.8% bias, collisions, 3.4% distrib, BIC |
| [Murmur2C](doc/Murmur2C.txt)                  |      4092.99 |    51.84 | 164.65 (2) | 444 | UB, 2^32 bad seeds, 91% bias, collisions, distr, BIC, LongNeighbors |
| [Murmur3A](doc/Murmur3A.txt)                  |      2413.88 |    53.36 | 182.37 (3) | 351 | UB, 1 bad seed, Moment Chi2 69     |
| [PMurHash32](doc/PMurHash32.txt)              |      2344.78 |    58.48 | 196.43 (4) |1862 | 1 bad seed, Moment Chi2 69         |
| [Murmur3C](doc/Murmur3C.txt)                  |      3197.63 |    67.90 | 198.00 (2) | 859 | UB, LongNeighbors, Text, DiffDist  |
| [mirhash32low](doc/mirhash32low.txt)          |      5412.76 |    39.79 | 182.13 (3) |1112 | UB, 4 bad seeds, Cyclic, LongNeighbors, machine-specific (32/64 differs) |
| [PMPML_32](doc/PMPML_32.txt)                  |      6704.53 |    53.50 | 197.43 (4) |1084 | Avalanche >512, unseeded: Seed, BIC, MomentChi2, PerlinNoise |
| [PMPML_64](doc/PMPML_64.txt)                  |      8161.19 |    53.20 | 179.16 (2) |1305 | unseeded: Seed, MomentChi2, BIC        |
| [xxHash32](doc/xxHash32.txt)                  |      6040.87 |    51.77 | 177.91 (4) | 738 | LongNeighbors, collisions with 4bit diff, MomentChi2 220 |
| [metrohash64](doc/metrohash64.txt)            |      9664.61 |    44.59 | 150.74 (2) | 624 | UB, LongNeighbors, BIC         |
| [metrohash64_1](doc/metrohash64_1.txt)        |      9664.57 |    45.37 | 152.31 (2) | 624 | UB, LongNeighbors, BIC, MomentChi2         |
| [metrohash64crc_1](doc/metrohash64crc_1.txt)  |     14000.50 |    49.08 | 150.54 (2) | 632 | UB, Cyclic 8/8 byte, DiffDist, BIC, MomentChi2, machine-specific (SSE4.2/NEON) |
| [metrohash64crc_2](doc/metrohash64crc_2.txt)  |     14034.84 |    48.94 | 162.54 (2) | 632 | UB, Cyclic 8/8 byte, DiffDist, BIC, machine-specific (SSE4.2/NEON) |
| [cmetrohash64_1o](doc/cmetrohash64_1o.txt)    |      9658.31 |    42.84 | 163.45 (1) |3506 | UB, LongNeighbors, BIC, MomentChi2  |
| [cmetrohash64_1](doc/cmetrohash64_1.txt)      |      9683.33 |    45.20 | 161.01 (2) | 652 | UB, LongNeighbors, BIC, MomentChi2 |
| [City64noSeed](doc/City64noSeed.txt)          |      9090.42 |    32.23 | 171.53 (3) |1038 | Avalanche, Sparse, TwoBytes, MomentChi2, Seed |
| [City64](doc/City64.txt)                      |      9066.90 |    47.81 | 197.78 (1) |1120 | Sparse, TwoBytes           |
| [t1ha1_64le](doc/t1ha1_64le.txt)              |      9723.86 |    34.39 | 176.91 (1) | 517 | Avalanche                  |
| [t1ha1_64be](doc/t1ha1_64be.txt)              |      7481.37 |    38.16 | 193.22 (2) | 555 | Avalanche                  |
| [t1ha0_32le](doc/t1ha0_32le.txt)              |      5132.18 |    54.83 | 193.53 (2) | 509 | Sparse, LongNeighbors      |
| [t1ha0_32be](doc/t1ha0_32be.txt)              |      4585.59 |    55.98 | 183.45 (2) | 533 | Sparse, LongNeighbors      |
| [t1ha2_stream](doc/t1ha2_stream.txt)          |      9068.55 |    74.56 | 219.85 (6) |1665 | Sparse, Permutation, LongNeighbors |
| [t1ha2_stream128](doc/t1ha2_stream128.txt)    |      9065.50 |    93.19 | 236.50 (3) |1665 | Sparse, Permutation, LongNeighbors |
| [aesnihash](doc/aesnihash.txt)                |      2963.39 |    71.24 | 217.73 (3) |1209 | fails many tests, machine-specific (x64 AES-NI) |
| [falkhash](doc/falkhash.txt)                  |     20202.42 |   173.63 | 321.52 (2) | 264 | Sparse, LongNeighbors, machine-specific (x64 AES-NI) |
| [MeowHash](doc/MeowHash.txt)                  |     17371.91 |    85.48 | 247.96 (2) |1764 | Sparse, invertible, machine-specific (x64 AES-NI)  |
| [MeowHash64low](doc/MeowHash64low.txt)        |     17378.06 |    85.48 | 237.60 (2) |1764 | Sparse, invertible, machine-specific (x64 AES-NI)    |
| [MeowHash32low](doc/MeowHash32low.txt)        |     17374.64 |    85.48 | 258.53 (2) |1764 | Sparse, invertible, machine-specific (x64 AES-NI)    |
| --------------------------------------        |              |          |            |     |                            |
| [tifuhash_64](doc/tifuhash_64.txt)            |        35.60 |  1679.52 |1212.75 (15)| 276 | Cyclic low32               |
| [floppsyhash](doc/floppsyhash.txt)            |        35.72 |  1868.92 |1411.07 (7) | 623 |                            |
| [chaskey](doc/chaskey.txt)                    |       753.23 |   153.42 | 288.26 (2) |1609 | PerlinNoise                |
| [SipHash](doc/SipHash.txt)                    |       980.88 |   127.77 | 246.19 (4) |1071 |                            |
| [HalfSipHash](doc/HalfSipHash.txt)            |       755.78 |   114.47 | 243.72 (4) | 700 | zeroes                     |
| [GoodOAAT](doc/GoodOAAT.txt)                  |      1052.00 |    71.62 | 192.19 (1) | 237 |                            |
| [pearsonbhash64](doc/pearsonbhash64.txt)      |      1486.34 |   104.32 | 185.03 (2) | 683 |                            |
| [pearsonbhash128](doc/pearsonbhash128.txt)    |      1347.03 |   121.75 | 214.84 (2) |1134 |                            |
| [pearsonbhash256](doc/pearsonbhash256.txt)    |       998.90 |   167.05 | 261.29 (3) | 844 |                            |
| [prvhash64_64m](doc/prvhash64_64m.txt)        |      2386.19 |    51.18 | 186.87 (1) | 349 |                            |
| [prvhash64_64](doc/prvhash64_64.txt)          |      2375.72 |    51.61 | 190.97 (2) | 384 |                            |
| [prvhash64_128](doc/prvhash64_128.txt)        |      2383.57 |   103.44 | 246.45 (1) | 718  |                           |
| [prvhash64s_64](doc/prvhash64s_64.txt)        |      5481.48 |   170.05 | 325.39 (6) |2640 |                            |
| [prvhash64s_128](doc/prvhash64s_128.txt)      |      5161.33 |   260.96 | 442.70 (22)|2799 |                            |
| [SipHash13](doc/SipHash13.txt)                |      1889.10 |    89.00 | 199.95 (4) | 778 | 0.9% bias                  |
| [TSip](doc/TSip.txt)                          |      3228.14 |    57.96 | 211.71 (3) | 519 | !msvc                      |
| [seahash](doc/seahash.txt)                    |      4796.97 |    58.55 | 201.58 (2) | 871 | PerlinNoise, !msvc         |
| [seahash32low](doc/seahash32low.txt)          |      4801.33 |    58.54 | 227.31 (4) | 871 | PerlinNoise 32, !msvc      |
| [clhash](doc/clhash.txt)                      |      4472.31 |    82.72 | 229.73 (3) |1809 | PerlinNoise, machine-specific (x64 SSE4.2) |
| [HighwayHash64](doc/HighwayHash64.txt)        |      6242.58 |    99.55 | 248.41 (3) |2546 |                            |
| [Murmur3F](doc/Murmur3F.txt)                  |      5226.40 |    52.18 | 175.85 (1) | 699 | UB                         |
| [fasthash32](doc/fasthash32.txt)              |      4737.61 |    45.32 | 181.86 (2) | 566 | UB, insecure                         |
| [fasthash64](doc/fasthash64.txt)              |      4737.21 |    42.79 | 164.87 (2) | 509 | UB, insecure, Moment Chi2 5159 !     |
| [MUM](doc/MUM.txt)                            |      7134.56 |    37.85 | 172.34 (1) |1912 | UB, too many bad seeds, machine-specific (32/64 differs) |
| [MUMlow](doc/MUMlow.txt)                      |      7225.18 |    37.85 | 197.92 (3) |1912 | UB, 5 bad seeds                      |
| [mirhash](doc/mirhash.txt)                    |      5413.73 |    39.68 | 154.47 (3) |1112 | UB, 2^36 bad seeds, LongNeighbors, machine-specific (32/64 differs) |
| [mirhashstrict](doc/mirhashstrict.txt)        |      2217.32 |    65.53 | 182.07 (2) |1112 |                            |
| [mirhashstrict32low](doc/mirhashstrict32low.txt)|    2218.87 |    65.48 | 190.59 (4) |1112 | 1 bad seed, MomentChi2 9   |
| [mx3](doc/mx3.txt)                            |      6146.02 |    52.48 | 173.09 (3) | 734 | UB                         |
| [pengyhash](doc/pengyhash.txt)                |      8744.48 |    85.31 | 222.45 (4) | 421 |                            |
| [City32](doc/City32.txt)                      |      3675.04 |    57.73 | 212.04 (3) |1319 |                            |
| [City64low](doc/City64low.txt)                |      9089.45 |    47.75 | 201.73 (1) |1120|                             |
| [City128](doc/City128.txt)                    |      9640.19 |    88.45 | 225.38 (3) |1841 |                            |
| [CityCrc128](doc/CityCrc128.txt)              |     12343.43 |    74.50 | 209.75 (2) | 295 |                            |
| [FarmHash32](doc/FarmHash32.txt)              |     17112.05 |    47.70 | 214.71 (2) |11489| machine-specific (x64 SSE4/AVX) |
| [FarmHash64](doc/FarmHash64.txt)              |      8684.76 |    48.13 | 200.51 (4) |3758 |                            |
| [FarmHash128](doc/FarmHash128.txt)            |      9409.63 |    74.52 | 210.25 (3) | 163 |                            |
| [farmhash32_c](doc/farmhash32_c.txt)          |     16299.81 |    47.79 | 219.19 (4) | 762 | machine-specific (x64 SSE4/AVX) |
| [farmhash64_c](doc/farmhash64_c.txt)          |      8713.16 |    47.96 | 201.00 (2) |3688 |                            |
| [farmhash128_c](doc/farmhash128_c.txt)        |      9244.13 |    79.08 | 209.44 (2) |1890 |                            |
| [metrohash64_2](doc/metrohash64_2.txt)        |      9668.51 |    44.45 | 164.30 (2) | 627 | UB, LongNeighbors          |
| [cmetrohash64_2](doc/cmetrohash64_2.txt)      |      9669.95 |    44.75 | 149.67 (2) | 655 | LongNeighbors              |
| [metrohash128](doc/metrohash128.txt)          |      9569.16 |    58.68 | 167.53 (2) | 773 | UB, LongNeighbors          |
| [metrohash128_1](doc/metrohash128_1.txt)      |      9558.17 |    59.04 | 175.94 (2) | 773 | UB, LongNeighbors          |
| [metrohash128_2](doc/metrohash128_2.txt)      |      9584.94 |    59.10 | 167.43 (2) | 773 | UB, LongNeighbors          |
| [metrohash128crc_1](doc/metrohash128crc_1.txt)|     13948.67 |    65.20 | 168.08 (2) | 723 | UB, machine-specific (SSE4.2/NEON) |
| [metrohash128crc_2](doc/metrohash128crc_2.txt)|     13920.19 |    65.12 | 176.70 (1) | 723 | UB, machine-specific (SSE4.2/NEON) |
| [xxHash64](doc/xxHash64.txt)                  |      8936.63 |    51.31 | 174.34 (3) |1999 |                            |
| [Spooky32](doc/Spooky32.txt)                  |      9747.13 |    62.24 | 196.96 (4) |2221 | UB                         |
| [Spooky64](doc/Spooky64.txt)                  |      9747.47 |    62.20 | 191.71 (2) |2221 | UB                         |
| [Spooky128](doc/Spooky128.txt)                |      9751.14 |    63.84 | 192.47 (2) |2221 | UB                         |
| [ahash64](doc/ahash64.txt)                    |      9862.62 |    27.32 | 181.68 (1) | 412 | rust                       |
| [xxh3](doc/xxh3.txt)                          |     16538.52 |    32.81 | 184.86 (2) | 744 | DiffDist bit 7 w. 36 bits, BIC |
| [xxh3low](doc/xxh3low.txt)                    |     16462.36 |    32.77 | 199.79 (2) | 756 |                            |
| [xxh128](doc/xxh128.txt)                      |     15174.14 |    40.46 | 195.65 (2) |1012 |                            |
| [xxh128low](doc/xxh128low.txt)                |     15174.85 |    33.79 | 187.05 (2) |1012 |                            |
| [t1ha2_atonce](doc/t1ha2_atonce.txt)          |      9237.12 |    38.94 | 194.32 (2) | 541 | Zeroes low3                |
| [t1ha2_atonce128](doc/t1ha2_atonce128.txt)    |      8350.99 |    55.65 | 203.53 (4) | 613 | LongNeighbors              |
| [t1ha0_aes_noavx](doc/t1ha0_aes_noavx.txt)    |     22785.26 |    38.71 | 180.61 (3) | 925 | LongNeighbors, machine-specific (x86 AES-NI) |
| [t1ha0_aes_avx1](doc/t1ha0_aes_avx1)          |     22714.85 |    48.12 | 226.52 (16)| 843 | LongNeighbors, machine-specific (x64 AVX.txt)|
| [t1ha0_aes_avx2](doc/t1ha0_aes_avx2.txt)      |     22345.33 |    44.38 | 556.47 (89)| 792 | LongNeighbors, machine-specific (x64 AVX2)   |
| [wyhash32](doc/wyhash32.txt)                  |      2532.89 |    48.40 | 484.57 (1) | 426 | 2 bad seeds, 32-bit           |
| [wyhash32low](doc/wyhash32low.txt)            |     12911.09 |    29.59 | 205.43 (2) | 474 | 2 bad seeds                  |
| [wyhash](doc/wyhash.txt)                      |     12879.00 |    30.35 | 196.77 (2) | 474 | 2^33 bad seeds               |
| [umash32](doc/umash32.txt)                    |      4633.19 |    53.42 | 216.33 (3) |1530 |                            |
| [umash32_hi](doc/umash32_hi.txt)              |      4662.92 |    54.22 | 214.20 (2) |1530 |                            |
| [umash64](doc/umash64.txt)                    |      4662.09 |    53.42 | 188.09 (1) |1530 |                            |
| [umash128](doc/umash128.txt)                  |      2427.46 |    70.60 | 197.29 (2) |1530 |                            |
| [halftime_hash64](doc/halftime_hash64.txt)    |      4990.72 |   120.55 | 281.64 (3) |2911 |                            |
| [halftime_hash128](doc/halftime_hash128.txt)  |     13478.23 |    97.79 | 252.14 (2) |2462 |                            |
| [halftime_hash256](doc/halftime_hash256.txt)  |     11620.28 |    98.44 | 252.60 (2) |2622 |                            |
| [halftime_hash512](doc/halftime_hash512.txt)  |      7681.62 |   125.81 | 274.01 (3) |3550 |                            |
| [nmhash32](doc/nmhash32.txt)                  |      7003.30 |    68.93 | 216.59 (2) |2445 |                            |
| [nmhash32x](doc/nmhash32x.txt)                |      6342.95 |    56.41 | 217.75 (2) |1494 |                            |

The sortable table variants:

* **[Default Intel i5-2300 2.8GHz](https://rurban.github.io/smhasher/doc/table.html)** 
* [Intel i5-2300 2.8GHz 32bit](https://rurban.github.io/smhasher/doc/i686.html)
* [AMD Ryzen 3 3200U 3.5GHz](https://rurban.github.io/smhasher/doc/ryzen3.html)
* [Mac Air i7-4650](https://rurban.github.io/smhasher/doc/air.html)
* [Cortex-A53 2GHz (Sony XPeria L4)](https://rurban.github.io/smhasher/doc/phone.html)

Summary
-------

I added some SSE assisted hashes and fast intel/arm CRC32-C, AES and SHA HW variants.
See also the old [https://github.com/aappleby/smhasher/wiki](https://github.com/aappleby/smhasher/wiki) and the improved, but unmaintained fork [https://github.com/demerphq/smhasher](https://github.com/demerphq/smhasher).

So the fastest hash functions on x86_64 without quality problems are:

- xxh3low
- wyhash
- ahash64
- t1ha2_atonce
- FarmHash (_not portable, too machine specific: 64 vs 32bit, old gcc, ..._)
- halftime_hash128
- Spooky32
- pengyhash
- nmhash32
- mx3
- MUM/mir (_different results on 32/64-bit archs, lots of bad seeds to filter out_)
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
[Hash Function Lounge](http://web.archive.org/web/20040205045909/planeta.terra.com.br/informatica/paulobarreto/hflounge.html) are not included here. We list some known attacks at
[GH #186](https://github.com/rurban/smhasher/issues/186).

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
scheme, such as Robin Hood or Cuckoo hashing with collision counting.

One more note regarding security: Nowadays even SHA1 can be solved in
a solver, like Z3 (or faster ones) for practical hash table collision
attacks (i.e. 14-20 bits). All hash functions with less than 160 bits
tested here cannot be considered "secure" at all.

The '\0' vulnerability attack with binary keys is tested in the 2nd
Sanity Zero test.

CRYPTO
------

Our crypto hashes are hardened with an added size_t seed, mixed into
the initial state, and the versions which require zero-padding are
hardened by adding the len also, to prevent from collisions with
AppendedZeroes for the padding. The libtomcrypt implementations
already provide for that, but others might not. Without, such crypto
hash functions are unsuitable for normal tasks, as it's trivial to
create collisions by padding or bad seeds.

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

* Bad Seeds
  
  Hash functions are typically initialized with a random seed. But
  some seed values may lead to bad hash functions, regardless of the
  key. In the regular case with random seeds the probablity of such
  bad seeds is very low, like 2^32 or 2^64.
  A practical application needs to know if bad seeds exist and choose another
  one. See e.g. `wyhash_seed_init()` and `wyhash_bad_seeds()` in `Hashes.h`.
  Note that a bad seed is not really a problem when you skip this seed during initialization.
  It can still be a GOOD or recommended hash function.
  But a bad seed of `0` leading to collisions is considered a bug, a bad hash function.
  
  We test for internal secrets, if they will be multiplied with 0. This
  is also called "blinding multiplication". `main.cpp` lists some
  secrets for each hash function we test against. The function
  `<hash>_bad_seeds()` lists the confirmed bad seeds.
  
  Special care needs to be taken for crc, most FNV1 variants, fletcher,
  Jenkins. And with GOOD hashes all MUM variants, like mirhash, MUM,
  wyhash.

  Independently from this, when the attacker knows the seed it will lead
  to DDOS attacks. Even with crypto hashes in power2 hashtables.

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
  FNV1a\_ variants (FNV1a\_YT, FNV1A\_Pippip\_Yurii, FNV1A\_Totenschiff,
  ...), fibonacci.

  The usual mitigation is to check the buffer alignment either in the
  caller, provide a pre-processing loop for the misaligned prefix, or
  copy the whole buffer into a fresh aligned area.
  Put that extra code inside `#ifdef HAVE_ALIGNED_ACCESS_REQUIRED`.

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

SUPPORT
-------
You can get professional support here: https://xscode.com/rurban/smhasher
