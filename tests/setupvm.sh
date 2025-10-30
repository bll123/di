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

gethostdata $host
if [[ $type != vm && $type != vmlocal ]];then
  echo "${host}: type is not vm"
  exit 1
fi

stop=F
echo -n "Start? "
read answer
case $answer in
  y*|Y*)
    stop=T
    ./tests/startvm.sh ${host} F
    rc=$?
    if [[ $rc -ne 0 ]]; then
      exit 1
    fi
    ;;
esac

if [[ x${ipaddr} == x || ${ipaddr} == - || ${ipaddr} == 192.168.2.x ]]; then
  echo "${host}: no host ip"
  exit 1
fi

echo "-- create .ssh dir"
ssh ${ipaddr} "test -d .ssh || mkdir .ssh; chmod 700 .ssh"
echo "-- copy auth-keys"
scp $HOME/.ssh/authorized_keys ${ipaddr}:.ssh
echo "-- create root .ssh dir"
ssh -l root ${ipaddr} "test -d .ssh || mkdir .ssh; chmod 700 .ssh"
echo "-- copy auth-keys for root"
scp $HOME/.ssh/authorized_keys root@${ipaddr}:.ssh
if [[ $stop == T ]]; then
  ./tests/stopvm.sh ${host}
fi
exit 0
