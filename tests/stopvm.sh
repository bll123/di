#!/bin/bash
#
# Copyright 2025 Brad Lanam Pleasant Hill CA
#

host=$1

if [[ x$host == x ]]; then
  echo "host must be specified."
  exit 1
fi

. ./tests/util.sh

gethostdata ${host}

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
