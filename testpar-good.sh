#!/bin/sh
make -C build
build/SMHasher --list | perl -alne 'print $F[0] if /GOOD$/' | \
    parallel -j4 --bar 'build/SMHasher {} >doc/{}.txt'
