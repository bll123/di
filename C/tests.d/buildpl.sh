#!/bin/sh
#
#  Copyright 2010 Brad Lanam Walnut Creek, CA USA
#

echo ${EN} "build w/mkconfig.pl${EC}" >&3

cd ..
make distclean
instdir="`pwd`/test_di"
make -e prefix=${instdir} LOCALEDIR=${prefix}/share/locale all-perl
rc=$?

exit $rc
