di - disk information utility (version 5.0.0)

Website: https://diskinfo-di.sourceforge.io/
SourceForge: https://sourceforge.net/projects/diskinfo-di/

CONTENTS
  Description
  Documentation
  Installation
  Requirements
  Bugs and Known Issues
  Acknowledgements
  Copyright

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

    The build will use cmake if it is available and recent enough,
    otherwise the mkconfig configuration tool will be used.

    If GMP or libtommath is found, they will be used,
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

REQUIREMENTS

  cmake build
    make
    cmake
    sed grep tr
    msgfmt
    pkg-config
    libintl/libiconv (gettext)
    C compiler
  mkconfig build
    make
    msgfmt
    cat chmod ln mkdir mv rm sed test expr grep sort
    pkg-config
    bourne/ksh/bash shell
    C compiler
    awk (mawk/nawk/gawk)
    C compiler

BUG REPORTS

    For build issues, capture the output from `make`, and the files
    listed below.

    For runtime issues, I want to see the output from 'di -A -a -X 5',
    output from your standard df command (as verbose as possible),

    Files to include in your report:

    cmake: build/config.h, build/CMakeOutput.log,
           build/CMakeError.log
    mkconfig: mkc_files/mkconfig.log, mkc_files/mkconfig_env.log,
              mkc_files/mkc_compile.log, di.env, and config.h

    Open a ticket at : https://sourceforge.net/p/diskinfo-di/tickets/
    Or e-mail : brad.lanam.di_at_gmail.com

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
