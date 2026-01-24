#!/bin/sh
#
# Copyright 2025 Brad Lanam Pleasant Hill CA
#

BUILDDIR=$1
PODIR=$2
INST_LOCALEDIR=$3
TMPPODIR=$4

DEST_LOCALEDIR="${DESTDIR}${INST_LOCALEDIR}"

if [ "x${BUILDDIR}" = x ]; then
  echo "instpo.sh: No build dir specified"
  exit 1
fi

if [ ! -f "${BUILDDIR}/diconfig.h" ]; then
  echo "instpo.sh: Could not locate diconfig.h"
  exit 1
fi

#   diconfig.h:
#   #define _enable_nls 1
grep -l '^#define _enable_nls 1' ${BUILDDIR}/diconfig.h >/dev/null 2>&1
rc=$?
if [ $rc -ne 0 ]; then
  exit 0
fi

if [ "x${PODIR}" = x ]; then
  echo "instpo.sh: No po/ dir specified"
  exit 1
fi

if [ ! -d "${PODIR}" ]; then
  echo "instpo.sh: po/ dir does not exist"
  exit 1
fi

if [ "x${INST_LOCALEDIR}" = x ]; then
  echo "instpo.sh: No locale dir specified"
  exit 1
fi

if [ "x${TMPPODIR}" = x ]; then
  echo "instpo.sh: No temporary dir specified"
  exit 1
fi

msgfmtcmd=`which gmsgfmt 2>/dev/null`
rc=$?
if [ $rc -ne 0 -o "x$msgfmtcmd" = x ]; then
  msgfmtcmd=`which msgfmt 2>/dev/null`
  rc=$?
  if [ $rc -ne 0 -o "x$msgfmtcmd" = x ]; then
    # maybe the which command is not there...
    # try some common spots
    if [ "x$msgfmtcmd" = x -a -f /usr/bin/msgfmt ]; then
      msgfmtcmd=/usr/bin/msgfmt
    fi
    if [ "x$msgfmtcmd" = x -a -f /opt/local/bin/msgfmt ]; then
      msgfmtcmd=/opt/local/bin/msgfmt
    fi
    if [ "x$msgfmtcmd" = x -a -f /opt/homebrew/bin/msgfmt ]; then
      msgfmtcmd=/opt/homebrew/bin/msgfmt
    fi
    if [ "x$msgfmtcmd" = x -a -f /usr/local/bin/msgfmt ]; then
      msgfmtcmd=/usr/local/bin/msgfmt
    fi
  fi
fi

if [ "x$msgfmtcmd" = x ]; then
  echo "instpo.sh: Unable to locate msgfmt command"
  exit 1
fi

cd "${PODIR}"
rc=$?
if [ $rc -ne 0 ]; then
  echo "No po/ directory."
  exit 0
fi

test -d "${DEST_LOCALEDIR}" || mkdir -p "${DEST_LOCALEDIR}"
test -d "${TMPPODIR}" || mkdir -p "${TMPPODIR}"

for i in *.po; do
  j=`echo $i | sed 's,\\.po$,,'`
  ${msgfmtcmd} -o "${TMPPODIR}/$j.mo" $i 2> /dev/null
  rc=$?
  if [ $rc -ne 0 ]; then
    echo "instpo.sh: msgfmt failed for $i"
    continue
  fi
  if [ ! -f "${TMPPODIR}/$j.mo" ]; then
    continue
  fi
  test -d "${DEST_LOCALEDIR}/$j/LC_MESSAGES" ||
      mkdir -p "${DEST_LOCALEDIR}/$j/LC_MESSAGES"
  cp -pf "${TMPPODIR}/$j.mo" "${DEST_LOCALEDIR}/$j/LC_MESSAGES/di.mo"
  rm -f "${TMPPODIR}/$j.mo"
  echo "-- Installing: ${DEST_LOCALEDIR}/$j/LC_MESSAGES/di.mo"
done

exit 0
