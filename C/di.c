/*
$Id$
$Source$
Copyright 1994-2002 Brad Lanam, Walnut Creek, CA
*/

/*
 * di.c
 *
 *   Copyright 1994-1999 Brad Lanam,  Walnut Creek, CA
 *
 *  Warning: Do not replace your systems 'df' command with this program.
 *           You will in all likelihood break your installation procedures.
 *
 *  Usage: di -AafntwWx [file [...]]
 *            -A   : print all fields (used for debugging)
 *            -a   : print all mounted devices; normally, those
 *                   with 0 total blocks are not printed.  e.g.
 *                   /dev/proc, /dev/fd.
 *            -d x : size to print blocks in (p, k, m, g, t, P, <n>)
 *            -f x : use format string <x>
 *            -g   : -dg
 *            -h   : usage
 *            -H   : "human readable"
 *            -i x : ignore filesystem type(s) x, where x is a comma
 *                   separated list.
 *            -I x : include only filesystem type(s) x, where x is a
 *                   separated list.
 *            -l   : local filesystems only
 *            -m   : -dm
 *            -n   : don't print header
 *            -s t : sort type (n, s, S, (r))
 *            -t   : print totals
 *            -w n : use <n> for the block print width (default 8).
 *            -W n : use <n> for the inode print width (default 7).
 *            -x n : debug level <n>
 *
 *  Display sizes:
 *      p - posix (512 bytes)
 *      k - kilobytes
 *      m - megabytes
 *      g - gigabytes
 *      t - terabytes
 *      P - petabytes
 *      h - "human readable"
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
 *             [ (tot - (free - avail) ]
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
 *  System V.4 `/usr/bin/df -v` Has format: msbuf1 (w/-d512 option: 512 byte blocks)
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
 *  HISTORY:
 *     11 Jul 2002 bll
 *          Snprintf stuff
 *          cleanup.
 *     14 apr 2002 bll
 *          Rewrite ignore/include list stuff to allow multiple
 *          command line options.
 *          Fix sort command line option.  Add sort size.
 *          Add petas.
 *     24 feb 2000 bll
 *          Updated for BeOS.  This required changes to support c++
 *          compilation (ansi style functions).  Fixes for linux.
 *     17 dec 99 bll
 *          Added sys/fs_types.h (Digital Unix (tru64)).
 *     3 jan 99 bll
 *          Finalize changes for metaconfig.
 *          Always get stat() for disk device.
 *          Remove duplicate disks from display.
 *          Add NetBSD, Unicos, IRIX changes
 *     8 sep 97 bll
 *          Solaris large file version; AIX typo fix
 *     15 aug 95 bll
 *          Ignore will check the file system type beforehand if possible.
 *          Fixed local mount stuff for osf1.
 *     9 aug 95 bll
 *          Changed totals to use double; the totals can now be displayed with
 *          any wanted block size; sizes > one k have a single decimal point.
 *          -d option specifies block size.
 *          -l option for local file systems only.
 *     7 aug 95 bll
 *          convex fix.
 *     29 jul 95 bll
 *          Fixed bsdi variants.  The fragment size wasn't being supported.
 *     22 jul 95 bll
 *          fix problem with display widths in conjunction with ignore/include
 *     20 feb 95 bll
 *          added ignore/include file system type lists.
 *     8 jan 95 bll
 *          Added FreeBsd 2.x
 *     6 dec 94 bll
 *          sco nfs 'nothing' fix.
 *     28 nov 94 bll
 *          bsdi [bag@clipper.cs.kiev.ua (Andrey Blochintsev)]
 *     24 nov 94 bll
 *          Added FreeBsd - like OSF/1, but w/o nice mount name table.
 *     1 may 94 bll
 *          removed ill conceived mount options stuff.  Added AIX
 *          (Erik O'Shaughnessy eriko@risc.austin.ibm.com).
 *          Added coherent.
 *     9 apr 94 bll
 *          T format from Bill Davidsen. SunOS file system type.
 *          mount options.
 *     7 apr 94 bll
 *          cdrom under solaris returning -1 in addition to -2.
 *          Changed the test to test for any negative value.
 *    5 apr 94 pm
 *         Filesystem type info, and whether its read-write, readonly
 *         etc is in the mntent structure.  Added code to support it.
 *                  [Pat Myrto <pat@rwing.uucp>]
 *    25 mar 94 bll
 *          sun had f_bfree instead of f_bavail!
 *          Xenix, linux.
 *          -w, -W options, -B option, fixed width for -f M, -f S.
 *          Usage.  Allow other characters in format string.
 *     4 mar 94 bll
 *          -f B, bug fixes. bcopy.
 *          Solaris cdrom reports total, not free or avail.
 *          Allow command line specification of file names.
 *     3 mar 94 bll
 *          support for OSF/1 and ULTRIX, -F, -f M, -f S
 *                  [mogul@wrl.dec.com (Jeffrey Mogul)]
 *     3 mar 94   bll
 *          Allow command line specification of filename (s).
 *          Sort output.  Test of nonexistent return code removed.
 *     1 mar 94   bll
 *          # of inodes can be -1L if unreportable.
 *                  [jjb@jagware.bcc.com (J.J.Bailey)]
 *                  [Mark Neale <mark@edscom.demon.co.uk>]
 *          di_getDiskInfo () returning garbage
 *                  [costales@ICSI.Berkeley.EDU (Bryan Costales)]
 *                  [Mark Neale <mark@edscom.demon.co.uk>]
 *          pyramid #ifdefs
 *                  [vogelke@c-17igp.wpafb.af.mil (Contr Karl Vogel)]
 *          name must be maxpathlen
 *                  [Mark Neale <mark@edscom.demon.co.uk>]
 *          error prints/compile warnings.
 *                  [Mark Neale <mark@edscom.demon.co.uk>]
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

extern int di_lib_debug;

#if _npt_getenv
  extern char *getenv _((char *));
#endif

#if ! defined (_lib_errno)
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
#define DI_FMT_BPERC_AVAIL     'p'
#define DI_FMT_BPERC_FREE      '1'
#define DI_FMT_BPERC_BSD       '2'
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

#define DI_ALL_FORMAT           "MTS\n\tIO\n\tbuf1\n\tbcvp\n\tBuv2\n\tiUFP"

#define DI_HALF_K               512.0
#define DI_ONE_K                1024.0
#define DI_ONE_MEG              1048576.0
#define DI_ONE_GIG              1073241824.0
#define DI_ONE_TERA             1099511627776.0
#define DI_ONE_PETA             1125899906842624.0
#define DI_ONE_EXA              1152921504606846976.0
#define DI_DISP_HR              0

   /* you may want to change some of these values.  Be sure to change all */
   /* related entries.                                                    */
/* #define DI_DEFAULT_FORMAT      "mbuvpiUFP" */
#if ! defined (DI_DEFAULT_FORMAT)
# define DI_DEFAULT_FORMAT      "smbuvpT"
#endif
#if _lib_mnt_time
# define DI_DEF_MOUNT_FORMAT    "MST\n\tI\n\tO"
#else
# define DI_DEF_MOUNT_FORMAT    "MST\n\tO"
#endif
#define DI_PERC_FMT             "%3.0f%% "
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

static int          debug = { 0 };
static __ulong      flags = { 0 };
static int          sortType = { DI_SORT_NAME };
static int          sortOrder = { DI_SORT_ASCENDING };

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
static double       dispBlockSize = { DI_ONE_MEG };
static int          sizeTableCount = { 6 };

sizeTable_t sizeTable [] =
{
    { 0,           DI_ONE_K,    1,           blockFormatNR, " " },
    { DI_ONE_K,    DI_ONE_MEG,  DI_ONE_K,    blockFormat,   "k" },
    { DI_ONE_MEG,  DI_ONE_GIG,  DI_ONE_MEG,  blockFormat,   "M" },
    { DI_ONE_GIG,  DI_ONE_TERA, DI_ONE_GIG,  blockFormat,   "G" },
    { DI_ONE_TERA, DI_ONE_PETA, DI_ONE_TERA, blockFormat,   "T" },
    { DI_ONE_PETA, DI_ONE_EXA,  DI_ONE_PETA, blockFormat,   "P" }
};


static void cleanup             _((di_DiskInfo *, iList *, iList *));
static void printDiskInfo       _((di_DiskInfo *, int));
static void printInfo           _((di_DiskInfo *));
static void printBlocks          _((_fs_size_t, _fs_size_t));
static void addTotals           _((di_DiskInfo *, di_DiskInfo *));
static void printTitle          _((void));
static void printPerc           _((_fs_size_t, _fs_size_t, char *));
static void sortArray           _((char *, Size_t, int, DI_SORT_FUNC));
static int  diCompare           _((void *, void *));
static void getDiskStatInfo     _((di_DiskInfo *, int));
static void getDiskSpecialInfo  _((di_DiskInfo *, int));
static void checkFileInfo       _((di_DiskInfo *, int, int, int, char *[]));
static void preCheckDiskInfo    _((di_DiskInfo *, iList *, iList *, int));
static void checkDiskInfo       _((di_DiskInfo *, iList *, int));
static void usage               _((void));
static void processArgs         _((int, char *[], iList *, iList *));
static void parseList           _((iList *, char *));
static void checkIgnoreList     _((di_DiskInfo *, iList *));
static void checkIncludeList    _((di_DiskInfo *, iList *));
static void setDispBlockSize    _((char *));

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
    int                 diCount = { 0 };
    iList               ignoreList = { 0, (char **) NULL };
    iList               includeList = { 0, (char **) NULL };
    char                *ptr;


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
        dispBlockSize = DI_HALF_K;
    }

        /* bsd df */
    if ((ptr = getenv ("BLOCKSIZE")) != (char *) NULL)
    {
        setDispBlockSize (ptr);
    }

        /* gnu df */
    if ((ptr = getenv ("DF_BLOCK_SIZE")) != (char *) NULL)
    {
        setDispBlockSize (ptr);
    }

    processArgs (argc, argv, &ignoreList, &includeList);

    if (debug > 0)
    {
        printf ("di ver %s\n", DI_VERSION);
    }

    if (di_getDiskEntries (&diskInfo, &diCount) < 0)
    {
        cleanup (diskInfo, &ignoreList, &includeList);
        exit (1);
    }

    preCheckDiskInfo (diskInfo, &ignoreList, &includeList, diCount);
    if (optind < argc)
    {
        getDiskStatInfo (diskInfo, diCount);
        checkFileInfo (diskInfo, diCount, optind, argc, argv);
    }
    di_getDiskInfo (&diskInfo, &diCount);
    getDiskSpecialInfo (diskInfo, diCount);
    checkDiskInfo (diskInfo, &includeList, diCount);
    printDiskInfo (diskInfo, diCount);

    cleanup (diskInfo, &ignoreList, &includeList);
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
cleanup (di_DiskInfo *diskInfo, iList *ignoreList, iList *includeList)
#else
cleanup (diskInfo, ignoreList, includeList)
    di_DiskInfo     *diskInfo;
    iList           *ignoreList;
    iList           *includeList;
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


    memset ((char *) &totals, '\0', sizeof (di_DiskInfo));
    totals.blockSize = 8192;
    strncpy (totals.name, GT("Total"), DI_NAME_LEN);
    totals.printFlag = DI_PRNT_OK;

    if ((flags & DI_F_NO_HEADER) != DI_F_NO_HEADER)
    {
        printTitle ();
    }

    if (sortType != DI_SORT_NONE)
    {
        sortArray ((char *) diskInfo, sizeof (di_DiskInfo), diCount, diCompare);
    }

    for (i = 0; i < diCount; ++i)
    {
        if (( (flags & DI_F_ALL) == DI_F_ALL &&
                diskInfo [i].printFlag != DI_PRNT_BAD) ||
                diskInfo [i].printFlag == DI_PRNT_OK)
        {
            printInfo (&diskInfo [i]);
        }

        if ((flags & DI_F_TOTAL) == DI_F_TOTAL &&
                (flags & DI_F_NO_HEADER) != DI_F_NO_HEADER)
        {
                /* only total the disks that make sense! */
                /* don't add memory filesystems to totals. */
            if (diskInfo [i].printFlag == DI_PRNT_OK &&
                strcmp (diskInfo [i].fsType, "tmpfs") != 0 &&
                strcmp (diskInfo [i].fsType, "mfs") != 0 &&
                diskInfo [i].isReadOnly == FALSE)
            {
                addTotals (&diskInfo [i], &totals);
            }
        }
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
                printBlocks (diskInfo->totalBlocks, diskInfo->blockSize);
                break;
            }

            case DI_FMT_BTOT_AVAIL:
            {
                printBlocks (diskInfo->totalBlocks -
                    (diskInfo->freeBlocks - diskInfo->availBlocks),
                    diskInfo->blockSize);
                break;
            }

            case DI_FMT_BUSED:
            {
                printBlocks (diskInfo->totalBlocks - diskInfo->freeBlocks,
                            diskInfo->blockSize);
                break;
            }

            case DI_FMT_BCUSED:
            {
                printBlocks (diskInfo->totalBlocks - diskInfo->availBlocks,
                            diskInfo->blockSize);
                break;
            }

            case DI_FMT_BFREE:
            {
                printBlocks (diskInfo->freeBlocks, diskInfo->blockSize);
                break;
            }

            case DI_FMT_BAVAIL:
            {
                printBlocks (diskInfo->availBlocks, diskInfo->blockSize);
                break;
            }

            case DI_FMT_BPERC_AVAIL:
            {
                used = diskInfo->totalBlocks - diskInfo->availBlocks;
                totAvail = diskInfo->totalBlocks;
                printPerc (used, totAvail, DI_PERC_FMT);
                break;
            }

            case DI_FMT_BPERC_FREE:
            {
                used = diskInfo->totalBlocks - diskInfo->freeBlocks;
                totAvail = diskInfo->totalBlocks;
                printPerc (used, totAvail, DI_PERC_FMT);
                break;
            }

            case DI_FMT_BPERC_BSD:
            {
                used = diskInfo->totalBlocks - diskInfo->freeBlocks;
                totAvail = diskInfo->totalBlocks -
                        (diskInfo->freeBlocks - diskInfo->availBlocks);
                printPerc (used, totAvail, DI_PERC_FMT);
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
printBlocks (_fs_size_t blocks, _fs_size_t blockSize)
#else
printBlocks (blocks, blockSize)
    _fs_size_t         blocks;
    _fs_size_t         blockSize;
#endif
{
    double          tdbs;
    double          mult;
    double          temp;
    char            *suffix;
    char            *format;
    int             i;


    suffix = "";
    format = blockFormat;
    tdbs = dispBlockSize;

    if (dispBlockSize == DI_DISP_HR)
    {
        for (i = 0; i < sizeTableCount; ++i)
        {
            temp = (double) blocks * (double) blockSize;
            if (temp >= sizeTable [i].low && temp < sizeTable [i].high)
            {
                tdbs = sizeTable [i].dbs;
                format = sizeTable [i].format;
                suffix = sizeTable [i].suffix;
                break;
            }
        }
    }
    mult = (double) blockSize / tdbs;
    printf (format, (double) blocks * mult, suffix);
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
    char                *tptr;
    int                 valid;
    char                tbuff [20];

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
                printf (mountFormat, GT("Mount"));
                break;
            }

            case DI_FMT_BTOT:
            case DI_FMT_BTOT_AVAIL:
            {
                if (dispBlockSize == DI_ONE_K)
                {
                    tptr = GT("Kbytes");
                }
                else if (dispBlockSize == DI_ONE_MEG)
                {
                    tptr = GT("Megs");
                }
                else if (dispBlockSize == DI_ONE_GIG)
                {
                    tptr = GT("Gigs");
                }
                else if (dispBlockSize == DI_ONE_TERA)
                {
                    tptr = GT("Teras");
                }
                else if (dispBlockSize == DI_ONE_PETA)
                {
                    tptr = GT("Petas");
                }
                else if (dispBlockSize == DI_DISP_HR)
                {
                    tptr = GT("Size");
                }
                else if (dispBlockSize == DI_HALF_K)
                {
                    tptr = "512b";
                }
                else
                {
                    Snprintf (SPF(tbuff, sizeof (tbuff), "%6.0f"),
                              dispBlockSize);
                    tptr = tbuff;
                }
                printf (blockLabelFormat, tptr);
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
                printf (blockLabelFormat, GT("Avail"));
                break;
            }

            case DI_FMT_BPERC_AVAIL:
            case DI_FMT_BPERC_FREE:
            case DI_FMT_BPERC_BSD:
            {
                printf (DI_PERC_LBL_FMT, GT("%used"));
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
                printf (DI_PERC_LBL_FMT, GT("%used"));
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


static void
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
    int                 i;
    int                 j;
    struct stat         statBuf;


    for (j = 0; j < diCount; ++j)
    {
        diskInfo [j].printFlag = DI_PRNT_IGNORE;
    }

    for (i = optidx; i < argc; ++i)
    {
        if (stat (argv [i], &statBuf) == 0)
        {
            for (j = 0; j < diCount; ++j)
            {
                if (diskInfo [j].printFlag != DI_PRNT_BAD &&
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
        }
    } /* for each file specified on command line */
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

    tempData = (char *) malloc ((unsigned) dataSize);
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
        if (stat (diskInfo [i].special, &statBuf) == 0)
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
        if (diskInfo [i].printFlag == DI_PRNT_IGNORE)
        {
            if (debug > 2)
            {
                printf ("chk: skipping:%s\n", diskInfo [i].name);
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

        if ((flags & DI_F_ALL) != DI_F_ALL)
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
            if (diskInfo [i].totalBlocks <= 0L &&
                diskInfo [i].printFlag != DI_PRNT_BAD)
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

        /* this loop sets duplicate entries to be ignored. */
    for (i = 0; i < diCount; ++i)
    {
            /* don't need to bother checking real partitions */
            /* don't bother if already ignored               */
        if (diskInfo [i].sp_rdev != 0 &&
            (diskInfo [i].printFlag == DI_PRNT_OK ||
            ((flags & DI_F_ALL) == DI_F_ALL &&
            diskInfo [i].printFlag != DI_PRNT_BAD)))
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
                printf ("dup: chk: %s %ld %ld dup: %d\n", diskInfo [i].name,
                    sp_dev, sp_rdev, dupCount);
            }

            for (j = 0; dupCount > 0 && j < diCount; ++j)
            {
                if (diskInfo [j].sp_rdev == 0 &&
                    diskInfo [j].sp_dev == sp_rdev)
                {
                    diskInfo [j].printFlag = DI_PRNT_IGNORE;
                    if (debug > 2)
                    {
                        printf ("chk: ignore: duplicate: %s\n",
                                diskInfo [j].name);
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

        /* this loop gets the max string lengths */
    for (i = 0; i < diCount; ++i)
    {
        if (diskInfo [i].printFlag == DI_PRNT_OK || (flags & DI_F_ALL) == DI_F_ALL)
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

    if (dispBlockSize == DI_DISP_HR)
    {
        --width;
    }

    if (dispBlockSize != DI_DISP_HR && dispBlockSize <= DI_ONE_K)
    {
        Snprintf (SPF(blockFormat, sizeof (blockFormat), "%%%d.0f%%s"), width);
    }
    else
    {
        Snprintf (SPF(blockFormatNR, sizeof (blockFormatNR), "%%%d.0f%%s"),
                  width);
        Snprintf (SPF(blockFormat, sizeof (blockFormat), "%%%d.1f%%s"), width);
    }

    if (dispBlockSize == DI_DISP_HR)
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
 *
 */

static void
#if _proto_stdc
preCheckDiskInfo (di_DiskInfo *diskInfo, iList *ignoreList,
        iList *includeList, int diCount)
#else
preCheckDiskInfo (diskInfo, ignoreList, includeList, diCount)
    di_DiskInfo         *diskInfo;
    iList               *ignoreList;
    iList               *includeList;
    int                 diCount;
#endif
{
    int           i;

    for (i = 0; i < diCount; ++i)
    {
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
    printf (GT("Usage: di [-ant] [-d display-size] [-f format] [-i ignore-fstyp-list]\n"));
    printf (GT("       [-I include-fstyp-list] [file [...]]\n"));
    printf (GT("   -a   : print all mounted devices\n"));
    printf (GT("   -d x : size to print blocks in (p - posix (512), k - kbytes,\n"));
    printf (GT("          m - megabytes, g - gigabytes, t - terabytes, h - human readable).\n"));
    printf (GT("   -f x : use format string <x>\n"));
    printf (GT("   -i x : ignore file system types in <x>\n"));
    printf (GT("   -I x : include only file system types in <x>\n"));
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
processArgs (int argc, char *argv [], iList *ignoreList, iList *includeList)
#else
processArgs (argc, argv, ignoreList, includeList)
    int         argc;
    char        *argv [];
    iList       *ignoreList;
    iList       *includeList;
#endif
{
    int         ch;


    while ((ch = getopt (argc, argv, "Aad:f:ghHi:I:Flmns:tw:W:x:")) != -1)
    {
        switch (ch)
        {
            case 'A':
            {
                formatString = DI_ALL_FORMAT;
                flags |= DI_F_ALL | DI_F_DEBUG_HDR | DI_F_TOTAL;
                flags &= ~ DI_F_NO_HEADER;
                width = 10;
                inodeWidth = 10;
                break;
            }

            case 'a':
            {
                flags |= DI_F_ALL;
                break;
            }

            case 'd':
            {
                switch (*optarg)
                {
                    case 'p':
                    {
                        dispBlockSize = DI_HALF_K;
                        break;
                    }

                    case 'k':
                    case 'K':
                    {
                        dispBlockSize = DI_ONE_K;
                        break;
                    }

                    case 'm':
                    case 'M':
                    {
                        dispBlockSize = DI_ONE_MEG;
                        break;
                    }

                    case 'g':
                    case 'G':
                    {
                        dispBlockSize = DI_ONE_GIG;
                        break;
                    }

                    case 't':
                    {
                        dispBlockSize = DI_ONE_TERA;
                        break;
                    }

                    case 'P':
                    {
                        dispBlockSize = DI_ONE_PETA;
                        break;
                    }

                    case 'h':
                    case 'H':
                    {
                        dispBlockSize = DI_DISP_HR;
                        break;
                    }

                    default:
                    {
                        if (isdigit ((int) (*optarg)))
                        {
                            dispBlockSize = atof (optarg);
                        }
                        break;
                    }
                }
                break;
            }

            case 'f':
            {
                formatString = optarg;
                break;
            }

            case 'g':
            {
                dispBlockSize = DI_ONE_GIG;
                break;
            }

            case 'h':
            case '?':
            {
                usage ();
                exit (1);
            }

            case 'H':
            {
                dispBlockSize = DI_DISP_HR;
                break;
            }

            case 'i':
            {
                parseList (ignoreList, optarg);
                break;
            }

            case 'I':
            {
                parseList (includeList, optarg);
                break;
            }

            case 'l':
            {
                flags |= DI_F_LOCAL_ONLY;
                break;
            }

            case 'm':
            {
                dispBlockSize = DI_ONE_MEG;
                break;
            }

            case 'n':
            {
                flags |= DI_F_NO_HEADER;
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

            case 'x':
            {
                debug = atoi (optarg);
                di_lib_debug = atoi (optarg);
                break;
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
        cleanup ((di_DiskInfo *) NULL, (iList *) NULL, (iList *) NULL);
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
        cleanup ((di_DiskInfo *) NULL, (iList *) NULL, (iList *) NULL);
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
            cleanup ((di_DiskInfo *) NULL, list, (iList *) NULL);
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
                diskInfo->printFlag = DI_PRNT_IGNORE;
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
                break;
            }
            else
            {
                diskInfo->printFlag = DI_PRNT_IGNORE;
                if (debug > 2)
                {
                    printf ("chkinc: ! include: fstype %s match: %s\n", ptr,
                            diskInfo->name);
                }
            }
        }
    } /* if an include list was specified */
}

static void
#if _proto_stdc
setDispBlockSize (char *ptr)
#else
setDispBlockSize (*ptr)
    char    *ptr;
#endif
{
    if (isdigit ((int) (*ptr)))
    {
        dispBlockSize = (_fs_size_t) atof (ptr);
    }
    else
    {
        switch (tolower (*ptr))
        {
            case 'k':
            {
                dispBlockSize = DI_ONE_K;
                break;
            }

            case 'm':
            {
                dispBlockSize = DI_ONE_MEG;
                break;
            }

            case 'g':
            {
                dispBlockSize = DI_ONE_GIG;
                break;
            }

            case 't':
            {
                dispBlockSize = DI_ONE_TERA;
                break;
            }

            case 'p':
            {
                dispBlockSize = DI_ONE_PETA;
                break;
            }
        }
    }
}
