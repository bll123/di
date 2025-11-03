#!/bin/sh
#
# Copyright 2025 Brad Lanam Pleasant Hill CA
#

SRCHOST=192.168.2.5
SRCDIR=s/di
PORT=166
USER=bll
math=-
comp=cc
host=`uname -n`
systype=`uname -s`

if [ $# -gt 0 ]; then
  SRCHOST=$1
fi

case ${systype} in
  CYGWIN*)
    host=${host}_cygwin
    ;;
  MSYS*|MINGW*)
    host=${host}_msys2
    ;;
esac

flag=${1:-R}

localdir=""
if [ x$1 != x -a -d "$1" ]; then
  localdir=$1
  flag=${2:-R}
fi

if [ x$localdir != x ]; then
  cp $localdir/di-*.tar.gz .
  cp $localdir/dibldrun.sh .
else
  scp -P ${PORT} ${USER}@${SRCHOST}:${SRCDIR}/di-*.tar.gz \
      ${USER}@${SRCHOST}:${SRCDIR}/tests/dibldrun.sh .
fi

if [ $flag != C ]; then
  chmod a+rx dibldrun.sh
  tarfn=`echo di-*.tar.gz`
  didir=`echo ${tarfn} | sed 's,\.tar.gz$,,'`
  ./dibldrun.sh ${host} ${tarfn} ${didir} ${math} ${comp}
fi

rsltdir=test_results/${host}_${comp}
testdir=${didir}_${comp}

if [ x$localdir != x ]; then
  ldir=${localdir}/${rsltdir}
  mkdir -p ${ldir}
  cp -f ${testdir}/*.out ${ldir}
else
  ssh -p ${PORT} -l ${USER} ${SRCHOST} \
      "test -d ${SRCDIR}/${rsltdir} || mkdir -p ${SRCDIR}/${rsltdir}"
  scp -q -P ${PORT} ${testdir}/*.out ${USER}@${SRCHOST}:${SRCDIR}/${rsltdir}
fi

if [ $flag = R -o $flag = C ]; then
  rm -rf di-[4-9].*.tar.gz di-[4-9].*.tar ${testdir} dibldrun.sh
fi

exit 0
