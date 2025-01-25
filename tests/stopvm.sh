#!/bin/bash

vm=$1
ipaddr=$2

cmd="shutdown -h now"
case ${vm} in
  *solaris*|nexenta*)
    cmd="init 5"
    ;;
  dragonflybsd*|freebsd*|netbsd*|openbsd*|mirbsd*)
    cmd="/sbin/halt -p"
    ;;
esac

ssh -l root ${ipaddr} "${cmd}" > /dev/null 2>&1
# VBoxManage controlvm ${vm} poweroff > /dev/null 2>&1
while : ; do
  ps -ef | grep "[c]omment ${vm}" > /dev/null 2>&1
  rc=$?
  if [[ $rc -ne 0 ]]; then
    break;
  fi
  sleep 1
done
exit 0
