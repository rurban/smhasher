#!/bin/sh
make -C build
mkdir partests2
build/SMHasher --list|perl -alne'print $F[0] | \
  parallel -j4 --bar 'build/SMHasher --test=Seed,Sanity,Avalanche,BIC,LongNeighbors,Diff,MomentChi2 {} >partests2/{}'
./fixupdoctests.pl partests2/*
