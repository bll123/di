#!/bin/sh

. $_MKCONFIG_DIR/testfuncs.sh

maindodisplay $1 rpmbuild
maindoquery $1 $_MKC_ONCE

getsname $0
dosetup $@

cd $_MKCONFIG_RUNTOPDIR
. ./mkconfig.cache

if [ "${mkc_c__command_rpmbuild}" = "0" ];then
  echo ${EN} " skipped${EC}" >&5
  exit 0
fi

march=`rpmbuild --showrc | grep '^build arch' | sed 's/.*: *//'`
echo "## Machine Architecture: ${march}"
rvers=`rpmbuild --version | tr -cd '0-9' | sed 's/^\(...\).*/\1/'`
if [ $rvers -lt 470 ]; then
  echo ${EN} " old version skipped${EC}" >&5
  exit 0
fi

DI_VERSION=`grep DI_VERSION version.h | sed  -e 's/"$//' -e 's/.*"//'`

make -e di.env
. ./di.env

grc=0
make -e DI_DIR=".." DI_VERSION=${DI_VERSION} MARCH=${march} testrpmbuild
rc=$?
if [ $rc -ne 0 ]; then grc=$rc; fi
# leave a copy there...realclean will get them...
cp mkconfig.log mkconfig.cache mkconfig*.vars di.env mkconfig.reqlibs \
    $_MKCONFIG_TSTRUNTMPDIR

exit $grc
