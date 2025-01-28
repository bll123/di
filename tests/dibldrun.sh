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
rempath=$5

# snarfed from mkconfig
test_egrep () {
  tfn=egreptest
  echo "a b c" > ${tfn}
  # use grep -E by preference
  (eval 'grep -E "a|b" ${tfn}') >/dev/null 2>&1
  if [ $? -eq 0 ]; then
    grepcmd="grep -E"
  else
    (eval 'egrep "a|b" ${tfn}') >/dev/null 2>&1
    if [ $? -eq 0 ]; then
      grepcmd="egrep"
    fi
  fi
  rm -f ${tfn}
}

bldrun () {
  tag=$1

  grc=0

  echo "-- `date '+%T'` ${host}: ${tag}/${comp}"
  make distclean
  make -e CC=${comp} PREFIX=${loc}/x ${tag}-all > di-${tag}-bld.out 2>&1
  # AIX: BSHIFT: nothing i can do about system headers
  # NetBSD: rpcsvc: deprecated and buggy (why do they complain when there is no alternative?)
  c=`${grepcmd} '(\(W\)|\(E\)|warning|error)' di-${tag}-bld.out |
      ${grepcmd} -v '(pragma|error[=,])' |
      ${grepcmd} -v 'BSHIFT has been redefined' |
      ${grepcmd} -v '\.h:.*warning' |
      ${grepcmd} -v 'unrecognized command line option' |
      ${grepcmd} -v 'rpcsvc.*deprecated and buggy' |
      ${grepcmd} -v '^COMPILE' |
      wc -l`
  if [ $c -gt 0 ]; then
    echo "== `date '+%T'` ${host}: ${tag}/${comp}: warnings or errors found"
    ${grepcmd} '(\(W\)|\(E\)|warning|error)' di-${tag}-bld.out |
        ${grepcmd} -v '(pragma|error[=,])' |
        ${grepcmd} -v 'BSHIFT has been redefined' |
        ${grepcmd} -v '\.h:.*warning' |
        ${grepcmd} -v 'unrecognized command line option' |
        ${grepcmd} -v 'rpcsvc.*deprecated and buggy' |
        ${grepcmd} -v '^COMPILE'
    grc=1
  fi
  if [ $tag = cmake ]; then
    for f in build/CMakeFiles/CMakeOutput.log \
        build/CMakeFiles/CMakeError.log \
        build/config.h; do
      if [ -f $f ]; then
        bnm=`basename ${f} | sed 's,\.[a-z]*,,'`
        cp $f di-${tag}-${bnm}.out
      fi
    done
  fi
  if [ $tag = mkc ]; then
    for f in mkc_files/mkc_compile.log \
        mkc_files/mkconfig.log \
        mkc_files/mkconfig_env.log \
        config.h \
        di.env; do
      if [ -f $f ]; then
        bnm=`basename ${f} | sed 's,\.[a-z]*,,'`
        cp $f di-${tag}-${bnm}.out
      fi
    done
  fi

  if [ $tag = cmake ]; then
    mathtest=./build/dimathtest
    getoptntest=./build/getoptn_test
  fi
  if [ $tag = mkc ]; then
    mathtest=./dimathtest
    getoptntest=./getoptn_test
  fi

  ${mathtest} > di-${tag}-math.out 2>&1
  rc=$?
  if [ $rc -ne 0 ]; then
    echo "== `date '+%T'` ${host}: ${tag}/${comp}: dimathtest failed"
    grc=1
  fi
  ${getoptntest} > di-${tag}-getoptn.out 2>&1
  rc=$?
  if [ $rc -ne 0 ]; then
    echo "== `date '+%T'` ${host}: ${tag}/${comp}: getoptn_test failed"
    grc=1
  fi

  make -e CC=${comp} PREFIX=${loc}/x ${tag}-install > di-${tag}-inst.out 2>&1
  ./x/bin/di -a -d g -f stbuf1cvpB2m -t > di-${tag}-run.out 2>&1
  rc=$?
  if [ $rc -ne 0 ]; then
    echo "== `date '+%T'` ${host}: ${tag}/${comp}: execution of di failed"
  fi
  if [ $grc -ne 0 ]; then
    exit 1
  fi
}

havecmake=F
# add paths for macos and *BSD
PATH="$PATH:/opt/local/bin:/usr/local/bin"
if [ "${rempath}" != - ]; then
  PATH="${rempath}:${PATH}"
fi
test_egrep

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
test -d ${testdir} && rm -rf ${testdir}  # aix is weird
starfn=`echo ${tarfn} | sed 's,\.gz$,,'`
if [ -f ${tarfn} ]; then
  rm -f ${starfn}
  gunzip ${tarfn}
fi
if [ ! -f ${starfn} ]; then
  echo "== ${host}: gunzip failed"
fi

tar xf ${starfn}
mv ${didir} ${testdir}

cd ${testdir}
rc=$?
if [ $rc -ne 0 ]; then
  exit 1
fi

loc=`pwd`

./utils/chkcmake.sh ${CMAKE_REQ_MAJ_VERSION} ${CMAKE_REQ_MIN_VERSION}
rc=$?
if [ $rc -eq 0 ] ;then
  havecmake=T
  bldrun cmake
fi

bldrun mkc

exit 0
