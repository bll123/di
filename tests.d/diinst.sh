#!/bin/sh

. $_MKCONFIG_DIR/testfuncs.sh

maindodisplay $1 install
maindoquery $1 $_MKC_ONCE

getsname $0
dosetup $@

cd $_MKCONFIG_RUNTOPDIR
grc=0
instdir="`pwd`/test_di"
make -e prefix=${instdir} install
rc=$?

# leave a copy laying around...make realclean will clean it up
cp mkconfig.log mkconfig.cache mkconfig*.vars di.env mkconfig.reqlibs \
    $_MKCONFIG_TSTRUNTMPDIR

if [ $rc -ne 0 ]; then grc=$rc; fi
${instdir}/bin/di
rc=$?
if [ $rc -ne 0 ]; then grc=$rc; fi

exit $grc
