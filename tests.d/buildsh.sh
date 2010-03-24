#!/bin/sh
#
#  Copyright 2010 Brad Lanam Walnut Creek, CA USA
#

echo ${EN} "build w/mkconfig.sh${EC}" >&3

dotest () {
  echo ${EN} "${shell} ${EC}" >&3
  echo "   testing with ${_MKCONFIG_SHELL} "
  make distclean
  instdir="`pwd`/test_di"
  time make -e prefix=${instdir} all-sh
  rc=$?
  if [ $rc != 0 ]; then grc=$rc; fi
}

cd $RUNTOPDIR
grc=0
echo ${EN} " ${EC}" >&3

plist=""
ls -ld /bin | grep -- '->' > /dev/null 2>&1
if [ $? -ne 0 ]; then
  plist=/bin
fi
plist="${plist} /usr/bin /usr/local/bin"

for p in $plist; do
  for s in sh bash posh ash dash; do
    if [ -x $p/$s ]; then
      ls -l $p/$s | grep -- '->' > /dev/null 2>&1
      rc1=$?
      ls -l $p/$s | grep '/etc/alternatives' > /dev/null 2>&1
      rc2=$?
      if [ $rc1 -ne 0 -o $rc2 -eq 0 ]; then
        _MKCONFIG_SHELL=${p}/${s}
        export _MKCONFIG_SHELL
        shell=$s
        dotest
      fi
    fi
  done
  for s in ksh; do
    if [ -x $p/$s ]; then
      ls -l $p/$s | grep -- '->' > /dev/null 2>&1
      rc1=$?
      ls -l $p/$s | grep '/etc/alternatives' > /dev/null 2>&1
      rc2=$?
      if [ $rc1 -ne 0 -o $rc2 -eq 0 ]; then
        cmd="$p/$s -c \". $mypath/shellfuncs.sh;getshelltype;echo \\\$shell\""
        tshell=`eval $cmd`
        case $tshell in
          pdksh)
            ;;
          *)
            _MKCONFIG_SHELL=${p}/${s}
            export _MKCONFIG_SHELL
            dotest
            ;;
        esac
      fi
    fi
  done
done

exit $grc
