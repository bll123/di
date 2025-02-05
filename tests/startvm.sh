#!/bin/bash
#
# Copyright 2025 Brad Lanam Pleasant Hill CA
#

host=$1
validate=${2:-T}

if [[ x$host == x ]]; then
  echo "host must be specified."
  exit 1
fi

. ./tests/util.sh

gethostdata ${host}
if [[ ${ipaddr} == "-" ]]; then
  gethostip ${host}
fi
if [[ x${ipaddr} == "-" ]]; then
  echo "${host}: Unable to get ip address"
  exti 1
fi

chkc=$(ps -ef | grep "[c]omment ${host}" | wc -l)
if [[ $chkc -eq 1 ]]; then
  (ssh -l bll ${ipaddr} "echo AAA" 2>&1 ) | grep '^AAA$' > /dev/null
  rc=$?
  if [[ $rc -eq 0 ]]; then
    echo "-- $(date '+%T') ${host}: connected"
    # already running
    exit 255
  fi
fi

echo "-- $(date '+%T') ${host}: starting vm"
nohup VBoxManage startvm ${host} > /dev/null 2>&1 &
sleep 5 # give time for vboxmanage to work...
count=0
ok=F
if [[ $validate == T ]]; then
  if [[ ${ipaddr} == "-" ]]; then
    gethostip ${host}
    if [[ x${ipaddr} == "-" ]]; then
      echo "${host}: Unable to get host ip"
      ./tests/stopvm.sh ${host}
      exit 1
    fi
  fi
  while : ; do
    (ssh -l bll ${ipaddr} "echo AAA" 2>&1 ) | grep '^AAA$' > /dev/null
    rc=$?
    if [[ $rc -eq 0 ]]; then
      echo "-- $(date '+%T') ${host}: connected"
      ok=T
      break
    fi
    count=$(($count + 1))
    if [[ $count -gt 40 ]]; then
      echo "== $(date '+%T') ${host}: unable to connect"
      ./tests/stopvm.sh ${host}
      exit 1
    fi
    sleep 5
  done
else
  ok=T
fi

if [[ $ok == T ]]; then
  rc=0
else
  rc=1
fi
exit $rc
