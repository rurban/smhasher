#!/bin/sh
echo "rather use: ./testspeed.sh; testpar.sh, or even testpar1.sh/testpar2.sh on different machines"
echo "./testall.sh will need 1.5-4 days to complete (~20m per hash)"
make -C build
test -f log.hashes && mv log.hashes log.hashes.bak
(for g in `build/SMHasher --listnames`; do build/SMHasher $g | tee doc/$g.txt; done) | tee log.hashes
