#!/bin/bash
make -C build
./start-bench.sh
if [ -z "$1" ]; then
    test -f log.speed-intel && mv log.speed-intel log.speed-intel.bak
    (for g in `build/SMHasher --listnames`; do \
         build/SMHasher --test=Speed,Hashmap $g 2>&1; done) | tee log.speed-intel
    ./speed.pl -h=doc/intel log.speed-intel
else
    for g in `build/SMHasher --listnames`; do
        for p in $@; do
             if [[ $g =~ $p.* ]]; then
                 build/SMHasher --test=Speed,Hashmap $g 2>&1
             fi
        done
    done | tee "log.speed-ryzen3-$1"
    ./speed.pl -h=doc/intel "log.speed-intel-$1"
fi
./stop-bench.sh
