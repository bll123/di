#!/bin/sh

echo ${EN} "di totals${EC}" >&3

FORMATS="b B u c f v i U F"

grc=0
for format in $FORMATS; do
  echo "Checking format: $format"
  didata=`../di -n -d1 -f $format -t`
  summtot=`(echo "0 ";echo $didata | sed 's/  */ + /g'; echo " - p") | dc`
  if [ $summtot -ne 0 ]; then
    echo "format: $format failed"
    grc=1
  fi
done

exit $grc
