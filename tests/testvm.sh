#!/bin/bash
#
# Copyright 2025 Brad Lanam Pleasant Hill CA
#

. ./tests/util.sh

host=$1
flag=${2:-R}

gethostdata ${host}

if [[ ${type} != vm && $type != vmlocal ]]; then
  echo "-- $(TZ=PST8PDT date '+%T') ${host}: not a vm"
  exit 1
fi

if [[ $ipaddr == 192.168.2.x ]]; then
  echo "-- $(TZ=PST8PDT date '+%T') ${host}: IP address not set"
  exit 1
fi

already=F
./tests/startvm.sh ${host} T
rc=$?
if [[ $rc -ne 0 && $rc -ne 255 ]]; then
  echo "-- $(TZ=PST8PDT date '+%T') ${host}: unable to start"
  exit 1
fi
if [[ $rc -eq 255 ]]; then
  echo "-- $(TZ=PST8PDT date '+%T') ${host}: already started"
  already=T
fi

if [[ $ipaddr == "-" ]]; then
  gethostip ${host}
fi
if [[ x${ipaddr} == x || ${ipaddr} == - ]]; then
  echo "${host}: Unable to get host ip"
  ./tests/stopvm.sh ${host}
  exit 1
fi

if [[ $flag != "CHK" ]]; then
  tarfn=$(echo di-*.tar.gz)
  didir=$(echo ${tarfn} | sed 's,\.tar.gz$,,')

  rsltdir=$(pwd)/test_results/${host}
  test -d ${rsltdir} && rm -rf ${rsltdir}
  mkdir -p ${rsltdir}

  ./tests/thost.sh ${tarfn} ${didir} ${host} ${type} \
      ${ipaddr} ${remuser} ${remport} ${rempath} \
      ${flag} ${complist} 2>&1 | tee ${rsltdir}/w
fi

if [[ $already == F ]]; then
  echo "-- $(TZ=PST8PDT date '+%T') ${host}: stopping"
  ./tests/stopvm.sh ${host}
fi

exit 0
