"""General-purpose z3 fall-back for the collision prover (optional: needs `pip install z3-solver`).

Idea: write a hash model ONCE with the small `BV` type below. A `BV` holds either a Python int or a z3 BitVec
of a fixed width, and its operators (+ - * ^ & | ~ << >> rotl rotr mulwide bswap ...) behave like C unsigned
arithmetic on both (>> is a logical shift, + - * wrap). So the same function is
  * the reference model when called with concrete bytes (`Key.concrete`), and
  * the bit-vector formula when called with symbolic bytes (`Key.symbolic`).

    from prover import smt
    def model(key, seed):                       # key: smt.Key, seed: smt.BV(32); returns a BV, the digest as
        h = smt.BV(0x811C9DC5, 32) ^ seed       # a little-endian integer (digest bytes = h.to_bytes())
        for b in key.bytes:                     # key.bytes: list of BV(8)
            h = (h ^ b.zext(32)) * 16777619
        return h
    f = smt.solve_preimage(model, ctx.target, nbytes=5, ctx=ctx, seed=ctx.seed)       # -> Finding | None
    f = smt.solve_collision(model, nbytes=8, ctx=ctx, seeds=(0, 1, 2))                 # -> Finding | None

Both honour `ctx.left()` (the z3 timeout is set to what is left of the attack's budget minus a small margin),
re-compute the answer with the concrete model before returning it, and return None when z3 is missing, the
problem is unsatisfiable, or the solver gives up (the reason stays in `smt.last_status`);
with raise_timeout=True a solver that gave up raises core.Timeout instead, which the harness prints as 'not found (2 min)'.
Optional `constraints=lambda key: [...]` fixes bytes of the key, e.g. `[key.bytes[0].eq(0x41), key.u(4, 4).eq(7)]`,
which is how a family narrows the search to a key shape it understands (length, pinned words, ...).

What it can do: hashes that are cheap bit-vector circuits - FNV-style byte loops over a few bytes, CRC-like xor
networks (any number of bytes, it is linear), additions/rotations/xors on a few words, inversion of a single
multiplication by a constant on 32-bit words. What it cannot: long chains of 64-bit multiplications (xxHash64,
Murmur2 64, CityHash on long keys, wyhash with two 128-bit multiplications), anything cryptographic (SipHash:
a 64-bit unknown in front of 8 ARX rounds is a 2^64 problem for a SAT solver, as it should be), or keys of many
bytes through many rounds. Those attempts are registered as expect="notfound" and are reported as
"not found (2 min)". `evals_log2` of a Finding is computed as log2(elapsed seconds * WORK_RATE) with a
deliberately generous WORK_RATE (hash evaluations per second of a fast C hash), so a slow solver never looks
cheaper than brute force than it really was.

Run `python -m prover.smt` for a self test (the z3 part is skipped when z3 is missing)."""
import math
import time

from .core import Finding, Timeout

try:                                    # optional dependency
    import z3
except Exception:                       # ImportError or a blocked/missing libz3
    z3 = None

HAVE_Z3 = z3 is not None
WORK_RATE = 1e8                         # hash evaluations per second charged to the solver (generous)
MARGIN_S = 3.0                          # seconds kept back from ctx.left() for model extraction and the caller
last_status = ""                        # why the last solve_* returned None / what it did


def _mask(w):
    return (1 << w) - 1


class BV:
    """Fixed-width unsigned bit-vector: a Python int (concrete) or a z3 BitVecRef (symbolic)."""
    __slots__ = ("v", "w")

    def __init__(self, v, w=64):
        if isinstance(v, BV):
            v = v.v
        self.w = w
        self.v = (v & _mask(w)) if isinstance(v, int) else v

    # -- helpers
    @property
    def symbolic(self):
        return not isinstance(self.v, int)

    def _o(self, o):
        if isinstance(o, BV):
            return o.v
        if self.symbolic:
            return z3.BitVecVal(o & _mask(self.w), self.w)
        return o & _mask(self.w)

    def _new(self, v):
        return BV(v, self.w)

    # -- arithmetic (modulo 2^w)
    def __add__(self, o): return self._new(self.v + self._o(o))
    def __sub__(self, o): return self._new(self.v - self._o(o))
    def __mul__(self, o): return self._new(self.v * self._o(o))
    __radd__ = __add__
    __rmul__ = __mul__
    def __rsub__(self, o): return self._new(self._o(o) - self.v)
    def __xor__(self, o): return self._new(self.v ^ self._o(o))
    __rxor__ = __xor__
    def __and__(self, o): return self._new(self.v & self._o(o))
    __rand__ = __and__
    def __or__(self, o): return self._new(self.v | self._o(o))
    __ror__ = __or__
    def __invert__(self): return self._new(~self.v)
    def __neg__(self): return self._new(-self.v)

    def __lshift__(self, n):
        if self.symbolic:
            return self._new(self.v << z3.BitVecVal(n, self.w)) if n < self.w else self._new(z3.BitVecVal(0, self.w))
        return self._new(self.v << n) if n < self.w else self._new(0)

    def __rshift__(self, n):            # logical shift right
        if self.symbolic:
            return self._new(z3.LShR(self.v, z3.BitVecVal(n, self.w))) if n < self.w \
                else self._new(z3.BitVecVal(0, self.w))
        return self._new(self.v >> n) if n < self.w else self._new(0)

    def rotl(self, r):
        r %= self.w
        return self if r == 0 else (self << r) | (self >> (self.w - r))

    def rotr(self, r):
        return self.rotl(self.w - (r % self.w))

    def bswap(self):
        n = self.w // 8
        if self.symbolic:
            return self._new(z3.Concat(*[z3.Extract(8 * i + 7, 8 * i, self.v) for i in range(n)]))
        return self._new(int.from_bytes(self.v.to_bytes(n, "little"), "big"))

    # -- width changes
    def zext(self, w):
        if self.symbolic:
            return BV(z3.ZeroExt(w - self.w, self.v) if w > self.w else z3.Extract(w - 1, 0, self.v), w)
        return BV(self.v, w)

    def trunc(self, w):
        return self.zext(w)

    def extract(self, hi, lo):
        if self.symbolic:
            return BV(z3.Extract(hi, lo, self.v), hi - lo + 1)
        return BV(self.v >> lo, hi - lo + 1)

    def mulwide(self, o):
        """Full product of two w-bit values as (low w bits, high w bits) - like a 64x64 -> 128 MUL."""
        o = o if isinstance(o, BV) else BV(o, self.w)
        w = self.w
        if self.symbolic or o.symbolic:
            a = self.v if self.symbolic else z3.BitVecVal(self.v, w)
            b = o.v if o.symbolic else z3.BitVecVal(o.v, w)
            p = z3.ZeroExt(w, a) * z3.ZeroExt(w, b)
            return BV(z3.Extract(w - 1, 0, p), w), BV(z3.Extract(2 * w - 1, w, p), w)
        p = self.v * o.v
        return BV(p & _mask(w), w), BV(p >> w, w)

    # -- comparison -> z3 Bool (or bool when concrete); no __eq__ so BV stays hashable
    def eq(self, o): return self.v == self._o(o)
    def ne(self, o): return self.v != self._o(o)

    def __int__(self):
        if self.symbolic:
            raise ValueError("symbolic BV has no integer value")
        return self.v

    def to_bytes(self):
        return int(self).to_bytes(self.w // 8, "little")

    def __repr__(self):
        return "BV(%s, %d)" % (hex(self.v) if not self.symbolic else self.v, self.w)


def concat_le(parts):
    """BV made of BV parts, first part = least significant (little-endian load of bytes)."""
    w = sum(p.w for p in parts)
    if any(p.symbolic for p in parts):
        vs = [p.v if p.symbolic else z3.BitVecVal(p.v, p.w) for p in reversed(parts)]
        return BV(z3.Concat(*vs) if len(vs) > 1 else vs[0], w)
    acc, sh = 0, 0
    for p in parts:
        acc |= p.v << sh
        sh += p.w
    return BV(acc, w)


class Key:
    """The key bytes of a model: `.bytes` is a list of BV(8); `.u(off, n)` loads n bytes little-endian as BV(8n);
    `.n` the length. Build with Key.symbolic(n) or Key.concrete(bytes)."""

    def __init__(self, bs):
        self.bytes = list(bs)
        self.n = len(self.bytes)

    @staticmethod
    def symbolic(n, prefix="k"):
        if z3 is None:
            raise RuntimeError("z3 is not available")
        return Key([BV(z3.BitVec("%s%d" % (prefix, i), 8), 8) for i in range(n)])

    @staticmethod
    def concrete(data):
        return Key([BV(b, 8) for b in bytes(data)])

    def u(self, off, n):
        return concat_le(self.bytes[off:off + n])

    def value(self, model):
        """Bytes of a symbolic key under a z3 model."""
        return bytes(model.eval(b.v, model_completion=True).as_long() for b in self.bytes)


# ---------------------------------------------------------------------------------------------- solving
def _budget(ctx, timeout_s):
    t = float(timeout_s) if timeout_s else 3600.0
    if ctx is not None:
        t = min(t, ctx.left() - MARGIN_S)
    return t


def _solver(seconds):
    try:
        s = z3.SolverFor("QF_BV")
    except Exception:
        s = z3.Solver()
    s.set("timeout", max(1, int(seconds * 1000)))
    return s


def _as_int(x):
    return int.from_bytes(x, "little") if isinstance(x, (bytes, bytearray)) else int(x)


def _digest(model_fn, key, seed, bits):
    d = model_fn(key, BV(seed, 32))
    return int(d) & _mask(bits)


def _run(s, status_extra=""):
    global last_status
    r = s.check()
    if r == z3.sat:
        return s.model()
    last_status = "unsat" if r == z3.unsat else "z3 gave up (%s)" % (s.reason_unknown() or "timeout")
    return None


def _gave_up(raise_timeout):
    """z3 stopped without a model. With raise_timeout the attack ends through core.Timeout, which the harness
    reports as 'not found (2 min)' (use it when the budget is the whole point, e.g. expect="notfound" attempts)."""
    if raise_timeout and last_status.startswith("z3 gave up"):
        raise Timeout()
    return None


def _evals_log2(elapsed):
    return math.log2(max(2.0, elapsed * WORK_RATE))


def solve_preimage(model_fn, target, nbytes, timeout_s=None, ctx=None, seed=0, bits=None, constraints=None,
                   work="z3 bit-vector solver", raise_timeout=False):
    """A key of `nbytes` bytes with model(key, seed) == target (bytes in memory order, or an int), or None.
    bits: digest width (default 8*len(target) for bytes)."""
    global last_status
    if z3 is None:
        last_status = "z3 not installed"
        return None
    if bits is None:
        bits = 8 * len(target) if isinstance(target, (bytes, bytearray)) else 64
    tv = _as_int(target) & _mask(bits)
    t0 = time.monotonic()
    budget = _budget(ctx, timeout_s)
    if budget <= 0:
        last_status = "no time left"
        return None
    key = Key.symbolic(nbytes)
    d = model_fn(key, BV(seed, 32))
    s = _solver(budget)
    s.add(d.extract(bits - 1, 0).v == z3.BitVecVal(tv, bits) if d.w != bits else d.v == z3.BitVecVal(tv, bits))
    for c in (constraints(key) if callable(constraints) else (constraints or [])):
        s.add(c)
    m = _run(s)
    if m is None:
        return _gave_up(raise_timeout)
    k = key.value(m)
    if _digest(model_fn, Key.concrete(k), seed, bits) != tv:      # the z3 answer must hold in the concrete model
        last_status = "z3 answer failed the concrete model check"
        return None
    last_status = "sat"
    el = time.monotonic() - t0
    return Finding([k], [seed], _evals_log2(el), "%s, %.1f s" % (work, el))


def solve_collision(model_fn, nbytes, timeout_s=None, ctx=None, seeds=(0,), bits=64, constraints=None,
                    work="z3 bit-vector solver", raise_timeout=False):
    """Two different keys (nbytes = int, or (n1, n2)) with equal digests for every seed in `seeds`, or None.
    `constraints(k1, k2)` may restrict both keys. With several seeds the result is a candidate for a
    seed-independent collision; the harness re-checks it natively on 16+ seeds."""
    global last_status
    if z3 is None:
        last_status = "z3 not installed"
        return None
    n1, n2 = (nbytes, nbytes) if isinstance(nbytes, int) else nbytes
    t0 = time.monotonic()
    budget = _budget(ctx, timeout_s)
    if budget <= 0:
        last_status = "no time left"
        return None
    k1, k2 = Key.symbolic(n1, "a"), Key.symbolic(n2, "b")
    s = _solver(budget)
    for sd in seeds:
        d1, d2 = model_fn(k1, BV(sd, 32)), model_fn(k2, BV(sd, 32))
        s.add(d1.v == d2.v)
    if n1 == n2:
        s.add(z3.Or(*[a.v != b.v for a, b in zip(k1.bytes, k2.bytes)]))
    for c in (constraints(k1, k2) if callable(constraints) else (constraints or [])):
        s.add(c)
    m = _run(s)
    if m is None:
        return _gave_up(raise_timeout)
    a, b = k1.value(m), k2.value(m)
    ok = a != b
    for sd in seeds:
        ok = ok and _digest(model_fn, Key.concrete(a), sd, bits) == _digest(model_fn, Key.concrete(b), sd, bits)
    if not ok:
        last_status = "z3 answer failed the concrete model check"
        return None
    last_status = "sat"
    el = time.monotonic() - t0
    return Finding([a, b], list(seeds), _evals_log2(el), "%s, %.1f s" % (work, el))


# ---------------------------------------------------------------------------------------------- self test
def _toy(key, seed):
    h = BV(0x811C9DC5, 32) ^ seed
    for b in key.bytes:
        h = (h ^ b.zext(32)) * 16777619
    return h


def selftest():
    k = bytes([1, 2, 3, 4])
    d = _toy(Key.concrete(k), 0)
    ref = 0x811C9DC5
    for b in k:
        ref = ((ref ^ b) * 16777619) & 0xFFFFFFFF
    assert int(d) == ref, "concrete BV model differs from the plain int model"
    x = BV(0x0123456789ABCDEF, 64)
    assert int(x.rotl(13)) == ((0x0123456789ABCDEF << 13) | (0x0123456789ABCDEF >> 51)) & _mask(64)
    lo, hi = x.mulwide(BV(0xFEDCBA9876543210, 64))
    assert (int(hi) << 64) | int(lo) == 0x0123456789ABCDEF * 0xFEDCBA9876543210
    assert int(x >> 4) == 0x00123456789ABCDE and int(x.bswap()) == 0xEFCDAB8967452301
    if not HAVE_Z3:
        print("smt selftest: concrete part OK; z3 missing, symbolic part skipped")
        return True
    f = solve_preimage(_toy, ref.to_bytes(4, "little"), 4, timeout_s=60, bits=32)
    assert f is not None and _digest(_toy, Key.concrete(f.keys[0]), 0, 32) == ref, last_status
    g = solve_collision(_toy, 5, timeout_s=60, bits=32)
    assert g is not None and g.keys[0] != g.keys[1], last_status
    print("smt selftest: OK (preimage in %s, collision in %s)" % (f.work, g.work))
    return True


if __name__ == "__main__":
    selftest()
