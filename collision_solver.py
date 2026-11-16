#!/usr/bin/env python3
"""Find and independently execute a 10-bit PMurHash32 collision with CBMC."""
import json
import os
from pathlib import Path
import shlex
import subprocess
import sys
import tempfile
import time


def main():
    root = Path(__file__).resolve().parent
    harness = root / "collision_solver_cbmc.c"
    source = root / "PMurHash.c"
    cmd = ["cbmc", "-I", str(root), str(harness), str(source), "--function", "main",
           "--property", "main.assertion.1", "--little-endian", "--unwind", "10",
           "--unwinding-assertions", "--trace", "--json-ui"]
    start = time.monotonic()
    solved = subprocess.run(cmd, stdout=subprocess.PIPE, stderr=subprocess.PIPE, text=True)
    elapsed = time.monotonic() - start
    if solved.returncode != 10:
        raise RuntimeError("CBMC did not produce a collision: " + solved.stderr.strip())
    report = json.loads(solved.stdout)
    results = [entry for group in report for entry in group.get("result", [])
               if entry.get("property") == "main.assertion.1"]
    if len(results) != 1 or results[0].get("status") != "FAILURE":
        raise RuntimeError("CBMC did not report the collision property")
    values = {}
    for step in results[0].get("trace", []):
        if step.get("lhs") in ("a", "b") and step.get("value", {}).get("data"):
            values[step["lhs"]] = int(step["value"]["data"].rstrip("uUlL"), 0)
    if set(values) != {"a", "b"} or values["a"] == values["b"]:
        raise RuntimeError("CBMC trace does not contain two distinct keys")
    with tempfile.TemporaryDirectory(prefix="smhasher-collision-") as tmp:
        verifier = Path(tmp) / "verify"
        compiler = shlex.split(os.environ.get("CC", "cc"))
        subprocess.run(compiler + ["-std=c99", "-O2", "-I", str(root), str(harness),
                                   str(source), "-o", str(verifier)], check=True)
        verified = subprocess.run([str(verifier), str(values["a"]), str(values["b"])],
                                  check=True, text=True, stdout=subprocess.PIPE)
    print("PMurHash32, known seed 0: 1 pair with equal low 10 bits")
    print("keys:", values["a"].to_bytes(4, "little").hex(), values["b"].to_bytes(4, "little").hex())
    print(verified.stdout.strip())
    print("CBMC solve time: %.6f seconds" % elapsed)
    return 0


if __name__ == "__main__":
    try:
        sys.exit(main())
    except (OSError, ValueError, KeyError, RuntimeError, subprocess.CalledProcessError) as error:
        print("collision solver:", error, file=sys.stderr)
        sys.exit(1)
