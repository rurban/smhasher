#!/bin/sh
make -C build
mkdir partests1
test -n "$@" && r="if /$@/"
build/SMHasher --list|perl -alne"print \$F[0] $r" | \
    parallel -j4 --bar \
      'build/SMHasher --test=Sparse,Permutation,Cyclic,TwoBytes,DiffDist,Text,Zeroes {} 2>&1 >partests1/{}'

./fixupdoctests.pl partests1/*
