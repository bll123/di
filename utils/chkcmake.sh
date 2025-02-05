#!/bin/sh
#
# Copyright 2025 Brad Lanam Pleasant Hill CA
#

rc=1
CMAKE_REQ_MAJ_VERSION=$1
CMAKE_REQ_MIN_VERSION=$2
export CMAKE_REQ_MAJ_VERSION
export CMAKE_REQ_MIN_VERSION
if [ x"$CMAKE_REQ_MAJ_VERSION" = x ]; then
  echo "chkcmake.sh: no max version specified"
  exit $rc
fi
if [ x"$CMAKE_REQ_MIN_VERSION" = x ]; then
  echo "chkcmake.sh: no min version specified"
  exit $rc
fi

cmvers=`cmake --version 2>/dev/null`
if [ x"$cmvers" = x ]; then
  # echo "no cmake"
  exit $rc
fi

PATH=/opt/local/bin:/usr/local/bin:$PATH

# need a more modern version of awk...
awkcmd=`which gawk 2>/dev/null`
rc=$?
if [ $rc -ne 0 -o "x$awkcmd" = x ]; then
  awkcmd=`which nawk 2>/dev/null`
  rc=$?
  if [ $rc -ne 0 -o "x$awkcmd" = x ]; then
    awkcmd=`which awk 2>/dev/null`
    rc=$?
    if [ $rc -ne 0 -o "x$awkcmd" = x ]; then
      # maybe the which command is not there...
      # try some common spots
      if [ -f /usr/bin/gawk ]; then
        awkcmd=gawk
      fi
      if [ "x$awkcmd" = x -a -f /usr/bin/nawk ]; then
        awkcmd=nawk
      fi
      if [ "x$awkcmd" = x -a -f /usr/bin/awk ]; then
        awkcmd=awk
      fi
    fi
  fi
fi

# no good awk, assume cmake is ng as well.
if [ "x$awkcmd" = x ]; then
  exit $rc
fi

echo "$cmvers" | ${awkcmd} '
BEGIN {
  reqmaj = ENVIRON["CMAKE_REQ_MAJ_VERSION"];
  reqmin = ENVIRON["CMAKE_REQ_MIN_VERSION"];
}
/version/ {
  gsub (/^[^0-9]*/, "");
  gsub (/[^0-9.].*$/, "");
  maj = $0;
  min = $0;
  gsub (/\..*$/, "", maj);
  gsub (/^[0-9]*/, "", min);
  gsub (/^\./, "", min);
  gsub (/\..*$/, "", min);
  rc = 1;
  # have to make sure it is a numeric comparison
  if (maj+0 >= reqmaj+0 && min+0 >= reqmin+0) {
    rc = 0;
  }
  exit rc;
}
'
rc=$?
exit $rc
