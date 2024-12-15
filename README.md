SMhasher
========

[![Linux Build status](https://travis-ci.org/rurban/smhasher.svg?branch=master)](https://travis-ci.org/rurban/smhasher/) [![Windows Build status](https://ci.appveyor.com/api/projects/status/tb6ckfcrhqi6pwn9/branch/master?svg=true)](https://ci.appveyor.com/project/rurban/smhasher) [![FreeBSD Build status](https://api.cirrus-ci.com/github/rurban/smhasher.svg?branch=master)](https://cirrus-ci.com/github/rurban/smhasher)

| Hash function                                 |      MiB/sec |cycl./hash|cycl./map   | size| Quality problems               |
|:----------------------------------------------|-------------:|---------:|-----------:|----:|--------------------------------|
| [donothing32](doc/donothing32.txt)            |  11536809.55 |     4.00 | -|  13 | bad seed 0, test NOP           |
| [donothing64](doc/donothing64.txt)            |  12269832.97 |     4.00 | -|  13 | bad seed 0, test NOP           |
| [donothing128](doc/donothing128.txt)          |  11961841.48 |     4.08 | -|  13 | bad seed 0, test NOP           |
| [NOP_OAAT_read64](doc/NOP_OAAT_read64.txt)    |  11652662.97 |    14.00 | -|  47 | test NOP                       |
| [BadHash](doc/BadHash.txt)                    |       794.68 |    72.80 | -|  47 | bad seed 0, test FAIL          |
| [sumhash](doc/sumhash.txt)                    |     10184.67 |    29.40 | -| 363 | bad seed 0, test FAIL          |
| [sumhash32](doc/sumhash32.txt)                |     44755.87 |    23.46 | -| 863 | UB, test FAIL                  |
| [multiply_shift](doc/multiply_shift.txt)      |      8286.63 |    25.39 | 209.06 (2) | 345 | bad seeds & 0xfffffff0, fails most tests |
| [pair_multiply_shift](doc/pair_multiply_shift.txt)|      5728.05 |    39.67 | 218.48 (12)| 609 | fails most tests |
| --------------------------                    |              |          |            |     |                      |
| [crc32](doc/crc32.txt)                        |       369.88 |   142.33 | 396.81 (101)| 422 | insecure, 8590x collisions, distrib, PerlinNoise |
| [md5_32](doc/md5_32.txt)                      |       359.43 |   638.92 | 865.21 (6) |4419 |  |
| [md5_64](doc/md5_64.txt)                      |       360.20 |   640.38 | 869.55 (6) |4419 |  |
| [md5-128](doc/md5-128.txt)                    |       344.63 |   669.59 | 856.34 (5) |4419 |                    |
| [sha1_32](doc/sha1_32.txt)                  |       353.03 |  1385.80 |1759.94 (5) |5126 | Sanity, Cyclic low32, 36.6% distrib |
| [sha1_64](doc/sha1_64.txt)                  |       353.03 |  1385.80 |1759.94 (5) |5126 | Sanity, Cyclic low32, 36.6% distrib |
| [sha1-160](doc/sha1-160.txt)                  |       364.95 |  1470.55 |1794.16 (13)|5126 | Comb/Cyclic low32  |
| [sha2-224](doc/sha2-224.txt)                  |       147.13 |  1354.81 |1589.92 (12)|     | Comb low32 |
| [sha2-224_64](doc/sha2-224_64.txt)            |       147.60 |  1360.10 |1620.93 (13)|     | Cyclic low32  |
| [sha2-256](doc/sha2-256.txt)                  |       147.80 |  1374.90 |1606.06 (16)|     |  |
| [sha2-256_64](doc/sha2-256_64.txt)            |       148.01 |  1376.34 |1624.71 (16)|     |  |
| [sha1ni](doc/sha1ni.txt)                      |      1632.88 |   170.77 | 379.84 (5) | 989 | insecure,sanity, Permutation, Zeroes, machine-specific |
| [sha1ni_32](doc/sha1ni_32.txt)                |      1583.50 |   172.17 | 387.70 (4) | 989 | machine-specific |
| [sha2ni-256](doc/sha2ni-256.txt)              |      1556.66 |   180.75 | 393.79 (5) |4241 | insecure,sanity, Permutation, Zeroes, machine-specific |
| [sha2ni-256_64](doc/sha2ni-256_64.txt)        |      1508.82 |   184.89 | 393.07 (6) |4241 | Zeroes, machine-specific |
| [blake3_c](doc/blake3_c.txt)                  |      1298.04 |   354.25 | 563.63 (4) |     | no 32bit portability     |
| [rmd128](doc/rmd128.txt)                      |       294.42 |   712.67 | 930.65 (4) |     |               |
| [rmd160](doc/rmd160.txt)                      |       202.16 |  1045.79 |1287.74 (16)|     | Cyclic hi32   |
| [rmd256](doc/rmd256.txt)                      |       366.21 |   615.39 | 830.04 (7) |     |               |
| [edonr224](doc/edonr224.txt)                  |       863.77 |   304.76 | 496.66 (3) |     |               |
| [edonr256](doc/edonr256.txt)                  |       870.97 |   296.40 | 517.82 (6) |     |               |
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
| [asconhashv12_64](doc/asconhashv12_64.txt)    |       159.33 |   402.54 | 473.15 (4) |6490 |               |
| [sha3-256](doc/sha3-256.txt)                  |       100.58 |  3877.18 |4159.79 (37)|     | PerlinNoise   |
| [sha3-256_64](doc/sha3-256_64.txt)            |       100.57 |  3909.00 |4174.63 (16)|     | PerlinNoise   |
| [hasshe2](doc/hasshe2.txt)                    |      2879.93 |    62.47 | 272.34 (2) | 445 | Permutation,TwoBytes,Zeroes,Seed|
| [poly_1_mersenne](doc/poly_1_mersenne.txt)    |      1278.37 |    63.42 | 244.41 (1) | 479 | fails most tests |
| [poly_2_mersenne](doc/poly_2_mersenne.txt)    |      1391.90 |    69.97 | 255.44 (3) | 479 |                  |
| [poly_3_mersenne](doc/poly_3_mersenne.txt)    |      1320.15 |    80.81 | 263.12 (2) | 479 |                  |
| [poly_4_mersenne](doc/poly_4_mersenne.txt)    |      1393.93 |    89.07 | 262.97 (2) | 479 |                  |
| [tabulation32](doc/tabulation32.txt)          |      5819.40 |    40.16 | 233.00 (3) | 848 | collisions       |
| [tabulation](doc/tabulation.txt)              |      7407.94 |    39.83 | 240.25 (4) | 554 |                  |
| [crc32_hw](doc/crc32_hw.txt)                  |      5537.79 |    40.80 | 225.65 (3) | 653 | insecure, 100% bias, collisions, distrib, BIC, machine-specific (SSE4.2/NEON) |
| [crc32_hw1](doc/crc32_hw1.txt)                |      7626.17 |    50.21 | 228.50 (2) | 671 | insecure, 100% bias, collisions, distrib, BIC, machine-specific (x86 SSE4.2)  |
| [crc64_hw](doc/crc64_hw.txt)                  |      5579.17 |    40.87 | 202.19 (1) | 652 | insecure, 100% bias, collisions, distrib, BIC, machine-specific (SSE4.2/NEON) |
| [crc32_pclmul](doc/crc32_pclmul.txt)          |      7963.20 |   106.02 | 567.70 (3) |     | insecure, 100% bias, collisions, distrib, BIC, machine-specific (x86 SSE4.2+PCLMUL) |
| [crc64_jones1](doc/crc64_jones1.txt)          |      1673.67 |    83.00 | 249.16 (2) |     | insecure, 100% bias, collisions, distrib, BIC, machine-specific  |
| [crc64_jones2](doc/crc64_jones2.txt)          |      2968.07 |   314.60 | 253.01 (3) |     | insecure, 100% bias, collisions, distrib, BIC, machine-specific  |
| [crc64_jones3](doc/crc64_jones3.txt)          |      3398.80 |   302.59 | 258.16 (8) |     | insecure, 100% bias, collisions, distrib, BIC, machine-specific  |
| [crc64_jones](doc/crc64_jones.txt)            |      3512.66 |    80.70 | 251.40 (2) |     | insecure, 100% bias, collisions, distrib, BIC, machine-specific  |
| [o1hash](doc/o1hash.txt)                      |  11530366.92 |    18.40 | 206.94 (1) | 101 | insecure, no seed, zeros, fails all tests |
| [fibonacci](doc/fibonacci.txt)                |     15409.62 |    22.67 | 872.83 (3) |1692 | UB, zeros, fails all tests       |
| [FNV1a](doc/FNV1a.txt)                        |       754.70 |    75.28 | 226.30 (2) | 204 | bad seed, zeros, fails all tests       |
| [FNV1A_Totenschiff](doc/FNV1A_Totenschiff.txt)|      6166.85 |    26.60 | 235.71 (2) | 270 | UB, zeros, fails all tests   |
| [FNV1A_Pippip_Yurii](doc/FNV1A_Pippip_Yurii.txt)|      6115.29 |    28.03 | 233.52 (2) | 147 | UB, sanity, fails all tests  |
| [FNV1a_YT](doc/FNV1a_YT.txt)                  |     13481.70 |    30.67 | 233.71 (7) | 321 | bad seed, UB, fails all tests          |
| [FNV2](doc/FNV2.txt)                          |      5630.87 |    32.09 | 207.08 (1) | 278 | fails all tests              |
| [FNV64](doc/FNV64.txt)                        |       747.70 |    74.18 | 189.18 (1) |  79 | fails all tests              |
| [FNV128](doc/FNV128.txt)                      |       408.59 |   130.82 | 299.47 (20)| 171 | fails all tests              |
| [k-hash32](doc/k-hash32.txt)                  |      2227.90 |    53.85 | 255.78 (2) | 808 | insecure, zeros, UB, bad seeds, fails all tests       |
| [k-hash64](doc/k-hash64.txt)                  |      2492.66 |    48.18 | 242.58 (2) | 609 | insecure, zeros, UB, bad seeds, fails all tests       |
| [fletcher2](doc/fletcher2.txt)                |     15410.12 |    20.72 | 345.55 (5) | 248 | bad seed 0, UB, fails all tests          |
| [fletcher4](doc/fletcher4.txt)                |     15603.68 |    21.24 | 320.09 (2) | 371 | bad seed 0, UB, fails all tests          |
| [bernstein](doc/bernstein.txt)                |      1032.00 |    59.04 | 225.09 (3) |  41 | bad seed 0, fails all tests              |
| [sdbm](doc/sdbm.txt)                          |       772.01 |    71.67 | 220.09 (2) |  41 | bad seed 0, fails all tests              |
| [x17](doc/x17.txt)                            |       765.01 |    75.61 | 225.81 (2) |  79 | 99.98% bias, fails all tests |
| [libiberty](doc/libiberty.txt)                |       618.39 |    86.58 | 220.95 (2) |  37 | insecure, 100% bias, fails all tests, bad seed |
| [gcc](doc/gcc.txt)                            |       612.10 |    88.44 | 224.62 (2) |  39 | insecure, 100% bias, fails all tests, bad seed |
| [JenkinsOOAT](doc/JenkinsOOAT.txt)            |       615.60 |   111.23 | 251.08 (2) | 153 | bad seed 0, 53.5% bias, fails all tests  |
| [JenkinsOOAT_perl](doc/JenkinsOOAT_perl.txt)  |       631.79 |    93.13 | 232.44 (1) |  65 | bad seed 0, 1.5-11.5% bias, 7.2x collisions, BIC, LongNeighbors |
| [MicroOAAT](doc/MicroOAAT.txt)                |       730.89 |    78.25 | 236.07 (3) |  68 | 100% bias, distrib, BIC      |
| [pearsonhash64](doc/pearsonhash64.txt)        |       439.85 |   123.07 | 213.68 (1) |     | Avalanche, Seed, SSSE3 only. broken MSVC     |
| [pearsonhash128](doc/pearsonhash128.txt)      |       438.49 |   123.81 | 212.89 (2) |     | Avalanche, Seed, SSSE3 only. broken MSVC     |
| [pearsonhash256](doc/pearsonhash256.txt)      |       440.63 |   120.72 | 224.63 (2) |     | Avalanche, Seed, SSSE3 only. broken MSVC     |
| [VHASH_32](doc/VHASH_32.txt)                  |     13084.37 |    65.45 | 280.14 (2) |1231 | sanity, Seed, MomentChi2     |
| [VHASH_64](doc/VHASH_64.txt)                  |     13217.64 |    64.90 | 270.04 (2) |1231 | sanity, Seed, Sparse         |
| [farsh32](doc/farsh32.txt)                    |     27583.89 |    65.65 | 266.64 (2) | 944 | insecure: AppendedZeroes, collisions+bias, MomentChi2, LongNeighbors |
| [farsh64](doc/farsh64.txt)                    |     13558.69 |   113.74 | 327.26 (3) | 944 | insecure: AppendedZeroes, collisions+bias, MomentChi2, LongNeighbors |
| [farsh128](doc/farsh128.txt)                  |      7055.39 |   229.42 | 375.57 (4) | 944 | insecure: AppendedZeroes, collisions+bias, permut,combin,2bytes,zeroes,PerlinNoise |
| [farsh256](doc/farsh256.txt)                  |      3466.26 |   444.74 | 610.23 (4) | 944 | insecure: AppendedZeroes, collisions+bias, permut,combin,2bytes,zeroes,PerlinNoise |
| [jodyhash32](doc/jodyhash32.txt)              |      1762.46 |    42.66 | 236.09 (2) | 102 | bias, collisions, distr, BIC LongNeighbors |
| [jodyhash64](doc/jodyhash64.txt)              |      4861.84 |    44.05 | 234.35 (2) | 118 | bias, collisions, distr, BIC, LongNeighbors |
| [lookup3](doc/lookup3.txt)                    |      2474.09 |    40.19 | 238.22 (3) | 341 | UB, 28% bias, collisions, 30% distr, BIC  |
| [superfast](doc/superfast.txt)                |      2085.79 |    49.89 | 230.76 (3) | 210 | UB, bad seed 0, 91% bias, 5273.01x collisions, 37% distr, BIC |
| [MurmurOAAT](doc/MurmurOAAT.txt)              |       513.75 |   105.23 | 244.81 (4) |  47 | bad seed 0, collisions, 99.998% distr., BIC, LongNeighbors |
| [Crap8](doc/Crap8.txt)                        |      3081.04 |    37.15 | 234.94 (2) | 342 | UB, 2.42% bias, collisions, 2% distrib |
| [Murmur1](doc/Murmur1.txt)                    |      1955.36 |    48.84 | 236.25 (2) | 358 | UB, 1 bad seed, 511x collisions, Diff, BIC |
| [Murmur2](doc/Murmur2.txt)                    |      3082.03 |    41.62 | 250.72 (4) | 358 | UB, 1 bad seed, 1.7% bias, 81x coll, 1.7% distrib, BIC |
| [Murmur2A](doc/Murmur2A.txt)                  |      2850.40 |    46.60 | 284.58 (13)| 407 | UB, 1 bad seed, 12.7% bias, LongNeighbors          |
| [Murmur2B](doc/Murmur2B.txt)                  |      6039.96 |    38.70 | 212.23 (1) | 433 | UB, 1.8% bias, collisions, 3.4% distrib, BIC |
| [Murmur2C](doc/Murmur2C.txt)                  |      3802.68 |    49.82 | 220.13 (2) | 444 | UB, 2^32 bad seeds, 91% bias, collisions, distr, BIC, LongNeighbors |
| [Murmur3A](doc/Murmur3A.txt)                  |      3027.30 |    48.99 | 234.49 (2) | 351 | UB, 1 bad seed, Moment Chi2 69     |
| [PMurHash32](doc/PMurHash32.txt)              |      3001.44 |    48.99 | 240.35 (3) |1862 | 1 bad seed, Moment Chi2 69         |
| [Murmur3C](doc/Murmur3C.txt)                  |      4824.95 |    57.39 | 243.91 (2) | 859 | UB, LongNeighbors, Text, DiffDist  |
| [mirhash32low](doc/mirhash32low.txt)          |      6168.04 |    38.35 | 234.32 (2) |1112 | UB, 4 bad seeds, Cyclic, LongNeighbors, machine-specific (32/64 differs) |
| [PMPML_32](doc/PMPML_32.txt)                  |      6904.30 |    44.25 | 233.59 (2) |1084 | Avalanche >512, unseeded: Seed, BIC, MomentChi2, PerlinNoise |
| [PMPML_64](doc/PMPML_64.txt)                  |     10030.67 |    49.22 | 239.07 (5) |1305 | unseeded: Seed, MomentChi2, BIC        |
| [xxHash32](doc/xxHash32.txt)                  |      6064.37 |    48.86 | 234.27 (3) | 738 | LongNeighbors, collisions with 4bit diff, MomentChi2 220 |
| [metrohash64](doc/metrohash64.txt)            |     14209.14 |    40.85 | 225.16 (2) | 624 | UB, LongNeighbors, BIC         |
| [metrohash64_1](doc/metrohash64_1.txt)        |     14495.30 |    40.83 | 213.00 (2) | 624 | UB, LongNeighbors, BIC, MomentChi2         |
| [metrohash64crc_1](doc/metrohash64crc_1.txt)  |      8010.90 |    44.43 | 213.94 (2) | 632 | UB, Cyclic 8/8 byte, DiffDist, BIC, MomentChi2, machine-specific (SSE4.2/NEON) |
| [metrohash64crc_2](doc/metrohash64crc_2.txt)  |      7939.65 |    44.84 | 222.01 (2) | 632 | UB, Cyclic 8/8 byte, DiffDist, BIC, machine-specific (SSE4.2/NEON) |
| [cmetrohash64_1o](doc/cmetrohash64_1o.txt)    |     14678.56 |    40.33 | 216.44 (2) |3506 | UB, LongNeighbors, BIC, MomentChi2  |
| [cmetrohash64_1](doc/cmetrohash64_1.txt)      |     14332.75 |    41.04 | 216.05 (2) | 652 | UB, LongNeighbors, BIC, MomentChi2 |
| [City64noSeed](doc/City64noSeed.txt)          |     14023.27 |    33.37 | 223.59 (2) |1038 | Avalanche, Sparse, TwoBytes, MomentChi2, Seed |
| [City64](doc/City64.txt)                      |     14390.09 |    46.69 | 231.99 (2) |1120 | Sparse, TwoBytes           |
| [t1ha1_64le](doc/t1ha1_64le.txt)              |     13425.03 |    31.37 | 221.97 (2) | 517 | Avalanche                  |
| [t1ha1_64be](doc/t1ha1_64be.txt)              |     12002.50 |    31.57 | 226.52 (2) | 555 | Avalanche                  |
| [t1ha0_32le](doc/t1ha0_32le.txt)              |      7276.16 |    49.11 | 236.39 (2) | 509 | Sparse, LongNeighbors      |
| [t1ha0_32be](doc/t1ha0_32be.txt)              |      6860.87 |    50.16 | 241.26 (2) | 533 | Sparse, LongNeighbors      |
| [t1ha0_aes_avx1](doc/t1ha0_aes_avx1.txt)      |     27881.52 |    36.78 | 227.13 (2) | 533 | Sparse, LongNeighbors      |
| [t1ha2_stream](doc/t1ha2_stream.txt)          |     13673.22 |    81.12 | 263.88 (3) |1665 | Sparse, Permutation, LongNeighbors |
| [t1ha2_stream128](doc/t1ha2_stream128.txt)    |     13913.43 |    94.60 | 296.15 (4) |1665 | Sparse, Permutation, LongNeighbors |
| [aesnihash](doc/aesnihash.txt)                |      5365.60 |    57.21 | 255.87 (3) |1209 | fails many tests, machine-specific (x64 AES-NI) |
| [aesni-hash-peterrk](doc/aesni-hash-peterrk.txt)|     29107.73 |    28.86 | 217.57 (1) | | machine-specific (x64 AES-NI) |
| [falkhash](doc/falkhash.txt)                  |     52401.48 |   122.70 | 316.79 (4) | 264 | Sparse, LongNeighbors, machine-specific (x64 AES-NI) |
| [MeowHash](doc/MeowHash.txt)                  |     29969.81 |    64.90 | 273.79 (8) |1764 | Sparse, invertible, machine-specific (x64 AES-NI)  |
| [MeowHash64low](doc/MeowHash64low.txt)        |     29438.45 |    63.76 | 269.41 (4) |1764 | Sparse, invertible, machine-specific (x64 AES-NI)    |
| [MeowHash32low](doc/MeowHash32low.txt)        |     30562.54 |    63.77 | 283.26 (3) |1764 | Sparse, invertible, machine-specific (x64 AES-NI)    |
| --------------------------------------        |              |          |            |     |                            |
| [tifuhash_64](doc/tifuhash_64.txt)            |        35.60 |  1679.52 |1212.75 (15)| 276 | Cyclic low32               |
| [floppsyhash](doc/floppsyhash.txt)            |        35.72 |  1868.92 |1411.07 (7) | 623 |                            |
| [beamsplitter](doc/beamsplitter.txt)          |       789.22 |   682.45 |1150.33 (26)|4203 | UB                         |
| [discohash1](doc/discohash1.txt)              |      4152.62 |   202.14 | 414.34 (4) |1294 |                            |
| [discohash1-128](doc/discohash1-128.txt)      |      4064.39 |   231.06 | 430.94 (6) |1294 |                            |
| [discohash2](doc/discohash2.txt)              |      4026.84 |   204.37 | 408.13 (6) |1294 |                            |
| [discohash2-128](doc/discohash2-128.txt)      |      4153.48 |   226.54 | 429.24 (4) |1294 |                            |
| [discoNONG](doc/discoNONG.txt)                |      3661.23 |   397.81 | 651.23 (54)|    | bad seeds                   |
| [chaskey](doc/chaskey.txt)                    |      1142.38 |   113.98 | 288.04 (1) |1609 | PerlinNoise                |
| [SipHash](doc/SipHash.txt)                    |       953.51 |   147.32 | 332.02 (5) |1071 |                            |
| [HalfSipHash](doc/HalfSipHash.txt)            |      1128.85 |    80.48 | 260.42 (2) | 700 | zeroes                     |
| [GoodOAAT](doc/GoodOAAT.txt)                  |       744.67 |    87.23 | 243.36 (2) | 237 |                            |
| [pearsonbhash64](doc/pearsonbhash64.txt)      |      1749.98 |    99.84 | 257.11 (2) | 683 |                            |
| [pearsonbhash128](doc/pearsonbhash128.txt)    |      1656.68 |   106.78 | 276.92 (3) |1134 |                            |
| [pearsonbhash256](doc/pearsonbhash256.txt)    |      1418.61 |   123.68 | 301.26 (3) | 844 |                            |
| [prvhash64_64m](doc/prvhash64_64m.txt)        |      3107.77 |    47.31 | 236.76 (2) | 349 |                            |
| [prvhash64_64](doc/prvhash64_64.txt)          |      3057.79 |    47.83 | 244.16 (2) | 384 |                            |
| [prvhash64_128](doc/prvhash64_128.txt)        |      3340.87 |    67.90 | 264.44 (2) | 718  |                           |
| [prvhash64s_64](doc/prvhash64s_64.txt)        |      6724.41 |   266.09 | 438.90 (3) |2640 |                           |
| [prvhash64s_128](doc/prvhash64s_128.txt)      |      6703.66 |   326.46 | 508.67 (3) |2799 |                            |
| [SipHash13](doc/SipHash13.txt)                |      1803.96 |   107.93 | 304.05 (3) | 778 | 0.9% bias                  |
| [TSip](doc/TSip.txt)                          |      4330.87 |    52.43 | 245.31 (2) | 519 | !msvc                      |
| [seahash](doc/seahash.txt)                    |      8240.58 |    58.61 | 260.35 (1) | 871 | PerlinNoise, !msvc         |
| [seahash32low](doc/seahash32low.txt)          |      8245.91 |    58.61 | 264.07 (2) | 871 | PerlinNoise 32, !msvc      |
| [clhash](doc/clhash.txt)                      |     22932.36 |    67.22 | 262.78 (2) |1809 | PerlinNoise, machine-specific (x64 SSE4.2) |
| [HighwayHash64](doc/HighwayHash64.txt)        |      6242.58 |    99.55 | 248.41 (3) |2546 |                            |
| [Murmur3F](doc/Murmur3F.txt)                  |      7104.08 |    52.69 | 216.34 (2) | 699 | UB                         |
| [MUM](doc/MUM.txt)                            |      9631.26 |    34.78 | 219.14 (2) |1912 | UB, too many bad seeds, machine-specific (32/64 differs) |
| [MUMlow](doc/MUMlow.txt)                      |      8532.17 |    35.95 | 238.37 (2) |1912 | UB, 5 bad seeds              |
| [xmsx32](doc/xmsx32.txt)                      |      2105.03 |    45.35 | 238.92 (3) | 192 | 2 bad seeds                  |
| [mirhash](doc/mirhash.txt)                    |      5793.85 |    38.15 | 212.91 (2) |1112 | UB, 2^36 bad seeds, LongNeighbors, machine-specific (32/64 differs) |
| [mirhashstrict](doc/mirhashstrict.txt)        |      3587.79 |    49.85 | 219.49 (2) |1112 |                            |
| [mirhashstrict32low](doc/mirhashstrict32low.txt)|      3642.77 |    50.16 | 233.34 (3) |1112 | 1 bad seed, MomentChi2 9   |
| [fasthash32](doc/fasthash32.txt)              |      6107.05 |    40.49 | 237.83 (2) | 566 | UB                         |
| [fasthash64](doc/fasthash64.txt)              |      5600.11 |    38.04 | 219.71 (2) | 509 | UB                         |
| [aesni](doc/aesni.txt)                        |     31185.98 |    29.45 | 226.75 (2) | 519 | machine-specific (x64 AES-NI)           |
| [aesni-low](doc/aesni-low.txt)                |     31027.39 |    29.47 | 232.54 (2) | 519 | machine-specific (x64 AES-NI)           |
| [mx3](doc/mx3.txt)                            |      9332.99 |    47.07 | 221.61 (2) | 734 | UB                         |
| [pengyhash](doc/pengyhash.txt)                |     13347.17 |    74.79 | 278.74 (3) | 421 |                            |
| [City32](doc/City32.txt)                      |      5745.83 |    52.44 | 242.69 (2) |1319 |                            |
| [City64low](doc/City64low.txt)                |     13119.17 |    47.92 | 251.81 (2) |1120|                             |
| [City128](doc/City128.txt)                    |     14472.20 |    88.71 | 285.61 (10)|1841 |                            |
| [CityCrc128](doc/CityCrc128.txt)              |     12255.05 |    89.25 | 281.85 (2) | 295 |                            |
| [CityCrc256](doc/CityCrc256.txt)              |     12428.45 |   164.93 | 358.73 (2) |     |                            |
| [FarmHash32](doc/FarmHash32.txt)              |     22136.14 |    46.75 | 258.73 (2) |11489| machine-specific (x64 SSE4/AVX) |
| [FarmHash64](doc/FarmHash64.txt)              |     12929.54 |    46.82 | 238.50 (2) |3758 |                            |
| [FarmHash128](doc/FarmHash128.txt)            |     14454.12 |    68.09 | 254.04 (2) | 163 |                            |
| [farmhash32_c](doc/farmhash32_c.txt)          |     22150.13 |    46.08 | 251.43 (3) | 762 | machine-specific (x64 SSE4/AVX) |
| [farmhash64_c](doc/farmhash64_c.txt)          |     12925.73 |    46.76 | 239.47 (2) |3688 |                            |
| [farmhash128_c](doc/farmhash128_c.txt)        |     14284.69 |    68.80 | 256.66 (3) |1890 |                            |
| [metrohash64_2](doc/metrohash64_2.txt)        |     14359.39 |    41.09 | 215.53 (2) | 627 | UB, LongNeighbors          |
| [cmetrohash64_2](doc/cmetrohash64_2.txt)      |     14498.20 |    40.87 | 219.65 (2) | 655 | LongNeighbors              |
| [metrohash128](doc/metrohash128.txt)          |     15847.63 |    72.33 | 266.54 (2) | 773 | UB, LongNeighbors          |
| [metrohash128_1](doc/metrohash128_1.txt)      |     15556.40 |    73.49 | 259.05 (2) | 773 | UB, LongNeighbors          |
| [metrohash128_2](doc/metrohash128_2.txt)      |     15408.56 |    74.36 | 259.57 (2) | 773 | UB, LongNeighbors          |
| [metrohash128crc_1](doc/metrohash128crc_1.txt)|      8182.02 |    77.38 | 256.80 (2) | 723 | UB, machine-specific (SSE4.2/NEON) |
| [metrohash128crc_2](doc/metrohash128crc_2.txt)|      7996.33 |    78.81 | 262.53 (2) | 723 | UB, machine-specific (SSE4.2/NEON) |
| [xxHash64](doc/xxHash64.txt)                  |     11176.72 |    49.67 | 230.24 (6) |1999 |                            |
| [Spooky32](doc/Spooky32.txt)                  |     13623.84 |    55.45 | 248.11 (3) |2221 | UB                         |
| [Spooky64](doc/Spooky64.txt)                  |     13400.95 |    55.06 | 244.86 (3) |2221 | UB                         |
| [Spooky128](doc/Spooky128.txt)                |     13621.00 |    59.05 | 235.42 (2) |2221 | UB                         |
| [SpookyV2_32](doc/SpookyV2_32.txt)            |     13626.51 |    56.19 | 256.57 (19)|2069 |                            |
| [SpookyV2_64](doc/SpookyV2_64.txt)            |     13401.13 |    56.18 | 246.33 (11)|2069 |                            |
| [SpookyV2_128](doc/SpookyV2_128.txt)          |     12252.64 |    58.94 | 235.46 (2) |2069 |                            |
| [ahash64](doc/ahash64.txt)                    |      9862.62 |    27.32 | 181.68 (1) | 412 | rust                       |
| [xxh3](doc/xxh3.txt)                          |     20853.74 |    29.46 | 220.07 (2) | 744 | DiffDist bit 7 w. 36 bits, BIC |
| [xxh3low](doc/xxh3low.txt)                    |     20568.96 |    29.98 | 243.82 (5) | 756 |                            |
| [xxh128](doc/xxh128.txt)                      |     19259.99 |    31.58 | 228.84 (2) |1012 |                            |
| [xxh128low](doc/xxh128low.txt)                |     19555.88 |    30.85 | 227.83 (2) |1012 |                            |
| [t1ha2_atonce](doc/t1ha2_atonce.txt)          |     14275.60 |    37.02 | 224.12 (2) | 541 | Zeroes low3                |
| [t1ha2_atonce128](doc/t1ha2_atonce128.txt)    |     14059.85 |    56.44 | 251.53 (3) | 613 | LongNeighbors              |
| [t1ha0_aes_noavx](doc/t1ha0_aes_noavx.txt)    |     27335.47 |    37.34 | 230.59 (2) | 925 | LongNeighbors, machine-specific (x86 AES-NI) |
| [t1ha0_aes_avx1](doc/t1ha0_aes_avx1.txt)      |     27881.52 |    36.78 | 227.13 (2) | 843 | LongNeighbors, machine-specific (x64 AVX)    |
| [t1ha0_aes_avx2](doc/t1ha0_aes_avx2.txt)      |     56581.83 |    36.51 | 223.59 (1) | 792 | LongNeighbors, machine-specific (x64 AVX2)   |
| [wyhash32](doc/wyhash32.txt)                  |      2532.89 |    48.40 | 484.57 (1) | 426 | 4 bad and broken seeds, 32-bit |
| [wyhash32low](doc/wyhash32low.txt)            |     23104.85 |    28.56 | 239.71 (4) | 474 | 5 bad seeds  |
| [wyhash](doc/wyhash.txt)                      |     22640.53 |    28.91 | 229.00 (2) | 474 |                           |
| [w1hash](doc/w1hash.txt)                      |     14208.56 |    26.85 | 221.76 (2) |     |                           |
| [rapidhash](doc/rapidhash.txt)                |     22147.09 |    29.07 | 214.80 (2) | 574 |                           |
| [rapidhash_unrolled](doc/rapidhash_unrolled.txt)|     21723.13 |    29.40 | 220.97 (3) | 782 |                           |
| [umash32](doc/umash32.txt)                    |     21999.19 |    41.14 | 239.07 (2) |1530 |                            |
| [umash32_hi](doc/umash32_hi.txt)              |     22347.33 |    41.22 | 251.12 (4) |1530 |                            |
| [umash64](doc/umash64.txt)                    |     21963.54 |    41.12 | 228.34 (1) |1530 |                            |
| [umash128](doc/umash128.txt)                  |     13629.66 |    41.65 | 228.25 (1) |1530 |                            |
| [halftime_hash64](doc/halftime_hash64.txt)    |      4801.79 |    99.05 | 310.02 (2) |2911 |                            |
| [halftime_hash128](doc/halftime_hash128.txt)  |     18220.34 |    94.31 | 307.50 (1) |2462 |                            |
| [halftime_hash256](doc/halftime_hash256.txt)  |     18249.32 |    97.56 | 322.85 (2) |2622 |                            |
| [halftime_hash512](doc/halftime_hash512.txt)  |     10906.18 |   118.54 | 326.76 (3) |3550 |                            |
| [nmhash32](doc/nmhash32.txt)                  |     12676.08 |    57.09 | 255.20 (2) |2445 |                            |
| [nmhash32x](doc/nmhash32x.txt)                |     13072.64 |    42.09 | 288.12 (3) |1494 |                            |
| [k-hashv32](doc/k-hashv32.txt)                |      8393.09 |    53.95 | 252.98 (3) |1280 |                            |
| [k-hashv64](doc/k-hashv64.txt)                |      9251.05 |    51.72 | 251.83 (2) |1279 |                            |
| [komihash](doc/komihash.txt)                  |     12179.74 |    33.23 | 224.80 (2) |1323 |                            |
| [polymur](doc/polymur.txt)                    |      9913.53 |    41.68 | 232.56 (3) |1128 |                            |
| [gxhash32](doc/gxhash32.txt)                  |     47943.08 |    37.71 | 251.38 (2) |736  | AES only                   |
| [gxhash64](doc/gxhash64.txt)                  |     48919.73 |    36.61 | 236.98 (3) |720  | AES only                   |

The sortable table variants:

* **[Default AMD Ryzen 5 3350G 3.6GHz](https://rurban.github.io/smhasher/doc/table.html)**
* [Intel i7-6820HQ 3.5GHz (Lenovo P50 from 2024)](https://rurban.github.io/smhasher/doc/i7.html)
* [fast AMD EPYC 9554P 64-Core Processor (Server from 2024)](https://rurban.github.io/smhasher/doc/epyc.html)
* [Intel i5-2300 2.8GHz](https://rurban.github.io/smhasher/doc/intel.html)
* [AMD Ryzen 5 PRO 3350G 3.6GHz 32bit 32bit](https://rurban.github.io/smhasher/doc/i686.html)
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
