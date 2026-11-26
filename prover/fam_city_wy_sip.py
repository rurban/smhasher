"""CityHash64WithSeed (City64), wyhash v4.2 and SipHash-2-4 (SMHasher names City64, wyhash, SipHash;
extras: City64noSeed, City64low).

Everything here rebuilds known ideas, nothing is new:
  * CityHash64: the seed only enters in the last step, CityHash64WithSeed(s, seed) = Hash128to64(CityHash64(s) - k2, seed),
    so any two keys with the same UNSEEDED CityHash64 collide for every seed. Seed-independent multicollisions for
    CityHash64 are in Aumasson, Bernstein, Boßlet, "Hash-flooding DoS reloaded: attacks and defenses" (2012). For keys of
    32 bytes (the 17..32 byte path, HashLen17to32) the 128-bit state (u, v) that is fed to the final mixer can be
    chosen freely, and the mixer is built from bijective steps (odd multiplications, xor-shift), so it can be run
    backwards: a pre-image needs a handful of 64-bit operations.
  * wyhash v4.2: the "blind multiplication" case is KNOWN and documented by the wyhash author (wyhash.h, comment at
    WYHASH_CONDOM: "extra protection against entropy loss (probability=2^-63), aka. blind multiplication", i.e. one
    operand of the 64x64->128 bit product is zero and its result loses the other operand). When
    a ^ secret[1] == 0 the 128-bit product is 0 and the digest no longer depends on the other half of the key nor on the
    seed. The collision hangs on the secret that SMHasher uses: the default secret _wyp, secret[1] = 0x8bb84b93962eacc9
    (Hashes.h calls wyhash(key, len, seed, _wyp)); with another secret the colliding key prefix is another one.
    What WYHASH_CONDOM changes: SMHasher builds with the default WYHASH_CONDOM=1, where _wymum() ASSIGNS the product
    halves (*A = lo, *B = hi), so a zero operand wipes out the other operand. With WYHASH_CONDOM=2 it does *A ^= lo,
    *B ^= hi, i.e. the inputs stay mixed into the result and the product 0 loses nothing; this collision then disappears
    (digests change too, so the VerifyAll constant would differ). We rebuild a documented weakness, we do not discover it.
  * City64noSeed and City64low are separate registry entries of SMHasher. They are listed as rows of their own and say
    "same construction as City64": same algebra, same keys, no additional finds.
  * SipHash-2-4 is a keyed PRF; with a 32-bit seed as key material nothing is expected to fall. Attempts go through the
    generic z3 fall-back (prover/smt.py); when z3 stops at the time limit without an answer the row says
    "not found (2 min)". The wyhash pre-image gets one z3 attempt as well and nothing more.
"""
import math

from .core import attack, Finding, register_model
from .bv import mask, inv, unxorshr, rotr
from . import city_wy_sip_models as mod
from .city_wy_sip_models import (M64, K1, K2, KMUL, WYP, le64, wymix, city64, hash128to64,
                                 city64_model, city64_nos_model, city64_low_model, wyhash_model, sip_model)
from . import smt

register_model("City64", city64_model)
register_model("City64noSeed", city64_nos_model)
register_model("City64low", city64_low_model)
register_model("wyhash", wyhash_model)
register_model("SipHash", sip_model)

KMUL_INV = inv(KMUL, 64)


# =============================================================================== CityHash64
def invert_hash128to64(t, v):
    """u with Hash128to64(u, v) == t, for any v (every step is invertible: odd multiplier, xor-shift by 47)."""
    b = unxorshr((t * KMUL_INV) & M64, 47, 64)        # b = (v ^ a) * kMul
    a = ((b * KMUL_INV) & M64) ^ v                    # a = (a1 ^ a1 >> 47)
    a1 = unxorshr(a, 47, 64)                          # a1 = (u ^ v) * kMul
    return ((a1 * KMUL_INV) & M64) ^ v


def invert_hashlen16(t, v, mul):
    """u with HashLen16(u, v, mul) == t, for odd mul."""
    mi = inv(mul, 64)
    b = unxorshr((t * mi) & M64, 47, 64)
    a = ((b * mi) & M64) ^ v
    a1 = unxorshr(a, 47, 64)
    return ((a1 * mi) & M64) ^ v


MUL32 = (K2 + 32 * 2) & M64                           # HashLen17to32 multiplier for len == 32


def key32_for_state(u, v, b, c):
    """A 32-byte key whose HashLen17to32 final mixer input is exactly (u, v).
    With words w0..w3 (little endian): a = w0*k1, b = w1, c = w3*mul, d = w2*k2,
        u = rot(a+b,43) + rot(c,30) + d,   v = a + rot(b+k2,18) + c.
    b and c are free; a follows from v, then d from u. All steps are invertible (k1, k2, mul are odd)."""
    a = (v - rotr((b + K2) & M64, 18, 64) - c) & M64
    d = (u - rotr((a + b) & M64, 43, 64) - rotr(c, 30, 64)) & M64
    w0, w1, w2, w3 = (a * inv(K1, 64)) & M64, b, (d * inv(K2, 64)) & M64, (c * inv(MUL32, 64)) & M64
    return le64(w0) + le64(w1) + le64(w2) + le64(w3)


def city_key_with_unseeded_hash(x, free=0):
    """32-byte key k with CityHash64(k) == x (unseeded). `free` picks one of 2^64 solutions."""
    v = (0x243F6A8885A308D3 + free * 0x9E3779B97F4A7C15) & M64       # free choice of v; u follows
    u = invert_hashlen16(x, v, MUL32)
    return key32_for_state(u, v, 0x13198A2E03707344 ^ free, 0xA4093822299F31D0 + free)


def _target64(ctx):
    return int.from_bytes(ctx.target[:8], "little")


@attack("City64", "preimage", 64, method="run Hash128to64 and HashLen17to32 backwards; 32-byte key, algebraic")
def city64_preimage(ctx):
    """CityHash64WithSeed = Hash128to64(CityHash64(key) - k2, seed). Invert Hash128to64 for the known seed to get the
    unseeded value x = CityHash64(key); then pick the 32-byte key (17..32 byte path, 4 free 64-bit words) with
    CityHash64(key) == x as above. ~20 64-bit operations; brute force is 2^64 evaluations."""
    u = invert_hash128to64(_target64(ctx), ctx.seed & M64)
    key = city_key_with_unseeded_hash((u + K2) & M64)
    assert city64(key) == (u + K2) & M64
    return Finding([key], [ctx.seed], 4, "algebraic inversion, 32-byte key, no search")


def _city_collision_keys():
    """Two different 32-byte keys that reach the same final-mixer input (u, v) => same CityHash64."""
    u, v = 0x452821E638D01377, 0xBE5466CF34E90C6C
    return [key32_for_state(u, v, b, c) for b, c in ((0, 0), (1, 0x0000000100000000))]


@attack("City64", "seed-independent collision", 64,
        method="same unseeded CityHash64 (state (u,v) of HashLen17to32 chosen), 2 keys of 32 bytes")
def city64_sic(ctx):
    """Seed enters only as Hash128to64(CityHash64(s) - k2, seed): equal CityHash64 => equal digest for ALL 2^32 seeds.
    The two 32-byte keys below differ in words w0, w1, w2, w3 but produce the same mixer input (u, v)."""
    ks = _city_collision_keys()
    assert ks[0] != ks[1] and city64(ks[0]) == city64(ks[1])
    return Finding(ks, [0], 4, "algebraic, same internal state (u,v); equal for every seed")


@attack("City64", "collision", 64, method="same unseeded CityHash64, 2 keys of 32 bytes (works for every seed)")
def city64_collision(ctx):
    ks = _city_collision_keys()
    return Finding(ks, [ctx.seed], 4, "algebraic, same internal state (u,v); equal for every seed")


# ---- extra table lines: City64noSeed and the 32-bit City64low use the SAME construction and the SAME keys as City64;
# they are separate registry entries of SMHasher, not additional findings.
@attack("City64noSeed", "preimage", 64, method="same construction as City64 (extra line, no new find): run HashLen17to32 backwards; 32-byte key")
def city64noseed_preimage(ctx):
    x = _target64(ctx)
    key = city_key_with_unseeded_hash(x)
    assert city64(key) == x
    return Finding([key], [ctx.seed], 4, "same construction as City64 (extra line): algebraic inversion, 32-byte key")


@attack("City64noSeed", "collision", 64, method="same construction as City64 (extra line, no new find): same state (u,v) of HashLen17to32, 2 keys of 32 bytes")
def city64noseed_collision(ctx):
    return Finding(_city_collision_keys(), [ctx.seed], 4, "same construction as City64 (extra line): same state (u,v), 32-byte keys")


@attack("City64low", "preimage", 32, method="same construction as City64 (extra line, no new find): run Hash128to64 and HashLen17to32 backwards, high 32 bits 0")
def city64low_preimage(ctx):
    t = int.from_bytes(ctx.target[:4], "little")
    u = invert_hash128to64(t, ctx.seed & M64)
    key = city_key_with_unseeded_hash((u + K2) & M64)
    return Finding([key], [ctx.seed], 4, "same construction as City64 (extra line): algebraic inversion, 32-byte key")


@attack("City64low", "seed-independent collision", 32,
        method="same construction as City64 (extra line, no new find): same unseeded CityHash64 => same low 32 bits for every seed, 2 keys of 32 bytes")
def city64low_sic(ctx):
    return Finding(_city_collision_keys(), [0], 4, "same construction as City64 (extra line): same state (u,v); equal for every seed")


# =============================================================================== wyhash v4.2
S0, S1 = WYP[0], WYP[1]


def _wy_zero_key(last):
    """9-byte key whose first 8 bytes are chosen so that a = (r4(p)<<32)|r4(p+4) == secret[1]; byte 8 is `last`."""
    return (S1 >> 32).to_bytes(4, "little") + (S1 & 0xFFFFFFFF).to_bytes(4, "little") + bytes([last])


def _wy_pair():
    return [_wy_zero_key(0), _wy_zero_key(1)]


@attack("wyhash", "seed-independent collision", 64,
        method="author-documented 'blind multiplication' (2^-63) with SMHasher's default secret: a ^ secret[1] == 0 makes the product 0; two 9-byte keys differing in the last byte; gone with WYHASH_CONDOM=2")
def wyhash_sic(ctx):
    """wyhash v4.2, 4 <= len <= 16: a = (r4(p)<<32)|r4(p+((len>>3)<<2)), b = (r4(p+len-4)<<32)|r4(p+len-4-((len>>3)<<2));
    _wymum(a ^ secret[1], b ^ seed') with seed' = seed ^ _wymix(seed ^ secret[0], secret[1]).
    If a == secret[1] the 128-bit product is 0 whatever b and seed' are, and the digest is _wymix(secret[0] ^ len, secret[1]),
    a constant that depends on the length only. For len == 9, a = bytes 0..7 and b depends on bytes 1..4 and 5..8,
    so the 9th byte is a free byte that only changes b: all 256 keys with a == secret[1] collide for EVERY seed
    (the author documents this "blind multiplication" in wyhash.h and gives its probability as 2^-63 for random inputs, a product with one zero operand; the pair below is constructed, not searched for. It exists only with SMHasher's default secret _wyp and WYHASH_CONDOM=1, where _wymum assigns the product halves; with WYHASH_CONDOM=2 they are xor-ed into the inputs, b stays in the result, and this pair no longer collides.)
    Shortest length: for len 4..7 a = (w << 32) | w cannot be secret[1]; for len == 8 b is a function of a
    (single key); for len 1..3 a has 24 bits."""
    ks = _wy_pair()
    for sd in (0, 1, 0xFFFFFFFF):
        assert mod.wyhash(ks[0], sd) == mod.wyhash(ks[1], sd)
    return Finding(ks, [0], 2, "known blind multiplication (documented by the author), default secret: a ^ secret[1] == 0 => product 0, 9-byte keys")


@attack("wyhash", "collision", 64,
        method="author-documented 'blind multiplication' (2^-63), SMHasher's default secret (a ^ secret[1] == 0), two 9-byte keys differing in the last byte; same for every seed; gone with WYHASH_CONDOM=2")
def wyhash_collision(ctx):
    ks = _wy_pair()
    assert mod.wyhash(ks[0], ctx.seed) == mod.wyhash(ks[1], ctx.seed)
    return Finding(ks, [ctx.seed], 2, "known blind multiplication (documented by the author), default secret: a ^ secret[1] == 0 => product 0, 9-byte keys")


def wy_bv(key, seed):
    """wyhash v4.2 on smt.BV values (same code runs on ints and on z3 bit-vectors)."""
    BV = smt.BV
    n = key.n

    def mix(a, b):
        lo, hi = a.mulwide(b)
        return lo ^ hi

    s = seed.zext(64)
    s = s ^ mix(s ^ S0, BV(S1))

    def r8(o): return key.u(o, 8)
    def r4(o): return key.u(o, 4).zext(64)

    if n <= 16:
        if n >= 4:
            q = (n >> 3) << 2
            a = (r4(0) << 32) | r4(q)
            b = (r4(n - 4) << 32) | r4(n - 4 - q)
        elif n > 0:
            a = (key.bytes[0].zext(64) << 16) | (key.bytes[n >> 1].zext(64) << 8) | key.bytes[n - 1].zext(64)
            b = BV(0)
        else:
            a = b = BV(0)
    else:
        i, o = n, 0
        if i >= 48:
            see1 = see2 = s
            while True:
                s = mix(r8(o) ^ WYP[1], r8(o + 8) ^ s)
                see1 = mix(r8(o + 16) ^ WYP[2], r8(o + 24) ^ see1)
                see2 = mix(r8(o + 32) ^ WYP[3], r8(o + 40) ^ see2)
                o += 48
                i -= 48
                if i < 48:
                    break
            s = s ^ see1 ^ see2
        while i > 16:
            s = mix(r8(o) ^ WYP[1], r8(o + 8) ^ s)
            i -= 16
            o += 16
        a, b = r8(o + i - 16), r8(o + i - 8)
    a = a ^ S1
    b = b ^ s
    lo, hi = a.mulwide(b)
    return mix(lo ^ (S0 ^ n), hi ^ S1)


@attack("wyhash", "preimage", 64, method="z3 SMT on a 16-byte key, second word pinned so that the first product is a * 1 "
        "(one attempt, no further tries; expected: not found (2 min))", expect="notfound", fast=False)
def wyhash_preimage(ctx):
    """wyhash ends in digest = wymix(lo ^ secret[0] ^ len, hi ^ secret[1]) with (lo, hi) = (a ^ secret[1]) * (b ^ seed') as a
    128-bit product. For a 16-byte key a and b are two free 64-bit words. Every algebraic route we found turns into
    factoring a 128-bit number or into a 64-item modular knapsack (density 1); there is no cheap inverse. Best shape
    for a solver: pin b ^ seed' == 1, so hi == 0 and digest = wymix(a ^ secret[0] ^ len ^ secret[1], secret[1]) is a
    function of one 64-bit word through one multiplication by a constant. z3 gets ctx.left(); we do not expect it to
    finish, and the result is reported as 'not found (2 min)' unless it does."""
    sd = ctx.seed & 0xFFFFFFFF
    seed1 = (sd ^ wymix(sd ^ S0, S1)) & M64
    b = seed1 ^ 1
    pin = lambda key: [key.u(8, 4).eq(b & 0xFFFFFFFF), key.u(12, 4).eq(b >> 32)]
    return smt.solve_preimage(wy_bv, ctx.target, 16, ctx=ctx, seed=ctx.seed, bits=64, constraints=pin,
                              work="z3 on 16-byte key with b ^ seed' pinned to 1", raise_timeout=True)


# =============================================================================== SipHash-2-4
def sip_bv(key, seed):
    """SipHash-2-4 with key = seed (32 bit, little endian) + 12 zero bytes, on smt.BV values."""
    BV = smt.BV
    k0, k1 = seed.zext(64), BV(0)
    v0, v1 = k0 ^ 0x736F6D6570736575, k1 ^ 0x646F72616E646F6D
    v2, v3 = k0 ^ 0x6C7967656E657261, k1 ^ 0x7465646279746573

    def rnd(v0, v1, v2, v3):
        v0 = v0 + v1
        v1 = v1.rotl(13) ^ v0
        v0 = v0.rotl(32)
        v2 = v2 + v3
        v3 = v3.rotl(16) ^ v2
        v0 = v0 + v3
        v3 = v3.rotl(21) ^ v0
        v2 = v2 + v1
        v1 = v1.rotl(17) ^ v2
        v2 = v2.rotl(32)
        return v0, v1, v2, v3

    n = key.n
    end = n - n % 8
    for o in range(0, end, 8):
        w = key.u(o, 8)
        v3 = v3 ^ w
        v0, v1, v2, v3 = rnd(*rnd(v0, v1, v2, v3))
        v0 = v0 ^ w
    last = BV((n & 0xFF) << 56)
    if n > end:
        last = last | key.u(end, n - end).zext(64)
    v3 = v3 ^ last
    v0, v1, v2, v3 = rnd(*rnd(v0, v1, v2, v3))
    v0 = v0 ^ last
    v2 = v2 ^ 0xFF
    for _ in range(4):
        v0, v1, v2, v3 = rnd(v0, v1, v2, v3)
    return v0 ^ v1 ^ v2 ^ v3


@attack("SipHash", "preimage", 64, method="z3 SMT on an 8-byte key (expected: not found, SipHash-2-4 is a PRF)",
        expect="notfound", fast=False)
def siphash_preimage(ctx):
    """Known seed = known 128-bit key, 8-byte message: 8 SipRounds of add/rotate/xor and a 64-bit unknown; for a PRF
    this is a 2^64 problem. We only try the generic solver; anything stronger than 'not found (2 min)' is not claimed."""
    return smt.solve_preimage(sip_bv, ctx.target, 8, ctx=ctx, seed=ctx.seed, bits=64, work="z3 on 8-byte key", raise_timeout=True)


@attack("SipHash", "collision", 64, method="z3 SMT, two 8-byte keys at the known seed (expected: not found; birthday 2^32 not claimed)",
        expect="notfound", fast=False)
def siphash_collision(ctx):
    return smt.solve_collision(sip_bv, 8, ctx=ctx, seeds=(ctx.seed,), work="z3, two 8-byte keys", raise_timeout=True)


@attack("SipHash", "seed-independent collision", 64, method="z3 SMT, two 8-byte keys equal at 3 seeds (expected: not found)",
        expect="notfound", fast=False)
def siphash_sic(ctx):
    return smt.solve_collision(sip_bv, 8, ctx=ctx, seeds=(0, 1, 0x80000000), work="z3, two 8-byte keys, 3 seeds", raise_timeout=True)
