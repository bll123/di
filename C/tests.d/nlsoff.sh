#!/bin/sh

echo ${EN} "check turnoffnls.sh${EC}" >&5

cat > config.h <<_HERE_
#define _lib_bindtextdomain 1
#define _lib_gettext 1
#define _lib_setlocale 1
#define _lib_textdomain 1
#define _hdr_libintl 1
#define _hdr_locale 1
#define _command_msgfmt 1
_HERE_

cat > config.tmp <<_HERE_
#define _lib_bindtextdomain 0
#define _lib_gettext 0
#define _lib_setlocale 0
#define _lib_textdomain 0
#define _hdr_libintl 0
#define _hdr_locale 0
#define _command_msgfmt 0
_HERE_

$_MKCONFIG_RUNTOPDIR/features/turnoffnls.sh
rc=$?

if [ $rc -eq 0 ]; then
  diff -b config.h config.tmp
  rc=$?
fi
exit $rc
