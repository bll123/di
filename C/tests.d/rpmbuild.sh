#!/bin/sh

. $_MKCONFIG_DIR/bin/testfuncs.sh

getsname $0
dosetup $@

maindodisplay $1 rpmbuild
maindoquery $1 $_MKC_ONCE

. $_MKCONFIG_DIR/bin/shellfuncs.sh
locatecmd locrpmbuild rpmbuild

if [ "${locrpmbuild}" = "" ];then
  putsnonl " skipped" >&5
  exit 0
fi

rvers=`rpmbuild --version | tr -cd '0-9' | sed 's/^\(...\).*/\1/'`
if [ $rvers -lt 470 ]; then
  putsnonl " old version skipped" >&5
  exit 0
fi

march=`rpmbuild --showrc | grep '^build arch' | sed 's/.*: *//'`
puts "## Machine Architecture: ${march}"

cd ${_MKCONFIG_RUNTOPDIR}

DI_VERSION=`grep DI_VERSION version.h | sed  -e 's/"$//' -e 's/.*"//'`

unset MAKEFLAGS
${MAKE:-make} ${TMAKEFLAGS} -e di.env
. ./di.env

grc=0
${MAKE:-make} ${TMAKEFLAGS} -e DI_DIR=".." DI_VERSION=${DI_VERSION} MARCH=${march} testrpmbuild
rc=$?
if [ $rc -ne 0 ]; then grc=$rc; fi
# leave a copy there...realclean will get them...
set +f
cp mkconfig.log mkconfig.cache mkc*.vars di.env di.reqlibs \
    $_MKCONFIG_TSTRUNTMPDIR
set -f

exit $grc
