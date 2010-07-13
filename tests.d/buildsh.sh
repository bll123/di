#!/bin/sh
#
#  Copyright 2010 Brad Lanam Walnut Creek, CA USA
#

if [ "$1" = "-d" ]; then
  echo ${EN} " build w/mkconfig.sh${EC}"
  exit 0
fi

. $_MKCONFIG_DIR/shellfuncs.sh
testshcapability

grc=0

cd $_MKCONFIG_RUNTOPDIR

instdir="`pwd`/test_di"
make realclean
make -e prefix=${instdir} all-sh
rc=$?
if [ $rc != 0 ]; then grc=$rc; fi
mkdir -p $_MKCONFIG_TSTRUNTMPDIR/buildsh${stag}
# leave these laying around for use by install.sh and rpmbuild.sh test
cp mkconfig.log mkconfig.cache mkconfig*.vars di.env mkconfig.reqlibs \
    $_MKCONFIG_TSTRUNTMPDIR/buildsh${stag}

exit $grc
