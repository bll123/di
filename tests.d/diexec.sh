#!/bin/sh
#
#  Copyright 2010 Brad Lanam Walnut Creek, CA USA
#

if [ "$1" = "-d" ]; then
  echo ${EN} " di executes${EC}"
  exit 0
fi

unset DI_ARGS
unset DIFMT
$_MKCONFIG_RUNTOPDIR/di
rc=$?
exit $rc
