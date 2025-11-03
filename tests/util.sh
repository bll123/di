#!/bin/bash
#
# Copyright 2025 Brad Lanam Pleasant Hill CA
#

HOSTLIST=tests/hostlist.txt

function gethostip {
  host=$1

  count=0
  ipaddr="-"
  while : ; do
    # Value: 192.168.56.3
    # No value set!
    ipdata=$(VBoxManage guestproperty get ${host} '/VirtualBox/GuestInfo/Net/0/V4/IP' 2>/dev/null)
    ipaddr=$(echo $ipdata | sed -e 's,[^0-9]*,,')
    if [[ "x$ipaddr" != x ]]; then
      break
    fi
    count=$(($count + 1))
    sleep 1
    if [[ $count -gt 80 ]]; then
      break
    fi
  done
}

function gethostdata {
  host=$1

  hdata=$(grep "^${host} " tests/hostlist.txt)
  trc=$?
  if [[ $trc -ne 0 ]]; then
    echo "${host}: not in hostlist.txt"
    return 1
  fi
  set ${hdata}
  type=$2
  ipaddr=$3
  remuser=$4
  remport=$5
  rempath=$6
  remmath=$7
  complist=cc
  shift; shift; shift; shift; shift; shift; shift
  while test $# -gt 0; do
    case $1 in
      \#*)
        break
        ;;
      cc|xlc|gcc*|clang*|c++)
        if [[ $complist == cc ]]; then
          complist=""
        fi
        complist+="$1 "
        shift
        ;;
      *)
        shift
        ;;
    esac
  done
}
