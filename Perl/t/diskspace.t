# Before 'make install' is performed this script should be runnable with
# 'make test'. After 'make install' it should work as 'perl Filesys-di.t'

#########################

use strict;
use warnings;

use Test::More;
BEGIN { use_ok( 'Filesys::di', qw(diskspace) ); }
#use Data::Dumper;

#########################

# Insert your test code below, the Test::More module is use()ed here so read
# its man page ( perldoc Test::More ) for help writing this test script.

my $h = diskspace ('');
my @a = keys %{$h};
ok ($#a > 0, 'standard');
$h = diskspace ('-d 1 -f buvp');
@a = keys %{$h};
my $path = '/';
if (! exists($h->{$path})) {
  $path = 'C:\\';
}
my $disp = $h->{$path}->{'display'};
my $count = $#{$h->{$path}->{'display'}};
ok ($#a > 0 && $count == 3, 'with arguments');
my $val = $h->{$path}->{'display'}[1];
ok ($#a > 0 && $count == 3 && $val =~ /^\d+$/, 'has a value');
done_testing();
