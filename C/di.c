/*
 * $Id$
 * $Source$
 *
 * Copyright 1994-2011 Brad Lanam, Walnut Creek, CA
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
 *  that do not report, available blocks, the number of available blocks is
 *  equal to the number of free blocks.
 *
 */

#include "config.h"
#include "di.h"
#include "version.h"
#include "getoptn.h"

#if _hdr_stdio
# include <stdio.h>
#endif
#if _hdr_stdlib
# include <stdlib.h>
#endif
#if _sys_types
# include <sys/types.h>
#endif
#if _hdr_ctype
# include <ctype.h>
#endif
#if _hdr_errno
# include <errno.h>
#endif
#if _hdr_fcntl
# include <fcntl.h>
#endif
#if _sys_file
# include <sys/file.h>
#endif
#if _hdr_string
# include <string.h>
#endif
#if _hdr_strings
# include <strings.h>
#endif
#if _hdr_memory
# include <memory.h>
#endif
#if _hdr_malloc
# include <malloc.h>
#endif
#if _hdr_unistd
# include <unistd.h>
#endif
#if _hdr_time
# include <time.h>
#endif
#if _sys_time && _inc_conflict__hdr_time__sys_time
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
#if _hdr_wchar
# include <wchar.h>
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

#define DI_FMT_VALID_CHARS     "mMsStTbBucfvp12a3iUFPIO"

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

#define DI_UNKNOWN_DEV          -1L
#define DI_LIST_SEP             ","
#define DI_ARGV_SEP             " 	"  /* space, tab */
#define DI_MAX_ARGV             50

#define DI_ALL_FORMAT           "MTS\n\tIO\n\tbuf13\n\tbcvpa\n\tBuv2\n\tiUFP"
#define DI_POSIX_FORMAT         "SbuvpM"
#define DI_DEF_MOUNT_FORMAT     "MST\n\tO"

#define DI_VAL_1000             1000.0
#define DI_DISP_1000_IDX        0
#define DI_VAL_1024             1024.0
#define DI_DISP_1024_IDX        1
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

#define DI_PERC_FMT             " %%3.0%s%%%% "
#define DI_POSIX_PERC_FMT       "    %%3.0%s%%%% "
#define DI_JUST_LEFT            0
#define DI_JUST_RIGHT           1

typedef struct
{
    int    count;
    char   **list;
} iList_t;

typedef struct
{
    _print_size_t   low;
    _print_size_t   high;
    _print_size_t   dbs;        /* display block size */
    char            *format;
    char            *suffix;
} sizeTable_t;

typedef struct
{
    _print_size_t   size;
    char            *disp[2];
} dispTable_t;

#if ! _lib_zone_list
# define zoneid_t       int
# define ZONENAME_MAX   65
#endif

typedef struct {
    zoneid_t    zoneid;
    char        name [ZONENAME_MAX + 1];
    char        rootpath [MAXPATHLEN + 1];
    Size_t      rootpathlen;
} zoneSummary_t;

typedef struct {
    Uid_t           uid;
    zoneid_t        myzoneid;
    zoneSummary_t   *zones;
    Uint_t          zoneCount;
    char            zoneDisplay [MAXPATHLEN + 1];
    int             globalIdx;
} zoneInfo_t;

typedef struct {
    const char      *formatString;
    _print_size_t   dispBlockSize;
    _print_size_t   baseDispSize;
    unsigned int    baseDispIdx;
    char            sortType [DI_SORT_MAX + 1];
    unsigned int    posix_compat;
    unsigned int    quota_check;
    unsigned int    csv_output;
    unsigned int    excludeLoopback;
    unsigned int    printTotals;
    unsigned int    printDebugHeader;
    unsigned int    printHeader;
    unsigned int    printAllColumns;
    unsigned int    localOnly;
} diOptions_t;

typedef struct {
    Size_t       inodeWidth;
    Size_t       maxMntTimeString;
    Size_t       maxMountString;
    Size_t       maxOptString;
    Size_t       maxSpecialString;
    Size_t       maxTypeString;
    Size_t       width;
    char         *dispBlockLabel;
    char         blockFormat [15];
    char         blockFormatNR [15];   /* no radix */
    char         inodeFormat [15];
    char         inodeLabelFormat [15];
    char         mountFormat [10];
    char         mTimeFormat [15];
    char         optFormat [15];
    char         specialFormat [15];
    char         typeFormat [10];
} diOutput_t;

typedef struct {
    int             count;
    int             haspooledfs;
    int             disppooledfs;
    int             totsorted;
    diOptions_t     options;
    diOutput_t      output;
    diDiskInfo_t    *diskInfo;
    iList_t         ignoreList;
    iList_t         includeList;
    zoneInfo_t      zoneInfo;
} diData_t;

int          debug = { 0 };

static sizeTable_t sizeTable [] =
{
    { 0, 0, 1, (char *) NULL, " " },
    { 0, 0, 0, (char *) NULL, "K" },
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
    { 0.0, { "KBytes", "KBytes" } },
    { 0.0, { "Megs", "Mebis" } },
    { 0.0, { "Gigs", "Gibis" } },
    { 0.0, { "Teras", "Tebis" } },
    { 0.0, { "Petas", "Pebis" } },
    { 0.0, { "Exas", "Exbis" } },
    { 0.0, { "Zettas", "Zebis" } },
    { 0.0, { "Yottas", "Yobis" } }
};
#define DI_DISPTAB_SIZE (sizeof (dispTable) / sizeof (dispTable_t))

#if defined (__cplusplus) || defined (c_plusplus)
  extern "C" {
#endif

static void addTotals           _((const diDiskInfo_t *, diDiskInfo_t *, int));
static void checkDiskInfo       _((diData_t *, int));
static void checkDiskQuotas     _((diData_t *));
static void getMaxFormatLengths _((diData_t *));
static int  checkFileInfo       _((diData_t *, int, int, const char *const[]));
static void checkIgnoreList     _((diDiskInfo_t *, iList_t *));
static void checkIncludeList    _((diDiskInfo_t *, iList_t *));
#if _lib_zone_list && _lib_getzoneid && _lib_zone_getattr
static void checkZone           _((diDiskInfo_t *, zoneInfo_t *, int));
#endif
static void cleanup             _((diData_t *, const char *));
static int  diCompare           _((const diOptions_t *, const diDiskInfo_t *, unsigned int, unsigned int));
static int  findDispSize        _((_print_size_t));
static int  getDiskSpecialInfo  _((diData_t *));
static void getDiskStatInfo     _((diData_t *));
static const char *getPrintFlagText _((int));
static Size_t istrlen           _((const char *));
static void parseList           _((iList_t *, char *));
static void preCheckDiskInfo     _((diData_t *));
static void printDiskInfo       _((diData_t *));
static void printInfo           _((diDiskInfo_t *, diOptions_t *, diOutput_t *));
static void printBlocks         _((const diOptions_t *, const diOutput_t *, _fs_size_t, _fs_size_t, int));
static void processTitles       _((diOptions_t *, diOutput_t *));
static void printPerc           _((_fs_size_t, _fs_size_t, const char *));
static int processArgs          _((int, const char * const [], diData_t *, char *, Size_t));
static void setDispBlockSize    _((char *, diOptions_t *, diOutput_t *));
static void sortArray           _((diOptions_t *, diDiskInfo_t *, int, int));
static void usage               _((void));

#if defined (__cplusplus) || defined (c_plusplus)
  }
#endif

int
#if _proto_stdc
main (int argc, const char * argv [])
#else
main (argc, argv)
    int         argc;
    const char  * argv [];
#endif
{
    diData_t            diData;
    diOptions_t         *diopts;
    diOutput_t          *diout;
    int                 i;
    int                 optidx;
    int                 hasLoop;
    char                *ptr;
#if _enable_nls
    char                *localeptr;
#endif
    const char          *argvptr;
    char                *dptr;
    char                dbsstr [30];

        /* initialization */
    diData.count = 0;
    diData.haspooledfs = FALSE;
    diData.disppooledfs = FALSE;
    diData.totsorted = FALSE;

    diData.diskInfo = (diDiskInfo_t *) NULL;

    diData.ignoreList.count = 0;
    diData.ignoreList.list = (char **) NULL;

    diData.includeList.count = 0;
    diData.includeList.list = (char **) NULL;

#if _lib_zone_list && _lib_getzoneid && _lib_zone_getattr
    diData.zoneInfo.uid = geteuid ();
#endif
    diData.zoneInfo.zoneDisplay [0] = '\0';
    diData.zoneInfo.zoneCount = 0;
    diData.zoneInfo.zones = (zoneSummary_t *) NULL;

        /* options defaults */
    diopts = &diData.options;
    diopts->formatString = DI_DEFAULT_FORMAT;
        /* change default display format here */
    diopts->dispBlockSize = DI_VAL_1024 * DI_VAL_1024;
    diopts->excludeLoopback = FALSE;
    diopts->printTotals = FALSE;
    diopts->printDebugHeader = FALSE;
    diopts->printHeader = TRUE;
    diopts->localOnly = FALSE;
    diopts->excludeLoopback = FALSE;
    /* loopback devices (lofs) should be excluded by default */
#if ! _lib_sys_dollar_device_scan  /* not VMS */
    diopts->excludeLoopback = TRUE;
#endif
    strncpy (diopts->sortType, "m", DI_SORT_MAX); /* default - by mount point*/
    diopts->posix_compat = FALSE;
    diopts->baseDispSize = DI_VAL_1024;
    diopts->baseDispIdx = DI_DISP_1024_IDX;
    diopts->quota_check = TRUE;
    diopts->csv_output = FALSE;

    diout = &diData.output;
    diout->width = 8;
    diout->inodeWidth = 9;
    diout->maxMountString = 0;  /* 15 */
    diout->maxSpecialString = 0; /* 18 */
    diout->maxTypeString = 0; /* 7 */
    diout->maxOptString = 0;
    diout->maxMntTimeString = 0;

    strncpy (dbsstr, DI_DEFAULT_DISP_SIZE, sizeof (dbsstr)); /* default */

#if _lib_setlocale && defined (LC_ALL)
    ptr = setlocale (LC_ALL, "");
#endif
#if _enable_nls
    if ((localeptr = getenv ("DI_LOCALE_DIR")) == (char *) NULL) {
      localeptr = DI_LOCALE_DIR;
    }
    ptr = bindtextdomain ("di", localeptr);
    ptr = textdomain ("di");
#endif

    argvptr = argv [0] + strlen (argv [0]) - 2;
    if (memcmp (argvptr, "mi", (Size_t) 2) == 0)
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
        diopts->posix_compat = TRUE;
        diopts->csv_output = FALSE;
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

    dptr = (char *) NULL;
    if ((ptr = getenv ("DI_ARGS")) != (char *) NULL)
    {
        char        *tptr;
        int         nargc;
        const char  *nargv [DI_MAX_ARGV];

        dptr = strdup (ptr);
        if (dptr == (char *) NULL)
        {
            fprintf (stderr, "strdup failed in main() (1).  errno %d\n", errno);
            exit (1);
        }
        if (dptr != (char *) NULL)
        {
            tptr = strtok (dptr, DI_ARGV_SEP);
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
            processArgs (nargc, nargv, &diData, dbsstr, sizeof (dbsstr) - 1);
        }
    }

    optidx = processArgs (argc, argv, &diData, dbsstr, sizeof (dbsstr) - 1);
    if (debug > 0 && (ptr = getenv ("DI_ARGS")) != (char *) NULL)
    {
        printf ("# DI_ARGS:%s\n", ptr);
    }
    if (debug > 0)
    {
        int j;
        printf ("# ARGS:");
        for (j = 0; j < argc; ++j)
        {
          printf (" %s", argv[j]);
        }
        printf ("\n");
        printf ("# blocksize: %s\n", dbsstr);
    }

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
        printf ("zone:my:%d:%s:glob:%d:\n", (int) diData.zoneInfo.myzoneid,
                diData.zoneInfo.zoneDisplay, diData.zoneInfo.globalIdx);
    }
#endif

        /* initialize dispTable array */
    dispTable [0].size = diopts->baseDispSize;
    for (i = 1; i < (int) DI_DISPTAB_SIZE; ++i)
    {
        dispTable [i].size = dispTable [i - 1].size *
                diopts->baseDispSize;
    }

    setDispBlockSize (dbsstr, diopts, diout);

        /* initialize display size tables */
    sizeTable [0].format = diout->blockFormatNR;
    sizeTable [1].format = diout->blockFormat;

    sizeTable [0].high = diopts->baseDispSize;
    sizeTable [1].low = diopts->baseDispSize;
    sizeTable [1].high = diopts->baseDispSize * diopts->baseDispSize;
    sizeTable [1].dbs = diopts->baseDispSize;
    for (i = 2; i < (int) DI_SIZETAB_SIZE; ++i)
    {
        sizeTable [i].format = diout->blockFormat;
        sizeTable [i].low = sizeTable [i - 1].low * diopts->baseDispSize;
        sizeTable [i].high = sizeTable [i - 1].high * diopts->baseDispSize;
        sizeTable [i].dbs = sizeTable [i - 1].dbs * diopts->baseDispSize;
    }

    if (debug > 0)
    {
        printf ("di version %s\n", DI_VERSION);
    }

    if (debug > 30)
    {
        for (i = 0; i < (int) DI_DISPTAB_SIZE; ++i)
        {
            printf ("i:%d ", i);
            printf ("size: %8.2Lf ", dispTable[i].size);
            printf ("%s ", dispTable[i].disp[0]);
            printf ("%s ", dispTable[i].disp[1]);
            printf ("\n");
        }

        printf ("dispBlockSize: %8.2Lf\n", diopts->dispBlockSize);
        for (i = 0; i < (int) DI_SIZETAB_SIZE; ++i)
        {
            printf ("i:%d ", i);
            printf ("suffix: %s ", sizeTable[i].suffix);
            printf ("low: %8.2Lf ", sizeTable[i].low);
            printf ("high: %8.2Lf ", sizeTable[i].high);
            printf ("dbs: %8.2Lf ", sizeTable[i].dbs);
            printf ("\n");
        }
    }

        /* main processing */

    if (di_getDiskEntries (&diData.diskInfo, &diData.count) < 0)
    {
        cleanup (&diData, dptr);
        exit (1);
    }

    hasLoop = FALSE;
    preCheckDiskInfo (&diData);
    if (optidx < argc || diopts->excludeLoopback)
    {
      getDiskStatInfo (&diData);
      hasLoop = getDiskSpecialInfo (&diData);
    }
    if (optidx < argc)
    {
        int     rc;

        rc = checkFileInfo (&diData, optidx, argc, argv);
        if (rc < 0)
        {
            cleanup (&diData, dptr);
            exit (1);
        }
    }
    di_getDiskInfo (&diData.diskInfo, &diData.count);
    checkDiskInfo (&diData, hasLoop);
    if (diopts->quota_check == TRUE) {
      checkDiskQuotas (&diData);
    }
    printDiskInfo (&diData);

    cleanup (&diData, dptr);
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
cleanup (diData_t *diData, const char *dptr)
#else
cleanup (diData, dptr)
    diData_t   *diData;
    const char *dptr;
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

    if (dptr != (char *) NULL)
    {
        free ((char *) dptr);
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
 * The method to get widths and handle titles and etc. is rather a
 * mess.  There may be a better way to handle it.
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
    diDiskInfo_t        *diskInfo;
    diDiskInfo_t        totals;
    char                lastpool [DI_SPEC_NAME_LEN + 1];
    Size_t              lastpoollen = { 0 };
    int                 inpool = { FALSE };
    diOutput_t          *diout;

    lastpool[0] = '\0';
    diopts = &diData->options;
    diout = &diData->output;

    if (diopts->printTotals)
    {
        di_initDiskInfo (&totals);
        totals.blockSize = 512;
        strncpy (totals.name, DI_GT("Total"), (Size_t) DI_NAME_LEN);
        totals.printFlag = DI_PRNT_OK;
    }

    getMaxFormatLengths (diData);
    processTitles (diopts, diout);

    if (diopts->dispBlockSize == DI_DISP_HR ||
        diopts->dispBlockSize == DI_DISP_HR_2)
    {
        --diout->width;
    }

    if (diopts->dispBlockSize != DI_DISP_HR &&
        diopts->dispBlockSize != DI_DISP_HR_2 &&
        (diopts->dispBlockSize > 0 && diopts->dispBlockSize <= DI_VAL_1024)) {
      if (diopts->csv_output) {
        Snprintf1 (diout->blockFormat, sizeof (diout->blockFormat),
            "%%.0%s%%s", DI_Lf);
      } else {
        Snprintf2 (diout->blockFormat, sizeof (diout->blockFormat),
            "%%%d.0%s%%s", (int) diout->width, DI_Lf);
      }
    } else {
      if (diopts->csv_output) {
        Snprintf1 (diout->blockFormatNR, sizeof (diout->blockFormatNR),
            "%%.0%s%%s", DI_Lf);
        Snprintf1 (diout->blockFormat, sizeof (diout->blockFormat),
            "\"%%.1%s%%s\"", DI_Lf);
      } else {
        Snprintf2 (diout->blockFormatNR, sizeof (diout->blockFormatNR),
            "%%%d.0%s%%s", (int) diout->width, DI_Lf);
        Snprintf2 (diout->blockFormat, sizeof (diout->blockFormat),
            "%%%d.1%s%%s", (int) diout->width, DI_Lf);
      }
    }

    if (diopts->dispBlockSize == DI_DISP_HR ||
        diopts->dispBlockSize == DI_DISP_HR_2)
    {
        ++diout->width;
    }

    if (diopts->csv_output) {
      Snprintf1 (diout->inodeFormat, sizeof (diout->inodeFormat),
          "%%%s", DI_LLu);
    } else {
      Snprintf2 (diout->inodeFormat, sizeof (diout->inodeFormat),
          "%%%d%s", (int) diout->inodeWidth, DI_LLu);
    }

    diskInfo = diData->diskInfo;
    if (diopts->printTotals)
    {
        int                  allocated;

        allocated = FALSE;
        if (diData->haspooledfs && ! diData->totsorted)
        {
          char tempSortType [DI_SORT_MAX + 1];
              /* in order to find the main pool entries,              */
              /* we must have the array sorted by special device name */
          strncpy (tempSortType, diopts->sortType, DI_SORT_MAX);
          strncpy (diopts->sortType, "s", DI_SORT_MAX);
          sortArray (diopts, diskInfo, diData->count, DI_TOT_SORT_IDX);
          strncpy (diopts->sortType, tempSortType, DI_SORT_MAX);
          diData->totsorted = TRUE;
        }

        for (i = 0; i < diData->count; ++i)
        {
            diDiskInfo_t    *dinfo;
            int             ispooled;
            int             startpool;
            int             poolmain;

            ispooled = FALSE;
            startpool = FALSE;
            poolmain = FALSE;
            dinfo = &(diskInfo [diskInfo [i].sortIndex[DI_TOT_SORT_IDX]]);

                /* is it a pooled filesystem type? */
            if (diData->haspooledfs &&
                (strcmp (dinfo->fsType, "zfs") == 0 ||
                 strcmp (dinfo->fsType, "advfs") == 0))
            {
              ispooled = TRUE;
              if (lastpoollen == 0 ||
                  strncmp (lastpool, dinfo->special, lastpoollen) != 0)
              {
                char        *ptr;

                strncpy (lastpool, dinfo->special, sizeof (lastpool));
                if (strcmp (dinfo->fsType, "advfs") == 0) {
                  ptr = strchr (lastpool, '#');
                  if (ptr != (char *) NULL) {
                    *ptr = '\0';
                  }
                }
                lastpoollen = strlen (lastpool);
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

            /* need to run addTotals for pooled filesystems */
            /* if a pooled filesystem is being displayed    */
            if (dinfo->doPrint ||
                (diData->disppooledfs && (inpool || poolmain)))
            {
              addTotals (dinfo, &totals, inpool);
            }
            else
            {
              if (debug > 2)
              {
                printf ("tot:%s:%s:skip\n", dinfo->special, dinfo->name);
              }
            }

            if (startpool)
            {
              inpool = TRUE;
            }
        } /* for each entry */
    } /* if the totals are to be printed */

    diskInfo = diData->diskInfo;
    if (strcmp (diopts->sortType, "n") != 0)
    {
      sortArray (diopts, diskInfo, diData->count, DI_MAIN_SORT_IDX);
    }

    for (i = 0; i < diData->count; ++i)
    {
      diDiskInfo_t        *dinfo;

      dinfo = &(diskInfo [diskInfo [i].sortIndex[DI_MAIN_SORT_IDX]]);
      if (debug > 5)
      {
        printf ("pdi:%s:%s:%d:\n", dinfo->name,
            getPrintFlagText ((int) dinfo->printFlag), dinfo->doPrint);
      }

      if (! dinfo->doPrint)
      {
        continue;
      }

      printInfo (dinfo, diopts, diout);
    }

    if (diopts->printTotals)
    {
      printInfo (&totals, diopts, diout);
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
printInfo (diDiskInfo_t *diskInfo, diOptions_t *diopts, diOutput_t *diout)
#else
printInfo (diskInfo, diopts, diout)
    diDiskInfo_t        *diskInfo;
    diOptions_t         *diopts;
    diOutput_t          *diout;
#endif
{
    _fs_size_t          used;
    _fs_size_t          totAvail;
    const char          *ptr;
    char                tfmt[2];
    int                 valid;
    _print_size_t       temp;
    int                 idx;
    int                 tidx;
    static char         percFormat [15];
    static int          percInit = FALSE;
    int                 first;

    first = TRUE;
    if (! percInit) {
      if (diopts->csv_output) {
        Snprintf1 (percFormat, sizeof(percFormat), "%%.0%s%%%%", DI_Lf);
      } else {
        if (diopts->posix_compat) {
          Snprintf1 (percFormat, sizeof(percFormat), DI_POSIX_PERC_FMT, DI_Lf);
        } else {
          Snprintf1 (percFormat, sizeof(percFormat), DI_PERC_FMT, DI_Lf);
        }
      }
      percInit = TRUE;
    }
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
              temp = (_print_size_t) diskInfo->totalBlocks;
              valid = TRUE;
              break;
          }

          case DI_FMT_BTOT_AVAIL:
          {
              temp = (_print_size_t) (diskInfo->totalBlocks -
                      (diskInfo->freeBlocks - diskInfo->availBlocks));
              valid = TRUE;
              break;
          }

          case DI_FMT_BUSED:
          {
              temp = (_print_size_t) (diskInfo->totalBlocks - diskInfo->freeBlocks);
              valid = TRUE;
              break;
          }

          case DI_FMT_BCUSED:
          {
              temp = (_print_size_t) (diskInfo->totalBlocks - diskInfo->availBlocks);
              valid = TRUE;
              break;
          }

          case DI_FMT_BFREE:
          {
              temp = (_print_size_t) diskInfo->freeBlocks;
              valid = TRUE;
              break;
          }

          case DI_FMT_BAVAIL:
          {
              temp = (_print_size_t) diskInfo->availBlocks;
              valid = TRUE;
              break;
          }
        }

        if (valid)
        {
          temp *= (_print_size_t) diskInfo->blockSize;
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
      tfmt[0] = *ptr;
      tfmt[1] = '\0';
      valid = strstr (DI_FMT_VALID_CHARS, tfmt) == NULL ? FALSE : TRUE;
      if (*ptr == DI_FMT_MOUNT_TIME && diout->maxMntTimeString == 0) {
        valid = FALSE;
      }
      if (valid && diopts->csv_output) {
        if (! first) {
          printf (",");
        }
        first = FALSE;
      }

      switch (*ptr)
      {
        case DI_FMT_MOUNT:
        case DI_FMT_MOUNT_FULL:
        {
          printf (diout->mountFormat, diskInfo->name);
          break;
        }

        case DI_FMT_BTOT:
        {
          printBlocks (diopts, diout,
              diskInfo->totalBlocks, diskInfo->blockSize, idx);
          break;
        }

        case DI_FMT_BTOT_AVAIL:
        {
          printBlocks (diopts, diout, diskInfo->totalBlocks -
              (diskInfo->freeBlocks - diskInfo->availBlocks),
              diskInfo->blockSize, idx);
          break;
        }

        case DI_FMT_BUSED:
        {
          printBlocks (diopts, diout,
              diskInfo->totalBlocks - diskInfo->freeBlocks,
              diskInfo->blockSize, idx);
          break;
        }

        case DI_FMT_BCUSED:
        {
          printBlocks (diopts, diout,
              diskInfo->totalBlocks - diskInfo->availBlocks,
              diskInfo->blockSize, idx);
          break;
        }

        case DI_FMT_BFREE:
        {
          printBlocks (diopts, diout,
               diskInfo->freeBlocks, diskInfo->blockSize, idx);
          break;
        }

        case DI_FMT_BAVAIL:
        {
          printBlocks (diopts, diout,
               diskInfo->availBlocks, diskInfo->blockSize, idx);
          break;
        }

        case DI_FMT_BPERC_NAVAIL:
        {
          used = diskInfo->totalBlocks - diskInfo->availBlocks;
          totAvail = diskInfo->totalBlocks;
          printPerc (used, totAvail, percFormat);
          break;
        }

        case DI_FMT_BPERC_USED:
        {
          used = diskInfo->totalBlocks - diskInfo->freeBlocks;
          totAvail = diskInfo->totalBlocks;
          printPerc (used, totAvail, percFormat);
          break;
        }

        case DI_FMT_BPERC_BSD:
        {
          used = diskInfo->totalBlocks - diskInfo->freeBlocks;
          totAvail = diskInfo->totalBlocks -
                  (diskInfo->freeBlocks - diskInfo->availBlocks);
          printPerc (used, totAvail, percFormat);
          break;
        }

        case DI_FMT_BPERC_AVAIL:
        {
          _fs_size_t          bfree;
          bfree = diskInfo->availBlocks;
          totAvail = diskInfo->totalBlocks;
          printPerc (bfree, totAvail, percFormat);
          break;
        }

        case DI_FMT_BPERC_FREE:
        {
          _fs_size_t          bfree;
          bfree = diskInfo->freeBlocks;
          totAvail = diskInfo->totalBlocks;
          printPerc (bfree, totAvail, percFormat);
          break;
        }

        case DI_FMT_ITOT:
        {
          printf (diout->inodeFormat, diskInfo->totalInodes);
          break;
        }

        case DI_FMT_IUSED:
        {
          printf (diout->inodeFormat, diskInfo->totalInodes - diskInfo->freeInodes);
          break;
        }

        case DI_FMT_IFREE:
        {
          printf (diout->inodeFormat, diskInfo->freeInodes);
          break;
        }

        case DI_FMT_IPERC:
        {
          used = diskInfo->totalInodes - diskInfo->availInodes;
          totAvail = diskInfo->totalInodes;
          printPerc (used, totAvail, percFormat);
          break;
        }

        case DI_FMT_SPECIAL:
        case DI_FMT_SPECIAL_FULL:
        {
          printf (diout->specialFormat, diskInfo->special);
          break;
        }

        case DI_FMT_TYPE:
        case DI_FMT_TYPE_FULL:
        {
          printf (diout->typeFormat, diskInfo->fsType);
          break;
        }

        case DI_FMT_MOUNT_OPTIONS:
        {
          if (diopts->csv_output) {
            printf ("\"");
          }
          printf (diout->optFormat, diskInfo->options);
          if (diopts->csv_output) {
            printf ("\"");
          }
          break;
        }

        case DI_FMT_MOUNT_TIME:
        {
          if (diout->maxMntTimeString > 0) {
            printf (diout->mTimeFormat, diskInfo->mountTime);
          }
          break;
        }

        default:
        {
          printf ("%c", *ptr);
          break;
        }
      }

      ++ptr;
      if (! diopts->csv_output && *ptr && valid)
      {
        printf (" ");
      }
    }

    printf ("\n");
}

static void
#if _proto_stdc
printBlocks (const diOptions_t *diopts, const diOutput_t *diout,
             _fs_size_t blocks, _fs_size_t blockSize, int idx)
#else
printBlocks (diopts, diout, blocks, blockSize, idx)
    const diOptions_t   *diopts;
    const diOutput_t    *diout;
    _fs_size_t          blocks;
    _fs_size_t          blockSize;
    int                 idx;
#endif
{
    _print_size_t   tdbs;
    _print_size_t   mult;
    _print_size_t   temp;
    char            *suffix;
    const char      *format;


    suffix = "";
    format = diout->blockFormat;
    tdbs = diopts->dispBlockSize;

    if (diopts->dispBlockSize == DI_DISP_HR)
    {
        temp = (_print_size_t) blocks * (_print_size_t) blockSize;
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

    mult = (_print_size_t) blockSize / tdbs;
    printf (format, (_print_size_t) blocks * mult, suffix);
}


static int
#if _proto_stdc
findDispSize (_print_size_t siz)
#else
findDispSize (siz)
    _print_size_t      siz;
#endif
{
    int         i;

    for (i = 0; i < (int) DI_SIZETAB_SIZE; ++i)
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
addTotals (const diDiskInfo_t *diskInfo, diDiskInfo_t *totals, int inpool)
#else
addTotals (diskInfo, totals, inpool)
    const diDiskInfo_t   *diskInfo;
    diDiskInfo_t   *totals;
    int            inpool;
#endif
{
    _print_size_t       mult;

    mult = (_print_size_t) diskInfo->blockSize / (_print_size_t) totals->blockSize;

    if (debug > 2)
    {
        printf ("tot:%s:%s:inp:%d:bs:%lld:mult:%Lf\n",
                    diskInfo->special, diskInfo->name,
                    inpool, diskInfo->blockSize, mult);
    }

    if (inpool)
    {
        if (debug > 2) {printf ("  tot:inpool:add total used\n"); }
        /* if in a pool of disks, add the total used to the totals also */
        totals->totalBlocks += (_fs_size_t) ((_print_size_t) (diskInfo->totalBlocks -
                diskInfo->freeBlocks) * mult);
        totals->totalInodes += diskInfo->totalInodes -
                diskInfo->freeInodes;
    }
    else
    {
        if (debug > 2) {printf ("  tot:not inpool:add all totals\n"); }
        totals->totalBlocks += (_fs_size_t) ((_print_size_t) diskInfo->totalBlocks * mult);
        totals->freeBlocks += (_fs_size_t) ((_print_size_t) diskInfo->freeBlocks * mult);
        totals->availBlocks += (_fs_size_t) ((_print_size_t) diskInfo->availBlocks * mult);
        totals->totalInodes += diskInfo->totalInodes;
        totals->freeInodes += diskInfo->freeInodes;
        totals->availInodes += diskInfo->availInodes;
    }
}

/*
 * processTitles
 *
 * Sets up the column format strings w/the appropriate defaults.
 * Loop through the format string and adjust the various column sizes.
 *
 * At the same time print the titles.
 *
 */

static void
#if _proto_stdc
processTitles (diOptions_t *diopts, diOutput_t *diout)
#else
processTitles (diopts, diout)
    diOptions_t   *diopts;
    diOutput_t    *diout;
#endif
{
    const char      *ptr;
    int             valid;
    Size_t          wlen;
    Size_t          *wlenptr;
    int             justification;
    char            *pstr;
    char            *fstr;
    Size_t          maxsize;
    char            tformat [30];
    char            ttitle [2];
    int             first;


    first = TRUE;
    if (diopts->printDebugHeader)
    {
        printf (DI_GT("di version %s Default Format: %s\n"),
                DI_VERSION, DI_DEFAULT_FORMAT);
    }

    ptr = diopts->formatString;

    while (*ptr)
    {
        valid = TRUE;
        wlen = 0;
        wlenptr = (Size_t *) NULL;
        fstr = (char *) NULL;
        maxsize = 0;
        justification = DI_JUST_LEFT;

        switch (*ptr)
        {
            case DI_FMT_MOUNT:
            {
                wlen = 15;
                wlenptr = &diout->maxMountString;
                pstr = "Mount";
                fstr = diout->mountFormat;
                maxsize = sizeof (diout->mountFormat);
                break;
            }

            case DI_FMT_MOUNT_FULL:
            {
                wlen = diout->maxMountString;
                if (wlen <= 0) {
                  wlen = 7;
                }
                wlenptr = &diout->maxMountString;
                fstr = diout->mountFormat;
                maxsize = sizeof (diout->mountFormat);
                if (diopts->posix_compat)
                {
                    pstr = "Mounted On";
                }
                else
                {
                    pstr = "Mount";
                }
                break;
            }

            case DI_FMT_BTOT:
            case DI_FMT_BTOT_AVAIL:
            {
                wlen = diout->width;
                wlenptr = &diout->width;
                justification = DI_JUST_RIGHT;
                pstr = diout->dispBlockLabel;
                break;
            }

            case DI_FMT_BUSED:
            case DI_FMT_BCUSED:
            {
                wlen = diout->width;
                wlenptr = &diout->width;
                justification = DI_JUST_RIGHT;
                pstr = "Used";
                break;
            }

            case DI_FMT_BFREE:
            {
                wlen = diout->width;
                wlenptr = &diout->width;
                justification = DI_JUST_RIGHT;
                pstr = "Free";
                break;
            }

            case DI_FMT_BAVAIL:
            {
                wlen = diout->width;
                wlenptr = &diout->width;
                justification = DI_JUST_RIGHT;
                if (diopts->posix_compat)
                {
                    pstr = "Available";
                }
                else
                {
                    pstr = "Avail";
                }
                break;
            }

            case DI_FMT_BPERC_NAVAIL:
            case DI_FMT_BPERC_USED:
            case DI_FMT_BPERC_BSD:
            {
                if (diopts->posix_compat)
                {
                    wlen = 9;
                    pstr = "Capacity";
                }
                else
                {
                    wlen = 6;
                    pstr = "%Used";
                }
                break;
            }

            case DI_FMT_BPERC_AVAIL:
            case DI_FMT_BPERC_FREE:
            {
                wlen = 5;
                pstr = "%Free";
                break;
            }

            case DI_FMT_ITOT:
            {
                justification = DI_JUST_RIGHT;
                wlen = diout->inodeWidth;
                wlenptr = &diout->inodeWidth;
                fstr = diout->inodeLabelFormat;
                maxsize = sizeof (diout->inodeLabelFormat);
                pstr = "Inodes";
                break;
            }

            case DI_FMT_IUSED:
            {
                justification = DI_JUST_RIGHT;
                wlen = diout->inodeWidth;
                wlenptr = &diout->inodeWidth;
                fstr = diout->inodeLabelFormat;
                maxsize = sizeof (diout->inodeLabelFormat);
                pstr = "Iused";
                break;
            }

            case DI_FMT_IFREE:
            {
                justification = DI_JUST_RIGHT;
                wlen = diout->inodeWidth;
                wlenptr = &diout->inodeWidth;
                fstr = diout->inodeLabelFormat;
                maxsize = sizeof (diout->inodeLabelFormat);
                pstr = "Ifree";
                break;
            }

            case DI_FMT_IPERC:
            {
                wlen = 6;
                pstr = "%Iused";
                break;
            }

            case DI_FMT_SPECIAL:
            {
                wlen = 18;
                wlenptr = &diout->maxSpecialString;
                fstr = diout->specialFormat;
                maxsize = sizeof (diout->specialFormat);
                pstr = "Filesystem";
                break;
            }

            case DI_FMT_SPECIAL_FULL:
            {
                wlen = diout->maxSpecialString;
                wlenptr = &diout->maxSpecialString;
                fstr = diout->specialFormat;
                maxsize = sizeof (diout->specialFormat);
                pstr = "Filesystem";
                break;
            }

            case DI_FMT_TYPE:
            {
                wlen = 7;
                wlenptr = &diout->maxTypeString;
                fstr = diout->typeFormat;
                maxsize = sizeof (diout->typeFormat);
                pstr = "fsType";
                break;
            }

            case DI_FMT_TYPE_FULL:
            {
                wlen = diout->maxTypeString;
                wlenptr = &diout->maxTypeString;
                fstr = diout->typeFormat;
                maxsize = sizeof (diout->typeFormat);
                pstr = "fs Type";
                break;
            }

            case DI_FMT_MOUNT_OPTIONS:
            {
                wlen = diout->maxOptString;
                wlenptr = &diout->maxOptString;
                fstr = diout->optFormat;
                maxsize = sizeof (diout->optFormat);
                pstr = "Options";
                break;
            }

            case DI_FMT_MOUNT_TIME:
            {
                pstr = "";
                if (diout->maxMntTimeString > 0) {
                  wlen = diout->maxMntTimeString;
                  wlenptr = &diout->maxMntTimeString;
                  fstr = diout->mTimeFormat;
                  maxsize = sizeof (diout->mTimeFormat);
                  pstr = "Mount Time";
                }
                break;
            }

            default:
            {
              if (diopts->printHeader) {
                printf ("%c", *ptr);
              }
              valid = FALSE;
              break;
            }
        }

        if (wlen > 0) {
          Size_t     ilen;
          Size_t     olen;
          Size_t     len;
          Size_t     tlen;
          const char *jstr;

          pstr = DI_GT (pstr);
          olen = (Size_t) strlen (pstr);
          ilen = (Size_t) istrlen (pstr);
/* fprintf (stderr, "%s: olen:%d wlen:%d ilen:%d\n", pstr, olen, wlen, ilen); */
          wlen = ilen > wlen ? ilen : wlen;
          len = wlen;
          tlen = len + olen - ilen;  /* for the title only */

          jstr = justification == DI_JUST_LEFT ? "-" : "";
          Snprintf3 (tformat, sizeof (tformat), "%%%s%d.%ds",
              jstr, (int) tlen, (int) tlen);

          if (diopts->printHeader) {
            if (diopts->csv_output) {
              if (! first) {
                printf (",");
              }
              first = FALSE;
            }
            if (diopts->csv_output) {
              ttitle[0] = *ptr;
              ttitle[1] = '\0';
              printf ("%s", ttitle);
            } else {
              printf (tformat, pstr);
            }
          }
/* fprintf (stderr, "%s: olen:%d wlen:%d ilen:%d len:%d tlen:%d %s\n", pstr, olen, wlen, ilen, len, tlen, tformat); */
          if (fstr != (char *) NULL) {
            if (diopts->csv_output) {
              strncpy (tformat, "%s", sizeof (tformat));
            }
            if (tlen != len) {
              if (! diopts->csv_output) {
                Snprintf3 (tformat, sizeof (tformat), "%%%s%d.%ds",
                    jstr, (int) len, (int) len);
              }
            }
            strncpy (fstr, tformat, maxsize);
          }
/* fprintf (stderr, "%s: %s\n", pstr, tformat); */
          if (wlenptr != (Size_t *) NULL) {
            *wlenptr = wlen;
          }
        }

        ++ptr;
        if (diopts->printHeader) {
          if (! diopts->csv_output && *ptr && valid)
          {
              printf (" ");
          }
        }
    }

    if (diopts->printHeader) {
      printf ("\n");
    }
}

/*
 * printPerc
 *
 * Calculate and print a percentage using the values and format passed.
 *
 */

static void
#if _proto_stdc
printPerc (_fs_size_t used, _fs_size_t totAvail, const char *format)
#else
printPerc (used, totAvail, format)
    _fs_size_t      used;
    _fs_size_t      totAvail;
    const char      *format;
#endif
{
    _print_size_t      perc;


    if (totAvail > 0L)
    {
        perc = (_print_size_t) used / (_print_size_t) totAvail;
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
checkFileInfo (
    diData_t *diData,
    int optidx,
    int argc,
    const char * const argv [])
#else
checkFileInfo (diData, optidx, argc, argv)
    diData_t            *diData;
    int                 optidx;
    int                 argc;
    const char          * const argv [];
#endif
{
    int                 rc;
    int                 i;
    int                 j;
    struct stat         statBuf;
    diOptions_t         *diopts;
    diDiskInfo_t        *diskInfo;


    rc = 0;

        /* turn everything off */
    for (j = 0; j < diData->count; ++j)
    {
      diDiskInfo_t        *dinfo;

      dinfo = &diData->diskInfo[j];
      if (dinfo->printFlag == DI_PRNT_OK)
      {
        dinfo->printFlag = DI_PRNT_IGNORE;
      }
    }

    diopts = &diData->options;

    diskInfo = diData->diskInfo;
    if (diData->haspooledfs && ! diData->totsorted)
    {
      char tempSortType [DI_SORT_MAX + 1];

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
                int             ispooled;
                int             startpool;
                int             poolmain;

                ispooled = FALSE;
                startpool = FALSE;
                poolmain = FALSE;

                dinfo = &(diskInfo [diskInfo [j].sortIndex[DI_TOT_SORT_IDX]]);

                if (found && ! inpool)
                {
                  break;
                }

                    /* is it a pooled filesystem type? */
                if (diData->haspooledfs &&
                    (strcmp (dinfo->fsType, "zfs") == 0 ||
                     strcmp (dinfo->fsType, "advfs") == 0))
                {
                  ispooled = TRUE;
                  if (lastpoollen == 0 ||
                      strncmp (lastpool, dinfo->special, lastpoollen) != 0)
                  {
                    strncpy (lastpool, dinfo->special, sizeof (lastpool));
                    lastpoollen = strlen (lastpool);
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

                if (poolmain)
                {
                  saveIdx = j;
                }

                if (found && inpool)
                {
                  dinfo = &(diskInfo [diskInfo [j].sortIndex[DI_TOT_SORT_IDX]]);
                  dinfo->printFlag = DI_PRNT_SKIP;
                  if (debug > 2)
                  {
                    printf ("  inpool B: also process %s %s\n",
                            dinfo->special, dinfo->name);
                  }
                }

                if (dinfo->st_dev != (__ulong) DI_UNKNOWN_DEV &&
                    (__ulong) statBuf.st_dev == dinfo->st_dev &&
                    ! dinfo->isLoopback)
                {
                  dinfo->printFlag = DI_PRNT_FORCE;
                  found = TRUE;
                  if (debug > 2)
                  {
                      printf ("file %s specified: found device %ld (%s %s)\n",
                              argv[i], dinfo->st_dev, dinfo->special, dinfo->name);
                  }
                  if (inpool)
                  {
                    int   k;
                    for (k = saveIdx; k < j; ++k)
                    {
                      dinfo = &(diskInfo [diskInfo [k].sortIndex[DI_TOT_SORT_IDX]]);
                      dinfo->printFlag = DI_PRNT_SKIP;
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
            fprintf (stderr, "stat: %s ", argv[i]);
            perror ("");
            rc = -1;
        }
    } /* for each file specified on command line */

    return rc;
}

/*
 * sortArray
 *
 */
static void
#if _proto_stdc
sortArray (diOptions_t *diopts, diDiskInfo_t *data, int count, int sidx)
#else
sortArray (diopts, data, count, sidx)
    diOptions_t     *diopts;
    diDiskInfo_t    *data;
    int             count;
    int             sidx;
#endif
{
  unsigned int  tempIndex;
  int           gap;
  register int  j;
  register int  i;

  if (count <= 1)
  {
    return;
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
      tempIndex = data[i].sortIndex[sidx];
      j = i - gap;

      while (j >= 0 && diCompare (diopts, data, data[j].sortIndex[sidx], tempIndex) > 0)
      {
        data[j + gap].sortIndex[sidx] = data[j].sortIndex[sidx];
        j -= gap;
      }

      j += gap;
      if (j != i)
      {
        data[j].sortIndex[sidx] = tempIndex;
      }
    }
  }
}

static int
#if _proto_stdc
diCompare (const diOptions_t *diopts, const diDiskInfo_t *data,
           unsigned int idx1, unsigned int idx2)
#else
diCompare (diopts, data, idx1, idx2)
    const diOptions_t     *diopts;
    const diDiskInfo_t    *data;
    unsigned int    idx1;
    unsigned int    idx2;
#endif
{
    int             rc;
    int             sortOrder;
    const char            *ptr;
    const diDiskInfo_t    *d1;
    const diDiskInfo_t    *d2;

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
            rc = strcoll (d1->name, d2->name);
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
            rc = strcoll (d1->special, d2->special);
            rc *= sortOrder;
            break;
        }

        case DI_SORT_TYPE:
        {
            rc = strcoll (d1->fsType, d2->fsType);
            rc *= sortOrder;
            break;
        }

        case DI_SORT_AVAIL:
        {
            rc = (int) ((d1->availBlocks * d1->blockSize) -
                    (d2->availBlocks * d2->blockSize));
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

static int
#if _proto_stdc
getDiskSpecialInfo (diData_t *diData)
#else
getDiskSpecialInfo (diData)
    diData_t            *diData;
#endif
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
            dinfo->sp_dev = (__ulong) statBuf.st_dev;
            dinfo->sp_rdev = (__ulong) statBuf.st_rdev;
              /* Solaris's loopback device is "lofs"            */
              /* linux loopback device is "none"                */
              /* linux has rdev = 0                             */
              /* DragonFlyBSD's loopback device is "null"       */
              /* DragonFlyBSD has rdev = -1                     */
              /* solaris is more consistent; rdev != 0 for lofs */
              /* solaris makes sense.                           */
            if ((strcmp (dinfo->fsType, "lofs") == 0 && dinfo->sp_rdev != 0) ||
                 strcmp (dinfo->fsType, "null") == 0 ||
                 strcmp (dinfo->fsType, "none") == 0) {
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

static void
#if _proto_stdc
checkDiskInfo (diData_t *diData, int hasLoop)
#else
checkDiskInfo (diData, hasLoop)
    diData_t    *diData;
    int         hasLoop;
#endif
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
          dinfo->doPrint = (char) diopts->printAllColumns;
        }

            /* Solaris reports a cdrom as having no free blocks,   */
            /* no available.  Their df doesn't always work right!  */
            /* -1 is returned.                                     */
        if (debug > 5)
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
          if (debug > 5)
          {
#if _siz_long_long >= 8
              printf ("chk: %s total: %lld\n", dinfo->name,
                      dinfo->totalBlocks);
#else
              printf ("chk: %s total: %ld\n", dinfo->name,
                      dinfo->totalBlocks);
#endif
          }

          if ((_s_fs_size_t) dinfo->totalBlocks <= 0L)
          {
            dinfo->printFlag = DI_PRNT_IGNORE;
            dinfo->doPrint = (char) diopts->printAllColumns;
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
              dinfo->doPrint = (char) diopts->printAllColumns;
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

static void
#if _proto_stdc
checkDiskQuotas (diData_t *diData)
#else
checkDiskQuotas (diData)
    diData_t            *diData;
#endif
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
    diqinfo.blockSize = dinfo->blockSize;
    diquota (&diqinfo);

    if (debug > 2) {
      printf ("quota: %s limit: %lld\n", dinfo->name, diqinfo.limit);
      printf ("quota:   tot: %lld\n", dinfo->totalBlocks);
      printf ("quota: %s used: %lld\n", dinfo->name, diqinfo.used);
      printf ("quota:   avail: %lld\n", dinfo->availBlocks);
    }

    /* remap block size if it changed (e.g. nfs) */
    if (diqinfo.blockSize != dinfo->blockSize) {
      dinfo->totalBlocks *= dinfo->blockSize;
      dinfo->freeBlocks *= dinfo->blockSize;
      dinfo->availBlocks *= dinfo->blockSize;
      dinfo->blockSize = diqinfo.blockSize;
      dinfo->totalBlocks /= dinfo->blockSize;
      dinfo->freeBlocks /= dinfo->blockSize;
      dinfo->availBlocks /= dinfo->blockSize;
    }

    if (diqinfo.limit != 0 &&
            diqinfo.limit < dinfo->totalBlocks) {
      dinfo->totalBlocks = diqinfo.limit;
      tsize = diqinfo.limit - diqinfo.used;
      if ((_s_fs_size_t) tsize < 0) {
        tsize = 0;
      }
      if (tsize < dinfo->availBlocks) {
        dinfo->availBlocks = tsize;
        dinfo->freeBlocks = tsize;
        if (debug > 2) {
          printf ("quota: using quota for: total free avail\n");
        }
      } else if (tsize > dinfo->availBlocks && tsize < dinfo->freeBlocks) {
        dinfo->freeBlocks = tsize;
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

static void
#if _proto_stdc
getMaxFormatLengths (diData_t *diData)
#else
getMaxFormatLengths (diData)
    diData_t            *diData;
#endif
{
    int             i;
    unsigned int    len;
    diOutput_t      *diout;

    diout = &diData->output;

        /* this loop gets the max string lengths */
    for (i = 0; i < diData->count; ++i)
    {
        diDiskInfo_t        *dinfo;

        dinfo = &diData->diskInfo[i];
        if (dinfo->doPrint)
        {
            if (diData->haspooledfs &&
                (strcmp (dinfo->fsType, "zfs") == 0 ||
                 strcmp (dinfo->fsType, "advfs") == 0))
            {
              diData->disppooledfs = TRUE;
            }

            len = (unsigned int) strlen (dinfo->name);
            if (len > diout->maxMountString)
            {
                diout->maxMountString = len;
            }

            len = (unsigned int) strlen (dinfo->special);
            if (len > diout->maxSpecialString)
            {
                diout->maxSpecialString = len;
            }

            len = (unsigned int) strlen (dinfo->fsType);
            if (len > diout->maxTypeString)
            {
                diout->maxTypeString = len;
            }

            len = (unsigned int) strlen (dinfo->options);
            if (len > diout->maxOptString)
            {
                diout->maxOptString = len;
            }

            len = (unsigned int) strlen (dinfo->mountTime);
            if (len > diout->maxMntTimeString)
            {
                diout->maxMntTimeString = len;
            }
        } /* if we are printing this item */
    } /* for all disks */
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
        checkZone (dinfo, &diData->zoneInfo, diopts->printAllColumns);
#endif

        if (strcmp (dinfo->fsType, "zfs") == 0 ||
            strcmp (dinfo->fsType, "advfs") == 0)
        {
          diData->haspooledfs = TRUE;
        }

        if (dinfo->printFlag == DI_PRNT_OK)
        {
              /* don't bother w/this check is all flag is set. */
          if (! diopts->printAllColumns)
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

/*
 * usage
 */

static void
#if _proto_stdc
usage (void)
#else
usage ()
#endif
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


struct pa_tmp {
  diData_t        *diData;
  diOptions_t     *diopts;
  diOutput_t      *diout;
  char            *dbsstr;
  Size_t          dbsstr_sz;
};

static void
#if _proto_stdc
processOptions (const char *arg, char *valptr)
#else
processOptions (arg, valptr)
    const char *arg;
    char *valptr;
#endif
{
  struct pa_tmp     *padata;

  padata = (struct pa_tmp *) valptr;
  if (strcmp (arg, "-a") == 0) {
    padata->diopts->printAllColumns = TRUE;
    strncpy (padata->diData->zoneInfo.zoneDisplay, "all", MAXPATHLEN);
  } else if (strcmp (arg, "--help") == 0 || strcmp (arg, "-?") == 0) {
    usage();
    exit (0);
  } else if (strcmp (arg, "-P") == 0) {
    if (strcmp (padata->dbsstr, "k") != 0) /* don't override -k option */
    {
      strncpy (padata->dbsstr, "512", padata->dbsstr_sz);
    }
    padata->diopts->formatString = DI_POSIX_FORMAT;
    padata->diopts->posix_compat = TRUE;
    padata->diopts->csv_output = FALSE;
  } else if (strcmp (arg, "--si") == 0) {
    padata->diopts->baseDispSize = DI_VAL_1000;
    padata->diopts->baseDispIdx = DI_DISP_1000_IDX;
    strncpy (padata->dbsstr, "H", padata->dbsstr_sz);
  } else if (strcmp (arg, "--version") == 0) {
    printf (DI_GT("di version %s  Default Format: %s\n"), DI_VERSION, DI_DEFAULT_FORMAT);
    exit (0);
  } else {
    fprintf (stderr, "di_panic: bad option setup\n");
  }

  return;
}

static void
#if _proto_stdc
processOptionsVal (const char *arg, char *valptr, char *value)
#else
processOptionsVal (arg, valptr, value)
    const char  *arg;
    char        *valptr;
    char        *value;
#endif
{
  struct pa_tmp     *padata;

  padata = (struct pa_tmp *) valptr;

  if (strcmp (arg, "-B") == 0) {
    if (isdigit ((int) (*value)))
    {
      padata->diopts->baseDispSize = atof (value);
      padata->diopts->baseDispIdx = DI_DISP_1000_IDX; /* unknown, really */
      if (padata->diopts->baseDispSize == DI_VAL_1024)
      {
        padata->diopts->baseDispIdx = DI_DISP_1024_IDX;
      }
    }
    else if (strcmp (value, "k") == 0)
    {
      padata->diopts->baseDispSize = DI_VAL_1024;
      padata->diopts->baseDispIdx = DI_DISP_1024_IDX;
    }
    else if (strcmp (value, "d") == 0 ||
        strcmp (value, "si") == 0)
    {
      padata->diopts->baseDispSize = DI_VAL_1000;
      padata->diopts->baseDispIdx = DI_DISP_1000_IDX;
    }
  } else if (strcmp (arg, "-I") == 0) {
    parseList (&padata->diData->includeList, value);
  } else if (strcmp (arg, "-s") == 0) {
    strncpy (padata->diopts->sortType, value,
        sizeof (padata->diopts->sortType));
      /* for backwards compatibility                       */
      /* reverse by itself - change to reverse mount point */
    if (strcmp (padata->diopts->sortType, "r") == 0)
    {
        strncpy (padata->diopts->sortType, "rm", DI_SORT_MAX);
    }
        /* add some sense to the sort order */
    if (strcmp (padata->diopts->sortType, "t") == 0)
    {
        strncpy (padata->diopts->sortType, "tm", DI_SORT_MAX);
    }
  } else if (strcmp (arg, "-x") == 0) {
    parseList (&padata->diData->ignoreList, value);
  } else if (strcmp (arg, "-X") == 0) {
    debug = atoi (value);
    padata->diopts->printDebugHeader = TRUE;
    padata->diopts->printTotals = TRUE;
    padata->diopts->printHeader = TRUE;
    padata->diout->width = 10;
    padata->diout->inodeWidth = 10;
  } else {
    fprintf (stderr, "di_panic: bad option setup\n");
  }

  return;
}

static int
#if _proto_stdc
processArgs (int argc,
             const char * const argv [],
             diData_t *diData,
             char *dbsstr,
             Size_t dbsstr_sz)
#else
processArgs (argc, argv, diData, dbsstr, dbsstr_sz)
    int             argc;
    const char      * const argv [];
    diData_t        *diData;
    char            *dbsstr;
    Size_t          dbsstr_sz;
#endif
{
  int           i;
  int           optidx;
  diOptions_t   *diopts;
  diOutput_t    *diout;
  struct pa_tmp padata;

    /* the really old compilers don't have automatic initialization */
  static getoptn_opt_t opts[] = {
    { "-A",     GETOPTN_STRPTR,
        NULL  /*&diopts->formatString*/,
        0,
        (void *) DI_ALL_FORMAT },
    { "-a",     GETOPTN_FUNC_BOOL,
        NULL  /*&padata*/,
        0,
        NULL  /*processOptions*/ },
    { "--all",  GETOPTN_ALIAS,
        (void *) "-a",
        0,
        NULL },
    { "-B",     GETOPTN_FUNC_VALUE,
        NULL  /*&padata*/,
        0,
        NULL  /*processOptionsVal*/ },
    { "-b",     GETOPTN_ALIAS,
        (void *) "-B",
        0,
        NULL },
    { "--block-size",   GETOPTN_ALIAS,      /* 5 */
        (void *) "-B",
        0,
        NULL },
    { "-c",     GETOPTN_BOOL,
        NULL  /*&diopts->csv_output*/,
        0  /*sizeof(diopts->csv_output)*/,
        NULL },
    { "--csv-output", GETOPTN_ALIAS,
        (void *) "-c",
        0,
        NULL },
    { "-d",     GETOPTN_STRING,
        NULL  /*dbsstr*/,
        0  /*dbsstr_sz*/,
        NULL },
    { "--display-size",     GETOPTN_ALIAS,
        (void *) "-d",
        0,
        NULL },
    { "-f",     GETOPTN_STRPTR,             /* 10 */
        NULL  /*&diopts->formatString*/,
        0,
        NULL },
    { "--format-string",    GETOPTN_ALIAS,
        (void *) "-f",
        0,
        NULL },
    { "-F",     GETOPTN_ALIAS,
        (void *) "-I",
        0,
        NULL },
    { "-g",     GETOPTN_STRING,
        NULL  /*dbsstr*/,
        0  /*dbsstr_sz*/,
        (void *) "g" },
    { "-h",     GETOPTN_STRING,
        NULL  /*dbsstr*/,
        0  /*dbsstr_sz*/,
        (void *) "h" },
    { "-H",     GETOPTN_STRING,             /* 15 */
        NULL  /*dbsstr*/,
        0  /*dbsstr_sz*/,
        (void *) "H" },
    { "--help", GETOPTN_FUNC_BOOL,
        NULL,
        0,
        NULL  /*processOptions*/ },
    { "--human-readable",   GETOPTN_ALIAS,
        (void *) "-H",
        0,
        NULL },
    { "-?",     GETOPTN_FUNC_BOOL,
        NULL,
        0,
        NULL  /*processOptions*/ },
    { "-i",     GETOPTN_ALIAS,
        (void *) "-x",
        0,
        NULL },
    { "-I",     GETOPTN_FUNC_VALUE,         /* 20 */
        NULL  /*&padata*/,
        0,
        NULL  /*processOptionsVal*/ },
    { "--inodes",GETOPTN_IGNORE,
        NULL,
        0,
        NULL },
    { "-k",     GETOPTN_STRING,
        NULL  /*dbsstr*/,
        0  /*dbsstr_sz*/,
        (void *) "k" },
    { "-l",     GETOPTN_BOOL,
        NULL  /*&diopts->localOnly*/,
        0  /*sizeof (diopts->localOnly)*/,
        NULL },
    { "--local",GETOPTN_ALIAS,
        (void *) "-l",
        0,
        NULL },
    { "-L",     GETOPTN_BOOL,               /* 25 */
        NULL  /*&diopts->excludeLoopback*/,
        0  /*sizeof (diopts->excludeLoopback)*/,
        NULL },
    { "-m",     GETOPTN_STRING,
        NULL  /*dbsstr*/,
        0  /*dbsstr_sz*/,
        (void *) "m" },
    { "-n",     GETOPTN_BOOL,
        NULL  /*&diopts->printHeader*/,
        0  /*sizeof (diopts->printHeader)*/,
        NULL },
    { "--no-sync",  GETOPTN_IGNORE,
        NULL,
        0,
        NULL },
    { "-P",     GETOPTN_FUNC_BOOL,
        NULL  /*&padata*/,
        0,
        NULL  /*processOptions*/ },
    { "--portability",  GETOPTN_ALIAS,      /* 30 */
        (void *) "-P",
        0,
        NULL },
    { "--print-type",   GETOPTN_IGNORE,
        NULL,
        0,
        NULL },
    { "-q",     GETOPTN_BOOL,
        NULL  /*&diopts->quota_check*/,
        0  /*sizeof (diopts->quota_check)*/,
        NULL },
    { "-s",     GETOPTN_FUNC_VALUE,
        NULL  /*&padata*/,
        0,
        NULL  /*processOptionsVal*/ },
    { "--si",   GETOPTN_FUNC_BOOL,
        NULL  /*&padata*/,
        0,
        NULL  /*processOptions*/ },
    { "--sync", GETOPTN_IGNORE,             /* 35 */
        NULL,
        0,
        NULL },
    { "-t",     GETOPTN_BOOL,
        NULL  /*&diopts->printTotals*/,
        0  /*sizeof (diopts->printTotals)*/,
        NULL },
    { "--total",GETOPTN_ALIAS,
        (void *) "-t",
        0,
        NULL },
    { "--type", GETOPTN_ALIAS,
        (void *) "-I",
        0,
        NULL },
    { "-v",     GETOPTN_IGNORE,
        NULL,
        0,
        NULL },
    { "--version", GETOPTN_FUNC_BOOL,       /* 40 */
        NULL,
        0,
        NULL  /*processOptions*/ },
    { "-w",     GETOPTN_SIZET,
        NULL  /*&diout->width*/,
        0  /*sizeof (diout->width)*/,
        NULL },
    { "-W",     GETOPTN_SIZET,
        NULL  /*&diout->inodeWidth*/,
        0  /*sizeof (diout->inodeWidth)*/,
        NULL },
    { "-x",     GETOPTN_FUNC_VALUE,
        NULL  /*&padata*/,
        0,
        NULL  /*processOptionsVal*/ },
    { "--exclude-type",     GETOPTN_ALIAS,
        (void *) "-x",
        0,
        NULL },
    { "-X",     GETOPTN_FUNC_VALUE,
        NULL  /*&padata*/,
        0,
        NULL  /*processOptionsVal*/ },
    { "-z",     GETOPTN_STRING,             /* 46 */
        NULL  /*&diData->zoneInfo.zoneDisplay*/,
        0  /*sizeof (diData->zoneInfo.zoneDisplay)*/,
        NULL },
    { "-Z",     GETOPTN_STRING,
        NULL  /*&diData->zoneInfo.zoneDisplay*/,
        0  /*sizeof (diData->zoneInfo.zoneDisplay)*/,
        (void *) "all" }
  };
  static int dbsids[] = { 8, 13, 14, 15, 22, 26 };
  static int paidb[] = { 1, 16, 18, 29, 34, 40, };
  static int paidv[] = { 3, 20, 33, 43, 45 };

  diopts = &diData->options;
  diout = &diData->output;

    /* this is seriously gross, but the old compilers don't have    */
    /* automatic array initialization                               */
  opts[0].valptr = (void *) &diopts->formatString;   /* -A */
  opts[6].valptr = (void *) &diopts->csv_output;     /* -c */
  opts[6].valsiz = sizeof (diopts->csv_output);
  opts[10].valptr = (void *) &diopts->formatString;  /* -f */
  opts[23].valptr = (void *) &diopts->localOnly;     /* -l */
  opts[23].valsiz = sizeof (diopts->localOnly);
  opts[25].valptr = (void *) &diopts->excludeLoopback; /* -L */
  opts[25].valsiz = sizeof (diopts->excludeLoopback);
  opts[27].valptr = (void *) &diopts->printHeader;   /* -n */
  opts[27].valsiz = sizeof (diopts->printHeader);
  opts[32].valptr = (void *) &diopts->quota_check;    /* -q */
  opts[32].valsiz = sizeof (diopts->quota_check);
  opts[36].valptr = (void *) &diopts->printTotals;    /* -t */
  opts[36].valsiz = sizeof (diopts->printTotals);
  opts[41].valptr = (void *) &diout->width;          /* -w */
  opts[41].valsiz = sizeof (diout->width);
  opts[42].valptr = (void *) &diout->inodeWidth;     /* -W */
  opts[42].valsiz = sizeof (diout->inodeWidth);
  opts[46].valptr = (void *) diData->zoneInfo.zoneDisplay;  /* -z */
  opts[46].valsiz = sizeof (diData->zoneInfo.zoneDisplay);
  opts[47].valptr = (void *) diData->zoneInfo.zoneDisplay;  /* -Z */
  opts[47].valsiz = sizeof (diData->zoneInfo.zoneDisplay);

  for (i = 0; i < (int) (sizeof (dbsids) / sizeof (int)); ++i) {
    opts[dbsids[i]].valptr = (void *) dbsstr;
    opts[dbsids[i]].valsiz = dbsstr_sz;
  }
  for (i = 0; i < (int) (sizeof (paidb) / sizeof (int)); ++i) {
    opts[paidb[i]].valptr = (void *) &padata;
    opts[paidb[i]].value2 = (void *) processOptions;
  }
  for (i = 0; i < (int) (sizeof (paidv) / sizeof (int)); ++i) {
    opts[paidv[i]].valptr = (void *) &padata;
    opts[paidv[i]].value2 = (void *) processOptionsVal;
  }

  padata.diData = diData;
  padata.diopts = diopts;
  padata.diout = diout;
  padata.dbsstr = dbsstr;
  padata.dbsstr_sz = dbsstr_sz;

  optidx = getoptn (GETOPTN_LEGACY, argc, argv,
       sizeof (opts) / sizeof (getoptn_opt_t), opts);

  if (diopts->csv_output) {
    diopts->printTotals = FALSE;
  }

  return optidx;
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
    unsigned int len;

    dstr = strdup (str);
    if (dstr == (char *) NULL)
    {
        fprintf (stderr, "strdup failed in parseList() (1).  errno %d\n", errno);
        exit (1);
    }

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
    list->list = (char **) _realloc ((char *) list->list,
            (Size_t) list->count * sizeof (char *));
    if (list->list == (char **) NULL)
    {
        fprintf (stderr, "malloc failed in parseList() (2).  errno %d\n", errno);
        exit (1);
    }

    ptr = dstr;
    for (i = ocount; i < ncount; ++i)
    {
        len = (unsigned int) strlen (ptr);
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


static void
#if _proto_stdc
setDispBlockSize (char *ptr, diOptions_t *diopts, diOutput_t *diout)
#else
setDispBlockSize (ptr, diopts, diout)
    char            *ptr;
    diOptions_t     *diopts;
    diOutput_t      *diout;
#endif
{
  unsigned int    len;
  _print_size_t   val;
  char            *tptr;
  static char     tempbl [15];
  char            ttempbl [15];

  if (isdigit ((int) (*ptr)))
  {
      val = atof (ptr);
  }
  else
  {
      val = 1.0;
  }

  tptr = ptr;
  len = (unsigned int) strlen (ptr);
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
          diout->dispBlockLabel = "Size";
          break;
      }

      case 'H':
      {
          val = DI_DISP_HR_2;
          diout->dispBlockLabel = "Size";
          break;
      }

      default:
      {
          if (strncmp (ptr, "HUMAN", (Size_t) 5) == 0)
          {
              val = DI_DISP_HR;
          }
          else
          {
              /* some unknown string value */
            idx = DI_ONE_MEG;
          }
          break;
      }
    }

    if (idx >= 0)
    {
      if (len > 1)
      {
        ++tptr;
        if (*tptr == 'B')
        {
           diopts->baseDispSize = DI_VAL_1000;
           diopts->baseDispIdx = DI_DISP_1000_IDX;
        }
      }

      if (val == 1.0)
      {
        diout->dispBlockLabel = dispTable [idx].disp [diopts->baseDispIdx];
      }
      else
      {
        Snprintf1 (ttempbl, sizeof (tempbl), "%%.0%s %%s", DI_Lf);
        Snprintf2 (tempbl, sizeof (tempbl), ttempbl,
            val, DI_GT (dispTable [idx].disp [diopts->baseDispIdx]));
        diout->dispBlockLabel = tempbl;
      }
      val *= dispTable [idx].size;
    } /* known size multiplier */
  }
  else
  {
    int         i;
    int         ok;

    ok = 0;
    for (i = 0; i < (int) DI_DISPTAB_SIZE; ++i)
    {
      if (val == dispTable [i].size)
      {
        diout->dispBlockLabel = dispTable [i].disp [diopts->baseDispIdx];
        ok = 1;
        break;
      }
    }

    if (ok == 0)
    {
      Snprintf1 (ttempbl, sizeof (ttempbl), "%%.0%sb", DI_Lf);
      Snprintf1 (tempbl, sizeof (tempbl), ttempbl, val);
      diout->dispBlockLabel = tempbl;
    }
  }  /* some oddball block size */

  if (diopts->posix_compat && val == 512)
  {
    diout->dispBlockLabel = "512-blocks";
  }
  if (diopts->posix_compat && val == 1024)
  {
    diout->dispBlockLabel = "1024-blocks";
  }

  diopts->dispBlockSize = val;
}

/* for debugging */
static const char *
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
        pf == DI_PRNT_OUTOFZONE ? "outofzone" :
        pf == DI_PRNT_FORCE ? "force" :
        pf == DI_PRNT_SKIP ? "skip" : "unknown";
}

static Size_t
#if _proto_stdc
istrlen (const char *str)
#else
istrlen (str)
    const char *str;
#endif
{
  Size_t            len;
#if _lib_mbrlen && _enable_nls
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
    if ((int) mlen <= 0) {
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
