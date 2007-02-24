/*
$Id$
$Source$
Copyright 1994-2006 Brad Lanam, Walnut Creek, CA
*/

/*
 * di.c
 *
 *   Copyright 1994-2004 Brad Lanam,  Walnut Creek, CA
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
 *  Sort types (by name is default):
 *      n - none (mount order)
 *      s - special
 *      S - size (available blocks)
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
 *          (number of blocks not available for use / total disk space)
 *             [ (tot - avail) / tot ]
 *      1 - percentage used.
 *          (actual number of blocks used / total disk space)
 *             [ (tot - free) / tot ]
 *      2 - percentage of user-available space in use (bsd style).
 *          Note that values over 100% are possible.
 *          (actual number of blocks used / disk space available to user)
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
 *  System V.4 `/usr/bin/df -v` Has format: msbuf1 (w/-dp option: 512 byte blocks)
 *  System V.4 `/usr/bin/df -k` Has format: sbcvpm
 *  System V.4 `/usr/ucb/df`    Has format: sbuv2m
 *
 *  The default format string for this program is: smbuvpT
 *
 *  The environment variable "DIFMT" may be set to the desired format
 *  string.
 *
 *  Note that for filesystems that do not have (S512K fs) or systems (SysV.3)
 *  that do not report available blocks, the number of available blocks is
 *  equal to the number of free blocks.
 *
 */

#include "config.h"
#include "di.h"
#include "version.h"

#include <stdio.h>
#if _hdr_ctype
# include <ctype.h>
#endif
#if _hdr_errno
# include <errno.h>
#endif
#if _hdr_stdlib
# include <stdlib.h>
#endif
#if _hdr_getopt
# include <getopt.h>
#endif
#if _sys_types
# include <sys/types.h>
#endif
#if _hdr_string
# include <string.h>
#endif
#if _hdr_strings && ((! defined (_hdr_string)) || (_include_string))
# include <strings.h>
#endif
#if _hdr_memory
# include <memory.h>
#endif
#if _include_malloc && _hdr_malloc
# include <malloc.h>
#endif
#if _hdr_math
# include <math.h>
#endif
#if _hdr_unistd
# include <unistd.h>
#endif
#if _hdr_time
# include <time.h>
#endif
#if _sys_time
# include <sys/time.h>
#endif
#if _sys_stat
# include <sys/stat.h>
#endif
#if _hdr_libintl
# include <libintl.h>
#endif
#if _hdr_locale
# include <locale.h>
#endif
#if _hdr_zone
# include <zone.h>
#endif

extern int di_lib_debug;

#if _npt_getenv
  extern char *getenv _((char *));
#endif

#if ! defined (_dcl_errno)
  extern int     errno;
#endif
#if ! defined (_dcl_optind)
  extern int optind;
#endif
#if ! defined (_dcl_optarg)
  extern char *optarg;
#endif

     /* macro for gettext() */
#ifndef GT
# if _enable_nls
#  define GT(args) gettext (args)
# else
#  define GT(args) (args)
# endif
#endif

/* end of system specific includes/configurations */

#define DI_F_ALL               0x00000001
#define DI_F_LOCAL_ONLY        0x00000002
#define DI_F_TOTAL             0x00000010
#define DI_F_NO_HEADER         0x00000020
#define DI_F_DEBUG_HDR         0x00000040
#define DI_F_INCLUDE_LOOPBACK  0x00000080

    /* mount information */
#define DI_FMT_MOUNT           'm'
#define DI_FMT_MOUNT_FULL      'M'
#define DI_FMT_SPECIAL         's'
#define DI_FMT_SPECIAL_FULL    'S'
#define DI_FMT_TYPE            't'
#define DI_FMT_TYPE_FULL       'T'

    /* disk information */
#define DI_FMT_BTOT            'b'
#define DI_FMT_BTOT_AVAIL      'B'
#define DI_FMT_BUSED           'u'
#define DI_FMT_BCUSED          'c'
#define DI_FMT_BFREE           'f'
#define DI_FMT_BAVAIL          'v'
#define DI_FMT_BPERC_NAVAIL    'p'
#define DI_FMT_BPERC_USED      '1'
#define DI_FMT_BPERC_BSD       '2'
#define DI_FMT_BPERC_AVAIL     'a'
#define DI_FMT_BPERC_FREE      '3'
#define DI_FMT_ITOT            'i'
#define DI_FMT_IUSED           'U'
#define DI_FMT_IFREE           'F'
#define DI_FMT_IPERC           'P'
#define DI_FMT_MOUNT_TIME      'I'
#define DI_FMT_MOUNT_OPTIONS   'O'

#define DI_SORT_NONE            0
#define DI_SORT_NAME            1
#define DI_SORT_SPECIAL         2
#define DI_SORT_AVAIL           3

#define DI_SORT_ASCENDING       1
#define DI_SORT_DESCENDING      -1

#define DI_UNKNOWN_DEV          -1L
#define DI_LIST_SEP             ","
#define DI_ARGV_SEP             " 	"  /* space, tab */
#define DI_MAX_ARGV             30

#define DI_ALL_FORMAT           "MTS\n\tIO\n\tbuf13\n\tbcvpa\n\tBuv2\n\tiUFP"
#define DI_POSIX_FORMAT         "SbuvpM"

#define DI_VAL_1000             1000.0
#define DI_VAL_1024             1024.0
    /* these are indexes into the dispTable array... */
#define DI_ONE_K                0
#define DI_ONE_MEG              1
#define DI_ONE_GIG              2
#define DI_ONE_TERA             3
#define DI_ONE_PETA             4
#define DI_ONE_EXA              5
#define DI_ONE_ZETTA            6
#define DI_ONE_YOTTA            7
#define DI_DISP_HR              -20
#define DI_DISP_HR_2            -21

   /* you may want to change some of these values.  Be sure to change all */
   /* related entries.                                                    */
#if ! defined (DI_DEFAULT_FORMAT)
# define DI_DEFAULT_FORMAT      "smbuvpT"
/* # define DI_DEFAULT_FORMAT      "MbuvpT" */ /* an alternative */
#endif
#if _lib_mnt_time
# define DI_DEF_MOUNT_FORMAT    "MST\n\tI\n\tO"
#else
# define DI_DEF_MOUNT_FORMAT    "MST\n\tO"
#endif
#define DI_PERC_FMT             "%3.0f%% "
#define DI_POSIX_PERC_FMT       "   %3.0f%% "
#define DI_POSIX_PERC_LBL_FMT   "%8s"
#define DI_PERC_LBL_FMT         "%5s"
#define DI_FSTYPE_FMT           "%-7.7s"
#define DI_MOUNT_FMT            "%-15.15s"
#define DI_SPEC_FMT             "%-18.18s"

typedef int (*DI_SORT_FUNC) _((void *, void *));

typedef struct
{
    int		    count;
    char        **list;
} iList;

typedef struct
{
    double          low;
    double          high;
    double          dbs;
    char            *format;
    char            *suffix;
} sizeTable_t;

typedef struct
{
    double          size;
    char            *disp;
} dispTable_t;

#if ! _lib_zone_list
# define zoneid_t       int
# define ZONENAME_MAX   65
#endif

typedef struct {
    zoneid_t    zoneid;
    char        name[ZONENAME_MAX];
    char        rootpath[MAXPATHLEN];
    Size_t      rootpathlen;
} zoneSummary_t;

typedef struct {
    Uid_t           uid;
    zoneid_t        myzoneid;
    zoneSummary_t   *zones;
    Uint_t          zoneCount;
    char            zoneDisplay [MAXPATHLEN];
    int             globalIdx;
} zoneInfo_t;

static int          debug = { 0 };
static __ulong      flags = { 0 };
/* Linux users may want as the default: */
/* static __ulong      flags = { DI_F_INCLUDE_LOOPBACK }; */
static int          sortType = { DI_SORT_NAME };
static int          sortOrder = { DI_SORT_ASCENDING };
static int          posix_compat = { 0 };

static char         *formatString = { DI_DEFAULT_FORMAT };
static char         mountFormat [20];
static char         specialFormat [20];
static char         typeFormat [20];
static char         optFormat [20];
static char         mTimeFormat [20];
static int          width = { 8 };
static int          inodeWidth = { 7 };
static char         blockFormat [40];
static char         blockFormatNR [40];   /* no radix */
static char         blockLabelFormat [40];
static char         inodeFormat [40];
static char         inodeLabelFormat [40];
static double       dispBlockSize = { DI_VAL_1024 * DI_VAL_1024 };
static char         dispBlockLabel [30];
static double       baseDispSize;

static sizeTable_t sizeTable [] =
{
    { 0, 0, 1, blockFormatNR, " " },
    { 0, 0, 0, blockFormat,   "k" },
#define DI_ONE_MEG_SZTAB   2
    { 0, 0, 0, blockFormat,   "M" },
    { 0, 0, 0, blockFormat,   "G" },
    { 0, 0, 0, blockFormat,   "T" },
    { 0, 0, 0, blockFormat,   "P" },
    { 0, 0, 0, blockFormat,   "E" },
    { 0, 0, 0, blockFormat,   "Z" },
    { 0, 0, 0, blockFormat,   "Y" }
};
#define DI_SIZETAB_SIZE (sizeof (sizeTable) / sizeof (sizeTable_t))

static dispTable_t dispTable [] =
{
    { 0.0, "KBytes" },
    { 0.0, "Megs" },
    { 0.0, "Gigs" },
    { 0.0, "Teras" },
    { 0.0, "Petas" },
    { 0.0, "Exas" },
    { 0.0, "Zettas" },
    { 0.0, "Yottas" }
};
#define DI_DISPTAB_SIZE (sizeof (dispTable) / sizeof (dispTable_t))

static void cleanup             _((di_DiskInfo *, iList *, iList *, char *, zoneInfo_t *));
static void printDiskInfo       _((di_DiskInfo *, int));
static void printInfo           _((di_DiskInfo *));
static void printBlocks          _((_fs_size_t, _fs_size_t, int));
static void addTotals           _((di_DiskInfo *, di_DiskInfo *));
static void printTitle          _((void));
static void printPerc           _((_fs_size_t, _fs_size_t, char *));
static void sortArray           _((char *, Size_t, int, DI_SORT_FUNC));
static int  diCompare           _((void *, void *));
static void getDiskStatInfo     _((di_DiskInfo *, int));
static void getDiskSpecialInfo  _((di_DiskInfo *, int));
static int  checkFileInfo       _((di_DiskInfo *, int, int, int, char *[]));
static void preCheckDiskInfo    _((di_DiskInfo *, iList *, iList *, int, zoneInfo_t *));
static void checkDiskInfo       _((di_DiskInfo *, iList *, int));
static void usage               _((void));
static void processArgs         _((int, char *[], iList *, iList *, char *, char *));
static void parseList           _((iList *, char *));
static void checkIgnoreList     _((di_DiskInfo *, iList *));
static void checkIncludeList    _((di_DiskInfo *, iList *));
static void checkZone           _((di_DiskInfo *, zoneInfo_t *, int));
static void setDispBlockSize    _((char *));
static int  findDispSize        _((double));
static char *getPrintFlagText   _((int));
#if ! _mth_fmod && ! _lib_fmod
 static double fmod _((double, double));
#endif

int
#if _proto_stdc
main (int argc, char *argv [])
#else
main (argc, argv)
    int                 argc;
    char                *argv [];
#endif
{
    di_DiskInfo         *diskInfo = { (di_DiskInfo *) NULL };
    int                 i;
    int                 diCount = { 0 };
    iList               ignoreList;
    iList               includeList;
    char                *ptr;
    char                *argvptr;
    char                dbsstr [30];
    zoneInfo_t          zoneInfo;

    *dbsstr = '\0';
    ignoreList.count = 0;
    ignoreList.list = (char **) NULL;
    includeList.count = 0;
    includeList.list = (char **) NULL;

#if _lib_setlocale && defined (LC_ALL)
    ptr = setlocale (LC_ALL, "");
#endif
#if _enable_nls
    ptr = bindtextdomain (PROG, DI_LOCALE_DIR);
    ptr = textdomain (PROG);
#endif
    baseDispSize = DI_VAL_1024;

    ptr = argv [0] + strlen (argv [0]) - 2;
    if (memcmp (ptr, MPROG, 2) == 0)
    {
        formatString = DI_DEF_MOUNT_FORMAT;
    }
    else    /* don't use DIFMT env var if running mi. */
    {
        if ((ptr = getenv ("DIFMT")) != (char *) NULL)
        {
            formatString = ptr;
        }
    }

        /* gnu df */
    if ((ptr = getenv ("POSIXLY_CORRECT")) != (char *) NULL)
    {
        strncpy (dbsstr, "512", sizeof (dbsstr));
        formatString = DI_POSIX_FORMAT;
        posix_compat = 1;
    }

        /* bsd df */
    if ((ptr = getenv ("BLOCKSIZE")) != (char *) NULL)
    {
        strncpy (dbsstr, ptr, sizeof (dbsstr));
    }

        /* gnu df */
    if ((ptr = getenv ("DF_BLOCK_SIZE")) != (char *) NULL)
    {
        strncpy (dbsstr, ptr, sizeof (dbsstr));
    }

    zoneInfo.uid = geteuid ();
    zoneInfo.zoneDisplay [0] = '\0';
    zoneInfo.zoneCount = 0;
    zoneInfo.zones = (zoneSummary_t *) NULL;

    argvptr = (char *) NULL;
    if ((ptr = getenv ("DI_ARGS")) != (char *) NULL)
    {
        char        *tptr;
        int         nargc;
        int         ooptind;
        char        *ooptarg;
        char        *nargv [DI_MAX_ARGV];

        argvptr = strdup (ptr);
        ooptind = optind;
        ooptarg = optarg;
        if (argvptr != (char *) NULL)
        {
            tptr = strtok (argvptr, DI_ARGV_SEP);
            nargc = 1;
            nargv[0] = argv[0];
            while (tptr != (char *) NULL)
            {
                if (nargc >= DI_MAX_ARGV)
                {
                    break;
                }
                nargv[nargc++] = tptr;
                tptr = strtok ((char *) NULL, DI_ARGV_SEP);
            }
            processArgs (nargc, nargv, &ignoreList, &includeList, dbsstr,
                         zoneInfo.zoneDisplay);
            optind = ooptind;     /* reset so command line can be parsed */
            optarg = ooptarg;
        }
    }

    processArgs (argc, argv, &ignoreList, &includeList, dbsstr,
            zoneInfo.zoneDisplay);

#if _lib_zone_list && _lib_getzoneid && _lib_zone_getattr
    {
        zoneid_t        *zids = (zoneid_t *) NULL;

        zoneInfo.myzoneid = getzoneid ();

        if (zone_list (zids, &zoneInfo.zoneCount) == 0)
        {
            if (zoneInfo.zoneCount > 0)
            {
                zids = malloc (sizeof (zoneid_t) * zoneInfo.zoneCount);
                if (zids == (zoneid_t *) NULL)
                {
                    fprintf (stderr, "malloc failed in main() (1).  errno %d\n", errno);
                    exit (1);
                }
                zone_list (zids, &zoneInfo.zoneCount);
                zoneInfo.zones = malloc (sizeof (zoneSummary_t) * zoneInfo.zoneCount);
                if (zoneInfo.zones == (zoneSummary_t *) NULL)
                {
                    fprintf (stderr, "malloc failed in main() (2).  errno %d\n", errno);
                    exit (1);
                }
            }
        }

        zoneInfo.globalIdx = 0;
        for (i = 0; i < zoneInfo.zoneCount; ++i)
        {
            int     len;

            zoneInfo.zones[i].zoneid = zids[i];
            len = zone_getattr (zids[i], ZONE_ATTR_ROOT,
                    zoneInfo.zones[i].rootpath, MAXPATHLEN);
            if (len >= 0)
            {
                zoneInfo.zones[i].rootpathlen = len;
                strncat (zoneInfo.zones[i].rootpath, "/", MAXPATHLEN - 1);
                if (zoneInfo.zones[i].zoneid == 0)
                {
                    zoneInfo.globalIdx = i;
                }

                len = zone_getattr (zids[i], ZONE_ATTR_NAME,
                        zoneInfo.zones[i].name, ZONENAME_MAX);
                if (*zoneInfo.zoneDisplay == '\0' &&
                    zoneInfo.myzoneid == zoneInfo.zones[i].zoneid)
                {
                    strncpy (zoneInfo.zoneDisplay, zoneInfo.zones[i].name,
                             ZONENAME_MAX);
                }
                if (debug > 4)
                {
                    printf ("zone:%d:%s:%s:\n",
                            (int) zoneInfo.zones[i].zoneid,
                            zoneInfo.zones[i].name,
                            zoneInfo.zones[i].rootpath);
                }
            }
        }

        free ((void *) zids);
    }

    if (debug > 4)
    {
        printf ("zone:my:%d:%s:glob:%d:\n", (int) zoneInfo.myzoneid,
                zoneInfo.zoneDisplay, zoneInfo.globalIdx);
    }
#endif

        /* initialize dispTable array */
    dispTable [0].size = baseDispSize;
    for (i = 1; i < DI_DISPTAB_SIZE; ++i)
    {
        dispTable [i].size = dispTable [i - 1].size * baseDispSize;
    }
    strncpy (dispBlockLabel, GT(dispTable [DI_ONE_MEG].disp),  /* default */
             sizeof (dispBlockLabel));

        /* do this afterwards, because we need to know what the */
        /* user wants for baseDispSize                          */
    setDispBlockSize (dbsstr);

        /* initialize display size tables */
    sizeTable [0].high = baseDispSize;
    sizeTable [1].low = baseDispSize;
    sizeTable [1].high = baseDispSize * baseDispSize;
    sizeTable [1].dbs = baseDispSize;
    for (i = 2; i < DI_SIZETAB_SIZE; ++i)
    {
        sizeTable [i].low = sizeTable [i - 1].low * baseDispSize;
        sizeTable [i].high = sizeTable [i - 1].high * baseDispSize;
        sizeTable [i].dbs = sizeTable [i - 1].dbs * baseDispSize;
    }

    if (debug > 0)
    {
        printf ("di ver %s\n", DI_VERSION);
    }

    if (debug > 4)
    {
        for (i = 0; i < DI_DISPTAB_SIZE; ++i)
        {
            printf ("i:%d ", i);
            printf ("size: %8.2f ", dispTable[i].size);
            printf ("%s ", dispTable[i].disp);
            printf ("\n");
        }

        printf ("dispBlockSize: %8.2f\n", dispBlockSize);
        for (i = 0; i < DI_SIZETAB_SIZE; ++i)
        {
            printf ("i:%d ", i);
            printf ("suffix: %s ", sizeTable[i].suffix);
            printf ("low: %8.2f ", sizeTable[i].low);
            printf ("high: %8.2f ", sizeTable[i].high);
            printf ("dbs: %8.2f ", sizeTable[i].dbs);
            printf ("\n");
        }
    }

    if (di_getDiskEntries (&diskInfo, &diCount) < 0)
    {
        cleanup (diskInfo, &ignoreList, &includeList, argvptr, &zoneInfo);
        exit (1);
    }

    preCheckDiskInfo (diskInfo, &ignoreList, &includeList, diCount, &zoneInfo);
    if (optind < argc)
    {
        int     rc;

        getDiskStatInfo (diskInfo, diCount);
        rc = checkFileInfo (diskInfo, diCount, optind, argc, argv);
        if (rc < 0)
        {
            cleanup (diskInfo, &ignoreList, &includeList, argvptr, &zoneInfo);
            exit (1);
        }
    }
    di_getDiskInfo (&diskInfo, &diCount);
    getDiskSpecialInfo (diskInfo, diCount);
    checkDiskInfo (diskInfo, &includeList, diCount);
    printDiskInfo (diskInfo, diCount);

    cleanup (diskInfo, &ignoreList, &includeList, argvptr, &zoneInfo);
    exit (0);
}

/*
 * cleanup
 *
 * free up allocated memory
 *
 */

static void
#if _proto_stdc
cleanup (di_DiskInfo *diskInfo, iList *ignoreList, iList *includeList,
            char *argvptr, zoneInfo_t *zoneInfo)
#else
cleanup (diskInfo, ignoreList, includeList, argvptr, zoneInfo)
    di_DiskInfo     *diskInfo;
    iList           *ignoreList;
    iList           *includeList;
    char            *argvptr;
    zoneInfo_t      *zoneInfo;
#endif
{
    if (diskInfo != (di_DiskInfo *) NULL)
    {
        free ((char *) diskInfo);
    }

    if (ignoreList != (iList *) NULL && ignoreList->count > 0 &&
        ignoreList->list != (char **) NULL)
    {
        free ((char *) ignoreList->list);
    }

    if (includeList != (iList *) NULL && includeList->count > 0 &&
        includeList->list != (char **) NULL)
    {
        free ((char *) includeList->list);
    }

    if (argvptr != (char *) NULL)
    {
        free ((char *) argvptr);
    }

    if (zoneInfo->zones != (zoneSummary_t *) NULL)
    {
        free ((void *) zoneInfo->zones);
    }
}

/*
 * printDiskInfo
 *
 * Print out the disk information table.
 * Loops through all mounted disks, prints and calculates totals.
 *
 */

static void
#if _proto_stdc
printDiskInfo (di_DiskInfo *diskInfo, int diCount)
#else
printDiskInfo (diskInfo, diCount)
    di_DiskInfo     *diskInfo;
    int             diCount;
#endif
{
    int                 i;
    di_DiskInfo         totals;
    char                lastpool [DI_SPEC_NAME_LEN + 1];
    Size_t              lastpoollen = { 0 };
    int                 inpool = { 0 };
    int                 tempSortType;

    lastpool[0] = '\0';
    memset ((char *) &totals, '\0', sizeof (di_DiskInfo));
    totals.blockSize = 8192;
    strncpy (totals.name, GT("Total"), DI_NAME_LEN);
    totals.printFlag = DI_PRNT_OK;

    if ((flags & DI_F_NO_HEADER) != DI_F_NO_HEADER)
    {
        printTitle ();
    }

    if ((flags & DI_F_TOTAL) == DI_F_TOTAL &&
            (flags & DI_F_NO_HEADER) != DI_F_NO_HEADER)
    {
            /* in order to find the main pool entry,                */
            /* we must have the array sorted by special device name */
        tempSortType = sortType;
        sortType = DI_SORT_SPECIAL;
        sortArray ((char *) diskInfo, sizeof (di_DiskInfo), diCount, diCompare);
        sortType = tempSortType;

        for (i = 0; i < diCount; ++i)
        {
            int ispooled = { 0 };

            if (diskInfo [i].printFlag == DI_PRNT_EXCLUDE ||
                diskInfo [i].printFlag == DI_PRNT_BAD ||
                diskInfo [i].printFlag == DI_PRNT_OUTOFZONE)
            {
                continue;
            }

            if (diskInfo [i].printFlag == DI_PRNT_IGNORE &&
                (flags & DI_F_ALL) != DI_F_ALL)
            {
                continue;
            }

                /* is it a pooled filesystem type? */
            if (strcmp (diskInfo [i].fsType, "zfs") == 0 ||
                strcmp (diskInfo [i].fsType, "advfs") == 0)
            {
                ispooled = 1;
                if (lastpoollen == 0 ||
                    strncmp (lastpool, diskInfo [i].special, lastpoollen) != 0)
                {
                    strncpy (lastpool, diskInfo [i].special, sizeof (lastpool));
                    lastpoollen = strlen (lastpool);
                    inpool = 0;
                }
            }

                /* only total the disks that make sense!                    */
                /* don't add memory filesystems to totals.                  */
                /* only add the main pool to totals for pooled filesystems  */
            if (strcmp (diskInfo [i].fsType, "tmpfs") != 0 &&
                strcmp (diskInfo [i].fsType, "mfs") != 0 &&
                diskInfo [i].isReadOnly == FALSE &&
                inpool == 0)
            {
                addTotals (&diskInfo [i], &totals);
            }

                /* is it a pooled filesystem type? */
            if (ispooled)
            {
                if (strncmp (lastpool, diskInfo [i].special, lastpoollen) == 0)
                {
                    inpool = 1;
                }
            }
        } /* for each entry */
    } /* if the totals are to be printed */

    if (sortType != DI_SORT_NONE)
    {
        sortArray ((char *) diskInfo, sizeof (di_DiskInfo), diCount, diCompare);
    }

    for (i = 0; i < diCount; ++i)
    {
        if (debug > 5)
        {
            printf ("pdi:%s:%s:\n", diskInfo[i].name,
                getPrintFlagText ((int) diskInfo[i].printFlag));
        }
        if (diskInfo [i].printFlag == DI_PRNT_EXCLUDE ||
            diskInfo [i].printFlag == DI_PRNT_BAD ||
            diskInfo [i].printFlag == DI_PRNT_OUTOFZONE)
        {
            continue;
        }

        if (diskInfo [i].printFlag == DI_PRNT_IGNORE &&
            (flags & DI_F_ALL) != DI_F_ALL)
        {
            continue;
        }

        printInfo (&diskInfo [i]);
    }

    if ((flags & DI_F_TOTAL) == DI_F_TOTAL &&
            (flags & DI_F_NO_HEADER) != DI_F_NO_HEADER)
    {
        printInfo (&totals);
    }
}

/*
 * printInfo
 *
 * Print the information for a single partition.  Loop through the
 * format string and print the particular items wanted.
 *
 */

static void
#if _proto_stdc
printInfo (di_DiskInfo *diskInfo)
#else
printInfo (diskInfo)
    di_DiskInfo         *diskInfo;
#endif
{
    _fs_size_t          used;
    _fs_size_t          totAvail;
    char                *ptr;
    int                 valid;
    double              temp;
    int                 idx;
    int                 tidx;

    idx = 0;
    temp = 0.0;  /* gcc compile warning */
    if (dispBlockSize == DI_DISP_HR_2)
    {
        idx = DI_ONE_MEG_SZTAB; /* default */

        ptr = formatString;
        while (*ptr)
        {
            valid = FALSE;

            switch (*ptr)
            {
                case DI_FMT_BTOT:
                {
                    temp = diskInfo->totalBlocks;
                    valid = TRUE;
                    break;
                }

                case DI_FMT_BTOT_AVAIL:
                {
                    temp = diskInfo->totalBlocks -
                            (diskInfo->freeBlocks - diskInfo->availBlocks);
                    valid = TRUE;
                    break;
                }

                case DI_FMT_BUSED:
                {
                    temp = diskInfo->totalBlocks - diskInfo->freeBlocks;
                    valid = TRUE;
                    break;
                }

                case DI_FMT_BCUSED:
                {
                    temp = diskInfo->totalBlocks - diskInfo->availBlocks;
                    valid = TRUE;
                    break;
                }

                case DI_FMT_BFREE:
                {
                    temp = diskInfo->freeBlocks;
                    valid = TRUE;
                    break;
                }

                case DI_FMT_BAVAIL:
                {
                    temp = diskInfo->availBlocks;
                    valid = TRUE;
                    break;
                }
            }

            if (valid)
            {
                temp *= diskInfo->blockSize;
                tidx = findDispSize (temp);
                    /* want largest index */
                if (tidx > idx)
                {
                    idx = tidx;
                }
            }
            ++ptr;
        }
    }

    ptr = formatString;
    while (*ptr)
    {
        valid = TRUE;

        switch (*ptr)
        {
            case DI_FMT_MOUNT:
            {
                printf (DI_MOUNT_FMT, diskInfo->name);
                break;
            }

            case DI_FMT_MOUNT_FULL:
            {
                printf (mountFormat, diskInfo->name);
                break;
            }

            case DI_FMT_BTOT:
            {
                printBlocks (diskInfo->totalBlocks, diskInfo->blockSize, idx);
                break;
            }

            case DI_FMT_BTOT_AVAIL:
            {
                printBlocks (diskInfo->totalBlocks -
                    (diskInfo->freeBlocks - diskInfo->availBlocks),
                    diskInfo->blockSize, idx);
                break;
            }

            case DI_FMT_BUSED:
            {
                printBlocks (diskInfo->totalBlocks - diskInfo->freeBlocks,
                            diskInfo->blockSize, idx);
                break;
            }

            case DI_FMT_BCUSED:
            {
                printBlocks (diskInfo->totalBlocks - diskInfo->availBlocks,
                            diskInfo->blockSize, idx);
                break;
            }

            case DI_FMT_BFREE:
            {
                printBlocks (diskInfo->freeBlocks, diskInfo->blockSize, idx);
                break;
            }

            case DI_FMT_BAVAIL:
            {
                printBlocks (diskInfo->availBlocks, diskInfo->blockSize, idx);
                break;
            }

            case DI_FMT_BPERC_NAVAIL:
            {
                used = diskInfo->totalBlocks - diskInfo->availBlocks;
                totAvail = diskInfo->totalBlocks;
                if (posix_compat == 1)
                {
                    printPerc (used, totAvail, DI_POSIX_PERC_FMT);
                }
                else
                {
                    printPerc (used, totAvail, DI_PERC_FMT);
                }
                break;
            }

            case DI_FMT_BPERC_USED:
            {
                used = diskInfo->totalBlocks - diskInfo->freeBlocks;
                totAvail = diskInfo->totalBlocks;
                if (posix_compat == 1)
                {
                    printPerc (used, totAvail, DI_POSIX_PERC_FMT);
                }
                else
                {
                    printPerc (used, totAvail, DI_PERC_FMT);
                }
                break;
            }

            case DI_FMT_BPERC_BSD:
            {
                used = diskInfo->totalBlocks - diskInfo->freeBlocks;
                totAvail = diskInfo->totalBlocks -
                        (diskInfo->freeBlocks - diskInfo->availBlocks);
                if (posix_compat == 1)
                {
                    printPerc (used, totAvail, DI_POSIX_PERC_FMT);
                }
                else
                {
                    printPerc (used, totAvail, DI_PERC_FMT);
                }
                break;
            }

            case DI_FMT_BPERC_AVAIL:
            {
                _fs_size_t          bfree;
                bfree = diskInfo->availBlocks;
                totAvail = diskInfo->totalBlocks;
                printPerc (bfree, totAvail, DI_PERC_FMT);
                break;
            }

            case DI_FMT_BPERC_FREE:
            {
                _fs_size_t          bfree;
                bfree = diskInfo->freeBlocks;
                totAvail = diskInfo->totalBlocks;
                printPerc (bfree, totAvail, DI_PERC_FMT);
                break;
            }

            case DI_FMT_ITOT:
            {
                printf (inodeFormat, diskInfo->totalInodes);
                break;
            }

            case DI_FMT_IUSED:
            {
                printf (inodeFormat, diskInfo->totalInodes - diskInfo->freeInodes);
                break;
            }

            case DI_FMT_IFREE:
            {
                printf (inodeFormat, diskInfo->freeInodes);
                break;
            }

            case DI_FMT_IPERC:
            {
                used = diskInfo->totalInodes - diskInfo->availInodes;
                totAvail = diskInfo->totalInodes;
                printPerc (used, totAvail, DI_PERC_FMT);
                break;
            }

            case DI_FMT_SPECIAL:
            {
                printf (DI_SPEC_FMT, diskInfo->special);
                break;
            }

            case DI_FMT_SPECIAL_FULL:
            {
                printf (specialFormat, diskInfo->special);
                break;
            }

            case DI_FMT_TYPE:
            {
                printf (DI_FSTYPE_FMT, diskInfo->fsType);
                break;
            }

            case DI_FMT_TYPE_FULL:
            {
                printf (typeFormat, diskInfo->fsType);
                break;
            }

            case DI_FMT_MOUNT_OPTIONS:
            {
                printf (optFormat, diskInfo->options);
                break;
            }

            case DI_FMT_MOUNT_TIME:
            {
#if _lib_mnt_time
                printf (mTimeFormat, diskInfo->mountTime);
#endif
                break;
            }

            default:
            {
                printf ("%c", *ptr);
                valid = FALSE;
                break;
            }
        }

        ++ptr;
        if (*ptr && valid)
        {
            printf (" ");
        }
    }

    printf ("\n");
}

static void
#if _proto_stdc
printBlocks (_fs_size_t blocks, _fs_size_t blockSize, int idx)
#else
printBlocks (blocks, blockSize, idx)
    _fs_size_t          blocks;
    _fs_size_t          blockSize;
    int                 idx;
#endif
{
    double          tdbs;
    double          mult;
    double          temp;
    char            *suffix;
    char            *format;


    suffix = "";
    format = blockFormat;
    tdbs = dispBlockSize;

    if (dispBlockSize == DI_DISP_HR)
    {
        temp = (double) blocks * (double) blockSize;
        idx = findDispSize (temp);
    }

    if (dispBlockSize == DI_DISP_HR ||
        dispBlockSize == DI_DISP_HR_2)
    {
        if (idx == -1)
        {
            tdbs = sizeTable [DI_ONE_MEG].dbs;
        }
        else
        {
            tdbs = sizeTable [idx].dbs;
            format = sizeTable [idx].format;
            suffix = sizeTable [idx].suffix;
        }
    }

    mult = (double) blockSize / tdbs;
    printf (format, (double) blocks * mult, suffix);
}


static int
#if _proto_stdc
findDispSize (double siz)
#else
findDispSize (siz)
    double      siz;
#endif
{
    int         i;

    for (i = 0; i < DI_SIZETAB_SIZE; ++i)
    {
        if (siz >= sizeTable [i].low && siz < sizeTable [i].high)
        {
            return i;
        }
    }

    return -1;
}

/*
 * addTotals
 *
 * Add up the totals for the blocks/inodes
 *
 */

static void
#if _proto_stdc
addTotals (di_DiskInfo *diskInfo, di_DiskInfo *totals)
#else
addTotals (diskInfo, totals)
    di_DiskInfo   *diskInfo;
    di_DiskInfo   *totals;
#endif
{
    double              mult;

    mult = (double) diskInfo->blockSize / (double) totals->blockSize;

    if (debug > 2)
    {
#if _siz_long_long >= 8
        printf ("totals:bs:%lld:mult:%f\n", diskInfo->blockSize, mult);
#else
        printf ("totals:bs:%ld:mult:%f\n", diskInfo->blockSize, mult);
#endif
    }

    totals->totalBlocks += diskInfo->totalBlocks * mult;
    totals->freeBlocks += diskInfo->freeBlocks * mult;
    totals->availBlocks += diskInfo->availBlocks * mult;
    totals->totalInodes += diskInfo->totalInodes;
    totals->freeInodes += diskInfo->freeInodes;
    totals->availInodes += diskInfo->availInodes;
}

/*
 * printTitle
 *
 * Loop through the format string and print the appropriate headings.
 *
 */

static void
#if _proto_stdc
printTitle (void)
#else
printTitle ()
#endif
{
    char                *ptr;
    int                 valid;

    if ((flags & DI_F_DEBUG_HDR) == DI_F_DEBUG_HDR)
    {
        printf (GT("di ver %s Default Format: %s\n"),
                DI_VERSION, DI_DEFAULT_FORMAT);
    }

    ptr = formatString;

    while (*ptr)
    {
        valid = TRUE;

        switch (*ptr)
        {
            case DI_FMT_MOUNT:
            {
                printf (DI_MOUNT_FMT, GT("Mount"));
                break;
            }

            case DI_FMT_MOUNT_FULL:
            {
                if (posix_compat == 1)
                {
                    printf (mountFormat, GT("Mounted On"));
                }
                else
                {
                    printf (mountFormat, GT("Mount"));
                }
                break;
            }

            case DI_FMT_BTOT:
            case DI_FMT_BTOT_AVAIL:
            {
                printf (blockLabelFormat, dispBlockLabel);
                break;
            }

            case DI_FMT_BUSED:
            case DI_FMT_BCUSED:
            {
                printf (blockLabelFormat, GT("Used"));
                break;
            }

            case DI_FMT_BFREE:
            {
                printf (blockLabelFormat, GT("Free"));
                break;
            }

            case DI_FMT_BAVAIL:
            {
                if (posix_compat == 1)
                {
                    printf (blockLabelFormat, GT("Available"));
                }
                else
                {
                    printf (blockLabelFormat, GT("Avail"));
                }
                break;
            }

            case DI_FMT_BPERC_NAVAIL:
            case DI_FMT_BPERC_USED:
            case DI_FMT_BPERC_BSD:
            {
                if (posix_compat == 1)
                {
                    printf (DI_POSIX_PERC_LBL_FMT, GT("Capacity"));
                }
                else
                {
                    printf (DI_PERC_LBL_FMT, GT("%Used"));
                }
                break;
            }

            case DI_FMT_BPERC_AVAIL:
            case DI_FMT_BPERC_FREE:
            {
                printf (DI_PERC_LBL_FMT, GT("%Free"));
                break;
            }

            case DI_FMT_ITOT:
            {
                printf (inodeLabelFormat, GT("Inodes"));
                break;
            }

            case DI_FMT_IUSED:
            {
                printf (inodeLabelFormat, GT("Used"));
                break;
            }

            case DI_FMT_IFREE:
            {
                printf (inodeLabelFormat, GT("Free"));
                break;
            }

            case DI_FMT_IPERC:
            {
                printf (DI_PERC_LBL_FMT, GT("%Used"));
                break;
            }

            case DI_FMT_SPECIAL:
            {
                printf (DI_SPEC_FMT, GT("Filesystem"));
                break;
            }

            case DI_FMT_SPECIAL_FULL:
            {
                printf (specialFormat, GT("Filesystem"));
                break;
            }

            case DI_FMT_TYPE:
            {
                printf (DI_FSTYPE_FMT, GT("fsType"));
                break;
            }

            case DI_FMT_TYPE_FULL:
            {
                printf (typeFormat, GT("fs Type"));
                break;
            }

            case DI_FMT_MOUNT_OPTIONS:
            {
                printf (optFormat, GT("Options"));
                break;
            }

            case DI_FMT_MOUNT_TIME:
            {
#if _lib_mnt_time
                printf (mTimeFormat, GT("Mount Time"));
#endif
                break;
            }

            default:
            {
                printf ("%c", *ptr);
                valid = FALSE;
                break;
            }
        }

        ++ptr;
        if (*ptr && valid)
        {
            printf (" ");
        }
    }

    printf ("\n");
}

/*
 * printPerc
 *
 * Calculate and print a percentage using the values and format passed.
 *
 */

static void
#if _proto_stdc
printPerc (_fs_size_t used, _fs_size_t totAvail, char *format)
#else
printPerc (used, totAvail, format)
    _fs_size_t      used;
    _fs_size_t      totAvail;
    char            *format;
#endif
{
    double      perc;


    if (totAvail > 0L)
    {
        perc = (double) used / (double) totAvail;
        perc *= 100.0;
    }
    else
    {
        perc = 0.0;
    }
    printf (format, perc);
}


static int
#if _proto_stdc
checkFileInfo (di_DiskInfo *diskInfo, int diCount,
        int optidx, int argc, char *argv [])
#else
checkFileInfo (diskInfo, diCount, optidx, argc, argv)
    di_DiskInfo         *diskInfo;
    int                 diCount;
    int                 optidx;
    int                 argc;
    char                *argv [];
#endif
{
    int                 rc;
    int                 i;
    int                 j;
    struct stat         statBuf;


    rc = 0;
        /* turn everything off */
    for (j = 0; j < diCount; ++j)
    {
        if (diskInfo [j].printFlag == DI_PRNT_OK)
        {
            diskInfo [j].printFlag = DI_PRNT_IGNORE;
        }
    }

    for (i = optidx; i < argc; ++i)
    {
        if (stat (argv [i], &statBuf) == 0)
        {
            for (j = 0; j < diCount; ++j)
            {
                if (diskInfo [j].printFlag == DI_PRNT_IGNORE &&
                    diskInfo [j].st_dev != DI_UNKNOWN_DEV &&
                    (__ulong) statBuf.st_dev == diskInfo [j].st_dev)
                {
                    diskInfo [j].printFlag = DI_PRNT_OK;
                    break; /* out of inner for */
                }
            }
        } /* if stat ok */
        else
        {
            fprintf (stderr, "stat: %s ", argv[i]);
            perror ("");
            rc = -1;
        }
    } /* for each file specified on command line */

    return rc;
}

/*
 *  sortArray ()
 *
 *      shellsort!
 *
 */

static void
#if _proto_stdc
sortArray (char *data, Size_t dataSize, int count, DI_SORT_FUNC compareFunc)
#else
sortArray (data, dataSize, count, compareFunc)
    char            *data;
    Size_t          dataSize;
    int             count;
    DI_SORT_FUNC    compareFunc;
#endif
{
    register int    j;
    char            *tempData;
    int             i;
    int             gap;


    if (count <= 1)
    {
        return;
    }

    tempData = (char *) malloc (dataSize);
    if (tempData == (char *) NULL)
    {
        fprintf (stderr, "malloc failed in sortArray().  errno %d\n", errno);
        exit (1);
    }

    gap = 1;
    while (gap < count)
    {
        gap = 3 * gap + 1;
    }

    for (gap /= 3; gap > 0; gap /= 3)
    {
        for (i = gap; i < count; ++i)
        {
            memcpy ((char *) tempData, (char *) &(data [i * dataSize]),
                    dataSize);
            j = i - gap;

            while (j >= 0 &&
                compareFunc (&(data [j * dataSize]), tempData) > 0)
            {
                memcpy ((char *) &(data [(j + gap) * dataSize]),
                        (char *) &(data [j * dataSize]), dataSize);
                j -= gap;
            }

            j += gap;
            if (j != i)
            {
                memcpy ((char *) &(data [j * dataSize]),
                        (char *) tempData, dataSize);
            }
        }
    }

    free ((char *) tempData);
}


static int
#if _proto_stdc
diCompare (void *a, void *b)
#else
diCompare (a, b)
    void        *a;
    void        *b;
#endif
{
    di_DiskInfo *di1;
    di_DiskInfo *di2;
    int         rc;


    di1 = (di_DiskInfo *) a;
    di2 = (di_DiskInfo *) b;

    rc = 0;
    switch (sortType)
    {
        case DI_SORT_NONE:
        {
            break;
        }

        case DI_SORT_NAME:
        {
            rc = strcmp (di1->name, di2->name);
            break;
        }

        case DI_SORT_SPECIAL:
        {
            rc = strcmp (di1->special, di2->special);
            break;
        }

        case DI_SORT_AVAIL:
        {
            rc = (int) ((di1->availBlocks * di1->blockSize) -
                    (di2->availBlocks * di2->blockSize));
            break;
        }
    } /* switch on sort type */

    rc *= sortOrder;
    return rc;
}


/*
 * getDiskStatInfo
 *
 * gets the disk device number for each entry.
 *
 */

static void
#if _proto_stdc
getDiskStatInfo (di_DiskInfo *diskInfo, int diCount)
#else
getDiskStatInfo (diskInfo, diCount)
    di_DiskInfo             *diskInfo;
    int                     diCount;
#endif
{
    int         i;
    struct stat statBuf;

    for (i = 0; i < diCount; ++i)
    {
            /* don't try to stat devices that are not accessible */
        if (diskInfo [i].printFlag == DI_PRNT_EXCLUDE ||
            diskInfo [i].printFlag == DI_PRNT_BAD ||
            diskInfo [i].printFlag == DI_PRNT_OUTOFZONE)
        {
            continue;
        }

        diskInfo [i].st_dev = (__ulong) DI_UNKNOWN_DEV;

        if (stat (diskInfo [i].name, &statBuf) == 0)
        {
            diskInfo [i].st_dev = (__ulong) statBuf.st_dev;
            if (debug > 2)
            {
                printf ("dev: %s: %ld\n", diskInfo [i].name,
                    diskInfo [i].st_dev);
            }
        }
        else
        {
            fprintf (stderr, "stat: %s ", diskInfo [i].name);
            perror ("");
        }
    }
}

/*
 * getDiskSpecialInfo
 *
 * gets the disk device number for each entry.
 *
 */

static void
#if _proto_stdc
getDiskSpecialInfo (di_DiskInfo *diskInfo, int diCount)
#else
getDiskSpecialInfo (diskInfo, diCount)
    di_DiskInfo         *diskInfo;
    int                 diCount;
#endif
{
    int         i;
    struct stat statBuf;

    for (i = 0; i < diCount; ++i)
    {
           /* check for initial slash; otherwise we can pick up normal files */
        if (*diskInfo [i].special == '/' &&
            stat (diskInfo [i].special, &statBuf) == 0)
        {
            diskInfo [i].sp_dev = (__ulong) statBuf.st_dev;
            diskInfo [i].sp_rdev = (__ulong) statBuf.st_rdev;
            if (debug > 2)
            {
                printf ("special dev: %s: %ld rdev: %ld\n",
                        diskInfo [i].special, diskInfo [i].sp_dev,
                        diskInfo [i].sp_rdev);
            }
        }
        else
        {
            diskInfo [i].sp_dev = 0;
            diskInfo [i].sp_rdev = 0;
        }
    }
}

/*
 * checkDiskInfo
 *
 * checks the disk information returned for various return values.
 *
 */

static void
#if _proto_stdc
checkDiskInfo (di_DiskInfo *diskInfo, iList *includeList, int diCount)
#else
checkDiskInfo (diskInfo, includeList, diCount)
    di_DiskInfo         *diskInfo;
    iList               *includeList;
    int                 diCount;
#endif
{
    int           i;
    int           j;
    int           dupCount;
    int           len;
    int           maxMountString;
    int           maxSpecialString;
    int           maxTypeString;
    int           maxOptString;
    int           maxMntTimeString;
    _fs_size_t    temp;
    __ulong       sp_dev;
    __ulong       sp_rdev;


    maxMountString = (flags & DI_F_NO_HEADER) == DI_F_NO_HEADER ? 0 : 5;
    maxSpecialString = (flags & DI_F_NO_HEADER) == DI_F_NO_HEADER ? 0 : 10;
    maxTypeString = (flags & DI_F_NO_HEADER) == DI_F_NO_HEADER ? 0 : 7;
    maxOptString = (flags & DI_F_NO_HEADER) == DI_F_NO_HEADER ? 0 : 7;
    maxMntTimeString = (flags & DI_F_NO_HEADER) == DI_F_NO_HEADER ? 0 : 26;

    for (i = 0; i < diCount; ++i)
    {
        if (diskInfo [i].printFlag == DI_PRNT_EXCLUDE ||
            diskInfo [i].printFlag == DI_PRNT_BAD ||
            diskInfo [i].printFlag == DI_PRNT_OUTOFZONE)
        {
            if (debug > 2)
            {
                printf ("chk: skipping(%s):%s\n",
                    getPrintFlagText ((int) diskInfo[i].printFlag),
                    diskInfo [i].name);
            }
            continue;
        }

            /* Solaris reports a cdrom as having no free blocks,   */
            /* no available.  Their df doesn't always work right!  */
            /* -1 is returned.                                     */
        if (debug > 2)
        {
#if _siz_long_long >= 8
            printf ("chk: %s free: %llu\n", diskInfo [i].name,
                diskInfo [i].freeBlocks);
#else
            printf ("chk: %s free: %lu\n", diskInfo [i].name,
                diskInfo [i].freeBlocks);
#endif
        }
        if ((_s_fs_size_t) diskInfo [i].freeBlocks < 0L)
        {
            diskInfo [i].freeBlocks = 0L;
        }
        if ((_s_fs_size_t) diskInfo [i].availBlocks < 0L)
        {
            diskInfo [i].availBlocks = 0L;
        }

        temp = (_fs_size_t) ~ 0;
        if (diskInfo [i].totalInodes == temp)
        {
            diskInfo [i].totalInodes = 0;
            diskInfo [i].freeInodes = 0;
            diskInfo [i].availInodes = 0;
        }

        if (diskInfo [i].printFlag == DI_PRNT_OK)
        {
            if (debug > 2)
            {
#if _siz_long_long >= 8
                printf ("chk: %s total: %lld\n", diskInfo [i].name,
                        diskInfo [i].totalBlocks);
#else
                printf ("chk: %s total: %ld\n", diskInfo [i].name,
                        diskInfo [i].totalBlocks);
#endif
            }
            if (diskInfo [i].totalBlocks <= 0L)
            {
                diskInfo [i].printFlag = DI_PRNT_IGNORE;
                if (debug > 2)
                {
                    printf ("chk: ignore: totalBlocks <= 0: %s\n",
                            diskInfo [i].name);
                }
            }
        }

        /* make sure anything in the include list didn't get turned off */
        checkIncludeList (&diskInfo [i], includeList);
    } /* for all disks */

    if ((flags & DI_F_INCLUDE_LOOPBACK) != DI_F_INCLUDE_LOOPBACK)
    {
            /* this loop sets duplicate entries to be ignored. */
        for (i = 0; i < diCount; ++i)
        {
            if (diskInfo [i].printFlag == DI_PRNT_IGNORE ||
                diskInfo [i].printFlag == DI_PRNT_EXCLUDE ||
                diskInfo [i].printFlag == DI_PRNT_BAD ||
                diskInfo [i].printFlag == DI_PRNT_OUTOFZONE)
            {
                if (debug > 2)
                {
                    printf ("chk: skipping(%s):%s\n",
                        getPrintFlagText ((int) diskInfo[i].printFlag),
                        diskInfo [i].name);
                }
                continue;
            }

                /* don't need to bother checking real partitions */
                /* don't bother if already ignored               */
            if (diskInfo [i].sp_rdev != 0 &&
                diskInfo [i].printFlag == DI_PRNT_OK)
            {
                sp_dev = diskInfo [i].sp_dev;
                sp_rdev = diskInfo [i].sp_rdev;
                dupCount = 0;

                for (j = 0; j < diCount; ++j)
                {
                    if (diskInfo [j].sp_dev == sp_rdev)
                    {
                        ++dupCount;
                    }
                }

                if (debug > 2)
                {
                    printf ("dup: chk: i: %s dev: %ld rdev: %ld dup: %d\n",
                        diskInfo [i].name, sp_dev, sp_rdev, dupCount);
                }

                for (j = 0; dupCount > 0 && j < diCount; ++j)
                {
                        /* don't reset flags! */
                    if (diskInfo [j].sp_rdev == 0 &&
                        diskInfo [j].sp_dev == sp_rdev &&
                        diskInfo [j].printFlag == DI_PRNT_OK)
                    {
                        diskInfo [j].printFlag = DI_PRNT_IGNORE;
                        if (debug > 2)
                        {
                            printf ("chk: ignore: duplicate: %s of %s\n",
                                    diskInfo [j].name, diskInfo [i].name);
                            printf ("dup: ign: j: rdev: %ld dev: %ld\n",
                                    diskInfo [j].sp_dev, diskInfo [j].sp_rdev);
                        }
                    }
                } /* duplicate check for each disk */
            } /* if this is a printable disk */
            else
            {
                if (debug > 2)
                {
                    printf ("chk: dup: not checked: %s prnt: %d dev: %ld rdev: %ld\n",
                            diskInfo [i].name, diskInfo [i].printFlag,
                            diskInfo [i].sp_dev, diskInfo [i].sp_rdev);
                }
            }
        } /* for each disk */
    } /* if the duplicate loopback mounts are to be excluded */

    if (posix_compat == 1)
    {
        /* Mounted On */
        len = strlen (GT("Mounted On"));
        maxMountString = maxMountString < len ? len : maxMountString;
        len = strlen (dispBlockLabel);
        width = width < len ? len : width;
    }

        /* this loop gets the max string lengths */
    for (i = 0; i < diCount; ++i)
    {
        if (diskInfo [i].printFlag == DI_PRNT_OK ||
            (diskInfo [i].printFlag == DI_PRNT_IGNORE &&
             (flags & DI_F_ALL) == DI_F_ALL))
        {
            len = strlen (diskInfo [i].name);
            if (len > maxMountString)
            {
                maxMountString = len;
            }

            len = strlen (diskInfo [i].special);
            if (len > maxSpecialString)
            {
                maxSpecialString = len;
            }

            len = strlen (diskInfo [i].fsType);
            if (len > maxTypeString)
            {
                maxTypeString = len;
            }

            len = strlen (diskInfo [i].options);
            if (len > maxOptString)
            {
                maxOptString = len;
            }

            len = strlen (diskInfo [i].mountTime);
            if (len > maxMntTimeString)
            {
                maxMntTimeString = len;
            }
        } /* if we are printing this item */
    } /* for all disks */

    Snprintf (SPF(mountFormat, sizeof (mountFormat), "%%-%d.%ds"),
              maxMountString, maxMountString);
    Snprintf (SPF(specialFormat, sizeof (specialFormat), "%%-%d.%ds"),
              maxSpecialString, maxSpecialString);
    Snprintf (SPF(typeFormat, sizeof (typeFormat), "%%-%d.%ds"),
              maxTypeString, maxTypeString);
    Snprintf (SPF(optFormat, sizeof (optFormat), "%%-%d.%ds"),
              maxOptString, maxOptString);
    Snprintf (SPF(mTimeFormat, sizeof (mTimeFormat), "%%-%d.%ds"),
              maxMntTimeString, maxMntTimeString);

    if (dispBlockSize == DI_DISP_HR ||
        dispBlockSize == DI_DISP_HR_2)
    {
        --width;
    }

    if (dispBlockSize != DI_DISP_HR &&
        dispBlockSize != DI_DISP_HR_2 &&
        (dispBlockSize > 0 && dispBlockSize <= DI_VAL_1024))
    {
        Snprintf (SPF(blockFormat, sizeof (blockFormat), "%%%d.0f%%s"), width);
    }
    else
    {
        Snprintf (SPF(blockFormatNR, sizeof (blockFormatNR), "%%%d.0f%%s"),
                  width);
        Snprintf (SPF(blockFormat, sizeof (blockFormat), "%%%d.1f%%s"), width);
    }

    if (dispBlockSize == DI_DISP_HR ||
        dispBlockSize == DI_DISP_HR_2)
    {
        ++width;
    }

    Snprintf (SPF(blockLabelFormat, sizeof (blockLabelFormat), "%%%ds"),
              width);
#if _siz_long_long >= 8
    Snprintf (SPF(inodeFormat, sizeof (inodeFormat), "%%%dllu"), inodeWidth);
#else
    Snprintf (SPF(inodeFormat, sizeof (inodeFormat), "%%%dlu"), inodeWidth);
#endif
    Snprintf (SPF(inodeLabelFormat, sizeof (inodeLabelFormat), "%%%ds"),
              inodeWidth);
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
#if _proto_stdc
preCheckDiskInfo (di_DiskInfo *diskInfo, iList *ignoreList,
        iList *includeList, int diCount, zoneInfo_t *zoneInfo)
#else
preCheckDiskInfo (diskInfo, ignoreList, includeList, diCount, zoneInfo)
    di_DiskInfo         *diskInfo;
    iList               *ignoreList;
    iList               *includeList;
    int                 diCount;
    zoneInfo_t          *zoneInfo;
#endif
{
    int           i;

    for (i = 0; i < diCount; ++i)
    {
        if (debug > 4)
        {
            printf ("## prechk:%s:\n", diskInfo[i].name);
        }
        checkZone (&diskInfo [i], zoneInfo,
            ((flags & DI_F_ALL) == DI_F_ALL));

        if (diskInfo [i].printFlag == DI_PRNT_OK)
        {
                /* don't bother w/this check is all flag is set. */
            if ((flags & DI_F_ALL) != DI_F_ALL)
            {
                di_testRemoteDisk (&diskInfo [i]);

                if (diskInfo [i].isLocal == FALSE &&
                        (flags & DI_F_LOCAL_ONLY) == DI_F_LOCAL_ONLY)
                {
                    diskInfo [i].printFlag = DI_PRNT_IGNORE;
                    if (debug > 2)
                    {
                        printf ("prechk: ignore: remote disk; local flag set: %s\n",
                            diskInfo [i].name);
                    }
                }
            }
        }

        if (diskInfo [i].printFlag == DI_PRNT_OK ||
            diskInfo [i].printFlag == DI_PRNT_IGNORE)
        {
            /* do do these checks to override the all flag */
            checkIgnoreList (&diskInfo [i], ignoreList);
            checkIncludeList (&diskInfo [i], includeList);
        }
    } /* for all disks */
}

/*
 * usage
 *
 */

static void
#if _proto_stdc
usage (void)
#else
usage ()
#endif
{
    printf (GT("di ver %s    Default Format: %s\n"), DI_VERSION, DI_DEFAULT_FORMAT);
            /*  12345678901234567890123456789012345678901234567890123456789012345678901234567890 */
    printf (GT("Usage: di [-ant] [-d display-size] [-f format] [-x exclude-fstyp-list]\n"));
    printf (GT("       [-I include-fstyp-list] [file [...]]\n"));
    printf (GT("   -a   : print all mounted devices\n"));
    printf (GT("   -d x : size to print blocks in (512 - POSIX, k - kbytes,\n"));
    printf (GT("          m - megabytes, g - gigabytes, t - terabytes, h - human readable).\n"));
    printf (GT("   -f x : use format string <x>\n"));
    printf (GT("   -I x : include only file system types in <x>\n"));
    printf (GT("   -x x : exclude file system types in <x>\n"));
    printf (GT("   -l   : display local filesystems only\n"));
    printf (GT("   -n   : don't print header\n"));
    printf (GT("   -t   : print totals\n"));
    printf (GT(" Format string values:\n"));
    printf (GT("    m - mount point                     M - mount point, full length\n"));
    printf (GT("    b - total kbytes                    B - kbytes available for use\n"));
    printf (GT("    u - used kbytes                     c - calculated kbytes in use\n"));
    printf (GT("    f - kbytes free                     v - kbytes available\n"));
    printf (GT("    p - percentage not avail. for use   1 - percentage used\n"));
    printf (GT("    2 - percentage of user-available space in use.\n"));
    printf (GT("    i - total file slots (i-nodes)      U - used file slots\n"));
    printf (GT("    F - free file slots                 P - percentage file slots used\n"));
    printf (GT("    s - filesystem name                 S - filesystem name, full length\n"));
    printf (GT("    t - disk partition type             T - partition type, full length\n"));
    printf (GT("See manual page for more options.\n"));
}


static void
#if _proto_stdc
processArgs (int argc, char *argv [],
             iList *ignoreList,
             iList *includeList,
             char *dbsstr,
             char *zoneDisplay)
#else
processArgs (argc, argv, ignoreList, includeList, dbsstr, zoneDisplay)
    int         argc;
    char        *argv [];
    iList       *ignoreList;
    iList       *includeList;
    char        *dbsstr;
    char        *zoneDisplay;
#endif
{
    int         ch;
    int         hasdashk;


    hasdashk = 0;
    while ((ch = getopt (argc, argv,
        "Aab:B:d:D:f:F:ghHi:I:klLmnPs:tvw:W:x:X:z:Z")) != -1)
    {
        switch (ch)
        {
            case 'A':   /* for debugging */
            {
                formatString = DI_ALL_FORMAT;
                break;
            }

            case 'a':
            {
                flags |= DI_F_ALL;
                strcpy (zoneDisplay, "all");
                break;
            }

            case 'b':
            case 'B':
            {
                if (isdigit ((int) (*optarg)))
                {
                    baseDispSize = atof (optarg);
                }
                else if (*optarg == 'k')
                {
                    baseDispSize = DI_VAL_1024;
                }
                else if (*optarg == 'd')
                {
                    baseDispSize = DI_VAL_1000;
                }
                break;
            }

            case 'd':
            {
                strncpy (dbsstr, optarg, sizeof (dbsstr));
                break;
            }

                /* for debugging; can be replaced */
            case 'D':
            {
                debug = atoi (optarg);
                di_lib_debug = debug;
                break;
            }

            case 'f':
            {
                formatString = optarg;
                break;
            }

            case 'g':
            {
                strncpy (dbsstr, "g", sizeof (dbsstr));
                break;
            }

            case 'h':
            {
                strncpy (dbsstr, "h", sizeof (dbsstr));
                break;
            }

            case 'H':
            {
                strncpy (dbsstr, "H", sizeof (dbsstr));
                break;
            }

            case 'm':
            {
                strncpy (dbsstr, "m", sizeof (dbsstr));
                break;
            }

            case 'i':  /* backwards compatibility */
            case 'x':  /* preferred */
            {
                parseList (ignoreList, optarg);
                break;
            }

            case 'F':  /* compatibility w/other df */
            case 'I':
            {
                parseList (includeList, optarg);
                break;
            }

            case 'k':
            {
                strncpy (dbsstr, "k", sizeof (dbsstr));
                hasdashk = 1;
                break;
            }

            case 'l':
            {
                flags |= DI_F_LOCAL_ONLY;
                break;
            }

            case 'L':
            {
                flags |= DI_F_INCLUDE_LOOPBACK;
                break;
            }

            case 'n':
            {
                flags |= DI_F_NO_HEADER;
                break;
            }

            case 'P':
            {
                if (hasdashk == 0) /* don't override -k option */
                {
                    strncpy (dbsstr, "512", sizeof (dbsstr));
                }
                formatString = DI_POSIX_FORMAT;
                posix_compat = 1;
                break;
            }

            case 's':
            {
                while (*optarg)
                {
                    switch (*optarg)
                    {
                        case 's':
                        {
                            sortType = DI_SORT_SPECIAL;
                            break;
                        }

                        case 'n':
                        {
                            sortType = DI_SORT_NONE;
                            break;
                        }

                        case 'a':
                        {
                            sortType = DI_SORT_AVAIL;
                            break;
                        }

                        case 'r':
                        {
                            sortOrder = DI_SORT_DESCENDING;
                            break;
                        }
                    }
                    ++optarg;
                }
                break;
            }

            case 't':
            {
                flags |= DI_F_TOTAL;
                break;
            }

            case 'v':
            {
                break;
            }

            case 'w':
            {
                width = atoi (optarg);
                break;
            }

            case 'W':
            {
                inodeWidth = atoi (optarg);
                break;
            }

            case 'X':
            {
                debug = atoi (optarg);
                di_lib_debug = debug;
                flags |= DI_F_DEBUG_HDR | DI_F_TOTAL;
                flags &= ~ DI_F_NO_HEADER;
                width = 10;
                inodeWidth = 10;
                break;
            }

            case 'z':
            {
                strcpy (zoneDisplay, optarg);
                break;
            }

            case 'Z':
            {
                strcpy (zoneDisplay, "all");
                break;
            }

            case '?':
            {
                usage ();
                exit (1);
            }
        }
    }
}

static void
#if _proto_stdc
parseList (iList *list, char *str)
#else
parseList (list, str)
    iList       *list;
    char        *str;
#endif
{
    char        *dstr;
    char        *ptr;
    char        *lptr;
    int         count;
    int         ocount;
    int         ncount;
    int         i;
    int         len;

    i = strlen (str);
    dstr = (char *) malloc ((Size_t) i + 1);
    if (dstr == (char *) NULL)
    {
        fprintf (stderr, "malloc failed in parseList() (1).  errno %d\n", errno);
        exit (1);
    }
    memcpy (dstr, str, (Size_t) i);
    dstr [i] = '\0';

    ptr = strtok (dstr, DI_LIST_SEP);
    count = 0;
    while (ptr != (char *) NULL)
    {
        ++count;
        ptr = strtok ((char *) NULL, DI_LIST_SEP);
    }

    ocount = list->count;
    list->count += count;
    ncount = list->count;
    list->list = (char **) Realloc ((char *) list->list,
            list->count * sizeof (char *));
    if (list->list == (char **) NULL)
    {
        fprintf (stderr, "malloc failed in parseList() (2).  errno %d\n", errno);
        exit (1);
    }

    ptr = dstr;
    for (i = ocount; i < ncount; ++i)
    {
        len = strlen (ptr);
        lptr = (char *) malloc ((Size_t) len + 1);
        if (lptr == (char *) NULL)
        {
            fprintf (stderr, "malloc failed in parseList() (3).  errno %d\n",
                    errno);
            exit (1);
        }
        strncpy (lptr, ptr, (Size_t) len);
        lptr[len] = '\0';
        list->list [i] = lptr;
        ptr += len + 1;
    }

    free ((char *) dstr);
}


static void
#if _proto_stdc
checkIgnoreList (di_DiskInfo *diskInfo, iList *ignoreList)
#else
checkIgnoreList (diskInfo, ignoreList)
    di_DiskInfo     *diskInfo;
    iList           *ignoreList;
#endif
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
            if (strcmp (ptr, diskInfo->fsType) == 0)
            {
                diskInfo->printFlag = DI_PRNT_EXCLUDE;
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
#if _proto_stdc
checkIncludeList (di_DiskInfo *diskInfo, iList *includeList)
#else
checkIncludeList (diskInfo, includeList)
    di_DiskInfo     *diskInfo;
    iList           *includeList;
#endif
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

            if (strcmp (ptr, diskInfo->fsType) == 0)
            {
                diskInfo->printFlag = DI_PRNT_OK;
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
                if (debug > 2)
                {
                    printf ("chkinc:!include:fstype %s no match: %s\n", ptr,
                            diskInfo->name);
                }
            }
        }
    } /* if an include list was specified */
}

static void
# if _proto_stdc
checkZone (di_DiskInfo *diskInfo, zoneInfo_t *zoneInfo, int allFlag)
# else
checkZone (diskInfo, zoneInfo, allFlag)
    di_DiskInfo *diskInfo;
    zoneInfo_t  *zoneInfo;
    int         allFlag;
# endif
{
    int         i;
    int         idx = { -1 };

#if _lib_zone_list && _lib_getzoneid && _lib_zone_getattr
    if (strcmp (zoneInfo->zoneDisplay, "all") == 0 &&
        zoneInfo->uid == 0)
    {
        return;
    }

    for (i = 0; i < zoneInfo->zoneCount; ++i)
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
        strcmp (diskInfo->fsType, "lofs") == 0)
    {
        if (debug > 4)
        {
            printf (" checkZone:non-global/lofs:ignore:\n");
        }

        diskInfo->printFlag = DI_PRNT_IGNORE;
    }
#endif

    return;
}


static void
#if _proto_stdc
setDispBlockSize (char *ptr)
#else
setDispBlockSize (ptr)
    char    *ptr;
#endif
{
    int         len;
    double      val;
    char        *tptr;

    if (ptr == (char *) NULL ||
        *ptr == '\0')
    {
        return;
    }

    if (isdigit ((int) (*ptr)))
    {
        val = (_fs_size_t) atof (ptr);
    }
    else
    {
        val = 1.0;
    }

    tptr = ptr;
    len = strlen (ptr);
    tptr += len;
    --tptr;
    if (! isdigit ((int) *tptr))
    {
        int             idx;

        idx = -1;
        switch (*tptr)
        {
            case 'k':
            case 'K':
            {
                idx = DI_ONE_K;
                break;
            }

            case 'm':
            case 'M':
            {
                idx = DI_ONE_MEG;
                break;
            }

            case 'g':
            case 'G':
            {
                idx = DI_ONE_GIG;
                break;
            }

            case 't':
            case 'T':
            {
                idx = DI_ONE_TERA;
                break;
            }

            case 'p':
            case 'P':
            {
                idx = DI_ONE_PETA;
                break;
            }

            case 'e':
            case 'E':
            {
                idx = DI_ONE_EXA;
                break;
            }

            case 'z':
            case 'Z':
            {
                idx = DI_ONE_ZETTA;
                break;
            }

            case 'y':
            case 'Y':
            {
                idx = DI_ONE_YOTTA;
                break;
            }

            case 'h':
            {
                val = DI_DISP_HR;
                strncpy (dispBlockLabel, GT("Size"),
                    sizeof (dispBlockLabel));
                break;
            }

            case 'H':
            {
                val = DI_DISP_HR_2;
                strncpy (dispBlockLabel, GT("Size"),
                    sizeof (dispBlockLabel));
                break;
            }

            default:
            {
                if (strncmp (ptr, "HUMAN", 5) == 0)
                {
                    val = DI_DISP_HR;
                }
                else
                {
                    /* some unknown string value */
                    val = dispBlockSize;
                }
                break;
            }
        }

        if (idx >= 0)
        {
            if (val == 1.0)
            {
                strncpy (dispBlockLabel, GT(dispTable [idx].disp),
                    sizeof (dispBlockLabel));
            }
            else
            {
                Snprintf (SPF (dispBlockLabel, sizeof (dispBlockLabel),
                    "%.0f %s"), val, GT(dispTable [idx].disp));
            }
            val *= dispTable [idx].size;
        } /* known size multiplier */
    }
    else
    {
        int         i;
        int         ok;

        ok = 0;
        for (i = 0; i < DI_DISPTAB_SIZE; ++i)
        {
            if (val == dispTable [i].size)
            {
                strncpy (dispBlockLabel, GT(dispTable [i].disp),
                     sizeof (dispBlockLabel));
                ok = 1;
                break;
            }
        }

        if (ok == 0)
        {
            Snprintf (SPF (dispBlockLabel, sizeof (dispBlockLabel),
                "%.0fb"), val);
        }
    }  /* some oddball block size */

    if (posix_compat == 1 && val == 512)
    {
        strncpy (dispBlockLabel, GT ("512-blocks"),
            sizeof (dispBlockLabel));
    }
    if (posix_compat == 1 && val == 1024)
    {
        strncpy (dispBlockLabel, GT ("1024-blocks"),
            sizeof (dispBlockLabel));
    }

    dispBlockSize = val;
}

/* for debugging */
static char *
#if _proto_stdc
getPrintFlagText (int pf)
#else
getPrintFlagText (pf)
    int pf;
#endif
{
    return pf == DI_PRNT_OK ? "ok" :
        pf == DI_PRNT_BAD ? "bad" :
        pf == DI_PRNT_IGNORE ? "ignore" :
        pf == DI_PRNT_EXCLUDE ? "exclude" :
        pf == DI_PRNT_OUTOFZONE ? "outofzone" : "unknown";
}


#if ! _mth_fmod && ! _lib_fmod

static double
#if _proto_stdc
fmod (double a, double b)
#else
fmod (a, b)
    double a;
    double b;
#endif
{
    double temp;
    temp = a - (int) (a/b) * b;
    return temp;
}

#endif
