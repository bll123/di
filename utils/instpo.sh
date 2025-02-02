#!/bin/sh
#
# Copyright 2025 Brad Lanam Pleasant Hill CA
#


INST_LOCALEDIR=$1

cd po
rc=$?
if [ $rc -ne 0 ]; then
  echo "No po/ directory."
  exit 0
fi

test -d "${INST_LOCALEDIR}" || mkdir -p "${INST_LOCALEDIR}"

# try for xmsgfmt first, for older systems
msgfmtcmd=$(which xmsgfmt 2>/dev/null)
rc=$?
if [ $rc -ne 0 -o "x$msgfmtcmd" != x ]; then
  msgfmtcmd=$(which gmsgfmt 2>/dev/null)
  rc=$?
  if [ $rc -ne 0 -o "x$msgfmtcmd" != x ]; then
    msgfmtcmd=$(which msgfmt 2>/dev/null)
    rc=$?
    if [ $rc -ne 0 -o "x$msgfmtcmd" != x ]; then
      # maybe the which command is not there...
      # try some common spots
      if [ -f /usr/bin/xmsgfmt ]; then
        msgfmtcmd=xmsgfmt
      fi
      if [ "x$msgfmtcmd" = x -f /usr/bin/msgfmt ]; then
        msgfmtcmd=msgfmt
      fi
    fi
  fi
fi

for i in *.po; do
  j=`echo $i | sed 's,\\.po$,,'`
  ${msgfmtcmd} -o $j.mo $i 2> /dev/null
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
