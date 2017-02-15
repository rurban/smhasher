#!/bin/sh
make -C build
test -f log.hashes && mv log.hashes log.hashes.bak
(for g in `build/SMHasher --list | cut -s -f1`; do build/SMHasher --test=Speed $g; done) | tee log.speed
./speed.sh
