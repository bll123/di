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

preserveoutput () {
  fn=$1

  bnm=`basename ${fn}`
  case $bnm in
    di*)
      bnm=`echo ${bnm} | sed 's,^di\.,,'`
      ;;
    *)
      bnm=`echo ${bnm} | sed 's,\.[a-z]*,,'`
      ;;
  esac
  cp $f di-${tag}-${bnm}.out
}

bldrun () {
  tag=$1

  grc=0

  echo "-- `TZ=PST8PDT date '+%T'` ${host}: ${tag}/${comp}"
  make distclean
  if [ $tag = pcmake ]; then
    # pure cmake
    cmake -DCMAKE_INSTALL_PREFIX=${loc}/x -S . -B build > di-${tag}-bld.out 2>&1
    cmake --build build >> di-${tag}-bld.out 2>&1
  else
    make -e CC=${comp} PREFIX=${loc}/x ${tag}-all > di-${tag}-bld.out 2>&1
  fi
  # AIX: BSHIFT: nothing i can do about system headers
  # NetBSD: rpcsvc: deprecated and buggy (why do they complain when there is no alternative?)
  # SCO OpenServer: stdint.h:252: warning: `WCHAR_MAX' redefined
  # SCO OpenServer: sys/mount.h:52: warning: `/*' within comment
  c=`${grepcmd} '(\([WE]\)|warning|error)' di-${tag}-bld.out |
      ${grepcmd} -v 'no-unknown-warning-option' |
      ${grepcmd} -v '_Werror_' |
      ${grepcmd} -v '(pragma|error[=,])' |
      ${grepcmd} -v 'BSHIFT has been redefined' |
      ${grepcmd} -v 'unrecognized command line option' |
      ${grepcmd} -v '/[^d][^i][^.]\.h:.*warning' |
      ${grepcmd} -v 'rpcsvc.*deprecated and buggy' |
      ${grepcmd} -v '^(COMPILE|LINK)' |
      wc -l`
  if [ $c -gt 0 ]; then
    echo "== `TZ=PST8PDT date '+%T'` ${host}: ${tag}/${comp}: warnings or errors found"
    ${grepcmd} '(\([WE]\)|warning|error)' di-${tag}-bld.out |
        ${grepcmd} -v 'no-unknown-warning-option' |
        ${grepcmd} -v '_Werror_' |
        ${grepcmd} -v '(pragma|error[=,])' |
        ${grepcmd} -v 'BSHIFT has been redefined' |
        ${grepcmd} -v 'unrecognized command line option' |
        ${grepcmd} -v '/[^d][^i][^.]\.h:.*warning' |
        ${grepcmd} -v 'rpcsvc.*deprecated and buggy' |
        ${grepcmd} -v '^(COMPILE|LINK)'
    grc=1
  fi

  if [ $tag = cmake -o $tag = pcmake ]; then
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
    echo "== `TZ=PST8PDT date '+%T'` ${host}: ${tag}/${comp}: dimathtest failed"
    grc=1
  fi
  ${getoptntest} > di-${tag}-getoptn.out 2>&1
  rc=$?
  if [ $rc -ne 0 ]; then
    echo "== `TZ=PST8PDT date '+%T'` ${host}: ${tag}/${comp}: getoptn_test failed"
    grc=1
  fi

  if [ $tag = pcmake ]; then
    cmake --install build >> di-${tag}-inst.out 2>&1
  else
    make -e CC=${comp} PREFIX=${loc}/x ${tag}-install > di-${tag}-inst.out 2>&1
  fi

  > di-${tag}-run.out

  echo "-- RUN: -a -d g -f stbuf1cvpB2m -t" >> di-${tag}-run.out
  ./x/bin/di -a -d g -f stbuf1cvpB2m -t >> di-${tag}-run.out 2>&1
  rc=$?
  if [ $rc -ne 0 ]; then
    echo "== `TZ=PST8PDT date '+%T'` ${host}: ${tag}/${comp}: execution of di failed"
    grc=1
  fi

  echo "-- RUN: -d h -f stbuf1cvpB2m -t" >> di-${tag}-run.out
  ./x/bin/di -d h -f stbuf1cvpB2m -t >> di-${tag}-run.out 2>&1
  rc=$?
  if [ $rc -ne 0 ]; then
    echo "== `TZ=PST8PDT date '+%T'` ${host}: ${tag}/${comp}: execution of di failed"
    grc=1
  fi

  # need a run with the basic debug info shown
  echo "-- RUN: -X 1" >> di-${tag}-run.out
  ./x/bin/di -X 1 >> di-${tag}-run.out 2>&1
  rc=$?
  if [ $rc -ne 0 ]; then
    echo "== `TZ=PST8PDT date '+%T'` ${host}: ${tag}/${comp}: execution of di failed"
    grc=1
  fi

  # json output
  echo "-- RUN: -j" >> di-${tag}-run.out
  ./x/bin/di -j >> di-${tag}-run.out 2>&1
  rc=$?
  if [ $rc -ne 0 ]; then
    echo "== `TZ=PST8PDT date '+%T'` ${host}: ${tag}/${comp}: execution of di failed"
    grc=1
  fi

  # csv output, no headers
  echo "-- RUN: -n -C" >> di-${tag}-run.out
  ./x/bin/di -n -C >> di-${tag}-run.out 2>&1
  rc=$?
  if [ $rc -ne 0 ]; then
    echo "== `TZ=PST8PDT date '+%T'` ${host}: ${tag}/${comp}: execution of di failed"
    grc=1
  fi

  # -I flag with unknown fs
  echo "-- RUN: -I something" >> di-${tag}-run.out
  ./x/bin/di -I something >> di-${tag}-run.out 2>&1
  rc=$?
  if [ $rc -ne 0 ]; then
    echo "== `TZ=PST8PDT date '+%T'` ${host}: ${tag}/${comp}: execution of di failed"
    grc=1
  fi

  # -x flag with unknown fs
  echo "-- RUN: -x something" >> di-${tag}-run.out
  ./x/bin/di -x something >> di-${tag}-run.out 2>&1
  rc=$?
  if [ $rc -ne 0 ]; then
    echo "== `TZ=PST8PDT date '+%T'` ${host}: ${tag}/${comp}: execution of di failed"
    grc=1
  fi

  fs=`./x/bin/di -f t -n | head -1`

  # -I flag with known fs
  echo "-- RUN: -I ${fs}" >> di-${tag}-run.out
  ./x/bin/di -I ${fs} >> di-${tag}-run.out 2>&1
  rc=$?
  if [ $rc -ne 0 ]; then
    echo "== `TZ=PST8PDT date '+%T'` ${host}: ${tag}/${comp}: execution of di failed"
    grc=1
  fi

  # -x flag with known fs
  echo "-- RUN: -x ${fs}" >> di-${tag}-run.out
  ./x/bin/di -x ${fs} >> di-${tag}-run.out 2>&1
  rc=$?
  if [ $rc -ne 0 ]; then
    echo "== `TZ=PST8PDT date '+%T'` ${host}: ${tag}/${comp}: execution of di failed"
    grc=1
  fi

  if [ $tag = cmake -o $tag = pcmake ]; then
    for f in build/CMakeFiles/CMakeOutput.log \
        build/CMakeFiles/CMakeError.log \
        build/CMakeFiles/CMakeConfigureLog.yaml \
        build/CMakeCache.txt \
        build/config.h \
        x/lib/pkgconfig/di.pc \
        x/lib64/pkgconfig/di.pc \
        ; do
      if [ -f $f ]; then
        preserveoutput $f
      fi
    done
  fi
  if [ $tag = mkc ]; then
    for f in mkc_files/mkc_compile.log \
        mkc_files/mkconfig.log \
        mkc_files/mkconfig_env.log \
        config.h \
        di.env \
        di.reqlibs \
        x/lib/pkgconfig/di.pc \
        x/lib64/pkgconfig/di.pc \
        ; do
      if [ -f $f ]; then
        preserveoutput $f
      fi
    done
  fi
  ls -1R x > di-${tag}-instdir.out 2>&1
}

systype=`uname -s`

havecmake=F
# add paths for macos and *BSD
PATH="$PATH:/opt/local/bin:/opt/homebrew/bin:/usr/local/bin:/usr/pkg/bin"
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
  exit 1
fi

tar xf ${starfn}
mv ${didir} ${testdir}

cd ${testdir}
rc=$?
if [ $rc -ne 0 ]; then
  echo "Could no cd to ${testdir}"
  exit 1
fi

loc=`pwd`

make -e PREFIX=${loc}/x chkswitcher |
    grep -v 'ing directory' \
    > di-chkswitcher.out 2>&1

bldvar=`./utils/chkcmake.sh ${CMAKE_REQ_MAJ_VERSION} ${CMAKE_REQ_MIN_VERSION}`
if [ $bldvar = cmake ] ;then
  havecmake=T
  bldrun pcmake
  bldrun cmake
fi

bldrun mkc
echo ${systype} > di-systype.out

exit $grc
