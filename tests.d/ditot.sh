#!/bin/sh

. $_MKCONFIG_DIR/testfuncs.sh

maindodisplay $1 'di totals'
maindoquery $1 $_MKC_SH

getsname $0
dosetup $@

unset DI_ARGS
unset DIFMT
FORMATS="b B u c f v i U F"

grc=0
for format in $FORMATS; do
  echo "Checking format: $format"
  # have to exclude zfs, otherwise this test won't work.
  # include the normally excluded to get some data.
  # ctfs,objfs,sharefs have weird used inode counts (U)
  didata=`$_MKCONFIG_RUNTOPDIR/di -n -d1 -f $format -t -a -x zfs,ctfs,objfs,sharefs 2>/dev/null `
  summtot=`(echo "0 ";echo $didata | sed 's/  */ + /g'; echo " - p") | dc`
  if [ $summtot -ne 0 ]; then
    echo "format: $format failed"
    grc=1
  fi
done

exit $grc
