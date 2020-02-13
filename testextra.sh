#!/bin/sh
make -C build
cat extra.lst | parallel -j4 --bar 'build/SMHasher --extra {} >doc/{}'
