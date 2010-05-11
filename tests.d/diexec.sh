#!/bin/sh
#
#  Copyright 2010 Brad Lanam Walnut Creek, CA USA
#

echo ${EN} "di executes${EC}" >&3

$_MKCONFIG_RUNTOPDIR/di
rc=$?
exit $rc
