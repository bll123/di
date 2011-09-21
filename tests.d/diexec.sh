#!/bin/sh

. $_MKCONFIG_DIR/testfuncs.sh

maindodisplay $1 'execute di'
maindoquery $1 $_MKC_SH

getsname $0
dosetup $@

unset DI_ARGS
unset DIFMT
$_MKCONFIG_RUNTOPDIR/di
rc=$?
exit $rc
