#!/bin/sh
#
#  Copyright 2010 Brad Lanam Walnut Creek, CA USA
#

echo ${EN} "di nls${EC}" >&3

$RUNTOPDIR/features/hasnls.sh $RUNTOPDIR/config.h
hasnls=$?

if [ ${hasnls} -ne 0 ];then
  echo ${EN} " skipped${EC}" >&3
  exit 0
fi

grc=1
for l in "de" "de_DE" "de_DE.utf-8" "de_DE.UTF-8" \
    "de_DE.ISO8859-1" "de_DE.ISO8859-15"; do
  LC_ALL="${l}" $RUNTOPDIR/test_di/bin/di -A | grep Benutzt >/dev/null 2>&1
  rc=$?
  if [ $rc -eq 0 ]; then
    grc=0
  fi
done

# cannot depend on german being installed...
if [ $grc -ne 0 ]; then
  echo ${EN} " de not installed?${EC}" >&3
fi

exit 0
