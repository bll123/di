#!/bin/bash
#
# Copyright 2025 Brad Lanam Pleasant Hill CA
#

tarfn=$1
didir=$2
shift;shift
host=$1
type=$2
shift;shift
ipaddr=$1
keep=$2
shift;shift
complist=$*

LOCKA=test_results/VM_A
LOCKB=test_results/VM_B
LOCKC=test_results/VM_C
locklist="${LOCKA} ${LOCKB}"

function remotebldrun  {
  ssh ${ipaddr} "./dibldrun.sh ${host} ${tarfn} ${didir} ${comp}"
  testdir=${didir}_${comp}
  scp -q ${ipaddr}:${testdir}/*.out ${rsltdir}
}

topdir=$(pwd)
if [[ ${type} == local ]]; then
  test -d tmp || mkdir tmp
  cp -f ${tarfn} tests/dibldrun.sh tmp
  cd tmp
  chmod a+rx dibldrun.sh
fi
if [[ ${type} == remote || ${type} == vm ]]; then
  scp -q ${tarfn} tests/dibldrun.sh ${ipaddr}:
  ssh ${ipaddr} "chmod a+rx dibldrun.sh"
fi

for comp in ${complist}; do
  rsltdir=${topdir}/test_results/${host}_${comp}
  test -d ${rsltdir} && rm -rf ${rsltdir}
  mkdir -p ${rsltdir}

  if [[ ${type} == local ]]; then
    ./dibldrun.sh ${host} ${tarfn} ${didir} ${comp}
    testdir=${didir}_${comp}
    cp -f ${testdir}/*.out ${rsltdir}
  elif [[ ${type} = remote ]]; then
    remotebldrun $ipaddr
  elif [[ ${type} == vm ]]; then
    echo "-- $(date '+%T') ${host}: waiting for vm lock"
    lockpid=F
    lockfn=F
    while : ; do
      for fn in ${locklist}; do
        lockfile-create ${fn} -r 1
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
    ./tests/startvm.sh ${host} T
    rc=$?
    if [[ $rc -ne 0 ]]; then
      exit 1
    fi
    remotebldrun $ipaddr
    echo "-- $(date '+%T') ${host}: stopping"
    ./tests/stopvm.sh ${host}
    kill ${lockpid}
    lockfile-remove ${lockfn}
  else
    echo "-- $(date '+%T') ${host}: unknown type ${type}"
  fi
done

if [ $keep = F ]; then
  for comp in ${complist}; do
    testdir=${didir}_${comp}
    if [[ $type == local ]]; then
      rm -rf di-[45].*.tar.gz ${testdir} dibldrun.sh
    fi
    if [[ $type == remote || $type == vm ]]; then
      ssh ${ipaddr} "rm -rf di-[45].*.tar.gz ${testdir} dibldrun.sh"
    fi
  done
fi
echo "-- $(date +%T) ${host}: finish"

exit 0
