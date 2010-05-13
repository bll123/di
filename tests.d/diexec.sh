#!/bin/sh
#
#  Copyright 2010 Brad Lanam Walnut Creek, CA USA
#

echo ${EN} "di executes${EC}" >&5

$_MKCONFIG_RUNTOPDIR/di
rc=$?
exit $rc
