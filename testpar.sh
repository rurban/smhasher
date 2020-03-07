#!/bin/sh
make -C build
if test -d partests
then
    rm partests/*
else
    mkdir partests
fi
test -n "$@" && r="if /$@/"
build/SMHasher --list|perl -alne"print \$F[0] $r" | \
    parallel -j4 --bar \
      'build/SMHasher --test=Sparse,Permutation,Cyclic,TwoBytes,DiffDist,Text,Zeroes,Seed,PerlinNoise,Sanity,Avalanche,BIC,Diff,MomentChi2,Prng {} 2>&1 >partests/{}'

./fixupdoctests.pl
