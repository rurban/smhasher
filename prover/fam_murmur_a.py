"""MurmurHash2 (32 bit, SMHasher name Murmur2), MurmurHash64A (Murmur2B) and MurmurHash3_x86_32 (Murmur3A).

All three hashes are chains of bijective steps (multiplication by an odd constant, xorshift, rotation,
xor/add of a message block), so a key with ONE free block reaches every digest (pre-image), and a
differential through the chain gives seed-independent collisions. Nothing here is new:

* Pre-image by inverting the finalizer and the block mix: the "MurmurHash inverse" idea (Austin Appleby's
  own remark that the mixing is invertible; bitsquid blog "Inverse of MurmurHash"; the same inversion is the
  base of the multi-collisions below).
* Seed-independent collisions for MurmurHash2/3: Aumasson, Bernstein, Bosslet, "Hash-flooding DoS reloaded:
  attacks and defenses" (28C3 / 2012) - their differential uses exactly this "difference in the top bit
  survives multiplication by an odd constant" trick.  Derivations are repeated per function below.

Models (seed handling exactly like the SMHasher wrappers in Hashes.h):
  Murmur2   MurmurHash2(key, len, uint32 seed)           -> uint32, stored little endian
  Murmur2B  MurmurHash64A(key, len, uint64(uint32 seed)) -> uint64, stored little endian
  Murmur3A  MurmurHash3_x86_32(key, len, uint32 seed)    -> uint32, stored little endian
"""
import math

from .core import attack, Finding, register_model
from .bv import mask, inv, rotl, rotr, unxorshr

M32 = 0xFFFFFFFF
M64 = 0xFFFFFFFFFFFFFFFF


# ---------------------------------------------------------------------------
# MurmurHash2, 32 bit
# ---------------------------------------------------------------------------
MA = 0x5BD1E995          # m
MA_INV = inv(MA, 32)


def f2(k):
    """block mix of MurmurHash2: k*=m; k^=k>>24; k*=m (bijective)."""
    k = (k * MA) & M32
    k ^= k >> 24
    return (k * MA) & M32


def f2_inv(y):
    k = (y * MA_INV) & M32
    k = unxorshr(k, 24, 32)
    return (k * MA_INV) & M32


def fin2(h):
    h ^= h >> 13
    h = (h * MA) & M32
    h ^= h >> 15
    return h


def fin2_inv(h):
    h = unxorshr(h, 15, 32)
    h = (h * MA_INV) & M32
    return unxorshr(h, 13, 32)


def murmur2(key, seed=0):
    n = len(key)
    h = (seed ^ n) & M32
    i = 0
    while n - i >= 4:
        k = int.from_bytes(key[i:i + 4], "little")
        h = ((h * MA) & M32) ^ f2(k)
        i += 4
    t = key[i:]
    if t:
        h ^= int.from_bytes(t, "little")
        h = (h * MA) & M32
    return fin2(h).to_bytes(4, "little")


register_model("Murmur2", lambda key, seed: murmur2(key, seed))


@attack("Murmur2", "preimage", 32,
        method="algebraic inversion: one free 4-byte block, finalizer and block mix are bijective")
def murmur2_preimage(ctx):
    """Key = 4 free bytes (len 4).  h0 = seed ^ 4;  h1 = h0*m ^ f(k);  digest = fin(h1).
    Invert fin, xor h0*m, invert f: k = f^-1(fin^-1(target) ^ h0*m).  Known technique (MurmurHash inverse,
    bitsquid blog); cost ~1 hash evaluation instead of 2^32.  Works for any ctx.seed."""
    target = int.from_bytes(ctx.target, "little")
    h0 = (ctx.seed ^ 4) & M32
    k = f2_inv(fin2_inv(target) ^ ((h0 * MA) & M32))
    return Finding([k.to_bytes(4, "little")], [ctx.seed], 1, "closed form, 4-byte key")


D32 = 1 << 31


def murmur2_pair(k1, k2):
    """Two 8-byte keys (k1,k2) / (k1',k2') as little-endian words that collide for EVERY seed.

    len 8:  h0 = s^8;  h1 = h0*m ^ f(k1);  h2 = h1*m ^ f(k2) = h0*m^2 ^ f(k1)*m ^ f(k2);  digest = fin(h2).
    Because m is odd, (x ^ 2^31)*m = x*m ^ 2^31 for every x (the top bit flips, carries fall off).  Take
    k1' with f(k1') = f(k1) ^ 2^31 and k2' with f(k2') = f(k2) ^ 2^31:
        f(k1')*m ^ f(k2') = f(k1)*m ^ 2^31 ^ f(k2) ^ 2^31 = f(k1)*m ^ f(k2),
    so h2 is identical for every seed s (h0 only enters through h0*m^2) and so is the digest."""
    return (k1, k2), (f2_inv(f2(k1) ^ D32), f2_inv(f2(k2) ^ D32))


def le2(words):
    return b"".join(w.to_bytes(4, "little") for w in words)


def murmur2_multi(t, base=0x01234567):
    """2^t keys of length 8*t from t independent pairs (block pair i chooses the original or the flipped
    words): every combination collides for every seed (the pairs cancel one by one)."""
    keys = [b""]
    for i in range(t):
        a, b = murmur2_pair((base + 0x9E3779B1 * i) & M32, (base ^ (0x85EBCA6B * (i + 1))) & M32)
        keys = [p + le2(w) for p in keys for w in (a, b)]
    return keys


@attack("Murmur2", "seed-independent collision", 32,
        method="Aumasson/Bernstein/Bosslet 2012 differential: top-bit difference survives *m, pairs cancel")
def murmur2_sic(ctx):
    """Four 16-byte keys (two pairs, 2^2 multi-collision) - and any two of them collide - equal digest for
    every 32-bit seed. See murmur2_pair for the argument; evaluation cost: a handful of block mixes."""
    keys = murmur2_multi(2)
    return Finding(keys, [ctx.seed], 4, "2 independent pairs -> 4 keys of 16 bytes, no search")


# ---------------------------------------------------------------------------
# MurmurHash64A (SMHasher: Murmur2B), 64 bit
# ---------------------------------------------------------------------------
MB = 0xC6A4A7935BD1E995
MB_INV = inv(MB, 64)
RB = 47


def f2b(k):
    k = (k * MB) & M64
    k ^= k >> RB
    return (k * MB) & M64


def f2b_inv(y):
    k = (y * MB_INV) & M64
    k = unxorshr(k, RB, 64)
    return (k * MB_INV) & M64


def fin2b(h):
    h ^= h >> RB
    h = (h * MB) & M64
    h ^= h >> RB
    return h


def fin2b_inv(h):
    h = unxorshr(h, RB, 64)
    h = (h * MB_INV) & M64
    return unxorshr(h, RB, 64)


def murmur64a(key, seed=0):
    n = len(key)
    h = (seed ^ (n * MB)) & M64
    i = 0
    while n - i >= 8:
        k = int.from_bytes(key[i:i + 8], "little")
        h = ((h ^ f2b(k)) * MB) & M64
        i += 8
    t = key[i:]
    if t:
        h ^= int.from_bytes(t, "little")
        h = (h * MB) & M64
    return fin2b(h).to_bytes(8, "little")


register_model("Murmur2B", lambda key, seed: murmur64a(key, seed))


@attack("Murmur2B", "preimage", 64,
        method="algebraic inversion: one free 8-byte block, finalizer and block mix are bijective")
def murmur2b_preimage(ctx):
    """Key = 8 free bytes (len 8).  h0 = seed ^ 8*m;  h1 = (h0 ^ f(k))*m;  digest = fin(h1).
    k = f^-1(fin^-1(target)*m^-1 ^ h0).  Known technique (MurmurHash inverse); ~1 evaluation vs 2^64.
    The 32-bit wrapper seed is widened to uint64 by zero extension, ctx.seed is used as such."""
    target = int.from_bytes(ctx.target, "little")
    h0 = (ctx.seed ^ (8 * MB)) & M64
    k = f2b_inv(((fin2b_inv(target) * MB_INV) & M64) ^ h0)
    return Finding([k.to_bytes(8, "little")], [ctx.seed], 1, "closed form, 8-byte key")


D64 = 1 << 63


def murmur2b_pair(k1, k2):
    """Two 16-byte keys that collide for every seed.

    len 16:  h0 = s ^ 16*m;  h1 = (h0 ^ f(k1))*m;  h2 = (h1 ^ f(k2))*m;  digest = fin(h2).
    (x ^ 2^63)*m = x*m ^ 2^63 (m odd).  Flip the top bit of f(k1): h1' = h1 ^ 2^63.  Flip the top bit of
    f(k2) too: h1' ^ f(k2') = h1 ^ f(k2), so h2 and the digest are equal - for every seed."""
    return (k1, k2), (f2b_inv(f2b(k1) ^ D64), f2b_inv(f2b(k2) ^ D64))


def le2b(words):
    return b"".join(w.to_bytes(8, "little") for w in words)


@attack("Murmur2B", "seed-independent collision", 64,
        method="Aumasson/Bernstein/Bosslet 2012 differential: top-bit difference survives *m, pair cancels")
def murmur2b_sic(ctx):
    """Two 16-byte keys, same 64-bit digest for every seed (see murmur2b_pair). No search at all."""
    a, b = murmur2b_pair(0x0123456789ABCDEF, 0xFEDCBA9876543210)
    return Finding([le2b(a), le2b(b)], [ctx.seed], 2, "closed form, 2 keys of 16 bytes")


@attack("Murmur2B", "collision", 64,
        method="algebraic inversion: 16-byte key vs. 8-byte key at the known seed (state of the long key is inverted)")
def murmur2b_collision(ctx):
    """Full 64-bit collision at the known seed ctx.seed (birthday would be 2^32 evaluations).
    Key A (16 bytes) = any two words.  Key B (8 bytes, one free block): its state after the block must equal
    the state of A before the finalizer: (h0b ^ f(kb))*m = hA, so kb = f^-1(hA*m^-1 ^ h0b), with
    h0b = seed ^ 8*m.  The lengths differ, so the init values differ, which is what makes the keys differ
    (same length keys never collide for one block: the map is bijective)."""
    s = ctx.seed
    ka = (0x0123456789ABCDEF, 0x0F1E2D3C4B5A6978)
    ha = ((s ^ (16 * MB)) & M64)
    for k in ka:
        ha = ((ha ^ f2b(k)) * MB) & M64
    kb = f2b_inv(((ha * MB_INV) & M64) ^ ((s ^ (8 * MB)) & M64))
    return Finding([le2b(ka), le2b([kb])], [s], 2, "closed form, keys of 16 and 8 bytes")


# ---------------------------------------------------------------------------
# MurmurHash3_x86_32 (SMHasher: Murmur3A)
# ---------------------------------------------------------------------------
C1, C2 = 0xCC9E2D51, 0x1B873593
C1_INV, C2_INV = inv(C1, 32), inv(C2, 32)
C5 = 0xE6546B64


def k3(k):
    """block mix of MurmurHash3_x86_32: k*=c1; k=rotl(k,15); k*=c2 (bijective)."""
    k = (k * C1) & M32
    k = rotl(k, 15, 32)
    return (k * C2) & M32


def k3_inv(y):
    k = (y * C2_INV) & M32
    k = rotr(k, 15, 32)
    return (k * C1_INV) & M32


def fmix32(h):
    h ^= h >> 16
    h = (h * 0x85EBCA6B) & M32
    h ^= h >> 13
    h = (h * 0xC2B2AE35) & M32
    h ^= h >> 16
    return h


def fmix32_inv(h):
    h = unxorshr(h, 16, 32)
    h = (h * inv(0xC2B2AE35, 32)) & M32
    h = unxorshr(h, 13, 32)
    h = (h * inv(0x85EBCA6B, 32)) & M32
    return unxorshr(h, 16, 32)


def murmur3a(key, seed=0):
    n = len(key)
    h = seed & M32
    i = 0
    while n - i >= 4:
        h ^= k3(int.from_bytes(key[i:i + 4], "little"))
        h = (rotl(h, 13, 32) * 5 + C5) & M32
        i += 4
    t = key[i:]
    if t:
        h ^= k3(int.from_bytes(t, "little"))
    h ^= n
    return fmix32(h & M32).to_bytes(4, "little")


register_model("Murmur3A", lambda key, seed: murmur3a(key, seed))


@attack("Murmur3A", "preimage", 32,
        method="algebraic inversion: one free 4-byte block, fmix32, *5+c, rotation and block mix are bijective")
def murmur3a_preimage(ctx):
    """Key = 4 free bytes (len 4).  h1 = rotl(seed ^ K(k),13)*5 + c;  digest = fmix32(h1 ^ 4).
    Invert fmix32, xor 4, subtract c and multiply by 5^-1, rotate right 13, xor seed, invert the block mix K.
    Known technique (MurmurHash3 inverse); ~1 evaluation instead of 2^32."""
    target = int.from_bytes(ctx.target, "little")
    h = fmix32_inv(target) ^ 4
    h = (((h - C5) * inv(5, 32)) & M32)
    h = rotr(h, 13, 32)
    k = k3_inv(h ^ (ctx.seed & M32))
    return Finding([k.to_bytes(4, "little")], [ctx.seed], 1, "closed form, 4-byte key")


def murmur3a_pair(k1, k2):
    """Two 8-byte keys that collide for EVERY seed (differential of Aumasson/Bernstein/Bosslet 2012).

    len 8, K = K(k) is the bijective block mix:  h1 = rotl(s ^ K1, 13)*5 + c;  h2 = rotl(h1 ^ K2, 13)*5 + c.
    Block 1: choose k1' with K(k1') = K(k1) ^ 2^18.  Then (s^K1') = (s^K1) ^ 2^18 exactly (xor), rotl 13 moves
    bit 18 to bit 31, and (y ^ 2^31)*5 = y*5 ^ 2^31 (5*2^31 = 2^31 mod 2^32, carries fall off), so
    h1' = h1 ^ 2^31 for every seed.  Block 2: choose k2' with K(k2') = K(k2) ^ 2^31: then h1' ^ K2' = h1 ^ K2,
    so h2 and everything after it (xor len, fmix32) are identical."""
    return (k1, k2), (k3_inv(k3(k1) ^ (1 << 18)), k3_inv(k3(k2) ^ (1 << 31)))


@attack("Murmur3A", "seed-independent collision", 32,
        method="Aumasson/Bernstein/Bosslet 2012 differential: bit-18 / bit-31 differences cancel through rotl13,*5")
def murmur3a_sic(ctx):
    """Two 8-byte keys, same digest for every 32-bit seed (see murmur3a_pair). Closed form, no search."""
    a, b = murmur3a_pair(0x01234567, 0x89ABCDEF)
    return Finding([le2(a), le2(b)], [ctx.seed], 2, "closed form, 2 keys of 8 bytes")
