#!/bin/sh
make -C build
which performance && performance
if [ -z "$@" ]; then
    test -f log.speed && mv log.speed log.speed.bak
    (for g in `build/SMHasher --listnames`; do \
         build/SMHasher --test=Speed,Hashmap $g 2>&1; done) | tee log.speed-ryzen3
    ./speed.pl -h=doc/ryzen3 log.speed-ryzen3
else
     (for g in `build/SMHasher --listnames | egrep "$@"`; do \
         build/SMHasher --test=Speed,Hashmap $g 2>&1; done) | tee "log.speed-ryzen3-$1"
    ./speed.pl -h=doc/ryzen3 "log.speed-ryzen3-$1"
fi
