/*
 * Copyright 1994-2018 Brad Lanam, Walnut Creek, CA
 * Copyright 2023 Brad Lanam, Pleasant Hill, CA
 */

#include "config.h"
#include "di.h"
#include "dimain.h"
#include "display.h"
#include "version.h"
#include "options.h"

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
#if _use_mcheck
# include <mcheck.h>
#endif

#if defined (__cplusplus) || defined (c_plusplus)
extern "C" {
#endif
#if _npt_getenv
  extern char *getenv _((const char *));
#endif
#if defined (__cplusplus) || defined (c_plusplus)
}
#endif

/* end of system specific includes/configurations */

#define DI_UNKNOWN_DEV          -1L

extern int debug;
int debug = { 0 };

#if _lib_zone_list && _lib_getzoneid && _lib_zone_getattr
static void checkZone           _((diDiskInfo_t *, zoneInfo_t *, unsigned int));
#endif
static void checkIgnoreList     _((diDiskInfo_t *, iList_t *));
static void checkIncludeList    _((diDiskInfo_t *, iList_t *));
static int  isIgnoreFSType      _((const char *));
static int  isIgnoreSpecial     _((const char *));
static int  isIgnoreFS          _((const char *, const char *));
#if _lib_realpath && _define_S_ISLNK && _lib_lstat
static int  checkForUUID        _((const char *));
#endif

char *
dimainproc (int argc, const char * const argv [], int intfcflag, diData_t *diData)
{
    diOptions_t         *diopts;
    diOutput_t          *diout;
    int                 hasLoop;
    int                 optidx;
    char                *disp;

        /* initialization */
    disp = (char *) NULL;

    diData->count = 0;
    diData->haspooledfs = FALSE;
    diData->disppooledfs = FALSE;
    diData->totsorted = FALSE;

    diData->diskInfo = (diDiskInfo_t *) NULL;

    diData->ignoreList.count = 0;
    diData->ignoreList.list = (char **) NULL;

    diData->includeList.count = 0;
    diData->includeList.list = (char **) NULL;

        /* options defaults */
    diopts = &diData->options;
    diopts->formatString = DI_DEFAULT_FORMAT;
        /* change default display format here */
    diopts->dispBlockSize = (_print_size_t) (DI_VAL_1024 * DI_VAL_1024);
    diopts->printTotals = FALSE;
    diopts->printDebugHeader = FALSE;
    diopts->printHeader = TRUE;
    diopts->localOnly = FALSE;
    diopts->displayAll = FALSE;
    diopts->dontResolveSymlink = FALSE;

    /* loopback devices (lofs) should be excluded by default */
    diopts->excludeLoopback = FALSE;
#if ! _lib_sys_dollar_device_scan  /* not VMS */
    diopts->excludeLoopback = TRUE;
#endif

    strncpy (diopts->sortType, "m", DI_SORT_MAX); /* default - by mount point*/
    diopts->posix_compat = FALSE;
    diopts->baseDispSize = (_print_size_t) DI_VAL_1024;
    diopts->baseDispIdx = DI_DISP_1024_IDX;
    diopts->quota_check = TRUE;
    diopts->csv_output = FALSE;
    diopts->csv_tabs = FALSE;
    diopts->exitFlag = DI_EXIT_NORM;
    diopts->errorCount = 0;
    diopts->json_output = FALSE;

    diout = &diData->output;
    diout->width = 8;
    diout->inodeWidth = 9;
    diout->maxMountString = 0;  /* 15 */
    diout->maxSpecialString = 0; /* 18 */
    diout->maxTypeString = 0; /* 7 */
    diout->maxOptString = 0;
    diout->maxMntTimeString = 0;

    if (intfcflag) {
      diopts->csv_output = TRUE;
      diopts->csv_tabs = TRUE;
      diopts->printHeader = FALSE;
    }

    initLocale ();

      /* first argument is defaults */
    optidx = getDIOptions (argc, argv, diData);
    if (diData->options.exitFlag != DI_EXIT_NORM) {
      return disp;
    }
    initZones (diData);
    if (diData->options.exitFlag != DI_EXIT_NORM) {
      return disp;
    }

    if (debug > 0)
    {
        printf ("di version %s\n", DI_VERSION);
    }

        /* main processing */

    if (di_getDiskEntries (&diData->diskInfo, &diData->count) < 0)
    {
        cleanup (diData);
        return disp;
    }

    hasLoop = FALSE;
    preCheckDiskInfo (diData);
    if (optidx < argc || diopts->excludeLoopback)
    {
      getDiskStatInfo (diData);
      hasLoop = getDiskSpecialInfo (diData, diopts->dontResolveSymlink);
    }
    if (optidx < argc)
    {
        int     rc;

        rc = checkFileInfo (diData, optidx, argc, argv);
        if (rc < 0)
        {
            cleanup (diData);
            diData->options.exitFlag = DI_EXIT_WARN;
            return disp;
        }
    }
    di_getDiskInfo (&diData->diskInfo, &diData->count);
    checkDiskInfo (diData, hasLoop);
    if (diopts->quota_check == TRUE) {
      checkDiskQuotas (diData);
    }
    disp = printDiskInfo (diData);
    return disp;
}

/*
 * cleanup
 *
 * free up allocated memory
 *
 */

extern void
cleanup (diData_t *diData)
{
    if (diData->diskInfo != (diDiskInfo_t *) NULL)
    {
        free ((char *) diData->diskInfo);
    }

    if (diData->ignoreList.count > 0 &&
        diData->ignoreList.list != (char **) NULL)
    {
        free ((char *) diData->ignoreList.list);
    }

    if (diData->includeList.count > 0 &&
        diData->includeList.list != (char **) NULL)
    {
        free ((char *) diData->includeList.list);
    }

    if (diData->zoneInfo.zones != (zoneSummary_t *) NULL)
    {
        free ((void *) diData->zoneInfo.zones);
    }
}

extern int
checkFileInfo (
    diData_t *diData,
    int optidx,
    int argc,
    const char * const argv [])
{
    int                 rc;
    int                 i;
    int                 j;
    struct stat         statBuf;
    diOptions_t         *diopts;
    diDiskInfo_t        *diskInfo;


    rc = 0;
    diopts = &diData->options;

    diskInfo = diData->diskInfo;
    if (diData->haspooledfs && ! diData->totsorted)
    {
      char tempSortType [DI_SORT_MAX + 2];

      strncpy (tempSortType, diopts->sortType, DI_SORT_MAX);
      strncpy (diopts->sortType, "s", DI_SORT_MAX);
      sortArray (diopts, diskInfo, diData->count, DI_TOT_SORT_IDX);
      strncpy (diopts->sortType, tempSortType, DI_SORT_MAX);
      diData->totsorted = TRUE;
    }

    for (i = optidx; i < argc; ++i)
    {
      int fd;
      int src;

      /* do this to automount devices.                    */
      /* stat() will not necessarily cause an automount.  */
      fd = open (argv[i], O_RDONLY | O_NOCTTY);
      if (fd < 0)
      {
        src = stat (argv [i], &statBuf);
      } else {
        src = fstat (fd, &statBuf);
      }

      if (src == 0)
      {
        int             saveIdx;
        int             found = { FALSE };
        int             inpool = { FALSE };
        Size_t          lastpoollen = { 0 };
        char            lastpool [DI_SPEC_NAME_LEN + 1];

        saveIdx = 0;  /* should get overridden below */
        for (j = 0; j < diData->count; ++j)
        {
          diDiskInfo_t    *dinfo;
          int             startpool;
          int             poolmain;

          startpool = FALSE;
          poolmain = FALSE;

          dinfo = &(diskInfo [diskInfo [j].sortIndex[DI_TOT_SORT_IDX]]);

              /* is it a pooled filesystem type? */
          if (diData->haspooledfs && di_isPooledFs (dinfo)) {
            if (lastpoollen == 0 ||
                strncmp (lastpool, dinfo->special, lastpoollen) != 0)
            {
              strncpy (lastpool, dinfo->special, DI_SPEC_NAME_LEN);
              lastpoollen = di_mungePoolName (lastpool);
              inpool = FALSE;
            }

            if (strncmp (lastpool, dinfo->special, lastpoollen) == 0)
            {
              startpool = TRUE;
              if (inpool == FALSE)
              {
                poolmain = TRUE;
              }
            }
          } else {
            inpool = FALSE;
          }

          if (poolmain) {
            saveIdx = j;
          }

          if (found && inpool) {
            dinfo = &(diskInfo [diskInfo [j].sortIndex[DI_TOT_SORT_IDX]]);
            dinfo->printFlag = DI_PRNT_SKIP;
            if (debug > 2) {
              printf ("  inpool B: also process %s %s\n",
                      dinfo->special, dinfo->name);
            }
          }

          if (dinfo->st_dev != (__ulong) DI_UNKNOWN_DEV &&
              (__ulong) statBuf.st_dev == dinfo->st_dev &&
              ! dinfo->isLoopback)
          {
            int foundnew = 0;

            ++foundnew;
            if (dinfo->printFlag == DI_PRNT_OK) {
              ++foundnew;
            }
            if (! isIgnoreFSType (dinfo->fsType)) {
              ++foundnew;
            }
            if (foundnew == 3) {
              dinfo->printFlag = DI_PRNT_FORCE;
              found = TRUE;
            }

            if (debug > 2) {
              printf ("file %s specified: found device %ld : %d (%s %s)\n",
                      argv[i], dinfo->st_dev, foundnew,
                      dinfo->special, dinfo->name);
            }

            if (inpool) {
              int   k;
              for (k = saveIdx; k < j; ++k) {
                dinfo = &(diskInfo [diskInfo [k].sortIndex[DI_TOT_SORT_IDX]]);
                if (dinfo->printFlag != DI_PRNT_FORCE) {
                  dinfo->printFlag = DI_PRNT_SKIP;
                }
                if (debug > 2)
                {
                  printf ("  inpool A: also process %s %s\n",
                          dinfo->special, dinfo->name);
                }
              }
            }
          }

          if (startpool)
          {
            inpool = TRUE;
          }
        }
      } /* if stat ok */
      else
      {
        if (errno != EACCES && errno != EPERM) {
          fprintf (stderr, "stat: %s ", argv[i]);
          perror ("");
        }
        rc = -1;
      }
      if (fd >= 0) {
        close (fd);
      }
    } /* for each file specified on command line */

        /* turn everything off */
    for (j = 0; j < diData->count; ++j)
    {
      diDiskInfo_t        *dinfo;

      dinfo = &diData->diskInfo[j];
      if (dinfo->printFlag == DI_PRNT_OK) {
        dinfo->printFlag = DI_PRNT_IGNORE;
      }
    }

    /* also turn off the -I and -x lists */
    diData->includeList.count = 0;
    diData->ignoreList.count = 0;
    return rc;
}


/*
 * getDiskStatInfo
 *
 * gets the disk device number for each entry.
 *
 */

extern void
getDiskStatInfo (diData_t *diData)
{
    int         i;
    struct stat statBuf;

    for (i = 0; i < diData->count; ++i)
    {
        diDiskInfo_t        *dinfo;

        dinfo = &diData->diskInfo [i];
            /* don't try to stat devices that are not accessible */
        if (dinfo->printFlag == DI_PRNT_EXCLUDE ||
            dinfo->printFlag == DI_PRNT_BAD ||
            dinfo->printFlag == DI_PRNT_OUTOFZONE)
        {
            continue;
        }

        dinfo->st_dev = (__ulong) DI_UNKNOWN_DEV;

        if (stat (dinfo->name, &statBuf) == 0)
        {
            dinfo->st_dev = (__ulong) statBuf.st_dev;
            if (debug > 2)
            {
                printf ("dev: %s: %ld\n", dinfo->name, dinfo->st_dev);
            }
        }
        else
        {
          if (errno != EACCES && errno != EPERM) {
            fprintf (stderr, "stat: %s ", dinfo->name);
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

extern int
getDiskSpecialInfo (diData_t *diData, unsigned int dontResolveSymlink)
{
    int         i;
    struct stat statBuf;
    int         hasLoop;

    hasLoop = FALSE;
    for (i = 0; i < diData->count; ++i)
    {
        diDiskInfo_t        *dinfo;

        dinfo = &diData->diskInfo [i];
           /* check for initial slash; otherwise we can pick up normal files */
        if (*(dinfo->special) == '/' &&
            stat (dinfo->special, &statBuf) == 0)
        {
#if _lib_realpath && _define_S_ISLNK && _lib_lstat
            int                 rc;

            if (! dontResolveSymlink && checkForUUID (dinfo->special)) {
              struct stat tstatBuf;

              rc = lstat (dinfo->special, &tstatBuf);
              if (rc == 0 && S_ISLNK(tstatBuf.st_mode)) {
                char tspecial [DI_SPEC_NAME_LEN + 1];
                if (realpath (dinfo->special, tspecial) != (char *) NULL) {
                  strncpy (dinfo->special, tspecial, DI_SPEC_NAME_LEN);
                }
              }
            }
#endif
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
              dinfo->isLoopback = TRUE;
              hasLoop = TRUE;
            }
            if (debug > 2)
            {
                printf ("special dev: %s %s: %ld rdev: %ld loopback: %d\n",
                        dinfo->special, dinfo->name, dinfo->sp_dev,
                        dinfo->sp_rdev, dinfo->isLoopback);
            }
        }
        else
        {
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

extern void
checkDiskInfo (diData_t *diData, int hasLoop)
{
    int             i;
    int             j;
    _fs_size_t      temp;
    diOptions_t     *diopts;


    diopts = &diData->options;

    for (i = 0; i < diData->count; ++i)
    {
        diDiskInfo_t        *dinfo;

        dinfo = &diData->diskInfo[i];
        dinfo->doPrint = TRUE;
          /* these are never printed... */
        if (dinfo->printFlag == DI_PRNT_EXCLUDE ||
            dinfo->printFlag == DI_PRNT_BAD ||
            dinfo->printFlag == DI_PRNT_OUTOFZONE)
        {
            if (debug > 2)
            {
                printf ("chk: skipping(%s):%s\n",
                    getPrintFlagText ((int) dinfo->printFlag), dinfo->name);
            }
            dinfo->doPrint = FALSE;
            continue;
        }

          /* need to check against include list */
        if (dinfo->printFlag == DI_PRNT_IGNORE ||
            dinfo->printFlag == DI_PRNT_SKIP)
        {
          if (debug > 2)
          {
              printf ("chk: skipping(%s):%s\n",
                  getPrintFlagText ((int) dinfo->printFlag), dinfo->name);
          }
          dinfo->doPrint = (char) diopts->displayAll;
        }

            /* Solaris reports a cdrom as having no free blocks,   */
            /* no available.  Their df doesn't always work right!  */
            /* -1 is returned.                                     */
        if (debug > 5)
        {
#if _siz_long_long >= 8
            printf ("chk: %s free: %llu\n", dinfo->name, dinfo->freeSpace);
#else
            printf ("chk: %s free: %lu\n", dinfo->name, dinfo->freeSpace);
#endif
        }
        if ((_s_fs_size_t) dinfo->freeSpace == -1L ||
            (_s_fs_size_t) dinfo->freeSpace == -2L)
        {
            dinfo->freeSpace = 0L;
        }
        if ((_s_fs_size_t) dinfo->availSpace == -1L ||
            (_s_fs_size_t) dinfo->availSpace == -2L)
        {
            dinfo->availSpace = 0L;
        }

        temp = (_fs_size_t) ~ 0;
        if (dinfo->totalInodes == temp)
        {
            dinfo->totalInodes = 0;
            dinfo->freeInodes = 0;
            dinfo->availInodes = 0;
        }

        if (dinfo->printFlag == DI_PRNT_OK)
        {
          if (debug > 5)
          {
#if _siz_long_long >= 8
              printf ("chk: %s total: %lld\n", dinfo->name, dinfo->totalSpace);
#else
              printf ("chk: %s total: %ld\n", dinfo->name, dinfo->totalSpace);
#endif
          }

          if (isIgnoreFSType (dinfo->fsType)) {
            dinfo->printFlag = DI_PRNT_IGNORE;
            dinfo->doPrint = (char) diopts->displayAll;
            if (debug > 2) {
              printf ("chk: ignore-fstype: %s\n", dinfo->fsType);
            }
          }
          if (isIgnoreSpecial (dinfo->special)) {
            dinfo->printFlag = DI_PRNT_IGNORE;
            dinfo->doPrint = (char) diopts->displayAll;
            if (debug > 2) {
              printf ("chk: ignore-special: %s\n", dinfo->special);
            }
          }
          if (isIgnoreFS (dinfo->fsType, dinfo->name)) {
            dinfo->printFlag = DI_PRNT_IGNORE;
            dinfo->doPrint = (char) diopts->displayAll;
            if (debug > 2) {
              printf ("chk: ignore-fs: %s\n", dinfo->name);
            }
          }

          /* Some systems return a -1 or -2 as an indicator.    */
          if (dinfo->totalSpace == 0L ||
              (_s_fs_size_t) dinfo->totalSpace == -1L ||
              (_s_fs_size_t) dinfo->totalSpace == -2L)
          {
            dinfo->printFlag = DI_PRNT_IGNORE;
            dinfo->doPrint = (char) diopts->displayAll;
            if (debug > 2)
            {
                printf ("chk: ignore: totalSpace <= 0: %s\n",
                        dinfo->name);
            }
          }
      }

      /* make sure anything in the include list didn't get turned off */
      checkIncludeList (dinfo, &diData->includeList);
    } /* for all disks */

    if (hasLoop && diopts->excludeLoopback)
    {
          /* this loop sets duplicate entries to be ignored. */
      for (i = 0; i < diData->count; ++i)
      {
        diDiskInfo_t        *dinfo;

        dinfo = &diData->diskInfo[i];
        if (dinfo->printFlag != DI_PRNT_OK)
        {
          if (debug > 2)
          {
              printf ("dup: chk: skipping(%s):%s\n",
                  getPrintFlagText ((int) dinfo->printFlag), dinfo->name);
          }
          continue;
        }

          /* don't need to bother checking real partitions  */
        if (dinfo->sp_dev != 0 && dinfo->isLoopback)
        {
          __ulong         sp_dev;
          __ulong         sp_rdev;

          sp_dev = dinfo->sp_dev;
          sp_rdev = dinfo->sp_rdev;

          if (debug > 2)
          {
              printf ("dup: chk: i: %s dev: %ld rdev: %ld\n",
                  dinfo->name, sp_dev, sp_rdev);
          }

          for (j = 0; j < diData->count; ++j)
          {
            diDiskInfo_t        *dinfob;

            if (i == j) {
              continue;
            }

            dinfob = &diData->diskInfo [j];
            if (dinfob->sp_dev != 0 &&
                dinfob->st_dev == sp_dev) {
              if (debug > 2)
              {
                printf ("dup: for %s %ld: found: %s %ld\n",
                    dinfo->name, sp_dev, dinfob->name, dinfob->st_dev);
              }

              dinfo->printFlag = DI_PRNT_IGNORE;
              dinfo->doPrint = (char) diopts->displayAll;
              if (debug > 2)
              {
                  printf ("dup: chk: ignore: %s duplicate of %s\n",
                          dinfo->name, dinfob->name);
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
                    dinfo->name, dinfo->printFlag,
                    dinfo->sp_dev, dinfo->sp_rdev, dinfo->fsType);
          }
        }
      } /* for each disk */
    } /* if the duplicate loopback mounts are to be excluded */
}

extern void
checkDiskQuotas (diData_t *diData)
{
  int           i;
  Uid_t         uid;
  Gid_t         gid;
  diQuota_t     diqinfo;
  _fs_size_t    tsize;

  uid = 0;
  gid = 0;
#if _has_std_quotas
  uid = geteuid ();
  gid = getegid ();
#endif

  for (i = 0; i < diData->count; ++i)
  {
    diDiskInfo_t        *dinfo;

    dinfo = &diData->diskInfo[i];
    if (! dinfo->doPrint) {
      continue;
    }

    diqinfo.special = dinfo->special;
    diqinfo.name = dinfo->name;
    diqinfo.type = dinfo->fsType;
    diqinfo.uid = uid;
    diqinfo.gid = gid;
    diquota (&diqinfo);

    if (debug > 2) {
      printf ("quota: %s limit: %lld\n", dinfo->name, diqinfo.limit);
      printf ("quota:   tot: %lld\n", dinfo->totalSpace);
      printf ("quota: %s used: %lld\n", dinfo->name, diqinfo.used);
      printf ("quota:   avail: %lld\n", dinfo->availSpace);
    }

    if (diqinfo.limit != 0 &&
            diqinfo.limit < dinfo->totalSpace) {
      dinfo->totalSpace = diqinfo.limit;
      tsize = diqinfo.limit - diqinfo.used;
      if ((_s_fs_size_t) tsize < 0) {
        tsize = 0;
      }
      if (tsize < dinfo->availSpace) {
        dinfo->availSpace = tsize;
        dinfo->freeSpace = tsize;
        if (debug > 2) {
          printf ("quota: using quota for: total free avail\n");
        }
      } else if (tsize > dinfo->availSpace && tsize < dinfo->freeSpace) {
        dinfo->freeSpace = tsize;
        if (debug > 2) {
          printf ("quota: using quota for: total free\n");
        }
      } else {
        if (debug > 2) {
          printf ("quota: using quota for: total\n");
        }
      }
    }

    if (diqinfo.ilimit != 0 &&
            diqinfo.ilimit < dinfo->totalInodes) {
      dinfo->totalInodes = diqinfo.ilimit;
      tsize = diqinfo.ilimit - diqinfo.iused;
      if ((_s_fs_size_t) tsize < 0) {
        tsize = 0;
      }
      if (tsize < dinfo->availInodes) {
        dinfo->availInodes = tsize;
        dinfo->freeInodes = tsize;
        if (debug > 2) {
          printf ("quota: using quota for inodes: total free avail\n");
        }
      } else if (tsize > dinfo->availInodes && tsize < dinfo->freeInodes) {
        dinfo->freeInodes = tsize;
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

extern void
preCheckDiskInfo (diData_t *diData)
{
    int             i;
    diOptions_t     *diopts;
    int             hasLoop;

    hasLoop = FALSE;
    diopts = &diData->options;
    for (i = 0; i < diData->count; ++i)
    {
        diDiskInfo_t        *dinfo;

        dinfo = &diData->diskInfo[i];

        dinfo->sortIndex[0] = (unsigned int) i;
        dinfo->sortIndex[1] = (unsigned int) i;
        if (debug > 4)
        {
            printf ("## prechk:%s:\n", dinfo->name);
        }
#if _lib_zone_list && _lib_getzoneid && _lib_zone_getattr
        checkZone (dinfo, &diData->zoneInfo, diopts->displayAll);
#endif

        if (di_isPooledFs (dinfo)) {
          diData->haspooledfs = TRUE;
        }

        if (dinfo->printFlag == DI_PRNT_OK)
        {
              /* don't bother w/this check is all flag is set. */
          if (! diopts->displayAll)
          {
            di_testRemoteDisk (dinfo);

            if (dinfo->isLocal == FALSE && diopts->localOnly)
            {
                dinfo->printFlag = DI_PRNT_IGNORE;
                if (debug > 2)
                {
                    printf ("prechk: ignore: remote disk; local flag set: %s\n",
                        dinfo->name);
                }
            }
          }
        }

        if (dinfo->printFlag == DI_PRNT_OK ||
            dinfo->printFlag == DI_PRNT_IGNORE)
        {
            /* do these checks to override the all flag */
            checkIgnoreList (dinfo, &diData->ignoreList);
            checkIncludeList (dinfo, &diData->includeList);
        }
    } /* for all disks */
}

static void
checkIgnoreList (diDiskInfo_t *diskInfo, iList_t *ignoreList)
{
    char            *ptr;
    int             i;

        /* if the file system type is in the ignore list, skip it */
    if (ignoreList->count > 0)
    {
      for (i = 0; i < ignoreList->count; ++i)
      {
        ptr = ignoreList->list [i];
        if (debug > 2)
        {
            printf ("chkign: test: fstype %s/%s : %s\n", ptr,
                    diskInfo->fsType, diskInfo->name);
        }
        if (strcmp (ptr, diskInfo->fsType) == 0 ||
            (strcmp (ptr, "fuse") == 0 &&
             strncmp ("fuse", diskInfo->fsType, (Size_t) 4) == 0))
        {
            diskInfo->printFlag = DI_PRNT_EXCLUDE;
            diskInfo->doPrint = FALSE;
            if (debug > 2)
            {
                printf ("chkign: ignore: fstype %s match: %s\n", ptr,
                        diskInfo->name);
            }
            break;
        }
      }
    } /* if an ignore list was specified */
}

static void
checkIncludeList (diDiskInfo_t *diskInfo, iList_t *includeList)
{
    char            *ptr;
    int             i;

        /* if the file system type is not in the include list, skip it */
    if (includeList->count > 0)
    {
      for (i = 0; i < includeList->count; ++i)
      {
        ptr = includeList->list [i];
        if (debug > 2)
        {
            printf ("chkinc: test: fstype %s/%s : %s\n", ptr,
                    diskInfo->fsType, diskInfo->name);
        }

        if (strcmp (ptr, diskInfo->fsType) == 0 ||
            (strcmp (ptr, "fuse") == 0 &&
             strncmp ("fuse", diskInfo->fsType, (Size_t) 4) == 0))
        {
            diskInfo->printFlag = DI_PRNT_OK;
            diskInfo->doPrint = TRUE;
            if (debug > 2)
            {
                printf ("chkinc:include:fstype %s match: %s\n", ptr,
                        diskInfo->name);
            }
            break;
        }
        else
        {
            diskInfo->printFlag = DI_PRNT_EXCLUDE;
            diskInfo->doPrint = FALSE;
            if (debug > 2)
            {
                printf ("chkinc:!include:fstype %s no match: %s\n", ptr,
                        diskInfo->name);
            }
        }
      }
    } /* if an include list was specified */
}

#if _lib_zone_list && _lib_getzoneid && _lib_zone_getattr
static void
checkZone (diDiskInfo_t *diskInfo, zoneInfo_t *zoneInfo, unsigned int allFlag)
{
    int         i;
    int         idx = { -1 };

    if (strcmp (zoneInfo->zoneDisplay, "all") == 0 &&
        zoneInfo->uid == 0)
    {
        return;
    }

    for (i = 0; i < (int) zoneInfo->zoneCount; ++i)
    {
        /* find the zone the filesystem is in, if non-global */
        if (debug > 5)
        {
            printf (" checkZone:%s:compare:%d:%s:\n",
                    diskInfo->name,
                    zoneInfo->zones[i].rootpathlen,
                    zoneInfo->zones[i].rootpath);
        }
        if (strncmp (zoneInfo->zones[i].rootpath,
             diskInfo->name, zoneInfo->zones[i].rootpathlen) == 0)
        {
            if (debug > 4)
            {
                printf (" checkZone:%s:found zone:%s:\n",
                        diskInfo->name, zoneInfo->zones[i].name);
            }
            idx = i;
            break;
        }
        if (idx == -1)
        {
            idx = zoneInfo->globalIdx;
        }
    }

        /* no root access, ignore any zones     */
        /* that don't match our zone id         */
        /* this will override any ignore flags  */
        /* already set                          */
    if (zoneInfo->uid != 0)
    {
        if (debug > 5)
        {
            printf (" checkZone:uid non-zero:chk zone:%d:%d:\n",
                    (int) zoneInfo->myzoneid,
                    (int) zoneInfo->zones[idx].zoneid);
        }
        if (zoneInfo->myzoneid != zoneInfo->zones[idx].zoneid)
        {
            if (debug > 4)
            {
                printf (" checkZone:not root, not zone:%d:%d:outofzone:\n",
                        (int) zoneInfo->myzoneid,
                        (int) zoneInfo->zones[idx].zoneid);
            }
            diskInfo->printFlag = DI_PRNT_OUTOFZONE;
        }
    }

    if (debug > 5)
    {
        printf (" checkZone:chk name:%s:%s:\n",
                zoneInfo->zoneDisplay, zoneInfo->zones[idx].name);
    }
        /* not the zone we want. ignore */
    if (! allFlag &&
        diskInfo->printFlag == DI_PRNT_OK &&
        strcmp (zoneInfo->zoneDisplay, "all") != 0 &&
        strcmp (zoneInfo->zoneDisplay,
                zoneInfo->zones[idx].name) != 0)
    {
        if (debug > 4)
        {
            printf (" checkZone:wrong zone:ignore:\n");
        }

        diskInfo->printFlag = DI_PRNT_IGNORE;
    }

        /* if displaying a non-global zone,   */
        /* don't display loopback filesystems */
    if (! allFlag &&
        diskInfo->printFlag == DI_PRNT_OK &&
        strcmp (zoneInfo->zoneDisplay, "global") != 0 &&
        diskInfo->isLoopback)
    {
        if (debug > 4)
        {
            printf (" checkZone:non-global/lofs:ignore:\n");
        }

        diskInfo->printFlag = DI_PRNT_IGNORE;
    }

    return;
}
#endif

extern void
initLocale (void)
{
#if _enable_nls
  const char      *localeptr;
#endif

#if _lib_setlocale && defined (LC_ALL)
  setlocale (LC_ALL, "");
#endif
#if _enable_nls
  if ((localeptr = getenv ("DI_LOCALE_DIR")) == (char *) NULL) {
    localeptr = DI_LOCALE_DIR;
  }
  bindtextdomain ("di", localeptr);
  textdomain ("di");
#endif
}

extern void
initZones (diData_t *diData)
{
#if _lib_zone_list && _lib_getzoneid && _lib_zone_getattr
  diData->zoneInfo.uid = geteuid ();
#endif
  diData->zoneInfo.zoneDisplay [0] = '\0';
  diData->zoneInfo.zoneCount = 0;
  diData->zoneInfo.zones = (zoneSummary_t *) NULL;

#if _lib_zone_list && _lib_getzoneid && _lib_zone_getattr
  {
    int             i;
    zoneid_t        *zids = (zoneid_t *) NULL;
    zoneInfo_t      *zi;

    zi = &diData->zoneInfo;
    zi->myzoneid = getzoneid ();

    if (zone_list (zids, &zi->zoneCount) == 0)
    {
      if (zi->zoneCount > 0)
      {
        zids = malloc (sizeof (zoneid_t) * zi->zoneCount);
        if (zids == (zoneid_t *) NULL)
        {
          fprintf (stderr, "malloc failed in main() (1).  errno %d\n", errno);
          diData->options.exitFlag = DI_EXIT_FAIL;
          return;
        }
        zone_list (zids, &zi->zoneCount);
        zi->zones = malloc (sizeof (zoneSummary_t) *
                zi->zoneCount);
        if (zi->zones == (zoneSummary_t *) NULL)
        {
          fprintf (stderr, "malloc failed in main() (2).  errno %d\n", errno);
          diData->options.exitFlag = DI_EXIT_FAIL;
          return;
        }
      }
    }

    zi->globalIdx = 0;
    for (i = 0; i < (int) zi->zoneCount; ++i)
    {
      int     len;

      zi->zones[i].zoneid = zids[i];
      len = zone_getattr (zids[i], ZONE_ATTR_ROOT,
              zi->zones[i].rootpath, MAXPATHLEN);
      if (len >= 0)
      {
        zi->zones[i].rootpathlen = (Size_t) len;
        strncat (zi->zones[i].rootpath, "/", MAXPATHLEN);
        if (zi->zones[i].zoneid == 0)
        {
          zi->globalIdx = i;
        }

        len = zone_getattr (zids[i], ZONE_ATTR_NAME,
            zi->zones[i].name, ZONENAME_MAX);
        if (*zi->zoneDisplay == '\0' &&
            zi->myzoneid == zi->zones[i].zoneid)
        {
          strncpy (zi->zoneDisplay, zi->zones[i].name, MAXPATHLEN);
        }
        if (debug > 4)
        {
          printf ("zone:%d:%s:%s:\n", (int) zi->zones[i].zoneid,
              zi->zones[i].name, zi->zones[i].rootpath);
        }
      }
    }

    free ((void *) zids);
  }

  if (debug > 4)
  {
    printf ("zone:my:%d:%s:glob:%d:\n", (int) diData->zoneInfo.myzoneid,
        diData->zoneInfo.zoneDisplay, diData->zoneInfo.globalIdx);
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
    return TRUE;
  }
  return FALSE;
}

static int
isIgnoreFS (const char *fstype, const char *name)
{
  static const char   *applesystem = "/System/";

  if (strcmp (fstype, "apfs") == 0 &&
       strncmp (name, applesystem, strlen(applesystem)) == 0) {
    return TRUE;
  }
  return FALSE;
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
    return TRUE;
  }
  return FALSE;
}

#if _lib_realpath && _define_S_ISLNK && _lib_lstat
static int
checkForUUID (const char *spec)
{
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
      return TRUE;
    }
  }
  return FALSE;
}
#endif /* have _lib_realpath, etc. */
