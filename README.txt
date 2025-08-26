di - disk information utility

Website: https://diskinfo-di.sourceforge.io/
SourceForge: https://sourceforge.net/projects/diskinfo-di/

CONTENTS
  Important Notes: Version 5/6
  Description
  Documentation
  Installation
  Requirements
  Porting Help
  Acknowledgements
  Copyright

IMPORTANT NOTES: VERSION 5/6  (2025-3-10)

  Version 5's code base has changed.  The build process has changed and
  optionally uses cmake (3.13+). The display processing is entirely new code.
  Version 6 had an ABI change to fix a crash.

  Version 5/6 installs a shared library that can be used to get the disk
  space or percentages.

  Version 5/6 is very new, and there could still be errors.

DESCRIPTION

  'di' is a disk information utility, displaying everything
  (and more) that your 'df' command does. It features the
  ability to display your disk usage in whatever format you
  prefer. It is designed to be highly portable. Great for
  heterogenous networks.

DOCUMENTATION

  wiki:         https://sourceforge.net/p/diskinfo-di/wiki/Home/
  Change Log:   https://sourceforge.net/p/diskinfo-di/wiki/ChangeLog
  Test Results: https://sourceforge.net/p/diskinfo-di/wiki/Testing

INSTALLATION

  To build:
    make -e PREFIX=$HOME/local
    make -e PREFIX=$HOME/local install

    The build will use cmake if it is available and recent enough (3.13),
    otherwise the mkconfig configuration tool will be used.

    To use cmake in a stand-alone fashion (version 5.0.4):
        cmake -DCMAKE_INSTALL_PREFIX=$HOME/local -S . -B build
        cmake --build build
        cmake --install build

    If the GMP or libtommath library is found, it will be used,
    otherwise the numerics will be handled using standard C data types
    (long double, double, etc.).

    To turn off the use of the multi-precision libraries;
      make -e PREFIX=$HOME/local DI_USE_MATH=DI_INTERNAL

      Long doubles are quite capable, there's nothing wrong with
      building it this way (as of 2025).
    To force GMP:
      make -e PREFIX=$HOME/local DI_USE_MATH=DI_GMP
    To force libtommath:
      make -e PREFIX=$HOME/local DI_USE_MATH=DI_TOMMATH
    To force the use of mkconfig:
      make -e PREFIX=$HOME/local mkc-all
      make -e PREFIX=$HOME/local mkc-install

REQUIREMENTS

  cmake build
    make
    cmake (3.13+)
    pkg-config
    msgfmt
    C compiler
    sed grep tr test /bin/sh
    awk (gawk/nawk/awk)
  mkconfig build
    make
    pkg-config
    msgfmt
    C compiler
    bourne/ksh/bash shell
    awk (gawk/nawk/awk)
    cat chmod ln mkdir mv rm sed test expr grep sort
  libraries:
    Linux: tirpc (nfs quotas)
    MP Math: gmp or libtommath (optional)

BUG REPORTS

    I need to know what operating system and what version of
    operating system you are on.  Also which compiler, and the version
    of the compiler.

    For build issues, capture the output from `make`, and the files
    listed below.

    For runtime issues, I want to see the output from 'di -A -a -X 5'
    and the output from your standard df command (as verbose as possible),

    Files to include in your report:

    cmake:
      build/config.h
      build/CMakeOutput.log
      build/CMakeError.log
      build/CMakeFiles/CMakeConfigureLog.yaml
    mkconfig:
      config.h
      di.env
      di.reqlibs
      mkc_files/mkconfig.log
      mkc_files/mkconfig_env.log
      mkc_files/mkc_compile.log

    Open a ticket at : https://sourceforge.net/p/diskinfo-di/tickets/
    Or e-mail : brad.lanam.di_at_gmail.com

PORTING HELP

    I use my own set of virtual machines, the gcc compile farm, and
    polarhome (which is now limited, and will probably die at some
    point).

    If you have a computer on the internet with a less common or older
    operating system, I could use access for portability testing.

    I need access to HP-UX.

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
