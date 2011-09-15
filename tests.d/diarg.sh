#!/bin/sh
#
#  Copyright 2010 Brad Lanam Walnut Creek, CA USA
#

if [ "$1" = "-d" ]; then
  echo ${EN} " di w/argument${EC}"
  exit 0
fi

unset DI_ARGS
unset DIFMT
# most all unix
$_MKCONFIG_RUNTOPDIR/di -n -f M / 2>/dev/null | grep '^/[ ]*$' > /dev/null 2>&1
rc=$?
if [ $rc -ne 0 ]; then
  # cygwin
  $_MKCONFIG_RUNTOPDIR/di -n -f M / 2>/dev/null | grep '/usr/bin$' > /dev/null 2>&1
  rc=$?
  if [ $rc -ne 0 ]; then
    # cygwin
    $_MKCONFIG_RUNTOPDIR/di -n -f M / 2>/dev/null | grep '^C:\\[ ]*$' > /dev/null 2>&1
    rc=$?
    if [ $rc -ne 0 ]; then
      # other machines w/odd setup
      $_MKCONFIG_RUNTOPDIR/di -n -f M /boot 2>/dev/null | grep '^/boot[ ]*$' > /dev/null 2>&1
      rc=$?
    fi
  fi
fi
exit $rc
