#!/bin/sh

echo ${EN} "di nls${EC}" >&3

../hasnls
hasnls=$?

if [ ${hasnls} -ne 0 ];then
  echo ${EN} " skipped${EC}" >&3
  exit 0
fi

grc=1
for l in "de" "de_DE" "de_DE.utf-8" "de_DE.UTF-8" \
    "de_DE.ISO8859-1" "de_DE.ISO8859-15"; do
  LC_ALL="${l}" ../test_di/bin/di -A | egrep Benutzt >/dev/null 2>&1
  rc=$?
  if [ $rc -eq 0 ]; then
    grc=0
  fi
done

# cannot depend on german being installed...
if [ $grc -ne 0 ]; then
  echo ${EN} " de not installed?${EC}" >&3
fi
#exit $grc
exit 0
