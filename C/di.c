/*
$Id$
$Source$
Copyright 1994-2007 Brad Lanam, Walnut Creek, CA
*/

/*
 * di.c
 *
 *   Copyright 1994-2007 Brad Lanam,  Walnut Creek, CA
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
 *  System V.4 `/usr/bin/df -v` Has format: msbuf1 (w/-P option: 512 byte blocks)
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

#define DI_SORT_MAX             10
#define DI_SORT_NONE            'n'
#define DI_SORT_MOUNT           'm'
#define DI_SORT_SPECIAL         's'
#define DI_SORT_AVAIL           'a'
#define DI_SORT_REVERSE         'r'
#define DI_SORT_TYPE            't'
#define DI_SORT_ASCENDING       1
#define DI_SORT_DESCENDING      -1

#define DI_UNKNOWN_DEV          -1L
#define DI_LIST_SEP             ","
#define DI_ARGV_SEP             " 	"  /* space, tab */
#define DI_MAX_ARGV             50

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

typedef struct
{
    int		        count;
    char            **list;
} iList_t;

typedef struct
{
    double          low;
    double          high;
    double          dbs;        /* display block size */
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
    char        name [ZONENAME_MAX];
    char        rootpath [MAXPATHLEN];
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

typedef struct {
    char         *formatString;
    char         mountFormat [20];
    char         specialFormat [20];
    char         typeFormat [20];
    char         optFormat [20];
    char         mTimeFormat [20];
    int          width;
    int          inodeWidth;
    char         blockFormat [40];
    char         blockFormatNR [40];   /* no radix */
    char         blockLabelFormat [40];
    char         inodeFormat [40];
    char         inodeLabelFormat [40];
    double       dispBlockSize;
    char         dispBlockLabel [30];
    double       baseDispSize;
    __ulong      flags;
    char         sortType [DI_SORT_MAX + 1];
    int          sortOrder;
    int          posix_compat;
} diOptions_t;

typedef struct {
    diOptions_t     options;
    diDiskInfo_t    *diskInfo;
    int             count;
    iList_t         ignoreList;
    iList_t         includeList;
    zoneInfo_t      zoneInfo;
} diData_t;

static int          debug = { 0 };

static sizeTable_t sizeTable [] =
{
    { 0, 0, 1, (char *) NULL, " " },
    { 0, 0, 0, (char *) NULL, "k" },
#define DI_ONE_MEG_SZTAB   2
    { 0, 0, 0, (char *) NULL, "M" },
    { 0, 0, 0, (char *) NULL, "G" },
    { 0, 0, 0, (char *) NULL, "T" },
    { 0, 0, 0, (char *) NULL, "P" },
    { 0, 0, 0, (char *) NULL, "E" },
    { 0, 0, 0, (char *) NULL, "Z" },
    { 0, 0, 0, (char *) NULL, "Y" }
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

typedef int (*DI_SORT_FUNC) _((diOptions_t *, void *, void *));

static void addTotals           _((diDiskInfo_t *, diDiskInfo_t *));
static void checkDiskInfo       _((diData_t *));
static int  checkFileInfo       _((diData_t *, int, int, char *[]));
static void checkIgnoreList     _((diDiskInfo_t *, iList_t *));
static void checkIncludeList    _((diDiskInfo_t *, iList_t *));
static void checkZone           _((diDiskInfo_t *, zoneInfo_t *, int));
static void cleanup             _((diData_t *, char *));
static int  diCompare           _((diOptions_t *, void *, void *));
static int  findDispSize        _((double));
static void getDiskSpecialInfo  _((diData_t *));
static void getDiskStatInfo     _((diData_t *));
static char *getPrintFlagText   _((int));
static void parseList           _((iList_t *, char *));
static void preCheckDiskInfo    _((diData_t *));
static void printDiskInfo       _((diData_t *));
static void printInfo           _((diDiskInfo_t *, diOptions_t *));
static void printBlocks          _((diOptions_t *, _fs_size_t, _fs_size_t, int));
static void printTitle          _((diOptions_t *));
static void printPerc           _((_fs_size_t, _fs_size_t, char *));
static void processArgs         _((int, char *[], diData_t *, char *));
static void setDispBlockSize    _((char *, diOptions_t *));
static void sortArray           _((diOptions_t *, char *, Size_t, int, DI_SORT_FUNC));
static void usage               _((void));
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
    diData_t            diData;
    diOptions_t         *diopts;
    int                 i;
    char                *ptr;
    char                *argvptr;
    char                dbsstr [30];

        /* initialization */
    diData.count = 0;

    diData.diskInfo = (diDiskInfo_t *) NULL;

    diData.ignoreList.count = 0;
    diData.ignoreList.list = (char **) NULL;

    diData.includeList.count = 0;
    diData.includeList.list = (char **) NULL;

    diData.zoneInfo.uid = geteuid ();
    diData.zoneInfo.zoneDisplay [0] = '\0';
    diData.zoneInfo.zoneCount = 0;
    diData.zoneInfo.zones = (zoneSummary_t *) NULL;

        /* options defaults */
    diopts = &diData.options;
    diopts->formatString = DI_DEFAULT_FORMAT;
    diopts->dispBlockSize = DI_VAL_1024 * DI_VAL_1024;
    diopts->width = 8;
    diopts->inodeWidth = 7;
    diopts->flags = 0;
/* Linux users may want as the default: */
/*    diopts->flags = { DI_F_INCLUDE_LOOPBACK }; */
    strcpy (diopts->sortType, "m"); /* default - sort by mount point */
    diopts->sortOrder = DI_SORT_ASCENDING;
    diopts->posix_compat = 0;
    diopts->baseDispSize = DI_VAL_1024;

    *dbsstr = '\0';

#if _lib_setlocale && defined (LC_ALL)
    ptr = setlocale (LC_ALL, "");
#endif
#if _enable_nls
    ptr = bindtextdomain (PROG, DI_LOCALE_DIR);
    ptr = textdomain (PROG);
#endif

    ptr = argv [0] + strlen (argv [0]) - 2;
    if (memcmp (ptr, MPROG, 2) == 0)
    {
        diopts->formatString = DI_DEF_MOUNT_FORMAT;
    }
    else    /* don't use DIFMT env var if running mi. */
    {
        if ((ptr = getenv ("DIFMT")) != (char *) NULL)
        {
            diopts->formatString = ptr;
        }
    }

        /* gnu df */
    if ((ptr = getenv ("POSIXLY_CORRECT")) != (char *) NULL)
    {
        strncpy (dbsstr, "512", sizeof (dbsstr));
        diopts->formatString = DI_POSIX_FORMAT;
        diopts->posix_compat = 1;
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
            processArgs (nargc, nargv, &diData, dbsstr);
            optind = ooptind;     /* reset so command line can be parsed */
            optarg = ooptarg;
        }
    }

    processArgs (argc, argv, &diData, dbsstr);

#if _lib_zone_list && _lib_getzoneid && _lib_zone_getattr
    {
        zoneid_t        *zids = (zoneid_t *) NULL;
        zoneInfo_t      *zi;

        zi = &diData.zoneInfo;
        zi->myzoneid = getzoneid ();

        if (zone_list (zids, &zi->zoneCount) == 0)
        {
            if (zi->zoneCount > 0)
            {
                zids = malloc (sizeof (zoneid_t) * zi->zoneCount);
                if (zids == (zoneid_t *) NULL)
                {
                    fprintf (stderr, "malloc failed in main() (1).  errno %d\n", errno);
                    exit (1);
                }
                zone_list (zids, &zi->zoneCount);
                zi->zones = malloc (sizeof (zoneSummary_t) *
                        zi->zoneCount);
                if (zi->zones == (zoneSummary_t *) NULL)
                {
                    fprintf (stderr, "malloc failed in main() (2).  errno %d\n", errno);
                    exit (1);
                }
            }
        }

        zi->globalIdx = 0;
        for (i = 0; i < zi->zoneCount; ++i)
        {
            int     len;

            zi->zones[i].zoneid = zids[i];
            len = zone_getattr (zids[i], ZONE_ATTR_ROOT,
                    zi->zones[i].rootpath, MAXPATHLEN);
            if (len >= 0)
            {
                zi->zones[i].rootpathlen = len;
                strncat (zi->zones[i].rootpath, "/", MAXPATHLEN - 1);
                if (zi->zones[i].zoneid == 0)
                {
                    zi->globalIdx = i;
                }

                len = zone_getattr (zids[i], ZONE_ATTR_NAME,
                        zi->zones[i].name, ZONENAME_MAX);
                if (*zi->zoneDisplay == '\0' &&
                    zi->myzoneid == zi->zones[i].zoneid)
                {
                    strncpy (zi->zoneDisplay, zi->zones[i].name, ZONENAME_MAX);
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
        printf ("zone:my:%d:%s:glob:%d:\n", (int) diData.zoneInfo.myzoneid,
                diData.zoneInfo.zoneDisplay, diData.zoneInfo.globalIdx);
    }
#endif

        /* initialize dispTable array */
    dispTable [0].size = diopts->baseDispSize;
    for (i = 1; i < DI_DISPTAB_SIZE; ++i)
    {
        dispTable [i].size = dispTable [i - 1].size *
                diopts->baseDispSize;
    }
        /* default - one meg */
    strncpy (diopts->dispBlockLabel, GT(dispTable [DI_ONE_MEG].disp),
             sizeof (diopts->dispBlockLabel));

        /* do this afterwards, because we need to know what the */
        /* user wants for baseDispSize                          */
    setDispBlockSize (dbsstr, diopts);

        /* initialize display size tables */
    sizeTable [0].format = diopts->blockFormatNR;
    sizeTable [1].format = diopts->blockFormat;

    sizeTable [0].high = diopts->baseDispSize;
    sizeTable [1].low = diopts->baseDispSize;
    sizeTable [1].high = diopts->baseDispSize * diopts->baseDispSize;
    sizeTable [1].dbs = diopts->baseDispSize;
    for (i = 2; i < DI_SIZETAB_SIZE; ++i)
    {
        sizeTable [i].format = diopts->blockFormat;
        sizeTable [i].low = sizeTable [i - 1].low * diopts->baseDispSize;
        sizeTable [i].high = sizeTable [i - 1].high * diopts->baseDispSize;
        sizeTable [i].dbs = sizeTable [i - 1].dbs * diopts->baseDispSize;
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

        printf ("dispBlockSize: %8.2f\n", diopts->dispBlockSize);
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

        /* main processing */

    if (di_getDiskEntries (&diData.diskInfo, &diData.count) < 0)
    {
        cleanup (&diData, argvptr);
        exit (1);
    }

    preCheckDiskInfo (&diData);
    if (optind < argc)
    {
        int     rc;

        getDiskStatInfo (&diData);
        rc = checkFileInfo (&diData, optind, argc, argv);
        if (rc < 0)
        {
            cleanup (&diData, argvptr);
            exit (1);
        }
    }
    di_getDiskInfo (&diData.diskInfo, &diData.count);
    getDiskSpecialInfo (&diData);
    checkDiskInfo (&diData);
    printDiskInfo (&diData);

    cleanup (&diData, argvptr);
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
cleanup (diData_t *diData, char *argvptr)
#else
cleanup (diData, argvptr)
    diData_t        *diskInfo;
    char            *argvptr;
#endif
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

    if (argvptr != (char *) NULL)
    {
        free ((char *) argvptr);
    }

    if (diData->zoneInfo.zones != (zoneSummary_t *) NULL)
    {
        free ((void *) diData->zoneInfo.zones);
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
printDiskInfo (diData_t *diData)
#else
printDiskInfo (diData)
    diData_t        *diData;
#endif
{
    int                 i;
    diOptions_t         *diopts;
    diDiskInfo_t        totals;
    char                lastpool [DI_SPEC_NAME_LEN + 1];
    Size_t              lastpoollen = { 0 };
    int                 inpool = { 0 };
    char                tempSortType [DI_SORT_MAX + 1];

    lastpool[0] = '\0';
    memset ((char *) &totals, '\0', sizeof (diDiskInfo_t));
    totals.blockSize = 8192;
    strncpy (totals.name, GT("Total"), DI_NAME_LEN);
    totals.printFlag = DI_PRNT_OK;

    diopts = &diData->options;
    if ((diopts->flags & DI_F_NO_HEADER) != DI_F_NO_HEADER)
    {
        printTitle (diopts);
    }

    if ((diopts->flags & DI_F_TOTAL) == DI_F_TOTAL &&
            (diopts->flags & DI_F_NO_HEADER) != DI_F_NO_HEADER)
    {
            /* need to make a copy of the diskinfo array, as we */
            /* want to preserve the sort order.                 */
        diDiskInfo_t         *tempDiskInfo;

            /* in order to find the main pool entry,                */
            /* we must have the array sorted by special device name */
        tempDiskInfo = (diDiskInfo_t *) malloc (sizeof (diDiskInfo_t) *
                diData->count);
        memcpy ((char *) tempDiskInfo, (char *) diData->diskInfo,
                sizeof (diDiskInfo_t) * diData->count);
        strcpy (tempSortType, diopts->sortType);
        strcpy (diopts->sortType, "s");
        sortArray (diopts, (char *) tempDiskInfo, sizeof (diDiskInfo_t),
                   diData->count, diCompare);
        strcpy (diopts->sortType, tempSortType);

        for (i = 0; i < diData->count; ++i)
        {
            int ispooled = { 0 };

            if (tempDiskInfo [i].printFlag == DI_PRNT_EXCLUDE ||
                tempDiskInfo [i].printFlag == DI_PRNT_BAD ||
                tempDiskInfo [i].printFlag == DI_PRNT_OUTOFZONE)
            {
                continue;
            }

            if (tempDiskInfo [i].printFlag == DI_PRNT_IGNORE &&
                (diopts->flags & DI_F_ALL) != DI_F_ALL)
            {
                continue;
            }

                /* is it a pooled filesystem type? */
            if (strcmp (tempDiskInfo [i].fsType, "zfs") == 0 ||
                strcmp (tempDiskInfo [i].fsType, "advfs") == 0)
            {
                ispooled = 1;
                if (lastpoollen == 0 ||
                    strncmp (lastpool, tempDiskInfo [i].special, lastpoollen) != 0)
                {
                    strncpy (lastpool, tempDiskInfo [i].special, sizeof (lastpool));
                    lastpoollen = strlen (lastpool);
                    inpool = 0;
                }
            }

                /* only total the disks that make sense!                    */
                /* don't add memory filesystems to totals.                  */
                /* only add the main pool to totals for pooled filesystems  */
            if (strcmp (tempDiskInfo [i].fsType, "tmpfs") != 0 &&
                strcmp (tempDiskInfo [i].fsType, "mfs") != 0 &&
                tempDiskInfo [i].isReadOnly == FALSE &&
                inpool == 0)
            {
                addTotals (&tempDiskInfo [i], &totals);
            }

                /* is it a pooled filesystem type? */
            if (ispooled)
            {
                if (strncmp (lastpool, tempDiskInfo [i].special, lastpoollen) == 0)
                {
                    inpool = 1;
                }
            }
        } /* for each entry */

        free ((char *) tempDiskInfo);
    } /* if the totals are to be printed */

    if (strcmp (diopts->sortType, "n") != 0)
    {
        sortArray (diopts, (char *) diData->diskInfo, sizeof (diDiskInfo_t),
                    diData->count, diCompare);
    }

    for (i = 0; i < diData->count; ++i)
    {
        diDiskInfo_t        *dinfo;

        dinfo = &diData->diskInfo [i];
        if (debug > 5)
        {
            printf ("pdi:%s:%s:\n", dinfo->name,
                getPrintFlagText ((int) dinfo->printFlag));
        }
        if (dinfo->printFlag == DI_PRNT_EXCLUDE ||
            dinfo->printFlag == DI_PRNT_BAD ||
            dinfo->printFlag == DI_PRNT_OUTOFZONE)
        {
            continue;
        }

        if (dinfo->printFlag == DI_PRNT_IGNORE &&
            (diopts->flags & DI_F_ALL) != DI_F_ALL)
        {
            continue;
        }

        printInfo (dinfo, diopts);
    }

    if ((diopts->flags & DI_F_TOTAL) == DI_F_TOTAL &&
            (diopts->flags & DI_F_NO_HEADER) != DI_F_NO_HEADER)
    {
        printInfo (&totals, diopts);
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
printInfo (diDiskInfo_t *diskInfo, diOptions_t *diopts)
#else
printInfo (diskInfo, diopts)
    diDiskInfo_t         *diskInfo;
    diOptions_t        *diopts;
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
    if (diopts->dispBlockSize == DI_DISP_HR_2)
    {
        idx = DI_ONE_MEG_SZTAB; /* default */

        ptr = diopts->formatString;
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

    ptr = diopts->formatString;
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
                printf (diopts->mountFormat, diskInfo->name);
                break;
            }

            case DI_FMT_BTOT:
            {
                printBlocks (diopts, diskInfo->totalBlocks, diskInfo->blockSize, idx);
                break;
            }

            case DI_FMT_BTOT_AVAIL:
            {
                printBlocks (diopts, diskInfo->totalBlocks -
                    (diskInfo->freeBlocks - diskInfo->availBlocks),
                    diskInfo->blockSize, idx);
                break;
            }

            case DI_FMT_BUSED:
            {
                printBlocks (diopts, diskInfo->totalBlocks - diskInfo->freeBlocks,
                            diskInfo->blockSize, idx);
                break;
            }

            case DI_FMT_BCUSED:
            {
                printBlocks (diopts, diskInfo->totalBlocks - diskInfo->availBlocks,
                            diskInfo->blockSize, idx);
                break;
            }

            case DI_FMT_BFREE:
            {
                printBlocks (diopts, diskInfo->freeBlocks, diskInfo->blockSize, idx);
                break;
            }

            case DI_FMT_BAVAIL:
            {
                printBlocks (diopts, diskInfo->availBlocks, diskInfo->blockSize, idx);
                break;
            }

            case DI_FMT_BPERC_NAVAIL:
            {
                used = diskInfo->totalBlocks - diskInfo->availBlocks;
                totAvail = diskInfo->totalBlocks;
                if (diopts->posix_compat == 1)
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
                if (diopts->posix_compat == 1)
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
                if (diopts->posix_compat == 1)
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
                printf (diopts->inodeFormat, diskInfo->totalInodes);
                break;
            }

            case DI_FMT_IUSED:
            {
                printf (diopts->inodeFormat, diskInfo->totalInodes - diskInfo->freeInodes);
                break;
            }

            case DI_FMT_IFREE:
            {
                printf (diopts->inodeFormat, diskInfo->freeInodes);
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
                printf (diopts->specialFormat, diskInfo->special);
                break;
            }

            case DI_FMT_TYPE:
            {
                printf (DI_FSTYPE_FMT, diskInfo->fsType);
                break;
            }

            case DI_FMT_TYPE_FULL:
            {
                printf (diopts->typeFormat, diskInfo->fsType);
                break;
            }

            case DI_FMT_MOUNT_OPTIONS:
            {
                printf (diopts->optFormat, diskInfo->options);
                break;
            }

            case DI_FMT_MOUNT_TIME:
            {
#if _lib_mnt_time
                printf (diopts->mTimeFormat, diskInfo->mountTime);
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
printBlocks (diOptions_t *diopts, _fs_size_t blocks,
                        _fs_size_t blockSize, int idx)
#else
printBlocks (diopts, blocks, blockSize, idx)
    diOptions_t        *diopts;
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
    format = diopts->blockFormat;
    tdbs = diopts->dispBlockSize;

    if (diopts->dispBlockSize == DI_DISP_HR)
    {
        temp = (double) blocks * (double) blockSize;
        idx = findDispSize (temp);
    }

    if (diopts->dispBlockSize == DI_DISP_HR ||
        diopts->dispBlockSize == DI_DISP_HR_2)
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
addTotals (diDiskInfo_t *diskInfo, diDiskInfo_t *totals)
#else
addTotals (diskInfo, totals)
    diDiskInfo_t   *diskInfo;
    diDiskInfo_t   *totals;
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
printTitle (diOptions_t *diopts)
#else
printTitle (diopts)
    diOptions_t        *diopts;
#endif
{
    char                *ptr;
    int                 valid;

    if ((diopts->flags & DI_F_DEBUG_HDR) == DI_F_DEBUG_HDR)
    {
        printf (GT("di ver %s Default Format: %s\n"),
                DI_VERSION, DI_DEFAULT_FORMAT);
    }

    ptr = diopts->formatString;

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
                if (diopts->posix_compat == 1)
                {
                    printf (diopts->mountFormat, GT("Mounted On"));
                }
                else
                {
                    printf (diopts->mountFormat, GT("Mount"));
                }
                break;
            }

            case DI_FMT_BTOT:
            case DI_FMT_BTOT_AVAIL:
            {
                printf (diopts->blockLabelFormat, diopts->dispBlockLabel);
                break;
            }

            case DI_FMT_BUSED:
            case DI_FMT_BCUSED:
            {
                printf (diopts->blockLabelFormat, GT("Used"));
                break;
            }

            case DI_FMT_BFREE:
            {
                printf (diopts->blockLabelFormat, GT("Free"));
                break;
            }

            case DI_FMT_BAVAIL:
            {
                if (diopts->posix_compat == 1)
                {
                    printf (diopts->blockLabelFormat, GT("Available"));
                }
                else
                {
                    printf (diopts->blockLabelFormat, GT("Avail"));
                }
                break;
            }

            case DI_FMT_BPERC_NAVAIL:
            case DI_FMT_BPERC_USED:
            case DI_FMT_BPERC_BSD:
            {
                if (diopts->posix_compat == 1)
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
                printf (diopts->inodeLabelFormat, GT("Inodes"));
                break;
            }

            case DI_FMT_IUSED:
            {
                printf (diopts->inodeLabelFormat, GT("Used"));
                break;
            }

            case DI_FMT_IFREE:
            {
                printf (diopts->inodeLabelFormat, GT("Free"));
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
                printf (diopts->specialFormat, GT("Filesystem"));
                break;
            }

            case DI_FMT_TYPE:
            {
                printf (DI_FSTYPE_FMT, GT("fsType"));
                break;
            }

            case DI_FMT_TYPE_FULL:
            {
                printf (diopts->typeFormat, GT("fs Type"));
                break;
            }

            case DI_FMT_MOUNT_OPTIONS:
            {
                printf (diopts->optFormat, GT("Options"));
                break;
            }

            case DI_FMT_MOUNT_TIME:
            {
#if _lib_mnt_time
                printf (diopts->mTimeFormat, GT("Mount Time"));
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
checkFileInfo (diData_t *diData, int optidx, int argc, char *argv [])
#else
checkFileInfo (diData, optidx, argc, argv)
    diData_t            *diData;
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
    for (j = 0; j < diData->count; ++j)
    {
        if (diData->diskInfo [j].printFlag == DI_PRNT_OK)
        {
            diData->diskInfo [j].printFlag = DI_PRNT_IGNORE;
        }
    }

    for (i = optidx; i < argc; ++i)
    {
        if (stat (argv [i], &statBuf) == 0)
        {
            for (j = 0; j < diData->count; ++j)
            {
                diDiskInfo_t        *dinfo;

                dinfo = &diData->diskInfo[j];
                if (dinfo->printFlag == DI_PRNT_IGNORE &&
                    dinfo->st_dev != DI_UNKNOWN_DEV &&
                    (__ulong) statBuf.st_dev == dinfo->st_dev)
                {
                    dinfo->printFlag = DI_PRNT_OK;
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
sortArray (diOptions_t *diopts, char *data, Size_t dataSize, int count, DI_SORT_FUNC compareFunc)
#else
sortArray (diopts, data, dataSize, count, compareFunc)
    diOptions_t    *diopts;
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
            j = i - gap;

            memcpy ((char *) tempData, (char *) &(data [i * dataSize]),
                    dataSize);
            while (j >= 0 &&
                compareFunc (diopts,
                    &(data [j * dataSize]), tempData) > 0)
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
diCompare (diOptions_t *diopts, void *a, void *b)
#else
diCompare (diopts, a, b)
    diOptions_t     *diopts;
    void            *a;
    void            *b;
#endif
{
    diDiskInfo_t    *di1;
    diDiskInfo_t    *di2;
    int             rc;
    char            *ptr;


    di1 = (diDiskInfo_t *) a;
    di2 = (diDiskInfo_t *) b;

        /* reset sort order to the default start value */
    diopts->sortOrder = DI_SORT_ASCENDING;
    rc = 0;
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
                rc = strcmp (di1->name, di2->name);
                break;
            }

            case DI_SORT_REVERSE:
            {
                diopts->sortOrder *= -1;
                break;
            }

            case DI_SORT_SPECIAL:
            {
                rc = strcmp (di1->special, di2->special);
                break;
            }

            case DI_SORT_TYPE:
            {
                rc = strcmp (di1->fsType, di2->fsType);
                break;
            }

            case DI_SORT_AVAIL:
            {
                rc = (int) ((di1->availBlocks * di1->blockSize) -
                        (di2->availBlocks * di2->blockSize));
                break;
            }
        } /* switch on sort type */

        rc *= diopts->sortOrder;
        if (rc != 0)
        {
            return rc;
        }

        ++ptr;
    }

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
getDiskStatInfo (diData_t *diData)
#else
getDiskStatInfo (diData)
    diData_t            *diData;
#endif
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
            fprintf (stderr, "stat: %s ", dinfo->name);
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
getDiskSpecialInfo (diData_t *diData)
#else
getDiskSpecialInfo (diData)
    diData_t            *diData;
#endif
{
    int         i;
    struct stat statBuf;

    for (i = 0; i < diData->count; ++i)
    {
        diDiskInfo_t        *dinfo;

        dinfo = &diData->diskInfo [i];
           /* check for initial slash; otherwise we can pick up normal files */
        if (*(dinfo->special) == '/' &&
            stat (dinfo->special, &statBuf) == 0)
        {
            dinfo->sp_dev = (__ulong) statBuf.st_dev;
            dinfo->sp_rdev = (__ulong) statBuf.st_rdev;
            if (debug > 2)
            {
                printf ("special dev: %s: %ld rdev: %ld\n",
                        dinfo->special, dinfo->sp_dev,
                        dinfo->sp_rdev);
            }
        }
        else
        {
            dinfo->sp_dev = 0;
            dinfo->sp_rdev = 0;
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
checkDiskInfo (diData_t *diData)
#else
checkDiskInfo (diData)
    diData_t            *diData;
#endif
{
    int             i;
    int             j;
    int             dupCount;
    int             len;
    int             maxMountString;
    int             maxSpecialString;
    int             maxTypeString;
    int             maxOptString;
    int             maxMntTimeString;
    _fs_size_t      temp;
    __ulong         sp_dev;
    __ulong         sp_rdev;
    diOptions_t     *diopts;


    diopts = &diData->options;
    maxMountString = (diopts->flags & DI_F_NO_HEADER) == DI_F_NO_HEADER ? 0 : 5;
    maxSpecialString = (diopts->flags & DI_F_NO_HEADER) == DI_F_NO_HEADER ? 0 : 10;
    maxTypeString = (diopts->flags & DI_F_NO_HEADER) == DI_F_NO_HEADER ? 0 : 7;
    maxOptString = (diopts->flags & DI_F_NO_HEADER) == DI_F_NO_HEADER ? 0 : 7;
    maxMntTimeString = (diopts->flags & DI_F_NO_HEADER) == DI_F_NO_HEADER ? 0 : 26;

    for (i = 0; i < diData->count; ++i)
    {
        diDiskInfo_t        *dinfo;

        dinfo = &diData->diskInfo[i];
        if (dinfo->printFlag == DI_PRNT_EXCLUDE ||
            dinfo->printFlag == DI_PRNT_BAD ||
            dinfo->printFlag == DI_PRNT_OUTOFZONE)
        {
            if (debug > 2)
            {
                printf ("chk: skipping(%s):%s\n",
                    getPrintFlagText ((int) dinfo->printFlag), dinfo->name);
            }
            continue;
        }

            /* Solaris reports a cdrom as having no free blocks,   */
            /* no available.  Their df doesn't always work right!  */
            /* -1 is returned.                                     */
        if (debug > 2)
        {
#if _siz_long_long >= 8
            printf ("chk: %s free: %llu\n", dinfo->name,
                dinfo->freeBlocks);
#else
            printf ("chk: %s free: %lu\n", dinfo->name,
                dinfo->freeBlocks);
#endif
        }
        if ((_s_fs_size_t) dinfo->freeBlocks < 0L)
        {
            dinfo->freeBlocks = 0L;
        }
        if ((_s_fs_size_t) dinfo->availBlocks < 0L)
        {
            dinfo->availBlocks = 0L;
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
            if (debug > 2)
            {
#if _siz_long_long >= 8
                printf ("chk: %s total: %lld\n", dinfo->name,
                        dinfo->totalBlocks);
#else
                printf ("chk: %s total: %ld\n", dinfo->name,
                        dinfo->totalBlocks);
#endif
            }
            if (dinfo->totalBlocks <= 0L)
            {
                dinfo->printFlag = DI_PRNT_IGNORE;
                if (debug > 2)
                {
                    printf ("chk: ignore: totalBlocks <= 0: %s\n",
                            dinfo->name);
                }
            }
        }

        /* make sure anything in the include list didn't get turned off */
        checkIncludeList (dinfo, &diData->includeList);
    } /* for all disks */

    if ((diopts->flags & DI_F_INCLUDE_LOOPBACK) != DI_F_INCLUDE_LOOPBACK)
    {
            /* this loop sets duplicate entries to be ignored. */
        for (i = 0; i < diData->count; ++i)
        {
            diDiskInfo_t        *dinfo;

            dinfo = &diData->diskInfo[i];
            if (dinfo->printFlag == DI_PRNT_IGNORE ||
                dinfo->printFlag == DI_PRNT_EXCLUDE ||
                dinfo->printFlag == DI_PRNT_BAD ||
                dinfo->printFlag == DI_PRNT_OUTOFZONE)
            {
                if (debug > 2)
                {
                    printf ("chk: skipping(%s):%s\n",
                        getPrintFlagText ((int) dinfo->printFlag), dinfo->name);
                }
                continue;
            }

                /* don't need to bother checking real partitions */
                /* don't bother if already ignored               */
            if (dinfo->sp_rdev != 0 &&
                dinfo->printFlag == DI_PRNT_OK)
            {
                sp_dev = dinfo->sp_dev;
                sp_rdev = dinfo->sp_rdev;
                dupCount = 0;

                for (j = 0; j < diData->count; ++j)
                {
                    if (diData->diskInfo [j].sp_dev == sp_rdev)
                    {
                        ++dupCount;
                    }
                }

                if (debug > 2)
                {
                    printf ("dup: chk: i: %s dev: %ld rdev: %ld dup: %d\n",
                        dinfo->name, sp_dev, sp_rdev, dupCount);
                }

                for (j = 0; dupCount > 0 && j < diData->count; ++j)
                {
                    diDiskInfo_t        *dinfob;

                    dinfob = &diData->diskInfo [j];
                        /* don't reset flags! */
                    if (dinfob->sp_rdev == 0 &&
                        dinfob->sp_dev == sp_rdev &&
                        dinfob->printFlag == DI_PRNT_OK)
                    {
                        dinfob->printFlag = DI_PRNT_IGNORE;
                        if (debug > 2)
                        {
                            printf ("chk: ignore: duplicate: %s of %s\n",
                                    dinfob->name, dinfo->name);
                            printf ("dup: ign: j: rdev: %ld dev: %ld\n",
                                    dinfob->sp_dev, dinfob->sp_rdev);
                        }
                    }
                } /* duplicate check for each disk */
            } /* if this is a printable disk */
            else
            {
                if (debug > 2)
                {
                    printf ("chk: dup: not checked: %s prnt: %d dev: %ld rdev: %ld\n",
                            dinfo->name, dinfo->printFlag,
                            dinfo->sp_dev, dinfo->sp_rdev);
                }
            }
        } /* for each disk */
    } /* if the duplicate loopback mounts are to be excluded */

    if (diopts->posix_compat == 1)
    {
        /* Mounted On */
        len = strlen (GT("Mounted On"));
        maxMountString = maxMountString < len ? len : maxMountString;
        len = strlen (diopts->dispBlockLabel);
        diopts->width = diopts->width < len ? len : diopts->width;
    }

        /* this loop gets the max string lengths */
    for (i = 0; i < diData->count; ++i)
    {
        diDiskInfo_t        *dinfo;

        dinfo = &diData->diskInfo[i];
        if (dinfo->printFlag == DI_PRNT_OK ||
            (dinfo->printFlag == DI_PRNT_IGNORE &&
             (diopts->flags & DI_F_ALL) == DI_F_ALL))
        {
            len = strlen (dinfo->name);
            if (len > maxMountString)
            {
                maxMountString = len;
            }

            len = strlen (dinfo->special);
            if (len > maxSpecialString)
            {
                maxSpecialString = len;
            }

            len = strlen (dinfo->fsType);
            if (len > maxTypeString)
            {
                maxTypeString = len;
            }

            len = strlen (dinfo->options);
            if (len > maxOptString)
            {
                maxOptString = len;
            }

            len = strlen (dinfo->mountTime);
            if (len > maxMntTimeString)
            {
                maxMntTimeString = len;
            }
        } /* if we are printing this item */
    } /* for all disks */

    Snprintf (SPF(diopts->mountFormat, sizeof (diopts->mountFormat), "%%-%d.%ds"),
              maxMountString, maxMountString);
    Snprintf (SPF(diopts->specialFormat, sizeof (diopts->specialFormat), "%%-%d.%ds"),
              maxSpecialString, maxSpecialString);
    Snprintf (SPF(diopts->typeFormat, sizeof (diopts->typeFormat), "%%-%d.%ds"),
              maxTypeString, maxTypeString);
    Snprintf (SPF(diopts->optFormat, sizeof (diopts->optFormat), "%%-%d.%ds"),
              maxOptString, maxOptString);
    Snprintf (SPF(diopts->mTimeFormat, sizeof (diopts->mTimeFormat), "%%-%d.%ds"),
              maxMntTimeString, maxMntTimeString);

    if (diopts->dispBlockSize == DI_DISP_HR ||
        diopts->dispBlockSize == DI_DISP_HR_2)
    {
        --diopts->width;
    }

    if (diopts->dispBlockSize != DI_DISP_HR &&
        diopts->dispBlockSize != DI_DISP_HR_2 &&
        (diopts->dispBlockSize > 0 && diopts->dispBlockSize <= DI_VAL_1024))
    {
        Snprintf (SPF(diopts->blockFormat,
                  sizeof (diopts->blockFormat), "%%%d.0f%%s"), diopts->width);
    }
    else
    {
        Snprintf (SPF(diopts->blockFormatNR,
                  sizeof (diopts->blockFormatNR), "%%%d.0f%%s"), diopts->width);
        Snprintf (SPF(diopts->blockFormat,
                  sizeof (diopts->blockFormat), "%%%d.1f%%s"), diopts->width);
    }

    if (diopts->dispBlockSize == DI_DISP_HR ||
        diopts->dispBlockSize == DI_DISP_HR_2)
    {
        ++diopts->width;
    }

    Snprintf (SPF(diopts->blockLabelFormat,
              sizeof (diopts->blockLabelFormat), "%%%ds"), diopts->width);
#if _siz_long_long >= 8
    Snprintf (SPF(diopts->inodeFormat,
              sizeof (diopts->inodeFormat), "%%%dllu"), diopts->inodeWidth);
#else
    Snprintf (SPF(diopts->inodeFormat,
              sizeof (diopts->inodeFormat), "%%%dlu"), diopts->inodeWidth);
#endif
    Snprintf (SPF(diopts->inodeLabelFormat,
              sizeof (diopts->inodeLabelFormat), "%%%ds"), diopts->inodeWidth);
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
preCheckDiskInfo (diData_t *diData)
#else
preCheckDiskInfo (diData)
    diData_t            *diData;
#endif
{
    int             i;
    diOptions_t     *diopts;

    diopts = &diData->options;
    for (i = 0; i < diData->count; ++i)
    {
        diDiskInfo_t        *dinfo;

        dinfo = &diData->diskInfo[i];
        if (debug > 4)
        {
            printf ("## prechk:%s:\n", dinfo->name);
        }
        checkZone (dinfo, &diData->zoneInfo,
            ((diopts->flags & DI_F_ALL) == DI_F_ALL));

        if (dinfo->printFlag == DI_PRNT_OK)
        {
                /* don't bother w/this check is all flag is set. */
            if ((diopts->flags & DI_F_ALL) != DI_F_ALL)
            {
                di_testRemoteDisk (dinfo);

                if (dinfo->isLocal == FALSE &&
                        (diopts->flags & DI_F_LOCAL_ONLY) == DI_F_LOCAL_ONLY)
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
            /* do do these checks to override the all flag */
            checkIgnoreList (dinfo, &diData->ignoreList);
            checkIncludeList (dinfo, &diData->includeList);
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
processArgs (int argc,
             char *argv [],
             diData_t *diData,
             char *dbsstr)
#else
processArgs (argc, argv, diData, dbsstr)
    int             argc;
    char            *argv [];
    diData_t        *diData;
    char            *dbsstr;
#endif
{
    int         ch;
    int         hasdashk;
    diOptions_t *diopts;


    diopts = &diData->options;
    hasdashk = 0;
    while ((ch = getopt (argc, argv,
        "Aab:B:d:D:f:F:ghHi:I:klLmnPs:tvw:W:x:X:z:Z")) != -1)
    {
        switch (ch)
        {
            case 'A':   /* for debugging */
            {
                diopts->formatString = DI_ALL_FORMAT;
                break;
            }

            case 'a':
            {
                diopts->flags |= DI_F_ALL;
                strcpy (diData->zoneInfo.zoneDisplay, "all");
                break;
            }

            case 'b':
            case 'B':
            {
                if (isdigit ((int) (*optarg)))
                {
                    diopts->baseDispSize = atof (optarg);
                }
                else if (*optarg == 'k')
                {
                    diopts->baseDispSize = DI_VAL_1024;
                }
                else if (*optarg == 'd')
                {
                    diopts->baseDispSize = DI_VAL_1000;
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
                diopts->formatString = optarg;
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
                parseList (&diData->ignoreList, optarg);
                break;
            }

            case 'F':  /* compatibility w/other df */
            case 'I':
            {
                parseList (&diData->includeList, optarg);
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
                diopts->flags |= DI_F_LOCAL_ONLY;
                break;
            }

            case 'L':
            {
                diopts->flags |= DI_F_INCLUDE_LOOPBACK;
                break;
            }

            case 'n':
            {
                diopts->flags |= DI_F_NO_HEADER;
                break;
            }

            case 'P':
            {
                if (hasdashk == 0) /* don't override -k option */
                {
                    strncpy (dbsstr, "512", sizeof (dbsstr));
                }
                diopts->formatString = DI_POSIX_FORMAT;
                diopts->posix_compat = 1;
                break;
            }

            case 's':
            {
                strncpy (diopts->sortType, optarg, sizeof (diopts->sortType));
                    /* for backwards compatibility                       */
                    /* reverse by itself - change to reverse mount point */
                if (strcmp (diopts->sortType, "r") == 0)
                {
                    strcpy (diopts->sortType, "rm");
                }
                    /* add some sense to the sort order */
                if (strcmp (diopts->sortType, "t") == 0)
                {
                    strcpy (diopts->sortType, "tm");
                }
                break;
            }

            case 't':
            {
                diopts->flags |= DI_F_TOTAL;
                break;
            }

            case 'v':
            {
                break;
            }

            case 'w':
            {
                diopts->width = atoi (optarg);
                break;
            }

            case 'W':
            {
                diopts->inodeWidth = atoi (optarg);
                break;
            }

            case 'X':
            {
                debug = atoi (optarg);
                di_lib_debug = debug;
                diopts->flags |= DI_F_DEBUG_HDR | DI_F_TOTAL;
                diopts->flags &= ~ DI_F_NO_HEADER;
                diopts->width = 10;
                diopts->inodeWidth = 10;
                break;
            }

            case 'z':
            {
                strcpy (diData->zoneInfo.zoneDisplay, optarg);
                break;
            }

            case 'Z':
            {
                strcpy (diData->zoneInfo.zoneDisplay, "all");
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
parseList (iList_t *list, char *str)
#else
parseList (list, str)
    iList_t       *list;
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
checkIgnoreList (diDiskInfo_t *diskInfo, iList_t *ignoreList)
#else
checkIgnoreList (diskInfo, ignoreList)
    diDiskInfo_t     *diskInfo;
    iList_t           *ignoreList;
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
checkIncludeList (diDiskInfo_t *diskInfo, iList_t *includeList)
#else
checkIncludeList (diskInfo, includeList)
    diDiskInfo_t     *diskInfo;
    iList_t           *includeList;
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
checkZone (diDiskInfo_t *diskInfo, zoneInfo_t *zoneInfo, int allFlag)
# else
checkZone (diskInfo, zoneInfo, allFlag)
    diDiskInfo_t *diskInfo;
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
setDispBlockSize (char *ptr, diOptions_t *diopts)
#else
setDispBlockSize (ptr, diopts)
    char            *ptr;
    diOptions_t     *diopts;
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
                strncpy (diopts->dispBlockLabel, GT("Size"),
                    sizeof (diopts->dispBlockLabel));
                break;
            }

            case 'H':
            {
                val = DI_DISP_HR_2;
                strncpy (diopts->dispBlockLabel, GT("Size"),
                    sizeof (diopts->dispBlockLabel));
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
                    val = diopts->dispBlockSize;
                }
                break;
            }
        }

        if (idx >= 0)
        {
            if (val == 1.0)
            {
                strncpy (diopts->dispBlockLabel, GT(dispTable [idx].disp),
                    sizeof (diopts->dispBlockLabel));
            }
            else
            {
                Snprintf (SPF (diopts->dispBlockLabel,
                    sizeof (diopts->dispBlockLabel), "%.0f %s"),
                    val, GT(dispTable [idx].disp));
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
                strncpy (diopts->dispBlockLabel, GT(dispTable [i].disp),
                     sizeof (diopts->dispBlockLabel));
                ok = 1;
                break;
            }
        }

        if (ok == 0)
        {
            Snprintf (SPF (diopts->dispBlockLabel,
                sizeof (diopts->dispBlockLabel), "%.0fb"), val);
        }
    }  /* some oddball block size */

    if (diopts->posix_compat == 1 && val == 512)
    {
        strncpy (diopts->dispBlockLabel, GT ("512-blocks"),
            sizeof (diopts->dispBlockLabel));
    }
    if (diopts->posix_compat == 1 && val == 1024)
    {
        strncpy (diopts->dispBlockLabel, GT ("1024-blocks"),
            sizeof (diopts->dispBlockLabel));
    }

    diopts->dispBlockSize = val;
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
