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

nohup VBoxManage startvm ${host} > /dev/null 2>&1 &
sleep 10
count=0
ok=F
if [[ $validate == T ]]; then
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
      echo "-- $(date '+%T') ${host}: stopping"
      ./tests/stopvm.sh ${host} ${ipaddr} &
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
