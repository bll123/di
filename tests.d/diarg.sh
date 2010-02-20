#!/bin/sh

echo ${EN} "di w/argument${EC}" >&3

cd ..
./di -n -f M / 2>/dev/null | egrep '^/[ ]*$' > /dev/null 2>&1
rc=$?
if [ $rc -ne 0 -o $val != "/" ]; then
  ./di -n -f M C: | egrep '^C:\\[ ]*$' > /dev/null 2>&1
  rc=$?
  if [ $rc -ne 0 ]; then
    ./di -n -f M /boot 2>/dev/null | egrep '^/boot[ ]*$' > /dev/null 2>&1
    rc=$?
  fi
fi
exit $rc
