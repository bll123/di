#!/bin/sh
#
# Copyright 2026 Brad Lanam Pleasant Hill CA
#

INST_PKGCDIR=$1
PREFIX=$2
INCDIR=$3
LIBDIR=$4

if [ "x${INST_PKGCDIR}" = x ]; then
  echo "instpc.sh: installation dir not specified"
  exit 1
fi

if [ "x${PREFIX}" = x ]; then
  echo "instpc.sh: prefix dir not specified"
  exit 1
fi

if [ "x${INCDIR}" = x ]; then
  echo "instpc.sh: include dir not specified"
  exit 1
fi

if [ "x${LIBDIR}" = x ]; then
  echo "instpc.sh: include dir not specified"
  exit 1
fi

. ./VERSION.txt

test -d ${INST_PKGCDIR} || mkdir -p ${INST_PKGCDIR}
cat di.pc.in | \
sed -e "s,@prefix@,${PREFIX},g" \
  -e "s,@includedir@,${INCDIR},g" \
  -e "s,@libdir@,${LIBDIR},g" \
  -e "s,@DI_VERSION@,${DI_VERSION},g" \
  > ${INST_PKGCDIR}/di.pc

exit 0
