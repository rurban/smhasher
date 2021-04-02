# Reference and optimized C and ASM implementations of Ascon

Ascon is a family of lightweight authenticated encryption schemes with associated data (AEAD), including a hash and extendible output function (XOF).

For more information on Ascon visit: https://ascon.iaik.tugraz.at/

This repository contains the following 5 Ascon algorithms:

- `crypto_aead/ascon128v12`: Ascon-128 v1.2
- `crypto_aead/ascon128av12`: Ascon-128a v1.2
- `crypto_aead/ascon80pqv12`: Ascon-80pq v1.2
- `crypto_hash/asconhashv12`: Ascon-Hash v1.2
- `crypto_hash/asconxofv12`: Ascon-Xof v1.2

and the following implementations:

- `ref`: reference implementation
- `opt64`: 64-bit speed-optimized C implementation
- `opt64_lowsize`: 64-bit size-optimized C implementation
- `avx512`: AVX512 speed-optimized inline assembly implementation
- `neon`: NEON speed-optimized ARM inline assembly implementation
- `bi32`: 32-bit speed-optimized bit-interleaved C implementation
- `bi32_arm`: 32-bit speed-optimized bit-interleaved ARM inline assembly implementation
- `bi32_lowreg`: 32-bit speed-optimized bit-interleaved C implementation (low register usage)
- `bi32_lowsize`: 32-bit size-optimized bit-interleaved C implementation
- `opt8`: 8-bit optimized C implementation
- `bi8`: 8-bit optimized bit-interleaved C implementation


## Performance results of Ascon-128 on different CPUs in cycles per byte:

| Message Length in Bytes: |    1 |    8 |   16 |   32 |   64 | 1536 | long |
|:-------------------------|-----:|-----:|-----:|-----:|-----:|-----:|-----:|
| AMD Ryzen 7 1700\*       |      |      |      |      | 14.5 |  8.8 |  8.6 |
| Intel Xeon E5-2609 v4\*  |      |      |      |      | 17.3 | 10.8 | 10.5 |
| Cortex-A53 (ARMv8)\*     |      |      |      |      | 18.3 | 11.3 | 11.0 |
| Intel Core i5-6300U      |  367 |   58 |   35 |   23 | 17.6 | 11.9 | 11.4 |
| Intel Core i5-4200U      |  521 |   81 |   49 |   32 | 23.9 | 16.2 | 15.8 |
| Cortex-A15 (ARMv7)\*     |      |      |      |      | 69.8 | 36.2 | 34.6 |
| Cortex-A7 (NEON)         | 2182 |  249 |  148 |   97 | 71.7 | 47.5 | 46.5 |
| Cortex-A7 (ARMv7)        | 1871 |  292 |  175 |  115 | 86.6 | 58.3 | 57.2 |
| ARM1176JZF-S (ARMv6)     | 2136 |  312 |  186 |  123 | 91.6 | 61.8 | 62.2 |

\* Results taken from eBACS: http://bench.cr.yp.to/


## Performance results of Ascon-128a on different CPUs in cycles per byte:

| Message Length in Bytes: |    1 |    8 |   16 |   32 |   64 | 1536 | long |
|:-------------------------|-----:|-----:|-----:|-----:|-----:|-----:|-----:|
| AMD Ryzen 7 1700\*       |      |      |      |      | 12.0 |  6.0 |  5.7 |
| Intel Xeon E5-2609 v4\*  |      |      |      |      | 14.1 |  7.3 |  6.9 |
| Cortex-A53 (ARMv8)\*     |      |      |      |      | 15.1 |  7.6 |  7.3 |
| Intel Core i5-6300U      |  365 |   47 |   31 |   19 | 13.5 |  8.0 |  7.8 |
| Intel Core i5-4200U      |  519 |   67 |   44 |   27 | 18.8 | 11.0 | 10.6 |
| Cortex-A15 (ARMv7)\*     |      |      |      |      | 60.3 | 25.3 | 23.8 |
| Cortex-A7 (NEON)         | 2204 |  226 |  132 |   82 | 55.9 | 31.7 | 30.7 |
| Cortex-A7 (ARMv7)        | 1911 |  255 |  161 |  102 | 71.3 | 42.3 | 41.2 |
| ARM1176JZF-S (ARMv6)     | 2118 |  261 |  170 |  107 | 75.6 | 46.0 | 46.6 |

\* Results taken from eBACS: http://bench.cr.yp.to/


## Implementation interface

All implementations use the interface defined by the ECRYPT Benchmarking of Cryptographic Systems (eBACS):

- https://bench.cr.yp.to/call-aead.html for CRYPTO\_AEAD (Ascon-128, Ascon-128a, Ascon-80pq)
- https://bench.cr.yp.to/call-hash.html for CRYPTO\_HASH (Ascon-Hash) and XOF (Ascon-Xof)


## Manually build and run a single Ascon target:

Build example for CRYPTO\_AEAD algorithms: 

```
gcc -march=native -O3 -DNDEBUG -Icrypto_aead/ascon128v12/opt64 crypto_aead/ascon128v12/opt64/*.c -Itests tests/genkat_aead.c -o genkat
gcc -march=native -O3 -DNDEBUG -Icrypto_aead/ascon128v12/opt64 crypto_aead/ascon128v12/opt64/*.c -DCRYPTO_AEAD -Itests tests/getcycles.c -o getcycles
```

Build example for CRYPTO\_HASH algorithms: 

```
gcc -march=native -O3 -DNDEBUG -Icrypto_hash/asconhashv12/opt64 crypto_hash/asconhashv12/opt64/*.c -Itests tests/genkat_hash.c -o genkat
gcc -march=native -O3 -DNDEBUG -Icrypto_hash/asconhashv12/opt64 crypto_hash/asconhashv12/opt64/*.c -DCRYPTO_HASH -Itests tests/getcycles.c -o getcycles
```

Generate KATs and get CPU cycles:

```
./genkat
./getcycles
```


## Build and test all Ascon v1.2 targets using performance flags:

```
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
cmake --build .
ctest
```


## Build and test all Ascon v1.2 targets using NIST flags and sanitizers:

```
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Debug
cmake --build .
ctest
```


## Build and run only specific algorithms, implementations and tests:

Build and test:

```
mkdir build && cd build
cmake .. -DVERSION_LIST="v12" -DALG_LIST="ascon128;asconhash" -DIMPL_LIST="opt64;bi32" -DTEST_LIST="genkat;getcycles"
cmake --build .
ctest -R genkat
```

Get CPU cycles:

```
./getcycles_crypto_aead_ascon128v12_opt64
./getcycles_crypto_aead_ascon128v12_bi32
./getcycles_crypto_hash_asconhashv12_opt64
./getcycles_crypto_hash_asconhashv12_bi32
```


## Hints to get more reliable getcycles results on Intel/AMD CPUs:

* Determine the processor base frequency (also called design frequency):
  - e.g. using the Intel/AMD website
  - or using `lscpu` listed under model name

* Disable turbo boost (this should lock the frequency to the next value
  below the processor base frequency):
  ```
  echo 1 | sudo tee /sys/devices/system/cpu/intel_pstate/no_turbo
  ```

* If the above does not work, manually set the frequency using e.g. `cpufreq-set`.

* Determine the actual frequency (under load):
  - e.g. by watching the frequency using `lscpu` or `cpufreq-info`

* Determine the scaling factor between the actual and base frequency:
  - factor = actual frequency / base frequency

* Run a getcycles program using the frequency factor and watch the results:
  ```
  while true; do ./getcycles_crypto_aead_ascon128v12_opt64 $factor; done
  ```

* Run the `benchmark-getcycles.sh` script with the frequency factor and a
  specific algorithm to benchmark all correspondng getcycles implementations:
  ```
  ./benchmark-getcycles.sh $factor ascon128
  ```


## Hints to activate the performance monitor unit (PMU) on ARM CPUs:

* First try to install `linux-tools` and see if it works.

* On many ARM platforms, the PMU has to be enabled using a kernel module:
  - Source code for Armv6 (32-bit):
    <http://sandsoftwaresound.net/raspberry-pi/raspberry-pi-gen-1/performance-counter-kernel-module/>
  - Source code for Armv7 (32-bit):
    <https://github.com/thoughtpolice/enable_arm_pmu>
  - Source code for Armv8/Aarch64 (64-bit):
    <https://github.com/rdolbeau/enable_arm_pmu>

* Steps to compile the kernel module on the raspberry pi:
  - Find out the kernel version using `uname -a`
  - Download the kernel header files, e.g. `raspberrypi-kernel-header`
  - Download the source code for the Armv6 kernel module
  - Build, install and load the kernel module


## Benchmark Ascon v1.2 using supercop

Download supercop according to the website: http://bench.cr.yp.to/supercop.html

To test only Ascon, just run the following commands:

```
./do-part init
./do-part crypto_aead ascon128v12
./do-part crypto_aead ascon128av12
./do-part crypto_aead ascon80pqv12
./do-part crypto_hash asconhashv12
./do-part crypto_hash asconxofv12
```


## Evaluate and optimize Ascon on constraint devices:

* The ascon-c code allows to set compile-time parameters `ASCON_INLINE_MODE`
  (IM), `ASCON_INLINE_PERM` (IP), `ASCON_UNROLL_LOOPS` (UL), via command line or
  in the `crypto_*/ascon*/*/config.h` files.
* Use the `benchmark-config.sh` script to evaluate all combinations of these
  parameters for a given list of ascon implementations. The script is called
  with an output file, frequency factor, the algorithm, and the list of
  implementations to test:
  ```
  ./benchmark-config.sh results.md $factor ascon128 ref opt64 opt64_lowsize
  ```
* The `results.md` file then contains a markup table with size and cycles for
  each implementation and parameter set to evaluate several time-area
  trade-offs.

