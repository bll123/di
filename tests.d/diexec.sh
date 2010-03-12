#!/bin/sh
#
#  Copyright 2010 Brad Lanam Walnut Creek, CA USA
#

echo ${EN} "di executes${EC}" >&3

$RUNTOPDIR/di
rc=$?
exit $rc
