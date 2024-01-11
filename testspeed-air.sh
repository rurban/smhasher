#!/bin/bash
make -C build
./start-bench.sh
if [ -z "$@" ]
then
    test -f log.speed && mv log.speed log.speed.bak
    (for g in `build/SMHasher --listnames`; do \
         build/SMHasher --test=Speed,Hashmap $g 2>&1; done) | tee log.speed-air
    ./speed.pl -h=doc/air log.speed-air
else
    for g in `build/SMHasher --listnames`; do
        for p in $@; do
             if [[ $g =~ $p.* ]]; then
                 build/SMHasher --test=Speed,Hashmap $g 2>&1
             fi
        done
    done | tee "log.speed-air-$1"
    ./speed.pl -h=doc/air "log.speed-air-$1"
fi
./stop-bench.sh
