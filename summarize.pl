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
    my %test_sets_failed;
    my %test_sets;
    my $last;
    my $failed= 0;
    my $passed= 0;
    my $count= 0;
    my $test_set= "Startup";
    while (<$fh>) {
        if (/- all tests passed #/) {
            $test_set = "OverAll";
        } elsif (/^### (.*?) ###$/ || /Keyset '([^']+)'/) {
            $test_set = $1;
            $test_set =~s/\s+Tests?\z//;
            $test_set =~s/Keyset '([^']+)'/$1/g;
            $test_set =~s/0x80000000/HighBit/;
            $test_set =~s/0x00000001/LowBit/;
            $test_set =~s/Combination (\S+)/$1/;
        }
        next if /^\s*#/;
        if (/^(not )?ok (\d+)/) {
            $ok[$2]= !$1;
            push @{$test_sets_failed{$test_set}}, $2 if $1 and $test_set ne "OverAll";
            $test_sets{$test_set}//=$2;
            $passed += !$1;
            $failed += !!$1;
            $count++;
        }
        elsif (/^1..(\d+)/) {
            $last = $1;
        }
    }
    next unless $last and defined $ok[$last];
    #my $failed_list= join ", ", grep { !$ok[$_] } 1 .. $last;
    my $failed_list= join ", ", 
        map { "$_: " . join(",",@{$test_sets_failed{$_}}) } 
        sort { $test_sets{$a} <=> $test_sets{$b} } keys %test_sets_failed;
    1 while $failed_list=~s/(\d+),\s*((??{ $1 + 1 }))/$1-$2/g;
    $failed_list=~s/(\d+)(?:-\d+)*-(\d+)/$1-$2/g;
    my $count_sets_failed= keys %test_sets_failed;
    my $test_group_plural= $count_sets_failed > 1 ? "s" : "";

    $data{$name}= {
        seedbits    => $seedbits,
        statebits   => $statebits,
        hashbits    => $hashbits,
        failed      => $failed,
        passed      => $passed,
        ok          => \@ok,
        status      => $failed ? "FAILED" : "PASSED",
        failed_list => $failed_list ? "$failed of $count tests. " : "all $count tests",
        failed_groups =>$failed_list ? $failed_list : "",
    };
}
foreach my $name (sort keys %data) {
    my $info= $data{$name};
    printf "%-20s %3d %3d %3d %6s %s%s\n",
        $name, @$info{qw(seedbits statebits hashbits status failed_list failed_groups)};
}

