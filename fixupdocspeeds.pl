#!/usr/bin/perl
use strict;
use File::Copy 'mv';
my $endtest = qr(^(?:---|\[\[\[|Input vcode 0x));

if (@ARGV) {
  readf($_) for @ARGV;
} else {
  readf("log.speed");
}

sub readf {
  my $fn = shift;
  my ($n,$speed,$hash);
  open(my $l, "<", $fn) or die "open $fn $!";
  while (<$l>) {
    if (/^--- Testing ([\w-_]+) \"/) {
      if ($n) { # fixup previous name
        fixup($n,$speed,$hash,$fn) if $hash or $speed;
      }
      $n = $1; $speed = $hash = undef;
    } elsif (/^\[\[\[ 'Hashmap' Speed/) {
      while (<$l>) {
        last if $_ =~ $endtest;
        $hash .= $_;
      }
    } elsif (/^\[\[\[ Speed Tests/) {
      while (<$l>) {
        last if $_ =~ $endtest;
        $speed .= $_;
      }
    } elsif (/^\[\[\[ /) {
      last;
    }
  }
  fixup($n,$speed,$hash,$fn) if $n; # fixup last name
}

sub fixup {
  my ($n,$speed,$hash,$fn) = @_;
  return unless $n;
  return if !defined($speed) && !defined($hash);
  return if "doc/$n.txt" eq $fn;
  open(my $I, "<", "doc/$n.txt") or die "open doc/$n.txt $!";
  open(my $O, ">", "doc/$n.new") or die "open doc/$n.new $!";
  my $found;
  while (<$I>) {
    print $O $_;
    # search for $n in doc
    if (/^--- Testing /) {
      $found = /^--- Testing $n /;
    }
    elsif ($found && defined $hash && /^\[\[\[ 'Hashmap' Speed/) {
      print $O $hash;
      while (<$I>) {
        last if $_ =~ $endtest;
      }
      print $O $_;
    } elsif ($found && defined $speed && /^\[\[\[ Speed Tests/) {
      print $O $speed;
      while (<$I>) {
        last if $_ =~ $endtest;
      }
      print $O $_;
    }
  }
  close $I;
  close $O;
  mv ("doc/$n.new", "doc/$n.txt") if $found;
}
