#!/bin/sh
make -C build
which performance && performance
if [ -z "$@" ]; then
    test -f log.speed && mv log.speed log.speed.bak
    (for g in `build/SMHasher --list | perl -alne'print $F[0]'`; do
         build/SMHasher --test=Speed,Hashmap $g; done) | tee log.speed
    ./speed.pl && \
        ./fixupdocspeeds.pl
else
     r="if /$@/"
     (for g in `build/SMHasher --list | perl -alne"print \$F[0] $r"`; do
         build/SMHasher --test=Speed,Hashmap $g; done) | tee log.speed-$1
    ./speed.pl log.speed-$1 && \
        ./fixupdocspeeds.pl log.speed-$1
fi

