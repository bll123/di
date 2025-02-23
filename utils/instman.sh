#!/bin/sh

prefix=$1
mandir=$2
grc=0

if [ "x${prefix}" = x ]; then
  echo "prefix is not set"
  exit 1
fi

if [ "x${mandir}" = x ]; then
  echo "mandir is not set"
  exit 1
fi

systype=`uname -s`
case ${systype} in
  NetBSD)
    # netbsd uses /usr/pkg/man
    mandir="${prefix}/man"
    ;;
  OpenBSD)
    # openbsd cmake puts the man pages at the top level.
    # match what cmake does.
    # I don't know why there is a mis-match between cmake and where
    # the os puts the system manual pages.
    mandir="${prefix}/man"
    ;;
esac

instmandir="${mandir}"
if [ ! "x${DESTDIR}" = x ]; then
  instmandir="${DESTDIR}/${mandir}"
fi

test -d ${instmandir}/man1 || mkdir -p ${instmandir}/man1
cp -f man/di.1 ${instmandir}/man1/
rc=$?
if [ $rc -ne 0 ]; then
  grc=1
fi

test -d ${instmandir}/man3 || mkdir -p ${instmandir}/man3
cp -f man/libdi.3 ${instmandir}/man3/
rc=$?
if [ $rc -ne 0 ]; then
  grc=1
fi

exit $grc
