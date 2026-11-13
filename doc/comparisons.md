# Top-five benchmark comparisons

Generated from the benchmark table in [README.md](../README.md). The bars
compare the five best reported values for each metric; the table describes the
hardware and measurement methods. Missing values and harness-only or deliberately
broken test entries are excluded. Faster or smaller does not imply better hash
quality or security; inspect the per-hash results before choosing a hash.

To regenerate after updating the table, run `perl generate-comparisons.pl`.

## Bulk throughput

MiB/sec; higher is better. Bars use a log(1 + value) scale.

![Bulk throughput: o1hash: 11530366.92, lanehash64: 130737.76, t1ha0_aes_avx2: 56581.83, falkhash: 52401.48, gxhash64: 48919.73](comparison-bulk.svg)

`o1hash` samples only the first, middle, and last four bytes; its reported throughput is not a full-input scan.

## Small-key latency

cycles/hash; lower is better.

![Small-key latency: lanehash64: 17.22, o1hash: 18.40, fletcher2: 20.72, fletcher4: 21.24, MUMv3: 21.58](comparison-small.svg)

## Hashmap lookup

cycles/map operation; lower is better.

![Hashmap lookup: lanehash64: 102.33, lanehash_aes64: 119.16, lanehash_aes128: 138.73, rapidhashNano: 166.80, rapidhash: 169.05](comparison-hashmap.svg)

## Code size

bytes; lower is better.

![Code size: libiberty: 37, MurmurOAAT: 45, FNV1a: 54, sdbm: 54, gcc: 55](comparison-size.svg)

