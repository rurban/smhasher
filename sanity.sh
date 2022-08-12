#!/bin/sh
./build.sh
(for g in `build/SMHasher --listnames`; do build/SMHasher --test=Sanity $g; done) \
	2>&1 | tee log.sanity
./build32.sh
(for g in `build32/SMHasher --listnames`; do build32/SMHasher --test=Sanity $g; done) \
	2>&1 | tee -a log.sanity
make -C build-asan
(for g in `build-asan/SMHasher --listnames`; do build-asan/SMHasher --test=Sanity $g; done) \
	2>&1 | tee -a log.sanity
