#!/bin/bash
ls doc/*.out | xargs perl r2p.pl --name "All Hashes"
ls doc/Jenkins*.out | xargs perl r2p.pl
ls doc/*OAAT*.out | xargs perl r2p.pl
ls doc/*Mur*.out | grep -v OAAT | xargs perl r2p.pl --name "Murmur Hashes"
ls doc/cmetro*.out doc/metro*.out | xargs perl r2p.pl --name "MetroHash Family"
ls doc/City*.out | xargs perl r2p.pl
ls doc/Farm*.out doc/farm*.out | xargs perl r2p.pl
ls doc/t1ha*.out | xargs perl r2p.pl



