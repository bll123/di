#!/bin/sh
#
#  Copyright 2010 Brad Lanam Walnut Creek, CA USA
#

echo ${EN} "di executes${EC}" >&3

../di
rc=$?
exit $rc
