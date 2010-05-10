#!/bin/sh
#
#  Copyright 2010 Brad Lanam Walnut Creek, CA USA
#

echo ${EN} "di sort${EC}" >&3

grc=0

LC_ALL="C"
export LC_ALL

echo "by special"
$RUNTOPDIR/di -n -a -f S | sort > s1
$RUNTOPDIR/di -n -a -f S -ss > s2
diff -w s1 s2
rc=$?
if [ $rc -ne 0 ]; then
  grc=$rc;
  echo "=== s1"
  cat s1
  echo "=== s2"
  cat s2
fi

echo "by special reverse"
$RUNTOPDIR/di -n -a -f S | sort -r > s1
$RUNTOPDIR/di -n -a -f S -srs > s2
diff -w s1 s2
rc=$?
if [ $rc -ne 0 ]; then
  grc=$rc;
  echo "=== s1"
  cat s1
  echo "=== s2"
  cat s2
fi

echo "by special w/total"
$RUNTOPDIR/di -n -a -f S~b | sort -t'~' > s1
$RUNTOPDIR/di -n -a -f S~b -ss -t | sed '$d' > s2
diff -w s1 s2
rc=$?
if [ $rc -ne 0 ]; then
  grc=$rc;
  echo "=== s1"
  cat s1
  echo "=== s2"
  cat s2
fi

echo "by special and mount w/total"
$RUNTOPDIR/di -n -a -f 'S~M~b' | sort -t'~' > s1
$RUNTOPDIR/di -n -a -f 'S~M~b' -ssm -t | sed '$d' > s2
diff -w s1 s2
rc=$?
if [ $rc -ne 0 ]; then
  grc=$rc;
  echo "=== s1"
  cat s1
  echo "=== s2"
  cat s2
fi

echo "by mount"
$RUNTOPDIR/di -n -a -f M | sort -t'~' > s1
$RUNTOPDIR/di -n -a -f M -sm > s2
diff -w s1 s2
rc=$?
if [ $rc -ne 0 ]; then
  grc=$rc;
  echo "=== s1"
  cat s1
  echo "=== s2"
  cat s2
fi

echo "by mount reverse"
$RUNTOPDIR/di -n -a -f M | sort -r > s1
$RUNTOPDIR/di -n -a -f M -srm > s2
diff -w s1 s2
rc=$?
if [ $rc -ne 0 ]; then
  grc=$rc;
  echo "=== s1"
  cat s1
  echo "=== s2"
  cat s2
fi

echo "by mount w/total"
$RUNTOPDIR/di -n -a -f M~b | sort -t'~' > s1
$RUNTOPDIR/di -n -a -f M~b -sm -t | sed '$d' > s2
diff -w s1 s2
rc=$?
if [ $rc -ne 0 ]; then
  grc=$rc;
  echo "=== s1"
  cat s1
  echo "=== s2"
  cat s2
fi

echo "by mount and special w/total"
$RUNTOPDIR/di -n -a -f 'M~S~b' | sort -t'~' > s1
$RUNTOPDIR/di -n -a -f 'M~S~b' -sms -t | sed '$d' > s2
diff -w s1 s2
rc=$?
if [ $rc -ne 0 ]; then
  grc=$rc;
  echo "=== s1"
  cat s1
  echo "=== s2"
  cat s2
fi

echo "by type"
$RUNTOPDIR/di -n -a -f T | sort > s1
$RUNTOPDIR/di -n -a -f T -st > s2
diff -w s1 s2
rc=$?
if [ $rc -ne 0 ]; then
  grc=$rc;
  echo "=== s1"
  cat s1
  echo "=== s2"
  cat s2
fi

echo "by type reverse"
$RUNTOPDIR/di -n -a -f T | sort -r > s1
$RUNTOPDIR/di -n -a -f T -srt > s2
diff -w s1 s2
rc=$?
if [ $rc -ne 0 ]; then
  grc=$rc;
  echo "=== s1"
  cat s1
  echo "=== s2"
  cat s2
fi

echo "by type w/total"
$RUNTOPDIR/di -n -a -f T | sort -t'~' > s1
$RUNTOPDIR/di -n -a -f T -st -t | sed '$d' > s2
diff -w s1 s2
rc=$?
if [ $rc -ne 0 ]; then
  grc=$rc;
  echo "=== s1"
  cat s1
  echo "=== s2"
  cat s2
fi

echo "by type and special and mount w/total"
$RUNTOPDIR/di -n -a -f 'T~S~M~b' | sort -t'~' > s1
$RUNTOPDIR/di -n -a -f 'T~S~M~b' -stsm -t | sed '$d' > s2
diff -w s1 s2
rc=$?
if [ $rc -ne 0 ]; then
  grc=$rc;
  echo "=== s1"
  cat s1
  echo "=== s2"
  cat s2
fi

sort -k1 > /dev/null < /dev/null
if [ $? = 0 ]; then
  echo "by type and special and mount reversed 2 and 3"
  $RUNTOPDIR/di -n -a -f 'T~S~M~b' | sort -t'~' -k1,1 -k2,2r -k3,3r > s1
  $RUNTOPDIR/di -n -a -f 'T~S~M~b' -strsm > s2
  diff -w s1 s2
  rc=$?
  if [ $rc -ne 0 ]; then
    grc=$rc;
    echo "=== s1"
    cat s1
    echo "=== s2"
    cat s2
  fi

  echo "by type and special and mount reversed 2 "
  $RUNTOPDIR/di -n -a -f 'T~S~M~b' | sort -t'~' -k1,1 -k2,2r -k3,3 > s1
  $RUNTOPDIR/di -n -a -f 'T~S~M~b' -strsrm > s2
  diff -w s1 s2
  rc=$?
  if [ $rc -ne 0 ]; then
    grc=$rc;
    echo "=== s1"
    cat s1
    echo "=== s2"
    cat s2
  fi
fi

rm -f s1 s2

exit $grc
