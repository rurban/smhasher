#!/usr/bin/env python3
"""Compare SMHasher Speed results from two separately compiled executables."""

import argparse
import re
import subprocess
import sys


def available(executable):
    result = subprocess.run([executable, "--listnames"], check=True,
                            text=True, stdout=subprocess.PIPE)
    return set(result.stdout.splitlines())


def measure(executable, name):
    result = subprocess.run([executable, "--test=Speed", name], text=True,
                            stdout=subprocess.PIPE, stderr=subprocess.PIPE)
    if result.returncode:
        raise RuntimeError("exit %d: %s" % (result.returncode, result.stderr.strip()))
    bulk = re.search(r"^Average\s+-\s+([0-9.]+) bytes/cycle", result.stdout, re.M)
    small = re.search(r"^Average\s+([0-9.]+) cycles/hash", result.stdout, re.M)
    if not bulk or not small:
        raise RuntimeError("missing speed averages")
    return bulk.group(1), small.group(1)


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("gcc", help="GCC-built SMHasher executable")
    parser.add_argument("clang", help="Clang-built SMHasher executable")
    parser.add_argument("hashes", nargs="*", help="hash names (default: all available)")
    args = parser.parse_args()
    executables = (args.gcc, args.clang)
    supported = tuple(available(exe) for exe in executables)
    names = sorted(set(args.hashes) if args.hashes else supported[0] | supported[1])
    unknown = set(names) - (supported[0] | supported[1])
    if unknown:
        parser.error("unknown hashes: " + ", ".join(sorted(unknown)))
    print("| Hash | GCC bulk B/tick | Clang bulk B/tick | GCC short ticks/hash | Clang short ticks/hash |")
    print("|---|---:|---:|---:|---:|")
    failed = False
    for index, name in enumerate(names):
        measurements = [None, None]
        for compiler in ((0, 1) if index % 2 == 0 else (1, 0)):
            if name not in supported[compiler]:
                continue
            try:
                measurements[compiler] = measure(executables[compiler], name)
            except RuntimeError as error:
                print("%s: %s: %s" % (executables[compiler], name, error), file=sys.stderr)
                measurements[compiler] = ("ERROR", "ERROR")
                failed = True
        gcc, clang = (value or ("—", "—") for value in measurements)
        print("| %s | %s | %s | %s | %s |" % (name, gcc[0], clang[0], gcc[1], clang[1]), flush=True)
    return 1 if failed else 0


if __name__ == "__main__":
    sys.exit(main())
