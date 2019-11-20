#!/bin/sh
make -C build
mv log.hashmap log.hashmap~;
for g in `build/SMHasher --list | cut -s -f1 | tail -n +15`
  do build/SMHasher --test=Hashmap $g
done | tee log.hashmap

./speed.pl log.hashmap && \
./fixupdocspeeds.pl log.hashmap
