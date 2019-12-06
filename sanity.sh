#!/bin/sh
./build.sh
(for g in `build/SMHasher --listnames`; do build/SMHasher --test=Sanity $g; done) \
	| tee log.sanity
./build32.sh
(for g in `build32/SMHasher --listnames`; do build32/SMHasher --test=Sanity $g; done) \
	| tee -a log.sanity
