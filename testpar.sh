#!/bin/sh
make -C build
mkdir partests
test -n "$@" && r="if /$@/"
build/SMHasher --list|perl -alne"print \$F[0] $r" | \
    parallel -j4 --bar \
      'build/SMHasher --test=Sparse,Permutation,Cyclic,TwoBytes,DiffDist,Text,Zeroes,Seed,Sanity,Avalanche,BIC,Diff,MomentChi2 {} 2>&1 >partests/{}'

./fixupdoctests.pl
