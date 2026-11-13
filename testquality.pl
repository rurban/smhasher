#!/usr/bin/perl -s
# testquality.pl - summarize the quality problems from a full SMHasher test
# log (doc/<hash>.txt, as written by testall.sh) and update the
# "Quality problems" column of README.md and the html tables.
#
# Usage: ./testquality.pl doc/xxh3.txt [doc/xxh3low.txt ...]
#        ./testquality.pl -h=doc/i7 doc/xxh3.txt
#
# A section that is not present in the log (e.g. BIC, LongNeighbors, or the
# full seed scan) is not re-evaluated: its previous entry is preserved.
# A section that ran and passed drops its entry. Manual notes in the cell
# (anything that is not a known auto entry) are always preserved.
#
# Quality results are machine-independent, so README.md and doc/table.html
# are always updated; with -h the machine table is updated as well.
use strict;
use vars '$h';
use File::Copy 'mv';

my $table = "doc/table.html";
die "$table not found $!" unless -e $table;
my $machine = undef;
if ($h) {
  $machine = "$h.html";
  die "$machine not found $!" unless -e $machine;
}
die "usage: $0 [-h=doc/<machine>] doc/<hash>.txt [...]\n" unless @ARGV;

# test section name => short label, in execution order
my @sections = (
  ["Avalanche Tests",                          "Avalanche"],
  ["Keyset 'Sparse' Tests",                    "Sparse"],
  ["Keyset 'Permutation' Tests",               "Permutation"],
  ["Keyset 'Window' Tests",                    "Window"],
  ["Keyset 'Cyclic' Tests",                    "Cyclic"],
  ["Keyset 'TwoBytes' Tests",                  "TwoBytes"],
  ["Keyset 'Text' Tests",                      "Text"],
  ["Keyset 'Zeroes' Tests",                    "Zeroes"],
  ["Arithmetic Collision Tests",               "Arithmetic"],
  ["Keyset 'Seed' Tests",                      "Seed"],
  ["Keyset 'PerlinNoise' Tests",               "PerlinNoise"],
  ["Diff 'Differential' Tests",                "Diff"],
  ["DiffDist 'Differential Distribution' Tests","DiffDist"],
  ["MomentChi2 Tests",                         "MomentChi2"],
  ["Prng Tests",                               "Prng"],
  ["LongNeighbors Tests",                      "LongNeighbors"],
  ["BIC 'Bit Independence Criteria' Tests",    "BIC"],
  ["BadSeeds Tests",                           "BadSeeds"],
);

# classify an existing cell entry: label of the auto entry it came from,
# or undef for a manual note (always preserved)
sub classify {
  my ($e) = @_;
  return "DiffDist"    if $e =~ /^DiffDist(?: bit \d+(?: w\. \d+ bits)?)?$/;
  return "MomentChi2"  if $e =~ /^Moment Chi2 [0-9.eE+]+ ?!?$/;
  return "BadSeeds"    if $e =~ /^(?:bad|broken) seed 0x[0-9a-f]+$/i;
  return "BadSeeds"    if $e =~ /^\d+ bad seeds$/;
  return "BadSeeds"    if $e =~ /^too many bad seeds$/;
  return "Avalanche"   if $e =~ /^Avalanche$/;
  return "Arithmetic"  if $e =~ /^Arithmetic$/;
  return "Sparse"      if $e =~ /^Sparse$/;
  return "Permutation" if $e =~ /^Permutation$/;
  return "Window"      if $e =~ /^Window$/;
  return "Cyclic"      if $e =~ /^Cyclic$/;
  return "TwoBytes"    if $e =~ /^TwoBytes$/;
  return "Text"        if $e =~ /^Text$/;
  return "Zeroes"      if $e =~ /^Zeroes( \w+)?$/;
  return "Seed"        if $e =~ /^Seed$/;
  return "PerlinNoise" if $e =~ /^PerlinNoise$/;
  return "Diff"        if $e =~ /^Diff$/;
  return "LongNeighbors" if $e =~ /^LongNeighbors$/;
  return "BIC"         if $e =~ /^BIC$/;
  return "Prng"        if $e =~ /^Prng$/;
  return undef;
}

# split a quality cell into (auto entries by label, manual notes in order)
sub splitcell {
  my ($cell) = @_;
  my (%old, @manual);
  for my $e (split /,\s*/, $cell) {
    $e =~ s/^\s+|\s+$//g;
    next unless length $e;
    my $l = classify($e);
    if ($l) { $old{$l} = $e; }
    else    { push @manual, $e; }
  }
  return (\%old, \@manual);
}

# return { present => {label=>0/1}, failed => {label=>0/1}, detail => {label=>str} }
sub parse_log {
  my ($fn) = @_;
  open(my $l, "<", $fn) or die "open $fn $!";
  my (%seen, %fail, %diff, %bad, %known, $chi2max, $abort, $scan, $internallist);
  my ($cur, $bit);
  while (my $line = <$l>) {
    if ($line =~ /^\[\[\[ (.+?) \]\]\]/) {
      $cur = $1;
      $seen{$1} = 1;
      $bit = undef;
      next;
    }
    if ($cur eq "DiffDist 'Differential Distribution' Tests") {
      $bit = $1 if $line =~ /^Testing bit (\d+)/;
      if (defined $bit && $line =~ /!!!!/ && $line =~ /\(([\d.]+)x\) !/) {
        my ($ratio, $win) = ($1, undef);
        $win = $1 if $line =~ /Worst is (\d+) bits/;
        my $old = $diff{$bit};
        $diff{$bit} = [$ratio, $win] if !$old || $ratio > $old->[0];
      }
    }
    elsif ($cur eq "MomentChi2 Tests"
           && $line =~ /MomentChi2 for .*?:\s*([0-9.eE+-]+)/) {
      my $v = $1 + 0;
      $chi2max = $v if !defined $chi2max || $v > $chi2max;
    }
    elsif ($cur eq "BadSeeds Tests") {
      if ($line =~ /^Testing \d+ internal bad seeds:/) { $internallist = 1; next; }
      if ($internallist) {
        if ($line =~ /^\s*(?:0x[0-9a-f]+\s*)+$/) {
          while ($line =~ /\b0x([0-9a-f]+)/g) { $known{$1} = 1; }
          next;
        }
        $internallist = 0;
      }
      $scan  = 1 if $line =~ /Testing the first 0xffffffff seeds/;
      $abort = 1 if $line =~ /Too many bad seeds, aborting/;
      if ($scan && $line =~ /\b(?:Bad|Broken) seed 0x([0-9a-f]+)/) {
        my $s = lc $1;
        $bad{$s} = 1 unless $known{$s};
      }
    }
    if (defined $cur && $line =~ /^\*+FAIL\*+$/) {
      $fail{$cur} = 1;
    }
  }
  close $l;

  my (%present, %failed, %detail);
  for my $s (@sections) {
    my ($name, $label) = @$s;
    if ($label eq "BadSeeds") {
      # only seeds found by the full scan count; known listed seeds do not.
      # without a scan in the log the section is not re-evaluated.
      next unless $scan;
      $present{BadSeeds} = 1;
      if ($abort) {
        $failed{BadSeeds} = 1;
        $detail{BadSeeds} = "too many bad seeds";
      } elsif (keys %bad) {
        $failed{BadSeeds} = 1;
        $detail{BadSeeds} = keys %bad == 1
          ? lc((keys %bad)[0])
          : scalar(keys %bad) . " bad seeds";
      }
      next;
    }
    $present{$label} = $seen{$name} ? 1 : 0;
    $failed{$label}  = $present{$label} && $fail{$name} ? 1 : 0;
    if ($label eq "DiffDist" && $failed{$label} && keys %diff) {
      my ($wbit, $wratio) = (0, -1);
      for my $b (keys %diff) {
        if ($diff{$b}[0] > $wratio) { $wratio = $diff{$b}[0]; $wbit = $b; }
      }
      my $d = "DiffDist bit $wbit";
      $d .= " w. $diff{$wbit}[1] bits" if defined $diff{$wbit}[1];
      $detail{DiffDist} = $d;
    }
    elsif ($label eq "MomentChi2" && $failed{$label} && defined $chi2max) {
      $detail{MomentChi2} = sprintf("Moment Chi2 %.6g", $chi2max);
      $detail{MomentChi2} .= " !" if $chi2max >= 1000000;
    }
  }
  return { present => \%present, failed => \%failed, detail => \%detail };
}

sub readcell {
  my ($file, $is_html, $name) = @_;
  open(my $I, "<", $file) or die "open $file $!";
  my ($inrow, $cell) = (0, "");
  my @row;
  while (my $line = <$I>) {
    if ($inrow) {
      push @row, $line;
      if ($line =~ /<\/tr>/) {
        for my $rl (reverse @row) {
          if ($rl =~ m{<td align="left">([^<]*)</td>}) { $cell = $1; last; }
        }
        last;
      }
      next;
    }
    if ($is_html) {
      $inrow = 1 if $line =~ /<a href="$name\.txt">$name<\/a>/;
    } else {
      if ($line =~ /^\| \[$name\]\(doc\/$name\.txt\)/) {
        $line =~ /\| ([^|]*)\|\s*$/;
        $cell = $1;
        last;
      }
    }
  }
  close $I;
  return $cell;
}

sub update {
  my ($file, $is_html, $name, $cell) = @_;
  open(my $I, "<", $file) or die "open $file $!";
  open(my $O, ">", "$file.new") or die "open $file.new $!";
  my $found;
  my $inrow;
  my @row;
  while (my $line = <$I>) {
    if ($inrow) {
      push @row, $line;
      if ($line =~ /<\/tr>/) {
        $found++;
        # replace the last <td align="left"> of the row (the quality cell)
        for (my $i = $#row; $i >= 0; $i--) {
          last if $row[$i] =~ s{<td align="left">[^<]*</td>}{<td align="left">$cell</td>};
        }
        print $O @row;
        $inrow = 0;
        @row = ();
      }
      next;
    }
    if ($is_html) {
      if ($line =~ /<a href="$name\.txt">$name<\/a>/) {
        $inrow = 1;
        push @row, $line;
        next;
      }
    }
    elsif ($line =~ /^\| \[$name\]\(doc\/$name\.txt\)/) {
      $line =~ s/\s*\| [^|]*\|\s*$//;
      $line .= " | " . sprintf("%-28s", $cell) . " |\n";
      $found++;
      print $O $line;
      next;
    }
    print $O $line;
  }
  warn "$name not found in $file\n" unless $found;
  close $I;
  close $O;
  mv("$file.new", $file) if $found;
}

for my $fn (@ARGV) {
  my $name;
  if (open(my $l, "<", $fn)) {
    while (my $line = <$l>) {
      if ($line =~ /^--- Testing (\S+) /) { $name = $1; last; }
    }
    close $l;
  }
  die "cannot determine hash name from $fn\n" unless $name;

  my $r = parse_log($fn);

  my @files = ("README.md", $table);
  push @files, $machine if $machine;
  for my $f (@files) {
    next unless -e $f;
    my $is_html = $f =~ /\.html$/ ? 1 : 0;
    my ($old, $manual) = splitcell(readcell($f, $is_html, $name));
    my @new;
    for my $s (@sections) {
      my $label = $s->[1];
      if ($r->{failed}{$label}) {
        push @new, $r->{detail}{$label} // $label;
      } elsif (!$r->{present}{$label} && $old->{$label}) {
        # section not in this log (not run): keep the previous entry
        push @new, $old->{$label};
      }
    }
    push @new, @$manual;
    my $cell = join(", ", @new);
    update($f, $is_html, $name, $cell);
    printf "%s: %s quality: [%s]\n", $fn, $name, $cell;
  }
}
