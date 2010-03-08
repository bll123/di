#!/bin/sh
#
#  Copyright 2010 Brad Lanam Walnut Creek, CA USA
#

echo ${EN} "di w/argument${EC}" >&3

# most all unix
../di -n -f M / 2>/dev/null | grep '^/[ ]*$' > /dev/null 2>&1
rc=$?
if [ $rc -ne 0 ]; then
  # cygwin
  ../di -n -f M / 2>/dev/null | grep '^C:\\[ ]*$' > /dev/null 2>&1
  rc=$?
  if [ $rc -ne 0 ]; then
    # other machines w/odd setup
    ../di -n -f M /boot 2>/dev/null | grep '^/boot[ ]*$' > /dev/null 2>&1
    rc=$?
  fi
fi
exit $rc
