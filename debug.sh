#!/bin/sh
test -d Debug || mkdir Debug
cd Debug
# CC=clang-mp-7.0 CXX=clang++-mp-7.0 
cmake -DCMAKE_BUILD_TYPE=Debug ..
cd ..
make -C Debug VERBOSE=1
