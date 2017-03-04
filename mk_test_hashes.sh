#!/bin/bash
echo "make"
echo "rm -f doc/*.tmp"
echo "export BITS=32"
./SMHasher --list | sort -k 4n -k 1 |
    grep -v '^crc' | 
    grep -v -i 'crap\|nothing\|bad\|sum\|sha1\|md5' |
    perl -ne'@F=split/[\s|]+/,$_; /[A-Z]/ and $F[2]=~/\d/ and print qq(
#$_
if [ "x\$BITS" == "x" -o "x\$BITS" == "x$F[2]" ]; then
    touch doc/$F[0].$F[2].out &&
    ./SMHasher $F[0] 2>&1 | tee doc/$F[0].$F[2].tmp &&
    mv doc/$F[0].$F[2].out doc/$F[0].$F[2].bak &&
    mv doc/$F[0].$F[2].tmp doc/$F[0].$F[2].out
fi

)'

