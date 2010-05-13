#!/bin/sh
#
#  Copyright 2010 Brad Lanam Walnut Creek, CA USA
#

echo ${EN} "di sort${EC}" >&5

grc=0

LC_ALL="C"
export LC_ALL

dotest () {
  diff -w s1 s2
  rc=$?
  if [ $rc -ne 0 ]; then
    echo "=== s1"
    cat s1
    echo "=== s2"
    cat s2
  fi
  if [ $rc -ne 0 ]; then
    grc=$rc;
  fi
}

echo "## regular sort first, then di sort"
echo "by special"
$RUNTOPDIR/di -n -a -f S | sort > s1
$RUNTOPDIR/di -n -a -f S -ss > s2
dotest

echo "by special reverse"
$RUNTOPDIR/di -n -a -f S | sort -r > s1
$RUNTOPDIR/di -n -a -f S -srs > s2
dotest

echo "by special w/total"
${_MKCONFIG_RUNTOPDIR}/di -n -a -f S | sort -t'~' > s1
${_MKCONFIG_RUNTOPDIR}/di -n -a -f S -ss -t | sed '$d' > s2
dotest

echo "by special and mount w/total"
${_MKCONFIG_RUNTOPDIR}/di -n -a -f 'S~M' | sort -t'~' > s1
${_MKCONFIG_RUNTOPDIR}/di -n -a -f 'S~M' -ssm -t | sed '$d' > s2
dotest

echo "by mount"
${_MKCONFIG_RUNTOPDIR}/di -n -a -f M | sort -t'~' > s1
${_MKCONFIG_RUNTOPDIR}/di -n -a -f M -sm > s2
dotest

echo "by mount reverse"
${_MKCONFIG_RUNTOPDIR}/di -n -a -f M | sort -r > s1
${_MKCONFIG_RUNTOPDIR}/di -n -a -f M -srm > s2
dotest

echo "by mount w/total"
${_MKCONFIG_RUNTOPDIR}/di -n -a -f M | sort -t'~' > s1
${_MKCONFIG_RUNTOPDIR}/di -n -a -f M -sm -t | sed '$d' > s2
dotest

echo "by mount and special w/total"
${_MKCONFIG_RUNTOPDIR}/di -n -a -f 'M~S' | sort -t'~' > s1
${_MKCONFIG_RUNTOPDIR}/di -n -a -f 'M~S' -sms -t | sed '$d' > s2
dotest

echo "by type"
${_MKCONFIG_RUNTOPDIR}/di -n -a -f T | sort > s1
${_MKCONFIG_RUNTOPDIR}/di -n -a -f T -st > s2
dotest

echo "by type reverse"
${_MKCONFIG_RUNTOPDIR}/di -n -a -f T | sort -r > s1
${_MKCONFIG_RUNTOPDIR}/di -n -a -f T -srt > s2
dotest

echo "by type w/total"
${_MKCONFIG_RUNTOPDIR}/di -n -a -f T | sort -t'~' > s1
${_MKCONFIG_RUNTOPDIR}/di -n -a -f T -st -t | sed '$d' > s2
dotest

echo "by type and special and mount w/total"
${_MKCONFIG_RUNTOPDIR}/di -n -a -f 'T~S~M' | sort -t'~' > s1
${_MKCONFIG_RUNTOPDIR}/di -n -a -f 'T~S~M' -stsm -t | sed '$d' > s2
dotest

sort -k1 > /dev/null < /dev/null
if [ $? = 0 ]; then
  echo "by type and special and mount reversed 2 and 3"
  ${_MKCONFIG_RUNTOPDIR}/di -n -a -f 'T~S~M' | sort -t'~' -k1,1 -k2,2r -k3,3r > s1
  ${_MKCONFIG_RUNTOPDIR}/di -n -a -f 'T~S~M' -strsm > s2
  dotest

  echo "by type and special and mount reversed 2 "
  ${_MKCONFIG_RUNTOPDIR}/di -n -a -f 'T~S~M' | sort -t'~' -k1,1 -k2,2r -k3,3 > s1
  ${_MKCONFIG_RUNTOPDIR}/di -n -a -f 'T~S~M' -strsrm > s2
  dotest
fi

rm -f s1 s2

exit $grc
