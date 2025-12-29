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
  runpath=.
  . ./VERSION.txt
elif [ $# -gt 0 ]; then
  runpath=${1}
  SRC=${2}
  . ${SRC}/VERSION.txt
else
  echo "## unknown path"
  exit 2
fi

case ${systype} in
  Darwin)
    if [ $# -gt 0 -a $1 = mkc ]; then
      DYLD_FALLBACK_LIBRARY_PATH=`pwd`
    elif [ $# -gt 0 ]; then
      DYLD_FALLBACK_LIBRARY_PATH=${1}
    else
      echo "## unknown path"
      exit 2
    fi
    export DYLD_FALLBACK_LIBRARY_PATH
    ;;
  MINGW64*)
    PATH=`pwd`:$PATH
    ;;
  *)
    if [ $# -gt 0 -a $1 = mkc ]; then
      LD_LIBRARY_PATH=`pwd`
    elif [ $# -gt 0 ]; then
      LD_LIBRARY_PATH=${1}
    else
      echo "## unknown path"
      exit 2
    fi
    export LD_LIBRARY_PATH
    ;;
esac

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
    echo "FAIL: no output [-n]"
    grc=1
    ;;
  " "*)
    case ${systype} in
      MINGW64*)
        ;;
      *)
        echo "FAIL: leading space [-n]"
        grc=1
        ;;
    esac
    ;;
  *)
    ;;
esac

out="`DI_ARGS="-f SMbuvp" ${runpath}/di -n /`"
case "$out" in
  "")
    echo "FAIL: no output (di_args) [-f SMbuvp]"
    grc=1
    ;;
  " "*)
    case ${systype} in
      MINGW64*)
        ;;
      *)
        echo "FAIL: leading space [-f SMbuvp]"
        grc=1
        ;;
    esac
    ;;
  *)
    ;;
esac

echo "localtest: $grc"
exit $grc
