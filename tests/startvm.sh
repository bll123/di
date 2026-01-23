#!/bin/bash
#
# Copyright 2025-2026 Brad Lanam Pleasant Hill CA
#

host=$1
validate=${2:-T}

dodisp=F
case $1 in
  --display|--disp)
    dodisp=T
    shift
    ;;
esac

if [[ x$host == x ]]; then
  echo "host must be specified."
  exit 1
fi

. ./tests/util.sh

gethostdata ${host}

if [[ ${ipaddr} != "" && ${ipaddr} != - ]]; then
  chkc=$(ps -ef | grep "[c]omment ${host}" | wc -l)
  if [[ $chkc -eq 1 ]]; then
    (ssh -l bll ${ipaddr} "echo AAA" 2>&1 ) | grep '^AAA$' > /dev/null
    rc=$?
    if [[ $rc -eq 0 ]]; then
      echo "-- $(TZ=PST8PDT date '+%T') ${host}: connected"
      # already running
      exit 255
    fi
  fi
fi

echo "-- $(TZ=PST8PDT date '+%T') ${host}: starting vm"
args=""
if [[ $dodisp == F ]]; then
  args="--type=headless"
fi
nohup VBoxManage startvm ${host} ${args} > /dev/null 2>&1 &

# give time for vboxmanage to work and the host to start...
if [[ $validate == T ]]; then
  sleep 10
else
  sleep 20
fi

count=0
ok=F
if [[ $validate == T ]]; then
  if [[ ${ipaddr} == "-" ]]; then
    gethostip ${host}
    if [[ x${ipaddr} == x || ${ipaddr} == - || ${ipaddr} == 192.168.2.x ]]; then
      echo "${host}: Unable to get host ip"
      ./tests/stopvm.sh ${host}
      exit 1
    fi
  fi
  while : ; do
    (ssh -l bll ${ipaddr} "echo AAA" 2>&1 ) | grep '^AAA$' > /dev/null
    rc=$?
    if [[ $rc -eq 0 ]]; then
      echo "-- $(TZ=PST8PDT date '+%T') ${host}: connected"
      ok=T
      break
    fi
    count=$(($count + 1))
    if [[ $count -gt 40 ]]; then
      echo "== $(TZ=PST8PDT date '+%T') ${host}: unable to connect"
      ./tests/stopvm.sh ${host}
      exit 1
    fi
    sleep 3
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
