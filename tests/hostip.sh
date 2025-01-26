#!/bin/bash

HOSTLIST=tests/hostlist.txt

host=$1

if [[ x$host == x ]]; then
  echo "host must be specified."
  exit 1
fi

hdata=$(grep "^${host} " ${HOSTLIST})
set ${hdata}
type=$2
ipaddr=$3

echo $ipaddr

exit 0
