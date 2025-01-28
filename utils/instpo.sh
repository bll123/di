#!/bin/sh

INST_LOCALEDIR=$1

cd po
rc=$?
if [ $rc -ne 0 ]; then
  echo "No po/ directory."
  exit 0
fi

test -d "${INST_LOCALEDIR}" || mkdir -p "${INST_LOCALEDIR}"

for i in *.po; do
  j=`echo $i | sed 's,\\.po$,,'`
  msgfmt -o $j.mo $i 2> /dev/null
  rc=$?
  if [ $rc -ne 0 ]; then
    echo "msgfmt failed"
    break
  fi
  if [ ! -f $j.mo ]; then
    continue
  fi
  test -d ${INST_LOCALEDIR}/$j/LC_MESSAGES ||
      mkdir -p ${INST_LOCALEDIR}/$j/LC_MESSAGES
  cp -pf $j.mo ${INST_LOCALEDIR}/$j/LC_MESSAGES/di.mo
  rm -f $j.mo
done

exit 0
