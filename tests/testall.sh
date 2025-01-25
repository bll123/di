#!/bin/bash

HOSTLIST=tests/hostlist.txt

hostlist=""

keep=F
bg=T
while test $# -gt 0; do
  case $1 in
    --keep)
      keep=T
      shift
      ;;
    --foreground)
      bg=F
      shift
      ;;
    *)
      hostlist="$1 "
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
      *)
        set ${line}
        hostlist+="$1 "
        ;;
    esac
  done < ${HOSTLIST}
fi

make distclean
rm -f *.tar.gz
make tar
tarfn=$(echo di-*.tar.gz)
didir=$(echo ${tarfn} | sed 's,\.tar.gz$,,')

for host in ${hostlist}; do
  hdata=$(grep "^${host} " ${HOSTLIST})
  rc=$?
  if [[ $rc -ne 0 ]]; then
    continue
  fi
  set ${hdata}
  type=$2
  ipaddr=$3

  rsltdir=$(pwd)/test_results/${host}
  test -d ${rsltdir} && rm -rf ${rsltdir}
  mkdir -p ${rsltdir}

  if [[ $bg == T ]]; then
    nohup ./tests/thost.sh ${tarfn} ${didir} ${host} ${type} ${ipaddr} ${keep} \
        2>&1 | tee ${rsltdir}/w
  else
    ./tests/thost.sh ${tarfn} ${didir} ${host} ${type} ${ipaddr} ${keep}  \
        2>&1 | tee ${rsltdir}/w
  fi
done

exit 0
