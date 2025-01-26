#!/bin/sh


CMAKE_REQ_MAJ_VERSION=3
CMAKE_REQ_MIN_VERSION=13

host=$1
tarfn=$2
didir=$3
quiet=$4

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

test -d ${didir} && rm -rf ${didir}
tar xf ${tarfn}
cd ${didir}
rc=$?
if [ $rc -ne 0 ]; then
  exit 1
fi

loc=`pwd`

bldrun () {
  tag=$1

  echo "-- $(date +%T) ${host}: build, install and run with ${tag}"
  make distclean
  make -e PREFIX=${loc}/x ${tag}-all > di-${tag}-bld.out 2>&1
  c=`grep -E '(warning|error)' di-${tag}-bld.out | grep -v 'error[=,]' | wc -l`
  if [ $c -gt 0 ]; then
    echo "== $(date +%T) ${host}: ${tag}: warnings or errors found"
  fi
  make -e PREFIX=${loc}/x ${tag}-install > di-${tag}-inst.out 2>&1
  ./x/bin/di -a -d g -f stbuf1cvpB2m -t > di-${tag}-run.out 2>&1
  rc=$?
  if [ $rc -ne 0 ]; then
    echo "== $(date +%T) ${host}: ${tag}: execution of di failed"
    exit 1
  fi
}

echo "start $(date +%T)" > di-start.out
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
echo "-- $(date +%T) ${host}: finish"
echo "done $(date +%T)" > di-finish.out
exit 0
