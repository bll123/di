#!/bin/sh

echo ${EN} "di build w/mkconfig.pl${EC}" >&3

cd ..
make distclean
instdir="test_di"
prefix=${instdir} ./Build -mkpl 
rc=$?

exit $rc
