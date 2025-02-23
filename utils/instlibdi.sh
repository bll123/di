#!/bin/sh

INST_LIBDIR=$1
INST_BINDIR=$2
DI_LIBVERSION=$3
DI_SOVERSION=$4
# SHLIB_EXT is in the environment

grc=0

if [ "x${INST_LIBDIR}" = x ]; then
  echo "INST_LIBDIR is not set"
  exit 1
fi

if [ "x${INST_BINDIR}" = x ]; then
  echo "INST_BINDIR is not set"
  exit 1
fi

if [ "x${DI_LIBVERSION}" = x ]; then
  echo "DI_LIBVERSION is not set"
  exit 1
fi

if [ "x${DI_SOVERSION}" = x ]; then
  echo "DI_SOVERSION is not set"
  exit 1
fi

if [ ! -d "${INST_LIBDIR}" ]; then
  echo "INST_LIBDIR does not exist"
  exit 1
fi

sym=T
instdest=${INST_LIBDIR}

case `uname -s` in
  Darwin)
    libnm=libdi.${DI_LIBVERSION}${SHLIB_EXT}
    libnmso=libdi.${DI_SOVERSION}${SHLIB_EXT}
    ;;
  CYGWIN*|MSYS*|MINGW*)
    instdest=${INST_BINDIR}
    libnm=libdi${SHLIB_EXT}
    sym=F
    ;;
  OpenBSD)
    libnm=libdi${SHLIB_EXT}.${DI_LIBVERSION}
    libnmso=libdi${SHLIB_EXT}.${DI_SOVERSION}
    sym=O
    ;;
  *)
    libnm=libdi${SHLIB_EXT}.${DI_LIBVERSION}
    libnmso=libdi${SHLIB_EXT}.${DI_SOVERSION}
    ;;
esac

cp -f libdi${SHLIB_EXT} ${instdest}/${libnm}
rc=$?
if [ $rc -ne 0 ]; then
  grc=1
fi

if [ $grc -eq 0 -a \( $sym = T -o $sym = O \) ]; then
  (
    cd "${instdest}"
    ln -sf ${libnm} ${libnmso}
    rc=$?
    if [ $rc -ne 0 ]; then
      grc=1
    fi
  )
fi

if [ $grc -eq 0 -a $sym = T ]; then
  (
    cd "${instdest}"
    ln -sf ${libnmso} libdi${SHLIB_EXT}
    rc=$?
    if [ $rc -ne 0 ]; then
      grc=1
    fi
  )
fi

exit $grc
