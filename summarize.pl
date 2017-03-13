use strict;
use warnings;
use warnings FATAL => "all";
use List::Util qw(shuffle);

my @files= shuffle glob "doc/*.out";
my %data;
foreach my $file (@files) {
    $file=~m!^doc/(\w+)\.(\d+)\.(\d+)\.(\d+)\.out\z!
        or next;
    my ($name, $seedbits, $statebits, $hashbits)= ($1, $2, $3, $4);
    open my $fh, "<", $file
        or die "Failed to open '$file' for reading: $!";
    my @ok;
    my $last;
    my $failed= 0;
    my $passed= 0;
    my $count= 0;
    while (<$fh>) {
        next if /^\s*#/;
        if (/^(not )?ok (\d+)/) {
            $ok[$2]= !$1;
            $passed += !$1;
            $failed += !!$1;
            $count++;
        }
        elsif (/^1..(\d+)/) {
            $last = $1;
        }
    }
    next unless $last and defined $ok[$last];
    my $failed_list= join ", ", grep { !$ok[$_] } 1 .. $last;
    1 while $failed_list=~s/(\d+), ((??{ $1 + 1 }))/$1-$2/g;
    $failed_list=~s/(\d+)(?:-\d+)*-(\d+)/$1-$2/g;

    $data{$name}= {
        seedbits    => $seedbits,
        statebits   => $statebits,
        hashbits    => $hashbits,
        failed      => $failed,
        passed      => $passed,
        ok          => \@ok,
        status      => $failed ? "FAILED" : "PASSED",
        failed_list => $failed_list ? "$failed of $count tests: $failed_list" : "all $count tests",
    };
}
foreach my $name (sort keys %data) {
    my $info= $data{$name};
    printf "%-20s %3d %3d %3d %6s %s\n",
        $name, @$info{qw(seedbits statebits hashbits status failed_list)};
}

