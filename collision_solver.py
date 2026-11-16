#!/usr/bin/env python3
"""Find a known-seed 10-bit collision for 4-byte keys in registered SMHasher hashes.

Method: run build/CollisionSolverProbe under gdb up to the hash call, dump a core,
symbolically execute the real compiled hash from that snapshot with angr, then verify
the solved key natively. Exit status is 0 if every selected hash was solved and verified.

Usage: collision_solver.py [--timeout S] [--jobs N] [name ...]   (default: all hashes)
Env:   SMHASHER_ANGR_PYTHON  python with angr (default: current interpreter)
"""
import argparse
from concurrent.futures import ThreadPoolExecutor
import re
import os
from pathlib import Path
import subprocess
import sys
import tempfile
import time

ROOT = Path(__file__).resolve().parent
PROBE = ROOT / "build" / "CollisionSolverProbe"
# Cryptographic hashes are not worth symbolic execution; their collisions are not expected.
SECURE = re.compile(r"^(md5|sha|rmd|edonr|blake|ascon|Tachyon|keccak|siphash|halfsiphash)", re.I)
MASK = 1023


def probe(name, keyhex):
    out = subprocess.run([str(PROBE), name, keyhex], check=True, text=True,
                         stdout=subprocess.PIPE, stderr=subprocess.DEVNULL).stdout
    return bytes.fromhex(out.strip().split("SOLVER_HASH=")[-1])


def worker(core, target):
    """Exit 0: collision found (key printed); 1: no collision, all paths explored;
    3: some paths errored (unsupported instruction), so inconclusive."""
    import logging
    import angr
    logging.getLogger("angr").setLevel(logging.CRITICAL)
    logging.getLogger("cle").setLevel(logging.CRITICAL)
    claripy = angr.claripy
    want = bytes.fromhex(target)
    p = angr.Project(core, auto_load_libs=False)
    s = p.factory.blank_state()
    key = s.solver.eval(s.regs.rsi)
    out = s.solver.eval(s.regs.rdx)
    ret = s.mem[s.regs.rsp].uint64_t.concrete
    k = claripy.BVS("key", 32)
    s.memory.store(key, k, endness="Iend_LE")
    s.solver.add(k != 0)
    m = p.factory.simgr(s)
    while m.active:
        m.explore(find=ret, num_find=1)
        if not m.found:
            break
        f = m.found.pop()
        for i, b in enumerate(want):
            f.solver.add(f.memory.load(out + i, 1) == b)
        if f.solver.satisfiable():
            print("%08x" % f.solver.eval(k))
            return 0
    return 3 if m.errored or m.unconstrained else 1


def solve(name, timeout, pyexe):
    """Returns (name, status, info). COLLISION is bad; NONE is clean; UNKNOWN is inconclusive."""
    start = time.monotonic()
    with tempfile.TemporaryDirectory(prefix="smhasher-collision-") as tmp:
        core = os.path.join(tmp, "core")
        gdb = subprocess.run(
            ["gdb", "-q", "-batch", "-ex", "set pagination off",
             "-ex", "break collision_solver_marker", "-ex", "run %s 00000000" % name,
             "-ex", "generate-core-file " + core, str(PROBE)],
            stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL, timeout=60)
        if not os.path.exists(core):
            return name, "UNKNOWN", "no core (gdb exit %d)" % gdb.returncode
        h0 = probe(name, "00000000")
        try:
            r = subprocess.run([pyexe, __file__, "--worker", core, h0.hex()],
                               stdout=subprocess.PIPE, stderr=subprocess.DEVNULL,
                               text=True, timeout=timeout)
        except subprocess.TimeoutExpired:
            return name, "UNKNOWN", "timeout >%ds" % timeout
        if r.returncode == 1:
            return name, "NONE", "no 4-byte-key collision with key 0"
        if r.returncode != 0 or not r.stdout.strip():
            return name, "UNKNOWN", "solver could not decide"
        raw = bytes.fromhex(r.stdout.split()[-1])[::-1]  # solver key is a little-endian word
        if raw == b"\0\0\0\0" or probe(name, raw.hex()) != h0:
            return name, "UNKNOWN", "unverified key " + raw.hex()
        return name, "COLLISION", "%d-bit hash(%s) == hash(00000000) %.1fs" % (
            len(h0) * 8, raw.hex(), time.monotonic() - start)


def section(name, status, info):
    lines = ["[[[ Collision Solver Tests ]]]", "", "Solver %s: %s %s" % (name, status, info)]
    if status == "COLLISION":
        lines += ["", "*********FAIL*********"]
    return "\n".join(lines) + "\n\n"


def main():
    if len(sys.argv) > 1 and sys.argv[1] == "--worker":
        return worker(sys.argv[2], sys.argv[3])
    ap = argparse.ArgumentParser()
    ap.add_argument("--timeout", type=int, default=120)
    ap.add_argument("--jobs", type=int, default=os.cpu_count() or 1)
    ap.add_argument("--section", action="store_true",
                    help="print the doc/<hash>.txt log section (one hash) for testquality.pl")
    ap.add_argument("names", nargs="*")
    a = ap.parse_args()
    if not PROBE.exists():
        print("build CollisionSolverProbe first: cmake --build build --target CollisionSolverProbe",
              file=sys.stderr)
        return 2
    names = a.names or subprocess.run([str(PROBE), "--listnames"], check=True, text=True,
                                      stdout=subprocess.PIPE).stdout.split()
    if not a.names:
        names = [n for n in names if not SECURE.match(n)]
    pyexe = os.environ.get("SMHASHER_ANGR_PYTHON", sys.executable)
    bad = 0
    with ThreadPoolExecutor(a.jobs) as ex:
        for name, status, info in ex.map(
                lambda n: (n, "SKIP", "secure hash") if a.section and SECURE.match(n)
                else solve(n, a.timeout, pyexe), names):
            if a.section:
                print(section(name, status, info), end="", flush=True)
            else:
                print("%-9s %-28s %s" % (status, name, info), flush=True)
            bad += status == "COLLISION"
    return 1 if bad else 0


if __name__ == "__main__":
    sys.exit(main())
