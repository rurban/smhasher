#!/bin/bash
make -C build
./start-bench.sh
if [ -z "$1" ]; then
    test -f log.speed-i7-6820 && mv log.speed-i7-6820 log.speed-i7-6820.bak
    (for g in `build/SMHasher --listnames`; do \
         build/SMHasher --test=Speed,Hashmap $g 2>&1; done) | tee log.speed-i7-6820
    ./speed.pl -h=doc/i7 log.speed-i7-6820
else
    for g in `build/SMHasher --listnames`; do
        for p in $@; do
             if [[ $g =~ $p.* ]]; then
                 build/SMHasher --test=Speed,Hashmap $g 2>&1
             fi
        done
    done | tee "log.speed-i7-6820-$1"
    ./speed.pl -h=doc/i7 "log.speed-i7-6820-$1"
fi
./stop-bench.sh
