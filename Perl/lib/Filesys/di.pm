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

  my $di = Filesys::di::diskspace ('-d 1 -f buvp');

=head1 DESCRIPTION

Returns a reference to a hash.  The hash keys are the mount points.

Each hash contains the following keys: device (special device name),
fstype (filesystem type), total, free, available, totalinodes, freeinodes,
availableinodes, mountoptions.

If a format is specified, the hash will also contain a 'display' key
pointing to a reference to an array containing the display output from
the 'di' program.

Perl XS does not seem to have a long long type, so there may
be a loss of precision in the conversion of long long to double.

See the main 'di' documentation for usage.

=head2 EXPORT

None by default.

=head1 SEE ALSO

di(1)

http://gentoo.com/di/

=head1 AUTHOR

Brad Lanam, E<lt>brad.lanam.di@gmail.comE<gt>

=head1 COPYRIGHT AND LICENSE

Copyright 2016 Brad Lanam, Walnut Creek CA

See the LICENSE file included with the 'di' distribution.

=cut
