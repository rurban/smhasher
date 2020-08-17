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
      'build/SMHasher --test=Sanity,Avalanche,Diff,BIC,MomentChi2,Prng {} >partests2/{}.txt 2>&1'

./fixupdoctests.pl partests2/*.txt
