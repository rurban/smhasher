#!/bin/sh
LOG=${1:-log.hashes}
perl -ne'/^--- Testing (\w+)/ && print $1.(" "x(16-length $1)); /Alignment  0.* ([\d\.]+ MiB.*)/ && print "\t".$1; /-   10-byte keys - (.*)/ && print "\t$1 10-byte keys\n"' $LOG
