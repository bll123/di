#!/bin/sh
#
#  Copyright 2010 Brad Lanam Walnut Creek, CA USA
#

echo ${EN} "build w/mkconfig.sh${EC}" >&3

dotest () {
  sh=$1

  _MKCONFIG_SHELL=${sh}
  export _MKCONFIG_SHELL
  unset _shell
  unset shell
  cmd="$sh -c \". $MKCONFIG_DIR/shellfuncs.sh;getshelltype;echo \\\$shell\""
  shell=`eval $cmd`
  if [ "$shell" = "sh" ]; then
    shell=`echo ${_MKCONFIG_SHELL} | sed 's,.*/,,'`
  fi
  echo ${EN} "${shell} ${EC}" >&3
  echo "   testing with ${_MKCONFIG_SHELL} "
  make distclean
  instdir="`pwd`/test_di"
  make -e prefix=${instdir} all-sh
  rc=$?
  if [ $rc != 0 ]; then grc=$rc; fi
}

grc=0
echo ${EN} " ${EC}" >&3

. $MKCONFIG_DIR/shellfuncs.sh
cd $RUNTOPDIR
getlistofshells
for s in $shelllist; do
  dotest $s
done

exit $grc
