#!/usr/bin/perl
use strict;
use File::Copy 'mv';
my $endtest = qr(^(?:---|\[\[\[|Input vcode 0x));
# mkdir lowcoll; build/SMHasher --list|perl -alne'print $F[0] if (/^(rmd|sha|blake2|md5)/ && !/_32/) || /128/' | parallel -j4 --bar 'build/SMHasher --test=Sparse,Permutation,Cyclic,TwoBytes,DiffDist,Text,Zeroes,Seed {} >lowcoll/{}'
my @lowcolls = qw(Sparse Permutation Cyclic TwoBytes DiffDist Text Zeroes Seed);
my %lowcolls = map {$_ => 1} @lowcolls;

if (@ARGV) {
  readf($_) for @ARGV;
} else {
  readf($_) for <lowcoll/*>;
}

sub readf {
  my $fn = shift;
  my ($n,%r);
  open(my $l, "<", $fn) or die "open $fn $!";
  while (<$l>) {
    if (/^--- Testing ([\w-_]+) \"/) {
      if ($n) { # fixup previous name
        fixup($n,\%r,$fn) if %r;
      }
      $n = $1; %r = ();
    } elsif (/^\[\[\[ Keyset '(.+?)' Tests/ && $lowcolls{$1}) {
      my $t = $1;
      while (<$l>) {
        last if $_ =~ $endtest;
        $r{$t} .= $_;
      }
    } elsif (/^\[\[\[ DiffDist /) {
      my $t = 'DiffDist';
      while (<$l>) {
        last if $_ =~ $endtest;
        $r{$t} .= $_;
      }
    } elsif (/^\[\[\[ /) {
      last;
    }
  }
  fixup($n,\%r,$fn) if $n; # fixup last name
}

sub fixup {
  my ($n,$r,$fn) = @_;
  return unless $n;
  return if !%$r;
  return if "doc/$n" eq $fn;
  open(my $I, "<", "doc/$n") or die "open doc/$n $!";
  open(my $O, ">", "doc/$n.new") or die "open doc/$n.new $!";
  my $found;
  my %r = %$r;
  while (<$I>) {
    print $O $_;
    # search for $n in doc
    if (/^--- Testing /) {
      $found = /^--- Testing $n /;
    }
    elsif ($found && /^\[\[\[ Keyset '(.+?)' Tests/ && $r{$1}) {
      my $t = $1;
      print $O $r{$t};
      while (<$I>) {
        last if $_ =~ $endtest;
      }
      print $O $_;
    } elsif ($found && /^\[\[\[ DiffDist / && $r{DiffDist}) {
      my $t = 'DiffDist';
      print $O $r{$t};
      while (<$I>) {
        last if $_ =~ $endtest;
      }
      print $O $_;
    }
  }
  close $I;
  close $O;
  mv ("doc/$n.new", "doc/$n") if $found;
}
