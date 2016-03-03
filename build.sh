#!/bin/sh
rm -rf build
mkdir build
cd build
case `uname -s` in
Darwin) CXX=clang++ CC=clang cmake .. ;;
*)      CXX=g++-5.0  CC=gcc-5.0 cmake ..   ;;
esac
make
cd ..
