#!/bin/sh

. $_MKCONFIG_DIR/bin/testfuncs.sh

maindodisplay $1 'di nls'
maindoquery $1 $_MKC_ONCE

getsname $0
dosetup $@

unset DI_ARGS
unset DIFMT

set -x
for d in C D; do
  if [ $d = D -a \( "$DC" = "" -o "$DC" = "skip" \) ]; then
    continue
  fi
  tdir=$_MKCONFIG_RUNTOPDIR/$d
  (
    cd $tdir
    if [ $? -eq 0 ]; then
      instdir="`pwd`/test_di"
      ${MAKE:-make} ${TMAKEFLAGS} -e prefix=${instdir} all
        > ${_MKCONFIG_TSTRUNTMPDIR}/make.log 2>&1
      ${MAKE:-make} ${TMAKEFLAGS} -e prefix=${instdir} install
        >> ${_MKCONFIG_TSTRUNTMPDIR}/make.log 2>&1
    fi
  )
  if [ -x ${tdir}/di ]; then
    echo ${EN} " ${d}${EC}" >&5
    if [ $d = C ]; then
      grep '^#define _enable_nls 1' ${tdir}/config.h
      hasnls=$?
    fi
    if [ $d = D ]; then
      grep '^enum int _enable_nls = 1;' ${tdir}/config.d
      hasnls=$?
    fi

    if [ ${hasnls} -ne 0 ];then
      echo ${EN} " skipped${EC}" >&5
      exit 0
    fi

    testdir=${tdir}/test_di
    DI_LOCALE_DIR=${testdir}/share/locale
    export DI_LOCALE_DIR

    grc=1
    for l in "de" "de_DE" "de_DE.utf-8" "de_DE.UTF-8" \
        "de_DE.ISO8859-1" "de_DE.ISO8859-15" ; do
      LC_ALL="${l}" ${testdir}/bin/di -A | grep Benutzt >/dev/null 2>&1
      rc=$?
      if [ $rc -eq 0 ]; then
        grc=0
        break   # only need to know that one works...
      fi
    done

    if [ $grc -ne 0 ]; then
      for l in "es" "es_ES" "es_ES.utf-8" "es_ES.UTF-8" \
        "es_ES.ISO8859-1" "es_ES.ISO8859-15" ; do
        LC_ALL="${l}" ${testdir}/bin/di -A | grep Disponible >/dev/null 2>&1
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
      grc=0
    fi
  else
    if [ $d = C ]; then
      echo "## no di executable found for dir $d"
      grc=1
    fi
  fi
done

exit $grc
