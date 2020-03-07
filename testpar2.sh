#!/bin/sh
make -C build
mkdir partests2
test -n "$@" && r="if /$@/"
build/SMHasher --list|perl -alne"print \$F[0] $r" | \
    parallel -j4 --bar \
      'build/SMHasher --test=Sanity,Avalanche,Seed,Perlin,Diff,BIC,MomentChi2,Prng {} 2>&1 >partests2/{}'

./fixupdoctests.pl partests2/*
