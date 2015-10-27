#!/bin/sh
rm -rf build
mkdir build
cd build
case `uname -s` in
Darwin) CXX=g++-mp-5 CC=gcc-mp-5 cmake .. ;;
*)      CXX=g++-5.0  CC=gcc-5.0 cmake ..   ;;
esac
make
cd ..
