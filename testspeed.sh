#!/bin/bash
hname="`hostname`"
if [ x$hname = xe495 ]; then
    ./testspeed-ryzen3.sh $@
    exit
fi
#if [ x$hname = xlorikeet ]; then
#    ./testspeed-ryzen5.sh $@
#    exit
#fi
if [ x$hname = xreini ]; then
    ./testspeed-intel.sh $@
    exit
fi
if [ x$hname = xairc ]; then
    ./testspeed-air.sh $@
    exit
fi
if [ x`uname -m` = xaarch64 ]; then
    ./testspeed-phone.sh $@
    exit
fi

make -C build
./start-bench.sh
if [ -z "$@" ]; then
    test -f log.speed && mv log.speed log.speed.bak
    (for g in `build/SMHasher --listnames`; do \
         build/SMHasher --test=Speed,Hashmap $g 2>&1; done) | tee log.speed
    ./speed.pl && \
        ./fixupdocspeeds.pl
else
    for g in `build/SMHasher --listnames`; do
        for p in $@; do
             if [[ $g =~ $p.* ]]; then
                 build/SMHasher --test=Speed,Hashmap $g 2>&1
             fi
        done
    done | tee "log.speed-$1"
    ./speed.pl "log.speed-$1" && \
        ./fixupdocspeeds.pl "log.speed-$1"
fi
./stop-bench.sh
