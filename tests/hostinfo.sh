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
echo "host: ${host}"
echo "  type: ${type}"
echo "  ipaddr: ${ipaddr}"
echo "  remuser: ${remuser}"
echo "  rempath: ${remport}"
echo "  complist: ${complist}"

exit 0
