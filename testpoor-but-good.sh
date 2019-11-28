#!/bin/sh
for f in `build/SMHasher --list | perl -alne 'print $F[0] if /POOR$/'`; do grep -l 'FAIL' doc/$f >/dev/null || echo good $f; done

for f in `build/SMHasher --list | perl -alne 'print $F[0] if /POOR$/'`; do grep GOOD doc/$f; done
