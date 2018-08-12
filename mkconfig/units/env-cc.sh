#!/bin/sh
#
# Copyright 2001-2018 Brad Lanam, Walnut Creek, California USA
#

#
# speed at the cost of maintainability...
# File Descriptors:
#    9 - >>$LOG                     (mkconfig.sh)
#    8 - >>$VARSFILE, >>$CONFH      (mkconfig.sh)
#    7 - temporary for mkconfig.sh  (mkconfig.sh)
#    6 - temporary for c-main.sh    (c-main.sh)
#    5 - temporary for c-main.sh    (c-main.sh)
#

require_unit env-main
require_unit env-systype

env_dogetconf=F
_MKCONFIG_32BIT_FLAGS=F

# helper routine
_setflags () {
  while test $# -gt 0; do
    _tvar=$1
    _tenm=$2
    shift;shift
    eval _tval=\$$_tvar
    dosubst _tval '^ *' ''
    setdata ${_MKCONFIG_PREFIX} ${_tenm} "$_tval"
  done
}

_dogetconf () {
  if [ "$env_dogetconf" = T ]; then
    return
  fi
  if [ ${_MKCONFIG_32BIT_FLAGS} = T ]; then
    lfccflags=
    lfldflags=
    lflibs=
    env_dogetconf=T
    return
  fi

  locatecmd xgetconf getconf
  if [ z${xgetconf} != z ]
  then
      puts "using flags from getconf" >&9
      lfccflags="`${xgetconf} LFS_CFLAGS 2>/dev/null`"
      if [ "$lfccflags" = undefined ]; then
        lfccflags=
      fi
      lfldflags="`${xgetconf} LFS_LDFLAGS 2>/dev/null`"
      if [ "$lfldflags" = undefined ]; then
        lfldflags=
      fi
      lflibs="`${xgetconf} LFS_LIBS 2>/dev/null`"
      if [ "$lflibs" = undefined ]; then
        lflibs=
      fi
  fi
  env_dogetconf=T
}

check_32bitflags () {
  _MKCONFIG_32BIT_FLAGS=T

  printlabel _MKCONFIG_32BIT_FLAGS "32 bit flags"
  printyesno_val _MKCONFIG_32BIT_FLAGS "${_MKCONFIG_32BIT_FLAGS}"
  setdata ${_MKCONFIG_PREFIX} _MKCONFIG_32BIT_FLAGS "${_MKCONFIG_32BIT_FLAGS}"
}

check_cc () {
  _read_option CC cc
  CC=${CC:-${oval}}

  printlabel CC "C compiler"

  case ${_MKCONFIG_SYSTYPE} in
      BeOS|Haiku)
        case ${CC} in
          cc|gcc)
            CC=g++
            ;;
        esac
        ;;
      syllable)
        case ${CC} in
          cc|gcc)
            CC=g++
            ;;
        esac
        ;;
  esac

  puts "cc:${CC}" >&9

  printyesno_val CC "${CC}"
  setdata ${_MKCONFIG_PREFIX} CC "${CC}"
  if [ ${_MKCONFIG_32BIT_FLAGS} = F ]; then
    setdata ${_MKCONFIG_PREFIX} _MKCONFIG_32BIT_FLAGS "${_MKCONFIG_32BIT_FLAGS}"
  fi
}

check_using_gcc () {
  usinggcc="N"

  printlabel _MKCONFIG_USING_GCC "Using gcc/g++"

  # check for gcc...
  ${CC} -v 2>&1 | grep 'gcc version' > /dev/null 2>&1
  rc=$?
  if [ $rc -eq 0 ]; then
      puts "found gcc" >&9
      usinggcc="Y"
  fi

  case ${CC} in
      *gcc*|*g++*)
          usinggcc="Y"
          ;;
  esac

  printyesno_val _MKCONFIG_USING_GCC "${usinggcc}"
  setdata ${_MKCONFIG_PREFIX} _MKCONFIG_USING_GCC "${usinggcc}"
}

check_using_clang () {
  usingclang="N"

  printlabel _MKCONFIG_USING_CLANG "Using clang"

  # check for clang...
  ${CC} -v 2>&1 | grep 'clang version' > /dev/null 2>&1
  rc=$?
  if [ $rc -eq 0 ]; then
      puts "found clang" >&9
      usingclang="Y"
  fi

  case ${CC} in
      *clang*)
          usingclang="Y"
          ;;
  esac

  printyesno_val _MKCONFIG_USING_CLANG "${usingclang}"
  setdata ${_MKCONFIG_PREFIX} _MKCONFIG_USING_CLANG "${usingclang}"
}

check_using_gnu_ld () {
  usinggnuld="N"

  printlabel _MKCONFIG_USING_GNU_LD "Using gnu ld"

  # check for gnu ld...
  ld -v 2>&1 | grep 'GNU ld' > /dev/null 2>&1
  rc=$?
  if [ $rc -eq 0 ]; then
      puts "found gnu ld" >&9
      usinggnuld="Y"
  fi

  printyesno_val _MKCONFIG_USING_GNU_LD "${usinggnuld}"
  setdata ${_MKCONFIG_PREFIX} _MKCONFIG_USING_GNU_LD "${usinggnuld}"
}

check_cflags () {
  cflags_debug=
  cflags_optimize="-O2"
  cflags_include=
  cflags_user=
  cflags_compiler=
  cflags_system=
  cflags_application=

  if [ "${_MKCONFIG_USING_GCC}" = Y ]; then
    puts "set gcc flags" >&9
    gccflags="-Wall -Waggregate-return -Wconversion -Wformat -Wmissing-prototypes -Wmissing-declarations -Wnested-externs -Wpointer-arith -Wshadow -Wstrict-prototypes -Wunused -Wno-unknown-pragmas"
    # -Wextra -Wno-unused-but-set-variable -Wno-unused-parameter
    case ${CC} in
      g++|c++)
        if [ "${_MKCONFIG_USING_GCC}" = Y ]; then
          puts "set g++ flags" >&9
          gccflags="-Wall -Waggregate-return -Wconversion -Wformat -Wpointer-arith -Wshadow -Wunused"
        fi
        ;;
    esac
    doappend cflags_compiler " $gccflags"
  fi
  if [ "${_MKCONFIG_USING_CLANG}" = Y ]; then
    puts "set clang flags" >&9
    doappend cflags_compiler " -Wno-unknown-warning-option -Weverything -Wno-padded -Wno-format-nonliteral -Wno-cast-align -Wno-system-headers -Wno-disabled-macro-expansion"
  fi

  TCC=${CC}
  if [ "${_MKCONFIG_USING_GCC}" = Y ]; then
    TCC=gcc
  fi

  case ${_MKCONFIG_SYSTYPE} in
      AIX)
        if [ "${_MKCONFIG_USING_GCC}" = N ]; then
          doappend cflags_system " -qhalt=e -qmaxmem=-1"
          case ${_MKCONFIG_SYSREV} in
            4.*)
              doappend cflags_system " -DUSE_ETC_FILESYSTEMS=1"
              ;;
          esac
        fi
        ;;
      FreeBSD)
        # FreeBSD has many packages that get installed in /usr/local
        doappend cflags_include " -I/usr/local/include"
        ;;
      HP-UX)
        if [ "z${lfccflags}" = z -a "${_MKCONFIG_32BIT_FLAGS}" = F ]; then
          doappend cflags_system " -D_LARGEFILE64_SOURCE -D_FILE_OFFSET_BITS=64"
        fi
        case ${TCC} in
          cc)
            case ${_MKCONFIG_SYSREV} in
              *.10.*)
                doappend cflags_system " +DAportable"
                ;;
            esac
            cc -v 2>&1 | grep -l Bundled > /dev/null 2>&1
            rc=$?
            if [ $rc -ne 0 ]; then
              doappend cflags_system " -Ae"
            fi
            _MKCONFIG_USING_GCC=N
            ;;
        esac

        if [ -d /usr/local/include -a \
            -d /usr/local/lib ]; then
          doappend cflags_include " -I/usr/local/include"
        fi
        ;;
      SunOS)
        case ${_MKCONFIG_SYSREV} in
          5.*)
            case ${TCC} in
              cc)
                # If solaris is compile w/strict ansi, we get
                # a work-around for the long long type with
                # large files.  So we compile w/extensions.
                doappend cflags_system " -Xa -v"
                # optimization; -xO3 is good. -xO4 must be set by user.
                cflags_optimize="-xO3"
                ;;
            esac
            ;;
        esac
        ;;
  esac

  # largefile flags
  doappend cflags_system " $lfccflags"

  _dogetconf

  # plain CFLAGS will be interpreted as the user's cflags
  _read_option CFLAGS ""
  cflags_user=$CFLAGS
  _read_option CFLAGS_DEBUG ""
  if [ "z$CFLAGS_DEBUG" != z ]; then
    cflags_debug="$CFLAGS_DEBUG"
  fi
  _read_option CFLAGS_OPTIMIZE ""
  if [ "z$CFLAGS_OPTIMIZE" != z ]; then
    cflags_optimize="$CFLAGS_OPTIMIZE"
  fi
  _read_option CFLAGS_INCLUDE ""
  doappend cflags_include " $CFLAGS_INCLUDE"
  doappend cflags_include " $CPPFLAGS"

  puts "cflags_debug:${cflags_debug}" >&9
  puts "cflags_optimize:${cflags_optimize}" >&9
  puts "cflags_include:${cflags_include}" >&9
  puts "cflags_user:${cflags_user}" >&9
  puts "cflags_compiler:${cflags_compiler}" >&9
  puts "cflags_system:${cflags_system}" >&9
  puts "cflags_application:${cflags_application}" >&9

  _setflags \
      cflags_debug CFLAGS_DEBUG \
      cflags_optimize CFLAGS_OPTIMIZE \
      cflags_user CFLAGS_USER \
      cflags_include CFLAGS_INCLUDE \
      cflags_compiler CFLAGS_COMPILER  \
      cflags_system CFLAGS_SYSTEM \
      cflags_application CFLAGS_APPLICATION
}

check_addcflag () {
  name=$1
  flag=$2

  printlabel CFLAGS_APPLICATION "Add C flag: ${flag}"

  puts "#include <stdio.h>
main () { return 0; }" > t.c
  puts "# test ${flag}" >&9
  # need to set w/all cflags; gcc doesn't always error out otherwise
  TMPF=t$$.txt
  setcflags
  ${CC} ${CFLAGS} ${flag} t.c > $TMPF 2>&1
  rc=$?
  if [ $rc -ne 0 ]; then
    flag=0
  fi
  grep -i "warning.*${flag}" $TMPF > /dev/null 2>&1
  rc=$?
  if [ $rc -eq 0 ]; then
    flag=0
  fi
  cat $TMPF >&9
  rm -f $TMPF > /dev/null 2>&1
  printyesno $name ${flag}
  if [ $flag != 0 ]; then
    doappend CFLAGS_APPLICATION " $flag"
    setdata ${_MKCONFIG_PREFIX} CFLAGS_APPLICATION "$CFLAGS_APPLICATION"
  fi
}

check_addldflag () {
  name=$1
  flag=$2

  printlabel LDFLAGS_APPLICATION "Add LD flag: ${flag}"

  setcflags
  setldflags
  setlibs
  puts "#include <stdio.h>
main () { return 0; }" > t.c
  puts "# test ${flag}" >&9
  # need to set w/all cflags/ldflags; gcc doesn't always error out otherwise
  TMPF=t$$.txt
  ${CC} ${CFLAGS} ${LDFLAGS} ${flag} -o t t.c > $TMPF 2>&1
  rc=$?
  if [ $rc -ne 0 ]; then
    flag=0
  fi
  grep -i "warning.*${flag}" $TMPF > /dev/null 2>&1
  rc=$?
  if [ $rc -eq 0 ]; then
    flag=0
  fi
  cat $TMPF >&9
  rm -f $TMPF > /dev/null 2>&1
  printyesno $name ${flag}
  if [ $flag != 0 ]; then
    doappend ldflags_application " $flag"
    _setflags ldflags_application LDFLAGS_APPLICATION
  fi
}

check_ldflags () {
  TCC=${CC}
  if [ "${_MKCONFIG_USING_GCC}" = Y ]; then
    TCC=gcc
  fi

  ldflags_debug=
  ldflags_optimize=
  ldflags_user=
  ldflags_compiler=
  ldflags_system=
  ldflags_application=

  doappend ldflags_system " $lfldflags"

  case ${_MKCONFIG_SYSTYPE} in
      FreeBSD)
        # FreeBSD has many packages that get installed in /usr/local
        doappend ldflags_system " -L/usr/local/lib"
        ;;
      HP-UX)
        # check for libintl in other places...
        if [ -d /usr/local/include -a \
            -d /usr/local/lib ]; then
          doappend ldflags_system " -L/usr/local/lib"
          if [ -d /usr/local/lib/hpux32 ]; then
            doappend ldflags_system " -L/usr/local/lib/hpux32"
          fi
        fi
        case ${TCC} in
          cc)
            doappend ldflags_system " -Wl,+s"
            ;;
        esac
        ;;
      OS/2)
        doappend ldflags_system " -Zexe"
        ;;
      SunOS)
        case ${_MKCONFIG_SYSREV} in
          5.*)
            case ${TCC} in
              cc)
                case $CFLAGS_OPTIMIZE in
                    -xO[3456])
                        ldflags_optimize="-fast"
                        ;;
                esac
                ;;
            esac
            ;;
        esac
        ;;
  esac

  _dogetconf

  # plain LDFLAGS will be interpreted as the user's ldflags
  _read_option LDFLAGS ""
  ldflags_user=$LDFLAGS
  _read_option LDFLAGS_DEBUG ""
  if [ "z$LDFLAGS_DEBUG" != z ]; then
    ldflags_debug="$LDFLAGS_DEBUG"
  fi
  _read_option LDFLAGS_OPTIMIZE ""
  if [ "z$LDFLAGS_OPTIMIZE" != z ]; then
    ldflags_optimize="$LDFLAGS_OPTIMIZE"
  fi

  puts "ldflags_debug:${ldflags_debug}" >&9
  puts "ldflags_optimize:${ldflags_optimize}" >&9
  puts "ldflags_user:${ldflags_user}" >&9
  puts "ldflags_compiler:${ldflags_compiler}" >&9
  puts "ldflags_system:${ldflags_system}" >&9
  puts "ldflags_application:${ldflags_application}" >&9

  _setflags \
      ldflags_debug LDFLAGS_DEBUG \
      ldflags_optimize LDFLAGS_OPTIMIZE \
      ldflags_user LDFLAGS_USER \
      ldflags_compiler LDFLAGS_COMPILER \
      ldflags_system LDFLAGS_SYSTEM \
      ldflags_application LDFLAGS_APPLICATION
}

check_libs () {
  _dogetconf

  ldflags_libs_user=
  ldflags_libs_application=
  ldflags_libs_system=

  TCC=${CC}
  if [ "${_MKCONFIG_USING_GCC}" = Y ]; then
    TCC=gcc
  fi

  case ${_MKCONFIG_SYSTYPE} in
      BeOS|Haiku)
        # uname -m does not reflect actual architecture
        doappend ldflags_libs_system " -lroot -lbe"
        ;;
  esac

  # largefile flags
  doappend ldflags_libs_system " $lflibs"

  _read_option LIBS ""
  doappend ldflags_libs_user " $LIBS"

  puts "ldflags_libs_user:${ldflags_libs_user}" >&9
  puts "ldflags_libs_application:${ldflags_libs_application}" >&9
  puts "ldflags_libs_system:${ldflags_libs_system}" >&9

  _setflags \
      ldflags_libs_user LDFLAGS_LIBS_USER \
      ldflags_libs_application LDFLAGS_LIBS_APPLICATION \
      ldflags_libs_system LDFLAGS_LIBS_SYSTEM
}

# for backwards compatibility
check_shcflags () {
  check_cflags_shared
}

check_shcflags () {
  printlabel CFLAGS_SHARED "shared library cflags"

  cflags_shared=""

  if [ "$_MKCONFIG_USING_GCC" != Y ]; then
    case ${_MKCONFIG_SYSTYPE} in
      CYGWIN*|MSYS*|MINGW*)
        doappend cflags_shared " -fPIC"
        ;;
      Darwin)
        doappend cflags_shared " -fno-common"
        ;;
      HP-UX)
        doappend cflags_shared " +Z"
        ;;
      IRIX*)
        doappend cflags_shared " -KPIC"
        ;;
      OSF1)
        # none
        doappend cflags_shared " -fPIC"
        ;;
      SCO_SV)
        doappend cflags_shared " -KPIC"
        ;;
      SunOS)
        doappend cflags_shared " -KPIC"
        ;;
      UnixWare)
        doappend cflags_shared " -KPIC"
        ;;
      *)
        doappend cflags_shared " -fPIC"
        ;;
    esac
  else
    doappend cflags_shared " -fPIC"
  fi

  _read_option CFLAGS_SHARED ""
  if [ "z$CFLAGS_SHARED" != z ]; then
    cflags_shared_user="${CFLAGS_SHARED}"
  fi
  printyesno_val CFLAGS_SHARED "$cflags_shared $cflags_shared_user"

  _setflags \
      cflags_shared CFLAGS_SHARED \
      cflags_shared_user CFLAGS_SHARED_USER
}

# for backwards compatibility
check_shldflags () {
  check_ldflags_shared
}

check_ldflags_shared () {
  printlabel LDFLAGS_SHARED "shared library ldflags"

  if [ "$_MKCONFIG_USING_GCC" != Y ]; then
    case ${_MKCONFIG_SYSTYPE} in
      AIX)
        doappend ldflags_shared " -G"
        ;;
      HP-UX)
        doappend ldflags_shared " -b"
        ;;
      IRIX*)
        # "-shared"
        doappend ldflags_shared " -shared"
        ;;
      OSF1)
        doappend ldflags_shared " -msym -no_archive"
        ;;
      SCO_SV)
        doappend ldflags_shared " -G"
        ;;
      SunOS)
        doappend ldflags_shared " -G"
        ;;
      UnixWare)
        doappend ldflags_shared " -G"
        ;;
      *)
        doappend ldflags_shared " -shared"
        ;;
    esac
  else
    doappend ldflags_shared " -shared"
  fi

  case ${_MKCONFIG_SYSTYPE} in
    Darwin)
      doappend ldflags_shared " -dynamiclib"
      ;;
  esac

  _read_option LDFLAGS_SHARED ""
  if [ "z$LDFLAGS_SHARED" != z ]; then
    ldflags_shared_user="${LDFLAGS_SHARED}"
  fi
  printyesno_val LDFLAGS_SHARED "$ldflags_shared $ldflags_shared_user"

  _setflags \
      ldflags_shared LDFLAGS_SHARED ldflags_shared_user LDFLAGS_SHARED_USER
}

check_sharednameflag () {
  printlabel SHLDNAMEFLAG "shared lib name flag"

  SHLDNAMEFLAG="-Wl,-soname="
  if [ "$_MKCONFIG_USING_GNU_LD" != Y ]; then
    case ${_MKCONFIG_SYSTYPE} in
      Darwin)
        # -compatibility_version -current_version
        ;;
      HP-UX)
        SHLDNAMEFLAG="-Wl,+h "
        ;;
      IRIX*)
        # -soname
        ;;
      OSF1)
        # -soname
        ;;
      SunOS)
        SHLDNAMEFLAG="-Wl,-h "
        ;;
    esac
  fi

  printyesno_val SHLDNAMEFLAG "$SHLDNAMEFLAG"
  setdata ${_MKCONFIG_PREFIX} SHLDNAMEFLAG "$SHLDNAMEFLAG"
}

check_shareexeclinkflag () {
  printlabel LDFLAGS_EXEC_LINK "shared executable link flag "

  LDFLAGS_EXEC_LINK="-Bdynamic "
  if [ "$_MKCONFIG_USING_GCC" != Y ]; then
    case ${_MKCONFIG_SYSTYPE} in
      AIX)
        LDFLAGS_EXEC_LINK="-brtl -bdynamic "
        ;;
      Darwin)
        LDFLAGS_EXEC_LINK=
        ;;
      HP-UX)
        LDFLAGS_EXEC_LINK="+Z"
        ;;
      OSF1)
        LDFLAGS_EXEC_LINK="-msym -no_archive "
        ;;
      SCO_SV)
        LDFLAGS_EXEC_LINK=
        ;;
      SunOS)
        # -Bdynamic
        ;;
      UnixWare)
        LDFLAGS_EXEC_LINK=
        ;;
    esac
  fi

  printyesno_val LDFLAGS_EXEC_LINK "$LDFLAGS_EXEC_LINK"
  setdata ${_MKCONFIG_PREFIX} LDFLAGS_EXEC_LINK "$LDFLAGS_EXEC_LINK"
}

check_sharerunpathflag () {
  printlabel LDFLAGS_RUNPATH "shared run path flag "

  LDFLAGS_RUNPATH="-Wl,-rpath="
  if [ "$_MKCONFIG_USING_GNU_LD" != Y ]; then
    case ${_MKCONFIG_SYSTYPE} in
      AIX)
        LDFLAGS_RUNPATH=
        ;;
      Darwin)
        LDFLAGS_RUNPATH=
        ;;
      HP-UX)
        LDFLAGS_RUNPATH="-Wl,+b "
        ;;
      IRIX*)
        LDFLAGS_RUNPATH="-Wl,-rpath "
        ;;
      OSF1)
        LDFLAGS_RUNPATH="-rpath "
        ;;
      SCO_SV)
        LDFLAGS_RUNPATH="-Wl,-R "
        ;;
      SunOS)
        LDFLAGS_RUNPATH="-Wl,-R"
        ;;
      UnixWare)
        LDFLAGS_RUNPATH="-Wl,-R "
        ;;
    esac
  fi

  printyesno_val LDFLAGS_RUNPATH "$LDFLAGS_RUNPATH"
  setdata ${_MKCONFIG_PREFIX} LDFLAGS_RUNPATH "$LDFLAGS_RUNPATH"
}

check_addconfig () {
  name=$1
  evar=$2
  addto=$3
  printlabel ADDCONFIG "Add Config: ${evar} ${addto}"
  eval _tvar="\$$evar"
  if [ "z$_tvar" != z ]; then
    printyesno_val $name yes
    doappend $addto " $_tvar"
    puts "got: ${evar} ${_tvar}" >&9
    eval puts "\"$addto: \$$addto\"" >&9
    ucaddto=$addto
    toupper ucaddto
    _setflags $addto $ucaddto
  else
    printyesno_val $name no
  fi
}

check_findconfig () {
  name=$1
  cfile=$2
  printlabel FINDCONFIG "Search for: ${cfile}"
  sp=
  incchk=
  pp=`puts $PATH | sed 's/:/ /g'`
  for p in $HOME/local/lib /usr/local/lib /opt/local/lib $pp; do
    td=$p
    case $p in
      */bin)
        td=`puts $p | sed 's,/bin$,/lib,'`
        ;;
    esac
    if [ -d $td ]; then
      if [ -f "$td/$cfile.sh" ]; then
        puts "found: ${td}" >&9
        sp=$td
        break
      fi
    fi
  done

  if [ z$sp != z ]; then
    printyesno_val $name yes
    setdata ${_MKCONFIG_PREFIX} config_${cfile} Y
    setdata ${_MKCONFIG_PREFIX} config_path_${cfile} $sp/$cfile
    . $sp/$cfile.sh ; # load the environment variables
  else
    printyesno_val $name no
    setdata ${_MKCONFIG_PREFIX} config_${cfile} N
  fi
}