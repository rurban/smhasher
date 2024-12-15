#!/bin/sh
set -e
echo build   64bit
make -j4 -C build && build/SMHasher --verbose --test=VerifyAll
if test -d build32; then
    echo build32 32bit
    make -j4 -C build32 && build32/SMHasher --verbose --test=VerifyAll
fi
echo build-debug with asan
test -d build-debug || cmake -S . -B build-debug -DCMAKE_RELEASE_TYPE=Debug
make -j4 -C build-debug && build-debug/SMHasher --verbose --test=VerifyAll
