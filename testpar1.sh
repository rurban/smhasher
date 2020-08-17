#!/bin/sh
make -C build
if test -d partests1
then
    rm partests1/*
else
    mkdir partests1
fi
test -n "$@" && r="if /$@/"
build/SMHasher --list | perl -alne"print \$F[0] $r" | \
    parallel -j4 --bar \
      'build/SMHasher --test=Sparse,Permutation,Cyclic,TwoBytes,DiffDist,Text,Zeroes,Seed,PerlinNoise {} >partests1/{}.txt 2>&1'

./fixupdoctests.pl partests1/*.txt
