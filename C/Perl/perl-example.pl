#use lib '../test-perl/local/lib/perl/5.18.2';
use Filesys::di;
use Data::Dumper;

my $h = diskspace ('');
print Dumper ($h);
$h = Filesys::di::diskspace ('-f buvp');
print Dumper ($h);
