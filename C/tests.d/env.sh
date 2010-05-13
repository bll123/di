#!/bin/sh

echo ${EN} "check environment in make${EC}" >&5

cd ${_MKCONFIG_RUNTOPDIR}
rc=$?
if [ $rc -ne 0 ]; then
  echo "ERROR: Unable to cd to ${_MKCONFIG_RUNTOPDIR}"
  exit $rc
fi

make di.env

. ./di.env

make --version | egrep "GNU Make" > /dev/null 2>&1
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

cat > e1 <<_HERE_
${_MKCONFIG_SYSTYPE}
${_MKCONFIG_SYSREV}
${_MKCONFIG_SYSARCH}
${CC}
${_MKCONFIG_USING_GCC}
${CFLAGS}
${LDFLAGS}
${LIBS}
${OBJ_EXT}
${EXE_EXT}
${XMSGFMT}
_HERE_

echo "## diff e1 (env) e2 (make)"
diff -b e1 e2
rc=$?

exit $rc
