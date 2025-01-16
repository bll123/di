/*
 * Copyright 1994-2018 Brad Lanam, Walnut Creek, CA
 * Copyright 2023-2025 Brad Lanam, Pleasant Hill, CA
 */

/*
 * di.c
 *
 *  Warning: Do not replace your system's 'df' command with this program.
 *           You will in all likelihood break your installation procedures.
 *
 *  Display sizes:
 *      512 - posix (512 bytes)
 *      k - kilobytes
 *      m - megabytes
 *      g - gigabytes
 *      t - terabytes
 *      P - petabytes
 *      E - exabytes
 *      h - "human readable" scaled alternative 1
 *      H - "human readable" scaled alternative 2
 *
 *  Sort types:
 *      N - name (default)
 *      n - none (mount order)
 *      s - special
 *      a - avail
 *      t - type
 *      r - reverse sort
 *
 *  Format string values:
 *      m - mount point
 *      M - mount point, full length
 *      b - total kbytes
 *      B - total kbytes available for use by the user.
 *             [ (tot - (free - avail)) ]
 *      u - kbytes used (actual number of kbytes used) [ (tot - free) ]
 *      c - calculated number of kbytes used [ (tot - avail) ]
 *      f - kbytes free
 *      v - kbytes available
 *      p - percentage not available for use.
 *          (space not available for use / total disk space)
 *             [ (tot - avail) / tot ]
 *      1 - percentage used.
 *          (actual space used / total disk space)
 *             [ (tot - free) / tot ]
 *      2 - percentage of user-available space in use (bsd style).
 *          Note that values over 100% are possible.
 *          (actual space used / disk space available to user)
 *             [ (tot - free) / (tot - (free - avail)) ]
 *      i - total i-nodes (files)
 *      U - used i-nodes
 *      F - free i-nodes
 *      P - percent i-nodes used     [ (tot - avail) / tot ]
 *      s - filesystem name (special)
 *      S - filesystem name (special), full length
 *      t - disk partition type
 *      T - disk partition type (full length)
 *      I - mount time
 *      O - mount options.
 *
 *  System V.4 `/usr/bin/df -v` Has format: msbuf1
 *  System V.4 `/usr/bin/df -k` Has format: sbcvpm
 *  System V.4 `/usr/ucb/df`    Has format: sbuv2m
 *
 *  The default format string for this program is: smbuvpT
 *
 *  Environment variables:
 *      DI_ARGS:            specifies any arguments to di.
 *      POSIXLY_CORRECT:    forces posix mode.
 *      BLOCKSIZE:          BSD df block size.
 *      DF_BLOCK_SIZE:      GNU df block size.
 *
 *  Note that for filesystems that do not have, or systems (SysV.3)
 *  that do not report, available space, the amount of available space is
 *  set to the free space.
 *
 */

#include "config.h"

#if _hdr_stdio
# include <stdio.h>
#endif
#if _hdr_stdlib
# include <stdlib.h>
#endif
#if _sys_types \
    && ! defined (DI_INC_SYS_TYPES_H) /* xenix */
# define DI_INC_SYS_TYPES_H
# include <sys/types.h>
#endif
#if _hdr_libintl
# include <libintl.h>
#endif

#include "dilib.h"
#include "di.h"
#include "display.h"
#include "version.h"

static void usage (void);

int
main (int argc, char * argv [])
{
  di_data_t di_data;
  int       exitflag;

  di_initialize (&di_data);
  exitflag = di_process_options (&di_data, argc, argv);
  switch (exitflag) {
    case DI_EXIT_FAIL:
    case DI_EXIT_WARN: {
      di_cleanup (&di_data);
      exit (exitflag);
    }
    case DI_EXIT_HELP:
    case DI_EXIT_VERS: {
      if (exitflag == DI_EXIT_HELP) {
        usage ();
      }
      if (exitflag == DI_EXIT_VERS) {
        printf (DI_GT("di version %s\n"), DI_VERSION);
      }
      di_cleanup (&di_data);
      exit (0);
    }
    case DI_EXIT_NORM: {
      break;
    }
  }
  di_get_data (&di_data);
  di_display_data (&di_data);
  di_cleanup (&di_data);
  return 0;
}

static void
usage (void)
{
  printf (DI_GT("di version %s    Default Format: %s\n"), DI_VERSION, DI_DEFAULT_FORMAT);
             /*  12345678901234567890123456789012345678901234567890123456789012345678901234567890 */
  printf (DI_GT("Usage: di [-ant] [-d display-size] [-f format] [-x exclude-fstyp-list]\n"));
  printf (DI_GT("       [-I include-fstyp-list] [file [...]]\n"));
  printf (DI_GT("   -a   : print all mounted devices\n"));
  printf (DI_GT("   -d x : size to print blocks in (512 - POSIX, k - kbytes,\n"));
  printf (DI_GT("          m - megabytes, g - gigabytes, t - terabytes, h - human readable).\n"));
  printf (DI_GT("   -f x : use format string <x>\n"));
  printf (DI_GT("   -I x : include only file system types in <x>\n"));
  printf (DI_GT("   -x x : exclude file system types in <x>\n"));
  printf (DI_GT("   -l   : display local filesystems only\n"));
  printf (DI_GT("   -n   : don't print header\n"));
  printf (DI_GT("   -t   : print totals\n"));
  printf (DI_GT(" Format string values:\n"));
  printf (DI_GT("    m - mount point                     M - mount point, full length\n"));
  printf (DI_GT("    b - total kbytes                    B - kbytes available for use\n"));
  printf (DI_GT("    u - used kbytes                     c - calculated kbytes in use\n"));
  printf (DI_GT("    f - kbytes free                     v - kbytes available\n"));
  printf (DI_GT("    p - percentage not avail. for use   1 - percentage used\n"));
  printf (DI_GT("    2 - percentage of user-available space in use.\n"));
  printf (DI_GT("    i - total file slots (i-nodes)      U - used file slots\n"));
  printf (DI_GT("    F - free file slots                 P - percentage file slots used\n"));
  printf (DI_GT("    s - filesystem name                 S - filesystem name, full length\n"));
  printf (DI_GT("    t - disk partition type             T - partition type, full length\n"));
  printf (DI_GT("See manual page for more options.\n"));
}

