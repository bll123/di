#!/bin/sh
#
#  Copyright 2010 Brad Lanam Walnut Creek, CA USA
#

echo ${EN} "build w/mkconfig.pl${EC}" >&5

cd $_MKCONFIG_RUNTOPDIR
make distclean
instdir="`pwd`/test_di"
make -e prefix=${instdir} all-perl
rc=$?
# leave a copy there...distclean will get them...
cp mkconfig.log mkconfig.cache mkconfig*.vars di.env reqlibs.txt \
    $_MKCONFIG_TSTRUNTMPDIR

exit $rc
