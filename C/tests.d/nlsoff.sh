#!/bin/sh

echo ${EN} "check NLS option${EC}" >&5

> config.h echo '
#define _lib_bindtextdomain 1
#define _lib_gettext 1
#define _lib_setlocale 1
#define _lib_textdomain 1
#define _hdr_libintl 1
#define _hdr_locale 1
#define _command_msgfmt 1
'

> config.tmp echo '
#define _lib_bindtextdomain 0
#define _lib_gettext 0
#define _lib_setlocale 0
#define _lib_textdomain 0
#define _hdr_libintl 0
#define _hdr_locale 0
#define _command_msgfmt 0
'

${_MKCONFIG_DIR}/mkpostconf.sh -i ${_MKCONFIG_RUNTOPDIR}/features/options.dat \
    config.h disable NLS
rc=$?

if [ $rc -eq 0 ]; then
  diff -w config.h config.tmp
  rc=$?
fi
exit $rc
