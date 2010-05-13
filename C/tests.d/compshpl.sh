#!/bin/sh
#
#  Copyright 2010 Brad Lanam Walnut Creek, CA USA
#

echo ${EN} "compare mkconfig.sh mkconfig.pl${EC}" >&5

cd $_MKCONFIG_RUNTOPDIR
grc=0

make distclean
rm -f config.h.sh config.h.pl cache.pl cache.sh vars.pl vars.sh

make -e config.h
mv config.h config.h.sh
cat mkconfig.cache | grep -v '^di_env' | sort > cache.sh
mv mkconfig_c.vars vars.sh
mv mkconfig.log $_MKCONFIG_TSTRUNTMPDIR/mkconfig_sh.log
mv di.env $_MKCONFIG_TSTRUNTMPDIR/di_sh.env
mv mkconfig.cache $_MKCONFIG_TSTRUNTMPDIR/mkconfig_sh.cache

make -e MKCONFIG_TYPE=perl config.h
mv config.h config.h.pl
cat mkconfig.cache | sort > cache.pl
mv mkconfig_c.vars vars.pl
mv mkconfig.log $_MKCONFIG_TSTRUNTMPDIR/mkconfig_pl.log
mv di.env $_MKCONFIG_TSTRUNTMPDIR/di_pl.env
mv mkconfig.cache $_MKCONFIG_TSTRUNTMPDIR/mkconfig_pl.cache

echo "## diff config.h.sh config.h.pl"
diff -w config.h.sh config.h.pl
rc=$?
if [ $rc -ne 0 ]; then grc=$rc; fi
mv config.h.sh config.h.pl $_MKCONFIG_TSTRUNTMPDIR

echo "## diff cache.sh cache.pl"
diff -w cache.sh cache.pl
rc=$?
if [ $rc -ne 0 ]; then grc=$rc; fi
mv cache.sh cache.pl $_MKCONFIG_TSTRUNTMPDIR

echo "## diff vars.sh vars.pl"
diff -w vars.sh vars.pl
rc=$?
if [ $rc -ne 0 ]; then grc=$rc; fi
mv vars.sh vars.pl $_MKCONFIG_TSTRUNTMPDIR

exit $grc
