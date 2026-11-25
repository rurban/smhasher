"""FNV-1a 64 bit (SMHasher name FNV64), FNV-1a 32 bit (seed-independent attempt only) and the zlib CRC-32
(SMHasher name crc32, "CRC-32 soft").

Models (checked against the VerifyAll constants of main.cpp: FNV64 0x103455FC, crc32 0x3719DB20):

    FNV64a(key, len, seed):  h = (uint64)seed ^ 0xcbf29ce484222325;  per byte  h = (h ^ b) * 0x100000001b3
    crc32(key, len, seed):   s = seed ^ 0xffffffff;  per byte  s = T[(s ^ b) & 255] ^ (s >> 8);  out = s ^ 0xffffffff

Nothing here is new; the literature is cited per attack. Honesty notes:
  * "automatic" = a closed algebraic reduction written for this hash, not a general solver.
  * the attack work reported is the number of table/dictionary operations the Python code really makes (one per
    list entry generated and one per join step): its own steps, not hash evaluations.

FNV64 pre-image (and the known-seed collision derived from it) - the algebra
    Let c_i be the state before byte i and v_i := low8(c_i) ^ b_i (the byte that is really multiplied).
    p = 2^40 + 0x1b3 = 0xb3 (mod 256), so low8(c_{i+1}) = g(v_i) := 0xb3 * v_i mod 256, independent of the
    rest of the state. Hence c_i ^ b_i = c_i - low8(c_i) + v_i = c_i - g(v_{i-1}) + v_i, and
        c_{i+1} = (c_i + v_i - g(v_{i-1})) * p      (v_{-1}: g(v_{-1}) := low8(c_0)).
    Unrolled over L bytes, with g(v) = 0xb3 v - 256 k(v), k(v) = floor(0xb3 v / 256):
        c_L = (c_0 - low8(c_0)) p^L + v_{L-1} p + 256 * sum_{i<=L-2} w(v_i) p^{L-1-i}   (mod 2^64)
        w(v) = (2^32 + 1) v + k(v)
    because p - 0xb3 = 2^8 (2^32 + 1). So (1) the last byte is fixed by the target (v_{L-1} = low8(T / p)),
    and (2) the rest is a pure knapsack  sum_i w(v_i) p^{m_i} == R (mod 2^56)  over L-1 free bytes with NO
    consistency side condition: every choice of the v_i is a valid key (b_i = v_i ^ g(v_{i-1})).
    That is the shape generalized-birthday (Wagner 2002, k-tree) attacks need: 2^t lists of 2^a entries
    (a = (56+4)/(t+1)) find a solution with ~2^(a+t+2) work instead of 2^64 (pre-image) / 2^32 (birthday).
    A lattice (LLL/CVP) approach on the same congruence (the folklore way to invert FNV) also works for
    longer keys but needs a consistency search; the k-tree needs none, so it is what is implemented.
    Works for every seed: the seed only changes c_0 and low8(c_0)."""
import math
import random

from .core import attack, Finding, register_model
from .bv import mask, inv

# ---------------------------------------------------------------------------
# FNV64
# ---------------------------------------------------------------------------
P64, B64 = 0x100000001B3, 0xCBF29CE484222325
M64, M56 = mask(64), mask(56)


def fnv64(key, seed=0):
    h = (seed ^ B64) & M64
    for b in key:
        h = ((h ^ b) * P64) & M64
    return h


register_model("FNV64", lambda key, seed: fnv64(key, seed).to_bytes(8, "little"))

G = [(0xB3 * v) & 255 for v in range(256)]          # g(v)
K = [(0xB3 * v) >> 8 for v in range(256)]           # k(v)
W = [((1 << 32) + 1) * v + K[v] for v in range(256)]


def keybytes_from_v(vs, c0):
    """Key bytes b_i = v_i ^ low8(c_i) for the chosen multiplied bytes v_i."""
    out, lo = [], c0 & 255
    for v in vs:
        out.append(v ^ lo)
        lo = G[v]
    return bytes(out)


class _Leaf:
    """List of N partial knapsack sums of one group of slots, entries decoded from their index."""

    def __init__(self, tabs, n, rnd):
        g = len(tabs)
        self.tabs, self.g = tabs, g
        last = max(1, n >> (8 * (g - 1)))
        self.alpha = rnd.sample(range(256), min(256, last))
        vals = []
        add = vals.append
        if g == 1:
            for a in self.alpha:
                add(tabs[0][a])
        elif g == 2:
            t0, t1 = tabs
            for a in range(256):
                x = t0[a]
                for b in self.alpha:
                    add((x + t1[b]) & M56)
        else:
            t0, t1, t2 = tabs[:3]
            for a in range(256):
                x = t0[a]
                for b in range(256):
                    y = (x + t1[b]) & M56
                    for c in self.alpha:
                        add((y + t2[c]) & M56)
        self.vals = vals

    def decode(self, idx):
        n_last = len(self.alpha)
        idx, last = divmod(idx, n_last)
        out = [self.alpha[last]]
        for _ in range(self.g - 1):
            idx, d = divmod(idx, 256)
            out.append(d)
        return out[::-1]


def _merge(A, B, mod, target, limit, ctx):
    """Pairs (i, j) with (A[i] + B[j]) == target (mod `mod`). Returns vals, ia, ib."""
    mm = mod - 1
    head, nxt = {}, [0] * len(B)
    for j, y in enumerate(B):
        k = (target - y) & mm
        nxt[j] = head.get(k, -1)
        head[k] = j
    vals, ia, ib = [], [], []
    get = head.get
    for i, x in enumerate(A):
        j = get(x & mm, -1)
        while j >= 0:
            vals.append((x + B[j]) & M56)
            ia.append(i)
            ib.append(j)
            j = nxt[j]
        if not (i & 0xFFFF):
            ctx.check()
        if len(vals) >= limit:
            break
    return vals, ia, ib


def _ktree(leaves, R, a, ctx, cost):
    """Wagner k-tree over the leaf lists; returns the list of slot values (concatenated by leaf) or None."""
    nodes = [(lf.vals, None) for lf in leaves]
    level, K_ = 0, len(leaves)
    t = K_.bit_length() - 1
    levels = []
    while len(nodes) > 1:
        last_level = len(nodes) == 2
        mod = (1 << 56) if last_level else (1 << ((level + 1) * a))
        new = []
        rec = []
        for q in range(0, len(nodes), 2):
            tgt = R if q == len(nodes) - 2 else 0
            va, vb = nodes[q][0], nodes[q + 1][0]
            cost[0] += len(va) + len(vb)
            vals, ia, ib = _merge(va, vb, mod, tgt, 4 << a, ctx)
            if not vals:
                return None
            new.append((vals, None))
            rec.append((ia, ib))
        levels.append(rec)
        nodes = new
        level += 1
    # backtrack from the first solution of the root
    pick = [0]
    for rec in reversed(levels):
        nxt = []
        for q, i in enumerate(pick):
            ia, ib = rec[q]
            nxt += [ia[i], ib[i]]
        pick = nxt
    out = []
    for lf, i in zip(leaves, pick):
        out += lf.decode(i)
    return out


def fnv64_solve(target, seed, L, K_, ctx, tries=6):
    """Key of length L with fnv64(key, seed) == target via a K_-list (K_ = 4 or 8) Wagner tree.
    Returns (key, ops) or None. Different `tries` use different random leaf subsets."""
    c0 = (seed ^ B64) & M64
    pinv = inv(P64, 64)
    vlast = ((target * pinv) & M64) & 255
    rhs = (target - ((c0 - (c0 & 255)) * pow(P64, L, 1 << 64)) - vlast * P64) & M64
    assert rhs & 255 == 0
    R = rhs >> 8
    n = L - 1                                   # free slots v_0 .. v_{L-2}
    assert n % K_ == 0
    per = n // K_
    t = K_.bit_length() - 1
    a = (56 + 4) // (t + 1)
    tabs_all = []
    for i in range(n):
        pw = pow(P64, L - 1 - i, 1 << 64)
        tabs_all.append([(W[v] * pw) & M56 for v in range(256)])
    cost = [0]
    for attempt in range(tries):
        rnd = random.Random(0xF00D + attempt)
        leaves = []
        for q in range(K_):
            lf = _Leaf(tabs_all[q * per:(q + 1) * per], 1 << a, rnd)
            leaves.append(lf)
            cost[0] += len(lf.vals)
            ctx.check()
        vs = _ktree(leaves, R, a, ctx, cost)
        if vs is None:
            continue
        key = keybytes_from_v(vs + [vlast], c0)
        if fnv64(key, seed) == target:
            return key, cost[0]
    return None


@attack("FNV64", "preimage", 64, method="algebraic reduction to a 56-bit knapsack + Wagner 4-list generalized birthday, 13-byte key", fast=False)
def fnv64_preimage(ctx):
    """13-byte key, pre-image of ctx.target for ctx.seed (any seed). 4 lists of 2^20 entries (3 slots each),
    ~2^23 table operations. Brute force: 2^64 evaluations."""
    target = int.from_bytes(ctx.target, "little")
    r = fnv64_solve(target, ctx.seed, 13, 4, ctx)
    if r is None:
        return None
    key, ops = r
    return Finding([key], [ctx.seed], math.log2(ops), "4 lists x 2^20, %d table ops" % ops)


@attack("FNV64", "collision", 64, method="pre-image of the hash of a random key by an 8-list Wagner tree, 17-byte keys")
def fnv64_collision(ctx):
    """Full 64-bit collision at the known seed: K1 = fixed 17 bytes, K2 = a pre-image of fnv64(K1) from an
    8-list Wagner tree (2^15 entries per list, ~2^18 table ops) with K2 != K1. Birthday would need 2^32."""
    k1 = bytes(range(0x41, 0x41 + 17))
    target = fnv64(k1, ctx.seed)
    r = fnv64_solve(target, ctx.seed, 17, 8, ctx)
    if r is None:
        return None
    k2, ops = r
    if k2 == k1:
        return None
    return Finding([k1, k2], [ctx.seed], math.log2(ops), "8 lists x 2^15, %d table ops" % ops)


@attack("FNV64", "seed-independent collision", 64, method="none known: needs one difference vector that works for all 256 values of low8(c_0)",
        expect="notfound")
def fnv64_sic(ctx):
    """Not found, honest attempt. Seed 32 bit = XOR into c_0; (c_0 - low8 c_0) p^L is common to both keys, so the
    two keys must agree for all 256 values of low8(c_0). The increments d_i = (v_i - v_i') - (g(v_{i-1}) - g(v_i'))
    would have to be one fixed short lattice vector for every low8(c_0); XOR differences (the only thing a key
    difference controls) give integer differences whose sign depends on the data bits (only 0x80 commutes with g),
    so no such pair of chains exists that I could construct. Experiment (one-off, not run in CI): for FNV1a 32 bit all 2^24 keys of length 1..3 were
    hashed for six values of low8(c_0)
    (0,1,2,3,0x80,0xff); no two distinct keys agree on all six, i.e. there is no short structural pair. Nothing is
    tried at run time (returns None)."""
    return None


@attack("FNV1a", "seed-independent collision", 32, method="none known (same argument as FNV64)", expect="notfound")
def fnv1a_sic(ctx):
    """Not found, honest attempt: see fnv64_sic (the 32-bit state has the same structure, low8(c_0) is the only
    seed-dependent part of the difference after the common term (c_0 - low8 c_0) p^L). Returns None."""
    return None


# ---------------------------------------------------------------------------
# CRC-32 (zlib, reflected, poly 0xEDB88320)
# ---------------------------------------------------------------------------
def _table():
    t = []
    for i in range(256):
        c = i
        for _ in range(8):
            c = (c >> 1) ^ (0xEDB88320 if c & 1 else 0)
        t.append(c)
    return t


T32 = _table()


def crc_state(key, state):
    for b in key:
        state = T32[(state ^ b) & 255] ^ (state >> 8)
    return state


def crc32(key, seed=0):
    return crc_state(key, (seed ^ 0xFFFFFFFF) & 0xFFFFFFFF) ^ 0xFFFFFFFF


register_model("crc32", lambda key, seed: crc32(key, seed).to_bytes(4, "little"))


def _forge4(state0, want):
    """4 bytes t with crc_state(t, state0) == want. The map t -> final state is affine over GF(2):
    final = F0 xor M t; M is solved exactly by Gaussian elimination (no search)."""
    f0 = crc_state(bytes(4), state0)
    cols = []                                    # image of every input bit (little endian, 32 bits)
    for i in range(32):
        t = bytearray(4)
        t[i // 8] = 1 << (i % 8)
        cols.append(crc_state(bytes(t), state0) ^ f0)
    # solve sum_i x_i cols[i] = want ^ f0
    rows = [(c, 1 << i) for i, c in enumerate(cols)]
    basis = {}
    for v, comb in rows:
        for bit in sorted(basis, reverse=True):
            if v >> bit & 1:
                bv, bc = basis[bit]
                v ^= bv
                comb ^= bc
        if v:
            basis[v.bit_length() - 1] = (v, comb)
    goal, x = want ^ f0, 0
    for bit in sorted(basis, reverse=True):
        if goal >> bit & 1:
            bv, bc = basis[bit]
            goal ^= bv
            x ^= bc
    assert goal == 0
    t = bytearray(4)
    for i in range(32):
        if x >> i & 1:
            t[i // 8] |= 1 << (i % 8)
    return bytes(t)


@attack("crc32", "preimage", 32, method="CRC is affine over GF(2): 4 key bytes forge any final register (linear solve)")
def crc32_preimage(ctx):
    """4-byte key. out = state ^ 0xffffffff, so the wanted final register is target ^ 0xffffffff; the register
    after 4 key bytes is affine in them (full-rank 32x32 map, because the table's top bytes are a bijection),
    solved by Gaussian elimination. ~2^7 operations instead of 2^32 evaluations. Works for every seed."""
    target = int.from_bytes(ctx.target, "little")
    s0 = (ctx.seed ^ 0xFFFFFFFF) & 0xFFFFFFFF
    key = _forge4(s0, target ^ 0xFFFFFFFF)
    return Finding([key], [ctx.seed], 7.0, "32x32 GF(2) elimination")


@attack("crc32", "seed-independent collision", 32,
        method="two equal-length keys whose difference polynomial is a multiple of the CRC generator")
def crc32_sic(ctx):
    """k1 = 00 00 00 00 00, k2 = 01 t0 t1 t2 t3 with t forged so that the register run from init 0 ends at 0.
    The register is s_n = A^n * init xor Z(key) (A the 8-step update, Z the same machine started at 0), so
    for equal lengths the init (= seed ^ 0xffffffff) contributes the same term to both keys and the CRCs
    are equal for EVERY seed iff Z(k1) == Z(k2). Here Z(k1) = 0 = Z(k2). Length n = 5 enters only through A^n."""
    k1 = bytes(5)
    s_after_01 = crc_state(b"\x01", 0)
    k2 = b"\x01" + _forge4(s_after_01, 0)
    assert crc_state(k1, 0) == crc_state(k2, 0) == 0
    return Finding([k1, k2], list(range(8)), 7.0, "GF(2) elimination, 5-byte keys")
