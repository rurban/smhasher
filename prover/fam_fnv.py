"""FNV-1a, 32 and 64 bit (SMHasher names FNV1a and FNV64).

    h = seed ^ offset_basis;  for each byte b:  h = (h ^ b) * prime   (mod 2^bits)

Known results: FNV is not collision resistant by design; pre-images by meet in the middle
and lattice reduction are folklore (e.g. Fowler-Noll-Vo hash page, "FNV-1a collisions"
posts, Aumasson/Bernstein 2012 for hash flooding). Nothing here is new."""
import math

from .core import attack, Finding, register_model
from .bv import mask, inv

P32, B32 = 16777619, 0x811C9DC5


def fnv1a32(key, seed=0):
    h = (seed ^ B32) & mask(32)
    for b in key:
        h = ((h ^ b) * P32) & mask(32)
    return h


def le32(h):
    return h.to_bytes(4, "little")


register_model("FNV1a", lambda key, seed: le32(fnv1a32(key, seed)))


@attack("FNV1a", "preimage", 32, method="meet in the middle: 2 free bytes forward, 2 free bytes backward")
def fnv1a32_preimage(ctx):
    """Key = prefix byte + 4 free bytes. Forward from the seed state over (prefix, b1, b2) gives 2^16
    states, backward from the target through the inverse prime over (b4, b3) gives 2^16 states; a
    state in both is a key. Expected 1 hit per prefix byte, so a few prefixes are tried: ~2^17 ops
    per prefix instead of 2^32 hash evaluations."""
    target = int.from_bytes(ctx.target, "little")
    pinv = inv(P32, 32)
    h0 = (ctx.seed ^ B32) & mask(32)
    back = {}
    for b4 in range(256):
        h3 = ((target * pinv) & mask(32)) ^ b4
        for b3 in range(256):
            back.setdefault(((h3 * pinv) & mask(32)) ^ b3, (b3, b4))
    for tries, p in enumerate(range(256), 1):
        ctx.check()
        hp = ((h0 ^ p) * P32) & mask(32)
        for b1 in range(256):
            x1 = ((hp ^ b1) * P32) & mask(32)
            for b2 in range(256):
                h2 = ((x1 ^ b2) * P32) & mask(32)
                hit = back.get(h2)
                if hit:
                    key = bytes([p, b1, b2, hit[0], hit[1]])
                    return Finding([key], [ctx.seed], 17 + math.log2(tries),
                                   "2^16-entry table, %d prefix byte(s) tried" % tries)
    return None
