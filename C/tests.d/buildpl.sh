#!/bin/sh

. $_MKCONFIG_DIR/bin/testfuncs.sh

maindodisplay $1 'build w/mkconfig.pl'
maindoquery $1 $_MKC_ONCE

getsname $0
dosetup $@

perl -v > /dev/null 2>&1
rc=$?
if [ $rc -ne 0 ] ; then
  # perl is not available
  exit 0
fi

cd $_MKCONFIG_RUNTOPDIR
unset MAKEFLAGS
${MAKE:-make} ${TMAKEFLAGS} realclean
instdir="`pwd`/test_di"
${MAKE:-make} ${TMAKEFLAGS} -e prefix=${instdir} all-perl
grc=$?
# leave a copy there...realclean will get them...
set +f
cp mkconfig.log mkconfig.cache mkc*.vars di.env di.reqlibs \
    $_MKCONFIG_TSTRUNTMPDIR
set -f

exit $grc
