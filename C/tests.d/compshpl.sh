#!/bin/sh

echo ${EN} "compare mkconfig.sh mkconfig.pl${EC}" >&3

cd ..
grc=0
make distclean
rm -f config.h.sh config.h.pl cache.pl cache.sh
./Build -mksh config.h
mv config.h config.h.sh
cat mkconfig.cache | sort > cache.sh
rm -f mkconfig.cache
./Build -mkpl config.h
mv config.h config.h.pl
cat mkconfig.cache | sort > cache.pl
diff -w config.h.sh config.h.pl
rc=$?
if [ $rc -ne 0 ]; then grc=$rc; fi
diff -w cache.sh cache.pl
rc=$?
if [ $rc -ne 0 ]; then grc=$rc; fi
rm -f config.h.sh config.h.pl cache.pl cache.sh

exit $grc
