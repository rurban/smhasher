#!/bin/bash
perl summarize.pl > doc/summary.txt
ls doc/*.out | xargs perl r2p.pl --name "All Hashes"
ls doc/Jenkins*.out | xargs perl r2p.pl
ls doc/*OAAT*.out | xargs perl r2p.pl
ls doc/*Mur*.out | grep -v OAAT | xargs perl r2p.pl --name "Murmur Hashes"
ls doc/cmetro*.out doc/metro*.out | xargs perl r2p.pl --name "MetroHash Family"
ls doc/City*.out | xargs perl r2p.pl
ls doc/Farm*.out doc/farm*.out | xargs perl r2p.pl
ls doc/t1ha*.out | xargs perl r2p.pl
ls doc/City*.64.out doc/*metro*.64.out doc/StadtX.*.64.out doc/Zaphod64.*.64.out doc/Sip*.64.out doc/Farm*.64.out doc/t1ha*.64.out doc/xxHash64.*.64.out | xargs perl r2p.pl --name "Selected 64 Bit"
ls $(grep PASSED doc/summary.txt | cut -d' ' -f1 | perl -lne'print "doc/$_.*.out"')  | xargs perl r2p.pl --name 'Passed All Tests'
ls doc/sdbm.*.out doc/*Sip*.out doc/StadtX.*.out doc/Zaphod*.out  | xargs perl r2p.pl --name 'SDBM Selected Comparison'


