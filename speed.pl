#!/usr/bin/perl
use strict;
use File::Copy 'mv';
my $ln = $ARGV ? shift : "log.speed";
open(my $l, "<", $ln) or die "open $ln $!";
my ($n,$bulk,$small,$hash);
while (<$l>) {
  if (/^--- Testing (\w+) \"/) {
    if ($n) {
      fixup($n,$bulk,$small,$hash);
    }
    $n = $1; $bulk = $small = $hash = undef;
  } elsif (/^Average.+ - +([\d\.]+) MiB\/sec/) {
    $bulk = sprintf("%12.2f",$1);;
  } elsif (/^Average +(\d.*)cycles\/hash/) {
    $small = sprintf("%8.2f",$1);
  } elsif (/^Running HashMapTest: +(\d\S+) cycles\/op \((\d.+) stdv\)/) {
    $hash = sprintf("%6.2f (%.0f)", $1, $2);
  } elsif (/^Running HashMapTest: SKIP/) {
    $hash = "too slow";
  }
}
fixup($n,$bulk,$small,$hash) if $n;

sub fixup {
  my ($n,$bulk,$small,$hash) = @_;
  open(my $I, "<", "README.md") or die "open README.md $!";
  open(my $O, ">", "README.md.new") or die "open README.md.new $!";
  my $found;
  while (<$I>) {
    # search for $n in README.md
    if (/^\| \[$n\]/) {
      # get old values, update line
      $hash = "   -    " unless defined $hash;
      my $spc = " " x (38-(2*length($n)));
      s/$n\)\s*\|\s+\d[\d\.]+\s*\|\s+\d[\d\.]+\s+\|\s+(\d[\d\.]+ \(\d+\)|skipped|too slow|-)\s*\|/$n)$spc| $bulk | $small | $hash |/;
      $found++;
    }
    print $O $_;
  }
  if (!$found) {
  }
  close $I;
  close $O;
  mv ("README.md.new", "README.md") if $found;
}
