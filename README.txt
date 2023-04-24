di - disk information utility (version 4.52)

Website: https://diskinfo-di.sourceforge.io/
SourceForge: https://sourceforge.net/projects/diskinfo-di/

CONTENTS
  Description
  Installation
  Tcl
  Perl
  Bugs and Known Issues
  Requirements
  Porting Information
  Acknowledgements
  Copyright

DESCRIPTION

  'di' is a disk information utility, displaying everything
  (and more) that your 'df' command does. It features the
  ability to display your disk usage in whatever format you
  prefer. It is designed to be highly portable. Great for
  heterogenous networks.

INSTALLATION

  To build:
    make -e dioptions.dat
    # Edit dioptions.dat and change it
    # to set your preferences.
    # Add your CC, CFLAGS and LDFLAGS settings.
    # This file will not be removed and can be reused
    # for the next release.
    make -e
    make -e install
  Example to change compiler and prefix:
    CC=gcc prefix=/opt/bin make -e

  If you don't have a bourne shell compatible shell,
  Try building with perl:
      make -e all-perl

  The CC, CFLAGS, CPPFLAGS, prefix, LDFLAGS, and LIBS environment
  variables may be set and exported prior to running make.  This
  will override any settings in 'dioptions.dat'.

  The LOCALEDIR environment variable defaults to ${prefix}/share/locale.
  The installation uses the INSTALL_DIR (default ${prefix}),
  INSTALL_BIN_DIR ($INSTALL_DIR/bin), and INST_LOCALEDIR
  ($INSTALL_DIR/share/locale) environment variables for installation.

  The internationalization files are automatically built and installed if
  your system supports internationalization.

  Windows installation:
    Alternative 1 - uses pre-built config.win or config.mingw
      Cygwin gcc: make -e windows-gcc
        (uses cygwin API)
      MinGW: mingw32-make -e MAKE=mingw32-make CC=gcc windows-mingw
        (uses windows API)
      Msys2: make WINAPI=T NO_PIE=yes
    Alternative 2 - builds a new config.h file (Cygwin).
      make -e NO_PIE=yes CC=gcc
      make -e NO_PIE=yes CC=gcc WINAPI=T  # windows API
    Alternative 3 - builds a new config.h file (MSys)
      # make sure your PATH is set properly
      make -e _MKCONFIG_SHELL=bash WINAPI=T NO_PIE=yes CC=gcc

    You may need to change _lib_GetDiskFreeSpaceEX to 0
    in the config.h file for older windows systems.

  HP-UX:
    Some versions of gcc on HP-UX break the include files.
    If you see:    header: rpc/rpc.h ... no
    try: env CC=gcc CFLAGS="-D_LABEL_T" make -e

    On some HP-UX systems, quotactl isn't declared.
    If you want quota support, make sure config.h has:
      #define _lib_quotactl 1
      #define _args_quotactl 4
      #define _c_arg_1__quotactl int
      #define _c_arg_2__quotactl char *
      #define _c_arg_3__quotactl int
      #define _c_arg_4__quotactl caddr_t
      #define _quotactl_pos_1 0
      #define _quotactl_pos_2 1

    64-bit:
      env CFLAGS="-z +Z +DD64" LDFLAGS="+DD64" make -e
    64-bit gcc:
      env CC=gcc CFLAGS="-fPIC -mlp64 -D_LABEL_T" \
          LDFLAGS="-mlp64 -L/usr/lib/hpux64" make -e
    32-bit:
      env CFLAGS="-z +Z +DD32" LDFLAGS="+DD32" make -e
    32-bit gcc:
      env CC=gcc CFLAGS="-fPIC -milp32 -D_LABEL_T" \
          LDFLAGS="-milp32 -L/usr/lib/hpux32" make -e
    Bundled cc:
      cd C;make hpux-cc

  Tru64:
    export BIN_SH=svr4 beforehand.

  DragonFlyBSD:
    If quotas are not turned on in the kernel (/boot/loader.conf),
    the vquotactl interface will not be configured into di.  The
    default build available from DragonFly dports does not have
    the vquotactl interface turned on.

  Installation Permissions:
    Some old systems (Sys V variants) only allow root to read the
    mount table.  In SysV.4 and Solaris, the mount command would
    reset the permissions to be root readable only (fixed in Solaris 7).

    If this is needed on your system, do:
        make installperms

TCL
    The Tcl extension provides access to the di utility via
    the [diskspace] command.

    To build:

      cd C; make -e tcl-sh

      Windows Cygwin:
        cd C; make CC=gcc NO_PIE=yes tcl-sh
      Windows Msys:
        cd C; make WINAPI=T NO_PIE=yes tcl-sh

    See C/di-example.tcl for a usage example.

PERL
    The Perl extension provides access to the di utility via
    the [Filesys::di::diskspace] command.

    To build:

      # perl-perl can be substituted for perl-sh to use the
      # perl configuration script instead of the shell
      # configuration scripts.
      cd C; make -e perl-sh; cd Perl; make PREFIX=your-prefix install

      Windows Cygwin:
        # I had to install libcrypt-devel for 'crypt.h'.
        # No idea why perl would require that.
        # perl-perl is faster than perl-sh.
        cd C; make CC=gcc NO_PIE=yes perl-perl
      Windows Msys:
        # untested; I don't have mingw perl.
        cd C; make -e WINAPI=T NO_PIE=yes CC=gcc perl-sh

    See C/Perl/ex/perl-example.pl for a usage example.

BUGS AND KNOWN ISSUES
    What's Not Tested:
      - pooled filesystems on Tru64.
        - I have access to a Tru64 system w/quotas, but there's
          only one filesystem per pool.
      - quotas:
        - MacOSX
          - No access to users with quotas.
      - MS VC++ has not been tested in a *very* long while.
      - A/UX, Cray, UNICOS, Next, Pyramid, SCO Unix, Sequent
        have not been tested in a *very* long while.
    Known Issues
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
      - cygwin with clang does not work.
      - the pre-configured builds have not been tested in a while.

    Send bug reports along with the output from 'di -A -a -X 5',
    output from your standard df command (as verbose as possible),
    mkconfig.log, mkconfig_env.log, di.env, and config.h to:

        brad.lanam.di_at_gmail.com

    If the 'config.h' doesn't get set up correctly, please let me know.
    E-mail me the incorrect (and corrected if possible) config.h file,
    and any other information as appropriate.

REQUIREMENTS

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

    di has been ported to the following systems in the past:

        A/UX, AIX, BeOS, BSD, BSDI, Convex, Cray UNICOS, Cray UNICOS/mk,
        DragonflyBSD, FreeBSD, Haiku, HP/UX, Linux, MacOSX, MirOS, NetBSD,
        Next, OpenBSD, OS/2, OSF/1, Pyramid, SCO OpenServer, SCO Unix,
        Sequent Dynix and PT/x, SGI Irix, Solaris, SunOS, Syllable,
        System V.3, System V.4, Tru64, ULTRIX, UnixWare, VMS, Windows, Xenix

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

    Copyright 1994-2023 Brad Lanam, Pleasant Hill, CA, USA
    brad.lanam.di_at_gmail.com
    https://diskinfo-di.sourceforge.io
