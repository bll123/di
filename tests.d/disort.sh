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
if [ $rc -ne 0 ]; then grc=$rc; fi

echo "by special reverse"
$RUNTOPDIR/di -n -a -f S | sort -r > s1
$RUNTOPDIR/di -n -a -f S -srs > s2
diff -w s1 s2
rc=$?
if [ $rc -ne 0 ]; then grc=$rc; fi

echo "by special w/total"
$RUNTOPDIR/di -n -a -f S | sort > s1
$RUNTOPDIR/di -n -a -f S -ss -t | sed '$d' > s2
diff -w s1 s2
rc=$?
if [ $rc -ne 0 ]; then grc=$rc; fi

echo "by special and mount w/total"
$RUNTOPDIR/di -n -a -f 'S~M' | sort -t'~' > s1
$RUNTOPDIR/di -n -a -f 'S~M' -ssm -t | sed '$d' > s2
diff -w s1 s2
rc=$?
if [ $rc -ne 0 ]; then grc=$rc; fi

echo "by mount"
$RUNTOPDIR/di -n -a -f M | sort > s1
$RUNTOPDIR/di -n -a -f M -sm > s2
diff -w s1 s2
rc=$?
if [ $rc -ne 0 ]; then grc=$rc; fi

echo "by mount reverse"
$RUNTOPDIR/di -n -a -f M | sort -r > s1
$RUNTOPDIR/di -n -a -f M -srm > s2
diff -w s1 s2
rc=$?
if [ $rc -ne 0 ]; then grc=$rc; fi

echo "by mount w/total"
$RUNTOPDIR/di -n -a -f M | sort > s1
$RUNTOPDIR/di -n -a -f M -sm -t | sed '$d' > s2
diff -w s1 s2
rc=$?
if [ $rc -ne 0 ]; then grc=$rc; fi

echo "by mount and special w/total"
$RUNTOPDIR/di -n -a -f 'M~S' | sort -t'~' > s1
$RUNTOPDIR/di -n -a -f 'M~S' -sms -t | sed '$d' > s2
diff -w s1 s2
rc=$?
if [ $rc -ne 0 ]; then grc=$rc; fi

echo "by type"
$RUNTOPDIR/di -n -a -f T | sort > s1
$RUNTOPDIR/di -n -a -f T -st > s2
diff -w s1 s2
rc=$?
if [ $rc -ne 0 ]; then grc=$rc; fi

echo "by type reverse"
$RUNTOPDIR/di -n -a -f T | sort -r > s1
$RUNTOPDIR/di -n -a -f T -srt > s2
diff -w s1 s2
rc=$?
if [ $rc -ne 0 ]; then grc=$rc; fi

echo "by type w/total"
$RUNTOPDIR/di -n -a -f T | sort > s1
$RUNTOPDIR/di -n -a -f T -st -t | sed '$d' > s2
diff -w s1 s2
rc=$?
if [ $rc -ne 0 ]; then grc=$rc; fi

echo "by type and special and mount w/total"
$RUNTOPDIR/di -n -a -f 'T~S~M' | sort -t'~' > s1
$RUNTOPDIR/di -n -a -f 'T~S~M' -stsm -t | sed '$d' > s2
diff -w s1 s2
rc=$?
if [ $rc -ne 0 ]; then grc=$rc; fi

sort -k1 > /dev/null < /dev/null
if [ $? = 0 ]; then
  echo "by type and special and mount reversed 2 and 3"
  $RUNTOPDIR/di -n -a -f 'T~S~M' | sort -t'~' -k1,1 -k2,2r -k3,3r > s1
  $RUNTOPDIR/di -n -a -f 'T~S~M' -strsm > s2
  diff -w s1 s2
  rc=$?
  if [ $rc -ne 0 ]; then grc=$rc; fi

  echo "by type and special and mount reversed 2 "
  $RUNTOPDIR/di -n -a -f 'T~S~M' | sort -t'~' -k1,1 -k2,2r -k3,3 > s1
  $RUNTOPDIR/di -n -a -f 'T~S~M' -strsrm > s2
  diff -w s1 s2
  rc=$?
  if [ $rc -ne 0 ]; then grc=$rc; fi
fi

rm -f s1 s2

exit $grc
