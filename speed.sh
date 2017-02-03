#!/bin/sh
LOG=${1:-log.speed}
perl -ne'/^--- Testing (\w+) "/ && print "| $1".(" "x(22-length $1)); /^Average.+ - +([\d\.]+) MiB\/sec/ && printf("\t| %12.2f",$1); /^Average +(\d.*)cycles\/hash/ && printf("\t| %8.2f\n",$1)' $LOG
