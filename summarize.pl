use strict;
use warnings;
use warnings FATAL => "all";
use List::Util qw(shuffle);

my @files= shuffle glob "doc/*.txt";
my %data;
my %all_test_sets;
foreach my $file (@files) {
    $file=~m!^doc/([^.]+)\.(\d+)\.(\d+)\.(\d+)\.txt\z!
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
    my $smhasher_version;
    
    while (<$fh>) {
        if (/- all tests passed #/) {
            $test_set = "OverAll";
        } 
        elsif (/^### (.*?) ###/ || /Keyset '([^']+)'/) {
            $test_set = $1;
            $test_set =~s/\s+Tests?\z//;
            $test_set =~s/Keyset '([^']+)'/$1/g;
            $test_set =~s/0x80000000/HighBit/;
            $test_set =~s/0x00000001/LowBit/;
            $test_set =~s/Combination (\S+)/$1/;
        }
        elsif (/# This is SMHasher version (\S+)/) {
            $smhasher_version = $1;
        }
        $test_set=~s/CRC[^-]*-?(?:Multi)?Collision/Crc-MultiCollision/i;
        next if /^\s*#/;
        if (/^(not )?ok (\d+)/) {
            $ok[$2]= !$1;
            push @{$test_sets_failed{$test_set}}, $2 if $1;
            $all_test_sets{$test_set}= $2 if $1 and (!defined $all_test_sets{$test_set} 
                                      or $all_test_sets{$test_set} > $2);
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
        $test_sets_failed{$set}= join(", ", @{$test_sets_failed{$set}});
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
    $test_sets_failed{$_} ||= "ok"
        for keys %test_sets;

    $data{$name}= {
        seedbits    => $seedbits,
        statebits   => $statebits,
        hashbits    => $hashbits,
        failed      => $failed,
        passed      => $passed,
        ok          => \@ok,
        num_tests   => $last,
        status      => $failed ? "FAILED" : "PASSED",
        failed_list => $failed_list ? "$failed of $count tests. " : "all $count tests",
        failed_groups =>$failed_list ? $failed_list : "",
        status_hash => \%test_sets_failed,
        smhasher_version => $smhasher_version,
    };
}

foreach my $name (sort keys %data) {
    my $info= $data{$name};
    printf "%-20s %3d %3d %3d %6s %s%s\n",
        $name, @$info{qw(seedbits statebits hashbits status failed_list failed_groups)};
}

open my $fh, ">", "doc/summary.html";
select $fh;
print "<head><style>\n",<<'EOF_STYLE';
* {
  box-sizing: border-box;
}
body {
  background: steelblue;
  font-family: "Open Sans", arial;
}
table {
  width: 100%;
  max-width: 600px;
  height: 320px;
  border-collapse: collapse;
  border: 1px solid #38678f;
  margin: 50px auto;
  background: white;
}
th {
  background: steelblue;
  height: 54px;
  font-weight: lighter;
  text-shadow: 0 1px 0 #38678f;
  color: white;
  border: 1px solid #38678f;
  box-shadow: inset 0px 1px 2px #568ebd;
  transition: all 0.2s;
}
tr {
  border-bottom: 1px solid #cccccc;
}
tr:last-child {
  border-bottom: 0px;
}
td {
  border-right: 1px solid #cccccc;
  padding: 10px;
  transition: all 0.2s;
}
td:last-child {
  border-right: 0px;
}
td.selected {
  background: #d7e4ef;
  z-index: ;
}
td input {
  font-size: 14px;
  background: none;
  outline: none;
  border: 0;
  display: table-cell;
  height: 100%;
  width: 100%;
}
td input:focus {
  box-shadow: 0 1px 0 steelblue;
  color: steelblue;
}
::-moz-selection {
  background: steelblue;
  color: white;
}
::selection {
  background: steelblue;
  color: white;
}
.main {
  max-width: 600px;
  padding: 10px;
  margin: auto;
}
.content {
  color: white;
  text-align: center;
}
.content p,
.content pre,
.content h2 {
  text-align: left;
}
.content pre {
  padding: 1.2em 0 0.5em;
  background: white;
  background: rgba(255, 255, 255, 0.7);
  border: 1px solid rgba(255, 255, 255, 0.9);
  color: #38678f;
}
h1 {
  text-shadow: 0 5px 15px rgba(0, 0, 0, 0.1);
  text-align: center;
}
tr.passed td.status,
tr.passed:hover {
  background: #b3ffb3;
}
tr.failed td.status,
tr.failed:hover {
  background: #ffb3b3;
}
</style>
<script src='https://code.jquery.com/jquery-2.2.4.min.js'></script>
<script src='http://tablesorter.com/__jquery.tablesorter.min.js'></script>
</head><body>
EOF_STYLE
print "<table id='testresults'>\n";
#printf "<thead><th colspan='%d'>Hash Details</th><th colspan='%d'>Test Group</th><th colspan='%d'>Misc</th></thead>\n", 5, 0+keys %all_test_sets, 2;
printf "<thead><th class='hashname'><span>%s</span></th><th class='seedbits'><span>%s</span></th><th class='statebits'><span>%s</span></th><th class='hashbits'><span>%s</span></th><th class='status'><span>%s</span></th>\n",
        qw(Name SeedBits StateBits HashBits Test-Status);
foreach my $set (sort{ $all_test_sets{$a} <=> $all_test_sets{$b} || $a cmp $b} keys %all_test_sets) {
    printf "<th class='%s'><span>%s</span></th>\n", $set, $set; 
}

print "<th><span class='num_tests'>Num-Tests</span></th><th class='smhasher_version'><span>SMHasher-Version</span></th></thead><tbody>\n";
foreach my $name (sort keys %data) {
    my $info= $data{$name};
    printf "<tr class='%s'><td class='hashname'>%s</td><td class='seedbits'>%d</td><td class='statebits'>%d</td><td class='hashbits'>%d</td><td class='status'>%s</td>\n",
        lc($info->{status}), $name, @$info{qw(seedbits statebits hashbits status )};
    foreach my $set (sort{ $all_test_sets{$a}<=>$all_test_sets{$b} || $a cmp $b} keys %all_test_sets) {
        printf "<td class='test_%s'>%s</td>\n", $set, $info->{status_hash}{$set} // "?"; 
    }
    printf "<td class='num_tests'>%s</td><td class='smhasher_version'>%s</td>\n",
        @$info{qw(num_tests smhasher_version)};
    print"</tr>\n";
}
print "</tbody></table>";
print <<'EOF_HTML'
<script>
$(function(){
      $('#testresults').tablesorter(); 
});
</script>
</body>
EOF_HTML


