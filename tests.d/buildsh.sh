#!/bin/sh
#
#  Copyright 2010 Brad Lanam Walnut Creek, CA USA
#

echo ${EN} "build w/mkconfig.sh${EC}" >&3

cd ..
make distclean
instdir="`pwd`/test_di"
make prefix=${instdir} LOCALEDIR=${prefix}/share/locale all-sh
rc=$?

exit $rc
