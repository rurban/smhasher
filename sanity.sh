#!/bin/sh
./build.sh
(for g in `build/SMHasher --list | cut -s -f1`; do build/SMHasher --test=Sanity $g; done) \
	| tee log.sanity
./build32.sh
(for g in `build32/SMHasher --list | cut -s -f1`; do build32/SMHasher --test=Sanity $g; done) \
	| tee -a log.sanity
