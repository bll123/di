#!/bin/sh

. $_MKCONFIG_DIR/bin/testfuncs.sh

maindodisplay $1 install
maindoquery $1 $_MKC_ONCE

getsname $0
dosetup $@

for d in C D; do
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
    cd ${tdir}
    grc=0

    instdir="`pwd`/test_di"
    unset MAKEFLAGS
    ${MAKE:-make} ${TMAKEFLAGS} -e prefix=${instdir} install
    rc=$?

    # leave a copy laying around...make realclean will clean it up
    set +f
    cp mkconfig.log mkconfig.cache mkc*.vars di.env di.reqlibs \
        $_MKCONFIG_TSTRUNTMPDIR
    set -f

    if [ $rc -ne 0 ]; then grc=$rc; fi
    ${instdir}/bin/di
    rc=$?
    if [ $rc -ne 0 ]; then grc=$rc; fi
    if [ $grc -ne 0 ]; then
      echo ${EN} "*${EC}" >&5
    fi
  else
    if [ $d = C ]; then
      echo "## no di executable found for dir $d"
      grc=1
    fi
  fi
done

exit $grc
