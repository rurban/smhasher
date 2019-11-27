#!/bin/sh
make -C build
mkdir partests1
build/SMHasher --list|perl -alne'print $F[0] | \
  parallel -j4 --bar 'build/SMHasher --test=Sparse,Permutation,Cyclic,TwoBytes,DiffDist,Text,Zeroes {} >partests1/{}'
./fixupdoctests.pl partests1/*
