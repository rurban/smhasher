use strict;
use warnings;
use Data::Dumper;
use GD::Graph::lines;
use GD::Graph::bars;

sub graph {
    my ($hash,$name)= @_;

    my $height=  600;
    my $width= 1200;

    my @key_bytes;
    my @datasets;
    my @legends;
    foreach my $key (sort keys %$hash) {
        push @legends, $key;
        push @datasets,[ map log($_->{cycles_per_hash}+1), @{$hash->{$key}{array}} ];
        @key_bytes= ( map $_->{key_bytes}, @{$hash->{$key}{array}} )
            unless @key_bytes;
        $name //= $key;
    }
    return unless @key_bytes;
    my $min= $key_bytes[0];
    my $max= $key_bytes[-1];

    my $graph= GD::Graph::lines->new($width, $height);

    #$line_types= [ 1, 1, 3, 3, 3 ];
    my $dclrs= [ "black", "lred", "lblue", "orange", "orange" ];

    $graph->set(
         x_label                => 'Bytes Hashed',
         y_label                => 'Cycles',
         title                  => "Hash Performance",
         #y_max_value            => $max,
         y_min_value            => 0,
         y_tick_number          => 10,
         #x_tick_offset          => $f2 ? ($f2->count % 30) : undef,
         #x_tick_number          => "auto", #( ( $max - $min) / @key_bytes),
         #y_label_skip           => 2,
         #x_label_skip           => 30,
         #x_last_label_skip      => 1,
         #x_labels_vertical      => 1,
         y_number_format        => sub { my $r= int exp($_[0])-1; $r },
         #x_number_format        => sub { my $r= exp($_[0])-1; $r },
         line_width             => 2,
         transparent            => 0,
         dclrs                  => $dclrs,
         bgclr                  => 'white',
         x_label_position       => 0.5,
         box_axis               => 0,
         tick_length            => -4,
         axis_space             => 6,
         legend_placement       => 'RC',
         legend_marker_width    => 12,
         legend_marker_height   => 12,
         long_ticks             => 0,
         #line_types             => $line_types,
         line_type_scale        => 4,
    ) or die $graph->error;

    my $font_dir  = '/usr/share/fonts';
    my $font_file = "$font_dir/nimbus-truetype-fonts/NimbusRomNo9L-Regu.ttf";

    if (-e $font_file) {
        $graph->set_title_font($font_file, 20) or warn $graph->error;
        $graph->set_x_label_font($font_file, 16) or warn $graph->error;
        $graph->set_y_label_font($font_file, 16) or warn $graph->error;
        $graph->set_x_axis_font($font_file, 9) or warn $graph->error;
        $graph->set_y_axis_font($font_file, 9) or warn $graph->error;
        $graph->set_legend_font($font_file, 12) or warn $graph->error;
    }

    $graph->set_legend(@legends);

    my $gd = $graph->plot(
        [
            \@key_bytes,
            @datasets,
        ]
    ) or die $graph->error;


    open my $ofh,">", "doc/$name.keyspeed.png"
        or die "Failed to open doc/$name.keyspeed.pn";
    print $ofh $gd->png;
    close $ofh;
}

my $output_name;
if ($ARGV[0] eq "--name") {
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
        next if $info->{key_bytes} > 128; # skip the long stuff for now
        if (!$hash{$name}{key_bytes}{ $info->{key_bytes} }) {
            push @{$hash{$name}{ array }}, $info;
            $hash{$name}{key_bytes}{ $info->{key_bytes} }= $info;
        }
    }
}
graph(\%hash,$output_name);
