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

if [[ $type != vm ]];then
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
ssh ${ipaddr} "test -d .ssh || mkdir .ssh; chmod 700 .ssh"
scp $HOME/.ssh/authorized_keys ${ipaddr}:.ssh
ssh -l root ${ipaddr} "test -d .ssh || mkdir .ssh; chmod 700 .ssh"
scp $HOME/.ssh/authorized_keys root@${ipaddr}:.ssh
if [[ $stop == T ]]; then
  ./tests/stopvm.sh ${host}
fi
exit 0
