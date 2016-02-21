package Filesys::di;

use 5.00503;
use strict;

require Exporter;
require DynaLoader;
use vars qw($VERSION @ISA @EXPORT @EXPORT_OK %EXPORT_TAGS);
@ISA = qw(Exporter
	DynaLoader);

# Items to export into callers namespace by default. Note: do not export
# names by default without a very good reason. Use EXPORT_OK instead.
# Do not simply export all your public functions/methods/constants.

# This allows declaration	use Filesys::di ':all';
# If you do not need this, moving things directly into @EXPORT or @EXPORT_OK
# will save memory.
%EXPORT_TAGS = ( 'all' => [ qw(

) ] );

@EXPORT_OK = ( @{ $EXPORT_TAGS{'all'} } );

@EXPORT = qw(
  diskspace
);

$VERSION = '0.01';

bootstrap Filesys::di $VERSION;

# Preloaded methods go here.

1;
__END__
# Below is stub documentation for your module. You'd better edit it!

=head1 NAME

Filesys::di - Perl extension for 'di'

=head1 SYNOPSIS

  use Filesys::di;

  my $di = Filesys::di::diskspace ('');

  my $di = Filesys::di::diskspace ('-f buvp');

=head1 DESCRIPTION

See the main 'di' documentation for argument usage.

=head2 EXPORT

None by default.

=head1 SEE ALSO

di(1)

http://gentoo.com/di/

=head1 AUTHOR

Brad Lanam, E<lt>brad.lanam.di@gmail.com<gt>

=head1 COPYRIGHT AND LICENSE

Copyright 2016 Brad Lanam, Walnut Creek CA

See the LICENSE file included with the 'di' distribution.

=cut
