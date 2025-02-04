#!/bin/bash
#
# Copyright 2025 Brad Lanam Pleasant Hill CA
#

. ./tests/util.sh

function dovm {
  vmhost=$1

  ./tests/startvm.sh ${vmhost} T
  rc=$?
  if [[ $rc -ne 0 && $rc -ne 255 ]]; then
    continue
  fi
  if [[ $rc -eq 255 ]]; then
    already=T
  fi
  vmcount=$(($vmcount+1))

  rsltdir=$(pwd)/test_results/${vmhost}
  test -d ${rsltdir} && rm -rf ${rsltdir}
  mkdir -p ${rsltdir}

  if [[ $bg == T ]]; then
    nohup ./tests/thost.sh ${tarfn} ${didir} ${vmhost} ${type} \
        ${ipaddr} ${remuser} ${remport} ${rempath} \
        ${flag} ${complist} 2>&1 | tee ${rsltdir}/w &
  else
    ./tests/thost.sh ${tarfn} ${didir} ${vmhost} ${type} \
        ${ipaddr} ${remuser} ${remport} ${rempath} \
        ${flag} ${complist} 2>&1 | tee ${rsltdir}/w
  fi

  if [[ $already == F ]]; then
    echo "-- $(date '+%T') ${vmhost}: stopping"
    ./tests/stopvm.sh ${vmhost}
    vmcount=$(($vmcount-1))
  fi
}

hostlist=""
vmcount=0

flag=R
bg=T
newtar=F
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

# do the vms
for host in ${hostlist}; do
  gethostdata ${host}
  already=F

  if [[ ${type} != vm ]]; then
    continue
  fi

  while test $vmcount -ge 2; do
    sleep 1
  done

  dovm $host
done

exit 0
