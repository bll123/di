#!/bin/sh
#
#  Copyright 2010 Brad Lanam Walnut Creek, CA USA
#

echo ${EN} "build w/mkconfig.pl${EC}" >&3

cd $RUNTOPDIR
make distclean
instdir="`pwd`/test_di"
make -e prefix=${instdir} all-perl
rc=$?

exit $rc
