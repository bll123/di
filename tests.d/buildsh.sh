#!/bin/sh

echo ${EN} "build w/mkconfig.sh${EC}" >&3

cd ..
make distclean
instdir="`pwd`/test_di"
prefix=${instdir} LOCALEDIR=${prefix}/share/locale ./Build -mksh 
rc=$?

exit $rc
