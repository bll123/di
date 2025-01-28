#!/bin/bash
#
# Copyright 2025 Brad Lanam Pleasant Hill CA
#

. ./tests/util.sh

hostlist=""

flag=R
bg=T
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

make distclean
rm -f *.tar.gz
make tar
tarfn=$(echo di-*.tar.gz)
didir=$(echo ${tarfn} | sed 's,\.tar.gz$,,')

for host in ${hostlist}; do
  gethostdata ${host}

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

exit 0
