#!/bin/sh

. $_MKCONFIG_DIR/bin/testfuncs.sh

maindodisplay $1 'di sort'
maindoquery $1 $_MKC_ONCE

getsname $0
dosetup $@

LC_ALL="C"
export LC_ALL
unset DI_ARGS
unset DIFMT

dotest () {
  chkdiff s1 s2
  rc=$?
  if [ $rc -ne 0 ]; then
    echo "=== s1"
    cat s1
    echo "=== s2"
    cat s2
  fi
  if [ $rc -ne 0 ]; then
    grc=$rc
  fi
}

for d in C D; do
  if [ $d = D -a \( "$DC" = "" -o "$DC" = "skip" \) ]; then
    continue
  fi
  tdir=$_MKCONFIG_RUNTOPDIR/$d
  (
    cd $tdir
    if [ $? -eq 0 ]; then
      instdir="`pwd`/test_di"
      ${MAKE:-make} ${TMAKEFLAGS} -e prefix=${instdir} all
        > ${_MKCONFIG_TSTRUNTMPDIR}/make.log 2>&1
    fi
  )
  if [ -x $tdir/di ]; then
    putsnonl " ${d}" >&5
    puts "## regular sort first, then di sort"
    puts "by special"
    ${tdir}/di -n -a -f S | sort > s1
    ${tdir}/di -n -a -f S -ss > s2
    dotest

    puts "by special reverse"
    ${tdir}/di -n -a -f S | sort -r > s1
    ${tdir}/di -n -a -f S -srs > s2
    dotest

    puts "by special w/total"
    ${tdir}/di -n -a -f S | sort -t'~' > s1
    ${tdir}/di -n -a -f S -ss -t | sed '$d' > s2
    dotest

    puts "by special and mount w/total"
    ${tdir}/di -n -a -f 'S~M' | sort -t'~' > s1
    ${tdir}/di -n -a -f 'S~M' -ssm -t | sed '$d' > s2
    dotest

    puts "by mount"
    ${tdir}/di -n -a -f M | sort -t'~' > s1
    ${tdir}/di -n -a -f M -sm > s2
    dotest

    puts "by mount reverse"
    ${tdir}/di -n -a -f M | sort -r > s1
    ${tdir}/di -n -a -f M -srm > s2
    dotest

    puts "by mount w/total"
    ${tdir}/di -n -a -f M | sort -t'~' > s1
    ${tdir}/di -n -a -f M -sm -t | sed '$d' > s2
    dotest

    puts "by mount and special w/total"
    ${tdir}/di -n -a -f 'M~S' | sort -t'~' > s1
    ${tdir}/di -n -a -f 'M~S' -sms -t | sed '$d' > s2
    dotest

    puts "by type"
    ${tdir}/di -n -a -f T | sort > s1
    ${tdir}/di -n -a -f T -st > s2
    dotest

    puts "by type reverse"
    ${tdir}/di -n -a -f T | sort -r > s1
    ${tdir}/di -n -a -f T -srt > s2
    dotest

    puts "by type w/total"
    ${tdir}/di -n -a -f T | sort -t'~' > s1
    ${tdir}/di -n -a -f T -st -t | sed '$d' > s2
    dotest

    puts "by type and special and mount w/total"
    ${tdir}/di -n -a -f 'T~S~M' | sort -t'~' > s1
    ${tdir}/di -n -a -f 'T~S~M' -stsm -t | sed '$d' > s2
    dotest

    sort -k1 > /dev/null < /dev/null
    if [ $? = 0 ]; then
      puts "by type and special and mount reversed 2 and 3"
      ${tdir}/di -n -a -f 'T~S~M' | sort -t'~' -k1,1 -k2,2r -k3,3r > s1
      ${tdir}/di -n -a -f 'T~S~M' -strsm > s2
      dotest

      puts "by type and special and mount reversed 2 "
      ${tdir}/di -n -a -f 'T~S~M' | sort -t'~' -k1,1 -k2,2r -k3,3 > s1
      ${tdir}/di -n -a -f 'T~S~M' -strsrm > s2
      dotest
    fi

    rm -f s1 s2
    if [ $grc -ne 0 ]; then
      putsnonl "*" >&5
    fi
  else
    if [ $d = C ]; then
      puts "## no di executable found for dir $d"
      putsnonl "*" >&5
      grc=1
    fi
  fi
done

exit $grc
