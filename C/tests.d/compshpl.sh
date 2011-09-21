#!/bin/sh

. $_MKCONFIG_DIR/testfuncs.sh

maindodisplay $1 'compare mkconfig.sh mkconfig.pl'
maindoquery $1 $_MKC_SH

getsname $0
dosetup $@

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

chkdiff config.h.sh config.h.pl
mv config.h.sh config.h.pl $_MKCONFIG_TSTRUNTMPDIR

chkdiff cache.sh cache.pl
mv cache.sh cache.pl $_MKCONFIG_TSTRUNTMPDIR

chkdiff vars.sh vars.pl
mv vars.sh vars.pl $_MKCONFIG_TSTRUNTMPDIR

exit $grc
