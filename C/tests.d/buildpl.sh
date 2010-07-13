#!/bin/sh
#
#  Copyright 2010 Brad Lanam Walnut Creek, CA USA
#

if [ "$1" = "-d" ]; then
  echo ${EN} " build w/mkconfig.pl${EC}"
  exit 0
fi

cd $_MKCONFIG_RUNTOPDIR
make realclean
instdir="`pwd`/test_di"
make -e prefix=${instdir} all-perl
rc=$?
# leave a copy there...realclean will get them...
cp mkconfig.log mkconfig.cache mkconfig*.vars di.env reqlibs.txt \
    $_MKCONFIG_TSTRUNTMPDIR

exit $rc
