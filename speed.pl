#!/usr/bin/perl -s
use strict;
use vars '$h';
use File::Copy 'mv';
my $ln = @ARGV ? $ARGV[0] : "log.speed";
use constant MAX_HASH_STDDEV => 150;

my $html = "doc/table.html";
if ($h) {
  die "$h.html not found $!" unless -e "$h.html";
  $html = "$h.html";
  my $b = $h; $b =~ s{^doc/}{};
  $ln = "log.speed-$b" unless @ARGV;
}

open(my $l, "<", $ln) or die "open $ln $!";
my ($n,$bulk,$small,$hash,@redo);
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
    if ($2 > MAX_HASH_STDDEV) {
      $hash = "-";
      push @redo, $n;
    }
  } elsif (/^Running fast HashMapTest: SKIP/) {
    $hash = "too slow   ";
  }
}
fixupmd($n,$bulk,$small,$hash) if $n;
warn "need to redo: ",join(" ",@redo),"\n" if @redo;

sub fixupmd {
  my ($n,$bulk,$small,$hash) = @_;
  open(my $I, "<", "README.md") or die "open README.md $!";
  open(my $O, ">", "README.md.new") or die "open README.md.new $!";
  my $found;
  while (<$I>) {
    # search for $n in README.md
    if (/^\| \[$n\]/) {
      # get old values, update line
      m/$n\.txt\)\s*\| (\s+\d[\d\.]+)\s*\| (\s+\d[\d\.]+)\s+\|\s+(\d[\d\.]+ \(\d+\)|skipped|too slow|-)\s*\|/;
      $bulk  = $1 unless defined $bulk;
      $small = $2 unless defined $small;
      $hash  = $3 unless defined $hash;
      $hash  = "   -    " unless $hash;
      my $spc = " " x (34-(2*length($n)));
      s/$n\.txt\)\s*\|\s+\d[\d\.]+\s*\|\s+\d[\d\.]+\s+\|\s+(\d[\d\.]+ \(\d+\)|skipped|too slow|-)\s*\|/$n.txt)$spc| $bulk | $small | $hash|/;
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
    mv ("README.md.new", "README.md") unless $h;
    fixuphtml ($n,$bulk,$small,$hash);
  }
}

sub fixuphtml {
  my ($n,$bulk,$small,$hash) = @_;
  open(my $I, "<", $html) or die "open $html $!";
  open(my $O, ">", "$html.new") or die "open $html.new $!";
  my $found;
  while (<$I>) {
    # search for $n in doc/table.html FIXME
    if (/<a href="$n\.txt">$n/) {
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
        $hash =~ s/\s+$//;
        s{<td align="right">(.+?)</td>}{<td align="right">$hash</td>};
      }
      print $O $_;
      $_ = <$I>;
      $found++;
    }
    print $O $_;
  }
  if (!$found) {
    warn "$n not found in $html\n";
  }
  close $I;
  close $O;
  mv ("$html.new", $html) if $found;
}
