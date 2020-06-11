#!/bin/sh

. $_MKCONFIG_DIR/bin/testfuncs.sh

maindodisplay $1 'compare mkconfig.sh mkconfig.pl'
maindoquery $1 $_MKC_ONCE

getsname $0
dosetup $@

perl -v > /dev/null 2>&1
rc=$?
if [ $rc -ne 0 ] ; then
  exit 166
fi

cd $_MKCONFIG_RUNTOPDIR
grc=0

unset MAKEFLAGS
${MAKE:-make} ${TMAKEFLAGS} realclean
rm -f config.h.sh config.h.pl cache.pl cache.sh \
    vars.pl vars.sh

${MAKE:-make} ${TMAKEFLAGS} -e config.h
sed -e '/Created on: /,/Using: mkc/d' config.h > config.h.sh
rm -f config.h
grep -v '^mkc_env' ${MKC_FILES}/mkconfig.cache | sort > cache.sh
sort ${MKC_FILES}/mkc_config_c.vars > vars.sh
mv ${MKC_FILES}/mkconfig.cache ${_MKCONFIG_TSTRUNTMPDIR}/mkconfig_sh.cache
mv ${MKC_FILES}/mkc_config_c.vars ${_MKCONFIG_TSTRUNTMPDIR}/mkc_config_c_sh.vars
mv ${MKC_FILES}/mkc_di_env.vars ${_MKCONFIG_TSTRUNTMPDIR}/mkc_di_env.vars
mv ${MKC_FILES}/mkconfig.log ${_MKCONFIG_TSTRUNTMPDIR}/mkconfig_sh.log
mv di.env ${_MKCONFIG_TSTRUNTMPDIR}/di_sh.env

${MAKE:-make} ${TMAKEFLAGS} -e MKCONFIG_TYPE=perl config.h
sed -e '/Created on: /,/Using: mkc/d' config.h > config.h.pl
rm -f config.h
sort ${MKC_FILES}/mkconfig.cache > cache.pl
sort ${MKC_FILES}/mkc_config_c.vars > vars.pl
mv ${MKC_FILES}/mkconfig.cache $_MKCONFIG_TSTRUNTMPDIR/mkconfig_pl.cache
mv ${MKC_FILES}/mkc_config_c.vars $_MKCONFIG_TSTRUNTMPDIR/mkc_config_c_pl.vars
mv ${MKC_FILES}/mkconfig.log $_MKCONFIG_TSTRUNTMPDIR/mkconfig_pl.log
mv di.env $_MKCONFIG_TSTRUNTMPDIR/di_pl.env

chkdiff config.h.sh config.h.pl
mv config.h.sh config.h.pl $_MKCONFIG_TSTRUNTMPDIR

chkdiff cache.sh cache.pl
mv cache.sh cache.pl $_MKCONFIG_TSTRUNTMPDIR

chkdiff vars.sh vars.pl
mv vars.sh vars.pl $_MKCONFIG_TSTRUNTMPDIR

exit $grc
