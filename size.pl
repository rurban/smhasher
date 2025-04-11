#!/usr/bin/perl -s
our ($t, $u, $b); # options -t for testfunc, -u for update in README.md and doc/*.html
use strict;
use warnings;

sub testfunc {
  my $n = shift;
  my $test;
  open my $fh, '<', 'main.cpp' or die "Could not open file: $!";
  while (<$fh>) {
    if (/^\{ (\w+),\s+\d+,\s+0x[a-fA-F0-9]+,\s+"\Q$n\E"/) {
      $test = $1;
      last;
    }
  }
  close $fh;
  return $test;
}

sub parse_objdump {
  my $dir = shift || 'build';
  my $test = shift;
  my $cmd = "nm -SC $dir/SMHasher | grep \"$test(\"";
  my $output = `$cmd`;
  my @lines = split /\n/, $output;
  if (@lines == 1) {
    my $line = $lines[0];
    if ($line =~ /^[0-9a-f]+ ([0-9a-f]+) T $test\(/) {
      my $size = $1;
      # TODO if $size < 200 check if the code does not contain call
      # if so, add the size of this call function also
      return hex($size);
    }
  }
}

my $n = shift or die "Usage: $0 <function_name>";
my $test = testfunc($n);
if ($t) {
  $test = $n;
} else {
  die "No Test function for $n in main.cpp" unless $test;
}
print STDERR "Symbol: $test\n";
my $size = parse_objdump('build', $test);
print "$size\n";

if ($size > 30 && $u && !$t) {
  updmd($n, $size, "README.md");
  # same sizes for all x64
  updhtml($n, $size, "doc/$_\.html") for qw(table i7 intel epyc ryzen3 air);
  # extrasizes for i686 with build32, phone with build-aarch64
}

sub updmd {
  my ($n, $size, $file) = @_;
  my ($changed, $in, $out);
  open $in, '<', $file or die "Could not open file: $!";
  open $out, '>', "$file.new" or die "Could not open file: $!";
  while (<$in>) {
    if (m{^(\| \[\Q$n\E\]\(doc/\Q$n\E\.txt\)\s+\|.+?\|.+?\|.+?)\|(.+?)\|(.+)$}) {
      my $oldsize = int($2);
      if ($oldsize != $size) {
        my $s = sprintf("%4d", $size);
        print $out "$1|$s |$3\n";
        $changed = 1;
      } else {
        print $out $_;
      }
    } else {
      print $out $_;
    }
  }
  close $in;
  close $out;
  if ($changed) {
    rename "$file.new", $file or die "Could not rename $file.new to $file: $!";
    print "Updated $file\n";
  } else {
    unlink "$file.new" or die "Could not delete $file.new: $!";
    print "$file already up-to-date\n";
  }
}

sub updhtml {
  my ($n, $size, $file) = @_;
  my ($changed, $found, $s, $in, $out);
  open $in, '<', $file or die "Could not open file: $!";
  open $out, '>', "$file.new" or die "Could not open file: $!";
  while (my $line = <$in>) {
    $_ = "$line";
    if (!$found and m{^<td align="left"><a href="\Q$n\E\.txt">\Q$n\E</a></td>}) {
      $found = 1;
    }
    if ($found and m{^<td align="right">(.+?)</td>}) {
      $s = $1;
      $found++;
    }
    if ($found == 5) {
      $s =~ s/^\s+//; #trim 
      $s =~ s/\s+$//; #trim
      my $oldsize = int($s);
      if ($oldsize != $size and abs($size - $oldsize) < 100) {
        print $out "<td align=\"right\">$size</td>";
        $changed = 1;
      } else {
        warn "suspicious $size != $oldsize\n" if $oldsize != $size;
        print $out $_;
      }
      $found = 0;
    } else {
      print $out $_;
    }
    if ($found > 1 and m{^<td align="left">}) {
      $found = 0;
    }
  }
  close $in;
  close $out;
  if ($changed) {
    rename "$file.new", $file or die "Could not rename $file.new to $file: $!";
    print "Updated $file\n";
  } else {
    unlink "$file.new" or die "Could not delete $file.new: $!";
    print "$file already up-to-date\n";
  }
}
