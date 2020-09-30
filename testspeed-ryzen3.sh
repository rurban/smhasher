#!/bin/bash
make -C build
which performance && performance
if [ -z "$1" ]; then
    test -f log.speed && mv log.speed log.speed.bak
    (for g in `build/SMHasher --listnames`; do \
         build/SMHasher --test=Speed,Hashmap $g 2>&1; done) | tee log.speed-ryzen3
    ./speed.pl -h=doc/ryzen3 log.speed-ryzen3
else
    for g in `build/SMHasher --listnames`; do
        for p in $@; do
             if [[ $g =~ $p.* ]]; then
                 build/SMHasher --test=Speed,Hashmap $g 2>&1
             fi
        done
    done | tee "log.speed-ryzen3-$1"
    ./speed.pl -h=doc/ryzen3 "log.speed-ryzen3-$1"
fi
