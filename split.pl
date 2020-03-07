#!/usr/bin/perl
# ./split.pl log.hashes
undef $/;
$_ = <>;

for $match (split(/-------------------------------------------------------------------------------/)) {
  $match =~ m/^--- Testing (\w+)/m;
  unless ($seen{$1}) {
    open(O, ">doc/$1.txt") and print "doc/$1.txt\n";
    $seen{$1}++;
    print O $match;
    close(O);
  }
}
