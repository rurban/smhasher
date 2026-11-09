#!/bin/sh
make -C build
which performance && performance
# same machine dispatch as testspeed.sh: speed.pl must update the per-machine html
hname=`hostname`
H=""
if [ x$hname = xe495-reini ] || [ x$hname = xe495 ]; then
    H="-h=doc/ryzen3"
elif [ x$hname = xreinip50 ]; then
    H="-h=doc/i7"
elif [ x$hname = xmamba ]; then
    H="-h=doc/epyc"
elif [ x$hname = xairc ]; then
    H="-h=doc/air"
elif [ x`uname -m` = xaarch64 ]; then
    H="-h=doc/phone"
elif [ x$hname = xreini ]; then
    H="-h=doc/intel"
fi
if test -z "$@"; then
    echo "rather use: ./testspeed.sh; testpar.sh, or even testpar1.sh/testpar2.sh on different machines"
    echo "./testall.sh will need 1.5-4 days to complete (~20m per hash)"
    test -f log.hashes && mv log.hashes log.hashes.bak
    (for g in `build/SMHasher --listnames`; do
         build/SMHasher $g | tee doc/$g.txt;
        ./testseeds.sh $g;
     done) | tee log.hashes
else
    for g in $@; do
        build/SMHasher $g | tee doc/$g.txt
        ./speed.pl $H doc/$g.txt && \
            ./fixupdocspeeds.pl doc/$g.txt
        ./testseeds.sh $g
        ./testquality.pl doc/$g.txt
    done
fi
