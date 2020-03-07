#!/usr/bin/perl
use strict;
use File::Copy 'mv';
my $endtest = qr(^(?:---|\[\[\[ |Input vcode 0x));
# mkdir partests; build/SMHasher --list|perl -alne'print $F[0] | parallel -j4 --bar 'build/SMHasher --test=Sparse,Permutation,Cyclic,TwoBytes,DiffDist,Text,Zeroes,Seed,Sanity,Avalanche,BIC,LongNeighbors,Diff,MomentChi2 {} >partests/{}'
my @keysettests = qw(Sparse Permutation Cyclic TwoBytes Window Text Zeroes Seed);
my @othertests = qw(Sanity Speed Hashmap Avalanche Diff DiffDist BIC LongNeighbors MomentChi2);
my %tests = map {$_ => 1} @keysettests, @othertests;
my $testrx = '(' . join('|',@othertests) . ')';
$testrx = qr($testrx);
delete $tests{LongNeighbors}; # invalid license

if (@ARGV) {
  readf($_) for @ARGV;
} else {
  readf($_) for <doc/*.txt>;
}

sub readf {
  my $fn = shift;
  my ($n,%r);
  open(my $l, "<", $fn) or die "open $fn $!";
  while (<$l>) {
    if (/^--- Testing ([\w_-]+) \"/) {
      #if ($n) { # fixup previous name
      #  check($n,\%r,$fn) if %r;
      #}
      $n = $1;
    }
  NEXT:
    if (/^\[\[\[ Keyset '(.+?)' Tests/) {
      my $t = $1;
      while (<$l>) {
        goto NEXT if $_ =~ $endtest;
        $r{$t} .= $_;
      }
    }
    if (/^\[\[\[ '?$testrx'? /) {
      my $t = $1;
      while (<$l>) {
        goto NEXT if $_ =~ $endtest;
        $r{$t} .= $_;
      }
    }
  }
  check($n,\%r,$fn) if $n; # fixup last name
}

sub check {
  my ($n,$r,$fn) = @_;
  return unless $n;
  my @keys = sort keys %$r;
  my @tests = sort keys %tests;
  my @t;
  if (@keys != @tests) {
    print "# missing tests in $fn for $n:\n";
    print "SMHasher --test=";
    for (@tests) {
      push @t, $_ unless $r->{$_};
    }
    print join(',',@t)," $n >>doc/$n.txt\n";
  }
}
