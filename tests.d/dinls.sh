#!/bin/sh
#
#  Copyright 2010 Brad Lanam Walnut Creek, CA USA
#

if [ "$1" = "-d" ]; then
  echo ${EN} " di nls${EC}"
  exit 0
fi

grep '^#define _enable_nls 1' ${_MKCONFIG_RUNTOPDIR}/config.h
hasnls=$?

if [ ${hasnls} -ne 0 ];then
  echo ${EN} " skipped${EC}" >&5
  exit 0
fi

DI_LOCALE=${_MKCONFIG_RUNTOPDIR}/test_di/share/locale
export DI_LOCALE

grc=1
for l in "de" "de_DE" "de_DE.utf-8" "de_DE.UTF-8" \
    "de_DE.ISO8859-1" "de_DE.ISO8859-15" ; do
  LC_ALL="${l}" ${_MKCONFIG_RUNTOPDIR}/test_di/bin/di -A | grep Benutzt >/dev/null 2>&1
  rc=$?
  if [ $rc -eq 0 ]; then
    grc=0
    break   # only need to know that one works...
  fi
done

if [ $grc -ne 0 ]; then
  for l in "es" "es_ES" "es_ES.utf-8" "es_ES.UTF-8" \
    "es_ES.ISO8859-1" "es_ES.ISO8859-15" ; do
    LC_ALL="${l}" ${_MKCONFIG_RUNTOPDIR}/test_di/bin/di -A | grep Disponible >/dev/null 2>&1
    rc=$?
    if [ $rc -eq 0 ]; then
      grc=0
      break   # only need to know that one works...
    fi
  done
fi

# cannot depend on german or spanish being installed...
if [ $grc -ne 0 ]; then
  echo ${EN} " de/es not installed?${EC}" >&5
fi

exit 0
