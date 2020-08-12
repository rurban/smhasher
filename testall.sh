#!/bin/sh
make -C build
which performance && performance
if [ -z "$@" ]; then
    echo "rather use: ./testspeed.sh; testpar.sh, or even testpar1.sh/testpar2.sh on different machines"
    echo "./testall.sh will need 1.5-4 days to complete (~20m per hash)"
    test -f log.hashes && mv log.hashes log.hashes.bak
    (for g in `build/SMHasher --listnames`; do build/SMHasher $g | tee doc/$g.txt; done) | tee log.hashes
else
    for g in `build/SMHasher --listnames | egrep "$@"`; do
        build/SMHasher $g | tee doc/$g.txt
        ./speed.pl doc/$g.txt && \
            ./fixupdocspeeds.pl doc/$g.txt
    done
fi
