#!/bin/sh

echo ${EN} "di totals${EC}" >&3

FORMATS="b B u c f v i U F"

grc=0
for format in $FORMATS; do
  echo "Checking format: $format"
  ditot=`../di -n -d1 -f $format -t | tail -1`
  didata=`../di -n -d1 -f $format`
  diline=`echo $didata`  # convert to single line
  sumtot=`(echo "0 ";echo $diline | sed 's/  */ + /g'; echo " + p") | dc`
  echo "Total from di: $ditot"
  echo "Total summed: $sumtot"
  if [ $ditot -ne $sumtot ]; then
    echo "format: $format failed"
    grc=1
  fi
done

exit $grc
