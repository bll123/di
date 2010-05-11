#!/bin/sh
#
#  Copyright 2010 Brad Lanam Walnut Creek, CA USA
#

echo ${EN} "rpmbuild${EC}" >&3

cd $_MKCONFIG_RUNTOPDIR
. ./mkconfig.cache

if [ "${di_c__command_rpmbuild}" = "0" ];then
  echo ${EN} " skipped${EC}" >&3
  exit 0
fi

rvers=`rpmbuild --version | tr -cd '0-9'`
if [ $rvers -lt 470 ]; then
  echo ${EN} " old version skipped${EC}" >&3
  exit 0
fi

DI_VERSION=`grep DI_VERSION version.h | sed  -e 's/"$//' -e 's/.*"//'`

grc=0
make -e DI_DIR="." DI_VERSION=${DI_VERSION} testrpmbuild
rc=$?
if [ $rc -ne 0 ]; then grc=$rc; fi
# leave a copy there...distclean will get them...
cp mkconfig.log mkconfig.cache mkconfig*.vars di.env reqlibs.txt \
    $_MKCONFIG_TSTRUNTMPDIR

exit $grc
