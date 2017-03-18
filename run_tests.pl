use strict;
use warnings;
use List::Util qw(shuffle);

while (1) {
    chomp(my @hashlines=`./SMHasher --mlist`);
    my %byname;
    my @names;
    foreach my $line (@hashlines) {
        my $info= {};
        my @info= split /\t/, $line;
        @$info{'name','seedbits','statebits','hashbits'}= @info;
        $byname{$info->{name}}= $info;
        $info->{file_base}= sprintf "doc/%s.%d.%d.%d", @info;
        $info->{file_name}= $info->{file_base} . ".txt";
        $info->{file_tmp}= $info->{file_base} . ".tmp";
        $info->{priority}= $info->{seedbits} * $info->{hashbits};
        my $old_name= sprintf "doc/%s.%d.txt", $info->{name}, $info->{hashbits};
        if (-e $old_name ) {
            rename $old_name, $info->{file_name};
        }
        $info->{file_time}= (stat($info->{file_name}))[9] // 0;
        push @names, $info->{name};
    }

    @names= sort {
        $byname{$a}{file_time} <=> $byname{$b}{file_time} ||
        $byname{$b}{priority} <=> $byname{$a}{priority} ||
        $a cmp $b
    } @names;
    $|++;
    foreach my $name ( $names[0] ) {
        my $info= $byname{$name};
        system("./SMHasher $name | tee $info->{file_tmp}");
        open my $ifh, "<", $info->{file_tmp}
            or die "Failed to open temp file '$info->{file_tmp}':$!";
        my $looks_ok= 0;
        while (<$ifh>) {
            if (/Testing took/) {
                $looks_ok=1;
            }
        }
        close $ifh;
        if ($looks_ok) {
            rename $info->{file_tmp}, $info->{file_name};
        } else {
            warn "Something is wrong, not renaming $info->{file_tmp} to $info->{file_name}\n";
            exit;
        }
    }
}

