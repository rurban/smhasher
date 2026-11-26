"""Small fixed-width integer helpers shared by the attacks (all values are Python ints)."""


def mask(bits):
    return (1 << bits) - 1


def inv(a, bits):
    """Multiplicative inverse of an odd constant modulo 2^bits."""
    return pow(a, -1, 1 << bits)


def rotl(x, r, bits):
    r %= bits
    return ((x << r) | (x >> (bits - r))) & mask(bits)


def rotr(x, r, bits):
    return rotl(x, bits - (r % bits), bits)


def unxorshr(y, s, bits):
    """x with y = x ^ (x >> s)."""
    x = y
    for _ in range(bits // s + 1):
        x = y ^ (x >> s)
    return x & mask(bits)


def unxorshl(y, s, bits):
    """x with y = x ^ (x << s) (mod 2^bits)."""
    x = y
    for _ in range(bits // s + 1):
        x = (y ^ (x << s)) & mask(bits)
    return x


def le(x, nbytes):
    return (x & mask(8 * nbytes)).to_bytes(nbytes, "little")


def from_le(b):
    return int.from_bytes(b, "little")


def verification_code(model, bits):
    """SMHasher's VerifyAll value of a Python model fn(key, seed) -> digest bytes: hashes of the keys
    {0}, {0,1}, ... with seed 256-len, then the hash of all digests; first 4 bytes, little endian.
    Compare with the constant in the registry table of main.cpp (e.g. 0x27864C1E for Murmur2)."""
    hb = bits // 8
    key = bytes(range(256))
    digests = b"".join(model(key[:i], 256 - i) for i in range(256))
    return int.from_bytes(model(digests, 0)[:4], "little")
