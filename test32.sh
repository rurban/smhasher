#!/bin/sh
make -C build32
which performance && performance
for g in $@; do
    build32/SMHasher $g | tee doc/$g.txt
    ./speed.pl -h=doc/i686 doc/$g.txt
    build32/SMHasher --extra --test=BadSeeds $g | tee -a doc/$g.txt
    perl -lne '/(Bad|Broken) seed (0x[0-9a-f]+)/ && print $2' doc/$g.txt | sort
done
