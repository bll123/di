#!/bin/bash
#
# Copyright 2025 Brad Lanam Pleasant Hill CA
#

function checkvm {
  vmhost=$1

  ps -ef | grep "[c]omment ${vmhost}" > /dev/null 2>&1
  trc=$?
  return $trc
}

host=$1

if [[ x$host == x ]]; then
  echo "host must be specified."
  exit 1
fi

. ./tests/util.sh

gethostdata ${host}

if [[ $type != vm && $type != vmlocal ]]; then
  echo "${host}: not a vm"
  exit 1
fi

checkvm ${host}
rc=$?
if [[ $rc -ne 0 ]]; then
  echo "${host}: already stopped"
  exit 1
fi

if [[ $ipaddr == "-" ]]; then
  gethostip ${host}
fi

if [[ x${ipaddr} != x && ${ipaddr} != - && ${ipaddr} != 192.168.2.x ]]; then
  cmd="shutdown -h now"
  case ${host} in
    *[sS]olaris*)
      cmd="init 5"
      ;;
    dragonflybsd*|freebsd*|netbsd*|openbsd*|mirbsd*)
      cmd="/sbin/halt -p"
      ;;
    alpine*)
      cmd="/sbin/poweroff"
      ;;
  esac

  ssh -l root ${ipaddr} "${cmd}"
else
  VBoxManage controlvm ${host} acpipowerbutton > /dev/null 2>&1
fi

count=0
while : ; do
  checkvm ${host}
  rc=$?
  if [[ $rc -ne 0 ]]; then
    sleep 2   # give some time for virtualbox to reset
    break
  fi
  count=$(($count+1))
  if [[ $count -gt 40 ]]; then
    break
  fi
  sleep 1
done

checkvm ${host}
rc=$?
if [[ $rc -eq 0 ]]; then
  VBoxManage controlvm ${host} poweroff > /dev/null 2>&1
fi

exit 0
