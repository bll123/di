#!/bin/sh
#
# Copyright 2025 Brad Lanam Pleasant Hill CA
#

CMAKE_REQ_MAJ_VERSION=3
CMAKE_REQ_MIN_VERSION=13

host=$1
tarfn=$2
didir=$3
comp=$4

havecmake=F
# add paths for macos and *BSD
PATH="$PATH:/opt/local/bin:/usr/local/bin"

if [ x${tarfn} = x ]; then
  echo "== ${host}: No tar filename"
  exit 1
fi
if [ x${didir} = x ]; then
  echo "== ${host}: No di directory"
  exit 1
fi

testdir=${didir}_${comp}
test -d ${testdir} && rm -rf ${testdir}
tar xf ${tarfn}
mv ${didir} ${testdir}

cd ${testdir}
rc=$?
if [ $rc -ne 0 ]; then
  exit 1
fi

loc=`pwd`

bldrun () {
  tag=$1

  echo "-- $(date +%T) ${host}: build, install and run with ${tag}/${comp}"
  make distclean
  make -e CC=${comp} PREFIX=${loc}/x ${tag}-all > di-${tag}-bld.out 2>&1
  c=`grep -E '(warning|error)' di-${tag}-bld.out |
      grep -E -v '(pragma|error[=,])' |
      wc -l`
  if [ $c -gt 0 ]; then
    echo "== $(date +%T) ${host}: ${tag}: warnings or errors found"
  fi
  if [ $tag = cmake ]; then
    for f in build/CMakeFiles/CMakeOutput.log build/CMakeFiles/CMakeError.log; do
      if [ -f $f ]; then
        cp $f di-${tag}-out.out
        cp $f di-${tag}-err.out
      fi
    done
  fi
  if [ $tag = mkc ]; then
    for f in mkc_files/mkc_compile.log mkc_files/mkconfig.log; do
      if [ -f $f ]; then
        cp $f di-${tag}-comp.out
        cp $f di-${tag}-conf.out
      fi
    done
  fi
  make -e CC=${comp} PREFIX=${loc}/x ${tag}-install > di-${tag}-inst.out 2>&1
  ./x/bin/di -a -d g -f stbuf1cvpB2m -t > di-${tag}-run.out 2>&1
  rc=$?
  if [ $rc -ne 0 ]; then
    echo "== $(date +%T) ${host}: ${tag}: execution of di failed"
    exit 1
  fi
}

if [ -f /usr/bin/cmake -o \
    -f /usr/local/bin/cmake -o \
    -f /opt/local/bin/cmake ]; then
  cmvers=`cmake --version 2>/dev/null`
  cmmajv=`echo ${cmvers} | \
      sed -n -e '/version/ s,[^0-9]*\([0-9]*\)\..*,\1, p'` ; \
  cmminv=`echo ${cmvers} | \
      sed -n -e '/version/ s,[^0-9]*3\.\([0-9]*\).*,\1, p'` ; \
  if [ x${cmmajv} = x ]; then cmmajv=0; fi
  if [ x${cmminv} = x ]; then cmminv=0; fi
  if [ "${cmmajv}" -ge ${CMAKE_REQ_MAJ_VERSION} -a \
      "${cmminv}" -ge ${CMAKE_REQ_MIN_VERSION} ]; then \
    havecmake=T
    bldrun cmake
  fi
fi

bldrun mkc
if [ $havecmake = T ]; then
  diff di-cmake-run.out di-mkc-run.out > di-diff.out 2>&1
  rc=$?
  if [ $rc -ne 0 ]; then
    echo "== $(date +%T) ${host}: diff of cmake and mkc failed"
    exit 1
  fi
fi
exit 0
