#!/bin/sh
#
#  Copyright 2010 Brad Lanam Walnut Creek, CA USA
#

if [ "$1" = "-d" ]; then
  echo ${EN} " rpmbuild${EC}"
  exit 0
fi

cd $_MKCONFIG_RUNTOPDIR
. ./mkconfig.cache

if [ "${di_c__command_rpmbuild}" = "0" ];then
  echo ${EN} " skipped${EC}" >&5
  exit 0
fi

rvers=`rpmbuild --version | tr -cd '0-9' | sed 's/^\(...\).*/\1/'`
if [ $rvers -lt 470 ]; then
  echo ${EN} " old version skipped${EC}" >&5
  exit 0
fi

DI_VERSION=`grep DI_VERSION version.h | sed  -e 's/"$//' -e 's/.*"//'`

make -e di.env
. ./di.env

grc=0
make -e DI_DIR=".." DI_VERSION=${DI_VERSION} testrpmbuild
rc=$?
if [ $rc -ne 0 ]; then grc=$rc; fi
# leave a copy there...realclean will get them...
cp mkconfig.log mkconfig.cache mkconfig*.vars di.env mkconfig.reqlibs \
    $_MKCONFIG_TSTRUNTMPDIR

exit $grc
