#!/bin/sh
#
#  Copyright 2010 Brad Lanam Walnut Creek, CA USA
#

echo ${EN} "install${EC}" >&3


cd ..
grc=0
instdir="`pwd`/test_di"
make prefix=${instdir} LOCALEDIR=${prefix}/share/locale install
rc=$?
if [ $rc -ne 0 ]; then grc=$rc; fi
${instdir}/bin/di
rc=$?
if [ $rc -ne 0 ]; then grc=$rc; fi

exit $grc
