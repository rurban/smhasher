#!/bin/bash
make -C build
./start-bench.sh
if [ -z "$1" ]; then
    test -f log.speed-i7 && mv log.speed-i7 log.speed-i7.bak
    (for g in `build/SMHasher --listnames`; do \
         build/SMHasher --test=Speed,Hashmap $g 2>&1; done) | tee log.speed-i7
    ./speed.pl -h=doc/i7 log.speed-i7
else
    for g in `build/SMHasher --listnames`; do
        for p in $@; do
             if [[ $g =~ $p.* ]]; then
                 build/SMHasher --test=Speed,Hashmap $g 2>&1
             fi
        done
    done | tee "log.speed-i7-$1"
    ./speed.pl -h=doc/i7 "log.speed-i7-$1"
fi
./stop-bench.sh
