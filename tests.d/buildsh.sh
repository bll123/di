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
for s in /bin/sh /bin/bash /bin/posh /bin/ash /bin/dash; do
  if [ -x $s ]; then
    ls -l $s | grep -- '->' > /dev/null 2>&1
    rc1=$?
    ls -l $s | grep '/etc/alternatives' > /dev/null 2>&1
    rc2=$?
    if [ $rc1 -ne 0 -o $rc2 -eq 0 ]; then
      _MKCONFIG_SHELL=`echo $s | sed 's,.*/,,'`
      export _MKCONFIG_SHELL
      dotest
    fi
  fi
done
for s in /usr/bin/ksh /bin/ksh; do 
  if [ $s = "/bin/ksh" ]; then
    ls -l /bin | grep -- '->' > /dev/null 2>&1
    if [ $? -eq 0 ]; then
      continue
    fi
  fi

  if [ -x $s ]; then
    ls -l $s | grep -- '->' > /dev/null 2>&1
    rc1=$?
    ls -l $s | grep '/etc/alternatives' > /dev/null 2>&1
    rc2=$?
    if [ $rc1 -ne 0 -o $rc2 -eq 0 ]; then
      cmd="$s -c \". $mypath/shellfuncs.sh;getshelltype;echo \\\$shell\""
      tshell=`eval $cmd`
      case $tshell in
        pdksh)
          ;;
        *)
          _MKCONFIG_SHELL=ksh
          export _MKCONFIG_SHELL
          dotest
          ;;
      esac
    fi
  fi
done

exit $grc
