#!/bin/sh
set -e
echo build   64bit
make -j4 -C build && build/SMHasher --verbose --test=VerifyAll
echo build32 32bit
make -j4 -C build32 && build32/SMHasher --verbose --test=VerifyAll
