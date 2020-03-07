#!/bin/sh
make -C build
if test -d partests2
then
    rm partests2/*
else
    mkdir partests2
fi
test -n "$@" && r="if /$@/"
build/SMHasher --list|perl -alne"print \$F[0] $r" | \
    parallel -j4 --bar \
      'build/SMHasher --test=Sanity,Avalanche,Seed,Perlin,Diff,BIC,MomentChi2,Prng {} 2>&1 >partests2/{}'

./fixupdoctests.pl partests2/*
