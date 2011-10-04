#!/bin/sh

. $_MKCONFIG_DIR/testfuncs.sh

maindodisplay $1 'getoptn works'
maindoquery $1 $_MKC_ONCE

getsname $0
dosetup $@

cd $_MKCONFIG_RUNTOPDIR

set -x

make realclean
make getoptn_test.exe > ${_MKCONFIG_TSTRUNTMPDIR}/make.log 2>&1
rc=$?
if [ $rc != 0 ]; then grc=$rc; fi
if [ $grc -eq 0 ]; then
  # it would be preferable to have the interlaced output...
  ./getoptn_test.exe 2>&1 | sort > ${_MKCONFIG_TSTRUNTMPDIR}/getoptn_test.out
  chkdiff ${_MKCONFIG_RUNTESTDIR}/getoptn.txt \
      ${_MKCONFIG_TSTRUNTMPDIR}/getoptn_test.out
fi

exit $grc

