#!/bin/sh
#
#  Copyright 2010 Brad Lanam Walnut Creek, CA USA
#

echo ${EN} "compare mkconfig.sh mkconfig.pl${EC}" >&3

cd ..
grc=0
make distclean
rm -f config.h.sh config.h.pl cache.pl cache.sh vars.pl vars.sh
./Build -mksh config.h
mv config.h config.h.sh
cat mkconfig.cache | sort > cache.sh
cat mkconfig.vars > vars.sh
rm -f mkconfig.cache
./Build -mkpl config.h
mv config.h config.h.pl
cat mkconfig.cache | sort > cache.pl
cat mkconfig.vars > vars.pl
echo "##== diff config.h"
diff -w config.h.sh config.h.pl
rc=$?
if [ $rc -ne 0 ]; then grc=$rc; fi
echo "##== diff cache"
diff -w cache.sh cache.pl
rc=$?
if [ $rc -ne 0 ]; then grc=$rc; fi
echo "##== diff vars"
diff -w vars.sh vars.pl
rc=$?
if [ $rc -ne 0 ]; then grc=$rc; fi
rm -f config.h.sh config.h.pl cache.pl cache.sh vars.pl vars.sh

exit $grc
