#!/bin/bash
make -C build32
./start-bench.sh
if [ -z "$@" ]; then
    test -f log.speed32 && mv log.speed32 log.speed32.bak
    (for g in `build32/SMHasher --listnames`; do \
         build32/SMHasher --test=Speed,Hashmap $g 2>&1; done) | tee log.speed32
    ./speed.pl -h=doc/i686 log.speed32
else
    for g in `build32/SMHasher --listnames`; do
        for p in $@; do
             if [[ $g =~ $p.* ]]; then
                 build32/SMHasher --test=Speed,Hashmap $g 2>&1
             fi
        done
     done | tee "log.speed32-$1"
    ./speed.pl -h=doc/i686 "log.speed32-$1"
fi
./stop-bench.sh
