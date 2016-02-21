use lib './blib/lib';
use lib './blib/arch/auto/Filesys/di';
use Filesys::di;
use Data::Dumper;

my $h = diskspace ('');
print Dumper ($h);
$h = Filesys::di::diskspace ('-f buvp');
print Dumper ($h);
