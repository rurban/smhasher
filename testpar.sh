#!/bin/sh
make -C build
build/SMHasher --list | perl -alne 'print $F[0]' | \
    parallel -j4 --bar 'build/SMHasher {} >doc/{}'
