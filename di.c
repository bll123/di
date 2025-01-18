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
 *      s - filesystem (device name / special)
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

typedef struct {
  unsigned int    *maxlen;
  unsigned int    *scaleidx;
  const char      **suffix;
  unsigned int    *leftjust;
  const char      **jsonident;
} di_disp_info_t;

typedef struct {
  char    *uc;
  char    *lc;
  char    *name;
  char    *nameb;
} di_disp_text_t;

static di_disp_text_t disptext [] =
{
  { " ", " ", "Byte", "Byte" },
  { "K", "k", "Kilo", "Kibi" },
  { "M", "m", "Mega", "Mebi" },
  { "G", "g", "Giga", "Gibi" },
  { "T", "t", "Tera", "Tebi" },
  { "P", "p", "Peta", "Pebi" },
  { "E", "e", "Exa", "Exbi" },
  { "Z", "z", "Zetta", "Zebi" },
  { "Y", "y", "Yotta", "Yobi" },
  { "R", "r", "Ronna", "Ronni" },
  { "Q", "q", "Quetta", "Quetti" }
};
#define DI_DISPTEXT_SZ ( (int) (sizeof (disptext) / sizeof (di_disp_text_t)))

static void di_display_data (void *);
static void di_display_header (void *, char **);
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
        fprintf (stdout, DI_GT ("di version %s\n"), DI_VERSION);
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
  fprintf (stdout, DI_GT ("di version %s\n"), DI_VERSION);
             /*  12345678901234567890123456789012345678901234567890123456789012345678901234567890 */
  fprintf (stdout, DI_GT ("Usage: di [-ant] [-d display-size] [-f format] [-x exclude-fstyp-list]\n"));
  fprintf (stdout, DI_GT ("       [-I include-fstyp-list] [file [...]]\n"));
  fprintf (stdout, DI_GT ("   -a   : print all mounted devices\n"));
  fprintf (stdout, DI_GT ("   -d x : size to print blocks in (k,m,g,t,etc.)\n"));
  fprintf (stdout, DI_GT ("          h - human readable.\n"));
  fprintf (stdout, DI_GT ("   -f x : use format string <x>\n"));
  fprintf (stdout, DI_GT ("   -I x : include only file system types in <x>\n"));
  fprintf (stdout, DI_GT ("   -x x : exclude file system types in <x>\n"));
  fprintf (stdout, DI_GT ("   -l   : display local filesystems only\n"));
  fprintf (stdout, DI_GT ("   -n   : don't print header\n"));
  fprintf (stdout, DI_GT ("   -t   : print totals\n"));
  fprintf (stdout, DI_GT (" Format string values:\n"));
  fprintf (stdout, DI_GT ("    m - mount point\n"));
  fprintf (stdout, DI_GT ("    d - device name\n"));
  fprintf (stdout, DI_GT ("    t - file-system type\n"));
  fprintf (stdout, DI_GT ("    b - total kbytes                    B - kbytes available for use\n"));
  fprintf (stdout, DI_GT ("    u - used kbytes                     c - calculated kbytes in use\n"));
  fprintf (stdout, DI_GT ("    f - kbytes free                     v - kbytes available\n"));
  fprintf (stdout, DI_GT ("    p - percentage not avail. for use   1 - percentage used\n"));
  fprintf (stdout, DI_GT ("    2 - percentage of user-available space in use.\n"));
  fprintf (stdout, DI_GT ("    i - total file slots (i-nodes)      U - used file slots\n"));
  fprintf (stdout, DI_GT ("    F - free file slots                 P - percentage file slots used\n"));
  fprintf (stdout, DI_GT ("See manual page for more options.\n"));
}

static void
di_display_data (void *di_data)
{
  di_pub_disk_info_t  *pub;
  int                 i;
  int                 iterval;
  int                 fmtstrlen;
  int                 count;
  int                 dispcount;
  char                **strdata;
  int                 csvout;
  int                 csvtabs;
  int                 jsonout;
  int                 scaleidx;
  int                 scalehr;
  char                temp [MAXPATHLEN * 2];
  di_disp_info_t      dispinfo;

  csvout = di_check_option (di_data, DI_OPT_DISP_CSV);
  csvtabs = di_check_option (di_data, DI_OPT_DISP_CSV_TAB);
  jsonout = di_check_option (di_data, DI_OPT_DISP_JSON);
  fmtstrlen = di_check_option (di_data, DI_OPT_FMT_STR_LEN);
  scaleidx = di_check_option (di_data, DI_OPT_SCALE);
  scalehr = 0;
  if (scaleidx == DI_SCALE_HR || scaleidx == DI_SCALE_HR_ALT) {
    scalehr = 1;
  }

  iterval = di_check_option (di_data, DI_OPT_DISP_ALL);
  count = di_iterate_init (di_data, iterval);
  if (di_check_option (di_data, DI_OPT_DISP_HEADER)) {
    ++count;
  }

  dispinfo.maxlen = malloc (sizeof (unsigned int) * fmtstrlen);
  dispinfo.scaleidx = malloc (sizeof (unsigned int) * count * fmtstrlen);
  dispinfo.suffix = malloc (sizeof (char *) * count * fmtstrlen);
  dispinfo.leftjust = malloc (sizeof (unsigned int) * fmtstrlen);
  dispinfo.jsonident = malloc (sizeof (char *) * fmtstrlen);
  for (i = 0; i < fmtstrlen; ++i) {
    dispinfo.maxlen [i] = DI_SCALE_GIGA;
    dispinfo.leftjust [i] = 0;
    dispinfo.jsonident [i] = NULL;
  }
  for (i = 0; i < count; ++i) {
    int   j;

    for (j = 0; j < fmtstrlen; ++j) {
      int       idx;

      idx = i * fmtstrlen + j;
      dispinfo.scaleidx [idx] = scaleidx;
      dispinfo.suffix [idx] = "";
    }
  }

  strdata = malloc (sizeof (char *) * count * fmtstrlen);

  dispcount = 0;

  if (di_check_option (di_data, DI_OPT_DISP_HEADER)) {
    di_display_header (di_data, strdata);
    dispcount = 1;
  }

  if (jsonout) {
    fprintf (stdout, "{\n");
    if (scalehr) {
      fprintf (stdout, "  \"scaling\" : \"human\",\n");
    } else {
      fprintf (stdout, "  \"scaling\" : \"%s\"\n", disptext [scaleidx].uc);
    }
    fprintf (stdout, "  \"partitions\" : [\n");
  }

  di_iterate_init (di_data, iterval);
  while ( (pub = di_iterate (di_data)) != NULL) {
    int         fmt;
    int         fmtcount;
    int         dataidx;

    di_format_iter_init (di_data);
    fmtcount = 0;
    while ( (fmt = di_format_iterate (di_data)) != DI_FMT_ITER_STOP) {
      dataidx = dispcount * fmtstrlen + fmtcount;
      strdata [dataidx] = NULL;

      switch (fmt) {
        /* string values */
        case DI_FMT_MOUNT:
        case DI_FMT_MOUNT_OLD: {
          dispinfo.jsonident [fmtcount] = "mount";
          dispinfo.leftjust [fmtcount] = 1;
          strdata [dataidx] = strdup (pub->strdata [DI_DISP_MOUNTPT]);
          break;
        }
        case DI_FMT_FILESYSTEM:
        case DI_FMT_FILESYSTEM_OLD: {
          dispinfo.leftjust [fmtcount] = 1;
          dispinfo.jsonident [fmtcount] = "filesystem";
          strdata [dataidx] = strdup (pub->strdata [DI_DISP_FILESYSTEM]);
          break;
        }
        case DI_FMT_FSTYPE:
        case DI_FMT_FSTYPE_OLD: {
          dispinfo.leftjust [fmtcount] = 1;
          dispinfo.jsonident [fmtcount] = "fstype";
          strdata [dataidx] = strdup (pub->strdata [DI_DISP_FSTYPE]);
          break;
        }
        case DI_FMT_MOUNT_OPTIONS: {
          dispinfo.leftjust [fmtcount] = 1;
          dispinfo.jsonident [fmtcount] = "options";
          strdata [dataidx] = strdup (pub->strdata [DI_DISP_MOUNTOPT]);
          break;
        }

        /* disk space values */
        case DI_FMT_BTOT: {
          dispinfo.jsonident [fmtcount] = "size";
          if (scalehr) {
            dispinfo.scaleidx [dataidx] = di_get_scale_max (di_data,
                pub->index, DI_SPACE_TOTAL, DI_VALUE_NONE, DI_VALUE_NONE);
            dispinfo.suffix [dataidx] =
                disptext [dispinfo.scaleidx [dataidx]].uc;
          }
          di_disp_scaled (di_data, temp, sizeof (temp), pub->index,
              dispinfo.scaleidx [dataidx],
              DI_SPACE_TOTAL, DI_VALUE_NONE, DI_VALUE_NONE);
          strdata [dataidx] = strdup (temp);
          break;
        }
        case DI_FMT_BTOT_AVAIL: {
          dispinfo.jsonident [fmtcount] = "size";
          if (scalehr) {
            dispinfo.scaleidx [dataidx] = di_get_scale_max (di_data,
                pub->index, DI_SPACE_TOTAL, DI_SPACE_FREE, DI_SPACE_AVAIL);
            dispinfo.suffix [dataidx] =
                disptext [dispinfo.scaleidx [dataidx]].uc;
          }
          di_disp_scaled (di_data, temp, sizeof (temp), pub->index,
              dispinfo.scaleidx [dataidx],
              DI_SPACE_TOTAL, DI_SPACE_FREE, DI_SPACE_AVAIL);
          strdata [dataidx] = strdup (temp);
          break;
        }
        case DI_FMT_BUSED: {
          dispinfo.jsonident [fmtcount] = "used";
          if (scalehr) {
            dispinfo.scaleidx [dataidx] = di_get_scale_max (di_data,
                pub->index, DI_SPACE_TOTAL, DI_SPACE_FREE, DI_VALUE_NONE);
            dispinfo.suffix [dataidx] =
                disptext [dispinfo.scaleidx [dataidx]].uc;
          }
          di_disp_scaled (di_data, temp, sizeof (temp), pub->index,
              dispinfo.scaleidx [dataidx],
              DI_SPACE_TOTAL, DI_SPACE_FREE, DI_VALUE_NONE);
          strdata [dataidx] = strdup (temp);
          break;
        }
        case DI_FMT_BCUSED: {
          dispinfo.jsonident [fmtcount] = "used";
          if (scalehr) {
            dispinfo.scaleidx [dataidx] = di_get_scale_max (di_data,
                pub->index, DI_SPACE_TOTAL, DI_SPACE_AVAIL, DI_VALUE_NONE);
            dispinfo.suffix [dataidx] =
                disptext [dispinfo.scaleidx [dataidx]].uc;
          }
          di_disp_scaled (di_data, temp, sizeof (temp), pub->index,
              dispinfo.scaleidx [dataidx],
              DI_SPACE_TOTAL, DI_SPACE_AVAIL, DI_VALUE_NONE);
          strdata [dataidx] = strdup (temp);
          break;
        }
        case DI_FMT_BFREE: {
          dispinfo.jsonident [fmtcount] = "free";
          if (scalehr) {
            dispinfo.scaleidx [dataidx] = di_get_scale_max (di_data,
                pub->index, DI_SPACE_FREE, DI_VALUE_NONE, DI_VALUE_NONE);
            dispinfo.suffix [dataidx] =
                disptext [dispinfo.scaleidx [dataidx]].uc;
          }
          di_disp_scaled (di_data, temp, sizeof (temp), pub->index,
              dispinfo.scaleidx [dataidx],
              DI_SPACE_FREE, DI_VALUE_NONE, DI_VALUE_NONE);
          strdata [dataidx] = strdup (temp);
          break;
        }
        case DI_FMT_BAVAIL: {
          dispinfo.jsonident [fmtcount] = "available";
          if (scalehr) {
            dispinfo.scaleidx [dataidx] = di_get_scale_max (di_data,
                pub->index, DI_SPACE_AVAIL, DI_VALUE_NONE, DI_VALUE_NONE);
            dispinfo.suffix [dataidx] =
                disptext [dispinfo.scaleidx [dataidx]].uc;
          }
          di_disp_scaled (di_data, temp, sizeof (temp), pub->index,
              dispinfo.scaleidx [dataidx],
              DI_SPACE_AVAIL, DI_VALUE_NONE, DI_VALUE_NONE);
          strdata [dataidx] = strdup (temp);
          break;
        }

        /* disk space percentages */
        case DI_FMT_BPERC_NAVAIL: {
          dispinfo.jsonident [fmtcount] = "percused";
          di_disp_perc (di_data, temp, sizeof (temp), pub->index,
              DI_SPACE_TOTAL, DI_SPACE_AVAIL,
              DI_SPACE_TOTAL, DI_VALUE_NONE, DI_VALUE_NONE);
          strdata [dataidx] = strdup (temp);
          dispinfo.suffix [dataidx] = "%";
          break;
        }
        case DI_FMT_BPERC_USED: {
          dispinfo.jsonident [fmtcount] = "percused";
          di_disp_perc (di_data, temp, sizeof (temp), pub->index,
              DI_SPACE_TOTAL, DI_SPACE_FREE,
              DI_SPACE_TOTAL, DI_VALUE_NONE, DI_VALUE_NONE);
          strdata [dataidx] = strdup (temp);
          dispinfo.suffix [dataidx] = "%";
          break;
        }
        case DI_FMT_BPERC_BSD: {
          dispinfo.jsonident [fmtcount] = "percused";
          di_disp_perc (di_data, temp, sizeof (temp), pub->index,
              DI_SPACE_TOTAL, DI_SPACE_FREE,
              DI_SPACE_TOTAL, DI_SPACE_FREE, DI_SPACE_AVAIL);
          strdata [dataidx] = strdup (temp);
          dispinfo.suffix [dataidx] = "%";
          break;
        }
        case DI_FMT_BPERC_AVAIL: {
          dispinfo.jsonident [fmtcount] = "percfree";
          di_disp_perc (di_data, temp, sizeof (temp), pub->index,
              DI_SPACE_AVAIL, DI_VALUE_NONE,
              DI_SPACE_TOTAL, DI_VALUE_NONE, DI_VALUE_NONE);
          strdata [dataidx] = strdup (temp);
          dispinfo.suffix [dataidx] = "%";
          break;
        }
        case DI_FMT_BPERC_FREE: {
          dispinfo.jsonident [fmtcount] = "percfree";
          di_disp_perc (di_data, temp, sizeof (temp), pub->index,
              DI_SPACE_FREE, DI_VALUE_NONE,
              DI_SPACE_TOTAL, DI_VALUE_NONE, DI_VALUE_NONE);
          strdata [dataidx] = strdup (temp);
          dispinfo.suffix [dataidx] = "%";
          break;
        }

        /* inode information */
        case DI_FMT_ITOT: {
          dispinfo.jsonident [fmtcount] = "inodes";
          break;
        }
        case DI_FMT_IUSED: {
          dispinfo.jsonident [fmtcount] = "inodesused";
          break;
        }
        case DI_FMT_IFREE: {
          dispinfo.jsonident [fmtcount] = "inodesfree";
          break;
        }
        case DI_FMT_IPERC: {
          dispinfo.jsonident [fmtcount] = "percinodesused";
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
        int     dataidx;
        Size_t  len;

        dataidx = i * fmtstrlen + j;
        if (strdata [dataidx] != NULL) {
          len = istrlen (strdata [dataidx]);
          if (* (dispinfo.suffix [dataidx])) {
            ++len;
          }
          if (len > dispinfo.maxlen [j]) {
            dispinfo.maxlen [j] = len;
          }
        }
      }
    }
  }

  for (i = 0; i < count; ++i) {
    int         j;
    const char  *comma;

    if (jsonout) {
      fprintf (stdout, "    {\n");
    }
    comma = ",";
    for (j = 0; j < fmtstrlen; ++j) {
      int         dataidx;
      const char  *tmp;

      if (j == fmtstrlen - 1) {
        comma = "";
      }

      dataidx = i * fmtstrlen + j;
      tmp = strdata [dataidx];
      if (tmp == NULL) {
        tmp = "n/a";
      }

      if (csvout) {
        if (csvtabs) {
          fprintf (stdout, "%s", strdata [dataidx]);
        } else {
          fprintf (stdout, "\"%s\"", strdata [dataidx]);
        }
      } else if (jsonout) {
        fprintf (stdout, "      \"%s\" : \"%s%s\"%s", dispinfo.jsonident [j],
            strdata [dataidx], dispinfo.suffix [dataidx], comma);
      } else {
        int       len;

        len = dispinfo.maxlen [j];
        if (*dispinfo.suffix [dataidx]) {
          --len;
        }
        if (dispinfo.leftjust [j]) {
          fprintf (stdout, "%-*s%s", len, strdata [dataidx], dispinfo.suffix [dataidx]);
        } else {
          fprintf (stdout, "%*s%s", len, strdata [dataidx], dispinfo.suffix [dataidx]);
        }
      }

      if (jsonout) {
        fprintf (stdout, "\n");
      } else {
        if (j != fmtstrlen - 1) {
          if (csvout) {
            if (csvtabs) {
              fprintf (stdout, "\t");
            } else {
              fprintf (stdout, ",");
            }
          } else {
            fprintf (stdout, " ");
          }
        }
      }
    }
    if (jsonout) {
      fprintf (stdout, "    }");
      if (i != count - 1) {
        fprintf (stdout, ",");
      }
    }
    fprintf (stdout, "\n");
  }

  if (jsonout) {
    fprintf (stdout, "  ]\n");
    fprintf (stdout, "}\n");
  }

  for (i = 0; i < count; ++i) {
    int     j;

    for (j = 0; j < fmtstrlen; ++j) {
      int     dataidx;

      dataidx = i * fmtstrlen + j;
      if (strdata [dataidx] != NULL) {
        free (strdata [dataidx]);
      }
    }
  }

  free (dispinfo.maxlen);
  free (dispinfo.scaleidx);
  free (dispinfo.suffix);
  free (dispinfo.leftjust);
  free (dispinfo.jsonident);
  free (strdata);
}

static void
di_display_header (void *di_data, char **strdata)
{
  int         fmt;
  int         fmtcount;
  int         fmtstrlen;
  int         csvout;

  csvout = di_check_option (di_data, DI_OPT_DISP_CSV);
  fmtstrlen = di_check_option (di_data, DI_OPT_FMT_STR_LEN);
  fmtcount = 0;
  di_format_iter_init (di_data);
  while ( (fmt = di_format_iterate (di_data)) != DI_FMT_ITER_STOP) {
    const char  *temp;
    char        tcsv [2];
    int         dataidx;

    temp = "";
    dataidx = fmtcount;
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
        case DI_FMT_FILESYSTEM_OLD: {
          temp = DI_GT ("Filesystem");
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

    strdata [dataidx] = strdup (temp);
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

