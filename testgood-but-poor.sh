#!/bin/sh
make -C build
echo "marked GOOD but FAIL (ignore HashMapTest and LongNeighbors)"
for f in `build/SMHasher --list | perl -alne 'print $F[0] if /GOOD$/'`; do
    grep 'FAIL' doc/$f.txt && echo $f; done

echo "marked GOOD but !!!!"
for f in `build/SMHasher --list | perl -alne 'print $F[0] if /GOOD$/'`; do
    grep -q '!!!!' doc/$f.txt && echo $f; done

echo "marked GOOD but POOR in doc"
for f in `build/SMHasher --list | perl -alne 'print $F[0] if /GOOD$/'`; do
    grep 'POOR' doc/$f.txt; done
