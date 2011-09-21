#!/bin/sh

. $_MKCONFIG_DIR/testfuncs.sh

maindodisplay $1 'run di w/arguments'
maindoquery $1 $_MKC_ONCE

getsname $0
dosetup $@

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
