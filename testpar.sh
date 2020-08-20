#!/bin/sh
make -C build
if test -d partests
then
    rm partests/*
else
    mkdir partests
fi

# all tests with the given hashes
test -n "$@" && r="if /$@/"
#       'build/SMHasher --test=Sparse,Permutation,Cyclic,TwoBytes,DiffDist,Text,Zeroes,Seed,PerlinNoise,Sanity,Avalanche,BIC,Diff,MomentChi2,Prng {} 2>&1 >partests/{}'

build/SMHasher --list|perl -alne"print \$F[0] $r" | \
    parallel -j4 --bar \
      'build/SMHasher {} 2>&1 >partests/{}.txt'

./fixupdoctests.pl partests/*.txt
