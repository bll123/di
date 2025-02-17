#!/bin/bash
#
# Copyright 2025 Brad Lanam Pleasant Hill CA
#

host=$1

. ./tests/util.sh

gethostdata ${host}
complist=${complist:-cc}

topdir=$(pwd)
tcount=0
failcount=0
havecmake=""
havemkc="mkc"

ok=0
for comp in cc gcc clang; do
  if [[ -d ${topdir}/test_results/${host}_${comp} ]]; then
    ok=1
    break
  fi
done
if [[ $ok -eq 0 ]]; then
  echo "FAIL ${host}: Unable to locate results dir"
  exit 1
fi

# chkswitcher
for comp in ${complist}; do
  rsltdir=${topdir}/test_results/${host}_${comp}
  tcount=$(($tcount+1))
  tfn=${rsltdir}/di-chkswitcher.out
  if [[ -f ${tfn} ]]; then
    tcount=$(($tcount+1))
    cdata=$(cat ${tfn})
    if [[ $cdata != cmake && $cdata != mkc ]]; then
      echo "FAIL $(date '+%T') ${host}/${comp}: chkswitcher failed"
      cat ${tfn}
      failcount=$(($failcount+1))
    fi
  else
    echo "FAIL $(date '+%T') ${host}/${comp}: chkswitcher does not exist"
    failcount=$(($failcount+1))
  fi
done

for comp in ${complist}; do
  rsltdir=${topdir}/test_results/${host}_${comp}
  tcount=$(($tcount+1))
  if [[ ! -f ${rsltdir}/di-mkc-config.out ]]; then
    echo "FAIL $(date '+%T') ${host}/${comp}: no mkc config"
    failcount=$(($failcount+1))
  fi
done

# cmake/mkc comparison
for comp in ${complist}; do
  rsltdir=${topdir}/test_results/${host}_${comp}
  if [[ -f ${rsltdir}/di-cmake-config.out &&
      -f ${rsltdir}/di-mkc-config.out ]]; then
    tcount=$(($tcount+1))
    diff -b -B ${rsltdir}/di-mkc-config.out ${rsltdir}/di-cmake-config.out \
        > ${rsltdir}/di-tmpdiff.out
    awk -f ./tests/chkdiff.awk ${rsltdir}/di-tmpdiff.out \
        > ${rsltdir}/di-diff.out
    dlc=$(cat ${rsltdir}/di-diff.out | wc -l)
    if [[ $dlc != 0 ]]; then
      echo "FAIL $(date '+%T') ${host}/${comp}: config.h diff failed"
      failcount=$(($failcount+1))
    fi

    tcount=$(($tcount+1))
    diff -b -B ${rsltdir}/di-cmake-config.out ${rsltdir}/di-pcmake-config.out \
        > ${rsltdir}/di-tmppdiff.out
    awk -f ./tests/chkdiff.awk ${rsltdir}/di-tmppdiff.out \
        > ${rsltdir}/di-pdiff.out
    dlc=$(cat ${rsltdir}/di-pdiff.out | wc -l)
    if [[ $dlc != 0 ]]; then
      echo "FAIL $(date '+%T') ${host}/${comp}: config.h pure-cmake diff failed"
      failcount=$(($failcount+1))
    fi

    tcount=$(($tcount+1))
    diff -q -b -B ${rsltdir}/di-mkc-math.out ${rsltdir}/di-cmake-math.out
    rc=$?
    if [[ $rc != 0 ]]; then
      echo "FAIL $(date '+%T') ${host}/${comp}: dimathtest diff failed"
      failcount=$(($failcount+1))
    fi

    tcount=$(($tcount+1))
    diff -q -b -B ${rsltdir}/di-cmake-math.out ${rsltdir}/di-pcmake-math.out
    rc=$?
    if [[ $rc != 0 ]]; then
      echo "FAIL $(date '+%T') ${host}/${comp}: dimathtest pure-cmake diff failed"
      failcount=$(($failcount+1))
    fi

    tcount=$(($tcount+1))
    diff -q -b -B ${rsltdir}/di-mkc-instdir.out ${rsltdir}/di-cmake-instdir.out
    rc=$?
    if [[ $rc != 0 ]]; then
      echo "FAIL $(date '+%T') ${host}/${comp}: installation dir diff failed"
      failcount=$(($failcount+1))
    fi

    tcount=$(($tcount+1))
    diff -q -b -B ${rsltdir}/di-cmake-instdir.out ${rsltdir}/di-pcmake-instdir.out
    rc=$?
    if [[ $rc != 0 ]]; then
      echo "FAIL $(date '+%T') ${host}/${comp}: installation dir pure-cmake diff failed"
      failcount=$(($failcount+1))
    fi

    havecmake=cmake
  fi
done

# different compilers should end up with the same data
rsltdir=""
for comp in ${complist}; do
  if [[ x$rsltdir != x ]]; then
    rsltdirb=${topdir}/test_results/${host}_${comp}

    # do mkc first, always there
    for bld in mkc cmake pcmake; do
      if [[ ! -f ${rsltdir}/di-${bld}-config.out ||
          ! -f ${rsltdirb}/di-${bld}-config.out ]]; then
        break
      fi

      tcount=$(($tcount+1))
      diff -b -B ${rsltdir}/di-${bld}-config.out ${rsltdirb}/di-${bld}-config.out \
          > ${rsltdir}/di-tmp${bld}diff.out
      awk -f ./tests/chkblddiff.awk ${rsltdir}/di-tmp${bld}diff.out \
          > ${rsltdir}/di-${bld}diff.out
      dlc=$(cat ${rsltdir}/di-${bld}diff.out | wc -l)
      if [[ $dlc != 0 ]]; then
        echo "FAIL $(date '+%T') ${host}: ${bld} config.h diff failed"
        failcount=$(($failcount+1))
      fi

      tcount=$(($tcount+1))
      diff -b -B ${rsltdir}/di-${bld}-instdir.out ${rsltdirb}/di-${bld}-instdir.out
      rc=$?
      if [[ $rc != 0 ]]; then
        echo "FAIL $(date '+%T') ${host}/${comp}: installation dir diff failed"
        failcount=$(($failcount+1))
      fi
    done
  fi

  rsltdir=${topdir}/test_results/${host}_${comp}
done

for comp in ${complist}; do
  rsltdir=${topdir}/test_results/${host}_${comp}

  for bld in mkc cmake pcmake; do
    if [[ ! -f ${rsltdir}/di-${bld}-run.out ]]; then
      continue
    fi

    # test that the execution of di actually worked.
    tcount=$(($tcount+1))
    grep -l "^# BUILD: [cm]" ${rsltdir}/di-${bld}-run.out > /dev/null 2>&1
    rc=$?
    if [[ $rc != 0 ]]; then
      echo "FAIL $(date '+%T') ${host}: ${bld}: ${comp}: no debug output in run file"
      failcount=$(($failcount+1))
    fi

    # check the installation dirs
    tcount=$(($tcount+1))
    trc=0
    for d in bin include share pkgconfig man man1 man3; do
      grep -l "^${d}$" ${rsltdir}/di-${bld}-instdir.out > /dev/null 2>&1
      rc=$?
      if [[ $rc -ne 0 ]]; then
        echo "FAIL $(date '+%T') ${host}: ${bld}: ${comp}: missing ${d}"
        trc=1
      fi
    done

    # check the installation files
    tcount=$(($tcount+1))
    trc=0
    for d in locale; do
      grep -l "^${d}$" ${rsltdir}/di-${bld}-instdir.out > /dev/null 2>&1
      rc=$?
      if [[ $rc -ne 0 ]]; then
        echo "WARN $(date '+%T') ${host}: ${bld}: ${comp}: missing ${d}"
      fi
    done

    tcount=$(($tcount+1))
    grep -E -l "^lib(64)?$" ${rsltdir}/di-${bld}-instdir.out > /dev/null 2>&1
    rc=$?
    if [[ $rc -ne 0 ]]; then
      echo "FAIL $(date '+%T') ${host}: ${bld}: ${comp}: missing lib dir"
      trc=1
    fi

    for f in di di.h di.pc di.1 libdi.3; do
      grep -l "^${f}$" ${rsltdir}/di-${bld}-instdir.out > /dev/null 2>&1
      rc=$?
      if [[ $rc -ne 0 ]]; then
        echo "FAIL $(date '+%T') ${host}: ${bld}: ${comp}: missing ${f}"
        trc=1
      fi
    done
    grep -l '^libdi\.[^3]' ${rsltdir}/di-${bld}-instdir.out > /dev/null 2>&1
    rc=$?
    if [[ $rc -ne 0 ]]; then
        echo "FAIL $(date '+%T') ${host}: ${bld}: ${comp}: missing libdi"
      trc=1
    fi
    if [[ $trc != 0 ]]; then
      echo "FAIL $(date '+%T') ${host}: ${bld}: ${comp}: installation files not present"
      failcount=$(($failcount+1))
    fi

    # check for json output
    tcount=$(($tcount+1))
    grep -l '"blocksize"' ${rsltdir}/di-${bld}-run.out > /dev/null 2>&1
    rc=$?
    if [[ $rc != 0 ]]; then
      echo "FAIL $(date '+%T') ${host}: ${bld}: ${comp}: no json output"
      failcount=$(($failcount+1))
    fi
  done
done

grc=0
fflag=""
if [[ $failcount -gt 0 ]]; then
  fflag="FAIL"
  grc=1
fi
echo "-- $(date '+%T') ${host}: ${havecmake} ${havemkc} ${complist} tests: $tcount failures: $failcount $fflag"

exit $grc
