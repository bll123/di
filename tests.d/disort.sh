#!/bin/sh

echo ${EN} "di sort${EC}" >&3

grc=0

export LC_ALL="C"

echo "by special"
../test_di/bin/di -n -a -f S | sort > s1
../test_di/bin/di -n -a -f S -ss > s2
diff -w s1 s2
rc=$?
if [ $rc -ne 0 ]; then grc=$rc; fi

echo "by special reverse"
../test_di/bin/di -n -a -f S | sort -r > s1
../test_di/bin/di -n -a -f S -srs > s2
diff -w s1 s2
rc=$?
if [ $rc -ne 0 ]; then grc=$rc; fi

echo "by special w/total"
../test_di/bin/di -n -a -f S | sort > s1
../test_di/bin/di -n -a -f S -ss -t | sed '$d' > s2
diff -w s1 s2
rc=$?
if [ $rc -ne 0 ]; then grc=$rc; fi

echo "by special and mount w/total"
../test_di/bin/di -n -a -f SM | sort > s1
../test_di/bin/di -n -a -f SM -ssm -t | sed '$d' > s2
diff -w s1 s2
rc=$?
if [ $rc -ne 0 ]; then grc=$rc; fi

echo "by mount"
../test_di/bin/di -n -a -f M | sort > s1
../test_di/bin/di -n -a -f M -sm > s2
diff -w s1 s2
rc=$?
if [ $rc -ne 0 ]; then grc=$rc; fi

echo "by mount reverse"
../test_di/bin/di -n -a -f M | sort -r > s1
../test_di/bin/di -n -a -f M -srm > s2
diff -w s1 s2
rc=$?
if [ $rc -ne 0 ]; then grc=$rc; fi

echo "by mount w/total"
../test_di/bin/di -n -a -f M | sort > s1
../test_di/bin/di -n -a -f M -sm -t | sed '$d' > s2
diff -w s1 s2
rc=$?
if [ $rc -ne 0 ]; then grc=$rc; fi

echo "by mount and special w/total"
../test_di/bin/di -n -a -f MS | sort > s1
../test_di/bin/di -n -a -f MS -sms -t | sed '$d' > s2
diff -w s1 s2
rc=$?
if [ $rc -ne 0 ]; then grc=$rc; fi

echo "by type"
../test_di/bin/di -n -a -f T | sort > s1
../test_di/bin/di -n -a -f T -st > s2
diff -w s1 s2
rc=$?
if [ $rc -ne 0 ]; then grc=$rc; fi

echo "by type reverse"
../test_di/bin/di -n -a -f T | sort -r > s1
../test_di/bin/di -n -a -f T -srt > s2
diff -w s1 s2
rc=$?
if [ $rc -ne 0 ]; then grc=$rc; fi

echo "by type w/total"
../test_di/bin/di -n -a -f T | sort > s1
../test_di/bin/di -n -a -f T -st -t | sed '$d' > s2
diff -w s1 s2
rc=$?
if [ $rc -ne 0 ]; then grc=$rc; fi

echo "by type and special and mount w/total"
../test_di/bin/di -n -a -f TSM | sort > s1
../test_di/bin/di -n -a -f TSM -stsm -t | sed '$d' > s2
diff -w s1 s2
rc=$?
if [ $rc -ne 0 ]; then grc=$rc; fi

sort -k1 > /dev/null < /dev/null
if [ $? = 0 ]; then
  echo "by type and special and mount reversed 2 and 3"
  ../test_di/bin/di -n -a -f TSM | sort -k1,1 -k2,2r -k3,3r > s1
  ../test_di/bin/di -n -a -f TSM -strsm > s2
  diff -w s1 s2
  rc=$?
  if [ $rc -ne 0 ]; then grc=$rc; fi

  echo "by type and special and mount reversed 2 "
  ../test_di/bin/di -n -a -f TSM | sort -k1,1 -k2,2r -k3,3 > s1
  ../test_di/bin/di -n -a -f TSM -strsrm > s2
  diff -w s1 s2
  rc=$?
  if [ $rc -ne 0 ]; then grc=$rc; fi
fi

rm -f s1 s2

exit $grc
