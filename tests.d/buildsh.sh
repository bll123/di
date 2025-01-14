#!/bin/sh

. $_MKCONFIG_DIR/bin/testfuncs.sh

maindodisplay $1 'build w/mkconfig.sh'
maindoquery $1 $_MKC_SH

getsname $0
dosetup $@

. $_MKCONFIG_DIR/bin/shellfuncs.sh
testshcapability

for d in C D; do
  if [ $d = D -a \( "$DC" = "" -o "$DC" = "skip" \) ]; then
    continue
  fi
  tdir=$_MKCONFIG_RUNTOPDIR/$d
  if [ -d ${tdir} ]; then
    cd $tdir
    instdir="`pwd`/test_di"
    unset MAKEFLAGS

    if [ $d = D -a "$DC" != "" ]; then
      ${MAKE:-make} ${TMAKEFLAGS} realclean
      ${MAKE:-make} ${TMAKEFLAGS} -e prefix=${instdir} di.env > env.make.log 2>&1
      rc=$?
      if [ $rc -ne 0 -o ! -f ./di.env ]; then
        test -f env.make.log && rm -f env.make.log
        continue;
      fi
      (
        . ./di.env
        if [ "${DVERSION}" = "" -o "${DVERSION}" = "1" ]; then
          exit 1
        fi
        exit 0
      )
      rc=$?
      if [ $rc -ne 0 ]; then
        test -f env.make.log && rm -f env.make.log
        continue
      fi
    fi

    putsnonl " ${d}" >&5
    ${MAKE:-make} ${TMAKEFLAGS} realclean
    ${MAKE:-make} ${TMAKEFLAGS} -e prefix=${instdir} all > make.log 2>&1
    rc=$?
    if [ $rc != 0 ]; then grc=$rc; fi

    if [ $grc -eq 0 ]; then
      systype=`uname -s`
      # haiku has too many warnings to deal with
      if [ "$systype" != "Haiku" ]; then
        # C:
        #  openbsd: malloc.h, always misused, in file included, in function
        #  openbsd: unfortunately, some output is on separate lines.
        #  openbsd: warning: strcat() is almost always misused, please use strlcat()
        #  solaris: tokens ignored... (dollar sign)
        #  solaris: Function has no return statement : main
        #  freebsd 4.9: from xxx.c:66: (separate line)
        #  freebsd 4.9: /usr/include....function declaration isn't a prototype
        #  AIX: extra msg from make: Target <target> is up to date
        #  Tru64: complains about long long being a new feature.
        #  Tru64: extra ----^ lines
        #  Tru64:(null command) - not sure where that's coming from
        #  SCO_SV:
        #  SCO_SV: warning: `/*' within comment
        #  clang: 'warning: unknown warning option'
        #  clang: X warnings? generated.
        #  many: unrecognized #pragma ignored
        #  miros:gcc: someone does not honour COPTS correctly
        #  gcc:fstack-protector not supported for this target
        # D:
        #  FLAGS= (makefile)
        #  --     (makefile)
        cat make.log |
          grep -v 'dioptions.dat' |
          grep -v '^load\-unit:' |
          grep -v '^output\-file:' |
          grep -v '^option\-file:' |
          grep -v ' \.\.\. ' |
          grep -vi mkconfig |
          grep -v reqlibs |
          grep -v 'Leaving directory' |
          grep -v 'Entering directory' |
          grep -v 'cc \-[co]' |
          grep -v 'cc32 \-[co]' |
          grep -v 'cc64 \-[co]' |
          grep -v 'clang \-[co]' |
          grep -v 'xlc \-[co]' |
          grep -v digetentries.o |
          grep -v '\*\*\* ' |
          grep -v 'di\.env' |
          grep -v '^CC=' |
          grep -v '#warning' |
          grep -v 'strcpy.*always misused' |
          grep -v 'strcat.*always misused' |
          grep -v '^In file included' |
          grep -v ': In function' |
          grep -v 'tokens ignored at end of directive line' |
          grep -v 'Function has no return statement : main' |
          grep -v '^ *from [^ ]*.[ch]:[0-9]*[:,]$' |
          grep -v '/usr/include.*function declaration.*a prototype' |
          grep -v '^Target.*is up to date' |
          grep -v '^------' |
          grep -v 'is a new feature' |
          grep -v '^ *typedef.* long long ' |
          grep -v 'warning: .... within comment' |
          grep -v 'Unknown option.*-Wextra.*ignored.' |
          grep -v 'Unknown option.*-Wno-unused-but-set-variable.*ignored' |
          grep -v 'warning: unknown warning option' |
          grep -v 'warning.*generated\.' |
          grep -v 'Unknown option.*-Wno-unused-parameter.*ignored' |
          grep -v 'unrecognized #pragma ignored' |
          grep -v 'someone does not honour COPTS correctly' |
          grep -v 'fstack-protector not supported for this target' |
          grep -v FLAGS= |
          grep -v '(null command)' |
          grep -v -- '--' |
          cat > make_extra.log
        extra=`cat make_extra.log | wc -l | sed -e 's/^ *//'`
        if [ $extra -ne 0 ]; then
          puts "## extra output"
          grc=1
        fi
      fi # not Haiku
    fi # if the build worked
  fi

  mkdir -p $_MKCONFIG_TSTRUNTMPDIR/buildsh_${d}${stag}
  set +f
  for f in mkconfig.log mkconfig.cache mkc*.vars di.env di.reqlibs \
      env.make.log make.log make_extra.log; do
    if [ -f $f ]; then
      mv $f $_MKCONFIG_TSTRUNTMPDIR/buildsh_${d}${stag}
    fi
  done
  set -f
done

exit $grc

