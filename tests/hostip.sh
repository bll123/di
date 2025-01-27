#!/bin/bash
#
# Copyright 2025 Brad Lanam Pleasant Hill CA
#

HOSTLIST=tests/hostlist.txt

host=$1

if [[ x$host == x ]]; then
  echo "host must be specified."
  exit 1
fi

. ./tests/util.sh

gethostdata ${host}
if [[ ${remuser} != - ]]; then
  remussh="-l ${remuser}"
fi
if [[ ${remport} != - ]]; then
  rempssh="-p ${remport}"
fi

echo ${rempssh} ${remussh} $ipaddr

exit 0
