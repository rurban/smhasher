#!/bin/sh
make -C build
mkdir partests
build/SMHasher --list|perl -alne'print $F[0] | \
  parallel -j4 --bar 'build/SMHasher --test=Sparse,Permutation,Cyclic,TwoBytes,DiffDist,Text,Zeroes,Seed,Sanity,Avalanche,BIC,LongNeighbors,Diff,MomentChi2 {} >partests/{}'
./fixupdoctests.pl
