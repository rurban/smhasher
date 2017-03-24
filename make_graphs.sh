#!/bin/bash
perl summarize.pl > doc/summary.txt
ls doc/*.txt | xargs perl r2p.pl --name "All Hashes"
ls doc/Jenkins*.txt | xargs perl r2p.pl
ls doc/*OAAT*.txt | xargs perl r2p.pl
ls doc/*Mur*.txt | grep -v OAAT | xargs perl r2p.pl --name "Murmur Hashes"
ls doc/StadtX*.txt doc/cmetro*.txt doc/metro*.txt | xargs perl r2p.pl --name "MetroHash Family"
ls doc/City*.txt | xargs perl r2p.pl
ls doc/Farm*.txt doc/farm*.txt | xargs perl r2p.pl
ls doc/Sip*.txt doc/TSip*.txt | xargs perl r2p.pl --name "SipHash Family"
ls doc/t1ha*.txt | xargs perl r2p.pl
ls doc/City*.64.txt doc/*metro*.64.txt doc/StadtX.*.64.txt doc/Zaphod64.*.64.txt doc/Sip*.64.txt doc/Farm*.64.txt doc/t1ha*.64.txt doc/xxHash64.*.64.txt | xargs perl r2p.pl --name "Selected 64 Bit"
ls $(grep PASSED doc/summary.txt | cut -d' ' -f1 | grep -v sha1 | perl -lne'print "doc/$_.*.txt"')  | xargs perl r2p.pl --name 'Passed All Tests'
ls doc/sdbm.*.txt doc/*Sip*.txt doc/StadtX.*.txt doc/Zaphod*.txt  | xargs perl r2p.pl --name 'SDBM Selected Comparison'
ls doc/Jenkins*.txt doc/Sip*.txt doc/StadtX*.txt | xargs perl r2p.pl --name 'Perl Hash Comparison'
ls doc/Sip*.txt doc/Jenkins*.txt doc/Lua*.txt | xargs perl r2p.pl --name 'Lua53'
ls doc/Sip*.txt doc/StadtX*.txt doc/Zaphod64.*.txt | xargs perl r2p.pl --name 'Perl 64 bit Candidates'
ls doc/Zaphod32.*.txt doc/Phat*.txt | xargs perl r2p.pl --name 'Perl 32 bit Candidates'
ls doc/FNV*.txt doc/bernstein*.txt doc/StadtX.*.txt doc/Zaphod*.txt doc/Sip*.txt | xargs perl r2p.pl --name 'FNV Comparison'

