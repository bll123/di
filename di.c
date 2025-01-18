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
#if _hdr_wchar
# include <wchar.h>
#endif

#include "di.h"
#include "disystem.h"
#include "distrutils.h"
#include "version.h"

static void di_display_data (void *);
static void di_display_header (void *di_data, char **strdata, unsigned int *maxlen);
static void usage (void);
static Size_t istrlen (const char *str);

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
        printf (DI_GT ("di version %s\n"), DI_VERSION);
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
  printf (DI_GT ("di version %s\n"), DI_VERSION);
             /*  12345678901234567890123456789012345678901234567890123456789012345678901234567890 */
  printf (DI_GT ("Usage: di [-ant] [-d display-size] [-f format] [-x exclude-fstyp-list]\n"));
  printf (DI_GT ("       [-I include-fstyp-list] [file [...]]\n"));
  printf (DI_GT ("   -a   : print all mounted devices\n"));
  printf (DI_GT ("   -d x : size to print blocks in (k,m,g,t,etc.)\n"));
  printf (DI_GT ("          h - human readable.\n"));
  printf (DI_GT ("   -f x : use format string <x>\n"));
  printf (DI_GT ("   -I x : include only file system types in <x>\n"));
  printf (DI_GT ("   -x x : exclude file system types in <x>\n"));
  printf (DI_GT ("   -l   : display local filesystems only\n"));
  printf (DI_GT ("   -n   : don't print header\n"));
  printf (DI_GT ("   -t   : print totals\n"));
  printf (DI_GT (" Format string values:\n"));
  printf (DI_GT ("    m - mount point\n"));
  printf (DI_GT ("    d - device name\n"));
  printf (DI_GT ("    t - file-system type\n"));
  printf (DI_GT ("    b - total kbytes                    B - kbytes available for use\n"));
  printf (DI_GT ("    u - used kbytes                     c - calculated kbytes in use\n"));
  printf (DI_GT ("    f - kbytes free                     v - kbytes available\n"));
  printf (DI_GT ("    p - percentage not avail. for use   1 - percentage used\n"));
  printf (DI_GT ("    2 - percentage of user-available space in use.\n"));
  printf (DI_GT ("    i - total file slots (i-nodes)      U - used file slots\n"));
  printf (DI_GT ("    F - free file slots                 P - percentage file slots used\n"));
  printf (DI_GT ("See manual page for more options.\n"));
}

static void
di_display_data (void *di_data)
{
  di_pub_disk_info_t  *pub;
  unsigned int        *maxlen;
  int                 i;
  int                 iterval;
  int                 fmtstrlen;
  int                 count;
  int                 dispcount;
  char                **strdata;
  int                 csvout;
  int                 csvtabs;
  int                 jsonout;
  char                temp [MAXPATHLEN * 2];

  csvout = di_check_option (di_data, DI_OPT_OUT_CSV);
  csvtabs = di_check_option (di_data, DI_OPT_OUT_CSV_TAB);
  jsonout = di_check_option (di_data, DI_OPT_OUT_JSON);

  fmtstrlen = di_check_option (di_data, DI_OPT_FMT_STR_LEN);
  maxlen = malloc (sizeof (unsigned int) * fmtstrlen);
  for (i = 0; i < fmtstrlen; ++i) {
    maxlen [i] = 0;
  }

  iterval = di_check_option (di_data, DI_OPT_DISP_ALL);
  count = di_iterate_init (di_data, iterval);
  if (di_check_option (di_data, DI_OPT_OUT_HEADER)) {
    ++count;
  }

  strdata = malloc (sizeof (char *) * count * fmtstrlen);

  dispcount = 0;

  if (di_check_option (di_data, DI_OPT_OUT_HEADER)) {
    di_display_header (di_data, strdata, maxlen);
    dispcount = 1;
  }

  if (jsonout) {
    printf ("{\n");
    printf ("  \"partitions\" : [\n");
  }

  di_iterate_init (di_data, iterval);
  while ( (pub = di_iterate (di_data)) != NULL) {
    int         fmt;
    int         fmtcount;
    int         stridx;
    const char  *comma;

    di_format_iter_init (di_data);
    fmtcount = 0;
    comma = ",";
    while ( (fmt = di_format_iterate (di_data)) != DI_FMT_ITER_STOP) {
      stridx = dispcount * fmtstrlen + fmtcount;

      strdata [stridx] = NULL;
      if (fmtcount == fmtstrlen - 1) {
        comma = "";
      }
      switch (fmt) {
        /* string values */
        case DI_FMT_MOUNT:
        case DI_FMT_MOUNT_OLD: {
          if (jsonout) {
            Snprintf3 (temp, sizeof (temp), "\"%s\" : \"%s\"%s",
                "mount", pub->strdata [DI_DISP_MOUNTPT], comma);
            strdata [stridx] = strdup (temp);
          } else {
            strdata [stridx] = strdup (pub->strdata [DI_DISP_MOUNTPT]);
          }
          break;
        }
        case DI_FMT_FILESYSTEM:
        case DI_FMT_FILESYSTEM_OLD:
        case DI_FMT_FILESYSTEM_OLD_B: {
          if (jsonout) {
            Snprintf3 (temp, sizeof (temp), "\"%s\" : \"%s\"%s",
                "filesystem", pub->strdata [DI_DISP_FILESYSTEM], comma);
            strdata [stridx] = strdup (temp);
          } else {
            strdata [stridx] = strdup (pub->strdata [DI_DISP_FILESYSTEM]);
          }
          break;
        }
        case DI_FMT_FSTYPE:
        case DI_FMT_FSTYPE_OLD: {
          if (jsonout) {
            Snprintf3 (temp, sizeof (temp), "\"%s\" : \"%s\"%s",
                "fstype", pub->strdata [DI_DISP_FSTYPE], comma);
            strdata [stridx] = strdup (temp);
          } else {
            strdata [stridx] = strdup (pub->strdata [DI_DISP_FSTYPE]);
          }
          break;
        }
        case DI_FMT_MOUNT_OPTIONS: {
          if (jsonout) {
            Snprintf3 (temp, sizeof (temp), "\"%s\" : \"%s\"%s",
                "options", pub->strdata [DI_DISP_FSTYPE], comma);
            strdata [stridx] = strdup (temp);
          } else {
            strdata [stridx] = strdup (pub->strdata [DI_DISP_MOUNTOPT]);
          }
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
          fprintf (stderr, "Unknown format: %c\n", fmt);
          exit (DI_EXIT_FAIL);
          break;
        }
      }

      ++fmtcount;
    }
    ++dispcount;
  }

  if (! csvout && ! jsonout) {
    for (i = 0; i < count; ++i) {
      int     j;

      for (j = 0; j < fmtstrlen; ++j) {
        int     stridx;
        Size_t  len;

        stridx = i * fmtstrlen + j;
        if (strdata [stridx] != NULL) {
          len = istrlen (strdata [stridx]);
          if (len > maxlen [j]) {
            maxlen [j] = len;
          }
        }
      }
    }
  }

  for (i = 0; i < count; ++i) {
    int     j;

    if (jsonout) {
      printf ("    {\n");
    }
    for (j = 0; j < fmtstrlen; ++j) {
      int     stridx;

      stridx = i * fmtstrlen + j;
      if (strdata [stridx] != NULL) {
        if (csvout) {
          if (csvtabs) {
            printf ("%s", strdata [stridx]);
          } else {
            printf ("\"%s\"", strdata [stridx]);
          }
        } else if (jsonout) {
          printf ("      %s", strdata [stridx]);
        } else {
          printf ("%-*s", maxlen [j], strdata [stridx]);
        }

        if (jsonout) {
          printf ("\n");
        } else {
          if (j != fmtstrlen - 1) {
            if (csvout) {
              if (csvtabs) {
                printf ("\t");
              } else {
                printf (",");
              }
            } else {
              printf (" ");
            }
          }
        }
      }
    }
    if (jsonout) {
      printf ("    }");
      if (i != count - 1) {
        printf (",");
      }
    }
    printf ("\n");
  }

  if (jsonout) {
    printf ("  ]\n");
    printf ("}\n");
  }


  for (i = 0; i < count; ++i) {
    int     j;

    for (j = 0; j < fmtstrlen; ++j) {
      int     stridx;

      stridx = i * fmtstrlen + j;
      if (strdata [stridx] != NULL) {
        free (strdata [stridx]);
      }
    }
  }

  free (maxlen);
  free (strdata);
}

static void
di_display_header (void *di_data, char **strdata, unsigned int *maxlen)
{
  int         fmt;
  int         fmtcount;
  int         fmtstrlen;
  int         csvout;

  csvout = di_check_option (di_data, DI_OPT_OUT_CSV);
  fmtstrlen = di_check_option (di_data, DI_OPT_FMT_STR_LEN);
  fmtcount = 0;
  di_format_iter_init (di_data);
  while ( (fmt = di_format_iterate (di_data)) != DI_FMT_ITER_STOP) {
    const char  *temp;
    char        tcsv [2];
    int         stridx;

    temp = "";
    stridx = fmtcount;
    if (csvout) {
      tcsv [0] = fmt;
      tcsv [1] = '\0';
      temp = tcsv;
    }

    if (! csvout) {
      switch (fmt) {
        /* string values */
        case DI_FMT_MOUNT:
        case DI_FMT_MOUNT_OLD: {
          temp = DI_GT ("Mount");
          break;
        }
        case DI_FMT_FILESYSTEM:
        case DI_FMT_FILESYSTEM_OLD:
        case DI_FMT_FILESYSTEM_OLD_B: {
          temp = DI_GT ("Device");
          break;
        }
        case DI_FMT_FSTYPE:
        case DI_FMT_FSTYPE_OLD: {
          temp = DI_GT ("Type");
          break;
        }
        case DI_FMT_MOUNT_OPTIONS: {
          temp = DI_GT ("Options");
          break;
        }

        /* disk space values */
        case DI_FMT_BTOT: {
          temp = DI_GT ("Size");
          break;
        }
        case DI_FMT_BTOT_AVAIL: {
          temp = DI_GT ("Size");
          break;
        }
        case DI_FMT_BUSED: {
          temp = DI_GT ("Used");
          break;
        }
        case DI_FMT_BCUSED: {
          temp = DI_GT ("Used");
          break;
        }
        case DI_FMT_BFREE: {
          temp = DI_GT ("Free");
          break;
        }
        case DI_FMT_BAVAIL: {
          temp = DI_GT ("Avail");
          break;
        }

        /* disk space percentages */
        case DI_FMT_BPERC_NAVAIL: {
          temp = DI_GT ("%Used");
          break;
        }
        case DI_FMT_BPERC_USED: {
          temp = DI_GT ("%Used");
          break;
        }
        case DI_FMT_BPERC_BSD: {
          temp = DI_GT ("%Used");
          break;
        }
        case DI_FMT_BPERC_AVAIL: {
          temp = DI_GT ("%Free");
          break;
        }
        case DI_FMT_BPERC_FREE: {
          temp = DI_GT ("%Free");
          break;
        }

        /* inode information */
        case DI_FMT_ITOT: {
          temp = DI_GT ("Inodes");
          break;
        }
        case DI_FMT_IUSED: {
          temp = DI_GT ("IUsed");
          break;
        }
        case DI_FMT_IFREE: {
          temp = DI_GT ("IFree");
          break;
        }
        case DI_FMT_IPERC: {
          temp = DI_GT ("%IUsed");
          break;
        }
        default: {
          break;
        }
      }
    }

    strdata [stridx] = strdup (temp);
    ++fmtcount;
  }

  return;
}

static Size_t
istrlen (const char *str)
{
  Size_t            len;
#if _lib_mbrlen
  Size_t            mlen;
  Size_t            slen;
  mbstate_t         ps;
  const char        *tstr;

  len = 0;
  memset (&ps, 0, sizeof (mbstate_t));
  slen = strlen (str);
  tstr = str;
  while (slen > 0) {
    mlen = mbrlen (tstr, slen, &ps);
    if ( (int) mlen <= 0) {
      return strlen (str);
    }
    ++len;
    tstr += mlen;
    slen -= mlen;
  }
#else
  len = strlen (str);
#endif
  return len;
}

