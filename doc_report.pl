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
my $last_speed;
foreach my $data (values %funcs) {
    $last_speed //= 0+@{$data->{small_speed}};
    push @rows, [
        $data->{name},
        $data->{ok},
        $data->{took},
        $data->{align}[0]{bytescycle},
        $data->{align}[0]{mibsec},
        $data->{small_speed}[0]{cycles_hash},
        $data->{small_speed}[1]{cycles_hash},
        $data->{small_speed}[-1]{cycles_hash},
    ];
}
push @sort, '+1', 2 if !@sort;
push @sort, '+1', '0';
@sort= map { split/\s*,\s*/, $_ } @sort;
my @sort_dir= map { s/^\+// ? 1 : 0 } @sort;

no warnings 'numeric';
printf "%-20s %-4s %5s %5s %8s %7s %7s %7s\n",
    "Hash Name", "Ok?", "Took", "b/c", "MiB/sec", "L0 b/h", "L1 b/h", "L$last_speed b/h";
printf "%-20s %-4s %5s %5s %8s %7s %7s %7s\n",
    "-" x 20, "-" x 4, "-" x 5, "-" x 5, "-" x 8, "-" x 7, "-" x 7, "-" x 7;
foreach my $row (sort {
    my $res= 0;
    map $res ||= $sort_dir[$_]
                 ? ($b->[$sort[$_]] <=> $a->[$sort[$_]] ||
                    lc($b->[$sort[$_]]) cmp lc($a->[$sort[$_]]))
                 : ($a->[$sort[$_]] <=> $b->[$sort[$_]] ||
                    lc($a->[$sort[$_]]) cmp lc($b->[$sort[$_]]))
             , 0 .. $#sort;
    $res
} @rows)
{
    printf "%-20s %4s %5d %5.3f %8.2f %7.3f %7.3f %7.3f\n",
        $row->[0], $row->[1] ? "PASS" : "FAIL", @$row[2..$#$row];
}
