#!/bin/sh

echo ${EN} "di w/argument${EC}" >&3

# most all unix
../di -n -f M / 2>/dev/null | egrep '^/[ ]*$' > /dev/null 2>&1
rc=$?
if [ $rc -ne 0 ]; then
  # cygwin
  ../di -n -f M / 2>/dev/null | egrep '^C:\\[ ]*$' > /dev/null 2>&1
  rc=$?
  if [ $rc -ne 0 ]; then
    # other machines w/odd setup
    ../di -n -f M /boot 2>/dev/null | egrep '^/boot[ ]*$' > /dev/null 2>&1
    rc=$?
  fi
fi
exit $rc
