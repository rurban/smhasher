#!/usr/bin/perl
use strict;
use File::Copy 'mv';
my $ln = @ARGV ? shift : "log.speed";

open(my $l, "<", $ln) or die "open $ln $!";
my ($n,$bulk,$small,$hash);
while (<$l>) {
  if (/^--- Testing ([\w_-]+)[\s\n]/) {
    if ($n) {
      fixupmd($n,$bulk,$small,$hash);
    }
    $n = $1; $bulk = $small = $hash = undef;
  } elsif (/^Average.+ - +([\d\.]+) MiB\/sec/) {
    $bulk = sprintf("%12.2f",$1);;
  } elsif (/^Average +(\d.*)cycles\/hash/) {
    $small = sprintf("%8.2f",$1);
  } elsif (/^Running fast HashMapTest: +(\d\S+) cycles\/op \((\d.+) stdv\)/) {
    $hash = sprintf("%6.2f (%.0f)", $1, $2);
    $hash .= " " if $2 < 10.0;
  } elsif (/^Running std HashMapTest: SKIP/) {
    $hash = "too slow";
  }
}
fixupmd($n,$bulk,$small,$hash) if $n;

sub fixupmd {
  my ($n,$bulk,$small,$hash) = @_;
  open(my $I, "<", "README.md") or die "open README.md $!";
  open(my $O, ">", "README.md.new") or die "open README.md.new $!";
  my $found;
  while (<$I>) {
    # search for $n in README.md
    if (/^\| \[$n\]/) {
      # get old values, update line
      m/$n\)\s*\| (\s+\d[\d\.]+)\s*\| (\s+\d[\d\.]+)\s+\|\s+(\d[\d\.]+ \(\d+\)|skipped|too slow|-)\s*\|/;
      $bulk  = $1 unless defined $bulk;
      $small = $2 unless defined $small;
      $hash  = $3 unless defined $hash;
      $hash  = "   -    " unless $hash;
      my $spc = " " x (38-(2*length($n)));
      s/$n\)\s*\|\s+\d[\d\.]+\s*\|\s+\d[\d\.]+\s+\|\s+(\d[\d\.]+ \(\d+\)|skipped|too slow|-)\s*\|/$n)$spc| $bulk | $small | $hash|/;
      $found++;
    }
    print $O $_;
  }
  if (!$found) {
    warn "$n not found in README.md\n";
  }
  close $I;
  close $O;
  if ($found) {
    mv ("README.md.new", "README.md");
    fixuphtml ($n,$bulk,$small,$hash);
  }
}

sub fixuphtml {
  my ($n,$bulk,$small,$hash) = @_;
  open(my $I, "<", "doc/table.html") or die "open doc/table.html $!";
  open(my $O, ">", "doc/table.html.new") or die "open doc/table.html.new $!";
  my $found;
  while (<$I>) {
    # search for $n in doc/table.html FIXME
    if (/<a href="$n">$n/) {
      # get old values, update line
      print $O $_;

      $_ = <$I>;
      if (defined $bulk) {
        $bulk =~ s/^\s+//;
        s{<td align="right">(.+?)</td>}{<td align="right">$bulk</td>};
      }
      print $O $_;

      $_ = <$I>;
      if (defined $small) {
        $small =~ s/^\s+//;
        s{<td align="right">(.+?)</td>}{<td align="right">$small</td>};
      }
      print $O $_;

      $_ = <$I>;
      if (defined $hash) {
        $hash =~ s/^\s+//;
        s{<td align="right">(.+?)</td>}{<td align="right">$hash</td>};
      }
      print $O $_;
      $_ = <$I>;
      $found++;
    }
    print $O $_;
  }
  if (!$found) {
    warn "$n not found in doc/table.html\n";
  }
  close $I;
  close $O;
  mv ("doc/table.html.new", "doc/table.html") if $found;
}
