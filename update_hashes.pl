use strict;
use warnings;

my %fix;
chomp(my @broken= `./SMHasher --validate`);
foreach my $line (@broken) {
    if ($line=~/^(\w+)\s+- Verification value (0x\S+)/) {
        $fix{$1}= $2;
    }
}

my $contents= do { local(@ARGV,$/)=("main.cpp"); <>};
my $hashes= 
$contents=~/HashInfo\s+g_hashes\[\]\s*=\s*\{(.*?)\};/s;
my $inner= $1;

sub _format {
    return 
      "{ $_[7], $_[8],\n" . 
    "    $_[3], $_[4], $_[5], $_[6],\n".
    "    $_[1], $_[2], $_[0] }";
}

# { mum_hash_test, NULL, NULL, 32, 32, 64, MUM_VERIFY, "MUM",
#     "github.com/vnmakarov/mum-hash", NULL }
$inner=~s/\{\s*(\w+)\s*,\s*(\w+)\s*,\s*(\w+)\s*,
           \s*([^,]+?)\s*,\s*([^,]+?)\s*,\s*([^,]+?)\s*,
           \s*([^\s,]+)\s*,\s*("[^"]+")\s*,
           \s*("[^"]+")\s*(?:,\s*(\w+)\s*)?\}/
            _format($1,$2,$3,$4,$5,$6,$7,$8,$9,$10)
        /xge;

$inner=~s/(0x[A-Fa-f0-9]+)(\s*,\s*"([^"]+)")/($fix{$3}||$1).$2/ge;
$contents=~s/(HashInfo\s+g_hashes\[\]\s*=\s*\{)(.*?)(\};)/$1$inner$3/s;

open my $fh,">", "main.cpp"
    or die "Failed to open main.cpp for write: $!";
print $fh $contents;
close $fh;
