#!/bin/bash
#
# Copyright 2025 Brad Lanam Pleasant Hill CA
#

HOSTLIST=tests/hostlist.txt

function gethostdata {
  host=$1

  hdata=$(grep "^${host} " tests/hostlist.txt)
  trc=$?
  if [[ $trc -ne 0 ]]; then
    echo "${host}: not in hostlist.txt"
    exit 1
  fi
  set ${hdata}
  type=$2
  ipaddr=$3
  remuser=$4
  remport=$5
  rempath=$6
  complist=cc
  shift; shift; shift; shift; shift; shift
  while test $# -gt 0; do
    case $1 in
      \#*)
        break
        ;;
      cc|xlc|gcc|clang|c++)
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

