#!/usr/bin/env perl
use strict;
use warnings;
use FindBin;
use Getopt::Long qw(GetOptions);
use File::Path qw(make_path);

my $input = "$FindBin::Bin/README.md";
my $out = "$FindBin::Bin/doc";
GetOptions('input=s' => \$input, 'out-dir=s' => \$out)
  or die "Usage: $0 [--input README.md] [--out-dir doc]\n";

open my $in, '<', $input or die "Cannot read $input: $!\n";
my @hashes;
while (my $line = <$in>) {
  next unless $line =~ /^\|\s*\[([^\]]+)\]\(doc\/[^)]+\)/;
  my $name = $1;
  my @fields = split /\|/, $line;
  next unless @fields >= 7;
  # These entries measure the harness or intentionally broken functions.
  next if $fields[6] =~ /\btest\s+(?:NOP|FAIL)\b/i;
  my @values;
  for my $field (@fields[2..5]) {
    push @values, $field =~ /^\s*(\d+(?:\.\d+)?)\s*(?:\([^)]*\))?\s*$/ && $1 > 0
      ? 0 + $1 : undef;
  }
  push @hashes, { name => $name, values => \@values };
}
close $in;

sub xml_escape {
  my ($s) = @_;
  $s =~ s/&/&amp;/g;
  $s =~ s/</&lt;/g;
  $s =~ s/>/&gt;/g;
  $s =~ s/"/&quot;/g;
  return $s;
}

make_path($out) unless -d $out;
open my $md, '>', "$out/comparisons.md" or die "Cannot write comparisons: $!\n";
print $md <<'INTRO';
# Top-five benchmark comparisons

Generated from the benchmark table in [README.md](../README.md). The bars
compare the five best reported values for each metric; the table describes the
hardware and measurement methods. Missing values and harness-only or deliberately
broken test entries are excluded. Faster or smaller does not imply better hash
quality or security; inspect the per-hash results before choosing a hash.

To regenerate after updating the table, run `perl generate-comparisons.pl`.

INTRO

my @metrics = (
  ['Bulk throughput', 'MiB/sec', 0, 1, 'bulk'],
  ['Small-key latency', 'cycles/hash', 1, 0, 'small'],
  ['Hashmap lookup', 'cycles/map operation', 2, 0, 'hashmap'],
  ['Code size', 'bytes', 3, 0, 'size'],
);
for my $metric (@metrics) {
  my ($title, $unit, $index, $higher, $slug) = @$metric;
  my @ranked = grep { defined $_->{values}[$index] } @hashes;
  @ranked = sort {
    ($higher ? $b->{values}[$index] <=> $a->{values}[$index]
             : $a->{values}[$index] <=> $b->{values}[$index])
      || $a->{name} cmp $b->{name}
  } @ranked;
  die "Fewer than five measured entries for $title\n" if @ranked < 5;
  @ranked = @ranked[0..4];
  my $max = 0;
  my $min = $ranked[0]{values}[$index];
  for my $row (@ranked) {
    my $value = $row->{values}[$index];
    $max = $value if $value > $max;
    $min = $value if $value < $min;
  }
  my $log_scale = $max / $min > 10;
  my $file = "comparison-$slug.svg";
  open my $svg, '>', "$out/$file" or die "Cannot write $file: $!\n";
  print $svg qq{<svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 900 200" role="img">\n};
  print $svg '<title>', xml_escape("$title ($unit)"), "</title>\n";
  print $svg "<style>\@media (prefers-color-scheme: dark) { svg { filter: invert(1); } }</style>\n";
  for my $i (0..$#ranked) {
    my $row = $ranked[$i];
    my $value = $row->{values}[$index];
    my $width = sprintf('%.2f', 500 * ($log_scale ? log(1 + $value) / log(1 + $max) : $value / $max));
    my $y = 12 + 37 * $i;
    my $name = xml_escape($row->{name});
    my $label = $index == 3 ? sprintf('%.0f', $value) : sprintf('%.2f', $value);
    my $end = 220 + $width + 10;
    print $svg qq{<text x="10" y="}, $y + 18, qq{" font-size="14" fill="currentColor">$name</text>\n};
    print $svg qq{<rect x="220" y="$y" width="$width" height="24" fill="currentColor" opacity="0.6"/>\n};
    print $svg qq{<text x="$end" y="}, $y + 18, qq{" font-size="14" fill="currentColor">$label</text>\n};
  }
  print $svg "</svg>\n";
  close $svg or die "Cannot finish $file: $!\n";
  my $direction = $higher ? 'higher is better' : 'lower is better';
  my $scale = $log_scale ? ' Bars use a log(1 + value) scale.' : '';
  my $alt = join ', ', map {
    my $v = $index == 3 ? sprintf('%.0f', $_->{values}[$index]) : sprintf('%.2f', $_->{values}[$index]);
    "$_->{name}: $v"
  } @ranked;
  $alt =~ s/\]/\\]/g;
  print $md "## $title\n\n$unit; $direction.$scale\n\n![$title: $alt]($file)\n\n";
  if ($slug eq 'bulk' && grep { $_->{name} eq 'o1hash' } @ranked) {
    print $md "`o1hash` samples only the first, middle, and last four bytes; its reported throughput is not a full-input scan.\n\n";
  }
}
close $md or die "Cannot finish comparisons: $!\n";
