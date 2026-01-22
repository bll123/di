#!/bin/sh

set -x

dilibd=""
pcarg="di"
if [ $# -gt 0 ]; then
  dilibd=$1
  if [ -d ${dilibd}/lib ]; then
    dilibd=${dilibd}/lib
  fi
  if [ -d ${dilibd}/lib64 ]; then
    dilibd=${dilibd}/lib64
  fi
  if [ -d ${dilibd}/pkgconfig ]; then
    pcpath=${dilibd}/pkgconfig
    pcarg=${pcpath}/di.pc
  fi
fi
if [ "$CC" = "" ]; then
  CC=cc
fi

diinc=`pkg-config --cflags ${pcarg}`
rc=$?
if [ $rc -ne 0 ]; then
  exit 2
fi

if [ "$dilibd" = "" ]; then
  dilibd=`pkg-config --libs-only-L ${pcarg}`
  dilibd=`echo ${dilibd} | sed -e 's,^-L,,'`
fi
dilibs=`pkg-config --libs ${pcarg}`
rc=$?
if [ $rc -ne 0 ]; then
  exit 2
fi

systype=`uname -s`
case ${systype} in
  SunOS|NetBSD)
    libargs="-Wl,-R${dilibd}"
    ;;
  Darwin|Linux)
    libargs="-Wl,-rpath,${dilibd}"
    ;;
esac

${CC} -o diex ${diinc} diex.c ${libargs} ${dilibs}
rc=$?

exit $rc
