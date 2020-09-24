#!/bin/sh
rm -rf build32
mkdir build32
cd build32
case `uname -s` in
Darwin) CXX="/usr/bin/c++ -m32" CC="/usr/bin/cc -m32" cmake .. $@ ;;
*)      CXX="i686-linux-gnu-g++" CC="i686-linux-gnu-gcc" cmake .. $@ ;;
esac
make
cd ..
