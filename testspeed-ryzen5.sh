#!/bin/bash
make -C build
./start-bench.sh
if [ -z "$1" ]; then
    test -f log.speed && mv log.speed log.speed.bak
    (for g in `build/SMHasher --listnames`; do \
         build/SMHasher --test=Speed,Hashmap $g 2>&1; done) | tee log.speed
    ./speed.pl log.speed
else
    for g in `build/SMHasher --listnames`; do
        for p in $@; do
             if [[ $g =~ $p.* ]]; then
                 build/SMHasher --test=Speed,Hashmap $g 2>&1
             fi
        done
    done | tee "log.speed-ryzen5-$1"
    ./speed.pl "log.speed-ryzen5-$1"
fi
./stop-bench.sh
