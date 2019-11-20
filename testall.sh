#!/bin/sh
echo "rather use: ./testpar.sh; ./testspeed.sh"
echo "./testall.sh will need 1.5-4 days to complete (~20m per hash)"
make -C build
test -f log.hashes && mv log.hashes log.hashes.bak
(for g in `build/SMHasher --list | cut -s -f1`; do build/SMHasher $g | tee doc/$g; done) | tee log.hashes
