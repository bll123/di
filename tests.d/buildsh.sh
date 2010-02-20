#!/bin/sh

echo ${EN} "di build w/mkconfig.sh${EC}" >&3

cd ..
make distclean
instdir="test_di"
prefix=${instdir} ./Build -mksh 
rc=$?

exit $rc
