#!/bin/sh
#
#  Copyright 2010 Brad Lanam Walnut Creek, CA USA
#

echo ${EN} "compare mkconfig.sh mkconfig.pl${EC}" >&3

cd ..
grc=0
make distclean
rm -f config.h.sh config.h.pl cache.pl cache.sh vars.pl vars.sh
make -e config.h
mv config.h config.h.sh
cat mkconfig.cache | sort > cache.sh
cat mkconfig_c.vars > vars.sh
rm -f mkconfig.cache
make -e MKCONFIG_TYPE=perl config.h
mv config.h config.h.pl
cat mkconfig.cache | sort > cache.pl
cat mkconfig_c.vars > vars.pl
echo "##== diff config.h.sh config.h.pl"
diff -w config.h.sh config.h.pl
rc=$?
if [ $rc -ne 0 ]; then grc=$rc; fi
echo "##== diff cache.sh cache.pl"
diff -w cache.sh cache.pl
rc=$?
if [ $rc -ne 0 ]; then grc=$rc; fi
echo "##== diff vars.sh vars.pl"
diff -w vars.sh vars.pl
rc=$?
if [ $rc -ne 0 ]; then grc=$rc; fi
rm -f config.h.sh config.h.pl cache.pl cache.sh vars.pl vars.sh

exit $grc
