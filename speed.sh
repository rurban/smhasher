#!/bin/sh
LOG=${1:-log.speed}
perl -ne'/^--- Testing ([\w-]+) "/ && print "| [$1](doc/$1.txt)".(" "x(36-(2*length($1)))); /^Average.+ - +([\d\.]+) MiB\/sec/ && printf("  | %12.2f",$1); /^Average +(\d.*)cycles\/hash/ && printf(" | %8.2f",$1); /^Running fast HashMapTest: +(\d\S+) cycles\/op \((\d.+) stdv\)/ && printf(" | %6.2f (%.0f) |\n",$1,$2);' $LOG
