di - disk information utility (version 5.0.0)

Website: https://diskinfo-di.sourceforge.io/
SourceForge: https://sourceforge.net/projects/diskinfo-di/

CONTENTS
  Description
  Documentation
  Installation
  Bugs and Known Issues
  Requirements
  Acknowledgements
  Copyright

DESCRIPTION

  'di' is a disk information utility, displaying everything
  (and more) that your 'df' command does. It features the
  ability to display your disk usage in whatever format you
  prefer. It is designed to be highly portable. Great for
  heterogenous networks.

DOCUMENTATION

  wiki: https://sourceforge.net/p/di-diskinfo/wiki/Home/
  Change Log: https://sourceforge.net/p/di-diskinfo/wiki/ChangeLog

INSTALLATION

  To build:
    make PREFIX=$HOME/local
    make PREFIX=$HOME/local install

    The build will use cmake if it is available and recent enough,
    otherwise the mkconfig configuration tool will be used.

    If GMP or libtommath is found, they will be used (GMP as priority),
    otherwise the numerics will be handled using standard C data types
    (long double, double, etc.).

    To turn off the use of the multi-precision libraries;
      make PREFIX=$HOME/local DI_USE_MATH=DI_INTERNAL

      Long doubles are quite capable, there's nothing wrong with
      building it this way.
    To force GMP:
      make PREFIX=$HOME/local DI_USE_MATH=DI_GMP
    To force libtommath:
      make PREFIX=$HOME/local DI_USE_MATH=DI_TOMMATH

BUGS AND KNOWN ISSUES

  - DragonFly BSD:
    If quotas are not turned on in the kernel (/boot/loader.conf),
    the vquotactl interface will not be configured into di.  The
    default build available from DragonFly dports does not have
    the vquotactl interface turned on.
  - quotas:
    - Solaris ufs requires the quotas file to be readable, or make
      the di program setuid.  Neither recommended.
    - Linux 2.4 - not working
    - HP-UX
      - quotactl() isn't declared. See HP-UX section above.
    - NetBSD 6
      - group quotas can't be fetched by ordinary users (EPERM).
        If the account is limited by a group quota, it won't be reported.
    - AIX 7
      - Unlike AIX 5, AIX 7 gives a permission
        denied error when a user tries to get their quota.
        Could make di suid, but that's not a good idea.
  - pooled filesystems:
    - hammer: the usage reported for pseudo-filesystems is the usage
      for the main pool.  Therefore the total used and size will be wrong.
    - btrfs: there's no way to identify the pools.

BUG REPORTS

    I want to see the output from 'di -A -a -X 5',
    output from your standard df command (as verbose as possible),
    and:
      cmake: build/config.h
      mkconfig: mkc_files/mkconfig.log, mkc_files/mkconfig_env.log,
                mkc_files/mkc_compile.log, di.env, and config.h

    Open a ticket at : https://sourceforge.net/p/di-diskinfo/tickets/
    Or e-mail : brad.lanam.di_at_gmail.com

REQUIREMENTS

  cmake build


    bourne/ksh/bash shell
    C compiler
    awk (mawk/nawk/gawk)
    make
        cat chmod ln mkdir mv rm sed test
    mkconfig.sh:
        a bourne compatible shell that supports shell functions,
          standard output from 'set'.
        cat egrep expr grep rm sed sort test
    mkconfig.pl:
        cat perl rm
    mksetopt.sh:
        mv rm sed test
    NLS:
        msgfmt sed

    runtests.sh (not a requirement for building or installing):
        ar cat cp dc diff env expr egrep mv rm sed sort test tr

PORTING

    di 4.47 has been tested on the following platforms:
      Linux
        RedHat 7.3 (gcc)
        CentOS 3.9 (gcc)
        Fedora 7 (gcc)
        Fedora 27 (gcc)
        MX Linux 17.1 (gcc, clang)
      BSD
        DragonflyBSD 4.4 (gcc)
        FreeBSD 7.0 (cc)
        FreeBSD 11.0 (clang)
        NetBSD 1.62 (cc)
        NetBSD 2.0 (cc)
        NetBSD 7.0.1 (gcc)
        OpenBSD 4.4 (gcc)
      Windows
        Msys2 (gcc)
        Cygwin (gcc)
      Other
        AIX 7.1 (gcc)
        Mac OS X 10.12.6 (clang)
        QNX 6.5 (cc)
        SCO SV 6.0.0 (cc)
        Solaris 11/x86 (cc12.3)
        Solaris 10/sparc (cc12.3)
        Solaris 9/x86 (gcc3)
        Tru64 5.1B (cc)
        UnixWare 7.1.4 (cc)

ACKNOWLEDGEMENTS

    And for their comments/source/manual pages and/or bug fixes, thanks!

        J.J.Bailey
        Karl Vogel [pyramid]
        Bryan Costales
        Mark Neale
        Pat Myrto [sunOS filesystem type stuff]
        Henri Karrenbeld [sgi]
        Peter Bray
        George M. Sipe [manual page]
        Jeffrey Mogul [ultrix, osf/1, manual page, new format options]
        Th. Bullinger [help usage]
        Seth Theriault [next, tru64]
        Stephen J. Walick [SCO]
        Gerald Rinske [sgi]
        Mike Grupenhoff [linux]
        R.K.Lloyd [hpux]
        William Gibbs [xenix]
        Randy Thompson [sequent]
        Erik O'Shaughnessy [aix]
        Bill Davidsen [linux, SCO, etc., new format options]
        Fred Smith [coherent 4.2.05]
        Andrey Blochintsev [bsdi]
        Brian Ginsbach [netbsd, irix, unicos]

        et. al.

COPYRIGHT

    Copyright 1994-2025 Brad Lanam, Pleasant Hill, CA, USA
    brad.lanam.di_at_gmail.com
    https://diskinfo-di.sourceforge.io
