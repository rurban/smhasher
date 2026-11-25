"""Registry, native oracle and harness of the collision prover.

An *attack* is a Python function registered for one (hash, goal) pair:

    @attack("FNV1a", "preimage", 32, method="meet in the middle over 4 free bytes")
    def fnv1a_preimage(ctx):
        ...
        return Finding(keys=[key], evals_log2=17, work="2^16-entry table")

It runs in its own worker process under a hard wall-clock timeout (2 minutes by
default). Whatever it returns is only a claim: the harness recomputes the claim
with the real, compiled hash function of SMHasher (build/CollisionProverVerify)
before it reports FOUND, and checks that the attack beat brute force.

Goals
  preimage                    a key whose hash is ctx.target, for the known seed ctx.seed
  collision                   two different keys, same full-width hash, known seed ctx.seed
  seed-independent collision  two different keys with the same full-width hash for every seed
"""
import hashlib
import json
import math
import os
import random
import subprocess
import sys
import threading
import time
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
SCRIPT = ROOT / "collision_prover.py"
GOALS = ("preimage", "collision", "seed-independent collision")
GOAL_ALIAS = {"pre": "preimage", "preimage": "preimage", "col": "collision", "collision": "collision",
              "sic": "seed-independent collision", "seed-independent": "seed-independent collision",
              "seed-independent collision": "seed-independent collision"}
DEFAULT_TIMEOUT = 120          # seconds: the agreed hard limit per (hash, goal)
MARGIN_LOG2 = 8                # "clearly beats brute force": at least 2^8 times cheaper
FAIL_LINE = "*********FAIL*********"
SECTION = "Collision Prover Tests"


class Timeout(Exception):
    """Raised by Context.check() when the soft deadline of an attack has passed."""


# ---------------------------------------------------------------------------
# registry
# ---------------------------------------------------------------------------
class Attack:
    def __init__(self, hash, goal, bits, method, fn, expect, fast):
        self.hash, self.goal, self.bits, self.method, self.fn = hash, goal, bits, method, fn
        self.expect, self.fast = expect, fast


REGISTRY = {}


def attack(hash, goal, bits, method, expect="found", fast=True):
    """Register fn(ctx) -> Finding | None for (hash, goal). `hash` is the registry name of SMHasher,
    `bits` its output width (the self test checks it against build/CollisionProverVerify --listnames).

    expect: "found" or "notfound" - what the self test (ctest) asserts.
    fast:   True if the attack ends within a few seconds (self test runs it)."""
    goal = GOAL_ALIAS[goal]

    def deco(fn):
        REGISTRY[(hash, goal)] = Attack(hash, goal, bits, method, fn, expect, fast)
        return fn
    return deco


MODELS = {}


def register_model(hash, fn):
    """Register a Python reference model fn(key: bytes, seed: int) -> digest bytes (memory order of the
    hash output, like the native oracle). The self test compares it with the compiled hash on random keys
    and seeds, so a wrong model is caught before an attack built on it is trusted."""
    MODELS[hash] = fn


class Finding:
    """What an attack claims. keys: 1 key (preimage) or >= 2 distinct keys (collision).
    seeds: seeds the claim is made for; evals_log2: log2 of the attack's own work, counted in
    whatever steps the attack takes (hash evaluations, table operations, field multiplications ...). It is
    NOT necessarily a number of hash evaluations; it is reported as "attack work (steps)" and only used to
    check that the attack is clearly cheaper than brute force (2^bits hash evaluations)."""

    def __init__(self, keys, seeds=(0,), evals_log2=0.0, work="", notes=""):
        self.keys = [bytes(k) for k in keys]
        self.seeds = [int(s) for s in seeds]
        self.evals_log2 = float(evals_log2)
        self.work = work
        self.notes = notes

    def to_json(self):
        return {"keys": [k.hex() for k in self.keys], "seeds": self.seeds, "evals_log2": self.evals_log2,
                "work": self.work, "notes": self.notes}

    @staticmethod
    def from_json(d):
        return Finding([bytes.fromhex(k) for k in d["keys"]], d["seeds"], d["evals_log2"], d["work"], d["notes"])


class Context:
    """Passed to an attack: which hash, which seed and target, how much time is left."""

    def __init__(self, name, goal, bits, seed=0, budget=DEFAULT_TIMEOUT, native=None):
        self.name, self.goal, self.bits, self.seed = name, goal, bits, seed
        self.start = time.monotonic()
        self.deadline = self.start + budget
        self.native = native
        self.target = target_for(name, bits)

    def left(self):
        return self.deadline - time.monotonic()

    def check(self):
        if time.monotonic() > self.deadline:
            raise Timeout()


def target_for(name, bits):
    """The fixed pre-image target of a hash: bytes of SHA-256('smhasher collision prover: <name>'),
    in memory order of the hash output. Nobody holds a key for it, so a pre-image is a real find."""
    return hashlib.sha256(("smhasher collision prover: %s" % name).encode()).digest()[: bits // 8]


# ---------------------------------------------------------------------------
# native oracle: the real compiled hash functions
# ---------------------------------------------------------------------------
class Native:
    """Talks to build/CollisionProverVerify --serve. Thread-safe, one process."""

    @staticmethod
    def find_exe():
        env = os.environ.get("SMHASHER_PROVER_VERIFY")
        cands = [Path(env)] if env else []
        for d in (ROOT / "build", ROOT):
            for n in ("CollisionProverVerify", "CollisionProverVerify.exe"):
                cands.append(d / n)
        for c in cands:
            if c.is_file():
                return str(c)
        return None

    def __init__(self, exe=None):
        self.exe = exe or self.find_exe()
        if not self.exe:
            raise RuntimeError("build CollisionProverVerify first: cmake --build build --target CollisionProverVerify")
        self.lock = threading.Lock()
        self.proc = subprocess.Popen([self.exe, "--serve"], stdin=subprocess.PIPE, stdout=subprocess.PIPE,
                                     stderr=subprocess.DEVNULL, text=True, bufsize=1)

    def _ask(self, line):
        with self.lock:
            self.proc.stdin.write(line + "\n")
            self.proc.stdin.flush()
            return self.proc.stdout.readline().strip()

    def hash(self, name, key, seed=0):
        """Digest bytes (memory order) of hash `name` over `key` with `seed`."""
        r = self._ask("H %s %d %s" % (name, seed, bytes(key).hex() or "-"))
        if not r or r == "ERR":
            raise RuntimeError("native hash failed for %s" % name)
        return bytes.fromhex(r)

    def rate(self, name, length, seconds=0.2):
        """Hashes per second of the real function for keys of this length (one thread)."""
        r = self._ask("R %s %d %s" % (name, length, seconds))
        return float(r) if r and r != "ERR" else None

    def close(self):
        try:
            self.proc.stdin.close()
            self.proc.wait(timeout=5)
        except Exception:
            self.proc.kill()

    @staticmethod
    def names(exe=None):
        exe = exe or Native.find_exe()
        out = subprocess.run([exe, "--listnames"], check=True, text=True, stdout=subprocess.PIPE).stdout
        return {l.split()[0]: int(l.split()[1]) for l in out.splitlines() if l.strip()}


# ---------------------------------------------------------------------------
# verification and reporting
# ---------------------------------------------------------------------------
PROBE_SEEDS = [0, 1, 2, 3, 0x9E3779B9, 0xDEADBEEF, 0x80000000, 0xFFFFFFFF, 12345678, 0x01234567]


def check_seeds(seeds):
    rnd = random.Random(20261125)
    s = list(dict.fromkeys(list(seeds) + PROBE_SEEDS + [rnd.getrandbits(32) for _ in range(6)]))
    return s


class Result:
    def __init__(self, hash, goal, method):
        self.hash, self.goal, self.method = hash, goal, method
        self.status = "NOTFOUND"      # FOUND | NOTFOUND | REJECTED
        self.seconds = 0.0
        self.finding = None
        self.verified = None          # True | False | None (native not available)
        self.why = ""                 # NOTFOUND/REJECTED reason or verification detail
        self.brute = ""               # what brute force would have cost
        self.counts = False           # FOUND and beats brute force by the agreed margin
        self.bits = 0

    @property
    def command(self):
        return "./collision_prover.py %s:%s" % (self.hash, {"preimage": "preimage", "collision": "collision",
                                                             "seed-independent collision": "sic"}[self.goal])


def brute_force(goal, bits, rate):
    """(log2 of brute-force evaluations, human sentence)."""
    if goal == "preimage":
        lg = bits
    elif goal == "collision":
        lg = bits / 2.0
    else:
        return None, "none: the seed is unknown, brute force has nothing to compare (birthday at one known seed: 2^%g evals)" % (bits / 2.0)
    txt = "2^%g evals" % lg
    if rate:
        sec = 2.0 ** lg / rate
        txt += " = " + human_time(sec) + " at %.0f M hashes/s" % (rate / 1e6)
    return lg, txt


def human_time(sec):
    if sec < 1:
        return "%.0f ms" % (sec * 1000)
    if sec < 120:
        return "%.0f s" % sec
    if sec < 7200:
        return "%.0f min" % (sec / 60)
    if sec < 172800:
        return "%.0f h" % (sec / 3600)
    if sec < 3.15e7 * 2:
        return "%.0f days" % (sec / 86400)
    y = sec / 3.15e7
    return "%.1e years" % y if y > 1e4 else "%.0f years" % y


def verify(res, finding, bits, native, seed):
    """Recompute the claim with the compiled hash. Sets res.verified / res.why."""
    name, goal = res.hash, res.goal
    ks = finding.keys
    try:
        if goal == "preimage":
            if len(ks) != 1:
                res.verified, res.why = False, "preimage needs exactly one key"
                return
            ok = native.hash(name, ks[0], seed) == target_for(name, bits)
            res.verified, res.why = ok, "native hash(key) %s target" % ("==" if ok else "!=")
        else:
            if len(ks) < 2 or len(set(ks)) != len(ks):
                res.verified, res.why = False, "collision needs distinct keys"
                return
            seeds = [seed] if goal == "collision" else check_seeds(finding.seeds)
            for s in seeds:
                d = {native.hash(name, k, s) for k in ks}
                if len(d) != 1:
                    res.verified, res.why = False, "keys differ in native hash at seed %#x" % s
                    return
            res.verified = True
            res.why = "%d keys, %s equal at %s" % (len(ks), "full hash", "seed %d" % seed if goal == "collision"
                                                    else "%d seeds" % len(seeds))
    except Exception as e:  # native oracle died
        res.verified, res.why = False, "native check failed: %s" % e


def run_worker(name, goal, budget):
    """Entry of the worker process: run one attack, print one JSON line."""
    from . import load_families
    load_families()
    atk = REGISTRY[(name, goal)]
    native = None
    try:
        native = Native()
    except Exception:
        pass
    ctx = Context(name, goal, atk.bits, 0, budget, native)
    t0 = time.monotonic()
    out = {"status": "notfound", "why": ""}
    try:
        f = atk.fn(ctx)
        if f is None:
            out["why"] = "attack found nothing"
        else:
            out = {"status": "found", "finding": f.to_json()}
    except Timeout:
        out["why"] = "soft deadline"
    except MemoryError:
        out["why"] = "out of memory"
    except Exception as e:
        out = {"status": "error", "why": "%s: %s" % (type(e).__name__, e)}
    out["seconds"] = time.monotonic() - t0
    print(json.dumps(out), flush=True)


def run_attack(name, goal, native, timeout=DEFAULT_TIMEOUT, seed=0):
    """Run one registered (hash, goal) under a hard timeout, verify natively, compare with brute force."""
    atk = REGISTRY[(name, goal)]
    bits = atk.bits
    res = Result(name, goal, atk.method)
    res.bits = bits
    cmd = [sys.executable, str(SCRIPT), "--worker", name, goal, str(max(1, timeout - 4))]
    t0 = time.monotonic()
    kw = {"start_new_session": True} if os.name == "posix" else {}
    p = subprocess.Popen(cmd, stdout=subprocess.PIPE, stderr=subprocess.PIPE, text=True, cwd=str(ROOT), **kw)
    try:
        out, err = p.communicate(timeout=timeout)
    except subprocess.TimeoutExpired:
        try:
            if os.name == "posix":
                os.killpg(p.pid, 9)
            else:
                p.kill()
        except Exception:
            p.kill()
        p.communicate()
        res.seconds = time.monotonic() - t0
        res.why = "timeout"
        return res
    res.seconds = time.monotonic() - t0
    try:
        d = json.loads(out.strip().splitlines()[-1])
    except Exception:
        res.status, res.why = "REJECTED", "worker crashed: " + (err or out).strip()[-200:]
        return res
    res.seconds = d.get("seconds", res.seconds)
    if d["status"] == "error":
        res.status, res.why = "REJECTED", d["why"]
        return res
    if d["status"] != "found":
        res.why = d.get("why", "")
        return res
    f = Finding.from_json(d["finding"])
    res.finding = f
    if native is not None:
        verify(res, f, bits, native, seed)
        rate = native.rate(name, len(f.keys[0])) if native else None
    else:
        res.why, rate = "native verifier not built", None
    lg, res.brute = brute_force(goal, bits, rate)
    if res.verified is False:
        res.status = "REJECTED"
        return res
    res.status = "FOUND"
    if goal == "preimage":
        res.counts = f.evals_log2 <= lg - MARGIN_LOG2
    elif goal == "collision":
        res.counts = bits > 32 and f.evals_log2 <= lg - MARGIN_LOG2
        if not res.counts:
            res.why += "; does not count: " + ("32-bit collision at a known seed is a birthday problem"
                                                if bits <= 32 else "not clearly cheaper than birthday")
    else:
        res.counts = True
    if goal == "preimage" and not res.counts:
        res.why += "; does not count: not clearly cheaper than brute force"
    if native is None:
        res.counts = False
    return res


def fmt_keys(f, limit=48):
    def one(k):
        h = k.hex()
        return h if len(h) <= limit else h[:limit] + "..(%d bytes)" % len(k)
    return " ".join(one(k) for k in f.keys[:4]) + (" ..(%d keys)" % len(f.keys) if len(f.keys) > 4 else "")


def line(res):
    """One line of the doc/<hash>.txt section, in the style of collision_solver.py."""
    g = res.goal
    if res.status == "FOUND":
        tag = "FOUND" if res.counts else "FOUND-NOT-COUNTED"
        return "Prover %s %s: %s %s (%.2fs, native verified: %s; brute force %s; attack work 2^%.3g steps, not hash evaluations; %s)" % (
            res.hash, g, tag, fmt_keys(res.finding), res.seconds,
            {True: "yes", False: "NO", None: "n/a"}[res.verified], res.brute, res.finding.evals_log2,
            res.finding.work or res.method)
    if res.status == "REJECTED":
        return "Prover %s %s: REJECTED %s" % (res.hash, g, res.why)
    if res.why in ("timeout", "soft deadline"):
        return "Prover %s %s: NOTFOUND (not found in %d min)" % (res.hash, g, round(DEFAULT_TIMEOUT / 60.0))
    return "Prover %s %s: NOTFOUND (stopped after %.1fs: %s)" % (res.hash, g, res.seconds, res.why or "no result")


def section(results):
    lines = ["[[[ %s ]]]" % SECTION, ""] + [line(r) for r in results]
    if any(r.status == "FOUND" and r.counts for r in results):
        lines += ["", FAIL_LINE]
    return "\n".join(lines) + "\n\n"
