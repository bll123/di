#!/bin/sh

. $_MKCONFIG_DIR/bin/testfuncs.sh

maindodisplay $1 'check environment in make'
maindoquery $1 $_MKC_ONCE

getsname $0
dosetup $@

cd ${_MKCONFIG_RUNTOPDIR}
rc=$?
if [ $rc -ne 0 ]; then
  echo "ERROR: Unable to cd to ${_MKCONFIG_RUNTOPDIR}"
  exit $rc
fi

${MAKE:-make} di.env

. ./di.env

unset MAKEFLAGS
${MAKE:-make} ${TMAKEFLAGS} --version 2>&1 | egrep "GNU Make" > /dev/null 2>&1
rc=$?
if [ $rc -eq 0 ]; then
  ${MAKE:-make} ${TMAKEFLAGS} -e --no-print-directory rtest-env |
    sed -e 's/^ *//' -e 's/ *$//' > ${_MKCONFIG_TSTRUNTMPDIR}/e2 2>/dev/null
        ${_MKCONFIG_TSTRUNTMPDIR}/e2 2>/dev/null
else
  ${MAKE:-make} ${TMAKEFLAGS} -e rtest-env |
    sed -e 's/^ *//' -e 's/ *$//' > ${_MKCONFIG_TSTRUNTMPDIR}/e2 2>/dev/null
fi

cd ${_MKCONFIG_TSTRUNTMPDIR}
rc=$?
if [ $rc -ne 0 ]; then
  echo "ERROR: Unable to cd to ${_MKCONFIG_TSTRUNTMPDIR}"
  exit $rc
fi

echo "${_MKCONFIG_SYSTYPE}
${_MKCONFIG_SYSREV}
${_MKCONFIG_SYSARCH}
${CC}
${_MKCONFIG_USING_GCC}
${CFLAGS}
${LDFLAGS}
${LIBS}
${OBJ_EXT}
${EXE_EXT}
${XMSGFMT}" |
sed -e 's/^ *//' -e 's/ *$//' > e1

chkdiff e1 e2

testcleanup e1 e2 env.log

exit $grc
