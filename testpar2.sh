#!/bin/sh
make -C build
mkdir partests2
test -n "$@" && r="if /$@/"
build/SMHasher --list|perl -alne"print \$F[0] $r" | \
    parallel -j4 --bar \
      'build/SMHasher --test=Seed,Sanity,Avalanche,BIC,Diff,MomentChi2 {} 2>&1 >partests2/{}'

./fixupdoctests.pl partests2/*
