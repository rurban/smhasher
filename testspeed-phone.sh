#!/bin/sh
if [ "x`uname -m`" != "xaarch64" ]; then
    make -C build-aarch64
    scp build-aarch64/SMHasher phone:Software/smhasher/
    ssh phone "cd Software/smhasher && ./testspeed-phone.sh $@"
    scp phone:Software/smhasher/log.speed-phone* .
else
    if [ -z "$@" ]
    then
        test -f log.speed-phone && mv log.speed-phone log.speed-phone.bak
        (for g in `./SMHasher --listnames`; do \
             ./SMHasher --test=Speed,Hashmap $g 2>&1; done) | tee log.speed-phone
        #./speed.pl -h=doc/air log.speed-phone
    else
        (for g in `./SMHasher --listnames | egrep "$@"`; do \
             ./SMHasher --test=Speed,Hashmap $g 2>&1; done) | tee "log.speed-phone-$1"
        #./speed.pl -h=doc/air "log.speed-phone-$1"
    fi
fi
