"""Pure Python reference models of CityHash64 (v1.1), wyhash v4.2 and SipHash-2-4 exactly as SMHasher wraps them.

Kept apart from the attacks so that they can be checked on their own:

    python -I -X utf8 -c "from prover.city_wy_sip_models import check; check()"

prints the VerifyAll value of each model next to the constant of main.cpp
(City64 0x5FABC5C5, City64noSeed 0x4C4E54B1, City64low 0x6C4EF416, wyhash 0x9DAE7DD3, SipHash 0xC58D7F9C).
All values are Python ints, every operation is reduced to 64 bit by hand."""
from .bv import mask, rotr, rotl, inv, unxorshr

M64 = mask(64)
K0, K1, K2 = 0xC3A5C85C97CB3127, 0xB492B66FBE98F273, 0x9AE16A3B2F90404F
KMUL = 0x9DDFEA08EB382D69


def f64(b, o=0):
    return int.from_bytes(b[o:o + 8], "little")


def f32(b, o=0):
    return int.from_bytes(b[o:o + 4], "little")


def bswap64(x):
    return int.from_bytes((x & M64).to_bytes(8, "little"), "big")


def shiftmix(v):
    return v ^ (v >> 47)


# ------------------------------------------------------------------ CityHash64 v1.1
def hash128to64(u, v):
    a = ((u ^ v) * KMUL) & M64
    a ^= a >> 47
    b = ((v ^ a) * KMUL) & M64
    b ^= b >> 47
    return (b * KMUL) & M64


def hashlen16(u, v, mul):
    a = ((u ^ v) * mul) & M64
    a ^= a >> 47
    b = ((v ^ a) * mul) & M64
    b ^= b >> 47
    return (b * mul) & M64


def hashlen0to16(s):
    n = len(s)
    if n >= 8:
        mul = (K2 + n * 2) & M64
        a = (f64(s) + K2) & M64
        b = f64(s, n - 8)
        c = (rotr(b, 37, 64) * mul + a) & M64
        d = ((rotr(a, 25, 64) + b) * mul) & M64
        return hashlen16(c, d, mul)
    if n >= 4:
        mul = (K2 + n * 2) & M64
        a = f32(s)
        return hashlen16((n + (a << 3)) & M64, f32(s, n - 4), mul)
    if n > 0:
        a, b, c = s[0], s[n >> 1], s[n - 1]
        y = a + (b << 8)
        z = n + (c << 2)
        return (shiftmix(((y * K2) & M64) ^ ((z * K0) & M64)) * K2) & M64
    return K2


def hashlen17to32(s):
    n = len(s)
    mul = (K2 + n * 2) & M64
    a = (f64(s) * K1) & M64
    b = f64(s, 8)
    c = (f64(s, n - 8) * mul) & M64
    d = (f64(s, n - 16) * K2) & M64
    return hashlen16((rotr((a + b) & M64, 43, 64) + rotr(c, 30, 64) + d) & M64,
                     (a + rotr((b + K2) & M64, 18, 64) + c) & M64, mul)


def weak32(w, x, y, z, a, b):
    a = (a + w) & M64
    b = rotr((b + a + z) & M64, 21, 64)
    c = a
    a = (a + x + y) & M64
    b = (b + rotr(a, 44, 64)) & M64
    return (a + z) & M64, (b + c) & M64


def weak32s(s, o, a, b):
    return weak32(f64(s, o), f64(s, o + 8), f64(s, o + 16), f64(s, o + 24), a, b)


def hashlen33to64(s):
    n = len(s)
    mul = (K2 + n * 2) & M64
    a = (f64(s) * K2) & M64
    b = f64(s, 8)
    c = f64(s, n - 24)
    d = f64(s, n - 32)
    e = (f64(s, 16) * K2) & M64
    f = (f64(s, 24) * 9) & M64
    g = f64(s, n - 8)
    h = (f64(s, n - 16) * mul) & M64
    u = (rotr((a + g) & M64, 43, 64) + (rotr(b, 30, 64) + c) * 9) & M64
    v = ((((a + g) & M64) ^ d) + f + 1) & M64
    w = (bswap64(((u + v) * mul) & M64) + h) & M64
    x = (rotr((e + f) & M64, 42, 64) + c) & M64
    y = ((bswap64(((v + w) * mul) & M64) + g) * mul) & M64
    z = (e + f + c) & M64
    a = (bswap64(((x + z) * mul + y) & M64) + b) & M64
    b = (shiftmix(((z + a) * mul + d + h) & M64) * mul) & M64
    return (b + x) & M64


def city64(s):
    """CityHash64 (unseeded) of the bytes s."""
    n = len(s)
    if n <= 32:
        return hashlen0to16(s) if n <= 16 else hashlen17to32(s)
    if n <= 64:
        return hashlen33to64(s)
    x = f64(s, n - 40)
    y = (f64(s, n - 16) + f64(s, n - 56)) & M64
    z = hash128to64((f64(s, n - 48) + n) & M64, f64(s, n - 24))
    v = weak32s(s, n - 64, n, z)
    w = weak32s(s, n - 32, (y + K1) & M64, x)
    x = (x * K1 + f64(s)) & M64
    left = (n - 1) & ~63
    o = 0
    while True:
        x = (rotr((x + y + v[0] + f64(s, o + 8)) & M64, 37, 64) * K1) & M64
        y = (rotr((y + v[1] + f64(s, o + 48)) & M64, 42, 64) * K1) & M64
        x ^= w[1]
        y = (y + v[0] + f64(s, o + 40)) & M64
        z = (rotr((z + w[0]) & M64, 33, 64) * K1) & M64
        v = weak32s(s, o, (v[1] * K1) & M64, (x + w[0]) & M64)
        w = weak32s(s, o + 32, (z + w[1]) & M64, (y + f64(s, o + 16)) & M64)
        z, x = x, z
        o += 64
        left -= 64
        if left == 0:
            break
    return hash128to64((hash128to64(v[0], w[0]) + (shiftmix(y) * K1) + z) & M64,
                       (hash128to64(v[1], w[1]) + x) & M64)


def city64_seeded(s, seed):
    """CityHash64WithSeed(s, len, seed) = Hash128to64(CityHash64(s) - k2, seed)."""
    return hash128to64((city64(s) - K2) & M64, seed & M64)


def le64(x):
    return (x & M64).to_bytes(8, "little")


def city64_model(key, seed):
    return le64(city64_seeded(key, seed))


def city64_nos_model(key, seed):
    return le64(city64(key))


def city64_low_model(key, seed):
    return le64(city64_seeded(key, seed))[:4]


# ------------------------------------------------------------------ wyhash v4.2 (64 bit, default secret)
WYP = (0x2D358DCCAA6C78A5, 0x8BB84B93962EACC9, 0x4B33A62ED433D4A3, 0x4D5A2DA51DE1AA47)


def wymum(a, b):
    r = a * b
    return r & M64, r >> 64


def wymix(a, b):
    lo, hi = wymum(a, b)
    return lo ^ hi


def wyr3(p, k):
    return (p[0] << 16) | (p[k >> 1] << 8) | p[k - 1]


def wyhash(p, seed, secret=WYP):
    n = len(p)
    seed = (seed ^ wymix(seed ^ secret[0], secret[1])) & M64
    if n <= 16:
        if n >= 4:
            q = (n >> 3) << 2
            a = (f32(p, 0) << 32) | f32(p, q)
            b = (f32(p, n - 4) << 32) | f32(p, n - 4 - q)
        elif n > 0:
            a, b = wyr3(p, n), 0
        else:
            a = b = 0
    else:
        i, o = n, 0
        if i >= 48:
            see1 = see2 = seed
            while True:
                seed = wymix(f64(p, o) ^ secret[1], f64(p, o + 8) ^ seed)
                see1 = wymix(f64(p, o + 16) ^ secret[2], f64(p, o + 24) ^ see1)
                see2 = wymix(f64(p, o + 32) ^ secret[3], f64(p, o + 40) ^ see2)
                o += 48
                i -= 48
                if i < 48:
                    break
            seed ^= see1 ^ see2
        while i > 16:
            seed = wymix(f64(p, o) ^ secret[1], f64(p, o + 8) ^ seed)
            i -= 16
            o += 16
        a = f64(p, o + i - 16)
        b = f64(p, o + i - 8)
    a ^= secret[1]
    b ^= seed
    a, b = wymum(a, b)
    return wymix(a ^ secret[0] ^ n, b ^ secret[1])


def wyhash_model(key, seed):
    return le64(wyhash(key, seed & 0xFFFFFFFF))


# ------------------------------------------------------------------ SipHash-2-4, key = seed (LE, 4 bytes) + 12 zero bytes
def sipround(v0, v1, v2, v3):
    v0 = (v0 + v1) & M64
    v1 = rotl(v1, 13, 64) ^ v0
    v0 = rotl(v0, 32, 64)
    v2 = (v2 + v3) & M64
    v3 = rotl(v3, 16, 64) ^ v2
    v0 = (v0 + v3) & M64
    v3 = rotl(v3, 21, 64) ^ v0
    v2 = (v2 + v1) & M64
    v1 = rotl(v1, 17, 64) ^ v2
    v2 = rotl(v2, 32, 64)
    return v0, v1, v2, v3


def siphash24(key16, m, crounds=2, drounds=4):
    k0, k1 = f64(key16, 0), f64(key16, 8)
    v0 = k0 ^ 0x736F6D6570736575
    v1 = k1 ^ 0x646F72616E646F6D
    v2 = k0 ^ 0x6C7967656E657261
    v3 = k1 ^ 0x7465646279746573
    n = len(m)
    end = n - (n % 8)
    for o in range(0, end, 8):
        w = f64(m, o)
        v3 ^= w
        for _ in range(crounds):
            v0, v1, v2, v3 = sipround(v0, v1, v2, v3)
        v0 ^= w
    last = (n & 0xFF) << 56
    for j, c in enumerate(m[end:]):
        last |= c << (8 * j)
    v3 ^= last
    for _ in range(crounds):
        v0, v1, v2, v3 = sipround(v0, v1, v2, v3)
    v0 ^= last
    v2 ^= 0xFF
    for _ in range(drounds):
        v0, v1, v2, v3 = sipround(v0, v1, v2, v3)
    return v0 ^ v1 ^ v2 ^ v3


def sip_model(key, seed):
    if not key:
        return bytes(8)            # wrapper stores only a 32-bit zero; the rest of the buffer stays as the caller left it
    return le64(siphash24((seed & 0xFFFFFFFF).to_bytes(4, "little") + bytes(12), bytes(key)))


EXPECT = {"City64": (city64_model, 64, 0x5FABC5C5), "City64noSeed": (city64_nos_model, 64, 0x4C4E54B1),
          "City64low": (city64_low_model, 32, 0x6C4EF416), "wyhash": (wyhash_model, 64, 0x9DAE7DD3),
          "SipHash": (sip_model, 64, 0xC58D7F9C)}


def check():
    from .bv import verification_code
    ok = True
    for name, (fn, bits, want) in EXPECT.items():
        got = verification_code(fn, bits)
        print("%-13s verification %#010x expected %#010x %s" % (name, got, want, "OK" if got == want else "MISMATCH"))
        ok &= got == want
    return ok
