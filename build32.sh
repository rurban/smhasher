#!/bin/sh
rm -rf build32
mkdir build32
cd build32
case `uname -s` in
Darwin) CXX="/usr/bin/c++ -m32" CC="/usr/bin/cc -m32" cmake .. ;;
*)      CXX="g++-5.0 -m32" CC="gcc-5.0 -m32" cmake ..   ;;
esac
make
cd ..
