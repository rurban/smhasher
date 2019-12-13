#!/bin/sh
make -C build

echo "marked POOR but no FAIL"
for f in `build/SMHasher --list | perl -alne 'print $F[0] if /POOR$/'`; do
    grep -q FAIL doc/$f || echo "no FAIL $f"; done

echo "marked POOR labeled as GOOD in doc"
for f in `build/SMHasher --list | perl -alne 'print $F[0] if /POOR$/'`; do
    grep GOOD doc/$f; done
