#!/bin/bash

tarfn=$1
didir=$2
host=$3
type=$4
ipaddr=$5
keep=$6

LOCKA=test_results/VM_A
LOCKB=test_results/VM_B
LOCKC=test_results/VM_C
locklist="${LOCKA} ${LOCKB}"

rsltdir=$(pwd)/test_results/${host}

function remotebldrun  {
  scp -q ${tarfn} tests/dibldrun.sh ${ipaddr}:
  ssh ${ipaddr} "chmod a+rx dibldrun.sh"
  ssh ${ipaddr} "./dibldrun.sh ${host} ${tarfn} ${didir} ${quiet}"
  scp -q ${ipaddr}:${didir}/*.out \
    ${ipaddr}:${didir}/build/CMakeFiles/CMakeOutput.log \
    ${ipaddr}:${didir}/build/CMakeFiles/CMakeError.log \
    ${ipaddr}:${didir}/mkc_files/mkc_compile.log \
    ${ipaddr}:${didir}/mkc_files/mkconfig.log \
    ${rsltdir}
  if [ $keep = F ]; then
    ssh ${ipaddr} "rm -rf di-[45].*.tar.gz ${didir} dibldrun.sh"
  fi
}

if [[ ${type} == local ]]; then
  test -d tmp || mkdir tmp
  cp -f ${tarfn} tests/dibldrun.sh tmp
  cd tmp
  chmod a+rx dibldrun.sh
  ./dibldrun.sh ${host} ${tarfn} ${didir} ${quiet}
  cp -f ${didir}/*.out \
    ${didir}/build/CMakeFiles/CMakeOutput.log \
    ${didir}/build/CMakeFiles/CMakeError.log \
    ${didir}/mkc_files/mkc_compile.log \
    ${didir}/mkc_files/mkconfig.log \
    ${rsltdir}
  if [ $keep = F ]; then
    rm -rf di-[45].*.tar.gz ${didir} dibldrun.sh
  fi
elif [[ ${type} = remote ]]; then
  remotebldrun $ipaddr
elif [[ ${type} == vm ]]; then
  echo "-- $(date '+%T') ${host}: waiting for vm lock"
  lockpid=F
  lockfn=F
  while : ; do
    for fn in ${locklist}; do
      lockfile-create ${fn} -r 2
      rc=$?
      if [[ $rc -eq 0 ]]; then
        lockfile-touch ${fn} &
        lockpid=$!
        lockfn=$fn
        break
      fi
    done
    if [[ $lockfn != F ]]; then
      break
    fi
  done
  echo "-- $(date '+%T') ${host}: starting vm"
  ./tests/startvm.sh ${host} ${ipaddr} T
  rc=$?
  if [[ $rc -ne 0 ]]; then
    exit 1
  fi
  remotebldrun $ipaddr
  echo "-- $(date '+%T') ${host}: stopping"
  ./tests/stopvm.sh ${host} ${ipaddr}
  kill ${lockpid}
  lockfile-remove ${lockfn}
else
  echo "-- $(date '+%T') ${host}: unknown type ${type}"
fi

exit 0
