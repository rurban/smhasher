#!/bin/bash
make -C build
./start-bench.sh
if [ -z "$1" ]; then
    test -f log.speed-epyc && mv log.speed-epyc log.speed-epyc.bak
    (for g in `build/SMHasher --listnames`; do \
         build/SMHasher --test=Speed,Hashmap $g 2>&1; done) | tee log.speed-epyc
    ./speed.pl -h=doc/epyc log.speed-epyc
else
    for g in `build/SMHasher --listnames`; do
        for p in $@; do
             if [[ $g =~ $p.* ]]; then
                 build/SMHasher --test=Speed,Hashmap $g 2>&1
             fi
        done
    done | tee "log.speed-epyc-$1"
    ./speed.pl -h=doc/epyc "log.speed-epyc-$1"
fi
./stop-bench.sh
