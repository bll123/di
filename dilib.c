/*
 * Copyright 1994-2018 Brad Lanam, Walnut Creek, CA
 * Copyright 2023-2025 Brad Lanam, Pleasant Hill, CA
 */

#include "config.h"

#if _hdr_stdio
# include <stdio.h>
#endif
#if _hdr_stddef
# include <stddef.h>
#endif
#if _hdr_stdbool
# include <stdbool.h>
#endif
#if _hdr_stdlib
# include <stdlib.h>
#endif
#if _hdr_stdbool
# include <stdbool.h>
#endif
#if _sys_types \
    && ! defined (DI_INC_SYS_TYPES_H) /* xenix */
# define DI_INC_SYS_TYPES_H
# include <sys/types.h>
#endif
#if _hdr_ctype
# include <ctype.h>
#endif
#if _hdr_errno
# include <errno.h>
#endif
#if _hdr_string
# include <string.h>
#endif
#if _hdr_strings
# include <strings.h>
#endif
#if _sys_stat
# include <sys/stat.h>
#endif
#if _hdr_unistd
# include <unistd.h>
#endif
#if _hdr_memory
# include <memory.h>
#endif
#if _hdr_malloc
# include <malloc.h>
#endif
#if _hdr_zone
# include <zone.h>
#endif
#if _sys_file
# include <sys/file.h>
#endif
#if _hdr_fcntl && ! defined (DI_INC_FCNTL_H)    /* xenix */
// # define DI_INC_FCNTL_H
# include <fcntl.h>     /* O_RDONLY, O_NOCTTY */
#endif

#include "di.h"
#include "disystem.h"
#include "dimath.h"
#include "diinternal.h"
#include "dizone.h"
#include "diquota.h"
#include "dioptions.h"
#include "distrutils.h"

#if defined (__cplusplus) || defined (c_plusplus)
extern "C" {
#endif
#if _npt_getenv
  extern char *getenv (const char *);
#endif
#if defined (__cplusplus) || defined (c_plusplus)
}
#endif

/* end of system specific includes/configurations */

#define DI_UNKNOWN_DEV          -1L

int debug = 0;

#define DI_SORT_OPT_NONE            'n'
#define DI_SORT_OPT_MOUNT           'm'
#define DI_SORT_OPT_FILESYSTEM      's'
#define DI_SORT_OPT_TOTAL           'T'
#define DI_SORT_OPT_FREE            'f'
#define DI_SORT_OPT_AVAIL           'a'
#define DI_SORT_OPT_REVERSE         'r'
#define DI_SORT_OPT_TYPE            't'
#define DI_SORT_OPT_ASCENDING       1

static void checkDiskInfo       (di_data_t *, int);
static void checkDiskQuotas     (di_data_t *);
static int  checkFileInfo       (di_data_t *);
static int  getDiskSpecialInfo  (di_data_t *, int);
static void getDiskStatInfo     (di_data_t *);
static void preCheckDiskInfo    (di_data_t *);

static void checkIgnoreList     (di_disk_info_t *, di_strarr_t *);
static void checkIncludeList    (di_disk_info_t *, di_strarr_t *);
static int  isIgnoreFSType      (const char *);
static int  isIgnoreSpecial     (const char *);
static int  isIgnoreFS          (const char *, const char *);
static int  checkForUUID        (const char *);
static int  di_sort_compare           (const di_opt_t *, const char *sortType, const di_disk_info_t *, int, int);
static void checkZone (di_disk_info_t *, di_zone_info_t *, di_opt_t *);
static void di_sort_disk_info (di_opt_t *, di_disk_info_t *, int, const char *, int);
static void init_scale_values (di_data_t *, di_opt_t *);
static void di_calc_space (di_data_t *di_data, int infoidx, int validxA, int validxB, int validxC, dinum_t *val);
static double di_calc_perc (di_data_t *di_data, int infoidx, int validxA, int validxB, int validxC, int validxD, int validxE);
static void processTotals (di_data_t *di_data);
static void addTotals (const di_disk_info_t *dinfo, di_disk_info_t *totals, int inpool);

void *
di_initialize (void)
{
  di_data_t   *di_data;

  di_data = (di_data_t *) malloc (sizeof (di_data_t));

  di_data->scale_values_init = false;
  di_data->fscount = 0;
  di_data->dispcount = 0;
  di_data->haspooledfs = false;
  di_data->disppooledfs = false;
  di_data->totsorted = false;

  di_data->diskInfo = (di_disk_info_t *) NULL;

  /* options defaults */
  di_data->options = di_init_options ();
  di_data->pub = NULL;
  di_data->iteridx = 0;
  di_data->iteropt = 0;

  return di_data;
}

void
di_cleanup (void *tdi_data)
{
  di_data_t       *di_data = (di_data_t *) tdi_data;
  int             i;
  di_opt_t        *diopts;
  di_zone_info_t  *zinfo;

  if (di_data == NULL) {
    return;
  }

  if (di_data->diskInfo != (di_disk_info_t *) NULL) {
    di_disk_info_t  *dinfo;

    /* the totals bucket is at di_data->fscount */
    for (i = 0; i <= di_data->fscount; ++i) {
      dinfo = &di_data->diskInfo [i];
      di_free_disk_info (dinfo);
    }
    free (di_data->diskInfo);
  }

  if (di_data->pub != NULL) {
    free (di_data->pub);
  }

  diopts = (di_opt_t *) di_data->options;
  di_opt_cleanup (diopts);

  zinfo = (di_zone_info_t *) di_data->zoneInfo;
  di_free_zones (zinfo);

  if (di_data->scale_values_init) {
    for (i = 0; i < DI_SCALE_MAX; ++i) {
      dinum_clear (&di_data->scale_values [i]);
    }
  }

  free (di_data);
}

const char *
di_version (void)
{
  return DI_VERSION;
}

int
di_process_options (void *tdi_data, int argc, const char * argv [])
{
  di_data_t   *di_data = (di_data_t *) tdi_data;
  di_opt_t    *diopts;
  int         exitflag;

  if (di_data == NULL) {
    return DI_EXIT_FAIL;
  }

  diopts = (di_opt_t *) di_data->options;
  exitflag = di_get_options (argc, argv, diopts);

  if (debug > 0) {
    fprintf (stdout, "# BUILD: %s\n", DI_BUILD_SYS);
#if _use_math == DI_GMP
    fprintf (stdout, "# GMP:\n");
#elif _use_math == DI_TOMMATH
    fprintf (stdout, "# TOMMATH:\n");
#else
    fprintf (stdout, "# INTERNAL: ld:%d d:%d u64:%d ll:%d l:%d\n", _siz_long_double, _siz_double, _siz_uint64_t, _siz_long, _siz_long_long);
#endif
  }

  init_scale_values (di_data, diopts);

  return exitflag;
}

/* option processing */

int
di_check_option (void *tdi_data, int optidx)
{
  di_data_t   *di_data = (di_data_t *) tdi_data;
  di_opt_t    *diopts;

  if (di_data == NULL) {
    return 0;
  }

  diopts = (di_opt_t *) di_data->options;
  return di_opt_check_option (diopts, optidx);
}

extern void
di_format_iter_init (void *tdi_data)
{
  di_data_t   *di_data = (di_data_t *) tdi_data;
  di_opt_t    *diopts;

  if (di_data == NULL) {
    return;
  }

  diopts = (di_opt_t *) di_data->options;

  di_opt_format_iter_init (diopts);
}

extern int
di_format_iterate (void *tdi_data)
{
  di_data_t   *di_data = (di_data_t *) tdi_data;
  di_opt_t    *diopts;

  if (di_data == NULL) {
    return -1;
  }

  diopts = (di_opt_t *) di_data->options;

  return di_opt_format_iterate (diopts);
}

/* data processing */

void
di_get_all_disk_info (void *tdi_data)
{
  di_data_t   *di_data = (di_data_t *) tdi_data;
  di_opt_t    *diopts;
  int         hasLoop;

  if (di_data == NULL) {
    return;
  }

  /* initialization */
  diopts = (di_opt_t *) di_data->options;

  di_data->zoneInfo = di_initialize_zones ();

  if (debug > 0) {
    printf ("di version %s %s\n", DI_VERSION, DI_RELEASE_STATUS);
  }

  /* main processing */

  if (di_get_disk_entries (&di_data->diskInfo, &di_data->fscount) < 0) {
    di_cleanup (di_data);
    return;
  }

  di_data->dispcount = di_data->fscount;
  if (diopts->optval [DI_OPT_DISP_TOTALS]) {
    di_data->dispcount += 1;
  }

  hasLoop = false;
  preCheckDiskInfo (di_data);
  if (diopts->optidx < diopts->argc ||
      diopts->optval [DI_OPT_EXCL_LOOPBACK]) {
    getDiskStatInfo (di_data);
    hasLoop = getDiskSpecialInfo (di_data, diopts->optval [DI_OPT_NO_SYMLINK]);
  }
  if (diopts->optidx < diopts->argc) {
    int     rc;

    rc = checkFileInfo (di_data);
    if (rc < 0) {
      di_cleanup (di_data);
      diopts->exitFlag = DI_EXIT_WARN;
      return;
    }
  }
  di_get_disk_info (&di_data->diskInfo, &di_data->fscount);

  /* need the sort-by-filesystem before checkDiskInfo() is called */
  if ((di_data->haspooledfs || diopts->optval [DI_OPT_DISP_TOTALS]) &&
      ! di_data->totsorted) {
    di_sort_disk_info (diopts, di_data->diskInfo, di_data->fscount, "s", DI_SORT_TOTAL);
    di_data->totsorted = true;
  }

  if (strcmp (diopts->sortType, "n") != 0) {
    /* user's specified sort */
    di_sort_disk_info (diopts, di_data->diskInfo, di_data->fscount,
        diopts->sortType, DI_SORT_MAIN);
  }

  checkDiskInfo (di_data, hasLoop);

  if (diopts->optval [DI_OPT_QUOTA_CHECK] == true) {
    checkDiskQuotas (di_data);
  }

  di_initialize_disk_info (&di_data->diskInfo [di_data->fscount], di_data->fscount);
  di_data->diskInfo [di_data->fscount].doPrint = 0;
  di_data->diskInfo [di_data->fscount].printFlag = DI_PRNT_SKIP;
  if (diopts->optval [DI_OPT_DISP_TOTALS]) {
    processTotals (di_data);
  }
}

int
di_iterate_init (void *tdi_data, int iteropt)
{
  di_data_t       *di_data = (di_data_t *) tdi_data;
  di_disk_info_t  *dinfo;
  int             i;
  int             count;

  if (di_data->pub == NULL) {
    di_data->pub = (di_pub_disk_info_t *) malloc (sizeof (di_pub_disk_info_t));
  }
  di_data->iteridx = 0;
  di_data->iteropt = iteropt;

  count = di_data->dispcount;
  if (iteropt == DI_ITER_PRINTABLE) {
    count = 0;
    for (i = 0; i < di_data->dispcount; ++i) {
      dinfo = &di_data->diskInfo [i];
      if (dinfo->doPrint) {
        ++count;
      }
    }
  }

  return count;
}

di_pub_disk_info_t *
di_iterate (void *tdi_data)
{
  di_data_t           *di_data = (di_data_t *) tdi_data;
  di_pub_disk_info_t  *pub;
  di_disk_info_t      *dinfo;
  int                 i;
  int                 sortidx;

  if (di_data == NULL) {
    return NULL;
  }

  if (di_data->iteridx >= di_data->dispcount) {
    return NULL;
  }

  pub = (di_pub_disk_info_t *) di_data->pub;
  if (pub == NULL) {
    return NULL;
  }

  sortidx = di_data->diskInfo [di_data->iteridx].sortIndex [DI_SORT_MAIN];
  dinfo = & (di_data->diskInfo [sortidx]);

  if (di_data->iteropt == DI_ITER_PRINTABLE) {
    while (! dinfo->doPrint) {
      ++di_data->iteridx;
      if (di_data->iteridx >= di_data->dispcount) {
        break;
      }
      sortidx = di_data->diskInfo [di_data->iteridx].sortIndex [DI_SORT_MAIN];
      dinfo = & (di_data->diskInfo [sortidx]);
    }
  }

  if (di_data->iteridx >= di_data->dispcount) {
    return NULL;
  }

  /* populate pub structure */
  pub->index = sortidx;
  for (i = 0; i < DI_DISP_MAX; ++i) {
    pub->strdata [i] = dinfo->strdata [i];
  }
  pub->doPrint = dinfo->doPrint;
  pub->printFlag = dinfo->printFlag;
  pub->isLocal = dinfo->isLocal;
  pub->isReadOnly = dinfo->isReadOnly;
  pub->isLoopback = dinfo->isLoopback;

  ++di_data->iteridx;

  return pub;
}

int
di_get_scale_max (void *tdi_data, int infoidx,
    int validxA, int validxB, int validxC)
{
  di_data_t   *di_data = (di_data_t *) tdi_data;
  di_opt_t    *diopts;
  int         scaleidx;
  dinum_t     val;
  int         i;

  if (di_data == NULL) {
    return DI_SCALE_GIGA;
  }

  if (infoidx < 0 || infoidx > di_data->dispcount) {
    return DI_SCALE_GIGA;
  }

  diopts = (di_opt_t *) di_data->options;
  init_scale_values (di_data, diopts);

  dinum_init (&val);
  di_calc_space (di_data, infoidx, validxA, validxB, validxC, &val);

  /* if the comparison loop doesn't find it, it's a large number */
  scaleidx = DI_SCALE_MAX - 1;
  for (i = DI_SCALE_KILO; i < DI_SCALE_MAX; ++i) {
    if (dinum_cmp (&val, &di_data->scale_values [i]) < 0) {
      scaleidx = i - 1;
      break;
    }
  }

  dinum_clear (&val);
  return scaleidx;
}

double
di_get_scaled (void *tdi_data, int infoidx,
    int scaleidx, int validxA, int validxB, int validxC)
{
  di_data_t   *di_data = (di_data_t *) tdi_data;
  di_opt_t    *diopts;
  dinum_t     val;
  double      dval;

  if (di_data == NULL) {
    return 0.0;
  }

  if (infoidx < 0 || infoidx > di_data->dispcount) {
    return 0.0;
  }

  diopts = (di_opt_t *) di_data->options;
  init_scale_values (di_data, diopts);

  dinum_init (&val);
  di_calc_space (di_data, infoidx, validxA, validxB, validxC, &val);
  dval = dinum_scale (&val, &di_data->scale_values [scaleidx]);
  return dval;
}

void
di_disp_scaled (void *tdi_data, char *buff, long sz, int infoidx,
    int scaleidx, int validxA, int validxB, int validxC)
{
  di_data_t   *di_data = (di_data_t *) tdi_data;
  double      dval;

  *buff = '\0';
  if (di_data == NULL) {
    return;
  }

  if (infoidx < 0 || infoidx > di_data->dispcount) {
    return;
  }

  dval = di_get_scaled (di_data, infoidx, scaleidx, validxA, validxB, validxC);
  if (scaleidx == DI_SCALE_BYTE) {
    Snprintf1 (buff, (Size_t) sz, "%.0f", dval);
  } else {
    Snprintf1 (buff, (Size_t) sz, "%.1f", dval);
  }
}

double
di_get_perc (void *tdi_data, int infoidx,
    int validxA, int validxB, int validxC, int validxD, int validxE)
{
  di_data_t   *di_data = (di_data_t *) tdi_data;
  di_opt_t    *diopts;
  double      dval;

  if (di_data == NULL) {
    return 0.0;
  }

  if (infoidx < 0 || infoidx > di_data->dispcount) {
    return 0.0;
  }

  diopts = (di_opt_t *) di_data->options;
  init_scale_values (di_data, diopts);

  dval = di_calc_perc (di_data, infoidx, validxA, validxB, validxC, validxD, validxE);
  return dval;
}

void
di_disp_perc (void *tdi_data, char *buff, long sz, int infoidx,
    int validxA, int validxB, int validxC, int validxD, int validxE)
{
  di_data_t   *di_data = (di_data_t *) tdi_data;
  double      dval;

  *buff = '\0';
  if (di_data == NULL) {
    return;
  }

  if (infoidx < 0 || infoidx >= di_data->dispcount) {
    return;
  }

  dval = di_get_perc (di_data, infoidx, validxA, validxB, validxC, validxD, validxE);
  Snprintf1 (buff, (Size_t) sz, "%.0f", dval);
}

/* internal routines */

static int
checkFileInfo (di_data_t *di_data)
{
  int                 rc;
  int                 i;
  int                 j;
  struct stat         statBuf;
  di_opt_t            *diopts;
  di_disk_info_t      *diskInfo;


  rc = 0;
  diopts = (di_opt_t *) di_data->options;
  diskInfo = di_data->diskInfo;

  for (i = diopts->optidx; i < diopts->argc; ++i) {
    int fd;
    int src = -1;

    /* do this to automount devices.                    */
    /* stat () will not necessarily cause an automount.  */
    fd = open (diopts->argv [i], O_RDONLY | O_NOCTTY);
    if (fd < 0) {
      src = stat (diopts->argv [i], &statBuf);
    } else {
      src = fstat (fd, &statBuf);
    }

    if (src == 0) {
      int             saveIdx;
      int             found = false;
      int             inpool = false;
      Size_t          lastpoollen = 0;
      char            lastpool [DI_FILESYSTEM_LEN];

      saveIdx = 0;  /* should get overridden below */
      for (j = 0; j < di_data->fscount; ++j) {
        di_disk_info_t  *dinfo;
        int             startpool;
        int             poolmain;

        startpool = false;
        poolmain = false;

        dinfo = & (diskInfo [diskInfo [j].sortIndex [DI_SORT_TOTAL]]);

        /* is it a pooled filesystem type? */
        if (di_data->haspooledfs && di_isPooledFs (dinfo)) {
          if (lastpoollen == 0 ||
              strncmp (lastpool, dinfo->strdata [DI_DISP_FILESYSTEM], lastpoollen) != 0) {
            stpecpy (lastpool, lastpool + DI_FILESYSTEM_LEN, dinfo->strdata [DI_DISP_FILESYSTEM]);
            lastpoollen = di_mungePoolName (lastpool);
            inpool = false;
          }

          if (strncmp (lastpool, dinfo->strdata [DI_DISP_FILESYSTEM], lastpoollen) == 0) {
            startpool = true;
            if (inpool == false) {
              poolmain = true;
            }
          }
        } else {
          inpool = false;
        }

        if (poolmain) {
          saveIdx = j;
        }

        if (found && inpool) {
          dinfo = & (diskInfo [diskInfo [j].sortIndex [DI_SORT_TOTAL]]);
          dinfo->printFlag = DI_PRNT_SKIP;
          if (debug > 2) {
            printf ("  inpool B: also process %s %s\n",
                    dinfo->strdata [DI_DISP_FILESYSTEM], dinfo->strdata [DI_DISP_MOUNTPT]);
          }
        }

        /* when the filesystem name is specified on the command line */
        /* report for that filesystem, not devfs or / */
        if (strcmp (dinfo->strdata [DI_DISP_FILESYSTEM],
                diopts->argv [i]) == 0 ||
            (dinfo->st_dev != (unsigned long) DI_UNKNOWN_DEV &&
            (unsigned long) statBuf.st_dev == dinfo->st_dev &&
            ! dinfo->isLoopback)) {
          int foundnew = 0;

          ++foundnew;
          if (dinfo->printFlag == DI_PRNT_OK) {
            ++foundnew;
          }
          if (! isIgnoreFSType (dinfo->strdata [DI_DISP_FSTYPE])) {
            ++foundnew;
          }
          if (foundnew == 3) {
            dinfo->printFlag = DI_PRNT_FORCE;
            found = true;
          }

          if (debug > 2) {
            printf ("file %s: found device or fs-match %ld : %d (%s %s)\n",
                diopts->argv [i], dinfo->st_dev, foundnew,
                dinfo->strdata [DI_DISP_FILESYSTEM], dinfo->strdata [DI_DISP_MOUNTPT]);
          }

          if (inpool) {
            int   k;
            for (k = saveIdx; k < j; ++k) {
              dinfo = & (diskInfo [diskInfo [k].sortIndex [DI_SORT_TOTAL]]);
              if (dinfo->printFlag != DI_PRNT_FORCE) {
                dinfo->printFlag = DI_PRNT_SKIP;
              }
              if (debug > 2) {
                printf ("  inpool A: also process %s %s\n",
                        dinfo->strdata [DI_DISP_FILESYSTEM], dinfo->strdata [DI_DISP_MOUNTPT]);
              }
            }
          }
        }

        if (startpool) {
          inpool = true;
        }
      }
      /* if stat ok */
    } else {
      if (errno != EACCES && errno != EPERM) {
        fprintf (stderr, "stat: %s", diopts->argv [i]);
        perror ("");
      }
      rc = -1;
    }
    if (fd >= 0) {
      close (fd);
    }
  } /* for each file specified on command line */

  /* turn everything off */
  for (j = 0; j < di_data->fscount; ++j) {
    di_disk_info_t        *dinfo;

    dinfo = &di_data->diskInfo [j];
    if (dinfo->printFlag == DI_PRNT_OK) {
      dinfo->printFlag = DI_PRNT_IGNORE;
    }
  }

  /* also turn off the -I and -x lists */
  diopts->include_list.count = 0;
  diopts->ignore_list.count = 0;
  return rc;
}


/*
 * getDiskStatInfo
 *
 * gets the disk device number for each entry.
 *
 */

static void
getDiskStatInfo (di_data_t *di_data)
{
  int         i;
  struct stat statBuf;

  for (i = 0; i < di_data->fscount; ++i) {
    di_disk_info_t        *dinfo;

    dinfo = &di_data->diskInfo [i];

    /* don't try to stat devices that are not accessible */
    if (dinfo->printFlag == DI_PRNT_EXCLUDE ||
        dinfo->printFlag == DI_PRNT_BAD ||
        dinfo->printFlag == DI_PRNT_OUTOFZONE) {
      continue;
    }

    dinfo->st_dev = (unsigned long) DI_UNKNOWN_DEV;

    if (stat (dinfo->strdata [DI_DISP_MOUNTPT], &statBuf) == 0) {
      dinfo->st_dev = (unsigned long) statBuf.st_dev;
      if (debug > 2) {
        printf ("dev: %s: %ld\n", dinfo->strdata [DI_DISP_MOUNTPT], dinfo->st_dev);
      }
    } else {
      if (errno != EACCES && errno != EPERM) {
        fprintf (stderr, "stat: %s ", dinfo->strdata [DI_DISP_MOUNTPT]);
        perror ("");
      }
    }
  }
}

/*
 * getDiskSpecialInfo
 *
 * gets the disk device number for each entry.
 *
 */

static int
getDiskSpecialInfo (di_data_t *di_data, int dontResolveSymlink)
{
  int         i;
  struct stat statBuf;
  int         hasLoop;

  hasLoop = false;
  for (i = 0; i < di_data->fscount; ++i)
  {
    di_disk_info_t        *dinfo;

    dinfo = &di_data->diskInfo [i];

    /* check for initial slash; otherwise we can pick up normal files */
    if (* (dinfo->strdata [DI_DISP_FILESYSTEM]) == '/' &&
        stat (dinfo->strdata [DI_DISP_FILESYSTEM], &statBuf) == 0) {
      int                 rc;

      if (! dontResolveSymlink && checkForUUID (dinfo->strdata [DI_DISP_FILESYSTEM])) {
        struct stat tstatBuf;

        rc = lstat (dinfo->strdata [DI_DISP_FILESYSTEM], &tstatBuf);
        if (rc == 0 && S_ISLNK (tstatBuf.st_mode)) {
          char tspecial [DI_FILESYSTEM_LEN];

          if (realpath (dinfo->strdata [DI_DISP_FILESYSTEM], tspecial) != (char *) NULL) {
            stpecpy (dinfo->strdata [DI_DISP_FILESYSTEM],
                dinfo->strdata [DI_DISP_FILESYSTEM] + DI_FILESYSTEM_LEN,
                tspecial);
          }
        }
      }
      dinfo->sp_dev = (unsigned long) statBuf.st_dev;
      dinfo->sp_rdev = (unsigned long) statBuf.st_rdev;

      /* Solaris's loopback device is "lofs"            */
      /* linux loopback device is "none"                */
      /* linux has rdev = 0                             */
      /* DragonFlyBSD's loopback device is "null"       */
      /*   but not with special = /.../@@-               */
      /* DragonFlyBSD has rdev = -1                     */
      /* solaris is more consistent; rdev != 0 for lofs */
      /* solaris makes sense.                           */
      if (di_isLoopbackFs (dinfo)) {
        dinfo->isLoopback = true;
        hasLoop = true;
      }
      if (debug > 2) {
        printf ("special dev: %s %s: %ld rdev: %ld loopback: %d\n",
            dinfo->strdata [DI_DISP_FILESYSTEM], dinfo->strdata [DI_DISP_MOUNTPT], dinfo->sp_dev,
            dinfo->sp_rdev, dinfo->isLoopback);
      }
    } else {
      dinfo->sp_dev = 0;
      dinfo->sp_rdev = 0;
    }
  }

  return hasLoop;
}

/*
 * checkDiskInfo
 *
 * checks the disk information returned for various return values.
 *
 */

static void
checkDiskInfo (di_data_t *di_data, int hasLoop)
{
  int             i;
  int             j;
  di_opt_t     *diopts;

  diopts = (di_opt_t *) di_data->options;

  for (i = 0; i < di_data->fscount; ++i) {
    di_disk_info_t        *dinfo;

    dinfo = &di_data->diskInfo [i];
    dinfo->doPrint = 1;

    /* these are never printed... */
    if (dinfo->printFlag == DI_PRNT_EXCLUDE ||
        dinfo->printFlag == DI_PRNT_BAD ||
        dinfo->printFlag == DI_PRNT_OUTOFZONE) {
      if (debug > 2) {
        printf ("chk: skipping (%s):%s\n",
            getPrintFlagText ( (int) dinfo->printFlag), dinfo->strdata [DI_DISP_MOUNTPT]);
      }
      dinfo->doPrint = 0;
      continue;
    }

    /* need to check against include list */
    if (dinfo->printFlag == DI_PRNT_IGNORE ||
        dinfo->printFlag == DI_PRNT_SKIP) {
      if (debug > 2) {
        printf ("chk: skipping (%s):%s\n",
            getPrintFlagText ( (int) dinfo->printFlag), dinfo->strdata [DI_DISP_MOUNTPT]);
      }
      dinfo->doPrint = diopts->optval [DI_OPT_DISP_ALL];
    }

    /* Solaris reports a cdrom as having no free blocks,   */
    /* no available.  Their df doesn't always work right!  */
    /* -1 is returned.                                     */
    if (debug > 5) {
      char  tbuff [100];

      dinum_str (&dinfo->values [DI_SPACE_FREE], tbuff, sizeof (tbuff));
      printf ("chk: %s free: %s\n", dinfo->strdata [DI_DISP_MOUNTPT], tbuff);
    }
    if (dinum_cmp_s (&dinfo->values [DI_SPACE_FREE], -1) == 0 ||
        dinum_cmp_s (&dinfo->values [DI_SPACE_FREE], -2) == 0) {
      dinum_set_u (&dinfo->values [DI_SPACE_FREE], 0);
    }
    if (dinum_cmp_s (&dinfo->values [DI_SPACE_AVAIL], -1) == 0 ||
        dinum_cmp_s (&dinfo->values [DI_SPACE_AVAIL], -2) == 0) {
      dinum_set_u (&dinfo->values [DI_SPACE_AVAIL], 0);
    }

    {
      dinum_t     temp;

      dinum_init (&temp);

      dinum_set_u (&temp, (di_ui_t) ~0);
      if (dinum_cmp (&dinfo->values [DI_INODE_TOTAL], &temp) == 0) {
        dinum_set_u (&dinfo->values [DI_INODE_TOTAL], 0);
        dinum_set_u (&dinfo->values [DI_INODE_FREE], 0);
        dinum_set_u (&dinfo->values [DI_INODE_AVAIL], 0);
      }
      dinum_clear (&temp);
    }

    if (dinfo->printFlag == DI_PRNT_OK) {
      if (debug > 5) {
        char    tbuff [100];

        dinum_str (&dinfo->values [DI_SPACE_TOTAL], tbuff, sizeof (tbuff));
        printf ("chk: %s total: %s\n", dinfo->strdata [DI_DISP_MOUNTPT], tbuff);
      }

      if (isIgnoreFSType (dinfo->strdata [DI_DISP_FSTYPE])) {
        dinfo->printFlag = DI_PRNT_IGNORE;
        dinfo->doPrint = diopts->optval [DI_OPT_DISP_ALL];
        if (debug > 2) {
          printf ("chk: ignore-fstype: %s\n", dinfo->strdata [DI_DISP_FSTYPE]);
        }
      }
      if (isIgnoreSpecial (dinfo->strdata [DI_DISP_FILESYSTEM])) {
        dinfo->printFlag = DI_PRNT_IGNORE;
        dinfo->doPrint = diopts->optval [DI_OPT_DISP_ALL];
        if (debug > 2) {
          printf ("chk: ignore-special: %s\n", dinfo->strdata [DI_DISP_FILESYSTEM]);
        }
      }
      if (isIgnoreFS (dinfo->strdata [DI_DISP_FSTYPE], dinfo->strdata [DI_DISP_MOUNTPT])) {
        dinfo->printFlag = DI_PRNT_IGNORE;
        dinfo->doPrint = diopts->optval [DI_OPT_DISP_ALL];
        if (debug > 2) {
          printf ("chk: ignore-fs: %s\n", dinfo->strdata [DI_DISP_MOUNTPT]);
        }
      }

      /* Some systems return a -1 or -2 as an indicator.    */
      if (dinum_cmp_s (&dinfo->values [DI_SPACE_TOTAL], 0) == 0 ||
          dinum_cmp_s (&dinfo->values [DI_SPACE_TOTAL], -1) == 0 ||
          dinum_cmp_s (&dinfo->values [DI_SPACE_TOTAL], -2L) == 0) {
        dinfo->printFlag = DI_PRNT_IGNORE;
        dinfo->doPrint = diopts->optval [DI_OPT_DISP_ALL];
        if (debug > 2) {
          printf ("chk: ignore: values [DI_SPACE_TOTAL] <= 0: %s\n",
              dinfo->strdata [DI_DISP_MOUNTPT]);
        }
      }
    }

    /* make sure anything in the include list didn't get turned off */
    checkIncludeList (dinfo, &diopts->include_list);
  } /* for all disks */

  if (hasLoop && diopts->optval [DI_OPT_EXCL_LOOPBACK]) {
    /* this loop sets duplicate entries to be ignored. */
    for (i = 0; i < di_data->fscount; ++i) {
      di_disk_info_t        *dinfo;

      dinfo = &di_data->diskInfo [i];
      if (dinfo->printFlag != DI_PRNT_OK) {
        if (debug > 2) {
          printf ("dup: chk: skipping (%s):%s\n",
              getPrintFlagText ( (int) dinfo->printFlag), dinfo->strdata [DI_DISP_MOUNTPT]);
        }
        continue;
      }

      /* don't need to bother checking real partitions  */
      if (dinfo->sp_dev != 0 && dinfo->isLoopback) {
        unsigned long         sp_dev;
        unsigned long         sp_rdev;

        sp_dev = dinfo->sp_dev;
        sp_rdev = dinfo->sp_rdev;

        if (debug > 2) {
          printf ("dup: chk: i: %s dev: %ld rdev: %ld\n",
              dinfo->strdata [DI_DISP_MOUNTPT], sp_dev, sp_rdev);
        }

        for (j = 0; j < di_data->fscount; ++j) {
          di_disk_info_t        *dinfob;

          if (i == j) {
            continue;
          }

          dinfob = &di_data->diskInfo [j];
          if (dinfob->sp_dev != 0 &&
              dinfob->st_dev == sp_dev) {
            if (debug > 2)
            {
              printf ("dup: for %s %ld: found: %s %ld\n",
                  dinfo->strdata [DI_DISP_MOUNTPT], sp_dev, dinfob->strdata [DI_DISP_MOUNTPT], dinfob->st_dev);
            }

            dinfo->printFlag = DI_PRNT_IGNORE;
            dinfo->doPrint = diopts->optval [DI_OPT_DISP_ALL];
            if (debug > 2)
            {
              printf ("dup: chk: ignore: %s duplicate of %s\n",
                  dinfo->strdata [DI_DISP_MOUNTPT], dinfob->strdata [DI_DISP_MOUNTPT]);
              printf ("dup: j: dev: %ld rdev: %ld \n",
                  dinfob->sp_dev, dinfob->sp_rdev);
            }
          } /* if dup */
        }
      } /* if this is a loopback, non-real */
      else
      {
        if (debug > 2)
        {
          printf ("chk: dup: not checked: %s prnt: %d dev: %ld rdev: %ld %s\n",
              dinfo->strdata [DI_DISP_MOUNTPT], dinfo->printFlag,
              dinfo->sp_dev, dinfo->sp_rdev, dinfo->strdata [DI_DISP_FSTYPE]);
        }
      }
    } /* for each disk */
  } /* if the duplicate loopback mounts are to be excluded */
}

static void
checkDiskQuotas (di_data_t *di_data)
{
  int           i;
  int           j;
  Uid_t         uid;
  Gid_t         gid;
  di_quota_t    diqinfo;

  uid = 0;
  gid = 0;
#if _has_std_quotas
  uid = geteuid ();
  gid = getegid ();
#endif

  for (i = 0; i < di_data->fscount; ++i) {
    di_disk_info_t        *dinfo;

    dinfo = &di_data->diskInfo [i];
    if (! dinfo->doPrint) {
      continue;
    }

    diqinfo.filesystem = dinfo->strdata [DI_DISP_FILESYSTEM];
    diqinfo.mountpt = dinfo->strdata [DI_DISP_MOUNTPT];
    diqinfo.fstype = dinfo->strdata [DI_DISP_FSTYPE];
    diqinfo.uid = uid;
    diqinfo.gid = gid;
    for (j = 0; j < DI_QVAL_MAX; ++j) {
      dinum_init (&diqinfo.values [j]);
    }
    diquota (&diqinfo);

    if (debug > 2) {
      char    tbuff [100];
      dinum_str (&diqinfo.values [DI_QUOTA_LIMIT], tbuff, sizeof (tbuff));
      printf ("quota: %s limit: %s\n", dinfo->strdata [DI_DISP_MOUNTPT], tbuff);
      dinum_str (&dinfo->values [DI_SPACE_TOTAL], tbuff, sizeof (tbuff));
      printf ("quota:   tot: %s\n", tbuff);
      dinum_str (&diqinfo.values [DI_QUOTA_USED], tbuff, sizeof (tbuff));
      printf ("quota: %s used: %s\n", dinfo->strdata [DI_DISP_MOUNTPT], tbuff);
      dinum_str (&dinfo->values [DI_SPACE_AVAIL], tbuff, sizeof (tbuff));
      printf ("quota:   avail: %s\n", tbuff);
    }

    if (dinum_cmp_s (&diqinfo.values [DI_QUOTA_LIMIT], 0) != 0 &&
        dinum_cmp (&diqinfo.values [DI_QUOTA_LIMIT], &dinfo->values [DI_SPACE_TOTAL]) < 0) {
      dinum_t     tsize;

      dinum_init (&tsize);
      dinum_set (&dinfo->values [DI_SPACE_TOTAL],
          &diqinfo.values [DI_QUOTA_LIMIT]);
      dinum_set (&tsize, &diqinfo.values [DI_QUOTA_LIMIT]);
      dinum_sub (&tsize, &diqinfo.values [DI_QUOTA_USED]);
      if (dinum_cmp_s (&tsize, 0) < 0) {
        dinum_set_s (&tsize, 0);
      }
      if (dinum_cmp (&tsize, &dinfo->values [DI_SPACE_AVAIL]) < 0) {
        dinum_set (&dinfo->values [DI_SPACE_AVAIL], &tsize);
        dinum_set (&dinfo->values [DI_SPACE_FREE], &tsize);
        if (debug > 2) {
          printf ("quota: using quota for: total free avail\n");
        }
      } else if (dinum_cmp (&tsize, &dinfo->values [DI_SPACE_AVAIL]) > 0 &&
          dinum_cmp (&tsize, &dinfo->values [DI_SPACE_FREE]) < 0) {
        dinum_set (&dinfo->values [DI_SPACE_FREE], &tsize);
        if (debug > 2) {
          printf ("quota: using quota for: total free\n");
        }
      } else {
        if (debug > 2) {
          printf ("quota: using quota for: total\n");
        }
      }
      dinum_clear (&tsize);
    }

    if (dinum_cmp_s (&diqinfo.values [DI_QUOTA_ILIMIT], 0) != 0 &&
          dinum_cmp (&diqinfo.values [DI_QUOTA_ILIMIT], &dinfo->values [DI_INODE_TOTAL]) < 0) {
      dinum_t   tsize;

      dinum_init (&tsize);
      dinum_set (&dinfo->values [DI_INODE_TOTAL], &diqinfo.values [DI_QUOTA_ILIMIT]);
      dinum_set (&tsize, &diqinfo.values [DI_QUOTA_ILIMIT]);
      dinum_sub (&tsize, &diqinfo.values [DI_QUOTA_IUSED]);
      if (dinum_cmp_s (&tsize, 0) < 0) {
        dinum_set_u (&tsize, 0);
      }
      if (dinum_cmp (&tsize, &dinfo->values [DI_INODE_AVAIL]) < 0) {
        dinum_set (&dinfo->values [DI_INODE_AVAIL], &tsize);
        dinum_set (&dinfo->values [DI_INODE_FREE], &tsize);
        if (debug > 2) {
          printf ("quota: using quota for inodes: total free avail\n");
        }
      } else if (dinum_cmp (&tsize, &dinfo->values [DI_INODE_AVAIL]) > 0 &&
          dinum_cmp (&tsize, &dinfo->values [DI_INODE_FREE]) < 0) {
        dinum_set (&dinfo->values [DI_INODE_FREE], &tsize);
        if (debug > 2) {
          printf ("quota: using quota for inodes: total free\n");
        }
      } else {
        if (debug > 2) {
          printf ("quota: using quota for inodes: total\n");
        }
      }
    }
  }

  for (j = 0; j < DI_QVAL_MAX; ++j) {
    dinum_clear (&diqinfo.values [j]);
  }

  return;
}

/*
 * preCheckDiskInfo
 *
 * checks for ignore/include list; check for remote filesystems
 * and local only flag set.
 * checks for zones (Solaris)
 *
 */

static void
preCheckDiskInfo (di_data_t *di_data)
{
  int           i;
  di_opt_t      *diopts;

  diopts = (di_opt_t *) di_data->options;
  for (i = 0; i < di_data->fscount; ++i) {
    di_disk_info_t        *dinfo;

    dinfo = &di_data->diskInfo [i];

    if (debug > 4) {
      printf ("## prechk:%s:\n", dinfo->strdata [DI_DISP_MOUNTPT]);
    }
    checkZone (dinfo, (di_zone_info_t *) &di_data->zoneInfo, diopts);

    if (di_isPooledFs (dinfo)) {
      di_data->haspooledfs = true;
    }

    if (dinfo->printFlag == DI_PRNT_OK) {
      /* don't bother w/this check is all flag is set. */
      if (! diopts->optval [DI_OPT_DISP_ALL]) {
        di_is_remote_disk (dinfo);

        if (dinfo->isLocal == false && diopts->optval [DI_OPT_LOCAL_ONLY]) {
          dinfo->printFlag = DI_PRNT_IGNORE;
          if (debug > 2) {
            printf ("prechk: ignore: remote disk; local flag set: %s\n",
                dinfo->strdata [DI_DISP_MOUNTPT]);
          }
        }
      }
    }

    if (dinfo->printFlag == DI_PRNT_OK ||
        dinfo->printFlag == DI_PRNT_IGNORE) {
      /* do these checks to override the all flag */
      checkIgnoreList (dinfo, &diopts->ignore_list);
      checkIncludeList (dinfo, &diopts->include_list);
    }
  } /* for all disks */
}

static void
checkIgnoreList (di_disk_info_t *dinfo, di_strarr_t *ignore_list)
{
  char            *ptr;
  int             i;

  /* if the file system type is in the ignore list, skip it */
  if (ignore_list->count > 0) {
    for (i = 0; i < ignore_list->count; ++i) {
      ptr = ignore_list->list [i];
      if (debug > 2) {
        printf ("chkign: test: fstype %s/%s : %s\n", ptr,
            dinfo->strdata [DI_DISP_FSTYPE], dinfo->strdata [DI_DISP_MOUNTPT]);
      }
      if (strcmp (ptr, dinfo->strdata [DI_DISP_FSTYPE]) == 0 ||
          (strcmp (ptr, "fuse") == 0 &&
          strncmp ("fuse", dinfo->strdata [DI_DISP_FSTYPE], (Size_t) 4) == 0)) {
        dinfo->printFlag = DI_PRNT_EXCLUDE;
        dinfo->doPrint = false;
        if (debug > 2) {
          printf ("chkign: ignore: fstype %s match: %s\n", ptr,
              dinfo->strdata [DI_DISP_MOUNTPT]);
        }
        break;
      }
    }
  } /* if an ignore list was specified */
}

static void
checkIncludeList (di_disk_info_t *dinfo, di_strarr_t *include_list)
{
  char            *ptr;
  int             i;

  /* if the file system type is not in the include list, skip it */
  if (include_list->count > 0) {
    for (i = 0; i < include_list->count; ++i) {
      ptr = include_list->list [i];
      if (debug > 2) {
        printf ("chkinc: test: fstype %s/%s : %s\n", ptr,
            dinfo->strdata [DI_DISP_FSTYPE], dinfo->strdata [DI_DISP_MOUNTPT]);
      }

      if (strcmp (ptr, dinfo->strdata [DI_DISP_FSTYPE]) == 0 ||
          (strcmp (ptr, "fuse") == 0 &&
          strncmp ("fuse", dinfo->strdata [DI_DISP_FSTYPE], (Size_t) 4) == 0)) {
        dinfo->printFlag = DI_PRNT_OK;
        dinfo->doPrint = true;
        if (debug > 2) {
          printf ("chkinc:include:fstype %s match: %s\n", ptr,
              dinfo->strdata [DI_DISP_MOUNTPT]);
        }
        break;
      } else {
        dinfo->printFlag = DI_PRNT_EXCLUDE;
        dinfo->doPrint = false;
        if (debug > 2) {
          printf ("chkinc:!include:fstype %s no match: %s\n", ptr,
              dinfo->strdata [DI_DISP_MOUNTPT]);
        }
      }
    }
  } /* if an include list was specified */
}

static void
checkZone (di_disk_info_t *dinfo, di_zone_info_t *zoneInfo, di_opt_t *diopts)
{
#if _lib_zone_list && _lib_getzoneid && _lib_zone_getattr
  int         i;
  int         idx = -1;

  if (strcmp (diopts->zoneDisplay, "all") == 0 &&
      zoneInfo->uid == 0) {
    return;
  }

  for (i = 0; i < (int) zoneInfo->zoneCount; ++i) {
    /* find the zone the filesystem is in, if non-global */
    if (debug > 5) {
      printf (" checkZone:%s:compare:%d:%s:\n",
          dinfo->strdata [DI_DISP_MOUNTPT],
          zoneInfo->zones [i].rootpathlen,
          zoneInfo->zones [i].rootpath);
    }
    if (strncmp (zoneInfo->zones [i].rootpath,
        dinfo->strdata [DI_DISP_MOUNTPT], zoneInfo->zones [i].rootpathlen) == 0) {
      if (debug > 4) {
          printf (" checkZone:%s:found zone:%s:\n",
                  dinfo->strdata [DI_DISP_MOUNTPT], zoneInfo->zones [i].name);
      }
      idx = i;
      break;
    }
    if (idx == -1) {
      idx = zoneInfo->globalIdx;
    }
  }

  /* no root access, ignore any zones     */
  /* that don't match our zone id         */
  /* this will override any ignore flags  */
  /* already set                          */
  if (zoneInfo->uid != 0) {
    if (debug > 5) {
      printf (" checkZone:uid non-zero:chk zone:%d:%d:\n",
          (int) zoneInfo->myzoneid,
          (int) zoneInfo->zones [idx].zoneid);
    }
    if (zoneInfo->myzoneid != zoneInfo->zones [idx].zoneid) {
      if (debug > 4) {
        printf (" checkZone:not root, not zone:%d:%d:outofzone:\n",
            (int) zoneInfo->myzoneid,
            (int) zoneInfo->zones [idx].zoneid);
      }
      dinfo->printFlag = DI_PRNT_OUTOFZONE;
    }
  }

  if (debug > 5) {
    printf (" checkZone:chk name:%s:%s:\n",
        diopts->zoneDisplay, zoneInfo->zones [idx].name);
  }

  /* not the zone we want. ignore */
  if (! diopts->optval [DI_OPT_DISP_ALL] &&
      dinfo->printFlag == DI_PRNT_OK &&
      strcmp (diopts->zoneDisplay, "all") != 0 &&
      strcmp (diopts->zoneDisplay, zoneInfo->zones [idx].name) != 0) {
    if (debug > 4) {
      printf (" checkZone:wrong zone:ignore:\n");
    }

    dinfo->printFlag = DI_PRNT_IGNORE;
  }

  /* if displaying a non-global zone,   */
  /* don't display loopback filesystems */
  if (! diopts->optval [DI_OPT_DISP_ALL] &&
      dinfo->printFlag == DI_PRNT_OK &&
      strcmp (diopts->zoneDisplay, "global") != 0 &&
      dinfo->isLoopback) {
    if (debug > 4) {
      printf (" checkZone:non-global/lofs:ignore:\n");
    }

    dinfo->printFlag = DI_PRNT_IGNORE;
  }

#endif
  return;
}

static int
isIgnoreSpecial (const char *special)
{
  static const char   *appletimemachine = "com.apple.TimeMachine.";

  /* solaris: swap */
  /* linux: cgroup, tmpfs */
  if (strncmp (special, appletimemachine, strlen (appletimemachine)) == 0 ||
      strcmp (special, "tmpfs") == 0 ||
      strcmp (special, "cgroup") == 0 ||
      strcmp (special, "swap") == 0) {
    return true;
  }
  return false;
}

static int
isIgnoreFS (const char *fstype, const char *name)
{
  static const char   *applesystem = "/System/";

  if (strcmp (fstype, "apfs") == 0 &&
       strncmp (name, applesystem, strlen (applesystem)) == 0) {
    return true;
  }
  return false;
}

static int
isIgnoreFSType (const char *fstype)
{
  if (strcmp (fstype, "rootfs") == 0 ||
      strcmp (fstype, "procfs") == 0 ||
      strcmp (fstype, "ptyfs") == 0 ||
      strcmp (fstype, "kernfs") == 0 ||
      strcmp (fstype, "devfs") == 0 ||
      strcmp (fstype, "devtmpfs") == 0) {
    return true;
  }
  return false;
}

static int
checkForUUID (const char *spec)
{
#if _lib_realpath && _define_S_ISLNK && _lib_lstat

/*
 *  /dev/mapper/luks-828fc648-9f30-43d8-a0b1-f7196a2edb66
 * /dev/disk/by-uuid/cfbbd7b3-b37a-4587-a711-58fd36b2cac6
 *             1         2         3         4
 *   01234567890123456789012345678901234567890
 *   cfbbd7b3-b37a-4587-a711-58fd36b2cac6
 */
  Size_t        len;

  len = strlen (spec);
  if (len > 36) {
    Size_t      offset;

    offset = len - 36;
    if (* (spec + offset + 8) == '-' &&
        * (spec + offset + 13) == '-' &&
        * (spec + offset + 18) == '-' &&
        * (spec + offset + 23) == '-' &&
        strspn (spec + offset, "-1234567890abcdef") == 36) {
      return true;
    }
  }

#endif /* have _lib_realpath, etc. */

  return false;
}

/* for debugging */
const char *
getPrintFlagText (int pf)
{
  return pf == DI_PRNT_OK ? "ok" :
      pf == DI_PRNT_BAD ? "bad" :
      pf == DI_PRNT_IGNORE ? "ignore" :
      pf == DI_PRNT_EXCLUDE ? "exclude" :
      pf == DI_PRNT_OUTOFZONE ? "outofzone" :
      pf == DI_PRNT_FORCE ? "force" :
      pf == DI_PRNT_SKIP ? "skip" : "unknown";
}


void
di_sort_disk_info (di_opt_t *diopts, di_disk_info_t *data, int count,
    const char *sortType, int sidx)
{
  int     tempIndex;
  int     gap;
  int     j;
  int     i;

  if (count <= 1) {
    return;
  }

  gap = 1;
  while (gap < count) {
    gap = 3 * gap + 1;
  }

  for (gap /= 3; gap > 0; gap /= 3) {
    for (i = gap; i < count; ++i) {
      tempIndex = data [i].sortIndex [sidx];
      j = i - gap;

      while (j >= 0 && di_sort_compare (diopts, sortType, data, data [j].sortIndex [sidx], tempIndex) > 0) {
        data [j + gap].sortIndex [sidx] = data [j].sortIndex [sidx];
        j -= gap;
      }

      j += gap;
      if (j != i) {
        data [j].sortIndex [sidx] = tempIndex;
      }
    }
  }
}

static int
di_sort_compare (const di_opt_t *diopts, const char *sortType,
    const di_disk_info_t *data, int idx1, int idx2)
{
  int                   rc;
  int                   sortOrder;
  const char            *ptr;
  const di_disk_info_t  *d1;
  const di_disk_info_t  *d2;

  /* reset sort order to the default start value */
  sortOrder = DI_SORT_OPT_ASCENDING;
  rc = 0;

  d1 = & (data [idx1]);
  d2 = & (data [idx2]);

  ptr = sortType;
  while (*ptr) {
    switch (*ptr) {
      case DI_SORT_OPT_NONE: {
          break;
      }

      case DI_SORT_OPT_MOUNT: {
          rc = strcoll (d1->strdata [DI_DISP_MOUNTPT], d2->strdata [DI_DISP_MOUNTPT]);
          rc *= sortOrder;
          break;
      }

      case DI_SORT_OPT_REVERSE: {
          sortOrder *= -1;
          break;
      }

      case DI_SORT_OPT_FILESYSTEM: {
          rc = strcoll (d1->strdata [DI_DISP_FILESYSTEM], d2->strdata [DI_DISP_FILESYSTEM]);
          rc *= sortOrder;
          break;
      }

      case DI_SORT_OPT_TYPE: {
          rc = strcoll (d1->strdata [DI_DISP_FSTYPE], d2->strdata [DI_DISP_FSTYPE]);
          rc *= sortOrder;
          break;
      }

      case DI_SORT_OPT_AVAIL:
      case DI_SORT_OPT_FREE:
      case DI_SORT_OPT_TOTAL: {
        switch (*ptr) {
          case DI_SORT_OPT_AVAIL: {
            rc = dinum_cmp (&d1->values [DI_SPACE_AVAIL], &d2->values [DI_SPACE_AVAIL]);
            break;
          }
          case DI_SORT_OPT_FREE: {
            rc = dinum_cmp (&d1->values [DI_SPACE_FREE], &d2->values [DI_SPACE_FREE]);
            break;
          }
          case DI_SORT_OPT_TOTAL: {
            rc = dinum_cmp (&d1->values [DI_SPACE_TOTAL], &d2->values [DI_SPACE_TOTAL]);
            break;
          }
          default: {
            break;
          }
        }

        rc *= sortOrder;
        break;
      }

      default: {
        break;
      }
    } /* switch on sort type */

    if (rc != 0)
    {
      return rc;
    }

    ++ptr;
  }

  return rc;
}

static void
init_scale_values (di_data_t *di_data, di_opt_t *diopts)
{
  int       i;
  dinum_t   base;

  if (di_data->scale_values_init) {
    return;
  }

  for (i = 0; i < DI_SCALE_MAX; ++i) {
    dinum_init (&di_data->scale_values [i]);
  }
  dinum_init (&base);
  dinum_set_u (&base, (di_ui_t) diopts->blockSize);
  dinum_set_u (&di_data->scale_values [DI_SCALE_BYTE], 1);
  for (i = DI_SCALE_KILO; i < DI_SCALE_MAX; ++i) {
    dinum_set (&di_data->scale_values [i], &base);
    dinum_mul (&di_data->scale_values [i], &di_data->scale_values [i - 1]);
  }

  di_data->scale_values_init = 1;
  dinum_clear (&base);
}

static void
di_calc_space (di_data_t *di_data, int infoidx,
    int validxA, int validxB, int validxC, dinum_t *val)
{
  dinum_t         sub;
  di_disk_info_t  *dinfo;

  dinfo = &di_data->diskInfo [infoidx];

  dinum_init (&sub);
  dinum_set_u (&sub, 0);
  if (validxB != DI_VALUE_NONE) {
    dinum_set (&sub, &dinfo->values [validxB]);
  }
  if (validxB != DI_VALUE_NONE && validxC != DI_VALUE_NONE) {
    dinum_sub (&sub, &dinfo->values [validxC]);
  }
  dinum_set (val, &dinfo->values [validxA]);
  dinum_sub (val, &sub);

  dinum_clear (&sub);
}

static double
di_calc_perc (di_data_t *di_data, int infoidx,
    int validxA, int validxB, int validxC, int validxD, int validxE)
{
  dinum_t         subd;
  dinum_t         subr;
  dinum_t         dividend;
  dinum_t         divisor;
  di_disk_info_t  *dinfo;
  double          dval;

  dinfo = &di_data->diskInfo [infoidx];

  dinum_init (&subd);
  dinum_init (&subr);
  dinum_init (&dividend);
  dinum_init (&divisor);
  dinum_set_u (&subd, 0);
  dinum_set_u (&subr, 0);

  if (validxB != DI_VALUE_NONE) {
    dinum_set (&subd, &dinfo->values [validxB]);
  }
  dinum_set (&dividend, &dinfo->values [validxA]);
  dinum_sub (&dividend, &subd);

  if (validxD != DI_VALUE_NONE) {
    dinum_set (&subr, &dinfo->values [validxD]);
  }
  if (validxD != DI_VALUE_NONE && validxE != DI_VALUE_NONE) {
    dinum_sub (&subr, &dinfo->values [validxE]);
  }
  dinum_set (&divisor, &dinfo->values [validxC]);
  dinum_sub (&divisor, &subr);
  if (dinum_cmp_s (&divisor, 0) == 0) {
    dval = 0.0;
  } else {
    dval = dinum_perc (&dividend, &divisor);
  }

  dinum_clear (&subd);
  dinum_clear (&subr);
  dinum_clear (&dividend);
  dinum_clear (&divisor);
  return dval;
}

static void
processTotals (di_data_t *di_data)
{
  int               i;
  Size_t            lastpoollen = 0;
  char              lastpool [DI_FILESYSTEM_LEN];
  int               inpool = 0;
  di_disk_info_t    *totals;

  /* all of the space allocations for diskinfo have + 1 */
  /* count is the number of filesystems */
  totals = &di_data->diskInfo [di_data->fscount];
  totals->doPrint = 1;
  totals->printFlag = DI_PRNT_OK;

  for (i = 0; i < di_data->fscount; ++i) {
    di_disk_info_t  *dinfo;
    int             sortidx;
    int             startpool;

    startpool = false;
    sortidx = di_data->diskInfo [i].sortIndex [DI_SORT_TOTAL];
    dinfo = & (di_data->diskInfo [sortidx]);

    /* is it a pooled filesystem type? */
    if (di_data->haspooledfs && di_isPooledFs (dinfo)) {
      if (lastpoollen == 0 ||
          strncmp (lastpool, dinfo->strdata [DI_DISP_FILESYSTEM], lastpoollen) != 0) {
        stpecpy (lastpool, lastpool + DI_FILESYSTEM_LEN,
            dinfo->strdata [DI_DISP_FILESYSTEM]);
        lastpoollen = di_mungePoolName (lastpool);
        inpool = false;
        startpool = true;
        if (strcmp (dinfo->strdata [DI_DISP_FSTYPE], "null") == 0 &&
            strcmp (dinfo->strdata [DI_DISP_FILESYSTEM] + strlen (dinfo->strdata [DI_DISP_FILESYSTEM]) - 5, "00000") != 0) {
          /* dragonflybsd doesn't have the main pool mounted */
          inpool = true;
        }
      }
    } else {
      inpool = false;
    }

    if (dinfo->doPrint) {
      addTotals (dinfo, totals, inpool);
    } else {
      if (debug > 2) {
        printf ("tot:%s:%s:skip\n", dinfo->strdata [DI_DISP_FILESYSTEM], dinfo->strdata [DI_DISP_MOUNTPT]);
      }
    }

    if (startpool) {
      inpool = true;
    }
  } /* for each entry */
}

static void
addTotals (const di_disk_info_t *dinfo, di_disk_info_t *totals, int inpool)
{
  if (debug > 2) {
    printf ("tot:%s:%s:inp:%d\n",
        dinfo->strdata [DI_DISP_FILESYSTEM], dinfo->strdata [DI_DISP_MOUNTPT], inpool);
  }

  /*
   * zfs:
   *   The total is the space used + the space free.
   *   Free space is the space available in the pool.
   *   Available space is the space available in the pool.
   *   Thus: total - free = used.
   *   -- To get the real total, add the total for the pool
   *      and all used space for all inpool filesystems.
   *
   * apfs:
   *   An easy hierarchy:
   *      /dev/disk1        the container, is not mounted.
   *      /dev/disk1s1      partition 1
   *      /dev/disk1s2      partition 2
   *      /dev/disk1s3      partition 3
   *   The first disk is used as the root of the pool, even though it
   *     is not the container.
   *   The total is the total space.
   *   Free space is the space available + space used.
   *      (or total space - space used).
   *   Available space is the space available in the pool.
   *   Thus: total - free = used.
   *   -- To get (totals - free) to work for the totals, subtract
   *      all free space returned by in-pool filesystems.
   *
   * hammer, hammer2:
   *    Typically, a null mount is used such as:
   *      /dev/serno/SERIAL.s1a   /build              hammer
   *      /build/usr              /usr                null
   *      /build/usr.local        /usr/local          null
   *    There are also pfs mounts:
   *      /dev/ad1s1a@LOCAL       /d1     hammer2
   *      /dev/ad1s1a@d1.a        /d1/a   hammer2
   *    Or
   *      /dev/serno/SERIAL.s1a         /mnt        hammer2
   *      /dev/serno/SERIAL.s1a@mnt.usr /mnt/usr    hammer2
   *    Or
   *      @build                  /build            hammer2
   *      @build.var              /build/var        null
   *  Difficult to process, as the naming is not consistent.
   *  Not implemented.
   *
   * advfs:
   *   Unknown.  Assume the same as zfs.
   */

  if (inpool) {
    if (debug > 2) {
      printf ("  tot:inpool:\n");
    }
    if (strcmp (dinfo->strdata [DI_DISP_FSTYPE], "apfs") == 0) {
      dinum_t   tval;

      dinum_init (&tval);
      dinum_set (&tval, &dinfo->values [DI_SPACE_TOTAL]);
      dinum_sub (&tval, &dinfo->values [DI_SPACE_FREE]);
      dinum_sub (&totals->values [DI_SPACE_FREE], &tval);
      dinum_clear (&tval);
    } else {
      /* zfs, old hammer, advfs */
      dinum_t   tval;

      dinum_init (&tval);
      dinum_set (&tval, &dinfo->values [DI_SPACE_TOTAL]);
      dinum_sub (&tval, &dinfo->values [DI_SPACE_FREE]);
      dinum_add (&totals->values [DI_SPACE_TOTAL], &tval);
      dinum_set (&tval, &dinfo->values [DI_INODE_TOTAL]);
      dinum_sub (&tval, &dinfo->values [DI_INODE_FREE]);
      dinum_add (&totals->values [DI_INODE_TOTAL], &tval);
      dinum_clear (&tval);
    }
  } else {
    if (debug > 2) {printf ("  tot:not inpool:add all totals\n"); }
    dinum_add (&totals->values [DI_SPACE_TOTAL], &dinfo->values [DI_SPACE_TOTAL]);
    dinum_add (&totals->values [DI_SPACE_FREE], &dinfo->values [DI_SPACE_FREE]);
    dinum_add (&totals->values [DI_SPACE_AVAIL], &dinfo->values [DI_SPACE_AVAIL]);
    dinum_add (&totals->values [DI_INODE_TOTAL], &dinfo->values [DI_INODE_TOTAL]);
    dinum_add (&totals->values [DI_INODE_FREE], &dinfo->values [DI_INODE_FREE]);
    dinum_add (&totals->values [DI_INODE_AVAIL], &dinfo->values [DI_INODE_AVAIL]);
  }
}

