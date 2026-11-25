#!/usr/bin/env python3
"""Collision and pre-image prover: try to invert registered SMHasher hashes, with a hard timeout.

For every (hash, goal) with a registered attack (see prover/fam_*.py) the attack runs in its
own process for at most --timeout seconds (default 120 = 2 minutes). A result counts only if
the compiled hash function of SMHasher (build/CollisionProverVerify) reproduces it natively
and the attack was clearly cheaper than brute force. What is not found in time is reported as
"not found (2 min)".

Goals: preimage (known seed, fixed target), collision (known seed, full width), seed-independent
collision (same full hash for every seed). A 32-bit collision at a known seed is a birthday
problem (2^16 tries) and does not count.

Usage: collision_prover.py [options] [hash | hash:goal ...]     goal = preimage | collision | sic
  --section   print the doc/<hash>.txt section that testquality.pl evaluates (one hash)
  --table     print a markdown table of the results
  --timeout S hard limit per (hash, goal), seconds (default 120)
  --jobs N    parallel attacks (default: CPUs)
  --list      list registered attacks
  --selftest  quick self test used by ctest
  --no-native only run the attacks (no native verification; nothing counts)
Exit status: 0, or 1 if a verified attack counted (like collision_solver.py), 2 on usage errors.
"""
import argparse
import os
import sys
from concurrent.futures import ThreadPoolExecutor
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))
import prover  # noqa: E402
from prover import core  # noqa: E402


def parse_names(items):
    """-> list of (hash, goal) from 'hash' / 'hash:goal'; empty list = everything registered."""
    out = []
    for it in items:
        name, _, goal = it.partition(":")
        goals = [core.GOAL_ALIAS[goal]] if goal else list(core.GOALS)
        hit = [(name, g) for g in goals if (name, g) in core.REGISTRY]
        if not hit and not goal:
            hit = [(name, None)]          # known to the user, no attack registered
        out += hit
    return out


def table(results):
    rows = ["| Hash | Goal | Method | Result | Time | Native verified | Brute force would cost | Attack work (own steps, not hash evaluations) | Re-run |",
            "|---|---|---|---|---|---|---|---|---|"]
    for r in results:
        if r.status == "FOUND":
            res = "found" if r.counts else "found, not counted"
        elif r.status == "REJECTED":
            res = "rejected"
        else:
            res = "not found (%d min)" % round(core.DEFAULT_TIMEOUT / 60.0) if r.why in ("timeout", "soft deadline") \
                else "not found (%s)" % (r.why or "no attack succeeded")
        ver = {True: "yes", False: "NO", None: "-"}[r.verified] if r.status != "NOTFOUND" else "-"
        work = "2^%.3g steps" % r.finding.evals_log2 if r.finding else "-"
        rows.append("| %s | %s | %s | %s | %.1f s | %s | %s | %s | `%s` |" % (
            r.hash, r.goal, r.method, res, r.seconds, ver, r.brute or "-", work, r.command))
    return "\n".join(rows) + "\n"


def run_all(pairs, timeout, jobs, native):
    def one(p):
        return core.run_attack(p[0], p[1], native, timeout)
    with ThreadPoolExecutor(max(1, jobs)) as ex:
        return list(ex.map(one, pairs))


def selftest(native):
    bad = 0
    names = None
    try:
        names = core.Native.names()
    except Exception:
        pass
    import random
    rnd = random.Random(7)
    for name, model in sorted(core.MODELS.items()):      # Python models against the compiled hashes
        if native is None:
            break
        n = 0
        for i in range(300):
            key = bytes(rnd.getrandbits(8) for _ in range(rnd.choice([0, 1, 2, 3, 4, 5, 7, 8, 9, 15, 16, 17, 31, 32,
                                                                       33, 63, 64, 65, 127, 128, 129, 200])))
            seed = rnd.choice([0, 1, rnd.getrandbits(32), rnd.getrandbits(32)])
            n += native.hash(name, key, seed) != model(key, seed)
        print("%s model %s: %d of 300 random keys differ from the native hash" % ("ok  " if not n else "FAIL", name, n))
        bad += n > 0
    for (name, goal), atk in sorted(core.REGISTRY.items()):
        if names is not None and names.get(name) != atk.bits:
            print("FAIL %s: registered %d bits, native says %s" % (name, atk.bits, names.get(name)))
            bad += 1
            continue
        if not atk.fast:
            continue
        r = core.run_attack(name, goal, native, 60)
        ok = (r.status == "FOUND" and r.counts) if atk.expect == "found" else r.status == "NOTFOUND"
        print("%s %s %s: %s" % ("ok  " if ok else "FAIL", name, goal, core.line(r)))
        bad += not ok
    return 1 if bad else 0


def main():
    ap = argparse.ArgumentParser(description=__doc__.split("\n")[0])
    ap.add_argument("--worker", nargs=3, metavar=("HASH", "GOAL", "SECONDS"), help=argparse.SUPPRESS)
    ap.add_argument("--timeout", type=int, default=core.DEFAULT_TIMEOUT)
    ap.add_argument("--jobs", type=int, default=os.cpu_count() or 1)
    ap.add_argument("--section", action="store_true")
    ap.add_argument("--table", action="store_true")
    ap.add_argument("--list", action="store_true")
    ap.add_argument("--selftest", action="store_true")
    ap.add_argument("--no-native", action="store_true")
    ap.add_argument("names", nargs="*")
    a = ap.parse_args()
    if a.worker:
        return core.run_worker(a.worker[0], a.worker[1], float(a.worker[2]))
    prover.load_families()
    if a.list:
        for (name, goal), atk in sorted(core.REGISTRY.items()):
            print("%-12s %-27s %3d bit  %s" % (name, goal, atk.bits, atk.method))
        return 0
    native = None
    if not a.no_native:
        try:
            native = core.Native()
        except RuntimeError as e:
            print(e, file=sys.stderr)
            return 2
    try:
        if a.selftest:
            return selftest(native)
        pairs = parse_names(a.names) if a.names else sorted(core.REGISTRY)
        if a.section:
            if len(a.names) != 1:
                print("--section takes exactly one hash name", file=sys.stderr)
                return 2
            if pairs[0][1] is None:
                print("[[[ %s ]]]\n\nProver %s: SKIP no attack registered\n" % (core.SECTION, pairs[0][0]))
                return 0
        real = [p for p in pairs if p[1] is not None]
        results = run_all(real, a.timeout, a.jobs, native)
        if a.section:
            print(core.section(results), end="")
        elif a.table:
            print(table(results), end="")
        else:
            for r in results:
                print(core.line(r), flush=True)
        return 1 if any(r.status == "FOUND" and r.counts for r in results) else 0
    finally:
        if native:
            native.close()


if __name__ == "__main__":
    sys.exit(main())
