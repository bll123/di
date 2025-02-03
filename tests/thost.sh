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
shift
remuser=$1
remport=$2
rempath=$3
shift;shift;shift
# flag is R (build and remove), K (build and keep), C (clean only)
flag=${1:=R}
shift
complist=$*

LOCKA=tmp/VM_A
LOCKB=tmp/VM_B
LOCKC=tmp/VM_C
locklist="${LOCKA} ${LOCKB}"

function remotebldrun  {
  ssh ${rempssh} ${remussh} ${ipaddr} "./dibldrun.sh ${host} ${tarfn} ${didir} ${comp} ${rempath}"
  testdir=${didir}_${comp}
  scp ${rempscp} -q ${remuscp}${ipaddr}:${testdir}/*.out ${rsltdir}
}

if [[ ${remuser} != - ]]; then
  remussh="-l ${remuser}"
  remuscp="${remuser}@"
fi
if [[ ${remport} != - ]]; then
  rempssh="-p ${remport}"
  rempscp="-P ${remport}"
fi

topdir=$(pwd)
already=F

if [[ $flag != C ]]; then
  if [[ ${type} == local ]]; then
    test -d tmp || mkdir tmp
    cp -f ${tarfn} tests/dibldrun.sh tmp
    cd tmp
    chmod a+rx dibldrun.sh
    cd ..
  fi

  for comp in ${complist}; do
    echo "-- $(date '+%T') ${host}: start ${comp}"
    rsltdir=${topdir}/test_results/${host}_${comp}
    test -d ${rsltdir} && rm -rf ${rsltdir}
    mkdir -p ${rsltdir}

    if [[ ${type} == local ]]; then
      cd tmp
      ./dibldrun.sh ${host} ${tarfn} ${didir} ${comp} ${rempath}
      testdir=${didir}_${comp}
      cp -f ${testdir}/*.out ${rsltdir}
      cd ..
    elif [[ ${type} = remote ]]; then
      echo "-- $(date '+%T') ${host}: copying files"
      scp ${rempscp} -q ${tarfn} tests/dibldrun.sh ${remuscp}${ipaddr}:
      ssh ${rempssh} ${remussh} ${ipaddr} "chmod a+rx dibldrun.sh"
      remotebldrun $ipaddr
    elif [[ ${type} == vm ]]; then
      echo "-- $(date '+%T') ${host}: waiting for vm lock"
      lockpid=F
      lockfn=F
      while : ; do
        for fn in ${locklist}; do
          lockfile-create -r 1 ${fn}
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
      if [[ $rc -ne 0 && $rc -ne 255 ]]; then
        exit 1
      fi
      if [[ $rc -eq 255 ]]; then
        already=T
      fi
      echo "-- $(date '+%T') ${host}: copying files"
      scp ${rempscp} -q ${tarfn} tests/dibldrun.sh ${remuscp}${ipaddr}:
      ssh ${rempssh} ${remussh} ${ipaddr} "chmod a+rx dibldrun.sh"
      remotebldrun $ipaddr
    else
      echo "-- $(date '+%T') ${host}: unknown type ${type}"
    fi
  done
fi

if [[ $flag == R || $flag == C ]]; then
  for comp in ${complist}; do
    if [[ $type == local ]]; then
      cd tmp
      rm -rf di-[45].*.tar.gz di-[45].*.tar di-[45].*_${comp} dibldrun.sh
      cd ..
    fi
    if [[ $type == remote || $type == vm ]]; then
      ssh ${rempssh} ${remussh} ${ipaddr} "rm -rf di-[45].*.tar.gz di-[45].*.tar di-[45].*_${comp} dibldrun.sh"
    fi
  done
fi

if [[ $flag != C && $type == vm ]]; then
  if [[ $already == F ]]; then
    echo "-- $(date '+%T') ${host}: stopping"
    ./tests/stopvm.sh ${host}
  fi
  if [[ $lockpid != F ]]; then
    kill ${lockpid}
  fi
  if [[ $lockfn != F ]]; then
    lockfile-remove ${lockfn}
  fi
fi

./tests/check.sh ${host} ${complist}

echo "-- $(date '+%T') ${host}: finish"

exit 0
