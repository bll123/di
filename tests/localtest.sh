#!/bin/sh
#
# Copyright 2025 Brad Lanam Pleasant Hill, CA
#

grc=0

if [ -f VERSION.txt ]; then
  . ./VERSION.txt
fi
systype=`uname -s`

if [ $# -gt 0 -a $1 = mkc ]; then
  LD_LIBRARY_PATH=`pwd`
  runpath=.
elif [ $# -gt 0 ]; then
  LD_LIBRARY_PATH=${1}
  SRC=${2}
  runpath=${1}
  . ${SRC}/VERSION.txt
else
  echo "## unknown path"
  exit 2
fi
export LD_LIBRARY_PATH

if [ $systype = Darwin ]; then
  DYLD_FALLBACK_LIBRARY_PATH=${LD_LIBRARY_PATH}
  export DYLD_FALLBACK_LIBRARY_PATH
  unset LD_LIBRARY_PATH
fi

${runpath}/dimathtest
rc=$?
if [ $rc -ne 0 ]; then
  echo "FAIL: math tests"
  grc=1
fi

${runpath}/getoptn_test
rc=$?
if [ $rc -ne 0 ]; then
  echo "FAIL: getoptn tests"
  grc=1
fi

vers=`${runpath}/di --version`
case ${vers} in
  "di version ${DI_VERSION} ${DI_RELEASE_STATUS}")
    ;;
  *)
    echo "FAIL: version mismatch /${vers}/${DI_VERSION}/"
    grc=1
    ;;
esac

vers=`${runpath}/di -X 1 | grep "# version:"`
case ${vers} in
  "# version: ${DI_VERSION}")
    ;;
  *)
    echo "FAIL: library version mismatch /${vers}/${DI_VERSION}/"
    grc=1
    ;;
esac

out="`${runpath}/di -n /`"
case "$out" in
  "")
    echo "FAIL: no output"
    grc=1
    ;;
  " "*)
    echo "FAIL: leading space"
    grc=1
    ;;
  *)
    ;;
esac

out="`DI_ARGS="-f SMbuvp" ${runpath}/di -n /`"
case "$out" in
  "")
    echo "FAIL: no output (di_args)"
    grc=1
    ;;
  " "*)
    echo "FAIL: leading space"
    grc=1
    ;;
  *)
    ;;
esac

echo "localtest: $grc"
exit $grc
