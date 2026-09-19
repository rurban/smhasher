# ChainHash registrations

ChainHash and ChainHash-128 are keyed hash functions with a proved
collision bound. Both split the message into blocks, compress each block
with carry-less multiplications by block keys, and combine the block
digests with a Horner chain in a second key over the field, with the
message length as the leading coefficient; an integer twist and a quintic
finalizer produce the digest. Source, specification and proof:
<https://github.com/thomasahle/chainhash>. The two headers in this
directory are unchanged copies of that repository's `include/chainhash.h`
and `include/chainhash128.h`.

| Registration | Digest | Blocks | Key | Verification value |
| --- | --- | --- | --- | --- |
| `chainhash` | 64 bits | 256 bytes, GF(2^64) | 64 random bytes | `ACBCBE2E` |
| `chainhash-128` | 128 bits | 512 bytes, GF(2^128) | 128 random bytes | `187D44BF` |

Collision bound (the repository's `docs/THEOREM.md` and
`docs/THEOREM-128.md`, checked in Lean): for two distinct messages of at
most L words fixed independently of the key, with a uniformly random key,
`Pr[collision] <= (p(L) + d(L)) / 2^64` for ChainHash and
`(p(L) + d(L)) / 2^128` for ChainHash-128, where `p(L)` is the block count
and `d(L)` a short-length term that saturates at 32. The strength score
`min over L of log2(L / eps)` is 63.0 bits for ChainHash and 127 bits for
ChainHash-128. These are noncryptographic hashes: the guarantee is not
adaptive and neither function is a MAC.

## Seed expansion

The suite supplies a 64-bit seed. The registrations pass it to the headers'
own seed constructors, `chainhash_key_from_seed` and
`chainhash128_key_from_seed`. Starting from the seed, each SplitMix64 step
adds `0x9e3779b97f4a7c15` modulo 2^64 and returns

```
z = (z ^ (z >> 30)) * 0xbf58476d1ce4e5b9
z = (z ^ (z >> 27)) * 0x94d049bb133111eb
z ^ (z >> 31)
```

with wrapping multiplication and no extra initial xor or discarded word.
ChainHash takes eight outputs, encoded little-endian, as the 64 key bytes
`s, y, c0, c1, c2, c3, c4, tau`; ChainHash-128 takes sixteen outputs as its
128 key bytes, two per 128-bit word in the same order. The key constructor
then expands the block keys as `kappa[m] = s^(m+1)` in the field and
precomputes the Horner powers. This expansion defines the verification
values above.

A 64-bit seed carries 64 bits of entropy. The collision bound assumes 64
(resp. 128) uniformly random key bytes; the benchmark adapter is outside
that assumption, and statistical suite results describe the adapter, not
the theorem, cryptographic security, or the absence of weak seeds.

SMHasher3's seed callback prepares a thread-local key and returns its
address; the hash function uses that pointer and no shared mutable state.
The callback runs again whenever the seed changes. Classic rurban
SMHasher's `pfHash` passes a **32-bit seed** with every call, and its speed
tests change it on every call. This adapter therefore expands the key once
per `Hash_Seed_init`, with that seed zero-extended into the same expansion,
and XORs each call's 32-bit seed into the key word `tau`, the word added to
the Horner value before the finalizer. Every pair of init seed and call
seed is thus an ordinary ChainHash key, and a call costs one store instead
of a key expansion. The verification values above follow this model:
rurban's verification test passes the same seed to `Hash_Seed_init` and to
each call.

## Bytes and verification values

Message words are canonical little-endian in every registration, and both
digests are written as canonical little-endian bytes. (In SMHasher3 the
byte-swapped 64-bit registration changes only the serialization of the
digest through `PUT_U64`, which is why its BE value differs; the 128-bit
digest is always its 16 canonical little-endian bytes, so both of its
values coincide.)

## Implementation and licensing

The headers include portable C99/C++11 paths and select their hardware
paths themselves: x86 builds need no global ISA flags (the PCLMUL and
VPCLMULQDQ paths use target attributes and are dispatched at run time),
and AArch64 needs crypto instructions enabled (`-march=armv8-a+crypto`, or
`-march=native+crypto` on Apple). Without them the headers use their
portable paths and produce the same digests. In SMHasher3, each registration's
`initfn` runs the header's self-test before any test. The ChainHash header
acknowledges Orson Peters's PolymurHash as the inspiration for its
long-input loop.

Copyright 2026 Thomas Dybdahl Ahle. MIT License; the complete text is in
LICENSE and in the registration source. No external library or submodule is
required.
