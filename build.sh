#!/bin/sh
rm -rf build
mkdir build
cd build
# Note that gcc-mp-5 on darwin is still ~2000 MiB/sec faster than the system clang
case `uname -s` in
Darwin)  cmake .. ;;
FreeBSD) CXX=clang++ CC=clang cmake .. ;;
*)       CXX=clang++-4.0  CC=clang-4.0 cmake ..   ;;
esac
make -j4
cd ..
