# Tachyon Hash — C Reference Implementation

A C99 reference implementation of the Tachyon hash algorithm.

> **Note:** Tachyon is designed for AVX-512 (VAES + VPCLMULQDQ) and AES-NI.
> The short-input path uses AES-NI even when AVX-512 is available.
> The portable backend exists for correctness verification and
> cross-platform compatibility, but is not the intended performance target.

For algorithm specification, architecture decisions, and security notes see the
[main repository](https://github.com/byt3forg3/Tachyon).

## Output

| Property          | Value                                    |
|---                |---                                       |
| Digest size       | 256 bit (32 bytes)                       |
| Internal state    | 4096 bit (≥ 64 B input), 512 bit (< 64 B)|
| Block size        | 512 bytes                                |

## Files

| File                     | Purpose                                                        |
|---                       |---                                                             |
| `tachyon.h`              | Public API (one-shot + streaming)                              |
| `tachyon_impl.h`         | Internal constants, macros, and kernel prototypes              |
| `tachyon_dispatcher.c`   | CPUID runtime dispatch, Merkle tree engine, public API wrappers|
| `tachyon_portable.c`     | Pure C fallback kernel (software AES + CLMUL)                  |
| `tachyon_aesni.c`        | AES-NI + PCLMUL kernel (SSE4.1)                                |
| `tachyon_avx512.c`       | AVX-512 + VAES + VPCLMULQDQ kernel                             |

## Backend Selection

Three backends are selected automatically at runtime via CPUID:

1. **AVX-512** — requires AVX-512F, AVX-512BW, VAES, VPCLMULQDQ + OS support (XCR0)
2. **AES-NI** — requires AES-NI, SSE4.1, PCLMUL
3. **Portable** — pure C, runs on any platform (ARM, RISC-V, etc.)

## License

Copyright (c) byt3forg3 — 260008633+byt3forg3@users.noreply.github.com

Licensed under the MIT or Apache 2.0 License.
