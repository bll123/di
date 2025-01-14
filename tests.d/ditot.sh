#!/bin/sh

. $_MKCONFIG_DIR/bin/testfuncs.sh

maindodisplay $1 'di totals'
maindoquery $1 $_MKC_ONCE

getsname $0
dosetup $@

. $_MKCONFIG_DIR/bin/shellfuncs.sh
testshcapability

unset DI_ARGS
unset DIFMT
FORMATS="b B u c f v i U F"

grc=0
for d in C D; do
  if [ $d = D -a \( "$DC" = "" -o "$DC" = "skip" \) ]; then
    continue
  fi
  tdir=$_MKCONFIG_RUNTOPDIR/$d
  (
    cd $tdir
    if [ $? -eq 0 ]; then
      instdir="`pwd`/test_di"
      ${MAKE:-make} ${TMAKEFLAGS} -e prefix=${instdir} all \
        > ${_MKCONFIG_TSTRUNTMPDIR}/make.log 2>&1
    fi
  )
  if [ -x ${tdir}/di ]; then
    putsnonl " ${d}" >&5
    for format in $FORMATS; do
      puts "Checking format: $format"
      # have to exclude zfs, null (dragonflybsd)
      # otherwise this test won't work.
      # include the normally excluded to get some data.
      # ctfs,objfs,sharefs have weird used inode counts (U)
      didata=`${tdir}/di -n -d1 -f $format -t -a -x apfs,null,zfs,ctfs,objfs,sharefs 2>/dev/null `
      summtot=`(echo "0 ";echo $didata | sed 's/  */ + /g'; echo " - p") | dc`
      if [ $summtot -ne 0 ]; then
        putsnonl "*" >&5
        puts "## format: $format failed"
        grc=1
      fi
    done
  else
    if [ $d = C ]; then
      puts "## no di executable found for dir $d"
      grc=1
    fi
  fi
done

exit $grc
