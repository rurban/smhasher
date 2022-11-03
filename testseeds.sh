#!/bin/sh
make -C build
which performance && performance
for g in `build/SMHasher --listnames | egrep "$@"`; do
    build/SMHasher --extra --test=BadSeeds $g | tee -a doc/$g.txt
    perl -lne '/(Bad|Broken) seed (0x[0-9a-f]+)/ && print $2' doc/$g.txt | sort
done
