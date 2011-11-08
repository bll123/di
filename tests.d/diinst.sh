#!/bin/sh

. $_MKCONFIG_DIR/bin/testfuncs.sh

maindodisplay $1 install
maindoquery $1 $_MKC_ONCE

getsname $0
dosetup $@

cd $_MKCONFIG_RUNTOPDIR
grc=0
instdir="`pwd`/test_di"
unset MAKEFLAGS
${MAKE:-make} ${TMAKEFLAGS} -e prefix=${instdir} install
rc=$?

# leave a copy laying around...make realclean will clean it up
set +f
cp mkconfig.log mkconfig.cache mkc*.vars di.env di.reqlibs \
    $_MKCONFIG_TSTRUNTMPDIR
set -f

if [ $rc -ne 0 ]; then grc=$rc; fi
${instdir}/bin/di
rc=$?
if [ $rc -ne 0 ]; then grc=$rc; fi

exit $grc
