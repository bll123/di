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
 *      1 - bytes
 *      512 - posix (512 bytes)
 *      k - kilo
 *      m - mega
 *      g - giga
 *      t - tera
 *      p - peta
 *      e - exa
 *      z - zetta
 *      r - ronna
 *      q - quetta
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
 *    Disk Space Percentages
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
 *    Inodes
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
  char            **strdata;
} di_disp_info_t;

typedef struct {
  char    *si_suffix;
  char    *si_name;
  char    *suffix;
  char    *name;
} di_disp_text_t;

static di_disp_text_t disptext [] =
{
  { "",  "Byte", "",  "Byte" },
  { "k", "Kilo", "ki", "Kibi" },
  { "M", "Mega", "Mi", "Mebi" },
  { "G", "Giga", "Gi", "Gibi" },
  { "T", "Tera", "Ti", "Tebi" },
  { "P", "Peta", "Pi", "Pebi" },
  { "E", "Exa", "Ei", "Exbi" },
  { "Z", "Zetta", "Zi", "Zebi" },
  { "Y", "Yotta", "Yi", "Yobi" },
  { "R", "Ronna", "Ri", "Ronni" },
  { "Q", "Quetta", "Qi", "Quetti" }
};
#define DI_DISPTEXT_SZ ( (int) (sizeof (disptext) / sizeof (di_disp_text_t)))

static void di_display_data (void *);
static void di_display_header (void *, di_disp_info_t *);
static void usage (void);
static Size_t istrlen (const char *str);
static void updateScaleValues (void *di_data, int iterval, di_disp_info_t *dispinfo);
static void determineMaxScaleValue (void *di_data, int iterval, di_disp_info_t *dispinfo);

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
  di_get_all_disk_info (di_data);
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
  int                 displinecount;
  int                 totline;
  int                 dispcount;
  int                 csvout;
  int                 csvtabs;
  int                 jsonout;
  int                 scaleidx;
  int                 scalehr;
  int                 blksz;
  char                temp [MAXPATHLEN * 2];
  di_disp_info_t      dispinfo;
  char                **strdata;

  csvout = di_check_option (di_data, DI_OPT_DISP_CSV);
  csvtabs = di_check_option (di_data, DI_OPT_DISP_CSV_TAB);
  jsonout = di_check_option (di_data, DI_OPT_DISP_JSON);
  fmtstrlen = di_check_option (di_data, DI_OPT_FMT_STR_LEN);
  blksz = di_check_option (di_data, DI_OPT_BLOCK_SZ);
  scaleidx = di_check_option (di_data, DI_OPT_SCALE);
  scalehr = 0;
  if (scaleidx == DI_SCALE_HR || scaleidx == DI_SCALE_HR_ALT) {
    scalehr = 1;
  }

  iterval = di_check_option (di_data, DI_OPT_DISP_ALL);
  displinecount = di_iterate_init (di_data, iterval);
  if (di_check_option (di_data, DI_OPT_DISP_HEADER)) {
    ++displinecount;
  }
  if (di_check_option (di_data, DI_OPT_DISP_TOTALS)) {
    totline = displinecount - 1;
  }

  dispinfo.maxlen = malloc (sizeof (unsigned int) * fmtstrlen);
  dispinfo.scaleidx = malloc (sizeof (unsigned int) * displinecount * fmtstrlen);
  dispinfo.suffix = malloc (sizeof (char *) * displinecount * fmtstrlen);
  dispinfo.leftjust = malloc (sizeof (unsigned int) * fmtstrlen);
  dispinfo.jsonident = malloc (sizeof (char *) * fmtstrlen);
  dispinfo.strdata = malloc (sizeof (char *) * displinecount * fmtstrlen);
  strdata = dispinfo.strdata;

  for (i = 0; i < fmtstrlen; ++i) {
    dispinfo.maxlen [i] = DI_SCALE_GIGA;
    dispinfo.leftjust [i] = 0;
    dispinfo.jsonident [i] = NULL;
  }
  for (i = 0; i < displinecount; ++i) {
    int   j;

    for (j = 0; j < fmtstrlen; ++j) {
      int       idx;

      idx = i * fmtstrlen + j;
      dispinfo.scaleidx [idx] = scaleidx;
      dispinfo.suffix [idx] = "";
      dispinfo.strdata [idx] = NULL;
    }
  }

  dispcount = 0;

  if (di_check_option (di_data, DI_OPT_DISP_HEADER)) {
    di_display_header (di_data, &dispinfo);
    dispcount = 1;
  }

  if (jsonout) {
    fprintf (stdout, "{\n");
    if (scalehr) {
      fprintf (stdout, "  \"scaling\" : \"human\",\n");
    } else {
      fprintf (stdout, "  \"scaling\" : \"%s\",\n", disptext [scaleidx].si_suffix);
    }
    fprintf (stdout, "  \"blocksize\" : \"%d\",\n", blksz);
    fprintf (stdout, "  \"partitions\" : [\n");
  }

  if (scalehr) {
    updateScaleValues (di_data, iterval, &dispinfo);
    if (scaleidx == DI_SCALE_HR_ALT) {
      determineMaxScaleValue (di_data, iterval, &dispinfo);
    }
  }

  di_iterate_init (di_data, iterval);
  while ( (pub = di_iterate (di_data)) != NULL) {
    int         fmt;
    int         fmtcount;
    int         dataidx;

    fmtcount = 0;
    di_format_iter_init (di_data);
    while ( (fmt = di_format_iterate (di_data)) != DI_FMT_ITER_STOP) {
      dataidx = dispcount * fmtstrlen + fmtcount;

      switch (fmt) {
        /* string values */
        case DI_FMT_MOUNT:
        case DI_FMT_MOUNT_FULL: {
          dispinfo.jsonident [fmtcount] = "mount";
          dispinfo.leftjust [fmtcount] = 1;
          if (dispcount == totline) {
            strdata [dataidx] = strdup (DI_GT ("Totals"));
          } else {
            strdata [dataidx] = strdup (pub->strdata [DI_DISP_MOUNTPT]);
          }
          break;
        }
        case DI_FMT_FILESYSTEM:
        case DI_FMT_FILESYSTEM_FULL: {
          dispinfo.leftjust [fmtcount] = 1;
          dispinfo.jsonident [fmtcount] = "filesystem";
          if (dispcount == totline) {
            strdata [dataidx] = strdup ("");
          } else {
            strdata [dataidx] = strdup (pub->strdata [DI_DISP_FILESYSTEM]);
          }
          break;
        }
        case DI_FMT_FSTYPE:
        case DI_FMT_FSTYPE_FULL: {
          dispinfo.leftjust [fmtcount] = 1;
          dispinfo.jsonident [fmtcount] = "fstype";
          if (dispcount == totline) {
            strdata [dataidx] = strdup ("");
          } else {
            strdata [dataidx] = strdup (pub->strdata [DI_DISP_FSTYPE]);
          }
          break;
        }
        case DI_FMT_MOUNT_OPTIONS: {
          dispinfo.leftjust [fmtcount] = 1;
          dispinfo.jsonident [fmtcount] = "options";
          if (dispcount == totline) {
            strdata [dataidx] = strdup ("");
          } else {
            strdata [dataidx] = strdup (pub->strdata [DI_DISP_MOUNTOPT]);
          }
          break;
        }

        /* disk space values */
        case DI_FMT_BTOT: {
          dispinfo.jsonident [fmtcount] = "size";
          if (scalehr) {
            dispinfo.suffix [dataidx] =
                disptext [dispinfo.scaleidx [dataidx]].si_suffix;
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
            dispinfo.suffix [dataidx] =
                disptext [dispinfo.scaleidx [dataidx]].si_suffix;
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
            dispinfo.suffix [dataidx] =
                disptext [dispinfo.scaleidx [dataidx]].si_suffix;
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
            dispinfo.suffix [dataidx] =
                disptext [dispinfo.scaleidx [dataidx]].si_suffix;
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
            dispinfo.suffix [dataidx] =
                disptext [dispinfo.scaleidx [dataidx]].si_suffix;
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
            dispinfo.suffix [dataidx] =
                disptext [dispinfo.scaleidx [dataidx]].si_suffix;
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
          di_disp_scaled (di_data, temp, sizeof (temp), pub->index,
              DI_SCALE_BYTE, DI_INODE_TOTAL, DI_VALUE_NONE, DI_VALUE_NONE);
          strdata [dataidx] = strdup (temp);
          break;
        }
        case DI_FMT_IUSED: {
          dispinfo.jsonident [fmtcount] = "inodesused";
          di_disp_scaled (di_data, temp, sizeof (temp), pub->index,
              DI_SCALE_BYTE, DI_INODE_TOTAL, DI_INODE_FREE, DI_VALUE_NONE);
          strdata [dataidx] = strdup (temp);
          break;
        }
        case DI_FMT_IFREE: {
          dispinfo.jsonident [fmtcount] = "inodesfree";
          di_disp_scaled (di_data, temp, sizeof (temp), pub->index,
              DI_SCALE_BYTE, DI_INODE_FREE, DI_VALUE_NONE, DI_VALUE_NONE);
          strdata [dataidx] = strdup (temp);
          break;
        }
        case DI_FMT_IPERC: {
          dispinfo.jsonident [fmtcount] = "percinodesused";
          di_disp_perc (di_data, temp, sizeof (temp), pub->index,
              DI_INODE_TOTAL, DI_INODE_AVAIL,
              DI_INODE_TOTAL, DI_VALUE_NONE, DI_VALUE_NONE);
          strdata [dataidx] = strdup (temp);
          dispinfo.suffix [dataidx] = "%";
          break;
        }
        default: {
          dispinfo.leftjust [fmtcount] = 1;
          dispinfo.jsonident [fmtcount] = NULL;
          temp [0] = fmt;
          temp [1] = '\0';
          strdata [dataidx] = strdup (temp);
          dispinfo.suffix [dataidx] = "";
          break;
        }
      }

      ++fmtcount;
    }

    ++dispcount;
  }

  if (! csvout && ! jsonout) {
    for (i = 0; i < displinecount; ++i) {
      int     j;

      for (j = 0; j < fmtstrlen; ++j) {
        int     dataidx;
        Size_t  len;

        dataidx = i * fmtstrlen + j;
        if (strdata [dataidx] != NULL) {
          len = istrlen (strdata [dataidx]);
          /* this is an assumption */
          /* if the non-si suffixes are implemented, this needs to change */
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

  for (i = 0; i < displinecount; ++i) {
    int         j;
    const char  *comma;
    int         fmtchar;

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

      fmtchar = 1;
      if (dispinfo.jsonident [j] == NULL) {
        fmtchar = 0;
      }

      if (fmtchar) {
        if (csvout) {
          if (csvtabs) {
            fprintf (stdout, "%s", tmp);
          } else {
            fprintf (stdout, "\"%s\"", tmp);
          }
        } else if (jsonout) {
          fprintf (stdout, "      \"%s\" : \"%s%s\"%s", dispinfo.jsonident [j],
              tmp, dispinfo.suffix [dataidx], comma);
        }
      }
      if (! csvout && ! jsonout) {
        int       len;

        len = dispinfo.maxlen [j];
        if (*dispinfo.suffix [dataidx]) {
          --len;
        }
        if (j == fmtstrlen - 1 && dispinfo.leftjust [j]) {
          fprintf (stdout, "%s%s", tmp, dispinfo.suffix [dataidx]);
        } else {
          if (dispinfo.leftjust [j]) {
            fprintf (stdout, "%-*s%s", len, tmp, dispinfo.suffix [dataidx]);
          } else {
            fprintf (stdout, "%*s%s", len, tmp, dispinfo.suffix [dataidx]);
          }
        }
      }

      if (fmtchar) {
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
          } /* not the last format character */
        } /* not json, json output is per-line */
      } /* is a standard format character */
    } /* for each format character */

    if (jsonout) {
      fprintf (stdout, "    }");
      if (i != displinecount - 1) {
        fprintf (stdout, ",");
      }
    }
    fprintf (stdout, "\n");
  }

  if (jsonout) {
    fprintf (stdout, "  ]\n");
    fprintf (stdout, "}\n");
  }

  for (i = 0; i < displinecount; ++i) {
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
di_display_header (void *di_data, di_disp_info_t *dispinfo)
{
  int         fmt;
  int         fmtcount;
  int         fmtstrlen;
  int         csvout;
  int         scaleidx;
  int         scalehr;
  int         posixcompat;
  char        **strdata = dispinfo->strdata;

  csvout = di_check_option (di_data, DI_OPT_DISP_CSV);
  fmtstrlen = di_check_option (di_data, DI_OPT_FMT_STR_LEN);
  posixcompat = di_check_option (di_data, DI_OPT_POSIX_COMPAT);
  fmtcount = 0;
  scaleidx = di_check_option (di_data, DI_OPT_SCALE);
  scalehr = 0;
  if (scaleidx == DI_SCALE_HR || scaleidx == DI_SCALE_HR_ALT) {
    scalehr = 1;
  }

  di_format_iter_init (di_data);
  while ( (fmt = di_format_iterate (di_data)) != DI_FMT_ITER_STOP) {
    const char  *temp;
    char        tbuff [2];
    int         dataidx;

    temp = "";
    dataidx = fmtcount;
    if (csvout) {
      tbuff [0] = fmt;
      tbuff [1] = '\0';
      temp = tbuff;
    }

    if (! csvout) {
      switch (fmt) {
        /* string values */
        case DI_FMT_MOUNT:
        case DI_FMT_MOUNT_FULL: {
          if (posixcompat) {
            temp = DI_GT ("Mounted On");
          } else {
            temp = DI_GT ("Mount");
          }
          break;
        }
        case DI_FMT_FILESYSTEM:
        case DI_FMT_FILESYSTEM_FULL: {
          temp = DI_GT ("Filesystem");
          break;
        }
        case DI_FMT_FSTYPE:
        case DI_FMT_FSTYPE_FULL: {
          temp = DI_GT ("Type");
          break;
        }
        case DI_FMT_MOUNT_OPTIONS: {
          temp = DI_GT ("Options");
          break;
        }

        /* disk space values */
        case DI_FMT_BTOT: {
          int   blksz;

          blksz = di_check_option (di_data, DI_OPT_BLOCK_SZ);
          if (posixcompat) {
            temp = DI_GT ("1024-blocks");
          } else if (scalehr || blksz == 1) {
            temp = DI_GT ("Size");
          } else {
            if (blksz == 1000) {
              temp = disptext [scaleidx].si_name;
            } else if (blksz == 1024) {
              temp = disptext [scaleidx].name;
            }
          }
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
          if (posixcompat) {
            temp = DI_GT ("Available");
          } else {
            temp = DI_GT ("Avail");
          }
          break;
        }

        /* disk space percentages */
        case DI_FMT_BPERC_NAVAIL: {
          if (posixcompat) {
            temp = DI_GT ("Capacity");
          } else {
            temp = DI_GT ("%Used");
          }
          break;
        }
        case DI_FMT_BPERC_USED: {
          if (posixcompat) {
            temp = DI_GT ("Capacity");
          } else {
            temp = DI_GT ("%Used");
          }
          break;
        }
        case DI_FMT_BPERC_BSD: {
          if (posixcompat) {
            temp = DI_GT ("Capacity");
          } else {
            temp = DI_GT ("%Used");
          }
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
          tbuff [0] = fmt;
          tbuff [1] = '\0';
          temp = tbuff;
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

static void
updateScaleValues (void *di_data, int iterval,
    di_disp_info_t *dispinfo)
{
  di_pub_disk_info_t  *pub;
  int                 dispcount;
  int                 fmtstrlen;

  dispcount = 0;
  if (di_check_option (di_data, DI_OPT_DISP_HEADER)) {
    dispcount = 1;
  }
  fmtstrlen = di_check_option (di_data, DI_OPT_FMT_STR_LEN);

  di_iterate_init (di_data, iterval);
  while ((pub = di_iterate (di_data)) != NULL) {
    int         fmt;
    int         fmtcount;
    int         dataidx;

    fmtcount = 0;

    di_format_iter_init (di_data);
    while ((fmt = di_format_iterate (di_data)) != DI_FMT_ITER_STOP) {
      dataidx = dispcount * fmtstrlen + fmtcount;

      switch (fmt) {
        /* disk space values */
        case DI_FMT_BTOT: {
          dispinfo->scaleidx [dataidx] = di_get_scale_max (di_data,
              pub->index, DI_SPACE_TOTAL, DI_VALUE_NONE, DI_VALUE_NONE);
          break;
        }
        case DI_FMT_BTOT_AVAIL: {
          dispinfo->scaleidx [dataidx] = di_get_scale_max (di_data,
              pub->index, DI_SPACE_TOTAL, DI_SPACE_FREE, DI_SPACE_AVAIL);
          break;
        }
        case DI_FMT_BUSED: {
          dispinfo->scaleidx [dataidx] = di_get_scale_max (di_data,
              pub->index, DI_SPACE_TOTAL, DI_SPACE_FREE, DI_VALUE_NONE);
          break;
        }
        case DI_FMT_BCUSED: {
          dispinfo->scaleidx [dataidx] = di_get_scale_max (di_data,
              pub->index, DI_SPACE_TOTAL, DI_SPACE_AVAIL, DI_VALUE_NONE);
          break;
        }
        case DI_FMT_BFREE: {
          dispinfo->scaleidx [dataidx] = di_get_scale_max (di_data,
              pub->index, DI_SPACE_FREE, DI_VALUE_NONE, DI_VALUE_NONE);
          break;
        }
        case DI_FMT_BAVAIL: {
          dispinfo->scaleidx [dataidx] = di_get_scale_max (di_data,
              pub->index, DI_SPACE_AVAIL, DI_VALUE_NONE, DI_VALUE_NONE);
          break;
        }
        default: {
          break;
        }
      }

      ++fmtcount;
    }

    ++dispcount;
  }

}

static void
determineMaxScaleValue (void *di_data, int iterval,
    di_disp_info_t *dispinfo)
{
  di_pub_disk_info_t  *pub;
  int                 dispcount;
  int                 fmtstrlen;

  dispcount = 0;
  if (di_check_option (di_data, DI_OPT_DISP_HEADER)) {
    dispcount = 1;
  }
  fmtstrlen = di_check_option (di_data, DI_OPT_FMT_STR_LEN);

  di_iterate_init (di_data, iterval);
  while ((pub = di_iterate (di_data)) != NULL) {
    int         fmt;
    int         fmtcount;
    int         dataidx;
    int         maxscaleidx;
    int         scaleidx;

    fmtcount = 0;
    maxscaleidx = DI_SCALE_BYTE;

    di_format_iter_init (di_data);
    while ((fmt = di_format_iterate (di_data)) != DI_FMT_ITER_STOP) {
      dataidx = dispcount * fmtstrlen + fmtcount;

      switch (fmt) {
        /* disk space values */
        case DI_FMT_BTOT:
        case DI_FMT_BTOT_AVAIL:
        case DI_FMT_BUSED:
        case DI_FMT_BCUSED:
        case DI_FMT_BFREE:
        case DI_FMT_BAVAIL: {
          scaleidx = dispinfo->scaleidx [dataidx];
          break;
        }
        default: {
          scaleidx = DI_SCALE_BYTE;
          break;
        }
      }

      if (scaleidx > maxscaleidx) {
        maxscaleidx = scaleidx;
      }

      ++fmtcount;
    }

    /* and loop through again, and set the scaleidx to the max scaleidx */
    fmtcount = 0;

    di_format_iter_init (di_data);
    while ((fmt = di_format_iterate (di_data)) != DI_FMT_ITER_STOP) {
      dataidx = dispcount * fmtstrlen + fmtcount;

      switch (fmt) {
        /* disk space values */
        case DI_FMT_BTOT:
        case DI_FMT_BTOT_AVAIL:
        case DI_FMT_BUSED:
        case DI_FMT_BCUSED:
        case DI_FMT_BFREE:
        case DI_FMT_BAVAIL: {
          dispinfo->scaleidx [dataidx] = maxscaleidx;
          break;
        }
        default: {
          break;
        }
      }

      ++fmtcount;
    }

    ++dispcount;
  }
}
