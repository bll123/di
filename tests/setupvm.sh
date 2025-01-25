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

echo -n "Start? "
read answer
case $answer in
  y*|Y*)
    ./tests/startvm.sh ${host} ${ipaddr} F
    rc=$?
    if [[ $rc -ne 0 ]]; then
      exit 1
    fi
    ;;
esac
scp $HOME/.ssh/authorized_keys ${ipaddr}:.ssh
ssh -l root ${ipaddr} "test -d .ssh || mkdir .ssh"
scp $HOME/.ssh/authorized_keys root@${ipaddr}:.ssh
./tests/stopvm.sh ${host} ${ipaddr}
exit 0
