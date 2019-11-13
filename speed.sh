#!/bin/sh
LOG=${1:-log.speed}
perl -ne'/^--- Testing (\w+) "/ && print "| [$1](doc/$1)".(" "x(32-(2*length($1)))); /^Average.+ - +([\d\.]+) MiB\/sec/ && printf("| %12.2f",$1); /^Average +(\d.*)cycles\/hash/ && printf("\t| %8.2f",$1); /^Running HashMapTest: +(\d\S+) cycles\/op \((\d.+) stdv\)/ && printf("\t| %6.2f (%.0f)|\n",$1,$2)' $LOG
