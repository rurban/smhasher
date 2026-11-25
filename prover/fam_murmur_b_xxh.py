"""MurmurHash3 128-bit (SMHasher names Murmur3C = x86_128 and Murmur3F = x64_128), xxHash32 and xxHash64.

All attacks here are algebraic: every step of these hashes (multiplication by an odd constant, xorshift,
rotation, addition, xor) is a bijection of one lane, so a block of free key bytes can be solved for.
Nothing here is new; literature relied on:
  * Aumasson / Bernstein / Bosslet, "Hash-flooding DoS reloaded: attacks and defenses" (29C3, 2012):
    seed-independent ("universal") multicollisions for MurmurHash2/3 from a difference in the top bit,
    which survives additions and multiplications by odd constants without carry;
  * the bitsquid "MurmurHash inverse" blog and Wang's integer-hash inverses (invertibility of the
    Murmur finalisers / multiply-xorshift steps).

Honest scope.  "Automatic" here means: per family, the bijective steps are inverted by code (no SMT
solver). Seed-independent collisions are a differential that holds for every seed by an algebraic
argument given in each docstring; the harness additionally checks many seeds natively.
"""
import hashlib  # noqa: F401  (kept for ad-hoc experiments)

from .core import attack, Finding, register_model
from .bv import mask, inv, rotl, rotr, unxorshr, le, from_le

M32, M64 = mask(32), mask(64)


# ---------------------------------------------------------------------------
# MurmurHash3 helpers
# ---------------------------------------------------------------------------
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
    h = unxorshr(h, 16, 32)
    return h


F64A, F64B = 0xFF51AFD7ED558CCD, 0xC4CEB9FE1A85EC53


def fmix64(k):
    k ^= k >> 33
    k = (k * F64A) & M64
    k ^= k >> 33
    k = (k * F64B) & M64
    k ^= k >> 33
    return k


def fmix64_inv(k):
    k = unxorshr(k, 33, 64)
    k = (k * inv(F64B, 64)) & M64
    k = unxorshr(k, 33, 64)
    k = (k * inv(F64A, 64)) & M64
    k = unxorshr(k, 33, 64)
    return k


INV5_32, INV5_64 = inv(5, 32), inv(5, 64)

# --- x64_128 ---------------------------------------------------------------
C64_1, C64_2 = 0x87C37B91114253D5, 0x4CF5AD432745937F
N64_1, N64_2 = 0x52DCE729, 0x38495AB5


def pre64a(k):   # k1 premix
    return (rotl((k * C64_1) & M64, 31, 64) * C64_2) & M64


def pre64a_inv(k):
    return (rotr((k * inv(C64_2, 64)) & M64, 31, 64) * inv(C64_1, 64)) & M64


def pre64b(k):   # k2 premix
    return (rotl((k * C64_2) & M64, 33, 64) * C64_1) & M64


def pre64b_inv(k):
    return (rotr((k * inv(C64_1, 64)) & M64, 33, 64) * inv(C64_2, 64)) & M64


def murmur3_x64_128(key, seed=0):
    key = bytes(key)
    n = len(key)
    h1 = h2 = seed & M32
    nb = n // 16
    for i in range(nb):
        k1 = from_le(key[16 * i:16 * i + 8])
        k2 = from_le(key[16 * i + 8:16 * i + 16])
        h1 ^= pre64a(k1)
        h1 = (((rotl(h1, 27, 64) + h2) & M64) * 5 + N64_1) & M64
        h2 ^= pre64b(k2)
        h2 = (((rotl(h2, 31, 64) + h1) & M64) * 5 + N64_2) & M64
    tail = key[16 * nb:]
    t = len(tail)
    if t > 8:
        h2 ^= pre64b(from_le(tail[8:]))
    if t > 0:
        h1 ^= pre64a(from_le(tail[:8]))
    h1 ^= n
    h2 ^= n
    h1 = (h1 + h2) & M64
    h2 = (h2 + h1) & M64
    h1, h2 = fmix64(h1), fmix64(h2)
    h1 = (h1 + h2) & M64
    h2 = (h2 + h1) & M64
    return le(h1, 8) + le(h2, 8)


register_model("Murmur3F", murmur3_x64_128)

# --- x86_128 ---------------------------------------------------------------
C32 = (0x239B961B, 0xAB0E9789, 0x38B34AE5, 0xA1E38B93)
R32 = (15, 16, 17, 18)       # premix rotation of k1..k4
S32 = (19, 17, 15, 13)       # rotation of h1..h4 in the block step
N32 = (0x561CCD1B, 0x0BCAA747, 0x96CD1C35, 0x32AC3B17)


def pre32(i, k):
    """premix of lane i: k *= c[i]; k = rotl(k, R32[i]); k *= c[(i+1)%4]."""
    return (rotl((k * C32[i]) & M32, R32[i], 32) * C32[(i + 1) % 4]) & M32


def pre32_inv(i, k):
    return (rotr((k * inv(C32[(i + 1) % 4], 32)) & M32, R32[i], 32) * inv(C32[i], 32)) & M32


def murmur3_x86_128(key, seed=0):
    key = bytes(key)
    n = len(key)
    s = seed & M32
    h = [s, s, s, s]
    nb = n // 16
    for b in range(nb):
        k = [from_le(key[16 * b + 4 * j:16 * b + 4 * j + 4]) for j in range(4)]
        kp = [pre32(j, k[j]) for j in range(4)]
        h[0] ^= kp[0]
        h[0] = (((rotl(h[0], 19, 32) + h[1]) & M32) * 5 + N32[0]) & M32
        h[1] ^= kp[1]
        h[1] = (((rotl(h[1], 17, 32) + h[2]) & M32) * 5 + N32[1]) & M32
        h[2] ^= kp[2]
        h[2] = (((rotl(h[2], 15, 32) + h[3]) & M32) * 5 + N32[2]) & M32
        h[3] ^= kp[3]
        h[3] = (((rotl(h[3], 13, 32) + h[0]) & M32) * 5 + N32[3]) & M32
    tail = key[16 * nb:]
    t = len(tail)
    for j in (3, 2, 1, 0):          # the switch falls through from lane 4 down to lane 1
        if t > 4 * j:
            chunk = tail[4 * j:4 * j + 4]
            h[j] ^= pre32(j, from_le(chunk))
    for j in range(4):
        h[j] ^= n & M32
    h[0] = (h[0] + h[1] + h[2] + h[3]) & M32
    for j in (1, 2, 3):
        h[j] = (h[j] + h[0]) & M32
    h = [fmix32(x) for x in h]
    h[0] = (h[0] + h[1] + h[2] + h[3]) & M32
    for j in (1, 2, 3):
        h[j] = (h[j] + h[0]) & M32
    return b"".join(le(x, 4) for x in h)


register_model("Murmur3C", murmur3_x86_128)


# ---------------------------------------------------------------------------
# xxHash32 / xxHash64 (library version 0.8.4 as vendored in xxhash.h)
# ---------------------------------------------------------------------------
P32 = (0x9E3779B1, 0x85EBCA77, 0xC2B2AE3D, 0x27D4EB2F, 0x165667B1)
P64 = (0x9E3779B185EBCA87, 0xC2B2AE3D27D4EB4F, 0x165667B19E3779F9, 0x85EBCA77C2B2AE63, 0x27D4EB2F165667C5)


def xxh32_avalanche(h):
    h ^= h >> 15
    h = (h * P32[1]) & M32
    h ^= h >> 13
    h = (h * P32[2]) & M32
    h ^= h >> 16
    return h


def xxh32_avalanche_inv(h):
    h = unxorshr(h, 16, 32)
    h = (h * inv(P32[2], 32)) & M32
    h = unxorshr(h, 13, 32)
    h = (h * inv(P32[1], 32)) & M32
    h = unxorshr(h, 15, 32)
    return h


def xxh32_round(acc, inp):
    acc = (acc + inp * P32[1]) & M32
    acc = rotl(acc, 13, 32)
    return (acc * P32[0]) & M32


def xxhash32(key, seed=0):
    key = bytes(key)
    n = len(key)
    s = seed & M32
    p = 0
    if n >= 16:
        acc = [(s + P32[0] + P32[1]) & M32, (s + P32[1]) & M32, s, (s - P32[0]) & M32]
        while p + 16 <= n:       # do { ... } while (input < bEnd - 15)
            for j in range(4):
                acc[j] = xxh32_round(acc[j], from_le(key[p + 4 * j:p + 4 * j + 4]))
            p += 16
        h = (rotl(acc[0], 1, 32) + rotl(acc[1], 7, 32) + rotl(acc[2], 12, 32) + rotl(acc[3], 18, 32)) & M32
    else:
        h = (s + P32[4]) & M32
    h = (h + n) & M32
    r = (n - p) & 15
    while r >= 4:
        h = (h + from_le(key[p:p + 4]) * P32[2]) & M32
        h = (rotl(h, 17, 32) * P32[3]) & M32
        p += 4
        r -= 4
    while r > 0:
        h = (h + key[p] * P32[4]) & M32
        h = (rotl(h, 11, 32) * P32[0]) & M32
        p += 1
        r -= 1
    return xxh32_avalanche(h)


register_model("xxHash32", lambda key, seed: le(xxhash32(key, seed), 4))


def xxh64_avalanche(h):
    h ^= h >> 33
    h = (h * P64[1]) & M64
    h ^= h >> 29
    h = (h * P64[2]) & M64
    h ^= h >> 32
    return h


def xxh64_avalanche_inv(h):
    h = unxorshr(h, 32, 64)
    h = (h * inv(P64[2], 64)) & M64
    h = unxorshr(h, 29, 64)
    h = (h * inv(P64[1], 64)) & M64
    h = unxorshr(h, 33, 64)
    return h


def xxh64_round(acc, inp):
    acc = (acc + inp * P64[1]) & M64
    acc = rotl(acc, 31, 64)
    return (acc * P64[0]) & M64


def xxh64_round_inv(r):
    """x with xxh64_round(0, x) == r (the premix of a tail word)."""
    a = (r * inv(P64[0], 64)) & M64
    a = rotr(a, 31, 64)
    return (a * inv(P64[1], 64)) & M64


def xxh64_merge(acc, val):
    val = xxh64_round(0, val)
    acc ^= val
    return (acc * P64[0] + P64[3]) & M64


def xxhash64(key, seed=0):
    key = bytes(key)
    n = len(key)
    s = seed & M32          # the wrapper widens the uint32 seed
    p = 0
    if n >= 32:
        acc = [(s + P64[0] + P64[1]) & M64, (s + P64[1]) & M64, s, (s - P64[0]) & M64]
        while p + 32 <= n:
            for j in range(4):
                acc[j] = xxh64_round(acc[j], from_le(key[p + 8 * j:p + 8 * j + 8]))
            p += 32
        h = (rotl(acc[0], 1, 64) + rotl(acc[1], 7, 64) + rotl(acc[2], 12, 64) + rotl(acc[3], 18, 64)) & M64
        for j in range(4):
            h = xxh64_merge(h, acc[j])
    else:
        h = (s + P64[4]) & M64
    h = (h + n) & M64
    r = (n - p) & 31
    while r >= 8:
        h ^= xxh64_round(0, from_le(key[p:p + 8]))
        h = (rotl(h, 27, 64) * P64[0] + P64[3]) & M64
        p += 8
        r -= 8
    if r >= 4:
        h ^= (from_le(key[p:p + 4]) * P64[0]) & M64
        h = (rotl(h, 23, 64) * P64[1] + P64[2]) & M64
        p += 4
        r -= 4
    while r > 0:
        h ^= (key[p] * P64[4]) & M64
        h = (rotl(h, 11, 64) * P64[0]) & M64
        p += 1
        r -= 1
    return xxh64_avalanche(h)


register_model("xxHash64", lambda key, seed: le(xxhash64(key, seed), 8))


# ---------------------------------------------------------------------------
# Murmur3F  (x64_128)
# ---------------------------------------------------------------------------
def x64_128_final_inverse(out, n):
    """(h1, h2) before the finalisation (after the body and tail) for a digest `out` and key length n."""
    o1, o2 = from_le(out[:8]), from_le(out[8:16])
    f2 = (o2 - o1) & M64
    f1 = (o1 - f2) & M64
    a, b = fmix64_inv(f1), fmix64_inv(f2)       # a = h1 + h2, b = h2 + a  (after xor of n)
    h2 = (b - a) & M64
    h1 = (a - h2) & M64
    return h1 ^ (n & M64), h2 ^ (n & M64)


@attack("Murmur3F", "preimage", 128,
        method="invert fmix64 and the final adds, then solve one 16-byte block (k1', k2') for the state; "
               "premix inverted by the inverse multipliers")
def murmur3f_preimage(ctx):
    """Key length 16 (one block, no tail).  After one block the state is
        h1 = (rotl(s ^ k1', 27) + s) * 5 + c1,     h2 = (rotl(s ^ k2', 31) + h1) * 5 + c2,
    with k1' = premix(k1), k2' = premix(k2) bijections of 64 bits, and s the seed.  For ANY wanted
    (h1, h2) the first equation is solved for k1' (subtract s, rotr 27, xor s), then the second for k2'.
    So one 16-byte block reaches every internal state, and the finalisation (xor len, h1+=h2, h2+=h1,
    fmix64, h1+=h2, h2+=h1) is a bijection of the state -> every 128-bit digest has a 16-byte preimage,
    in about 40 word operations instead of 2^128 evaluations."""
    h1, h2 = x64_128_final_inverse(ctx.target, 16)
    s = ctx.seed & M32
    k2p = rotr(((h2 - N64_2) * INV5_64 - h1) & M64, 31, 64) ^ s
    k1p = rotr(((h1 - N64_1) * INV5_64 - s) & M64, 27, 64) ^ s
    key = le(pre64a_inv(k1p), 8) + le(pre64b_inv(k2p), 8)
    return Finding([key], [ctx.seed], 0, "closed form, 1 block of 16 bytes")


def x64_pair(seed_independent_len=24):
    """Two 24-byte keys with equal Murmur3 x64_128 hash for every seed. Returns (key_a, key_b)."""
    # block 1: k1' differs in bit 36, k2' in bit 32; tail (8 bytes): k1' differs in bit 63.
    ka1p, ka2p, kat = 0x0123456789ABCDEF, 0x0F1E2D3C4B5A6978, 0x1357924680ACEBDF
    kb1p, kb2p, kbt = ka1p ^ (1 << 36), ka2p ^ (1 << 32), kat ^ (1 << 63)
    a = le(pre64a_inv(ka1p), 8) + le(pre64b_inv(ka2p), 8) + le(pre64a_inv(kat), 8)
    b = le(pre64a_inv(kb1p), 8) + le(pre64b_inv(kb2p), 8) + le(pre64a_inv(kbt), 8)
    return a, b


@attack("Murmur3F", "seed-independent collision", 128,
        method="top-bit differential (Aumasson/Bernstein/Bosslet 2012 idea): block diff -> (2^63, 0), tail xor cancels it")
def murmur3f_seed_independent(ctx):
    """Keys of 24 bytes (one 16-byte block + 8-byte tail), valid for every 32-bit seed s (indeed any h1,h2).
    A difference of 2^63 is the same as an xor difference and an additive difference (no carry), and
    5 * 2^63 = 2^63 (mod 2^64).  Block: k1' ^= 2^36 flips bit 36 of h1 ^ k1', so bit 63 of
    rotl(.,27) flips, the sum with h2 changes by exactly 2^63 and h1_new = 5*(.)+c changes by 2^63.
    k2' ^= 2^32 flips bit 63 of rotl(h2 ^ k2', 31): a change of 2^63 that cancels the 2^63 carried
    in by h1_new, so h2_new is equal; state difference (2^63, 0) for every seed.  Tail: k1' ^= 2^63
    flips bit 63 of h1 ^ k1' exactly like the state difference and so h1 is equal again.  Same
    length, same state -> same digest.  Nothing depends on s, so brute force would need 2^64 (birthday)."""
    a, b = x64_pair()
    return Finding([a, b], [ctx.seed], 0, "closed form, 24-byte keys (16-byte block + 8-byte tail)")


@attack("Murmur3F", "collision", 128,
        method="full-width collision at the known seed via the seed-independent differential (24-byte keys)")
def murmur3f_collision(ctx):
    """Same pair of 24-byte keys as the seed-independent attack (it holds for every seed, hence for
    ctx.seed). Birthday brute force: 2^64 evaluations for a 128-bit digest."""
    a, b = x64_pair()
    return Finding([a, b], [ctx.seed], 0, "closed form, 24-byte keys")


# ---------------------------------------------------------------------------
# Murmur3C  (x86_128)
# ---------------------------------------------------------------------------
@attack("Murmur3C", "preimage", 128,
        method="invert the 4-lane finalisation and fmix32, then solve one 16-byte block (k1'..k4') lane by lane")
def murmur3c_preimage(ctx):
    """Key length 16 (one block).  Initial state h1=h2=h3=h4=s.  After one block
        h1 = (rotl(s^k1',19) + s)*5 + n1,  h2 = (rotl(s^k2',17) + s)*5 + n2,  h3 = (rotl(s^k3',15) + s)*5 + n3,
        h4 = (rotl(s^k4',13) + h1)*5 + n4
    so each lane is solved for its premixed k_i' (h4 after h1 is known), then the premix is inverted.
    The finalisation (xor len, h1+=h2+h3+h4, h_j+=h1, fmix32 per lane, again) is a bijection of the
    4 lanes, so every 128-bit digest has a 16-byte preimage; cost ~100 word operations vs 2^128."""
    n = 16
    s = ctx.seed & M32
    w = [from_le(ctx.target[4 * j:4 * j + 4]) for j in range(4)]
    # undo final: h1 += h2+h3+h4 ; h_j += h1 (j=1..3)
    h1 = w[0]
    h2, h3, h4 = [(w[j] - h1) & M32 for j in (1, 2, 3)]
    # h1 was h1 + h2 + h3 + h4 (the fmix outputs) after the first add: here h1 = f1 + f2 + f3 + f4 -> f1 = h1 - f2 - f3 - f4
    f2, f3, f4 = h2, h3, h4
    f1 = (h1 - f2 - f3 - f4) & M32
    g = [fmix32_inv(x) for x in (f1, f2, f3, f4)]     # g1 = H1+H2+H3+H4 ; g_j = H_j + g1
    g1 = g[0]
    H2, H3, H4 = [(g[j] - g1) & M32 for j in (1, 2, 3)]
    H1 = (g1 - H2 - H3 - H4) & M32
    H = [x ^ n for x in (H1, H2, H3, H4)]               # states after block (xor len undone)
    kp = [0] * 4
    kp[0] = rotr(((H[0] - N32[0]) * INV5_32 - s) & M32, 19, 32) ^ s
    kp[1] = rotr(((H[1] - N32[1]) * INV5_32 - s) & M32, 17, 32) ^ s
    kp[2] = rotr(((H[2] - N32[2]) * INV5_32 - s) & M32, 15, 32) ^ s
    kp[3] = rotr(((H[3] - N32[3]) * INV5_32 - H[0]) & M32, 13, 32) ^ s
    key = b"".join(le(pre32_inv(j, kp[j]), 4) for j in range(4))
    return Finding([key], [ctx.seed], 0, "closed form, 1 block of 16 bytes")


def x86_pair():
    """Two 20-byte keys with equal Murmur3 x86_128 hash for every seed. Returns (key_a, key_b)."""
    ka = [0x11223344, 0x55667788, 0x99AABBCC, 0xDDEEFF01]       # premixed block lanes k1'..k4'
    kt = 0x0BADF00D                                              # premixed tail k1'
    kb = [ka[0] ^ (1 << 12), ka[1], ka[2], ka[3] ^ (1 << 18)]
    ktb = kt ^ (1 << 31)
    a = b"".join(le(pre32_inv(j, ka[j]), 4) for j in range(4)) + le(pre32_inv(0, kt), 4)
    b = b"".join(le(pre32_inv(j, kb[j]), 4) for j in range(4)) + le(pre32_inv(0, ktb), 4)
    return a, b


@attack("Murmur3C", "seed-independent collision", 128,
        method="top-bit differential (Aumasson/Bernstein/Bosslet 2012 idea): block diff (2^31,0,0,0), tail xor cancels it")
def murmur3c_seed_independent(ctx):
    """Keys of 20 bytes (one 16-byte block + 4-byte tail), valid for every 32-bit seed.
    2^31 is both xor and additive difference and 5*2^31 = 2^31 (mod 2^32).  Block: k1' ^= 2^12
    flips bit 12 of h1^k1', so bit 31 of rotl(.,19) flips: h1 changes by exactly 2^31 (after + h2 and
    *5+c).  k4' ^= 2^18 flips bit 31 of rotl(h4^k4',13): +2^31, which cancels the 2^31 that h4 takes
    over from h1 (h4 = (rotl(..)+h1_new)*5+c).  h2, h3 are untouched, state difference (2^31,0,0,0)
    for every seed.  Tail (4 bytes, lane k1 only): k1' ^= 2^31 flips bit 31 of h1 ^ k1' like the
    state difference, h1 is equal again.  Same length, equal state -> equal digest. Birthday cost: 2^64."""
    a, b = x86_pair()
    return Finding([a, b], [ctx.seed], 0, "closed form, 20-byte keys (16-byte block + 4-byte tail)")


@attack("Murmur3C", "collision", 128,
        method="full-width collision at the known seed via the seed-independent differential (20-byte keys)")
def murmur3c_collision(ctx):
    """Same pair of 20-byte keys as the seed-independent attack (holds for every seed, so for ctx.seed).
    Birthday brute force: 2^64 evaluations for a 128-bit digest."""
    a, b = x86_pair()
    return Finding([a, b], [ctx.seed], 0, "closed form, 20-byte keys")


# ---------------------------------------------------------------------------
# xxHash32
# ---------------------------------------------------------------------------
@attack("xxHash32", "preimage", 32,
        method="invert avalanche, multiply, rotate; one 4-byte word solved in closed form")
def xxhash32_preimage(ctx):
    """Key length 4. For n=4 (<16 bytes) XXH32 computes
        h = seed + P5 + 4;  h += w*P3;  h = rotl(h,17)*P4;  avalanche(h)
    All of avalanche, *P4, rotl, +c, *P3 are bijections of 32 bits, so w is the unique solution:
    w = (rotr(avalanche^-1(t) * P4^-1, 17) - seed - P5 - 4) * P3^-1.  Every 32-bit target has such a
    4-byte key (length 4 is the shortest that reaches all 2^32 digests). Cost: a handful of operations,
    brute force would need 2^32 evaluations."""
    t = from_le(ctx.target)
    h = xxh32_avalanche_inv(t)
    h = (h * inv(P32[3], 32)) & M32
    h = rotr(h, 17, 32)
    h = (h - ((ctx.seed + P32[4] + 4) & M32)) & M32
    w = (h * inv(P32[2], 32)) & M32
    return Finding([le(w, 4)], [ctx.seed], 0, "closed form, 4-byte key")


@attack("xxHash32", "seed-independent collision", 32,
        method="no deterministic differential known: the seed is added (not xored), the rotation moves the top bit",
        expect="notfound")
def xxhash32_seed_independent(ctx):
    """Not found.  XXH32 injects words by ADDITION (h += w*P3, resp. acc += w*P2) before the rotation.
    A difference that is carry free (2^31) is moved by the rotation (17 resp. 13) to bit 16 resp. 12,
    where it is an addition or a subtraction depending on one seed-dependent bit; the multiplication then
    keeps that sign-dependent difference. A cancelling second word works for one sign only; measured with
    the Python model on 20000 random seeds, a two-word key pair built this way collides for 50.2 % of
    the seeds (the other sign: 49.8 %) - not seed independent. (xxHash64 injects tail words by XOR, which is why
    the top-bit differential works there.) A full-width collision at a known seed for a 32-bit hash is a
    birthday problem (2^16) and does not count."""
    return None


# ---------------------------------------------------------------------------
# xxHash64
# ---------------------------------------------------------------------------
@attack("xxHash64", "preimage", 64,
        method="invert avalanche, multiply-add, rotate and xor; one 8-byte word solved in closed form")
def xxhash64_preimage(ctx):
    """Key length 8. For n=8 (<32 bytes) XXH64 computes
        h = seed + P5 + 8;  h ^= round(0, w);  h = rotl(h,27)*P1 + P4;  avalanche(h)
    with round(0,w) = rotl(w*P2,31)*P1 a bijection.  Inverting each step gives w uniquely for every
    64-bit target. Cost: a handful of operations; brute force would need 2^64 evaluations."""
    t = from_le(ctx.target)
    h = xxh64_avalanche_inv(t)
    h = ((h - P64[3]) * inv(P64[0], 64)) & M64
    h = rotr(h, 27, 64)
    r = h ^ ((ctx.seed + P64[4] + 8) & M64)
    w = xxh64_round_inv(r)
    return Finding([le(w, 8)], [ctx.seed], 0, "closed form, 8-byte key")


def xxh64_pair():
    """Two 16-byte keys with equal XXH64 for every seed (and in fact for every initial h). (key_a, key_b)"""
    r1, r2 = 0x0123456789ABCDEF & ~(1 << 63), 0x1F2E3D4C5B6A7988 & ~(1 << 63)
    a = le(xxh64_round_inv(r1), 8) + le(xxh64_round_inv(r2), 8)
    b = le(xxh64_round_inv(r1 ^ (1 << 36)), 8) + le(xxh64_round_inv(r2 ^ (1 << 63)), 8)
    return a, b


@attack("xxHash64", "seed-independent collision", 64,
        method="top-bit differential: word 1 produces +2^63 in h, word 2 cancels it by an xor in bit 63")
def xxhash64_seed_independent(ctx):
    """Keys of 16 bytes (two 8-byte tail words, length < 32 so no lane accumulators), valid for EVERY seed.
    Tail words are injected by xor: h ^= r(w), r(w) = rotl(w*P2,31)*P1 (bijective), then
    h = rotl(h,27)*P1 + P4.  Word 1 with r(w1') = r(w1) ^ 2^36 flips bit 36 of h ^ r(w1), hence bit 63
    of rotl(.,27): a change of exactly 2^63 (add = xor = subtract at the top bit), and 2^63 * P1 = 2^63
    (P1 odd), so h1' differs by exactly 2^63 whatever the seed is.  Word 2 with r(w2') = r(w2) ^ 2^63
    flips bit 63 of h1 ^ r(w2) in the same way, so both keys reach the same h2.  Same length, same
    state -> same digest, independent of the seed (the argument never looks at h).  The idea is the top-bit
    differential of Aumasson/Bernstein/Bosslet 2012, applied here to xxHash64's xor injection - we did not
    find it in the literature for xxHash64, so we state it as our own application of a known technique.
    Brute force: 2^32 evaluations by birthday; for a seed independent pair that is not available at all."""
    a, b = xxh64_pair()
    return Finding([a, b], [ctx.seed], 0, "closed form, 16-byte keys")


@attack("xxHash64", "collision", 64,
        method="full-width collision at the known seed via the seed-independent differential (16-byte keys)")
def xxhash64_collision(ctx):
    """Same pair of 16-byte keys as the seed-independent attack (holds for every seed, so for ctx.seed).
    Birthday brute force: ~2^32 evaluations for a 64-bit digest."""
    a, b = xxh64_pair()
    return Finding([a, b], [ctx.seed], 0, "closed form, 16-byte keys")
