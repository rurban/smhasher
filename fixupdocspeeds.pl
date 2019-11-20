#!/usr/bin/perl
use strict;
use File::Copy 'mv';
my $ln = @ARGV ? shift : "log.speed";
open(my $l, "<", $ln) or die "open $ln $!";
my ($n,$speed,$hash);
my $endtest = qr(^(?:---|\[\[\[|Input vcode 0x));
while (<$l>) {
  if (/^--- Testing (\w+) \"/) {
    if ($n) { # fixup previous name
      fixup($n,$speed,$hash);
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
  }
}
fixup($n,$speed,$hash) if $n;

sub fixup {
  my ($n,$speed,$hash) = @_;
  return unless $n;
  return if !defined($speed) && !defined($hash);
  open(my $I, "<", "doc/$n") or die "open doc/$n $!";
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
  mv ("doc/$n.new", "doc/$n") if $found;
}
