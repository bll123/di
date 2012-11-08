#!/bin/sh

. $_MKCONFIG_DIR/bin/testfuncs.sh

maindodisplay $1 'execute di'
maindoquery $1 $_MKC_ONCE

getsname $0
dosetup $@

unset DI_ARGS
unset DIFMT
for d in C D; do
  if [ $d = D -a \( "$DC" = "" -o "$DC" = "skip" \) ]; then
    continue
  fi
  tdir=$_MKCONFIG_RUNTOPDIR/$d
  (
    cd $tdir
    if [ $? -eq 0 ]; then
      instdir="`pwd`/test_di"
      ${MAKE:-make} ${TMAKEFLAGS} -e prefix=${instdir} all
        > ${_MKCONFIG_TSTRUNTMPDIR}/make.log 2>&1
    fi
  )
  if [ -x ${tdir}/di ]; then
    echo ${EN} " ${d}${EC}" >&5
    ${tdir}/di
    grc=$?
    if [ $grc -ne 0 ]; then
      echo ${EN} "*${EC}" >&5
    else
      # look for invalid floating point numbers
      ${tdir}/di -n | egrep '(inf|nan)'
      rc=$?
      if [ $rc -eq 0 ]; then
        echo ${EN} "*${EC}" >&5
        grc=1
      fi
    fi
  else
    if [ $d = C ]; then
      echo "## no di executable found for dir $d"
      grc=1
    fi
  fi
done

exit $grc
