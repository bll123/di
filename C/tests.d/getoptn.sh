#!/bin/sh

. $_MKCONFIG_DIR/bin/testfuncs.sh

maindodisplay $1 'getoptn'
maindoquery $1 $_MKC_ONCE

getsname $0
dosetup $@

cd $_MKCONFIG_RUNTOPDIR

unset MAKEFLAGS
${MAKE:-make} ${TMAKEFLAGS} realclean
${MAKE:-make} ${TMAKEFLAGS} -e di.env
. ./di.env
${MAKE:-make} ${TMAKEFLAGS} -e getoptn_test.exe > ${_MKCONFIG_TSTRUNTMPDIR}/make.log 2>&1
rc=$?
if [ $rc != 0 ]; then grc=$rc; fi
if [ $grc -eq 0 ]; then
  ./getoptn_test.exe > ${_MKCONFIG_TSTRUNTMPDIR}/getoptn_test.out 2>&1
  chkdiff ${_MKCONFIG_RUNTESTDIR}/getoptn.txt \
      ${_MKCONFIG_TSTRUNTMPDIR}/getoptn_test.out
fi

exit $grc

