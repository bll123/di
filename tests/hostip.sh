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

echo $ipaddr

exit 0
