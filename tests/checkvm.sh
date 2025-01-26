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

if [[ $type != vm ]];then
  echo "${host}: type is not vm"
  exit 1
fi

./tests/startvm.sh ${host} F
rc=$?
if [[ $rc -ne 0 ]]; then
  exit 1
fi

grc=0

(ssh ${ipaddr} "echo AAA" 2>&1 ) | grep '^AAA$' > /dev/null
rc=$?
if [[ $rc -ne 0 ]]; then
  echo "${host}: Could not ssh as $USER"
  grc=1
fi

(ssh -l root ${ipaddr} "echo AAA" 2>&1 ) | grep '^AAA$' > /dev/null
rc=$?
if [[ $rc -ne 0 ]]; then
  echo "${host}: Could not ssh as root"
  grc=1
fi

./tests/stopvm.sh ${host}
if [[ $grc -eq 0 ]]; then
  echo "${host}: OK"
fi
exit $grc
