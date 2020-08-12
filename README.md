SMhasher
========

[![Linux Build status](https://travis-ci.org/rurban/smhasher.svg?branch=master)](https://travis-ci.org/rurban/smhasher/) [![Windows Build status](https://ci.appveyor.com/api/projects/status/tb6ckfcrhqi6pwn9/branch/master?svg=true)](https://ci.appveyor.com/project/rurban/smhasher) [![FreeBSD Build status](https://api.cirrus-ci.com/github/rurban/smhasher.svg?branch=master)](https://cirrus-ci.com/github/rurban/smhasher)

| Hash function                                 |      MiB/sec |cycl./hash|cycl./map   | size| Quality problems               |
|:----------------------------------------------|-------------:|---------:|-----------:|----:|--------------------------------|
| [donothing32](doc/donothing32.txt)            |  14924689.47 |     6.00 | -          |  13 | test NOP                       |
| [donothing64](doc/donothing64.txt)            |  14942317.78 |     6.00 | -          |  13 | test NOP                       |
| [donothing128](doc/donothing128.txt)          |  14943678.61 |     6.00 | -          |  13 | test NOP                       |
| [NOP_OAAT_read64](doc/NOP_OAAT_read64.txt)    |     29794.83 |    35.50 | -          |  47 | test NOP                       |
| [BadHash](doc/BadHash.txt)                    |       522.75 |    96.39 | -          |  47 | test FAIL                      |
| [sumhash](doc/sumhash.txt)                    |      7135.41 |    31.14 | -          | 363 | test FAIL                      |
| [sumhash32](doc/sumhash32.txt)                |     23873.87 |    22.49 | -          | 863 | test FAIL                      |
| [multiply_shift](doc/multiply_shift.txt)      |      4909.57 |    45.28 | too slow   | 345 | fails all tests                |
| [pair_multiply_shift](doc/pair_multiply_shift)|     13604.46 |    31.71 | too slow   | 609 | fails all tests                |
| --------------------------                    |              |          |            |     |                                |
| [crc32](doc/crc32.txt)                        |       392.05 |   130.08 | 199.87 (3) | 422 | insecure, 8590x collisions, distrib |
| [md5_32a](doc/md5_32a.txt)                    |       351.96 |   670.99 | 863.30 (23)|4419 | 8590x collisions, distrib |
| [sha1_32a](doc/sha1_32a.txt)                  |       353.03 |  1385.80 |1759.94 (5) |5126 | collisions, 36.6% distrib |
| [md5-128](doc/md5-128.txt)                    |       317.78 |   730.30 | 830.69 (7) |4419 |               |
| [sha1-160](doc/sha1-160.txt)                  |       364.95 |  1470.55 |1794.16 (13)|5126 |               |
| [sha2-224](doc/sha2-224.txt)                  |       147.13 |  1354.81 |1589.92 (12)|     |               |
| [sha2-224_64](doc/sha2-224_64.txt)            |       147.60 |  1360.10 |1620.93 (13)|     |               |
| [sha2-256](doc/sha2-256.txt)                  |       147.80 |  1374.90 |1606.06 (16)|     | Moment Chi2 4 |
| [sha2-256_64](doc/sha2-256_64.txt)            |       148.01 |  1376.34 |1624.71 (16)|     | Moment Chi2 7 |
| [sha1ni](doc/sha1ni.txt)                      |      2019.96 |   135.84 | 564.40 (6) | 989 | insecure,sanity, Permutation, Zeroes, machine-specific |
| [sha1ni_32](doc/sha1ni_32.txt)                |      2019.94 |   136.82 | 589.46 (1) | 989 | machine-specific |
| [sha2ni-256](doc/sha2ni-256.txt)              |      1906.77 |   145.47 | 603.08 (22)|4241 | insecure,sanity, Permutation, Zeroes, machine-specific |
| [sha2ni-256_64](doc/sha2ni-256_64.txt)        |      1910.34 |   146.06 | 595.16 (6) |4241 | machine-specific |
| [blake3_c](doc/blake3_c.txt)                  |      1233.40 |   360.68 | 524.40 (4) |     | Moment Chi2, no 32bit portability |
| [rmd128](doc/rmd128.txt)                      |       332.78 |   672.35 | 903.43 (13)|     |               |
| [rmd160](doc/rmd160.txt)                      |       202.16 |  1045.79 |1287.74 (16)|     |               |
| [rmd256](doc/rmd256.txt)                      |       356.57 |   638.30 | 815.39 (16)|     |               |
| [blake2s-128](doc/blake2s-128.txt)            |       295.30 |   698.09 |1059.24 (51)|     |               |
| [blake2s-160](doc/blake2s-160.txt)            |       215.01 |  1026.74 |1239.54 (11)|     |               |
| [blake2s-224](doc/blake2s-224.txt)            |       207.06 |  1063.86 |1236.50 (20)|     |               |
| [blake2s-256](doc/blake2s-256.txt)            |       215.28 |  1014.88 |1230.38 (28)|     |               |
| [blake2s-256_64](doc/blake2s-256_64.txt)      |       211.52 |  1044.22 |1228.43 (8) |     |               |
| [blake2b-160](doc/blake2b-160.txt)            |       356.08 |  1236.84 |1458.15 (12)|     |               |
| [blake2b-224](doc/blake2b-224.txt)            |       356.59 |  1228.50 |1425.87 (16)|     |               |
| [blake2b-256](doc/blake2b-256.txt)            |       355.97 |  1232.22 |1443.31 (19)|     | Sparse high 32-bit |
| [blake2b-256_64](doc/blake2b-256_64.txt)      |       356.97 |  1222.76 |1435.03 (9) |     |               |
| [sha3-256](doc/sha3-256.txt)                  |       100.58 |  3877.18 |4159.79 (37)|     |               |
| [sha3-256_64](doc/sha3-256_64.txt)            |       100.57 |  3909.00 |4174.63 (16)|     |               |
| [hasshe2](doc/hasshe2.txt)                    |      2357.32 |    76.10 | 172.41 (3) | 445 | insecure, Permutation,TwoBytes,Zeroes,Seed  |
| [crc32_hw](doc/crc32_hw.txt)                  |      6292.63 |    30.38 | 204.19 (18)| 653 | insecure, 100% bias, collisions, distrib, BIC, machine-specific (x86 SSE4.2) |
| [crc32_hw1](doc/crc32_hw1.txt)                |     23382.53 |    36.84 | 197.39 (20)| 671 | insecure, 100% bias, collisions, distrib, BIC, machine-specific (x86 SSE4.2) |
| [crc64_hw](doc/crc64_hw.txt)                  |      8387.01 |    29.96 | 161.44 (13)| 652 | insecure, 100% bias, collisions, distrib, BIC, machine-specific (x64 SSE4.2) |
| [crc32_pclmul](doc/crc32_pclmul.txt)          |   1932125.24 |     6.50 |   -        | 481 | insecure, 100% bias, collisions, distrib, BIC, machine-specific (x86 SSE4.2+PCLMUL) |
| [o1hash](doc/o1hash.txt)                      |  14635755.30 |    16.04 | 357.77 (3) |     | insecure, zeros, fails all tests |
| [fibonacci](doc/fibonacci.txt)                |      9462.17 |    33.52 | 774.87 (14)|1692 | zeros, fails all tests       |
| [FNV1a](doc/FNV1a.txt)                        |       780.53 |    69.65 | 197.06 (20)| 204 | zeros, fails all tests       |
| [FNV1A_Totenschiff](doc/FNV1A_Totenschiff.txt)|      6235.86 |    36.56 | 214.61 (18)| 270 | zeros, fails all tests       |
| [FNV1A_Pippip_Yurii](doc/FNV1A_Pippip_Yurii.txt)|    6238.08 |    37.22 | 218.06 (25)| 147 | sanity, fails all tests      |
| [FNV1a_YT](doc/FNV1a_YT.txt)                  |      9618.28 |    28.26 | 192.33 (18)| 321 | fails all tests              |
| [FNV2](doc/FNV2.txt)                          |      6244.26 |    38.25 | 168.46 (15)| 278 | fails all tests              |
| [FNV64](doc/FNV64.txt)                        |       791.82 |    69.82 | 184.87 (19)|  79 | fails all tests              |
| [fletcher2](doc/fletcher2.txt)                |     11809.88 |    26.85 | 321.91 (13)| 248 | fails all tests              |
| [fletcher4](doc/fletcher4.txt)                |     11973.55 |    26.94 | 324.53 (12)| 371 | fails all tests              |
| [bernstein](doc/bernstein.txt)                |       791.83 |    67.15 | 200.11 (17)|  41 | fails all tests              |
| [sdbm](doc/sdbm.txt)                          |       785.84 |    67.02 | 209.44 (17)|  41 | fails all tests              |
| [x17](doc/x17.txt)                            |       522.46 |    97.51 | 214.03 (18)|  79 | 99.98% bias, fails all tests |
| [JenkinsOOAT](doc/JenkinsOOAT.txt)            |       446.57 |   142.43 | 245.34 (20)| 153 | 53.5% bias, fails all tests  |
| [JenkinsOOAT_perl](doc/JenkinsOOAT_perl.txt)  |       445.69 |   118.69 | 222.62 (18)|  65 | 1.5-11.5% bias, 7.2x collisions, BIC, LongNeighbors |
| [MicroOAAT](doc/MicroOAAT.txt)                |       950.84 |    62.23 | 233.94 (19)|  68 | 100% bias, distrib, BIC      |
| [VHASH_32](doc/VHASH_32.txt)                  |      9502.36 |    86.97 | 273.67 (8) |1231 | sanity, Seed, MomentChi2     |
| [VHASH_64](doc/VHASH_64.txt)                  |      9449.24 |    86.97 | 253.42 (9) |1231 | sanity, Seed, Sparse         |
| [farsh32](doc/farsh32.txt)                    |     14053.09 |    74.29 | 245.33 (3) | 944 | insecure: AppendedZeroes, collisions+bias, MomentChi2, LongNeighbors |
| [farsh64](doc/farsh64.txt)                    |      7216.29 |   130.30 | 302.44 (3) | 944 | insecure: AppendedZeroes, collisions+bias, MomentChi2, LongNeighbors |
| [jodyhash32](doc/jodyhash32.txt)              |      1386.76 |    46.13 | 232.12 (21)| 102 | bias, collisions, distr, BIC LongNeighbors |
| [jodyhash64](doc/jodyhash64.txt)              |      2765.84 |    40.89 | 222.41 (20)| 118 | bias, collisions, distr, BIC, LongNeighbors |
| [lookup3](doc/lookup3.txt)                    |      1702.13 |    48.25 | 243.90 (19)| 341 | 28% bias, collisions, 30% distr, BIC  |
| [superfast](doc/superfast.txt)                |      1899.70 |    54.19 | 240.49 (19)| 210 | 91% bias, 5273.01x collisions, 37% distr, BIC |
| [MurmurOAAT](doc/MurmurOAAT.txt)              |       437.03 |   117.00 | 236.62 (19)|  47 | collisions, 99.998% distr., BIC, LongNeighbors |
| [Crap8](doc/Crap8.txt)                        |      3060.17 |    35.25 | 242.59 (20)| 342 | 2.42% bias, collisions, 2% distrib |
| [Murmur2](doc/Murmur2.txt)                    |      3032.69 |    41.97 | 233.68 (19)| 358 | 1.7% bias, 81x coll, 1.7% distrib, BIC |
| [Murmur2A](doc/Murmur2A.txt)                  |      3033.56 |    46.63 | 227.70 (20)| 407 | 12.7% bias, LongNeighbors          |
| [Murmur2B](doc/Murmur2B.txt)                  |      4728.94 |    47.90 | 192.82 (18)| 433 | 1.8% bias, collisions, 3.4% distrib, BIC |
| [Murmur2C](doc/Murmur2C.txt)                  |      3817.99 |    47.67 | 207.53 (18)| 444 | 91% bias, collisions, distr, BIC, LongNeighbors |
| [Murmur3A](doc/Murmur3A.txt)                  |      2347.90 |    51.38 | 237.58 (19)| 351 | Moment Chi2 69             |
| [PMurHash32](doc/PMurHash32.txt)              |      2281.97 |    58.65 | 259.59 (20)|1862 | Moment Chi2 69             |
| [Murmur3C](doc/Murmur3C.txt)                  |      3093.46 |    68.53 | 256.49 (19)| 859 | LongNeighbors, DiffDist    |
| [PMPML_32](doc/PMPML_32.txt)                  |      6516.19 |    52.60 | 227.47 (8) |1084 | Avalanche >512, unseeded: Seed, BIC, MomentChi2 |
| [PMPML_64](doc/PMPML_64.txt)                  |      8119.83 |    61.49 | 186.24 (4) |1305 | unseeded: Seed, MomentChi2, BIC        |
| [xxHash32](doc/xxHash32.txt)                  |      5868.40 |    49.08 | 222.09 (21)| 738 | LongNeighbors, collisions with 4bit diff, MomentChi2 220 |
| [metrohash64](doc/metrohash64.txt)            |      9490.26 |    49.84 | 150.49 (3) | 624 | LongNeighbors, BIC         |
| [metrohash64_1](doc/metrohash64_1.txt)        |      9274.60 |    50.85 | 201.37 (19)| 624 | LongNeighbors, BIC, MomentChi2         |
| [metrohash64crc_1](doc/metrohash64crc_1.txt)  |     13641.79 |    56.19 | 193.28 (19)| 632 | cyclic collisions 8 byte, BIC, MomentChi2, machine-specific (x64 SSE4.2) |
| [metrohash64crc_2](doc/metrohash64crc_2.txt)  |     13550.77 |    56.17 | 204.84 (18)| 632 | cyclic collisions 8 byte, BIC, machine-specific (x64 SSE4.2) |
| [cmetrohash64_1o](doc/cmetrohash64_1o.txt)    |      8851.32 |    50.59 | 193.19 (18)|3506 | LongNeighbors, BIC, MomentChi2  |
| [cmetrohash64_1](doc/cmetrohash64_1.txt)      |      9074.37 |    50.92 | 201.33 (18)| 652 | LongNeighbors, BIC, MomentChi2 |
| [City64noSeed](doc/City64noSeed.txt)          |      8873.07 |    38.70 | 171.57 (4) |1038 | Avalanche, Sparse, TwoBytes, MomentChi2, Seed |
| [City64](doc/City64.txt)                      |      9213.05 |    55.52 | 200.24 (2) |1120 | Sparse, TwoBytes           |
| [t1ha1_64le](doc/t1ha1_64le.txt)              |      9541.33 |    39.33 | 239.04 (16)| 517 | Avalanche                  |
| [t1ha1_64be](doc/t1ha1_64be.txt)              |      6848.76 |    41.08 | 230.22 (17)| 555 | Avalanche                  |
| [t1ha0_32le](doc/t1ha0_32le.txt)              |      4913.74 |    55.00 | 233.39 (18)| 509 | Sparse, LongNeighbors      |
| [t1ha0_32be](doc/t1ha0_32be.txt)              |      4241.68 |    55.12 | 236.50 (18)| 533 | Sparse, LongNeighbors      |
| [t1ha2_stream](doc/t1ha2_stream.txt)          |      8558.73 |    93.45 | 305.03 (16)|1665 | Sparse, Permutation, LongNeighbors |
| [t1ha2_stream128](doc/t1ha2_stream128.txt)    |      8846.52 |   117.19 | 270.95 (2) |1665 | Sparse, Permutation, LongNeighbors |
| [aesnihash](doc/aesnihash.txt)                |      2963.39 |    71.24 | 217.73 (3) |     | fails most tests, machine-specific (x64 AES-NI) |
| [falkhash](doc/falkhash.txt)                  |     20374.98 |   169.84 | 328.42 (5) | 264 | LongNeighbors, machine-specific (x64 AES-NI) |
| [MeowHash](doc/MeowHash.txt)                  |     36695.65 |    58.09 | 233.81 (3) |1764 | Sparse low32, machine-specific (x64 AES-NI)  |
| [MeowHash32low](doc/MeowHash32low.txt)        |     17247.34 |    87.32 | 245.98 (3) |1764 | Sparse, machine-specific (x64 AES-NI.txt)    |
| --------------------------------------        |              |          |            |     |                            |
| [tifuhash_64](doc/tifuhash_64.txt)            |        35.60 |  1679.52 |1212.75 (15)| 276 |                            |
| [floppsyhash_64](doc/floppsyhash.txt)         |       191.95 |   450.93 | 821.11 (152)| 623 |                            |
| [prvhash](doc/prvhash.txt)                    |       393.21 |   142.03 | 532.15 (125)| 157 |                            |
| [chaskey](doc/chaskey.txt)                    |       735.85 |   168.05 | 336.07 (4) |1609 |                            |
| [SipHash](doc/SipHash.txt)                    |       958.78 |   141.84 | 278.15 (3) |1071 |                            |
| [HalfSipHash](doc/HalfSipHash.txt)            |       741.59 |   122.25 | 256.22 (20)| 700 | zeroes                     |
| [beamsplitter](doc/beamsplitter.txt)          |       789.22 |   682.45 |1150.33 (26)|     |                            |
| [GoodOAAT](doc/GoodOAAT.txt)                  |      1052.90 |    70.80 | 208.00 (3) | 237 |                            |
| [SipHash13](doc/SipHash13.txt)                |      1762.44 |   104.61 | 304.84 (17)| 778 | 0.9% bias                  |
| [TSip](doc/TSip.txt)                          |      3346.72 |    60.02 | 203.82 (5) | 519 | !msvc                      |
| [seahash](doc/seahash.txt)                    |      4529.38 |    65.58 | 240.34 (7) | 871 | !msvc                      |
| [seahash32low](doc/seahash32low.txt)          |      4524.65 |    65.60 | 253.50 (3) | 871 | !msvc                      |
| [clhash](doc/clhash.txt)                      |      4405.28 |    85.35 | 288.20 (14)|1809 | machine-specific (x64 SSE4.2) |
| [HighwayHash64](doc/HighwayHash64.txt)        |      6239.64 |    98.65 | 263.94 (4) |2546 |                            |
| [BEBB4185](doc/BEBB4185.txt)                  |      2655.75 |   241.48 | 420.58 (7) |     |                            |
| [Murmur3F](doc/Murmur3F.txt)                  |      5076.20 |    51.66 | 222.39 (18)| 699 |                            |
| [fasthash32](doc/fasthash32.txt)              |      4658.24 |    50.50 | 181.96 (2) | 566 |                            |
| [fasthash64](doc/fasthash64.txt)              |      4657.64 |    47.71 | 168.22 (3) | 509 | Moment Chi2 5159 !         |
| [MUM](doc/MUM.txt)                            |      6890.75 |    39.90 | 174.94 (3) |1912 | machine-specific (32/64 differs) |
| [MUMlow](doc/MUMlow.txt)                      |      6893.98 |    46.02 | 191.55 (3) |1912 |                            |
| [mirhash](doc/mirhash.txt)                    |      5453.50 |    42.31 | 163.35 (2) |1112 | LongNeighbors, machine-specific (32/64 differs) |
| [mirhash32low](doc/mirhash32low.txt)          |      5452.48 |    42.31 | 190.92 (2) |1112 | LongNeighbors, machine-specific (32/64 differs) |
| [mirhashstrict](doc/mirhashstrict.txt)        |      2217.70 |    65.39 | 175.38 (3) |1112 |                            |
| [mirhashstrict32low](doc/mirhashstrict32low.txt)|    2217.87 |    64.72 | 188.44 (4) |1112 | MomentChi2 9               |
| [City32](doc/City32.txt)                      |      3833.52 |    52.97 | 211.69 (4) |1319 |                            |
| [City64low](doc/City64low.txt)                |      9209.88 |    63.55 | 274.62 (17)|1120|                             |
| [City128](doc/City128.txt)                    |      9747.33 |    77.22 | 269.02 (14)|1841 |                            |
| [CityCrc128](doc/CityCrc128.txt)              |     12958.29 |    77.31 | 267.14 (17)| 295 |                            |
| [FarmHash32](doc/FarmHash32.txt)              |     16663.96 |    63.67 | 265.35 (19)|11489| machine-specific (x64 SSE4/AVX) |
| [FarmHash64](doc/FarmHash64.txt)              |      8476.56 |    65.05 | 251.81 (17)|3758 |                            |
| [FarmHash128](doc/FarmHash128.txt)            |      9814.32 |    82.06 | 229.38 (4) | 163 |                            |
| [farmhash32_c](doc/farmhash32_c.txt)          |     17335.51 |    63.77 | 277.75 (18)| 762 | machine-specific (x64 SSE4/AVX) |
| [farmhash64_c](doc/farmhash64_c.txt)          |      8335.98 |    77.01 | 266.15 (18)|3688 |                            |
| [farmhash128_c](doc/farmhash128_c.txt)        |     10158.20 |    98.80 | 232.00 (2) |1890 |                            |
| [metrohash64_2](doc/metrohash64_2.txt)        |      9281.99 |    51.04 | 191.44 (20)| 627 | LongNeighbors              |
| [cmetrohash64_2](doc/cmetrohash64_2.txt)      |      9302.98 |    51.09 | 194.04 (18)| 655 | LongNeighbors              |
| [metrohash128](doc/metrohash128.txt)          |      9450.78 |    78.18 | 215.79 (2) | 773 | LongNeighbors              |
| [metrohash128_1](doc/metrohash128_1.txt)      |      9263.64 |    79.71 | 240.70 (20)| 773 | LongNeighbors              |
| [metrohash128_2](doc/metrohash128_2.txt)      |      9060.39 |    79.74 | 240.46 (20)| 773 | LongNeighbors              |
| [metrohash128crc_1](doc/metrohash128crc_1.txt)|     13668.29 |    85.94 | 249.99 (18)| 723 | machine-specific (x64 SSE4.2) |
| [metrohash128crc_2](doc/metrohash128crc_2.txt)|     13600.83 |    85.92 | 239.28 (20)| 723 | machine-specific (x64 SSE4.2) |
| [xxHash64](doc/xxHash64.txt)                  |      8719.19 |    59.22 | 210.40 (16)|1999 |                            |
| [Spooky32](doc/Spooky32.txt)                  |      9570.40 |    70.27 | 250.23 (18)|2221 |                            |
| [Spooky64](doc/Spooky64.txt)                  |      9603.18 |    70.19 | 231.73 (17)|2221 |                            |
| [Spooky128](doc/Spooky128.txt)                |      9865.67 |    70.84 | 185.64 (2) |2221 |                            |
| [xxh3](doc/xxh3.txt)                          |     16377.47 |    37.20 | 179.88 (1) | 744 | BIC                        |
| [xxh3low](doc/xxh3low.txt)                    |     16403.22 |    37.19 | 184.18 (2) | 756 |                            |
| [xxh128](doc/xxh128.txt)                      |     15117.76 |    44.70 | 178.72 (1) |1012 |                            |
| [xxh128low](doc/xxh128low.txt)                |     15109.04 |    38.98 | 174.91 (3) |1012 |                            |
| [t1ha2_atonce](doc/t1ha2_atonce.txt)          |      8192.03 |    48.48 | 230.46 (14)| 541 |                            |
| [t1ha2_atonce128](doc/t1ha2_atonce128.txt)    |      8382.53 |    64.15 | 199.63 (4) | 613 | LongNeighbors              |
| [t1ha0_aes_noavx](doc/t1ha0_aes_noavx.txt)    |     21782.86 |    48.17 | 238.14 (18)| 925 | LongNeighbors, machine-specific (x86 AES-NI) |
| [t1ha0_aes_avx1](doc/t1ha0_aes_avx1)          |     22714.85 |    48.12 | 226.52 (16)| 843 | LongNeighbors, machine-specific (x64 AVX.txt)|
| [t1ha0_aes_avx2](doc/t1ha0_aes_avx2.txt)      |     22345.33 |    44.38 | 556.47 (89)| 792 | LongNeighbors, machine-specific (x64 AVX2)   |
| [wyhash32](doc/wyhash32.txt)                  |      2532.89 |    48.40 | 484.57 (1)|      | 32-bit                     |
| [wyhash32low](doc/wyhash32low.txt)            |     12049.79 |    37.95 | 222.17 (4) | 922 |                            |
| [wyhash](doc/wyhash.txt)                      |     12063.37 |    38.11 | 208.49 (3) | 938 |                            |

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
- fasthash32
- MUM/mir (_different results on 32/64-bit archs_)

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
