#!/bin/bash
#
# Copyright 2025 Brad Lanam Pleasant Hill CA
#

. ./tests/util.sh

function countvm {
  vmcount=0
  for vmidx in $vmidxlist; do
    pid=${vmpidlist[$vmidx]}
    if [[ $pid -ne 0 ]]; then
      kill -0 $pid 2>/dev/null
      rc=$?
      if [[ $rc -eq 0 ]]; then
        vmcount=$(($vmcount+1))
      else
        vmpidlist[$vmidx]=0
      fi
    fi
  done
}

function dovm {
  vmhost=$1

  vmpid=0
  if [[ $bg == T ]]; then
    ./tests/testvm.sh ${vmhost} ${flag} &
    vmpid=$!
  else
    ./tests/testvm.sh ${vmhost} ${flag}
  fi
  if [[ $vmpid -ne 0 ]]; then
    for vmidx in $vmidxlist; do
      if [[ ${vmpidlist[$vmidx]} -eq 0 ]]; then
        vmpidlist[$vmidx]=$vmpid
        break
      fi
    done
  fi
}

hostlist=""
vmcount=0
declare -a vmpidlist
vmmax=2
vmidx=0
vmidxlist=""
while test $vmidx -lt $vmmax; do
  vmpidlist[$vmidx]=0
  vmidxlist+="$vmidx "
  vmidx=$(($vmidx+1))
done

flag=R
bg=T
newtar=F
procvm=T
procnotvm=T
while test $# -gt 0; do
  case $1 in
    --keep)
      flag=K
      shift
      ;;
    --clean)
      flag=C
      bg=F
      shift
      ;;
    --list)
      flag=L
      bg=F
      shift
      ;;
    --notvm)
      procvm=F
      shift
      ;;
    --vm)
      procnotvm=F
      shift
      ;;
    --newtar)
      newtar=T
      shift
      ;;
    --fg)
      bg=F
      shift
      ;;
    *)
      hostlist+="$1 "
      shift
      ;;
  esac
done

if [[ "x$hostlist" == x ]]; then
  while read line ; do
    case ${line} in
      \#*)
        ;;
      "")
        ;;
      noauto)
        if [[ $flag != L ]]; then
          break
        fi
        ;;
      *)
        set ${line}
        hostlist+="$1 "
        ;;
    esac
  done < ${HOSTLIST}
fi

if [[ $flag == L ]]; then
  echo $hostlist |
      sed 's, ,\n,g' |
      sort
  exit 0
fi

if [[ $newtar == T ]]; then
  make distclean
  rm -f *.tar.gz
  make tar
fi
tarfn=$(echo di-*.tar.gz)
didir=$(echo ${tarfn} | sed 's,\.tar.gz$,,')

if [[ ${procnotvm} == T ]]; then
  # start all the non-vms
  for host in ${hostlist}; do
    gethostdata ${host}

    if [[ ${type} == vm ]]; then
      continue
    fi

    rsltdir=$(pwd)/test_results/${host}
    test -d ${rsltdir} && rm -rf ${rsltdir}
    mkdir -p ${rsltdir}

    if [[ $bg == T ]]; then
      nohup ./tests/thost.sh ${tarfn} ${didir} ${host} ${type} \
          ${ipaddr} ${remuser} ${remport} ${rempath} \
          ${flag} ${complist} 2>&1 | tee ${rsltdir}/w &
    else
      ./tests/thost.sh ${tarfn} ${didir} ${host} ${type} \
          ${ipaddr} ${remuser} ${remport} ${rempath} \
          ${flag} ${complist} 2>&1 | tee ${rsltdir}/w
    fi
  done
fi

if [[ ${procvm} == T ]]; then
  # do the vms
  for host in ${hostlist}; do
    gethostdata ${host}
    already=F

    if [[ ${type} != vm ]]; then
      continue
    fi

    countvm
    while test $vmcount -ge $vmmax; do
      sleep 1
      countvm
    done

    dovm $host
  done
fi

exit 0
