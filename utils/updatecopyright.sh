#!/bin/bash
#
# Copyright 2025 Brad Lanam Pleasant Hill CA
# Copyright 2021-2025 Brad Lanam Pleasant Hill CA
#

curryear=$(date '+%Y')

# update the list in depcheck.sh also
for fn in *.c *.h */*.sh CMakeLists.txt Makefile config.h.in; do
  case $fn in
    *tt.sh|*z.sh)
      continue
      ;;
    build/*)
      continue
      ;;
    dev/*)
      continue
      ;;
    utils/*.sh)
      continue
      ;;
    mkconfig*)
      continue
      ;;
  esac

  grep "Copyright.*${curryear}" $fn > /dev/null 2>&1
  rc=$?
  if [[ $rc -ne 0 ]]; then
    echo "$fn"
    grep "Copyright.*-" $fn > /dev/null 2>&1
    hasdash=$?
    if [[ $hasdash -eq 0 ]]; then
      sed -i -e "s,Copyright \([0-9]*\)-[0-9]* ,Copyright \1-${curryear} ," $fn
    else
      sed -i -e "s,Copyright \([0-9]*\) ,Copyright \1-${curryear} ," $fn
    fi
  fi
done

exit 0
