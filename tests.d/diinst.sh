#!/bin/sh
#
#  Copyright 2010 Brad Lanam Walnut Creek, CA USA
#

echo ${EN} "install${EC}" >&3


cd $_MKCONFIG_RUNTOPDIR
grc=0
instdir="`pwd`/test_di"
make -e prefix=${instdir} install
rc=$?

# leave a copy laying around...make distclean will clean it up
cp mkconfig.log mkconfig.cache mkconfig*.vars di.env reqlibs.txt \
    $_MKCONFIG_TSTRUNTMPDIR

if [ $rc -ne 0 ]; then grc=$rc; fi
${instdir}/bin/di
rc=$?
if [ $rc -ne 0 ]; then grc=$rc; fi

exit $grc
