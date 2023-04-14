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
| [hasshe2](doc/hasshe2.txt)                    |      2791.93 |    63.87 | 278.67 (5) | 445 | Permutation,TwoBytes,Zeroes,Seed|
| [poly_1_mersenne](doc/poly_1_mersenne.txt)    |      1344.26 |    62.49 | 258.52 (4) | 479 | fails most tests |
| [poly_2_mersenne](doc/poly_2_mersenne.txt)    |      1355.51 |    70.68 | 267.09 (5) | 479 |                  |
| [poly_3_mersenne](doc/poly_3_mersenne.txt)    |      1344.95 |    80.08 | 277.61 (6) | 479 |                  |
| [poly_4_mersenne](doc/poly_4_mersenne.txt)    |      1358.32 |    88.02 | 281.05 (5) | 479 |                  |
| [tabulation32](doc/tabulation32.txt)          |      5666.25 |    40.75 | 247.27 (4) | 848 | collisions       |
| [tabulation](doc/tabulation.txt)              |      7860.10 |    39.88 | 260.99 (6) | 554 |                  |
| [crc32_hw](doc/crc32_hw.txt)                  |      6056.18 |    42.22 | 244.54 (5) | 653 | insecure, 100% bias, collisions, distrib, BIC, machine-specific (SSE4.2/NEON) |
| [crc32_hw1](doc/crc32_hw1.txt)                |      7304.11 |    49.65 | 248.61 (6) | 671 | insecure, 100% bias, collisions, distrib, BIC, machine-specific (x86 SSE4.2)  |
| [crc64_hw](doc/crc64_hw.txt)                  |      6149.22 |    40.10 | 214.15 (5) | 652 | insecure, 100% bias, collisions, distrib, BIC, machine-specific (SSE4.2/NEON) |
| [crc32_pclmul](doc/crc32_pclmul.txt)          |           -  |     -    |     -      |     | insecure, 100% bias, collisions, distrib, BIC, machine-specific (x86 SSE4.2+PCLMUL) |
| [o1hash](doc/o1hash.txt)                      |  11599548.76 |    18.21 | 223.19 (4) | 101 | insecure, no seed, zeros, fails all tests |
| [fibonacci](doc/fibonacci.txt)                |     15940.72 |    22.98 | 840.87 (4) |1692 | UB, zeros, fails all tests       |
| [FNV1a](doc/FNV1a.txt)                        |       762.10 |    73.72 | 250.05 (6) | 204 | bad seed, zeros, fails all tests       |
| [FNV1A_Totenschiff](doc/FNV1A_Totenschiff.txt)|      6150.00 |    26.35 | 251.72 (5) | 270 | UB, zeros, fails all tests   |
| [FNV1A_Pippip_Yurii](doc/FNV1A_Pippip_Yurii.txt)|      6094.18 |    28.29 | 260.05 (6) | 147 | UB, sanity, fails all tests  |
| [FNV1a_YT](doc/FNV1a_YT.txt)                  |     13451.31 |    30.57 | 244.22 (5) | 321 | bad seed, UB, fails all tests          |
| [FNV2](doc/FNV2.txt)                          |      6199.86 |    32.10 | 216.13 (5) | 278 | fails all tests              |
| [FNV64](doc/FNV64.txt)                        |       762.15 |    73.38 | 204.27 (5) |  79 | fails all tests              |
| [FNV128](doc/FNV128.txt)                      |       388.63 |   138.02 | 310.77 (11)| 171 | fails all tests              |
| [k-hash32](doc/k-hash32.txt)                  |      2221.69 |    53.26 | 264.07 (4) | 808 | insecure, zeros, UB, bad seeds, fails all tests       |
| [k-hash64](doc/k-hash64.txt)                  |      2418.02 |    49.37 | 247.61 (4) | 609 | insecure, zeros, UB, bad seeds, fails all tests       |
| [fletcher2](doc/fletcher2.txt)                |     15241.81 |    21.05 | 349.08 (4) | 248 | bad seed 0, UB, fails all tests          |
| [fletcher4](doc/fletcher4.txt)                |     15339.28 |    20.88 | 352.19 (4) | 371 | bad seed 0, UB, fails all tests          |
| [bernstein](doc/bernstein.txt)                |      1014.45 |    58.11 | 238.29 (4) |  41 | bad seed 0, fails all tests              |
| [sdbm](doc/sdbm.txt)                          |       770.27 |    70.06 | 234.11 (6) |  41 | bad seed 0, fails all tests              |
| [x17](doc/x17.txt)                            |       761.56 |    74.03 | 240.13 (6) |  79 | 99.98% bias, fails all tests |
| [libiberty](doc/libiberty.txt)                |       610.09 |    85.35 | 242.93 (5) |  37 | insecure, 100% bias, fails all tests, bad seed |
| [gcc](doc/gcc.txt)                            |       616.49 |    85.62 | 242.07 (5) |  39 | insecure, 100% bias, fails all tests, bad seed |
| [JenkinsOOAT](doc/JenkinsOOAT.txt)            |       619.78 |   108.61 | 268.47 (5) | 153 | bad seed 0, 53.5% bias, fails all tests  |
| [JenkinsOOAT_perl](doc/JenkinsOOAT_perl.txt)  |       615.95 |    93.03 | 257.38 (4) |  65 | bad seed 0, 1.5-11.5% bias, 7.2x collisions, BIC, LongNeighbors |
| [MicroOAAT](doc/MicroOAAT.txt)                |       739.74 |    75.39 | 244.15 (5) |  68 | 100% bias, distrib, BIC      |
| [pearsonhash64](doc/pearsonhash64.txt)        |       434.37 |   123.30 | 229.72 (6) |     | Avalanche, Seed, SSSE3 only. broken MSVC     |
| [pearsonhash128](doc/pearsonhash128.txt)      |       441.53 |   119.06 | 238.70 (5) |     | Avalanche, Seed, SSSE3 only. broken MSVC     |
| [pearsonhash256](doc/pearsonhash256.txt)      |       430.46 |   118.16 | 248.71 (6) |     | Avalanche, Seed, SSSE3 only. broken MSVC     |
| [VHASH_32](doc/VHASH_32.txt)                  |     13283.54 |    64.91 | 295.63 (5) |1231 | sanity, Seed, MomentChi2     |
| [VHASH_64](doc/VHASH_64.txt)                  |     13057.72 |    66.00 | 291.31 (5) |1231 | sanity, Seed, Sparse         |
| [farsh32](doc/farsh32.txt)                    |     27073.86 |    66.85 | 291.09 (5) | 944 | insecure: AppendedZeroes, collisions+bias, MomentChi2, LongNeighbors |
| [farsh64](doc/farsh64.txt)                    |     13742.68 |   114.13 | 331.15 (5) | 944 | insecure: AppendedZeroes, collisions+bias, MomentChi2, LongNeighbors |
| [farsh128](doc/farsh128.txt)                  |      6915.41 |   234.60 | 401.77 (4) | 944 | insecure: AppendedZeroes, collisions+bias, permut,combin,2bytes,zeroes,PerlinNoise |
| [farsh256](doc/farsh256.txt)                  |      3411.21 |   451.97 | 628.91 (3) | 944 | insecure: AppendedZeroes, collisions+bias, permut,combin,2bytes,zeroes,PerlinNoise |
| [jodyhash32](doc/jodyhash32.txt)              |      1758.56 |    41.88 | 249.24 (5) | 102 | bias, collisions, distr, BIC LongNeighbors |
| [jodyhash64](doc/jodyhash64.txt)              |      3482.23 |    30.03 | 244.51 (6) | 118 | bias, collisions, distr, BIC, LongNeighbors |
| [lookup3](doc/lookup3.txt)                    |      2467.12 |    39.90 | 247.48 (6) | 341 | UB, 28% bias, collisions, 30% distr, BIC  |
| [superfast](doc/superfast.txt)                |      2062.25 |    49.52 | 247.20 (4) | 210 | UB, bad seed 0, 91% bias, 5273.01x collisions, 37% distr, BIC |
| [MurmurOAAT](doc/MurmurOAAT.txt)              |       507.14 |   103.15 | 246.66 (6) |  47 | bad seed 0, collisions, 99.998% distr., BIC, LongNeighbors |
| [Crap8](doc/Crap8.txt)                        |      3097.22 |    36.49 | 252.62 (6) | 342 | UB, 2.42% bias, collisions, 2% distrib |
| [Murmur1](doc/Murmur1.txt)                    |      2065.77 |    47.62 | 247.08 (4) | 358 | UB, 1 bad seed, 511x collisions, Diff, BIC |
| [Murmur2](doc/Murmur2.txt)                    |      3043.78 |    41.85 | 249.46 (6) | 358 | UB, 1 bad seed, 1.7% bias, 81x coll, 1.7% distrib, BIC |
| [Murmur2A](doc/Murmur2A.txt)                  |      3073.22 |    46.11 | 253.88 (5) | 407 | UB, 1 bad seed, 12.7% bias, LongNeighbors          |
| [Murmur2B](doc/Murmur2B.txt)                  |      6178.48 |    38.12 | 237.04 (5) | 433 | UB, 1.8% bias, collisions, 3.4% distrib, BIC |
| [Murmur2C](doc/Murmur2C.txt)                  |      3794.65 |    49.28 | 231.58 (5) | 444 | UB, 2^32 bad seeds, 91% bias, collisions, distr, BIC, LongNeighbors |
| [Murmur3A](doc/Murmur3A.txt)                  |      3037.83 |    48.32 | 250.33 (5) | 351 | UB, 1 bad seed, Moment Chi2 69     |
| [PMurHash32](doc/PMurHash32.txt)              |      3112.42 |    47.26 | 252.54 (5) |1862 | 1 bad seed, Moment Chi2 69         |
| [Murmur3C](doc/Murmur3C.txt)                  |      4834.87 |    56.75 | 258.31 (5) | 859 | UB, LongNeighbors, Text, DiffDist  |
| [mirhash32low](doc/mirhash32low.txt)          |      6057.51 |    37.61 | 254.30 (8) |1112 | UB, 4 bad seeds, Cyclic, LongNeighbors, machine-specific (32/64 differs) |
| [PMPML_32](doc/PMPML_32.txt)                  |      6870.14 |    43.94 | 265.62 (5) |1084 | Avalanche >512, unseeded: Seed, BIC, MomentChi2, PerlinNoise |
| [PMPML_64](doc/PMPML_64.txt)                  |      9984.71 |    49.26 | 249.64 (7) |1305 | unseeded: Seed, MomentChi2, BIC        |
| [xxHash32](doc/xxHash32.txt)                  |      5896.91 |    48.59 | 248.97 (7) | 738 | LongNeighbors, collisions with 4bit diff, MomentChi2 220 |
| [metrohash64](doc/metrohash64.txt)            |     14443.60 |    40.32 | 225.16 (5) | 624 | UB, LongNeighbors, BIC         |
| [metrohash64_1](doc/metrohash64_1.txt)        |     14549.52 |    40.57 | 259.47 (5) | 624 | UB, LongNeighbors, BIC, MomentChi2         |
| [metrohash64crc_1](doc/metrohash64crc_1.txt)  |      8164.67 |    45.69 | 225.83 (4) | 632 | UB, Cyclic 8/8 byte, DiffDist, BIC, MomentChi2, machine-specific (SSE4.2/NEON) |
| [metrohash64crc_2](doc/metrohash64crc_2.txt)  |      7971.63 |    44.07 | 234.54 (7) | 632 | UB, Cyclic 8/8 byte, DiffDist, BIC, machine-specific (SSE4.2/NEON) |
| [cmetrohash64_1o](doc/cmetrohash64_1o.txt)    |     14734.93 |    39.62 | 223.84 (4) |3506 | UB, LongNeighbors, BIC, MomentChi2  |
| [cmetrohash64_1](doc/cmetrohash64_1.txt)      |     14453.95 |    40.43 | 239.68 (6) | 652 | UB, LongNeighbors, BIC, MomentChi2 |
| [City64noSeed](doc/City64noSeed.txt)          |     13900.14 |    32.21 | 244.86 (4) |1038 | Avalanche, Sparse, TwoBytes, MomentChi2, Seed |
| [City64](doc/City64.txt)                      |     14011.88 |    45.67 | 244.04 (5) |1120 | Sparse, TwoBytes           |
| [t1ha1_64le](doc/t1ha1_64le.txt)              |     13369.16 |    31.55 | 242.73 (6) | 517 | Avalanche                  |
| [t1ha1_64be](doc/t1ha1_64be.txt)              |     11591.96 |    32.69 | 244.10 (5) | 555 | Avalanche                  |
| [t1ha0_32le](doc/t1ha0_32le.txt)              |      7245.81 |    49.24 | 258.06 (6) | 509 | Sparse, LongNeighbors      |
| [t1ha0_32be](doc/t1ha0_32be.txt)              |      6754.43 |    50.64 | 253.15 (7) | 533 | Sparse, LongNeighbors      |
| [t1ha2_stream](doc/t1ha2_stream.txt)          |     13718.38 |    82.35 | 293.58 (5) |1665 | Sparse, Permutation, LongNeighbors |
| [t1ha2_stream128](doc/t1ha2_stream128.txt)    |     13801.39 |    96.21 | 320.21 (6) |1665 | Sparse, Permutation, LongNeighbors |
| [aesnihash](doc/aesnihash.txt)                |      5183.33 |    58.52 | 322.74 (36)|1209 | fails many tests, machine-specific (x64 AES-NI) |
| [falkhash](doc/falkhash.txt)                  |     50838.94 |   123.38 | 330.16 (6) | 264 | Sparse, LongNeighbors, machine-specific (x64 AES-NI) |
| [MeowHash](doc/MeowHash.txt)                  |     29521.98 |    65.83 | 287.28 (4) |1764 | Sparse, invertible, machine-specific (x64 AES-NI)  |
| [MeowHash64low](doc/MeowHash64low.txt)        |     29793.14 |    65.24 | 285.78 (5) |1764 | Sparse, invertible, machine-specific (x64 AES-NI)    |
| [MeowHash32low](doc/MeowHash32low.txt)        |     30002.99 |    64.81 | 297.76 (6) |1764 | Sparse, invertible, machine-specific (x64 AES-NI)    |
| --------------------------------------        |              |          |            |     |                            |
| [tifuhash_64](doc/tifuhash_64.txt)            |        35.60 |  1679.52 |1212.75 (15)| 276 | Cyclic low32               |
| [floppsyhash](doc/floppsyhash.txt)            |        35.72 |  1868.92 |1411.07 (7) | 623 |                            |
| [beamsplitter](doc/beamsplitter.txt)          |       789.22 |   682.45 |1150.33 (26)|4203 | UB       |
| [BEBB4185](doc/BEBB4185.txt)                  |      3959.08 |   199.92 | 413.43 (6) |1294 | UB, msvc-specific |
| [chaskey](doc/chaskey.txt)                    |      1150.69 |   113.09 | 308.01 (5) |1609 | PerlinNoise                |
| [SipHash](doc/SipHash.txt)                    |       945.64 |   146.95 | 346.53 (5) |1071 |                            |
| [HalfSipHash](doc/HalfSipHash.txt)            |      1108.77 |    82.02 | 285.78 (5) | 700 | zeroes                     |
| [GoodOAAT](doc/GoodOAAT.txt)                  |       744.66 |    85.45 | 244.27 (5) | 237 |                            |
| [pearsonbhash64](doc/pearsonbhash64.txt)      |      1743.30 |   100.63 | 280.97 (6) | 683 |                            |
| [pearsonbhash128](doc/pearsonbhash128.txt)    |      1668.94 |   105.95 | 294.48 (4) |1134 |                            |
| [pearsonbhash256](doc/pearsonbhash256.txt)    |      1413.41 |   124.67 | 321.99 (6) | 844 |                            |
| [prvhash64_64m](doc/prvhash64_64m.txt)        |      3044.42 |    48.75 | 241.19 (5) | 349 |                            |
| [prvhash64_64](doc/prvhash64_64.txt)          |      3092.32 |    48.61 | 242.33 (4) | 384 |                            |
| [prvhash64_128](doc/prvhash64_128.txt)        |      3213.89 |    87.23 | 297.25 (6) | 718  |                           |
| [prvhash64s_64](doc/prvhash64s_64.txt)        |      5999.29 |   296.28 | 501.91 (5) |2640 |                            |
| [prvhash64s_128](doc/prvhash64s_128.txt)      |      5671.81 |   357.67 | 559.14 (6) |2799 |                            |
| [SipHash13](doc/SipHash13.txt)                |      1813.55 |   106.44 | 314.29 (5) | 778 | 0.9% bias                  |
| [TSip](doc/TSip.txt)                          |      4233.52 |    53.23 | 261.81 (6) | 519 | !msvc                      |
| [seahash](doc/seahash.txt)                    |      8322.53 |    58.50 | 267.77 (5) | 871 | PerlinNoise, !msvc         |
| [seahash32low](doc/seahash32low.txt)          |      8177.83 |    59.45 | 292.87 (6) | 871 | PerlinNoise 32, !msvc      |
| [clhash](doc/clhash.txt)                      |     17688.81 |    69.15 | 287.95 (6) |1809 | PerlinNoise, machine-specific (x64 SSE4.2) |
| [HighwayHash64](doc/HighwayHash64.txt)        |      6242.58 |    99.55 | 248.41 (3) |2546 |                            |
| [Murmur3F](doc/Murmur3F.txt)                  |      7625.10 |    52.69 | 228.96 (5) | 699 | UB                         |
| [MUM](doc/MUM.txt)                            |      9353.53 |    35.80 | 239.36 (6) |1912 | UB, too many bad seeds, machine-specific (32/64 differs) |
| [MUMlow](doc/MUMlow.txt)                      |      8978.00 |    36.15 | 267.84 (5) |1912 | UB, 5 bad seeds                      |
| [mirhash](doc/mirhash.txt)                    |      6008.40 |    37.79 | 237.03 (5) |1112 | UB, 2^36 bad seeds, LongNeighbors, machine-specific (32/64 differs) |
| [mirhashstrict](doc/mirhashstrict.txt)        |      3545.19 |    50.03 | 234.09 (6) |1112 |                            |
| [mirhashstrict32low](doc/mirhashstrict32low.txt)|      3566.47 |    49.73 | 263.25 (4) |1112 | 1 bad seed, MomentChi2 9   |
| [fasthash32](doc/fasthash32.txt)              |      6075.38 |    40.68 | 262.40 (6) | 566 | UB                         |
| [fasthash64](doc/fasthash64.txt)              |      6173.22 |    38.08 | 228.68 (6) | 509 | UB                         |
| [aesni](doc/aesni.txt)                        |      6104.04 |    31.60 | 272.99 (21)| 519 | !msvc, machine-specific (x64 AES-NI)           |
| [aesni-low](doc/aesni-low.txt)                |      6063.23 |    31.78 | 239.88 (24)| 519 | !msvc, machine-specific (x64 AES-NI)           |
| [mx3](doc/mx3.txt)                            |      9039.63 |    48.63 | 236.57 (5) | 734 | UB                         |
| [pengyhash](doc/pengyhash.txt)                |     13293.76 |    75.49 | 286.41 (6) | 421 |                            |
| [City32](doc/City32.txt)                      |      5637.89 |    53.19 | 260.99 (6) |1319 |                            |
| [City64low](doc/City64low.txt)                |     13897.36 |    46.24 | 267.69 (5) |1120|                             |
| [City128](doc/City128.txt)                    |     14129.60 |    87.98 | 290.70 (7) |1841 |                            |
| [CityCrc128](doc/CityCrc128.txt)              |      8041.40 |    54.69 | 257.22 (5) | 295 |                            |
| [FarmHash32](doc/FarmHash32.txt)              |     21706.27 |    47.69 | 271.51 (6) |11489| machine-specific (x64 SSE4/AVX) |
| [FarmHash64](doc/FarmHash64.txt)              |     12965.76 |    46.42 | 258.29 (6) |3758 |                            |
| [FarmHash128](doc/FarmHash128.txt)            |     14144.18 |    69.22 | 269.64 (5) | 163 |                            |
| [farmhash32_c](doc/farmhash32_c.txt)          |     21913.08 |    47.22 | 267.19 (5) | 762 | machine-specific (x64 SSE4/AVX) |
| [farmhash64_c](doc/farmhash64_c.txt)          |     12853.81 |    46.99 | 250.29 (5) |3688 |                            |
| [farmhash128_c](doc/farmhash128_c.txt)        |     13877.86 |    68.19 | 271.10 (4) |1890 |                            |
| [metrohash64_2](doc/metrohash64_2.txt)        |     13634.01 |    40.14 | 231.99 (6) | 627 | UB, LongNeighbors          |
| [cmetrohash64_2](doc/cmetrohash64_2.txt)      |     14292.65 |    40.64 | 224.73 (4) | 655 | LongNeighbors              |
| [metrohash128](doc/metrohash128.txt)          |     15640.32 |    73.22 | 269.23 (5) | 773 | UB, LongNeighbors          |
| [metrohash128_1](doc/metrohash128_1.txt)      |     15802.66 |    72.40 | 266.20 (6) | 773 | UB, LongNeighbors          |
| [metrohash128_2](doc/metrohash128_2.txt)      |     15576.05 |    73.58 | 273.15 (6) | 773 | UB, LongNeighbors          |
| [metrohash128crc_1](doc/metrohash128crc_1.txt)|      7961.55 |    78.91 | 276.21 (5) | 723 | UB, machine-specific (SSE4.2/NEON) |
| [metrohash128crc_2](doc/metrohash128crc_2.txt)|      8016.22 |    78.59 | 274.52 (5) | 723 | UB, machine-specific (SSE4.2/NEON) |
| [xxHash64](doc/xxHash64.txt)                  |     12226.39 |    49.20 | 236.97 (5) |1999 |                            |
| [Spooky32](doc/Spooky32.txt)                  |     13122.35 |    56.13 | 260.40 (5) |2221 | UB                         |
| [Spooky64](doc/Spooky64.txt)                  |     13340.58 |    55.22 | 250.99 (5) |2221 | UB                         |
| [Spooky128](doc/Spooky128.txt)                |     13119.47 |    58.89 | 250.41 (6) |2221 | UB                         |
| [SpookyV2_32](doc/SpookyV2_32.txt)            |     13326.98 |    56.54 | 261.84 (4) |2069 |                            |
| [SpookyV2_64](doc/SpookyV2_64.txt)            |     13104.98 |    57.54 | 252.86 (7) |2069 |                            |
| [SpookyV2_128](doc/SpookyV2_128.txt)          |     13316.09 |    59.19 | 252.60 (5) |2069 |                            |
| [ahash64](doc/ahash64.txt)                    |      9862.62 |    27.32 | 181.68 (1) | 412 | rust                       |
| [xxh3](doc/xxh3.txt)                          |     20383.53 |    30.79 | 237.87 (6) | 744 | DiffDist bit 7 w. 36 bits, BIC |
| [xxh3low](doc/xxh3low.txt)                    |     20722.94 |    30.26 | 256.34 (7) | 756 |                            |
| [xxh128](doc/xxh128.txt)                      |     18819.68 |    32.66 | 256.52 (4) |1012 |                            |
| [xxh128low](doc/xxh128low.txt)                |     19043.36 |    31.99 | 250.40 (6) |1012 |                            |
| [t1ha2_atonce](doc/t1ha2_atonce.txt)          |     13926.52 |    37.70 | 245.33 (6) | 541 | Zeroes low3                |
| [t1ha2_atonce128](doc/t1ha2_atonce128.txt)    |     13854.28 |    57.06 | 266.53 (4) | 613 | LongNeighbors              |
| [t1ha0_aes_noavx](doc/t1ha0_aes_noavx.txt)    |     27527.84 |    37.23 | 245.10 (6) | 925 | LongNeighbors, machine-specific (x86 AES-NI) |
| [t1ha0_aes_avx1](doc/t1ha0_aes_avx1)          |     22714.85 |    48.12 | 226.52 (16)| 843 | LongNeighbors, machine-specific (x64 AVX.txt)|
| [t1ha0_aes_avx2](doc/t1ha0_aes_avx2.txt)      |     55655.02 |    37.25 | 258.23 (5) | 792 | LongNeighbors, machine-specific (x64 AVX2)   |
| [wyhash32](doc/wyhash32.txt)                  |      2532.89 |    48.40 | 484.57 (1) | 426 | 4 bad and broken seeds, 32-bit |
| [wyhash32low](doc/wyhash32low.txt)            |     22895.72 |    28.87 | 260.96 (6) | 474 | 12 bad and broken seeds  |
| [wyhash](doc/wyhash.txt)                      |     22513.04 |    29.01 | 228.76 (5) | 474 |                           |
| [umash32](doc/umash32.txt)                    |     22373.83 |    41.98 | 257.19 (6) |1530 |                            |
| [umash32_hi](doc/umash32_hi.txt)              |     22935.43 |    42.16 | 257.93 (5) |1530 |                            |
| [umash64](doc/umash64.txt)                    |     22695.69 |    42.02 | 246.91 (5) |1530 |                            |
| [umash128](doc/umash128.txt)                  |     13024.01 |    42.98 | 242.48 (4) |1530 |                            |
| [halftime_hash64](doc/halftime_hash64.txt)    |      4765.33 |    99.67 | 324.58 (4) |2911 |                            |
| [halftime_hash128](doc/halftime_hash128.txt)  |     17545.03 |    98.17 | 322.50 (5) |2462 |                            |
| [halftime_hash256](doc/halftime_hash256.txt)  |     18018.00 |    99.86 | 326.90 (4) |2622 |                            |
| [halftime_hash512](doc/halftime_hash512.txt)  |     10894.15 |   118.66 | 335.38 (5) |3550 |                            |
| [nmhash32](doc/nmhash32.txt)                  |     12785.12 |    56.66 | 271.67 (4) |2445 |                            |
| [nmhash32x](doc/nmhash32x.txt)                |     12702.23 |    42.94 | 261.02 (5) |1494 |                            |
| [k-hashv32](doc/k-hashv32.txt)                |      9044.40 |    53.38 | 259.25 (5) |1280 |                            |
| [k-hashv64](doc/k-hashv32.txt)                |      7850.92 |    46.94 | 193.94 (1) |1279 |                            |
| [komihash](doc/komihash.txt)                  |     12339.00 |    33.52 | 245.07 (4) | 728 |                            |

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

* xxh3low
* wyhash
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
