use strict;
use warnings;
use Getopt::Long;

GetOptions( "sort=s", \my @sort);


my @files= @ARGV;
my %funcs;
foreach my $file (@files) {
    open my $fh, "<", $file
        or die "Failed to open '$file': $!";
    my %data;
    while (<$fh>) {
        if (/^---\s+Testing\s+(\w+)/x) {
            $data{name}= $1;
            $funcs{$1}=\%data;
        }
        if (
            m!Alignment\s+(?<align>\d)\s+-\s+(?<bytescycle>\S+)\s+bytes/cycle\s+-\s+(?<mibsec>\S+)
            \s+MiB/sec\s+\@\s+(?<clock_speed>\d)\s+(?<clock_unit>\w+)!x
        ) {
            $data{align}[$+{align}]= { %+ };
        }
        elsif (
            m!Small\s+key\s+speed\s+test\s+-\s+
            (?<len>\d+)-byte\s+keys\s+-\s+(?<cycles_hash>\S+)\s+cycles/hash!x
        ) {
            $data{small_speed}[$+{len}]= { %+ };
        }
        elsif (
            m!ALL\s+TESTS\s+PASSED!x
        ) {
            $data{ok}= 1;
        }
        elsif (
            m!FAILED!x
        ) {
            $data{ok}= 0;
        }
        elsif (
            m!Testing\s+took\s+(?<took>\S+)\s+seconds!x
        ) {
            $data{took}= $+{took};
        }
    }
}

use Data::Dumper;
my @rows;
my $l1;
my $l2;
foreach my $data (values %funcs) {
    $l1 //= $data->{small_speed}[1]{len};
    $l2 //= $data->{small_speed}[-1]{len};
    push @rows, [
        $data->{ok},
        $data->{name},
        $data->{took},
        $data->{align}[0]{bytescycle},
        $data->{align}[0]{mibsec},
        $data->{small_speed}[1]{cycles_hash},
        $data->{small_speed}[-1]{cycles_hash},
    ];
}
push @sort, 1;
@sort= map { split/\s*,\s*/, $_ } @sort;

no warnings 'numeric';
foreach my $row (sort {
    my $res= 0;
    map $res ||= /\+/
                 ? ($b->[$_] <=> $a->[$_] || $b->[$_] cmp $a->[$_])
                 : ($a->[$_] <=> $b->[$_] || $a->[$_] cmp $b->[$_])
             , @sort;
    $res
} @rows)
{
    printf "%4s %-20s took:%5ds Al0 %s b/c %9.2f Mbs L$l1:%s L$l2:%s\n",
        $row->[0] ? "PASS" : "FAIL", @$row[1..$#$row];
}
