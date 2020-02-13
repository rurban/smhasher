#!/bin/sh
make -C build
test -f log.speed && mv log.speed log.speed.bak
(for g in `build/SMHasher --listnames`; do build/SMHasher --test=Speed $g; done) | tee log.speed
