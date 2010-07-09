#!/bin/sh
#
#  Copyright 2010 Brad Lanam Walnut Creek, CA USA
#

if [ "$1" = "-d" ]; then
  echo ${EN} "build w/mkconfig.sh${EC}"
  exit 0
fi

. $_MKCONFIG_DIR/shellfuncs.sh
testshcapability

count=1

dotest () {
  sh=$1

  _MKCONFIG_SHELL=${sh}
  export _MKCONFIG_SHELL
  unset _shell
  unset shell
  cmd="$sh -c \". $_MKCONFIG_DIR/shellfuncs.sh;getshelltype;echo \\\$shell\""
  shell=`eval $cmd`
  if [ "$shell" = "sh" ]; then
    shell=`echo ${_MKCONFIG_SHELL} | sed 's,.*/,,'`
  fi
  echo ${EN} "${shell} ${EC}" >&5
  echo "##"
  echo "##   testing with ${_MKCONFIG_SHELL} "
  echo "##"
  make distclean
  instdir="`pwd`/test_di"
  make -e prefix=${instdir} all-sh
  rc=$?
  if [ $rc != 0 ]; then grc=$rc; fi
  mkdir -p $_MKCONFIG_TSTRUNTMPDIR/${count}_${shell}
  # leave these laying around for use by install.sh and rpmbuild.sh test
  cp mkconfig.log mkconfig.cache mkconfig*.vars di.env reqlibs.txt \
      $_MKCONFIG_TSTRUNTMPDIR/${count}_${shell}
  domath count "$count + 1"
}

grc=0
echo ${EN} " ${EC}" >&5

cd $_MKCONFIG_RUNTOPDIR
for s in $shelllist; do
  dotest $s
done

exit $grc
