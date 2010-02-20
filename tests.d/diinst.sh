#!/bin/sh

echo ${EN} "di install${EC}" >&3

instdir="test_di"

cd ..
grc=0
prefix=${instdir} ./Build -mksh install
rc=$?
if [ $rc -ne 0 ]; then grc=$rc; fi
${instdir}/bin/di
rc=$?
if [ $rc -ne 0 ]; then grc=$rc; fi
rm -rf test_di 

exit $grc
