use strict;
use warnings;
use v5.12;
use Data::Dumper;
use IO::Handle;

sub common_prefix {
    my $s1= shift;
    while (length $s1 and @_) {
        my $s2= shift;
        my $i = 0;
        while (lc substr($s1,$i,1) eq lc substr($s2,$i,1)) { $i++ };
        $s1= substr($s1,0,$i);
    }
    $s1=~s/[-_.]+\z//;
    return $s1;
}

sub graph {
    my ($hash,$title)= @_;

    my @plot;
    my @ret;
    my %file_titles;
    my @names= sort keys %$hash;
    $title ||= common_prefix(@names);
    if (!$title and @names==grep /OAAT/, @names) {
        $title= "One At A Time Hashes";
    }
    $title ||= "Mixed";
    (my $ftitle= $title)=~s/[\W_]+/_/g;
    s/_+\z//,s/^_+// for $ftitle;
    foreach my $name (@names) {
        push @plot, "'-' with lines title columnhead(2)";
        push @ret, "bytes $name";
        foreach my $h ( @{$hash->{$name}{array}} ) {
            push @ret, join " ", map { $_ } ($h->{key_bytes},$h->{cycles_per_hash});
        }
        push @ret,"e";
    }
    my $save= 1;
    my $dir= "doc/graph";
    open my $out,'|-','gnuplot'
        or die "Failed to open pipe to 'gnuplot'";
    #my $out = \*STDOUT;
    my $ext;
    my $file;
    say $out qq(set key below);
    say $out qq(set xtics 1);
    say $out qq(set ytics);
    say $out qq(set grid);
    say $out qq(set yrange [0:*]);
    say $out qq(set xlabel "Key Length");
    say $out qq(set ylabel "cycles/hash");
    if ($save) {
        $ext = "svg"; 
        #say $out qq(set term $ext font "/usr/share/fonts/truetype/liberation/LiberationMono-Regular.ttf,11");
        say $out qq(set term $ext size 1100,500 linewidth 2 mouse standalone font "mono,11");
    }
    
    if ($save) {
        $file= "$dir/$ftitle.tiny.$ext";
        say $out qq(set output "$file");
        say qq(writing '$file');
    }
    my $long_title= "$title - Hash Speed for Tiny Keys";
    $file_titles{$file}=$long_title;
    say $out qq(set title "$long_title");
    say $out qq(plot [0:12] ) ,join(",",@plot);
    say $out join "\n", @ret;

    if ($save) {
        $file= "$dir/$ftitle.short.$ext";
        say $out qq(set output "$file");
        say qq(writing '$file');
    }
    $long_title= "$title - Hash Speed for Short Keys";
    $file_titles{$file}=$long_title;
    say $out qq(set title "$long_title");
    say $out qq(plot [0:32] ) ,join(",",@plot);
    say $out join "\n", @ret;

    if ($save) {
        $file = "$dir/$ftitle.medium.$ext";
        say $out qq(set output "$file"); 
        say qq(writing '$file');
    }
    $long_title= "$title - Hash Speed for Medium Keys";
    $file_titles{$file}=$long_title;
    say $out qq(set xtics 4);
    say $out qq(set title "$long_title");
    say $out qq(plot [32:128]) ,join(",",@plot);
    say $out join "\n", @ret;
    
    if ($save) {
        $file= "$dir/$ftitle.long.$ext";
        say $out qq(set output "$file"); 
        say qq(writing '$file');
    }
    say $out qq(set yrange [1:*]);
    say $out qq(set logscale xy);
    say $out qq(set xtics);
    say $out qq(set ytics);
    $long_title= "$title - Hash Speed for Long Keys (Log Scale)";
    $file_titles{$file}=$long_title;
    say $out qq(set title "$long_title");
    say $out qq(plot [1:65536]) ,join(",",@plot);
    say $out join "\n", @ret;
    flush $out;
    unless ($save) {
        <>;
        close $out;
        return;
    }
    close $out;
    return unless $ext eq "svg";

    my @files= map { "$ftitle.$_.$ext" } qw(tiny short medium long);

    foreach my $idx (0..$#files) {
        my $file= $files[$idx];
        my $left_links= join " ", map { qq(<a href="$files[$_]">--</a> ) } (0 .. ($idx-1));
        my $right_links= join " ", map { qq( <a href="$files[$_]">++</a>) } ($idx+1 .. $#files);
        #say "cleaning '$type'";
        open my $fh, "<", "$dir/$file"
            or die "Failed to open '$dir/$file' for reading:$!";
        my %map;
        my $text;
        while (<$fh>) {
            s!<title>Gnuplot</title>!<title>$file_titles{"$dir/$file"}</title>!;
            s!<text>(\Q$title\E - .*?)</text>!<text>$left_links$1$right_links</text>!;
            #s!<desc>Produced by.*</desc>!<desc></desc>!
            s/gnuplot_plot_(\d+)/$names[$1-1]/g;
            $text .= $_;
        }
        close $fh;
        open my $ofh, ">", "$dir/$file"
            or die "Failed to open '$dir/$file' for write: $!";
        print $ofh $text;
        close $ofh;

    }
}

my $output_name;
if (@ARGV and $ARGV[0] eq "--name") {
    shift @ARGV;
    $output_name= shift @ARGV;
}
    


# Zaphod32                  1 byte keys       21.143 c/h       21.143 c/b        0.047 b/c
#
#
my %hash;
while (<>) {
    if (m!^\s*\#\s(?<name>\w+)\s+(?<key_bytes>\d+)\s+byte\s+keys\s+
        (?<cycles_per_hash>\S+)\s+c/h(?:\s+(?<cycles_per_byte>\S+)\s+c/b\s+(?<bytes_per_cycle>\S+)\s+b/c)?!x) {
        my $info= { %+ };
        my $name= $info->{name};
        #next if $info->{key_bytes} > 128; # skip the long stuff for now
        if (!$hash{$name}{key_bytes}{ $info->{key_bytes} }) {
            push @{$hash{$name}{ array }}, $info;
            $hash{$name}{key_bytes}{ $info->{key_bytes} }= $info;
        }
    }
}
graph(\%hash,$output_name) if keys %hash;
