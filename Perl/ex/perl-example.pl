use lib './blib/lib';
use lib './blib/arch/auto/Filesys/di';
use Filesys::di;
use Data::Dumper;

my $r_h = diskspace ('');
print Dumper ($r_h);
$r_h = Filesys::di::diskspace ('-f buvp');
if ( exists($r_h->{'/'}) ) {
  # unix
  my $r_a = $r_h->{'/'}->{'display'};
  my @b = @{$r_a};
  my $count = $#{$r_a};
}
print Dumper ($r_h);
