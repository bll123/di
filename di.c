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
 *    Strings
 *      m - mount point
 *      d - device name (special)
 *      t - disk partition type
 *      O - mount options.
 *    Disk Space
 *      b - total kbytes
 *      B - total kbytes available for use by the user.
 *             [ (tot - (free - avail)) ]
 *      u - kbytes used (actual number of kbytes used) [ (tot - free) ]
 *      c - calculated number of kbytes used [ (tot - avail) ]
 *      f - kbytes free
 *      v - kbytes available
 *  Disk Space Percentages
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
 *  Inodes
 *      i - total i-nodes (files)
 *      U - used i-nodes
 *      F - free i-nodes
 *      P - percent i-nodes used     [ (tot - avail) / tot ]
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
#if _hdr_stddef
# include <stddef.h>
#endif
#if _hdr_stdlib
# include <stdlib.h>
#endif
#if _hdr_string
# include <string.h>
#endif
#if _hdr_strings
# include <strings.h>
#endif
#if _hdr_libintl
# include <libintl.h>
#endif

#include "di.h"
#include "disystem.h"
#include "strutils.h"
#include "version.h"

#define DI_INIT_DISP_SZ   10000

static void di_display_data (void *);
static void usage (void);

int
main (int argc, char * argv [])
{
  void      *di_data;
  int       exitflag;

  di_data = di_initialize ();
  exitflag = di_process_options (di_data, argc, argv);
  switch (exitflag) {
    case DI_EXIT_FAIL:
    case DI_EXIT_WARN: {
      di_cleanup (di_data);
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
      di_cleanup (di_data);
      exit (0);
    }
    case DI_EXIT_NORM: {
      break;
    }
  }
  di_get_data (di_data);
  di_display_data (di_data);
  di_cleanup (di_data);
  return 0;
}

static void
usage (void)
{
  printf (DI_GT("di version %s\n"), DI_VERSION);
             /*  12345678901234567890123456789012345678901234567890123456789012345678901234567890 */
  printf (DI_GT("Usage: di [-ant] [-d display-size] [-f format] [-x exclude-fstyp-list]\n"));
  printf (DI_GT("       [-I include-fstyp-list] [file [...]]\n"));
  printf (DI_GT("   -a   : print all mounted devices\n"));
  printf (DI_GT("   -d x : size to print blocks in (k,m,g,t,etc.)\n"));
  printf (DI_GT("          h - human readable.\n"));
  printf (DI_GT("   -f x : use format string <x>\n"));
  printf (DI_GT("   -I x : include only file system types in <x>\n"));
  printf (DI_GT("   -x x : exclude file system types in <x>\n"));
  printf (DI_GT("   -l   : display local filesystems only\n"));
  printf (DI_GT("   -n   : don't print header\n"));
  printf (DI_GT("   -t   : print totals\n"));
  printf (DI_GT(" Format string values:\n"));
  printf (DI_GT("    m - mount point\n"));
  printf (DI_GT("    d - device name\n"));
  printf (DI_GT("    t - file-system type\n"));
  printf (DI_GT("    b - total kbytes                    B - kbytes available for use\n"));
  printf (DI_GT("    u - used kbytes                     c - calculated kbytes in use\n"));
  printf (DI_GT("    f - kbytes free                     v - kbytes available\n"));
  printf (DI_GT("    p - percentage not avail. for use   1 - percentage used\n"));
  printf (DI_GT("    2 - percentage of user-available space in use.\n"));
  printf (DI_GT("    i - total file slots (i-nodes)      U - used file slots\n"));
  printf (DI_GT("    F - free file slots                 P - percentage file slots used\n"));
  printf (DI_GT("See manual page for more options.\n"));
}

static void
di_display_data (void *di_data)
{
  di_pub_disk_info_t  *pub;
  unsigned int        maxlen [DI_DISP_MAX];
  int                 i;
  int                 iterval;
  char                *disp = NULL;
  char                *end = NULL;
  char                *dp = NULL;
  char                temp [MAXPATHLEN + 1];

  for (i = 0; i < DI_DISP_MAX; ++i) {
    maxlen [i] = 0;
  }

  iterval = di_check_option (di_data, DI_OPT_DISP_ALL);
  di_iterate_init (di_data, iterval);
  while ((pub = di_iterate (di_data)) != NULL) {
    Size_t    len;

    for (i = 0; i < DI_DISP_MAX; ++i) {
      if (pub->strdata [i] == NULL) {
        continue;
      }

      len = strlen (pub->strdata [i]);
      if (len > maxlen [i]) {
        maxlen [i] = len;
      }
    }
  }

  /* some large number to start with */
  disp = malloc (DI_INIT_DISP_SZ);
  *disp = '\0';
  end = disp + DI_INIT_DISP_SZ;
  dp = disp;

  di_iterate_init (di_data, iterval);
  while ((pub = di_iterate (di_data)) != NULL) {
    int   fmt;

    di_format_iter_init (di_data);
fprintf (stdout, "== %-*s\n", maxlen [DI_DISP_MOUNTPT], pub->strdata [DI_DISP_MOUNTPT]);
    while ((fmt = di_format_iterate (di_data)) != DI_FMT_ITER_STOP) {
      *temp = '\0';

      switch (fmt) {
        /* string values */
        case DI_FMT_MOUNT:
        case DI_FMT_MOUNT_OLD: {
          Snprintf1 (temp, sizeof (temp), "%-*s",
              maxlen [DI_DISP_MOUNTPT], pub->strdata [DI_DISP_MOUNTPT]);
          break;
        }
        case DI_FMT_DEVNAME:
        case DI_FMT_DEVNAME_OLD:
        case DI_FMT_DEVNAME_OLD_B: {
          Snprintf1 (temp, sizeof (temp), "%-*s",
              maxlen [DI_DISP_DEVNAME], pub->strdata [DI_DISP_DEVNAME]);
          break;
        }
        case DI_FMT_FSTYPE:
        case DI_FMT_FSTYPE_OLD: {
          Snprintf1 (temp, sizeof (temp), "%-*s",
              maxlen [DI_DISP_FSTYPE], pub->strdata [DI_DISP_FSTYPE]);
          break;
        }
        case DI_FMT_MOUNT_OPTIONS: {
          Snprintf1 (temp, sizeof (temp), "%-*s",
              maxlen [DI_DISP_MOUNTOPT], pub->strdata [DI_DISP_MOUNTOPT]);
          break;
        }

        /* disk space values */
        case DI_FMT_BTOT: {
          break;
        }
        case DI_FMT_BTOT_AVAIL: {
          break;
        }
        case DI_FMT_BUSED: {
          break;
        }
        case DI_FMT_BCUSED: {
          break;
        }
        case DI_FMT_BFREE: {
          break;
        }
        case DI_FMT_BAVAIL: {
          break;
        }

        /* disk space percentages */
        case DI_FMT_BPERC_NAVAIL: {
          break;
        }
        case DI_FMT_BPERC_USED: {
          break;
        }
        case DI_FMT_BPERC_BSD: {
          break;
        }
        case DI_FMT_BPERC_AVAIL: {
          break;
        }
        case DI_FMT_BPERC_FREE: {
          break;
        }

        /* inode information */
        case DI_FMT_ITOT: {
          break;
        }
        case DI_FMT_IUSED: {
          break;
        }
        case DI_FMT_IFREE: {
          break;
        }
        case DI_FMT_IPERC: {
          break;
        }
        default: {
          break;
        }
      }

      dp = stpecpy (dp, end, temp);
      dp = stpecpy (dp, end, " ");
    }
    dp = stpecpy (dp, end, "\n");

#if 0
    fprintf (stdout, "%-*s %-*s %-*s dp:%d pf:%d l:%d ro:%d lo:%d\n",
        maxlen [DI_DISP_MOUNTPT], pub->strdata [DI_DISP_MOUNTPT],
        maxlen [DI_DISP_DEVNAME], pub->strdata [DI_DISP_DEVNAME],
        maxlen [DI_DISP_FSTYPE], pub->strdata [DI_DISP_FSTYPE],
        pub->doPrint, pub->printFlag,
        pub->isLocal, pub->isReadOnly, pub->isLoopback);
#endif
  }
  fprintf (stdout, "%s", disp);
  free (disp);
}
