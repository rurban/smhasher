SMhasher
========

[![Linux Build status](https://travis-ci.org/rurban/smhasher.svg?branch=master)](https://travis-ci.org/rurban/smhasher/) [![Windows Build status](https://ci.appveyor.com/api/projects/status/tb6ckfcrhqi6pwn9/branch/master?svg=true)](https://ci.appveyor.com/project/rurban/smhasher) [![FreeBSD Build status](https://api.cirrus-ci.com/github/rurban/smhasher.svg?branch=master)](https://cirrus-ci.com/github/rurban/smhasher)

| Hash function                                 |      MiB/sec |cycl./hash|cycl./map   | size| Quality problems               |
|:----------------------------------------------|-------------:|---------:|-----------:|----:|--------------------------------|
| [donothing32](doc/donothing32.txt)            |  11149460.06 |     4.00 | -|  13 | bad seed 0, test NOP           |
| [donothing64](doc/donothing64.txt)            |  11787676.42 |     4.00 | -|  13 | bad seed 0, test NOP           |
| [donothing128](doc/donothing128.txt)          |  11745060.76 |     4.06 | -|  13 | bad seed 0, test NOP           |
| [NOP_OAAT_read64](doc/NOP_OAAT_read64.txt)    |  11372846.37 |    14.00 | -|  47 | test NOP                       |
| [BadHash](doc/BadHash.txt)                    |       769.94 |    73.97 | -|  47 | bad seed 0, test FAIL          |
| [sumhash](doc/sumhash.txt)                    |     10699.57 |    29.53 | -| 363 | bad seed 0, test FAIL          |
| [sumhash32](doc/sumhash32.txt)                |     42877.79 |    23.12 | -| 863 | UB, test FAIL                  |
| [multiply_shift](doc/multiply_shift.txt)      |      8026.77 |    26.05 | 226.80 (8) | 345 | bad seeds & 0xfffffff0, fails most tests |
| [pair_multiply_shift](doc/pair_multiply_shift)|      3716.95 |    40.22 | 186.34 (3) | 609 | fails most tests |
| --------------------------                    |              |          |            |     |                      |
| [crc32](doc/crc32.txt)                        |       383.12 |   134.21 | 257.50 (11)| 422 | insecure, 8590x collisions, distrib, PerlinNoise |
| [md5_32](doc/md5_32.txt)                      |       350.53 |   644.31 | 894.12 (10)|4419 |  |
| [md5_64](doc/md5_64.txt)                      |       351.01 |   656.67 | 897.43 (12)|4419 |  |
| [md5-128](doc/md5-128.txt)                    |       350.89 |   681.88 | 894.03 (13)|4419 |                    |
| [sha1_32](doc/sha1_32.txt)                  |       353.03 |  1385.80 |1759.94 (5) |5126 | Sanity, Cyclic low32, 36.6% distrib |
| [sha1_64](doc/sha1_64.txt)                  |       353.03 |  1385.80 |1759.94 (5) |5126 | Sanity, Cyclic low32, 36.6% distrib |
| [sha1-160](doc/sha1-160.txt)                  |       364.95 |  1470.55 |1794.16 (13)|5126 | Comb/Cyclic low32  |
| [sha2-224](doc/sha2-224.txt)                  |       147.13 |  1354.81 |1589.92 (12)|     | Comb low32 |
| [sha2-224_64](doc/sha2-224_64.txt)            |       147.60 |  1360.10 |1620.93 (13)|     | Cyclic low32  |
| [sha2-256](doc/sha2-256.txt)                  |       147.80 |  1374.90 |1606.06 (16)|     |  |
| [sha2-256_64](doc/sha2-256_64.txt)            |       148.01 |  1376.34 |1624.71 (16)|     |  |
| [sha1ni](doc/sha1ni.txt)                      |      1601.21 |   174.16 | 397.28 (6) | 989 | insecure,sanity, Permutation, Zeroes, machine-specific |
| [sha1ni_32](doc/sha1ni_32.txt)                |      1576.17 |   174.04 | 405.56 (6) | 989 | machine-specific |
| [sha2ni-256](doc/sha2ni-256.txt)              |      1527.38 |   184.35 | 404.40 (4) |4241 | insecure,sanity, Permutation, Zeroes, machine-specific |
| [sha2ni-256_64](doc/sha2ni-256_64.txt)        |      1501.85 |   186.20 | 407.96 (5) |4241 | Zeroes, machine-specific |
| [blake3_c](doc/blake3_c.txt)                  |      1288.84 |   357.69 | 582.89 (6) |     | no 32bit portability     |
| [rmd128](doc/rmd128.txt)                      |       290.90 |   710.49 | 965.55 (6) |     |               |
| [rmd160](doc/rmd160.txt)                      |       202.16 |  1045.79 |1287.74 (16)|     | Cyclic hi32   |
| [rmd256](doc/rmd256.txt)                      |       364.81 |   584.86 | 835.02 (11)|     |               |
| [edonr224](doc/edonr224.txt)                  |       864.69 |   303.42 | 526.94 (6) |     |               |
| [edonr256](doc/edonr256.txt)                  |       847.85 |   305.79 | 510.01 (4) |     |               |
| [blake2s-128](doc/blake2s-128.txt)            |       295.30 |   698.09 |1059.24 (51)|     |               |
| [blake2s-160](doc/blake2s-160.txt)            |       215.01 |  1026.74 |1239.54 (11)|     |               |
| [blake2s-224](doc/blake2s-224.txt)            |       207.06 |  1063.86 |1236.50 (20)|     |               |
| [blake2s-256](doc/blake2s-256.txt)            |       215.28 |  1014.88 |1230.38 (28)|     |               |
| [blake2s-256_64](doc/blake2s-256_64.txt)      |       211.52 |  1044.22 |1228.43 (8) |     |               |
| [blake2b-160](doc/blake2b-160.txt)            |       356.08 |  1236.84 |1458.15 (12)|     |               |
| [blake2b-224](doc/blake2b-224.txt)            |       356.59 |  1228.50 |1425.87 (16)|     |               |
| [blake2b-256](doc/blake2b-256.txt)            |       355.97 |  1232.22 |1443.31 (19)|     |               |
| [blake2b-256_64](doc/blake2b-256_64.txt)      |       356.97 |  1222.76 |1435.03 (9) |     |               |
| [asconhashv12](doc/asconhashv12.txt)          |       144.98 |   885.02 |1324.23 (38)|4341 |               |
| [asconhashv12_64](doc/asconhashv12_64.txt)    |       159.68 |   386.90 | 480.86 (4) |6490 |               |
| [sha3-256](doc/sha3-256.txt)                  |       100.58 |  3877.18 |4159.79 (37)|     | PerlinNoise   |
| [sha3-256_64](doc/sha3-256_64.txt)            |       100.57 |  3909.00 |4174.63 (16)|     | PerlinNoise   |
| [hasshe2](doc/hasshe2.txt)                    |      2773.89 |    64.35 | 282.30 (3) | 445 | Permutation,TwoBytes,Zeroes,Seed|
| [poly_1_mersenne](doc/poly_1_mersenne.txt)    |      1369.21 |    61.59 | 248.86 (4) | 479 | fails most tests |
| [poly_2_mersenne](doc/poly_2_mersenne.txt)    |      1364.03 |    70.30 | 261.00 (6) | 479 |                  |
| [poly_3_mersenne](doc/poly_3_mersenne.txt)    |      1342.82 |    80.22 | 268.79 (2) | 479 |                  |
| [poly_4_mersenne](doc/poly_4_mersenne.txt)    |      1343.19 |    89.13 | 277.52 (3) | 479 |                  |
| [tabulation32](doc/tabulation32.txt)          |      5781.16 |    40.00 | 241.79 (10) | 848 | collisions       |
| [tabulation](doc/tabulation.txt)              |      7875.01 |    39.95 | 249.49 (3) | 554 |                  |
| [crc32_hw](doc/crc32_hw.txt)                  |      6244.38 |    41.23 | 226.80 (2) | 653 | insecure, 100% bias, collisions, distrib, BIC, machine-specific (SSE4.2/NEON) |
| [crc32_hw1](doc/crc32_hw1.txt)                |      7569.29 |    49.07 | 233.75 (3) | 671 | insecure, 100% bias, collisions, distrib, BIC, machine-specific (x86 SSE4.2)  |
| [crc64_hw](doc/crc64_hw.txt)                  |      6143.62 |    40.48 | 223.13 (2) | 652 | insecure, 100% bias, collisions, distrib, BIC, machine-specific (SSE4.2/NEON) |
| [crc32_pclmul](doc/crc32_pclmul.txt)          |           -  |     -    |     -      |     | insecure, 100% bias, collisions, distrib, BIC, machine-specific (x86 SSE4.2+PCLMUL) |
| [o1hash](doc/o1hash.txt)                      |  11629440.57 |    18.15 | 199.35 (2) | 101 | insecure, no seed, zeros, fails all tests |
| [fibonacci](doc/fibonacci.txt)                |     16878.32 |    22.94 | 803.18 (15)|1692 | UB, zeros, fails all tests       |
| [FNV1a](doc/FNV1a.txt)                        |       760.52 |    73.83 | 254.29 (5) | 204 | bad seed, zeros, fails all tests       |
| [FNV1A_Totenschiff](doc/FNV1A_Totenschiff.txt)|      6274.78 |    26.23 | 251.13 (2) | 270 | UB, zeros, fails all tests   |
| [FNV1A_Pippip_Yurii](doc/FNV1A_Pippip_Yurii.txt)|      6172.14 |    27.55 | 244.80 (2) | 147 | UB, sanity, fails all tests  |
| [FNV1a_YT](doc/FNV1a_YT.txt)                  |     13486.49 |    30.50 | 237.43 (4) | 321 | bad seed, UB, fails all tests          |
| [FNV2](doc/FNV2.txt)                          |      6171.60 |    32.20 | 208.59 (4) | 278 | fails all tests              |
| [FNV64](doc/FNV64.txt)                        |       774.37 |    72.43 | 201.15 (2) |  79 | fails all tests              |
| [FNV128](doc/FNV128.txt)                      |       390.14 |   136.42 | 289.00 (3) | 171 | fails all tests              |
| [k-hash32](doc/k-hash32.txt)                  |      2230.42 |    53.05 | 264.64 (3) | 808 | insecure, zeros, UB, bad seeds, fails all tests       |
| [k-hash64](doc/k-hash64.txt)                  |      2451.88 |    48.66 | 249.44 (2) | 609 | insecure, zeros, UB, bad seeds, fails all tests       |
| [fletcher2](doc/fletcher2.txt)                |     15552.61 |    20.61 | 335.31 (3) | 248 | bad seed 0, UB, fails all tests          |
| [fletcher4](doc/fletcher4.txt)                |     15556.93 |    20.60 | 358.60 (3) | 371 | bad seed 0, UB, fails all tests          |
| [bernstein](doc/bernstein.txt)                |      1045.97 |    58.31 | 225.78 (3) |  41 | bad seed 0, fails all tests              |
| [sdbm](doc/sdbm.txt)                          |       784.83 |    68.57 | 222.68 (5) |  41 | bad seed 0, fails all tests              |
| [x17](doc/x17.txt)                            |       748.75 |    74.13 | 236.00 (10) |  79 | 99.98% bias, fails all tests |
| [libiberty](doc/libiberty.txt)                |       628.66 |    84.95 | 225.07 (4) |  37 | insecure, 100% bias, fails all tests, bad seed |
| [gcc](doc/gcc.txt)                            |       611.69 |    86.47 | 231.51 (5) |  39 | insecure, 100% bias, fails all tests, bad seed |
| [JenkinsOOAT](doc/JenkinsOOAT.txt)            |       627.64 |   107.04 | 252.79 (3) | 153 | bad seed 0, 53.5% bias, fails all tests  |
| [JenkinsOOAT_perl](doc/JenkinsOOAT_perl.txt)  |       608.10 |    94.17 | 254.09 (4) |  65 | bad seed 0, 1.5-11.5% bias, 7.2x collisions, BIC, LongNeighbors |
| [MicroOAAT](doc/MicroOAAT.txt)                |       701.35 |    76.68 | 251.01 (3) |  68 | 100% bias, distrib, BIC      |
| [pearsonhash64](doc/pearsonhash64.txt)        |       434.17 |   124.14 | 230.79 (4) |     | Avalanche, Seed, SSSE3 only. broken MSVC     |
| [pearsonhash128](doc/pearsonhash128.txt)      |       434.23 |   121.34 | 221.03 (7) |     | Avalanche, Seed, SSSE3 only. broken MSVC     |
| [pearsonhash256](doc/pearsonhash256.txt)      |       444.08 |   119.11 | 229.75 (4) |     | Avalanche, Seed, SSSE3 only. broken MSVC     |
| [VHASH_32](doc/VHASH_32.txt)                  |     13053.40 |    65.84 | 289.86 (3) |1231 | sanity, Seed, MomentChi2     |
| [VHASH_64](doc/VHASH_64.txt)                  |     13465.50 |    63.88 | 286.38 (5) |1231 | sanity, Seed, Sparse         |
| [farsh32](doc/farsh32.txt)                    |     27038.23 |    66.88 | 278.89 (5) | 944 | insecure: AppendedZeroes, collisions+bias, MomentChi2, LongNeighbors |
| [farsh64](doc/farsh64.txt)                    |     13829.32 |   112.46 | 332.59 (3) | 944 | insecure: AppendedZeroes, collisions+bias, MomentChi2, LongNeighbors |
| [farsh128](doc/farsh128.txt)                  |      6878.88 |   233.35 | 384.85 (3) | 944 | insecure: AppendedZeroes, collisions+bias, permut,combin,2bytes,zeroes,PerlinNoise |
| [farsh256](doc/farsh256.txt)                  |      3467.37 |   440.40 | 593.57 (5) | 944 | insecure: AppendedZeroes, collisions+bias, permut,combin,2bytes,zeroes,PerlinNoise |
| [jodyhash32](doc/jodyhash32.txt)              |      1794.34 |    41.12 | 235.12 (4) | 102 | bias, collisions, distr, BIC LongNeighbors |
| [jodyhash64](doc/jodyhash64.txt)              |      4813.10 |    40.72 | 239.22 (6) | 118 | bias, collisions, distr, BIC, LongNeighbors |
| [lookup3](doc/lookup3.txt)                    |      2475.35 |    39.65 | 240.10 (3) | 341 | UB, 28% bias, collisions, 30% distr, BIC  |
| [superfast](doc/superfast.txt)                |      2058.22 |    49.56 | 254.12 (3) | 210 | UB, bad seed 0, 91% bias, 5273.01x collisions, 37% distr, BIC |
| [MurmurOAAT](doc/MurmurOAAT.txt)              |       506.66 |   103.33 | 236.89 (3) |  47 | bad seed 0, collisions, 99.998% distr., BIC, LongNeighbors |
| [Crap8](doc/Crap8.txt)                        |      3041.14 |    37.25 | 247.87 (4) | 342 | UB, 2.42% bias, collisions, 2% distrib |
| [Murmur1](doc/Murmur1.txt)                    |      2027.85 |    48.51 | 253.34 (3) | 358 | UB, 1 bad seed, 511x collisions, Diff, BIC |
| [Murmur2](doc/Murmur2.txt)                    |      3089.18 |    41.22 | 238.42 (4) | 358 | UB, 1 bad seed, 1.7% bias, 81x coll, 1.7% distrib, BIC |
| [Murmur2A](doc/Murmur2A.txt)                  |      3087.98 |    45.90 | 238.54 (4) | 407 | UB, 1 bad seed, 12.7% bias, LongNeighbors          |
| [Murmur2B](doc/Murmur2B.txt)                  |      5919.38 |    38.18 | 215.96 (3) | 433 | UB, 1.8% bias, collisions, 3.4% distrib, BIC |
| [Murmur2C](doc/Murmur2C.txt)                  |      3810.98 |    49.09 | 218.51 (3) | 444 | UB, 2^32 bad seeds, 91% bias, collisions, distr, BIC, LongNeighbors |
| [Murmur3A](doc/Murmur3A.txt)                  |      2982.67 |    49.08 | 245.78 (4) | 351 | UB, 1 bad seed, Moment Chi2 69     |
| [PMurHash32](doc/PMurHash32.txt)              |      3005.85 |    48.88 | 242.38 (3) |1862 | 1 bad seed, Moment Chi2 69         |
| [Murmur3C](doc/Murmur3C.txt)                  |      4833.18 |    56.87 | 250.47 (6) | 859 | UB, LongNeighbors, Text, DiffDist  |
| [mirhash32low](doc/mirhash32low.txt)          |      6145.39 |    36.95 | 235.09 (4) |1112 | UB, 4 bad seeds, Cyclic, LongNeighbors, machine-specific (32/64 differs) |
| [PMPML_32](doc/PMPML_32.txt)                  |      6639.68 |    45.33 | 257.45 (3) |1084 | Avalanche >512, unseeded: Seed, BIC, MomentChi2, PerlinNoise |
| [PMPML_64](doc/PMPML_64.txt)                  |      9833.77 |    50.00 | 251.64 (6) |1305 | unseeded: Seed, MomentChi2, BIC        |
| [xxHash32](doc/xxHash32.txt)                  |      5865.17 |    49.20 | 242.74 (3) | 738 | LongNeighbors, collisions with 4bit diff, MomentChi2 220 |
| [metrohash64](doc/metrohash64.txt)            |     14741.56 |    39.44 | 215.76 (2) | 624 | UB, LongNeighbors, BIC         |
| [metrohash64_1](doc/metrohash64_1.txt)        |     14298.77 |    40.31 | 223.25 (4) | 624 | UB, LongNeighbors, BIC, MomentChi2         |
| [metrohash64crc_1](doc/metrohash64crc_1.txt)  |      6929.69 |    44.65 | 223.68 (3) | 632 | UB, Cyclic 8/8 byte, DiffDist, BIC, MomentChi2, machine-specific (SSE4.2/NEON) |
| [metrohash64crc_2](doc/metrohash64crc_2.txt)  |      8150.65 |    43.72 | 219.45 (5) | 632 | UB, Cyclic 8/8 byte, DiffDist, BIC, machine-specific (SSE4.2/NEON) |
| [cmetrohash64_1o](doc/cmetrohash64_1o.txt)    |     14921.73 |    38.95 | 213.25 (2) |3506 | UB, LongNeighbors, BIC, MomentChi2  |
| [cmetrohash64_1](doc/cmetrohash64_1.txt)      |     14151.73 |    40.90 | 211.89 (2) | 652 | UB, LongNeighbors, BIC, MomentChi2 |
| [City64noSeed](doc/City64noSeed.txt)          |     14209.19 |    31.80 | 225.90 (5) |1038 | Avalanche, Sparse, TwoBytes, MomentChi2, Seed |
| [City64](doc/City64.txt)                      |     13887.84 |    46.32 | 239.77 (3) |1120 | Sparse, TwoBytes           |
| [t1ha1_64le](doc/t1ha1_64le.txt)              |     13442.64 |    31.41 | 219.58 (3) | 517 | Avalanche                  |
| [t1ha1_64be](doc/t1ha1_64be.txt)              |     11586.02 |    32.74 | 232.55 (3) | 555 | Avalanche                  |
| [t1ha0_32le](doc/t1ha0_32le.txt)              |      7401.21 |    48.27 | 238.99 (3) | 509 | Sparse, LongNeighbors      |
| [t1ha0_32be](doc/t1ha0_32be.txt)              |      6217.37 |    50.66 | 244.51 (3) | 533 | Sparse, LongNeighbors      |
| [t1ha2_stream](doc/t1ha2_stream.txt)          |     14011.63 |    80.72 | 275.17 (3) |1665 | Sparse, Permutation, LongNeighbors |
| [t1ha2_stream128](doc/t1ha2_stream128.txt)    |     13136.06 |    97.80 | 306.11 (7) |1665 | Sparse, Permutation, LongNeighbors |
| [aesnihash](doc/aesnihash.txt)                |      5579.32 |    56.83 | 258.71 (5) |1209 | fails many tests, machine-specific (x64 AES-NI) |
| [falkhash](doc/falkhash.txt)                  |     50631.69 |   123.02 | 322.14 (7) | 264 | Sparse, LongNeighbors, machine-specific (x64 AES-NI) |
| [MeowHash](doc/MeowHash.txt)                  |     29969.40 |    64.96 | 274.29 (4) |1764 | Sparse, invertible, machine-specific (x64 AES-NI)  |
| [MeowHash64low](doc/MeowHash64low.txt)        |     29485.59 |    65.98 | 278.05 (3) |1764 | Sparse, invertible, machine-specific (x64 AES-NI)    |
| [MeowHash32low](doc/MeowHash32low.txt)        |     26944.58 |    65.95 | 292.79 (9) |1764 | Sparse, invertible, machine-specific (x64 AES-NI)    |
| --------------------------------------        |              |          |            |     |                            |
| [tifuhash_64](doc/tifuhash_64.txt)            |        35.60 |  1679.52 |1212.75 (15)| 276 | Cyclic low32               |
| [floppsyhash](doc/floppsyhash.txt)            |        35.72 |  1868.92 |1411.07 (7) | 623 |                            |
| [beamsplitter](doc/beamsplitter.txt)          |       789.22 |   682.45 |1150.33 (26)|4203 | UB                         |
| [discohash1](doc/discohash1.txt)              |      4131.12 |   199.00 | 398.35 (5) |1294 |                            |
| [discohash1-128](doc/discohash1-128.txt)      |      4072.95 |   234.17 | 438.43 (5) |1294 |                            |
| [discohash2](doc/discohash2.txt)              |      3986.52 |   207.52 | 421.99 (2) |1294 |                            |
| [discohash2-128](doc/discohash2-128.txt)      |      4094.73 |   236.61 | 433.35 (4) |1294 |                            |
| [discoNONG](doc/discoNONG.txt)                |      3698.45 |   399.67 | 597.78 (9) |    | bad seeds                   |
| [chaskey](doc/chaskey.txt)                    |      1143.05 |   113.70 | 294.43 (4) |1609 | PerlinNoise                |
| [SipHash](doc/SipHash.txt)                    |       943.53 |   147.15 | 338.74 (4) |1071 |                            |
| [HalfSipHash](doc/HalfSipHash.txt)            |      1141.57 |    79.65 | 263.96 (3) | 700 | zeroes                     |
| [GoodOAAT](doc/GoodOAAT.txt)                  |       743.81 |    85.62 | 231.22 (3) | 237 |                            |
| [pearsonbhash64](doc/pearsonbhash64.txt)      |      1794.83 |    97.80 | 268.90 (8) | 683 |                            |
| [pearsonbhash128](doc/pearsonbhash128.txt)    |      1691.62 |   104.57 | 272.38 (4) |1134 |                            |
| [pearsonbhash256](doc/pearsonbhash256.txt)    |      1442.59 |   126.04 | 309.34 (4) | 844 |                            |
| [prvhash64_64m](doc/prvhash64_64m.txt)        |      3077.18 |    47.31 | 241.92 (3) | 349 |                            |
| [prvhash64_64](doc/prvhash64_64.txt)          |      3015.08 |    48.03 | 240.64 (3) | 384 |                            |
| [prvhash64_128](doc/prvhash64_128.txt)        |      3353.81 |    67.64 | 266.32 (2) | 718  |                           |
| [prvhash64s_64](doc/prvhash64s_64.txt)        |      6591.34 |   273.50 | 464.65 (3) |2640 |                           |
| [prvhash64s_128](doc/prvhash64s_128.txt)      |      6581.40 |   333.83 | 528.07 (5) |2799 |                            |
| [SipHash13](doc/SipHash13.txt)                |      1812.75 |   106.56 | 310.76 (5) | 778 | 0.9% bias                  |
| [TSip](doc/TSip.txt)                          |      4233.17 |    53.31 | 249.19 (3) | 519 | !msvc                      |
| [seahash](doc/seahash.txt)                    |      8261.80 |    58.94 | 256.08 (4) | 871 | PerlinNoise, !msvc         |
| [seahash32low](doc/seahash32low.txt)          |      8266.17 |    58.90 | 290.21 (16)| 871 | PerlinNoise 32, !msvc      |
| [clhash](doc/clhash.txt)                      |     18703.04 |    70.19 | 282.12 (6) |1809 | PerlinNoise, machine-specific (x64 SSE4.2) |
| [HighwayHash64](doc/HighwayHash64.txt)        |      6242.58 |    99.55 | 248.41 (3) |2546 |                            |
| [Murmur3F](doc/Murmur3F.txt)                  |      7623.44 |    52.69 | 221.87 (3) | 699 | UB                         |
| [MUM](doc/MUM.txt)                            |      9563.99 |    34.99 | 228.55 (5) |1912 | UB, too many bad seeds, machine-specific (32/64 differs) |
| [MUMlow](doc/MUMlow.txt)                      |      9261.89 |    36.17 | 247.66 (4) |1912 | UB, 5 bad seeds              |
| [xmsx32](doc/xmsx32.txt)                      |      2039.10 |    46.39 | 249.30 (7) | 192 | 2 bad seeds                  |
| [mirhash](doc/mirhash.txt)                    |      6139.07 |    37.02 | 209.47 (3) |1112 | UB, 2^36 bad seeds, LongNeighbors, machine-specific (32/64 differs) |
| [mirhashstrict](doc/mirhashstrict.txt)        |      3549.01 |    49.99 | 224.91 (2) |1112 |                            |
| [mirhashstrict32low](doc/mirhashstrict32low.txt)|    3441.35 |    50.60 | 247.19 (3) |1112 | 1 bad seed, MomentChi2 9   |
| [fasthash32](doc/fasthash32.txt)              |      6128.28 |    40.30 | 241.64 (4) | 566 | UB                         |
| [fasthash64](doc/fasthash64.txt)              |      5818.92 |    38.70 | 220.74 (2) | 509 | UB                         |
| [aesni](doc/aesni.txt)                        |     31232.34 |    29.21 | 230.14 (4) | 519 | machine-specific (x64 AES-NI)           |
| [aesni-low](doc/aesni-low.txt)                |     31221.14 |    29.64 | 226.18 (3) | 519 | machine-specific (x64 AES-NI)           |
| [mx3](doc/mx3.txt)                            |      9034.90 |    48.71 | 227.89 (2) | 734 | UB                         |
| [pengyhash](doc/pengyhash.txt)                |     13428.80 |    74.24 | 275.42 (5) | 421 |                            |
| [City32](doc/City32.txt)                      |      5551.28 |    54.40 | 261.64 (2) |1319 |                            |
| [City64low](doc/City64low.txt)                |     13904.10 |    46.24 | 260.08 (3) |1120|                             |
| [City128](doc/City128.txt)                    |     14031.96 |    89.09 | 290.05 (10)|1841 |                            |
| [CityCrc128](doc/CityCrc128.txt)              |      7916.44 |    55.50 | 240.79 (2) | 295 |                            |
| [FarmHash32](doc/FarmHash32.txt)              |     21755.58 |    47.54 | 258.35 (3) |11489| machine-specific (x64 SSE4/AVX) |
| [FarmHash64](doc/FarmHash64.txt)              |     12845.53 |    47.11 | 251.58 (3) |3758 |                            |
| [FarmHash128](doc/FarmHash128.txt)            |     13913.65 |    70.25 | 263.06 (3) | 163 |                            |
| [farmhash32_c](doc/farmhash32_c.txt)          |     21601.86 |    47.38 | 273.00 (3) | 762 | machine-specific (x64 SSE4/AVX) |
| [farmhash64_c](doc/farmhash64_c.txt)          |     12834.10 |    47.23 | 246.20 (2) |3688 |                            |
| [farmhash128_c](doc/farmhash128_c.txt)        |     13753.24 |    68.96 | 263.76 (3) |1890 |                            |
| [metrohash64_2](doc/metrohash64_2.txt)        |     14316.37 |    40.23 | 218.28 (3) | 627 | UB, LongNeighbors          |
| [cmetrohash64_2](doc/cmetrohash64_2.txt)      |     14294.26 |    40.76 | 221.40 (4) | 655 | LongNeighbors              |
| [metrohash128](doc/metrohash128.txt)          |     15634.66 |    73.28 | 261.23 (4) | 773 | UB, LongNeighbors          |
| [metrohash128_1](doc/metrohash128_1.txt)      |     15806.97 |    72.30 | 260.90 (4) | 773 | UB, LongNeighbors          |
| [metrohash128_2](doc/metrohash128_2.txt)      |     15822.60 |    72.30 | 255.34 (3) | 773 | UB, LongNeighbors          |
| [metrohash128crc_1](doc/metrohash128crc_1.txt)|      8009.23 |    78.72 | 281.55 (13)| 723 | UB, machine-specific (SSE4.2/NEON) |
| [metrohash128crc_2](doc/metrohash128crc_2.txt)|      7878.22 |    79.90 | 275.22 (4) | 723 | UB, machine-specific (SSE4.2/NEON) |
| [xxHash64](doc/xxHash64.txt)                  |     12108.87 |    49.78 | 228.83 (2) |1999 |                            |
| [Spooky32](doc/Spooky32.txt)                  |     13108.95 |    56.27 | 255.36 (3) |2221 | UB                         |
| [Spooky64](doc/Spooky64.txt)                  |     13529.36 |    58.76 | 236.31 (3) |2221 | UB                         |
| [Spooky128](doc/Spooky128.txt)                |     11781.35 |    58.91 | 242.91 (3) |2221 | UB                         |
| [SpookyV2_32](doc/SpookyV2_32.txt)            |     13529.16 |    55.55 | 248.37 (4) |2069 |                            |
| [SpookyV2_64](doc/SpookyV2_64.txt)            |     12678.82 |    56.71 | 243.21 (4) |2069 |                            |
| [SpookyV2_128](doc/SpookyV2_128.txt)          |     13512.82 |    58.33 | 244.56 (5) |2069 |                            |
| [ahash64](doc/ahash64.txt)                    |      9862.62 |    27.32 | 181.68 (1) | 412 | rust                       |
| [xxh3](doc/xxh3.txt)                          |     21033.55 |    29.48 | 226.77 (4) | 744 | DiffDist bit 7 w. 36 bits, BIC |
| [xxh3low](doc/xxh3low.txt)                    |     17093.19 |    30.57 | 242.07 (7) | 756 |                            |
| [xxh128](doc/xxh128.txt)                      |     18802.16 |    32.37 | 234.30 (4) |1012 |                            |
| [xxh128low](doc/xxh128low.txt)                |     18833.05 |    32.30 | 234.68 (3) |1012 |                            |
| [t1ha2_atonce](doc/t1ha2_atonce.txt)          |     13854.44 |    37.92 | 233.54 (2) | 541 | Zeroes low3                |
| [t1ha2_atonce128](doc/t1ha2_atonce128.txt)    |     14148.42 |    55.70 | 253.74 (6) | 613 | LongNeighbors              |
| [t1ha0_aes_noavx](doc/t1ha0_aes_noavx.txt)    |     27231.59 |    37.70 | 236.10 (3) | 925 | LongNeighbors, machine-specific (x86 AES-NI) |
| [t1ha0_aes_avx1](doc/t1ha0_aes_avx1)          |     22714.85 |    48.12 | 226.52 (16)| 843 | LongNeighbors, machine-specific (x64 AVX.txt)|
| [t1ha0_aes_avx2](doc/t1ha0_aes_avx2.txt)      |     56919.46 |    36.70 | 233.14 (2) | 792 | LongNeighbors, machine-specific (x64 AVX2)   |
| [wyhash32](doc/wyhash32.txt)                  |      2532.89 |    48.40 | 484.57 (1) | 426 | 4 bad and broken seeds, 32-bit |
| [wyhash32low](doc/wyhash32low.txt)            |     22393.77 |    29.04 | 243.40 (3) | 474 | 5 bad seeds  |
| [wyhash](doc/wyhash.txt)                      |     22540.23 |    28.87 | 236.16 (8) | 474 |                           |
| [rapidhash](doc/rapidhash.txt)                |     23789.79 |    22.80 | 138.71 (7) | 574 |                           |
| [rapidhash_unrolled](doc/rapidhash_unrolled.txt)|     23892.88 |    23.41 | 139.47 (12)| 782 |                           |
| [umash32](doc/umash32.txt)                    |     21427.57 |    42.12 | 255.55 (5) |1530 |                            |
| [umash32_hi](doc/umash32_hi.txt)              |     21483.12 |    42.65 | 251.09 (4) |1530 |                            |
| [umash64](doc/umash64.txt)                    |     21690.08 |    41.67 | 238.01 (4) |1530 |                            |
| [umash128](doc/umash128.txt)                  |     13211.88 |    43.37 | 237.40 (3) |1530 |                            |
| [halftime_hash64](doc/halftime_hash64.txt)    |      4735.63 |    99.90 | 315.34 (3) |2911 |                            |
| [halftime_hash128](doc/halftime_hash128.txt)  |     17534.53 |    97.97 | 311.10 (4) |2462 |                            |
| [halftime_hash256](doc/halftime_hash256.txt)  |     18003.39 |    99.46 | 315.09 (3) |2622 |                            |
| [halftime_hash512](doc/halftime_hash512.txt)  |     10890.15 |   118.05 | 333.45 (3) |3550 |                            |
| [nmhash32](doc/nmhash32.txt)                  |     12969.62 |    55.88 | 265.69 (4) |2445 |                            |
| [nmhash32x](doc/nmhash32x.txt)                |     12775.08 |    42.66 | 246.05 (3) |1494 |                            |
| [k-hashv32](doc/k-hashv32.txt)                |      9181.87 |    52.76 | 245.14 (3) |1280 |                            |
| [k-hashv64](doc/k-hashv32.txt)                |      7850.92 |    46.94 | 193.94 (1) |1279 |                            |
| [komihash](doc/komihash.txt)                  |     12242.78 |    33.02 | 236.07 (2) |1323 |                            |
| [polymur](doc/polymur.txt)                    |      9676.33 |    42.70 | 246.53 (3) |1128 |                            |

The sortable table variants:

* **[Default AMD Ryzen 5 3350G 3.6GHz](https://rurban.github.io/smhasher/doc/table.html)**
* [Intel i5-2300 2.8GHz](https://rurban.github.io/smhasher/doc/intel.html)
* [Intel i5-2300 2.8GHz 32bit](https://rurban.github.io/smhasher/doc/i686.html)
* [AMD Ryzen 3 3200U 3.5GHz](https://rurban.github.io/smhasher/doc/ryzen3.html)
* [Mac Air i7-4650](https://rurban.github.io/smhasher/doc/air.html)
* [Cortex-A53 2GHz (Sony XPeria L4)](https://rurban.github.io/smhasher/doc/phone.html)

Summary
-------

I added some SSE assisted hashes and fast intel/arm CRC32-C, AES and SHA HW variants.
See also the old [https://github.com/aappleby/smhasher/wiki](https://github.com/aappleby/smhasher/wiki), the improved, but unmaintained fork [https://github.com/demerphq/smhasher](https://github.com/demerphq/smhasher), and the new improved version SMHasher3 [https://gitlab.com/fwojcik/smhasher3](https://gitlab.com/fwojcik/smhasher3).

So the fastest hash functions on x86_64 without quality problems are:

* rapidhash (an improved wyhash)
* xxh3low
* wyhash
* umash (even universal!)
* ahash64
* t1ha2_atonce
* komihash
* FarmHash (_not portable, too machine specific: 64 vs 32bit, old gcc, ..._)
* halftime_hash128
* Spooky32
* pengyhash
* nmhash32
* mx3
* MUM/mir (_different results on 32/64-bit archs, lots of bad seeds to filter out_)
* fasthash32

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

* <https://github.com/martinus/better-faster-stronger-mixer> Mixers. And in his
  blog the best C++ hashmap benchmarks.
* <http://nohatcoder.dk/2019-05-19-1.html> gives a new, useful hash level
  classification 1-5.
* <http://www.strchr.com/hash_functions> lists other benchmarks and quality of
  most simple and fast hash functions.
* <http://bench.cr.yp.to/primitives-hash.html> lists the benchmarks of all
  currently tested secure hashes.
* <http://valerieaurora.org/hash.html> Lifetimes of cryptographic hash functions

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
scheme, such as swisstable/folly-F14, Robin Hood or Cuckoo hashing
with collision counting.

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
June 2017. Available: <http://csrc.nist.gov/groups/STM/cavp>
(No testing source code provided, just high-level descriptions)

Two other independent third party testsuites found an extensive number
of bugs and weaknesses in the SHA3 candidates.
"Finding Bugs in Cryptographic Hash Function Implementations",
Nicky Mouha, Mohammad S Raunak, D. Richard Kuhn, and Raghu Kacker, 2017.
<https://eprint.iacr.org/2017/891.pdf>

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
  one. See e.g. `mirhash_seed_init()` and `mirhash_bad_seeds()` in `Hashes.h`.
  Note that a bad seed is not really a problem when you skip this seed during initialization.
  It can still be a GOOD or recommended hash function.
  But a bad seed of `0` leading to collisions is considered a bug, a bad hash function.

  We test for internal secrets, if they will be multiplied with 0. This
  is also called "blinding multiplication". `main.cpp` lists some
  secrets for each hash function we test against. The function
  `<hash>_bad_seeds()` lists the confirmed bad seeds.

  Special care needs to be taken for crc, most FNV1 variants, fletcher,
  Jenkins. And with GOOD hashes most MUM variants, like mirhash, MUM, wyhash.

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
  Murmur3*, Murmur2*, metrohash* (all but cmetro*), Crap8,
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
