#!/bin/sh
make -s
test -f log.hashes && mv log.hashes log.hashes.bak
(for g in `./SMHasher --list | cut -s -f1`; do ./SMHasher $g; done) > log.hashes
