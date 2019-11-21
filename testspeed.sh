#!/bin/sh
make -C build
test -f log.speed && mv log.speed log.speed.bak
which performance && performance
(for g in `build/SMHasher --list | cut -s -f1`; do
    build/SMHasher --test=Speed,Hashmap $g; done) | tee log.speed

./speed.pl && \
./fixupdocspeeds.pl
