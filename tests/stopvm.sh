#!/bin/bash

host=$1

if [[ x$host == x ]]; then
  echo "host must be specified."
  exit 1
fi

hdata=$(grep "^${host} " tests/hostlist.txt)
rc=$?
if [[ $rc -ne 0 ]]; then
  echo "${host}: not in hostlist.txt"
  exit 1
fi
set ${hdata}
type=$2
ipaddr=$3

if [[ $type != vm ]]; then
  echo "${host}: not a vm"
  exit 1
fi

cmd="shutdown -h now"
case ${host} in
  *solaris*|nexenta*)
    cmd="init 5"
    ;;
  dragonflybsd*|freebsd*|netbsd*|openbsd*|mirbsd*)
    cmd="/sbin/halt -p"
    ;;
esac

ssh -l root ${ipaddr} "${cmd}" > /dev/null 2>&1
# VBoxManage controlvm ${host} poweroff > /dev/null 2>&1
while : ; do
  ps -ef | grep "[c]omment ${host}" > /dev/null 2>&1
  rc=$?
  if [[ $rc -ne 0 ]]; then
    break;
  fi
  sleep 1
done
exit 0
