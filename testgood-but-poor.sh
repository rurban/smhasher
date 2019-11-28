#!/bin/sh
for f in `build/SMHasher --list | perl -alne 'print $F[0] if /GOOD$/'`; do grep 'FAIL' doc/$f; done

for f in `build/SMHasher --list | perl -alne 'print $F[0] if /GOOD$/'`; do grep '!!!!' doc/$f; done

for f in `build/SMHasher --list | perl -alne 'print $F[0] if /GOOD$/'`; do grep 'POOR' doc/$f; done
