#!/bin/sh
make -C build
which performance && performance
for g in `build/SMHasher --listnames | egrep "$@"`; do
    build/SMHasher --extra --test=BadSeeds $g
done
