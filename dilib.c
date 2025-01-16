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
#if _hdr_libintl
# include <libintl.h>
#endif
#if _hdr_locale
# include <locale.h>
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
#if _hdr_fcntl \
    && ! defined (DI_INC_FCNTL_H)    /* xenix */
# define DI_INC_FCNTL_H
# include <fcntl.h>     /* O_RDONLY, O_NOCTTY */
#endif

#include "di.h"
#include "dilib.h"
#include "options.h"
#include "version.h"

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

#define DI_SORT_NONE            'n'
#define DI_SORT_MOUNT           'm'
#define DI_SORT_SPECIAL         's'
#define DI_SORT_TOTAL           'T'
#define DI_SORT_FREE            'f'
#define DI_SORT_AVAIL           'a'
#define DI_SORT_REVERSE         'r'
#define DI_SORT_TYPE            't'
#define DI_SORT_ASCENDING       1

static void checkDiskInfo       (di_data_t *, int);
static void checkDiskQuotas     (di_data_t *);
static int  checkFileInfo       (di_data_t *);
static int  getDiskSpecialInfo  (di_data_t *, unsigned int);
static void getDiskStatInfo     (di_data_t *);
static void preCheckDiskInfo    (di_data_t *);
static void initZones           (di_data_t *);

static void checkZone           (di_disk_info_t *, di_zone_info_t *, di_opt_t *);
static void checkIgnoreList     (di_disk_info_t *, di_strarr_t *);
static void checkIncludeList    (di_disk_info_t *, di_strarr_t *);
static int  isIgnoreFSType      (const char *);
static int  isIgnoreSpecial     (const char *);
static int  isIgnoreFS          (const char *, const char *);
static int  checkForUUID        (const char *);
static int  diCompare           (const di_opt_t *, const di_disk_info_t *, unsigned int, unsigned int);

void
di_initialize (di_data_t *di_data)
{
  di_opt_t    *diopts;

  di_data->count = 0;
  di_data->haspooledfs = false;
  di_data->disppooledfs = false;
  di_data->totsorted = false;

  di_data->diskInfo = (di_disk_info_t *) NULL;

  /* options defaults */
  di_data->options = di_init_options ();
  diopts = (di_opt_t *) di_data->options;
}

int
di_process_options (di_data_t *di_data, int argc, char * argv [])
{
  di_opt_t    *diopts;

  diopts = (di_opt_t *) di_data->options;
  diopts->optidx = di_get_options (argc, argv, (di_opt_t *) di_data->options);

  if (debug > 0) {
#if _use_math == DI_GMP
    fprintf (stdout, "# GMP:\n");
#elif _use_math == DI_TOMMATH
    fprintf (stdout, "# TOMMATH:\n");
#else
    fprintf (stdout, "# INTERNAL: ld:%d d:%d u64:%d ll:%d l:%d\n", _siz_long_double, _siz_double, _siz_uint64_t, _siz_long, _siz_long_long);
#endif
  }

  return diopts->exitFlag;
}

void
di_get_data (di_data_t *di_data)
{
  di_opt_t    *diopts;
  int         hasLoop;
  char        *disp;

  /* initialization */
  disp = (char *) NULL;
  diopts = (di_opt_t *) di_data->options;

  initZones (di_data);
  if (diopts->exitFlag != DI_EXIT_NORM) {
    return;
  }

  if (debug > 0) {
    printf ("di version %s\n", DI_VERSION);
  }

  /* main processing */

  if (di_get_disk_entries (&di_data->diskInfo, &di_data->count) < 0) {
    di_cleanup (di_data);
    return;
  }

  hasLoop = false;
  preCheckDiskInfo (di_data);
  if (diopts->optidx < diopts->argc ||
      diopts->excludeLoopback) {
    getDiskStatInfo (di_data);
    hasLoop = getDiskSpecialInfo (di_data, diopts->dontResolveSymlink);
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
  di_get_disk_info (&di_data->diskInfo, &di_data->count);
  checkDiskInfo (di_data, hasLoop);
  if (diopts->quota_check == true) {
    checkDiskQuotas (di_data);
  }
}

/*
 * di_cleanup
 *
 * free up allocated memory
 *
 */

void
di_cleanup (di_data_t *di_data)
{
  int         i;
  di_opt_t    *diopts;

  if (di_data->diskInfo != (di_disk_info_t *) NULL) {
    for (i = 0; i < DI_VALUE_MAX; ++i) {
      dinum_clear (&di_data->diskInfo->values [i]);
    }
    free ((char *) di_data->diskInfo);
  }

  diopts = (di_opt_t *) di_data->options;
  if (diopts != NULL) {
    if (diopts->ignore_list.count > 0 &&
        diopts->ignore_list.list != (char **) NULL) {
      free ((char *) diopts->ignore_list.list);
      diopts->ignore_list.count = 0;
    }

    if (diopts->include_list.count > 0 &&
        diopts->include_list.list != (char **) NULL) {
      free ((char *) diopts->include_list.list);
      diopts->include_list.count = 0;
    }
    free (diopts);
  }

  if (di_data->zoneInfo.zones != (di_zone_summ_t *) NULL) {
    free ((void *) di_data->zoneInfo.zones);
  }
}

extern int
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
  if (di_data->haspooledfs && ! di_data->totsorted)
  {
    char tempSortType [DI_SORT_MAX + 2];

    strncpy (tempSortType, diopts->sortType, DI_SORT_MAX);
    strncpy (diopts->sortType, "s", DI_SORT_MAX);
    sortArray (diopts, diskInfo, di_data->count, DI_TOT_SORT_IDX);
    strncpy (diopts->sortType, tempSortType, DI_SORT_MAX);
    di_data->totsorted = true;
  }

  for (i = diopts->optidx; i < diopts->argc; ++i) {
    int fd;
    int src;

    /* do this to automount devices.                    */
    /* stat() will not necessarily cause an automount.  */
    fd = open (diopts->argv [i], O_RDONLY | O_NOCTTY);
    if (fd < 0) {
      src = stat (diopts->argv [i], &statBuf);
    } else {
      src = fstat (fd, &statBuf);
    }

    if (src == 0) {
      int             saveIdx;
      int             found = { false };
      int             inpool = { false };
      Size_t          lastpoollen = { 0 };
      char            lastpool [DI_DEVNAME_LEN + 1];

      saveIdx = 0;  /* should get overridden below */
      for (j = 0; j < di_data->count; ++j) {
        di_disk_info_t  *dinfo;
        int             startpool;
        int             poolmain;

        startpool = false;
        poolmain = false;

        dinfo = &(diskInfo [diskInfo [j].sortIndex [DI_TOT_SORT_IDX]]);

        /* is it a pooled filesystem type? */
        if (di_data->haspooledfs && di_isPooledFs (dinfo)) {
          if (lastpoollen == 0 ||
              strncmp (lastpool, dinfo->devname, lastpoollen) != 0) {
            strncpy (lastpool, dinfo->devname, DI_DEVNAME_LEN);
            lastpoollen = di_mungePoolName (lastpool);
            inpool = false;
          }

          if (strncmp (lastpool, dinfo->devname, lastpoollen) == 0) {
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
          dinfo = &(diskInfo [diskInfo [j].sortIndex[DI_TOT_SORT_IDX]]);
          dinfo->printFlag = DI_PRNT_SKIP;
          if (debug > 2) {
            printf ("  inpool B: also process %s %s\n",
                    dinfo->devname, dinfo->mountpt);
          }
        }

        if (dinfo->st_dev != (__ulong) DI_UNKNOWN_DEV &&
            (__ulong) statBuf.st_dev == dinfo->st_dev &&
            ! dinfo->isLoopback) {
          int foundnew = 0;

          ++foundnew;
          if (dinfo->printFlag == DI_PRNT_OK) {
            ++foundnew;
          }
          if (! isIgnoreFSType (dinfo->fstype)) {
            ++foundnew;
          }
          if (foundnew == 3) {
            dinfo->printFlag = DI_PRNT_FORCE;
            found = true;
          }

          if (debug > 2) {
            printf ("file %s specified: found device %ld : %d (%s %s)\n",
                    diopts->argv [i], dinfo->st_dev, foundnew,
                    dinfo->devname, dinfo->mountpt);
          }

          if (inpool) {
            int   k;
            for (k = saveIdx; k < j; ++k) {
              dinfo = &(diskInfo [diskInfo [k].sortIndex[DI_TOT_SORT_IDX]]);
              if (dinfo->printFlag != DI_PRNT_FORCE) {
                dinfo->printFlag = DI_PRNT_SKIP;
              }
              if (debug > 2) {
                printf ("  inpool A: also process %s %s\n",
                        dinfo->devname, dinfo->mountpt);
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
  for (j = 0; j < di_data->count; ++j) {
    di_disk_info_t        *dinfo;

    dinfo = &di_data->diskInfo[j];
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

  for (i = 0; i < di_data->count; ++i) {
    di_disk_info_t        *dinfo;

    dinfo = &di_data->diskInfo [i];

    /* don't try to stat devices that are not accessible */
    if (dinfo->printFlag == DI_PRNT_EXCLUDE ||
        dinfo->printFlag == DI_PRNT_BAD ||
        dinfo->printFlag == DI_PRNT_OUTOFZONE) {
      continue;
    }

    dinfo->st_dev = (__ulong) DI_UNKNOWN_DEV;

    if (stat (dinfo->mountpt, &statBuf) == 0) {
      dinfo->st_dev = (__ulong) statBuf.st_dev;
      if (debug > 2) {
        printf ("dev: %s: %ld\n", dinfo->mountpt, dinfo->st_dev);
      }
    } else {
      if (errno != EACCES && errno != EPERM) {
        fprintf (stderr, "stat: %s ", dinfo->mountpt);
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
getDiskSpecialInfo (di_data_t *di_data, unsigned int dontResolveSymlink)
{
  int         i;
  struct stat statBuf;
  int         hasLoop;

  hasLoop = false;
  for (i = 0; i < di_data->count; ++i)
  {
    di_disk_info_t        *dinfo;

    dinfo = &di_data->diskInfo [i];

    /* check for initial slash; otherwise we can pick up normal files */
    if (*(dinfo->devname) == '/' &&
        stat (dinfo->devname, &statBuf) == 0) {
      int                 rc;

      if (! dontResolveSymlink && checkForUUID (dinfo->devname)) {
        struct stat tstatBuf;

        rc = lstat (dinfo->devname, &tstatBuf);
        if (rc == 0 && S_ISLNK (tstatBuf.st_mode)) {
          char tspecial [DI_DEVNAME_LEN + 1];
          if (realpath (dinfo->devname, tspecial) != (char *) NULL) {
            strncpy (dinfo->devname, tspecial, DI_DEVNAME_LEN);
          }
        }
      }
      dinfo->sp_dev = (__ulong) statBuf.st_dev;
      dinfo->sp_rdev = (__ulong) statBuf.st_rdev;

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
            dinfo->devname, dinfo->mountpt, dinfo->sp_dev,
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

    for (i = 0; i < di_data->count; ++i) {
        di_disk_info_t        *dinfo;

        dinfo = &di_data->diskInfo[i];
        dinfo->doPrint = true;
          /* these are never printed... */
        if (dinfo->printFlag == DI_PRNT_EXCLUDE ||
            dinfo->printFlag == DI_PRNT_BAD ||
            dinfo->printFlag == DI_PRNT_OUTOFZONE) {
            if (debug > 2) {
                printf ("chk: skipping(%s):%s\n",
                    getPrintFlagText ((int) dinfo->printFlag), dinfo->mountpt);
            }
            dinfo->doPrint = false;
            continue;
        }

          /* need to check against include list */
        if (dinfo->printFlag == DI_PRNT_IGNORE ||
            dinfo->printFlag == DI_PRNT_SKIP) {
          if (debug > 2) {
              printf ("chk: skipping(%s):%s\n",
                  getPrintFlagText ((int) dinfo->printFlag), dinfo->mountpt);
          }
          dinfo->doPrint = (char) diopts->displayAll;
        }

            /* Solaris reports a cdrom as having no free blocks,   */
            /* no available.  Their df doesn't always work right!  */
            /* -1 is returned.                                     */
        if (debug > 5) {
          char  tbuff [100];

          dinum_str (&dinfo->values [DI_SPACE_FREE], tbuff, sizeof (tbuff));
          printf ("chk: %s free: %s\n", dinfo->mountpt, tbuff);
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
          dinum_t      temp;

          dinum_init (&temp);

          dinum_set_u (&temp, ~0);
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
            printf ("chk: %s total: %s\n", dinfo->mountpt, tbuff);
          }

          if (isIgnoreFSType (dinfo->fstype)) {
            dinfo->printFlag = DI_PRNT_IGNORE;
            dinfo->doPrint = (char) diopts->displayAll;
            if (debug > 2) {
              printf ("chk: ignore-fstype: %s\n", dinfo->fstype);
            }
          }
          if (isIgnoreSpecial (dinfo->devname)) {
            dinfo->printFlag = DI_PRNT_IGNORE;
            dinfo->doPrint = (char) diopts->displayAll;
            if (debug > 2) {
              printf ("chk: ignore-special: %s\n", dinfo->devname);
            }
          }
          if (isIgnoreFS (dinfo->fstype, dinfo->mountpt)) {
            dinfo->printFlag = DI_PRNT_IGNORE;
            dinfo->doPrint = (char) diopts->displayAll;
            if (debug > 2) {
              printf ("chk: ignore-fs: %s\n", dinfo->mountpt);
            }
          }

          /* Some systems return a -1 or -2 as an indicator.    */
          if (dinum_cmp_s (&dinfo->values [DI_SPACE_TOTAL], 0) == 0 ||
              dinum_cmp_s (&dinfo->values [DI_SPACE_TOTAL], -1) == 0 ||
              dinum_cmp_s (&dinfo->values [DI_SPACE_TOTAL], -2L) == 0) {
            dinfo->printFlag = DI_PRNT_IGNORE;
            dinfo->doPrint = (char) diopts->displayAll;
            if (debug > 2) {
              printf ("chk: ignore: values [DI_SPACE_TOTAL] <= 0: %s\n",
                  dinfo->mountpt);
            }
          }
      }

      /* make sure anything in the include list didn't get turned off */
      checkIncludeList (dinfo, &diopts->include_list);
    } /* for all disks */

    if (hasLoop && diopts->excludeLoopback) {
          /* this loop sets duplicate entries to be ignored. */
      for (i = 0; i < di_data->count; ++i) {
        di_disk_info_t        *dinfo;

        dinfo = &di_data->diskInfo[i];
        if (dinfo->printFlag != DI_PRNT_OK) {
          if (debug > 2) {
              printf ("dup: chk: skipping(%s):%s\n",
                  getPrintFlagText ((int) dinfo->printFlag), dinfo->mountpt);
          }
          continue;
        }

          /* don't need to bother checking real partitions  */
        if (dinfo->sp_dev != 0 && dinfo->isLoopback) {
          __ulong         sp_dev;
          __ulong         sp_rdev;

          sp_dev = dinfo->sp_dev;
          sp_rdev = dinfo->sp_rdev;

          if (debug > 2) {
              printf ("dup: chk: i: %s dev: %ld rdev: %ld\n",
                  dinfo->mountpt, sp_dev, sp_rdev);
          }

          for (j = 0; j < di_data->count; ++j) {
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
                    dinfo->mountpt, sp_dev, dinfob->mountpt, dinfob->st_dev);
              }

              dinfo->printFlag = DI_PRNT_IGNORE;
              dinfo->doPrint = (char) diopts->displayAll;
              if (debug > 2)
              {
                  printf ("dup: chk: ignore: %s duplicate of %s\n",
                          dinfo->mountpt, dinfob->mountpt);
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
                    dinfo->mountpt, dinfo->printFlag,
                    dinfo->sp_dev, dinfo->sp_rdev, dinfo->fstype);
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

  for (i = 0; i < di_data->count; ++i)
  {
    di_disk_info_t        *dinfo;

    dinfo = &di_data->diskInfo[i];
    if (! dinfo->doPrint) {
      continue;
    }

    diqinfo.devname = dinfo->devname;
    diqinfo.mountpt = dinfo->mountpt;
    diqinfo.fstype = dinfo->fstype;
    diqinfo.uid = uid;
    diqinfo.gid = gid;
    for (j = 0; j < DI_QVAL_MAX; ++j) {
      dinum_init (&diqinfo.values [j]);
    }
    diquota (&diqinfo);

    if (debug > 2) {
      char    tbuff [100];
      dinum_str (&diqinfo.values [DI_QUOTA_LIMIT], tbuff, sizeof (tbuff));
      printf ("quota: %s limit: %s\n", dinfo->mountpt, tbuff);
      dinum_str (&dinfo->values [DI_SPACE_TOTAL], tbuff, sizeof (tbuff));
      printf ("quota:   tot: %s\n", tbuff);
      dinum_str (&diqinfo.values [DI_QUOTA_USED], tbuff, sizeof (tbuff));
      printf ("quota: %s used: %s\n", dinfo->mountpt, tbuff);
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
  int           hasLoop;

  hasLoop = false;
  diopts = (di_opt_t *) di_data->options;
  for (i = 0; i < di_data->count; ++i) {
    di_disk_info_t        *dinfo;

    dinfo = &di_data->diskInfo[i];

    dinfo->sortIndex[0] = (unsigned int) i;
    dinfo->sortIndex[1] = (unsigned int) i;
    if (debug > 4) {
      printf ("## prechk:%s:\n", dinfo->mountpt);
    }
    checkZone (dinfo, &di_data->zoneInfo, diopts);

    if (di_isPooledFs (dinfo)) {
      di_data->haspooledfs = true;
    }

    if (dinfo->printFlag == DI_PRNT_OK) {
      /* don't bother w/this check is all flag is set. */
      if (! diopts->displayAll) {
        di_is_remote_disk (dinfo);

        if (dinfo->isLocal == false && diopts->localOnly) {
          dinfo->printFlag = DI_PRNT_IGNORE;
          if (debug > 2) {
            printf ("prechk: ignore: remote disk; local flag set: %s\n",
                dinfo->mountpt);
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
checkIgnoreList (di_disk_info_t *diskInfo, di_strarr_t *ignore_list)
{
  char            *ptr;
  int             i;

  /* if the file system type is in the ignore list, skip it */
  if (ignore_list->count > 0) {
    for (i = 0; i < ignore_list->count; ++i) {
      ptr = ignore_list->list [i];
      if (debug > 2) {
        printf ("chkign: test: fstype %s/%s : %s\n", ptr,
            diskInfo->fstype, diskInfo->mountpt);
      }
      if (strcmp (ptr, diskInfo->fstype) == 0 ||
          (strcmp (ptr, "fuse") == 0 &&
          strncmp ("fuse", diskInfo->fstype, (Size_t) 4) == 0)) {
        diskInfo->printFlag = DI_PRNT_EXCLUDE;
        diskInfo->doPrint = false;
        if (debug > 2) {
          printf ("chkign: ignore: fstype %s match: %s\n", ptr,
              diskInfo->mountpt);
        }
        break;
      }
    }
  } /* if an ignore list was specified */
}

static void
checkIncludeList (di_disk_info_t *diskInfo, di_strarr_t *include_list)
{
  char            *ptr;
  int             i;

  /* if the file system type is not in the include list, skip it */
  if (include_list->count > 0) {
    for (i = 0; i < include_list->count; ++i) {
      ptr = include_list->list [i];
      if (debug > 2) {
        printf ("chkinc: test: fstype %s/%s : %s\n", ptr,
            diskInfo->fstype, diskInfo->mountpt);
      }

      if (strcmp (ptr, diskInfo->fstype) == 0 ||
          (strcmp (ptr, "fuse") == 0 &&
          strncmp ("fuse", diskInfo->fstype, (Size_t) 4) == 0)) {
        diskInfo->printFlag = DI_PRNT_OK;
        diskInfo->doPrint = true;
        if (debug > 2) {
          printf ("chkinc:include:fstype %s match: %s\n", ptr,
              diskInfo->mountpt);
        }
        break;
      } else {
        diskInfo->printFlag = DI_PRNT_EXCLUDE;
        diskInfo->doPrint = false;
        if (debug > 2) {
          printf ("chkinc:!include:fstype %s no match: %s\n", ptr,
              diskInfo->mountpt);
        }
      }
    }
  } /* if an include list was specified */
}

static void
checkZone (di_disk_info_t *diskInfo, di_zone_info_t *zoneInfo, di_opt_t *diopts)
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
          diskInfo->mountpt,
          zoneInfo->zones[i].rootpathlen,
          zoneInfo->zones[i].rootpath);
    }
    if (strncmp (zoneInfo->zones[i].rootpath,
        diskInfo->mountpt, zoneInfo->zones[i].rootpathlen) == 0) {
      if (debug > 4) {
          printf (" checkZone:%s:found zone:%s:\n",
                  diskInfo->mountpt, zoneInfo->zones[i].name);
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
          (int) zoneInfo->zones[idx].zoneid);
    }
    if (zoneInfo->myzoneid != zoneInfo->zones[idx].zoneid) {
      if (debug > 4) {
        printf (" checkZone:not root, not zone:%d:%d:outofzone:\n",
            (int) zoneInfo->myzoneid,
            (int) zoneInfo->zones[idx].zoneid);
      }
      diskInfo->printFlag = DI_PRNT_OUTOFZONE;
    }
  }

  if (debug > 5) {
    printf (" checkZone:chk name:%s:%s:\n",
        diopts->zoneDisplay, zoneInfo->zones [idx].name);
  }

  /* not the zone we want. ignore */
  if (! diopts->displayAll &&
      diskInfo->printFlag == DI_PRNT_OK &&
      strcmp (diopts->zoneDisplay, "all") != 0 &&
      strcmp (diopts->zoneDisplay, zoneInfo->zones[idx].name) != 0) {
    if (debug > 4) {
      printf (" checkZone:wrong zone:ignore:\n");
    }

    diskInfo->printFlag = DI_PRNT_IGNORE;
  }

  /* if displaying a non-global zone,   */
  /* don't display loopback filesystems */
  if (! diopts->displayAll &&
      diskInfo->printFlag == DI_PRNT_OK &&
      strcmp (diopts->zoneDisplay, "global") != 0 &&
      diskInfo->isLoopback) {
    if (debug > 4) {
      printf (" checkZone:non-global/lofs:ignore:\n");
    }

    diskInfo->printFlag = DI_PRNT_IGNORE;
  }

#endif
  return;
}

static void
initZones (di_data_t *di_data)
{
#if _lib_zone_list && _lib_getzoneid && _lib_zone_getattr
  di_data->zoneInfo.uid = geteuid ();
#endif
  di_data->zoneInfo.zoneCount = 0;
  di_data->zoneInfo.zones = (di_zone_summ_t *) NULL;

#if _lib_zone_list && _lib_getzoneid && _lib_zone_getattr
  {
    int             i;
    zoneid_t        *zids = (zoneid_t *) NULL;
    di_zone_info_t      *zi;

    zi = &di_data->zoneInfo;
    zi->myzoneid = getzoneid ();

    if (zone_list (zids, &zi->zoneCount) == 0)
    {
      if (zi->zoneCount > 0)
      {
        zids = malloc (sizeof (zoneid_t) * zi->zoneCount);
        if (zids == (zoneid_t *) NULL)
        {
          fprintf (stderr, "malloc failed in main() (1).  errno %d\n", errno);
          diopts->exitFlag = DI_EXIT_FAIL;
          return;
        }
        zone_list (zids, &zi->zoneCount);
        zi->zones = malloc (sizeof (di_zone_summ_t) *
                zi->zoneCount);
        if (zi->zones == (di_zone_summ_t *) NULL)
        {
          fprintf (stderr, "malloc failed in main() (2).  errno %d\n", errno);
          diopts->exitFlag = DI_EXIT_FAIL;
          return;
        }
      }
    }

    zi->globalIdx = 0;
    for (i = 0; i < (int) zi->zoneCount; ++i) {
      int     len;

      zi->zones[i].zoneid = zids[i];
      len = zone_getattr (zids[i], ZONE_ATTR_ROOT,
          zi->zones[i].rootpath, MAXPATHLEN);
      if (len >= 0) {
        zi->zones[i].rootpathlen = (Size_t) len;
        strncat (zi->zones[i].rootpath, "/", MAXPATHLEN);
        if (zi->zones[i].zoneid == 0) {
          zi->globalIdx = i;
        }

        len = zone_getattr (zids[i], ZONE_ATTR_NAME,
            zi->zones[i].name, ZONENAME_MAX);
        if (*diopts->zoneDisplay == '\0' &&
            zi->myzoneid == zi->zones[i].zoneid) {
          strncpy (diopts->zoneDisplay, zi->zones[i].name, MAXPATHLEN);
        }
        if (debug > 4) {
          printf ("zone:%d:%s:%s:\n", (int) zi->zones[i].zoneid,
              zi->zones[i].name, zi->zones[i].rootpath);
        }
      }
    }

    free ((void *) zids);
  }

  if (debug > 4) {
    printf ("zone:my:%d:%s:glob:%d:\n", (int) di_data->zoneInfo.myzoneid,
        di_data->zoneInfo.zoneDisplay, di_data->zoneInfo.globalIdx);
  }
#endif
}

static int
isIgnoreSpecial (const char *special)
{
  static const char   *appletimemachine = "com.apple.TimeMachine.";

  /* solaris: swap */
  /* linux: cgroup, tmpfs */
  if (strncmp (special, appletimemachine, strlen(appletimemachine)) == 0 ||
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
       strncmp (name, applesystem, strlen(applesystem)) == 0) {
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
    if (*(spec + offset + 8) == '-' &&
        *(spec + offset + 13) == '-' &&
        *(spec + offset + 18) == '-' &&
        *(spec + offset + 23) == '-' &&
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


/*
 * sortArray
 *
 */
void
sortArray (di_opt_t *diopts, di_disk_info_t *data, int count, int sidx)
{
  unsigned int  tempIndex;
  int           gap;
  int           j;
  int           i;

  if (count <= 1) {
    return;
  }

  gap = 1;
  while (gap < count) {
    gap = 3 * gap + 1;
  }

  for (gap /= 3; gap > 0; gap /= 3) {
    for (i = gap; i < count; ++i) {
      tempIndex = data[i].sortIndex[sidx];
      j = i - gap;

      while (j >= 0 && diCompare (diopts, data, data[j].sortIndex[sidx], tempIndex) > 0) {
        data[j + gap].sortIndex[sidx] = data[j].sortIndex[sidx];
        j -= gap;
      }

      j += gap;
      if (j != i) {
        data[j].sortIndex[sidx] = tempIndex;
      }
    }
  }
}

static int
diCompare (const di_opt_t *diopts, const di_disk_info_t *data,
           unsigned int idx1, unsigned int idx2)
{
    int             rc;
    int             sortOrder;
    const char            *ptr;
    const di_disk_info_t    *d1;
    const di_disk_info_t    *d2;

        /* reset sort order to the default start value */
    sortOrder = DI_SORT_ASCENDING;
    rc = 0;

    d1 = &(data[idx1]);
    d2 = &(data[idx2]);

    ptr = diopts->sortType;
    while (*ptr)
    {
      switch (*ptr)
      {
        case DI_SORT_NONE:
        {
            break;
        }

        case DI_SORT_MOUNT:
        {
            rc = strcoll (d1->mountpt, d2->mountpt);
            rc *= sortOrder;
            break;
        }

        case DI_SORT_REVERSE:
        {
            sortOrder *= -1;
            break;
        }

        case DI_SORT_SPECIAL:
        {
            rc = strcoll (d1->devname, d2->devname);
            rc *= sortOrder;
            break;
        }

        case DI_SORT_TYPE:
        {
            rc = strcoll (d1->fstype, d2->fstype);
            rc *= sortOrder;
            break;
        }

        case DI_SORT_AVAIL:
        case DI_SORT_FREE:
        case DI_SORT_TOTAL:
        {
          int   temp;

          temp = 0;
          switch (*ptr) {
            case DI_SORT_AVAIL:
            {
              temp = dinum_cmp (&d1->values [DI_SPACE_AVAIL], &d2->values [DI_SPACE_AVAIL]);
              break;
            }
            case DI_SORT_FREE:
            {
              temp = dinum_cmp (&d1->values [DI_SPACE_FREE], &d2->values [DI_SPACE_FREE]);
              break;
            }
            case DI_SORT_TOTAL:
            {
              temp = dinum_cmp (&d1->values [DI_SPACE_TOTAL], &d2->values [DI_SPACE_TOTAL]);
              break;
            }
          }

          rc *= sortOrder;
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

