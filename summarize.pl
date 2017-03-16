use strict;
use warnings;
use warnings FATAL => "all";
use List::Util qw(shuffle);

my @files= shuffle glob "doc/*.out";
my %data;
my %all_test_sets;
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
            $all_test_sets{$test_set}= $2 if !defined $all_test_sets{$test_set} 
                                      or $all_test_sets{$test_set} > $2;
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
    for my $set (keys %test_sets_failed) {
        $test_sets_failed{$set}= join(",", @{$test_sets_failed{$set}});
    }
    #my $failed_list= join ", ", grep { !$ok[$_] } 1 .. $last;
    my $failed_list= join ", ", sort map { "$_: " . $test_sets_failed{$_} } 
        sort { $test_sets{$a} <=> $test_sets{$b} } keys %test_sets_failed;
    for ($failed_list, values %test_sets_failed) {
        1 while s/(\d+),\s*((??{ $1 + 1 }))/$1-$2/g;
        s/(\d+)(?:-\d+)*-(\d+)/$1-$2/g;
    }
    my $count_sets_failed= keys %test_sets_failed;
    my $test_group_plural= $count_sets_failed > 1 ? "s" : "";
    $test_sets_failed{$_} = $test_sets_failed{$_} ? "not ok: $test_sets_failed{$_}" : "ok"
        for keys %test_sets;

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
        status_hash => \%test_sets_failed,
    };
}

foreach my $name (sort keys %data) {
    my $info= $data{$name};
    printf "%-20s %3d %3d %3d %6s %s%s\n",
        $name, @$info{qw(seedbits statebits hashbits status failed_list failed_groups)};
}

open my $fh, ">", "doc/summary.html";
select $fh;
print "<table>\n";
printf "<tr><th class='hashname'>%s</th><th class='seedbits'>%s</td><th class='statebits'>%s</td><td class='hashbits'>%s</td><th class='status'>%s</th>\n",
        qw(Name SeedBits StateBits HashBits Test-Status);
foreach my $set (sort{ $all_test_sets{$a} <=> $all_test_sets{$b} || $a cmp $b} keys %all_test_sets) {
    printf "<th class='%s'>%s Test</td>\n", $set, $set; 
}
print"</tr>\n";
foreach my $name (sort keys %data) {
    my $info= $data{$name};
    printf "<tr><th class='hashname'>%s</th><td class='seedbits'>%d</td><td class='statebits'>%d</td><td class='hashbits'>%d</td><th class='status'>%s</th>\n",
        $name, @$info{qw(seedbits statebits hashbits status)};
    foreach my $set (sort{ $all_test_sets{$a}<=>$all_test_sets{$b} || $a cmp $b} keys %all_test_sets) {
        printf "<td class='%s'>%s</td>\n", $set, $info->{status_hash}{$set} // "?"; 
    }
    print"</tr>\n";
}
print "</table>\n";


