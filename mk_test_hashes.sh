#!/bin/bash
echo "make"
echo "rm -f doc/*.tmp"
echo "export BITS=32"
./SMHasher --list | 
    sort -k 2n -k 1 | 
    grep -v '^crc' | 
    grep -v -i 'crap\|nothing\|bad\|sum\|sha1\|md5' |
    perl -ane'print qq(
#$_
if [ "x\$BITS" == "x" -o "x\$BITS" == "x$F[1]" ]; then    
    touch doc/$F[0]_$F[1].out &&
    ./SMHasher $F[0] 2>&1 | tee doc/$F[0]_$F[1].tmp &&
    mv doc/$F[0]_$F[1].out doc/$F[0]_$F[1].bak &&
    mv doc/$F[0]_$F[1].tmp doc/$F[0]_$F[1].out
fi

)'

