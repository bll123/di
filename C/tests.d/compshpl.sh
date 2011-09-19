#!/bin/sh
#
#  Copyright 2010 Brad Lanam Walnut Creek, CA USA
#

if [ "$1" = "-d" ]; then
  echo ${EN} " compare mkconfig.sh mkconfig.pl${EC}"
  exit 0
fi

cd $_MKCONFIG_RUNTOPDIR
grc=0

make realclean
rm -f config.h.sh config.h.pl cache.pl cache.sh \
    vars.pl vars.sh

make -e config.h
mv config.h config.h.sh
grep -v '^mkc_env' mkconfig.cache | sort > cache.sh
sort mkconfig_c.vars > vars.sh
mv mkconfig.cache $_MKCONFIG_TSTRUNTMPDIR/mkconfig_sh.cache
mv mkconfig_c.vars $_MKCONFIG_TSTRUNTMPDIR/mkconfig_c_sh.vars
mv mkconfig_env.vars $_MKCONFIG_TSTRUNTMPDIR/mkconfig_env_sh.vars
mv mkconfig.log $_MKCONFIG_TSTRUNTMPDIR/mkconfig_sh.log
mv di.env $_MKCONFIG_TSTRUNTMPDIR/di_sh.env

make -e MKCONFIG_TYPE=perl config.h
mv config.h config.h.pl
sort mkconfig.cache > cache.pl
sort mkconfig_c.vars > vars.pl
mv mkconfig.cache $_MKCONFIG_TSTRUNTMPDIR/mkconfig_pl.cache
mv mkconfig_c.vars $_MKCONFIG_TSTRUNTMPDIR/mkconfig_c_pl.vars
mv mkconfig.log $_MKCONFIG_TSTRUNTMPDIR/mkconfig_pl.log
mv di.env $_MKCONFIG_TSTRUNTMPDIR/di_pl.env

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
