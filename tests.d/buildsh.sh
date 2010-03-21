#!/bin/sh
#
#  Copyright 2010 Brad Lanam Walnut Creek, CA USA
#

echo ${EN} "build w/mkconfig.sh${EC}" >&3

cd $RUNTOPDIR
grc=0
dotest () {
  echo ${EN} "${_MKCONFIG_SHELL} ${EC}" >&3
  echo "   testing with ${_MKCONFIG_SHELL} "
  make distclean
  instdir="`pwd`/test_di"
  time make -e prefix=${instdir} all-sh
  rc=$?
  if [ $rc != 0 ]; then grc=$rc; fi
}

echo ${EN} " ${EC}" >&3
if [ -x /bin/bash ]; then
  export _MKCONFIG_SHELL=bash
  dotest
fi
if [ -x /bin/ksh ]; then
  tshell=`/usr/bin/ksh -c ". $mypath/shellfuncs.sh;getshelltype"`
  case $tshell in
    pdksh)
      ;;
    *)
      _MKCONFIG_SHELL=ksh
      dotest
      ;;
  esac
fi
if [ -x /bin/ash ]; then
  _MKCONFIG_SHELL=ash
  dotest
fi
if [ -x /bin/dash ]; then
  _MKCONFIG_SHELL=dash
  dotest
fi

exit $grc
