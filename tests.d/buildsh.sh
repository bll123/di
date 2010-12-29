#!/bin/sh
#
#  Copyright 2010 Brad Lanam Walnut Creek, CA USA
#

if [ "$1" = "-d" ]; then
  echo ${EN} " build w/mkconfig.sh${EC}"
  exit 0
fi

. $_MKCONFIG_DIR/shellfuncs.sh
testshcapability

grc=0
stag=$1
shift

cd $_MKCONFIG_RUNTOPDIR

instdir="`pwd`/test_di"
make realclean
make -e prefix=${instdir} all-sh > make.log 2>&1
rc=$?
if [ $rc != 0 ]; then grc=$rc; fi

if [ $grc -eq 0 ]; then
  cat make.log |
    grep -v '^load\-unit:' |
    grep -v '^output\-file:' |
    grep -v '^option\-file:' |
    grep -v ' \.\.\. ' |
    grep -vi mkconfig |
    grep -v 'Leaving directory' |
    grep -v 'Entering directory' |
    grep -v '^cc \-[co]' |
    grep -v digetentries.o |
    grep -v 'Using libs:' |
    grep -v 'di\.env' |
    grep -v '^CC=' > make_extra.log
  extra=`cat make_extra.log | wc -l make_extra.log`
  if [ $extra -ne 0 ]; then
    echo "## extra output"
    grc=1
  fi
fi

mkdir -p $_MKCONFIG_TSTRUNTMPDIR/buildsh${stag}
# leave these laying around for use by install.sh and rpmbuild.sh test
cp mkconfig.log mkconfig.cache mkconfig*.vars di.env mkconfig.reqlibs \
    make.log make_extra.log \
    $_MKCONFIG_TSTRUNTMPDIR/buildsh${stag}
rm -f make.log make_extra.log >/dev/null 2>&1

exit $grc
