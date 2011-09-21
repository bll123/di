#!/bin/sh

. $_MKCONFIG_DIR/testfuncs.sh

maindodisplay $1 'check environment in make'
maindoquery $1 $_MKC_SH

getsname $0
dosetup $@

cd ${_MKCONFIG_RUNTOPDIR}
rc=$?
if [ $rc -ne 0 ]; then
  echo "ERROR: Unable to cd to ${_MKCONFIG_RUNTOPDIR}"
  exit $rc
fi

make di.env

. ./di.env

make --version | egrep "GNU Make"
rc=$?
if [ $rc -eq 0 ]; then
  make -e --no-print-directory rtest-env > \
        ${_MKCONFIG_TSTRUNTMPDIR}/e2 2>/dev/null
else
  make -e rtest-env > ${_MKCONFIG_TSTRUNTMPDIR}/e2 2>/dev/null
fi

cd ${_MKCONFIG_TSTRUNTMPDIR}
rc=$?
if [ $rc -ne 0 ]; then
  echo "ERROR: Unable to cd to ${_MKCONFIG_TSTRUNTMPDIR}"
  exit $rc
fi

> e1 echo "${_MKCONFIG_SYSTYPE}
${_MKCONFIG_SYSREV}
${_MKCONFIG_SYSARCH}
${CC}
${_MKCONFIG_USING_GCC}
${CFLAGS}
${LDFLAGS}
${LIBS}
${OBJ_EXT}
${EXE_EXT}
${XMSGFMT}"

echo "## diff e1 (env) e2 (make)"
diff -w e1 e2
rc=$?

exit $rc
