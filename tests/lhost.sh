#!/bin/bash
#
# Copyright 2025 Brad Lanam Pleasant Hill CA
#

SRCHOST=192.168.2.13
SRCDIR=s/di
PORT=166
USER=bll
comp=cc
host=`uname -n`


keep=${1:-F}

scp -P ${PORT} ${USER}@${SRCHOST}:${SRCDIR}/di-*.tar.gz \
    ${USER}@${SRCHOST}:${SRCDIR}/tests/dibldrun.sh .
chmod a+rx dibldrun.sh
tarfn=$(echo di-*.tar.gz)
didir=$(echo ${tarfn} | sed 's,\.tar.gz$,,')

rsltdir=test_results/${host}_${comp}

./dibldrun.sh ${host} ${tarfn} ${didir} ${comp}
ssh -p ${PORT} -l ${USER} ${SRCHOST}: \
    "test -d ${SRCDIR}/${rsltdir} || mkdir -p ${SRCDIR}/${rsltdir}"
testdir=${didir}_${comp}
scp -q -P ${PORT} ${testdir}/*.out ${USER}@${SRCHOST}:${SRCDIR}/${rsltdir}

if [ $keep = F ]; then
  rm -rf di-[45].*.tar.gz ${testdir} dibldrun.sh
fi

exit 0
