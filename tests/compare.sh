#!/bin/bash
#
# Copyright 2025 Brad Lanam Pleasant Hill CA
#

host=$1
shift
complist=$*
if [[ $complist == "" ]]; then
  complist="cc"
fi

topdir=$(pwd)

# cmake/mkc comparison
for comp in ${complist}; do
  rsltdir=${topdir}/test_results/${host}_${comp}
  if [[ -f ${rsltdir}/di-cmake-config.out && \
      -f ${rsltdir}/di-mkc-config.out ]]; then
    diff -b -B ${rsltdir}/di-mkc-config.out ${rsltdir}/di-cmake-config.out \
        > ${rsltdir}/di-tmpdiff.out
    awk -f ./tests/chkdiff.awk ${rsltdir}/di-tmpdiff.out \
        > ${rsltdir}/di-diff.out
    dlc=$(cat ${rsltdir}/di-diff.out | wc -l)
    if [[ $dlc != 0 ]]; then
      echo "== $(date '+%T') ${host}/${comp}: config.h diff failed"
    fi

    diff -q -b -B ${rsltdir}/di-mkc-math.out ${rsltdir}/di-cmake-math.out
    rc=$?
    if [[ $rc != 0 ]]; then
      echo "== $(date '+%T') ${host}/${comp}: dimathtest diff failed"
    fi

    diff -q -b -B ${rsltdir}/di-mkc-instdir.out ${rsltdir}/di-cmake-instdir.out
    rc=$?
    if [[ $rc != 0 ]]; then
      echo "== $(date '+%T') ${host}/${comp}: installation dir diff failed"
    fi
  fi
done

# different compilers should end up with the same data
rsltdir=""
for comp in ${complist}; do
  if [[ x$rsltdir != x ]]; then
    rsltdirb=${topdir}/test_results/${host}_${comp}
    # do mkc first, always there
    for bld in mkc cmake; do
      if [[ ! -f ${rsltdir}/di-${bld}-config.out ||
          ! -f ${rsltdirb}/di-${bld}-config.out ]]; then
        break
      fi
      diff -b -B ${rsltdir}/di-${bld}-config.out ${rsltdirb}/di-${bld}-config.out \
          > ${rsltdir}/di-tmp${bld}diff.out
      awk -f ./tests/chkblddiff.awk ${rsltdir}/di-tmp${bld}diff.out \
          > ${rsltdir}/di-${bld}diff.out
      dlc=$(cat ${rsltdir}/di-${bld}diff.out | wc -l)
      if [[ $dlc != 0 ]]; then
        echo "== $(date '+%T') ${host}: ${bld} config.h diff failed"
      fi

      diff -q -b -B ${rsltdir}/di-mkc-instdir.out ${rsltdir}/di-cmake-instdir.out
      rc=$?
      if [[ $rc != 0 ]]; then
        echo "== $(date '+%T') ${host}/${comp}: installation dir diff failed"
      fi
    done
  fi

  rsltdir=${topdir}/test_results/${host}_${comp}
done
