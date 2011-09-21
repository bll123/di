#!/bin/sh

. $_MKCONFIG_DIR/testfuncs.sh

maindodisplay $1 'build w/mkconfig.pl'
maindoquery $1 $_MKC_ONCE

getsname $0
dosetup $@

cd $_MKCONFIG_RUNTOPDIR
make realclean
instdir="`pwd`/test_di"
make -e prefix=${instdir} all-perl
grc=$?
# leave a copy there...realclean will get them...
cp mkconfig.log mkconfig.cache mkconfig*.vars di.env mkconfig.reqlibs \
    $_MKCONFIG_TSTRUNTMPDIR

exit $grc
