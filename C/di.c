#ifndef lint
static char    di_c_sccsid [] =
"@(#)di.c	1.21";
static char    di_c_rcsid [] =
"$Id$";
static char    di_c_source [] =
"$Source$";
#endif

/*
 * di.c
 *
 *   Copyright 1994, 1995 Brad Lanam,  Walnut Creek, CA
 *
 *  Warning: Do not replace your systems 'df' command with this program.
 *           You will in all likelihood break your installation procedures.
 *
 *  Usage: di -AafntwWx [file [...]]
 *            -A   : print all fields (used for debugging)
 *            -a   : print all mounted devices; normally, those
 *                   with 0 total blocks are not printed.  e.g.
 *                   /dev/proc, /dev/fd.
 *            -d x : size to print blocks in (p, k, m, g, <n>)
 *            -f x : use format string <x>
 *            -i x : ignore filesystem type(s) x, where x is a comma
 *                   separated list.
 *            -I x : include only filesystem type(s) x, where x is a
 *                   separated list.
 *            -l   : local filesystems only
 *            -n   : don't print header
 *            -s t : sort type
 *            -t   : print totals
 *            -w n : use <n> for the block print width (default 8).
 *            -W n : use <n> for the inode print width (default 7).
 *            -x n : debug level <n>
 *
 *  All values are reported in K (1024 bytes).
 *
 *  Sort types (by name is default):
 *      n - none (mount order)
 *      s - special
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
 *  The default format string for this program is: mbuvpiUFP
 *
 *  The environment variable "DIFMT" may be set to the desired format
 *  string.
 *
 *  Note that for filesystems that do not have (S512K fs) or systems (SysV.3)
 *  that do not report available blocks, the number of available blocks is
 *  equal to the number of free blocks.
 *
 *  HISTORY:
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
 *          getDiskInfo () returning garbage
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

#if defined (__STDC__) || defined (M_XENIX)
# define PROTO(x)         x
#endif

#if ! defined (PROTO)
# define PROTO(x)         ()
#endif

#include <stdio.h>
#if ! defined (HAS_NO_STDLIB_H)
# include <stdlib.h>
#else
extern char *getenv PROTO ((char *));
#endif
#include <errno.h>
#if ! defined (HAS_NO_MALLOC_H)
# include <malloc.h>
#endif
#if ! defined (HAS_NO_MEMORY_H)
# include <memory.h>
#else
# define memcpy(dst, src, cnt)     (bcopy((src), (dst), (cnt)), dst)
# if defined (NEED_BCOPY_DEFS)
extern int bcopy (char *, char *, int);
# endif
#endif
#if defined (HAS_STRINGS_H)
# include <strings.h>
#else
# include <string.h>
#endif
#if ! defined (HAS_NO_UNISTD_H)
# include <unistd.h>
#endif
#if defined (NEED_TIME_H)
# include <time.h>
#endif
#include <ctype.h>
#include <sys/types.h>
#if ! defined (HAS_NO_SYSPARAM_H)
# include <sys/param.h>
#endif
#include <sys/stat.h>

#if defined (_POSIX_SOURCE) && defined (NEED_ULONG_DEF)
typedef ulong_t u_long;
#endif

#if defined (HAS_MNTTAB)
# include <sys/mnttab.h>
#endif
#if defined (HAS_SVR3_MNTTAB)
# include <mnttab.h>
# if ! defined (M_XENIX)
#  include <limits.h>
#  ifdef PATH_MAX
#   define MAXPATHLEN        PATH_MAX
#  endif
#  ifdef _POSIX_PATH_MAX
#   define MAXPATHLEN        _POSIX_PATH_MAX
#  endif
# else
#  define MAXPATHLEN         LPNMAX
# endif
#endif

#if defined (HAS_MNTENT)
# define NO_MOUNT_TIME
# include <mntent.h>
#endif

#if defined (HAS_GETMNTINFO)
# define NO_MOUNT_TIME
# include <sys/mount.h>
# if ! defined (HAS_NO_FSTYPES_H)
#  include <sys/fs_types.h>
# endif
#endif
#if defined (HAS_GETMNT)
# include <limits.h>
# include <sys/mount.h>
# include <sys/fs_types.h>
#endif
#if defined (HAS_MNTCTL)          /* AIX v3.2 */
# include <sys/mntctl.h>
# include <sys/vmount.h>
# include <sys/statfs.h>
# include <fshelp.h>
# define FSMAGIC 5      /* base AIX configuration has 5 file systems */
#endif /* HAS_MNTCTL */

#if defined (HAS_STATVFS)
# include <sys/statvfs.h>
#endif
#if defined (HAS_SYSFS)
# include <sys/fstyp.h>
# define TYPE_LEN          FSTYPSZ
#endif
#if defined (HAS_STATFS)
# include <sys/vfs.h>
# if defined (NEED_STATFS_DEFS)
extern int statfs PROTO ((char *, struct statfs *));
# endif
#endif
#if defined (HAS_SYSV_STATFS)
# include <sys/statfs.h>
#endif

#if defined (HAS_VFSTAB)
# include <sys/vfstab.h>
# if ! defined (TYPE_LEN)
#  define TYPE_LEN         FSTYPSZ
# endif
# define VFSTAB_FILE       "/etc/vfstab"
#endif

#if defined (HAS_GETDISKFREESPACE)
# include <windows.h>            /* ms-dos */
#endif

#if ! defined (TYPE_LEN)
# define TYPE_LEN          16
#endif

#if ! defined (UBSIZE)
# define UBSIZE            512
#endif

#if defined (MOUNTED)
# define MOUNT_FILE        MOUNTED
#else
# if defined (MNTTAB)
#  define MOUNT_FILE       MNTTAB
# else
#  define MOUNT_FILE       "/etc/mnttab"
# endif
#endif

#if ! defined (TRUE)
# define TRUE             1
#endif
#if ! defined (FALSE)
# define FALSE            0
#endif

#if defined (HAS_BZERO)
# if defined (NEED_BZERO_DEFS)
extern int bzero (char *, int);
# endif
#  define memset(s,c,n)    (bzero ((s), (n)), s)
#endif

#define F_ALL               0x00000001
#define F_LOCAL_ONLY        0x00000002
#define F_TOTAL             0x00000010
#define F_NO_HEADER         0x00000020
#define F_DEBUG_HDR         0x00000040

    /* mount information */
#define FMT_MOUNT           'm'
#define FMT_MOUNT_FULL      'M'
#define FMT_SPECIAL         's'
#define FMT_SPECIAL_FULL    'S'
#define FMT_TYPE            't'
#define FMT_TYPE_FULL       'T'

    /* disk information */
#define FMT_BTOT            'b'
#define FMT_BTOT_AVAIL      'B'
#define FMT_BUSED           'u'
#define FMT_BCUSED          'c'
#define FMT_BFREE           'f'
#define FMT_BAVAIL          'v'
#define FMT_BPERC_AVAIL     'p'
#define FMT_BPERC_FREE      '1'
#define FMT_BPERC_BSD       '2'
#define FMT_ITOT            'i'
#define FMT_IUSED           'U'
#define FMT_IFREE           'F'
#define FMT_IPERC           'P'
#define FMT_MOUNT_TIME      'I'
#define FMT_MOUNT_OPTIONS   'O'

#define DI_IGNORE           0
#define DI_OK               1
#define DI_BAD              2

#define DI_SORT_NONE        0
#define DI_SORT_NAME        1
#define DI_SORT_SPECIAL     2

#define DI_SORT_ASCENDING   1
#define DI_SORT_DESCENDING  -1

#define DI_UNKNOWN_DEV      -1L
#define DI_LIST_SEP         ","

#define ALL_FORMAT          "MTS\n\tIO\n\tbuf1\n\tbcvp\n\tBuv2\n\tiUFP"

#define HALF_K              512.0
#define ONE_K               1024.0
#define ONE_MEG             1048576.0
#define ONE_GIG             1073241824.0

#if ! defined (MAXPATHLEN)
# define MAXPATHLEN         255
#endif
#define SPEC_NAME_LEN       MAXPATHLEN
#define OPT_LEN             MAXPATHLEN
#define MNT_TIME_LEN        24

#define RETRY_COUNT         5
#define MAX_REMOTE_TYPES    20

   /* you may want to change some of these values.  Be sure to change all */
   /* related entries.                                                    */
/* #define DEFAULT_FORMAT      "mbuvpiUFP" */
#define DEFAULT_FORMAT      "smbuvpT"
#if defined (NO_MOUNT_TIME)
# define DEF_MOUNT_FORMAT    "MSTO"
#else
# define DEF_MOUNT_FORMAT    "MSTIO"
#endif
#define PERC_FMT            "%3.0f%% "
#define PERC_LBL_FMT        "%5s"
#define NAME_LEN            MAXPATHLEN
#define FSTYPE_FMT          "%-7.7s"
#define MOUNT_FMT           "%-15.15s"
#define SPEC_FMT            "%-18.18s"

#define UNKNOWN_FSTYPE      "Unknown fstyp %.2d"

typedef unsigned long _ulong;
typedef int (*SORT_FUNC) PROTO ((char *, char *));

#if defined (NEED_GETOPT_DEFS)
extern char    *optarg;
extern int     optind;
    /* SCO will throw up on this...just comment it out */
extern int     getopt PROTO ((int, char *[], const char *));
#endif
#if defined (NEED_ERRNO_DEFS)
extern int     errno;
#endif

#if defined (_LARGEFILE_SOURCE) && _FILE_OFFSET_BITS == 64
typedef unsigned long long _fs_size_t;
typedef long long _s_fs_size_t;
#else
typedef unsigned long _fs_size_t;
typedef long _s_fs_size_t;
#endif

typedef struct
{
    double          totalBlocks;
    double          freeBlocks;
    double          availBlocks;
    _fs_size_t      totalInodes;
    _fs_size_t      freeInodes;
    _fs_size_t      availInodes;
    long            st_dev;                      /* disk device number   */
    char            printFlag;                   /* do we want to print  */
                                                 /* this entry?          */
    char            isLocal;                     /* is this mount point  */
                                                 /* local?               */
    char            name [NAME_LEN + 1];         /* mount point          */
    char            special [SPEC_NAME_LEN + 1]; /* special device name  */
    char            fsType [TYPE_LEN + 1];       /* type of file system  */
    char            options [OPT_LEN + 1];
    char            mountTime [MNT_TIME_LEN + 1];
} DiskInfo;

static DiskInfo     *diskInfo = { (DiskInfo *) NULL };
static int          diCount = { 0 };
static int          debug = { 0 };
static _ulong       flags = { 0 };
static int          sortType = { DI_SORT_NAME };
static int          sortOrder = { DI_SORT_ASCENDING };
static char         *formatString = { DEFAULT_FORMAT };
static char         mountFormat [20];
 static char         specialFormat [20];
static char         typeFormat [20];
static char         optFormat [20];
static char         mTimeFormat [20];
static int          width = { 8 };
static int          inodeWidth = { 7 };
static char         blockFormat [20];
static char         blockLabelFormat [20];
static char         inodeFormat [20];
static char         inodeLabelFormat [20];
static char         **ignoreList = { (char **) NULL };
static char         **includeList = { (char **) NULL };
static double       dispBlockSize = { ONE_MEG };
static int          remoteFileSystemCount = { 0 };
static char         remoteFileSystemTypes [MAX_REMOTE_TYPES][TYPE_LEN];


static void cleanup             PROTO ((void));
static void printDiskInfo       PROTO ((void));
static void printInfo           PROTO ((DiskInfo *));
static void addTotals           PROTO ((DiskInfo *, DiskInfo *));
static void printTitle          PROTO ((void));
static void printPerc           PROTO ((double, double, char *));
static char *Realloc            PROTO ((char *, long));
static void sortArray           PROTO ((char *, int, int, SORT_FUNC));
static int  diCompare           PROTO ((char *, char *));
static void getDiskStatInfo     PROTO ((void));
static void printFileInfo       PROTO ((int, int, char *[]));
static void checkDiskInfo       PROTO ((void));
static void usage               PROTO ((void));
static void processArgs         PROTO ((int, char *[]));
static void parseList           PROTO ((char ***, char *));
static void checkIgnoreList     PROTO ((DiskInfo *));
static void checkIncludeList    PROTO ((DiskInfo *));

static int  getDiskEntries      PROTO ((void));
static void getDiskInfo         PROTO ((void));

int
main (argc, argv)
    int                 argc;
    char                *argv [];
{
    char                *ptr;


    ptr = argv [0] + strlen (argv [0]) - 2;
    if (memcmp (ptr, MPROG, 2) == 0)
    {
        formatString = DEF_MOUNT_FORMAT;
    }
    else    /* don't use DIFMT env var if running mi. */
    {
        if ((ptr = getenv ("DIFMT")) != (char *) NULL)
        {
            formatString = ptr;
        }
    }

    processArgs (argc, argv);
    if (debug > 0)
    {
        printf ("di ver $Revision$\n");
    }

    if (getDiskEntries () < 0)
    {
        cleanup ();
        exit (1);
    }

    getDiskInfo ();
    checkDiskInfo ();
    if (optind < argc)
    {
        getDiskStatInfo ();
        printFileInfo (optind, argc, argv);
    }
    else
    {
        printDiskInfo ();
    }

    cleanup ();
    exit (0);
}

/*
 * cleanup
 *
 * free up allocated memory
 *
 */

static void
cleanup ()
{
    char        **lptr;


    if (diskInfo != (DiskInfo *) NULL)
    {
        free ((char *) diskInfo);
    }

    if (ignoreList != (char **) NULL)
    {
        lptr = ignoreList;
        while (*lptr != (char *) NULL)
        {
            free ((char *) *lptr);
            ++lptr;
        }
        free ((char *) ignoreList);
    }

    if (includeList != (char **) NULL)
    {
        lptr = includeList;
        while (*lptr != (char *) NULL)
        {
            free ((char *) *lptr);
            ++lptr;
        }
        free ((char *) includeList);
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
printDiskInfo ()
{
    int                 i;
    DiskInfo            totals;


    memset ((char *) &totals, '\0', sizeof (DiskInfo));
    strcpy (totals.name, "Total");
    totals.printFlag = DI_OK;

    if ((flags & F_NO_HEADER) != F_NO_HEADER)
    {
        printTitle ();
    }

    if (sortType != DI_SORT_NONE)
    {
        sortArray ((char *) diskInfo, sizeof (DiskInfo), diCount, diCompare);
    }

    for (i = 0; i < diCount; ++i)
    {
        if (( (flags & F_ALL) == F_ALL && diskInfo [i].printFlag != DI_BAD) ||
                diskInfo [i].printFlag == DI_OK)
        {
            printInfo (&diskInfo [i]);
            addTotals (&diskInfo [i], &totals);
        }
    }

    if ((flags & F_TOTAL) == F_TOTAL && (flags & F_NO_HEADER) != F_NO_HEADER)
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
printInfo (diskInfo)
    DiskInfo            *diskInfo;
{
    double              used;
    double              totAvail;
    char                *ptr;
    int                 valid;


    ptr = formatString;
    while (*ptr)
    {
        valid = TRUE;

        switch (*ptr)
        {
            case FMT_MOUNT:
            {
                printf (MOUNT_FMT, diskInfo->name);
                break;
            }

            case FMT_MOUNT_FULL:
            {
                printf (mountFormat, diskInfo->name);
                break;
            }

            case FMT_BTOT:
            {
                printf (blockFormat, diskInfo->totalBlocks);
                break;
            }

            case FMT_BTOT_AVAIL:
            {
                printf (blockFormat, diskInfo->totalBlocks -
                        (diskInfo->freeBlocks - diskInfo->availBlocks));
                break;
            }

            case FMT_BUSED:
            {
                printf (blockFormat, diskInfo->totalBlocks -
                        diskInfo->freeBlocks);
                break;
            }

            case FMT_BCUSED:
            {
                printf (blockFormat, diskInfo->totalBlocks - diskInfo->availBlocks);
                break;
            }

            case FMT_BFREE:
            {
                printf (blockFormat, diskInfo->freeBlocks);
                break;
            }

            case FMT_BAVAIL:
            {
                printf (blockFormat, diskInfo->availBlocks);
                break;
            }

            case FMT_BPERC_AVAIL:
            {
                used = diskInfo->totalBlocks - diskInfo->availBlocks;
                totAvail = diskInfo->totalBlocks;
                printPerc (used, totAvail, PERC_FMT);
                break;
            }

            case FMT_BPERC_FREE:
            {
                used = diskInfo->totalBlocks - diskInfo->freeBlocks;
                totAvail = diskInfo->totalBlocks;
                printPerc (used, totAvail, PERC_FMT);
                break;
            }

            case FMT_BPERC_BSD:
            {
                used = diskInfo->totalBlocks - diskInfo->freeBlocks;
                totAvail = diskInfo->totalBlocks -
                        (diskInfo->freeBlocks - diskInfo->availBlocks);
                printPerc (used, totAvail, PERC_FMT);
                break;
            }

            case FMT_ITOT:
            {
                printf (inodeFormat, diskInfo->totalInodes);
                break;
            }

            case FMT_IUSED:
            {
                printf (inodeFormat, diskInfo->totalInodes - diskInfo->freeInodes);
                break;
            }

            case FMT_IFREE:
            {
                printf (inodeFormat, diskInfo->freeInodes);
                break;
            }

            case FMT_IPERC:
            {
                used = diskInfo->totalInodes - diskInfo->availInodes;
                totAvail = diskInfo->totalInodes;
                printPerc (used, totAvail, PERC_FMT);
                break;
            }

            case FMT_SPECIAL:
            {
                printf (SPEC_FMT, diskInfo->special);
                break;
            }

            case FMT_SPECIAL_FULL:
            {
                printf (specialFormat, diskInfo->special);
                break;
            }

            case FMT_TYPE:
            {
                printf (FSTYPE_FMT, diskInfo->fsType);
                break;
            }

            case FMT_TYPE_FULL:
            {
                printf (typeFormat, diskInfo->fsType);
                break;
            }

            case FMT_MOUNT_OPTIONS:
            {
                printf (optFormat, diskInfo->options);
                break;
            }

            case FMT_MOUNT_TIME:
            {
                printf (mTimeFormat, diskInfo->mountTime);
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
 * addTotals
 *
 * Add up the totals for the blocks/inodes
 *
 */

static void
addTotals (diskInfo, totals)
    DiskInfo      *diskInfo;
    DiskInfo      *totals;
{
    totals->totalBlocks += diskInfo->totalBlocks;
    totals->freeBlocks += diskInfo->freeBlocks;
    totals->availBlocks += diskInfo->availBlocks;
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
printTitle ()
{
    char                *ptr;
    int                 valid;
    char                tbuff [20];

    if ((flags & F_DEBUG_HDR) == F_DEBUG_HDR)
    {
        printf ("di ver $Revision$ Default Format: %s\n", DEFAULT_FORMAT);
    }

    ptr = formatString;

    while (*ptr)
    {
        valid = TRUE;

        switch (*ptr)
        {
            case FMT_MOUNT:
            {
                printf (MOUNT_FMT, "Mount");
                break;
            }

            case FMT_MOUNT_FULL:
            {
                printf (mountFormat, "Mount");
                break;
            }

            case FMT_BTOT:
            case FMT_BTOT_AVAIL:
            {
                if (dispBlockSize == ONE_K)
                {
                    printf (blockLabelFormat, "Kbytes");
                }
                else if (dispBlockSize == ONE_MEG)
                {
                    printf (blockLabelFormat, "  Megs");
                }
                else if (dispBlockSize == ONE_GIG)
                {
                    printf (blockLabelFormat, "  Gigs");
                }
                else if (dispBlockSize == HALF_K)
                {
                    printf (blockLabelFormat, "  512b");
                }
                else
                {
                    sprintf (tbuff, "%6.0f", dispBlockSize);
                    printf (blockLabelFormat, tbuff);
                }
                break;
            }

            case FMT_BUSED:
            case FMT_BCUSED:
            {
                printf (blockLabelFormat, "Used");
                break;
            }

            case FMT_BFREE:
            {
                printf (blockLabelFormat, "Free");
                break;
            }

            case FMT_BAVAIL:
            {
                printf (blockLabelFormat, "Avail");
                break;
            }

            case FMT_BPERC_AVAIL:
            case FMT_BPERC_FREE:
            case FMT_BPERC_BSD:
            {
                printf (PERC_LBL_FMT, "%used");
                break;
            }

            case FMT_ITOT:
            {
                printf (inodeLabelFormat, "Inodes");
                break;
            }

            case FMT_IUSED:
            {
                printf (inodeLabelFormat, "Used");
                break;
            }

            case FMT_IFREE:
            {
                printf (inodeLabelFormat, "Free");
                break;
            }

            case FMT_IPERC:
            {
                printf (PERC_LBL_FMT, "%used");
                break;
            }

            case FMT_SPECIAL:
            {
                printf (SPEC_FMT, "Filesystem");
                break;
            }

            case FMT_SPECIAL_FULL:
            {
                printf (specialFormat, "Filesystem");
                break;
            }

            case FMT_TYPE:
            {
                printf (FSTYPE_FMT, "fsType");
                break;
            }

            case FMT_TYPE_FULL:
            {
                printf (typeFormat, "fs Type");
                break;
            }

            case FMT_MOUNT_OPTIONS:
            {
                printf (optFormat, "Options");
                break;
            }

            case FMT_MOUNT_TIME:
            {
                printf (mTimeFormat, "Mount Time");
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
printPerc (used, totAvail, format)
    double      used;
    double      totAvail;
    char        *format;
{
    double      perc;


    if (totAvail > 0.0)
    {
        perc = used / totAvail;
        perc *= 100.0;
    }
    else
    {
        perc = 0.0;
    }
    printf (format, perc);
}


/*
 * Realloc
 *
 * portable realloc
 *
 */

static char *
Realloc (ptr, size)
    char      *ptr;
    long      size;
{
    if (ptr == (char *) NULL)
    {
        ptr = (char *) malloc (size);
    }
    else
    {
        ptr = (char *) realloc (ptr, size);
    }

    return ptr;
}


static void
printFileInfo (optind, argc, argv)
    int                 optind;
    int                 argc;
    char                *argv [];
{
    int                 i;
    int                 j;
    struct stat         statBuf;
    DiskInfo            totals;


    memset ((char *) &totals, '\0', sizeof (DiskInfo));
    strcpy (totals.name, "Total");
    totals.printFlag = DI_OK;

    if ((flags & F_NO_HEADER) != F_NO_HEADER)
    {
        printTitle ();
    }

    for (i = optind; i < argc; ++i)
    {
        if (stat (argv [i], &statBuf) == 0)
        {
            for (j = 0; j < diCount; ++j)
            {
                if (diskInfo [j].printFlag != DI_BAD &&
                        diskInfo [j].st_dev != DI_UNKNOWN_DEV &&
                        (long) statBuf.st_dev == diskInfo [j].st_dev)
                {
                    printInfo (&diskInfo [j]);
                    addTotals (&diskInfo [j], &totals);
                    break; /* out of inner for */
                }
            }
        } /* if stat ok */
        else
        {
            perror (argv [i]);
        }
    } /* for each file specified on command line */

    if ((flags & F_TOTAL) == F_TOTAL && (flags & F_NO_HEADER) != F_NO_HEADER)
    {
        printInfo (&totals);
    }
}

/*
 *  sortArray ()
 *
 *      shellsort!
 *
 */

static void
sortArray (data, dataSize, count, compareFunc)
    char            *data;
    int             dataSize;
    int             count;
    SORT_FUNC       compareFunc;
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
        cleanup ();
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
            memcpy ((char *) tempData, (char *) &(data [i * dataSize]), dataSize);
            j = i - gap;

            while (j >= 0 &&
                compareFunc (&(data [j * dataSize]), tempData) > 0)
            {
                memcpy ((char *) &(data [(j + gap) * dataSize]), (char *) &(data [j * dataSize]), dataSize);
                j -= gap;
            }

            j += gap;
            if (j != i)
            {
                memcpy ((char *) &(data [j * dataSize]), (char *) tempData, dataSize);
            }
        }
    }

    free ((char *) tempData);
}


static int
diCompare (a, b)
    char        *a;
    char        *b;
{
    DiskInfo    *di1;
    DiskInfo    *di2;


    di1 = (DiskInfo *) a;
    di2 = (DiskInfo *) b;


    switch (sortType)
    {
        case DI_SORT_NONE:
        {
            break;
        }

        case DI_SORT_NAME:
        {
            return (strcmp (di1->name, di2->name) * sortOrder);
        }

        case DI_SORT_SPECIAL:
        {
            return (strcmp (di1->special, di2->special) * sortOrder);
        }
    } /* switch on sort type */

    return 0;
}


/*
 * getDiskStatInfo
 *
 * gets the disk device number for each entry.
 *
 */

static void
getDiskStatInfo ()
{
    int         i;
    struct stat statBuf;

    for (i = 0; i < diCount; ++i)
    {
        diskInfo [i].st_dev = DI_UNKNOWN_DEV;

        if (stat (diskInfo [i].name, &statBuf) == 0)
        {
            diskInfo [i].st_dev = (long) statBuf.st_dev;
        }
        else
        {
            perror (diskInfo [i].name);
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
checkDiskInfo ()
{
    int           i;
    int           len;
    int           maxMountString;
    int           maxSpecialString;
    int           maxTypeString;
    int           maxOptString;
    int           maxMntTimeString;
    _fs_size_t    temp;


    maxMountString = (flags & F_NO_HEADER) == F_NO_HEADER ? 0 : 5;
    maxSpecialString = (flags & F_NO_HEADER) == F_NO_HEADER ? 0 : 10;
    maxTypeString = (flags & F_NO_HEADER) == F_NO_HEADER ? 0 : 7;
    maxOptString = (flags & F_NO_HEADER) == F_NO_HEADER ? 0 : 7;
    maxMntTimeString = (flags & F_NO_HEADER) == F_NO_HEADER ? 0 : 26;

    for (i = 0; i < diCount; ++i)
    {
            /* Solaris reports a cdrom as having no free blocks,   */
            /* no available.  Their df doesn't always work right!  */
            /* -1 is returned.                                     */
        if (debug > 2)
        {
            printf ("chk: free: %f\n", diskInfo [i].freeBlocks);
        }
        if (diskInfo [i].freeBlocks < 0.0)
        {
            diskInfo [i].freeBlocks = 0.0;
        }
        if (diskInfo [i].availBlocks < 0.0)
        {
            diskInfo [i].availBlocks = 0.0;
        }

        temp = ~ 0;
        if (diskInfo [i].totalInodes == temp)
        {
            diskInfo [i].totalInodes = 0;
            diskInfo [i].freeInodes = 0;
            diskInfo [i].availInodes = 0;
        }

        if (debug > 2)
        {
            printf ("chk: total: %f\n", diskInfo [i].totalBlocks);
        }
        if (diskInfo [i].totalBlocks <= 0.0 &&
            diskInfo [i].printFlag != DI_BAD)
        {
            diskInfo [i].printFlag = DI_IGNORE;
        }

        checkIgnoreList (&diskInfo [i]);
        checkIncludeList (&diskInfo [i]);
    } /* for all disks */

        /* this loop gets the max string lengths */
    for (i = 0; i < diCount; ++i)
    {
        if (diskInfo [i].printFlag == DI_OK || (flags & F_ALL) == F_ALL)
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

    sprintf (mountFormat, "%%-%d.%ds", maxMountString, maxMountString);
    sprintf (specialFormat, "%%-%d.%ds", maxSpecialString,
             maxSpecialString);
    sprintf (typeFormat, "%%-%d.%ds", maxTypeString, maxTypeString);
    sprintf (optFormat, "%%-%d.%ds", maxOptString, maxOptString);
    sprintf (mTimeFormat, "%%-%d.%ds", maxMntTimeString, maxMntTimeString);

    if (dispBlockSize <= ONE_K)
    {
        sprintf (blockFormat, "%%%d.0f", width);
    }
    else
    {
        sprintf (blockFormat, "%%%d.1f", width);
    }
    sprintf (blockLabelFormat, "%%%ds", width);
#if defined (_LARGEFILE_SOURCE) && _FILE_OFFSET_BITS == 64
    sprintf (inodeFormat, "%%%dllu", inodeWidth);
#else
    sprintf (inodeFormat, "%%%dlu", inodeWidth);
#endif
    sprintf (inodeLabelFormat, "%%%ds", inodeWidth);
}

/*
 * usage
 *
 */

static void
usage ()
{
    printf ("di ver $Revision$    Default Format: %s\n", DEFAULT_FORMAT);
         /*  12345678901234567890123456789012345678901234567890123456789012345678901234567890 */
    printf ("Usage: di [-ant] [-f format] [-s sort-type] [-i ignore-fstyp-list]\n");
    printf ("       [-I include-fstyp-list] [-w kbyte-width] [-W inode-width] [file [...]]\n");
    printf ("   -a   : print all mounted devices; normally, those with 0 total blocks are\n");
    printf ("          not printed.  e.g. /dev/proc, /dev/fd.\n");
    printf ("   -d x : size to print blocks in (p - posix (512), k - kbytes,\n");
    printf ("          m - megabytes, g - gigabytes, <x> - numeric size).\n");
    printf ("   -f x : use format string <x>\n");
    printf ("   -i x : ignore file system types in <x>\n");
    printf ("   -I x : include only file system types in <x>\n");
    printf ("   -l   : display local filesystems only\n");
    printf ("   -n   : don't print header\n");
    printf ("   -s t : sort type; n - no sort, s - by special device, r - reverse\n");
    printf ("   -t   : print totals\n");
    printf ("   -w n : use width <n> for kbytes\n");
    printf ("   -W n : use width <n> for i-nodes\n");
    printf (" Format string values:\n");
    printf ("    m - mount point                     M - mount point, full length\n");
    printf ("    b - total kbytes                    B - kbytes available for use\n");
    printf ("    u - used kbytes                     c - calculated kbytes in use\n");
    printf ("    f - kbytes free                     v - kbytes available\n");
    printf ("    p - percentage not avail. for use   1 - percentage used\n");
    printf ("    2 - percentage of user-available space in use.\n");
    printf ("    i - total file slots (i-nodes)      U - used file slots\n");
    printf ("    F - free file slots                 P - percentage file slots used\n");
    printf ("    s - filesystem name                 S - filesystem name, full length\n");
    printf ("    t - disk partition type             T - partition type, full length\n");
}


static void
processArgs (argc, argv)
    int         argc;
    char        *argv [];
{
    int         ch;


    while ((ch = getopt (argc, argv, "Aad:f:hi:I:Flns:tw:W:x:")) != -1)
    {
        switch (ch)
        {
            case 'A':
            {
                formatString = ALL_FORMAT;
                flags |= F_ALL | F_DEBUG_HDR | F_TOTAL;
                flags &= ~ F_NO_HEADER;
                width = 10;
                inodeWidth = 10;
                break;
            }

            case 'a':
            {
                flags |= F_ALL;
                break;
            }

            case 'd':
            {
                switch (tolower (*optarg))
                {
                    case 'p':
                    {
                        dispBlockSize = HALF_K;
                        break;
                    }

                    case 'k':
                    {
                        dispBlockSize = ONE_K;
                        break;
                    }

                    case 'm':
                    {
                        dispBlockSize = ONE_MEG;
                        break;
                    }

                    case 'g':
                    {
                        dispBlockSize = ONE_GIG;
                        break;
                    }

                    default:
                    {
                        if (isdigit (*optarg))
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

            case 'h':
            {
                usage ();
                cleanup ();
                exit (0);
            }

            case 'i':
            {
                parseList (&ignoreList, optarg);
                break;
            }

            case 'I':
            {
                parseList (&includeList, optarg);
                break;
            }

            case 'l':
            {
                flags |= F_LOCAL_ONLY;
                break;
            }

            case 'n':
            {
                flags |= F_NO_HEADER;
                break;
            }

            case 's':
            {
                switch (ch)
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

                    case 'r':
                    {
                        sortOrder = DI_SORT_DESCENDING;
                        break;
                    }
                }
                break;
            }

            case 't':
            {
                flags |= F_TOTAL;
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
                break;
            }

            case '?':
            {
                usage ();
                cleanup ();
                exit (1);
            }
        }
    }
}

    /* list is assumed to be global */
static void
parseList (list, str)
    char        ***list;
    char        *str;
{
    char        *dstr;
    char        *ptr;
    char        **lptr;
    int         count;
    int         i;
    int         len;

    i = strlen (str);
    dstr = (char *) malloc (i + 1);
    if (dstr == (char *) NULL)
    {
        fprintf (stderr, "malloc failed in parseList() (1).  errno %d\n", errno);
        cleanup ();
        exit (1);
    }

    memcpy (dstr, str, i + 1);

    ptr = strtok (dstr, DI_LIST_SEP);
    count = 0;
    while (ptr != (char *) NULL)
    {
        ++count;
        ptr = strtok ((char *) NULL, DI_LIST_SEP);
    }

    *list = (char **) malloc ((count + 1) * sizeof (char *));
    if (*list == (char **) NULL)
    {
        fprintf (stderr, "malloc failed in parseList() (2).  errno %d\n", errno);
        cleanup ();
        exit (1);
    }

    ptr = dstr;
    lptr = *list;
    for (i = 0; i < count; ++i)
    {
        len = strlen (ptr);
        *lptr = (char *) malloc (len + 1);
        if (*lptr == (char *) NULL)
        {
            fprintf (stderr, "malloc failed in parseList() (3).  errno %d\n",
                    errno);
            cleanup ();
            exit (1);
        }
        strcpy (*lptr, ptr);
        ptr += len + 1;
        ++lptr;
    }

    *lptr = (char *) NULL;
    free ((char *) dstr);
}


static void
checkIgnoreList (diskInfo)
    DiskInfo        *diskInfo;
{
    char            **ptr;

        /* if the file system type is in the ignore list, skip it */
    if (ignoreList != (char **) NULL)
    {
        ptr = ignoreList;
        while (*ptr != (char *) NULL)
        {
            if (strcmp (*ptr, diskInfo->fsType) == 0)
            {
                diskInfo->printFlag = DI_IGNORE;
                break;
            }
            ++ptr;
        }
    } /* if an ignore list was specified */
}

static void
checkIncludeList (diskInfo)
    DiskInfo        *diskInfo;
{
    char            **ptr;

        /* if the file system type is not in the include list, skip it */
    if (includeList != (char **) NULL)
    {
        ptr = includeList;
        while (*ptr != (char *) NULL)
        {
            if (strcmp (*ptr, diskInfo->fsType) == 0)
            {
                diskInfo->printFlag = DI_OK;
                break;
            }
            else
            {
                diskInfo->printFlag = DI_IGNORE;
            }
            ++ptr;
        }
    } /* if an include list was specified */
}


#if defined (HAS_MNTTAB)

#define DFS_FS_TABLE 	"/etc/dfs/fstypes"

/*
 * getDiskEntries
 *
 * For SysV.4, we open the file and call getmntent () repeatedly.
 *
 */

static int
getDiskEntries ()
{
    FILE            *f;
    int             idx;
    int             i;
    struct mnttab   mntEntry;
    char            buff [80];
    time_t          mtime;
    char            *devp;   /* local ptr to dev entry */


    if ((flags & F_LOCAL_ONLY) == F_LOCAL_ONLY)
    {
        i = remoteFileSystemCount;

        if ((f = fopen (DFS_FS_TABLE, "r")) != (FILE *) NULL)
        {
            fgets (buff, 80, f);
            if (debug > 1)
            {
                printf ("remote file system type: %s\n", buff);
            }
            sscanf (buff, "%s", remoteFileSystemTypes [i++]);
            fclose (f);
        }

        remoteFileSystemCount = i;
    }

    if ((f = fopen (MOUNT_FILE, "r")) == (FILE *) NULL)
    {
        fprintf (stderr, "Unable to open: %s errno %d\n", MOUNT_FILE, errno);
        return -1;
    }

    while (getmntent (f, &mntEntry) == 0)
    {
        idx = diCount;
        ++diCount;
        diskInfo = (DiskInfo *) Realloc ((char *) diskInfo,
                sizeof (DiskInfo) * diCount);
        memset ((char *) &diskInfo [idx], '\0', sizeof (DiskInfo));
        diskInfo [idx].printFlag = DI_OK;

        strncpy (diskInfo [idx].special, mntEntry.mnt_special, SPEC_NAME_LEN);
        strncpy (diskInfo [idx].name, mntEntry.mnt_mountp, NAME_LEN);
        if (devp = strstr (mntEntry.mnt_mntopts, "dev"))
        {
            if (devp != mntEntry.mnt_mntopts)
            {
                --devp;
            }
            *devp = 0;   /* point to preceeding comma and cut off */
        }
        strncpy (diskInfo [idx].options, mntEntry.mnt_mntopts, OPT_LEN);
        mtime = atol (mntEntry.mnt_time);
        strncpy (diskInfo [idx].mountTime, ctime (&mtime), MNT_TIME_LEN);

            /* get the file system type now... */
        strncpy (diskInfo [idx].fsType, mntEntry.mnt_fstype, TYPE_LEN);

        diskInfo [idx].isLocal = TRUE;
        for (i = 0; i < remoteFileSystemCount; ++i)
        {
            if (strcmp (diskInfo [idx].fsType, remoteFileSystemTypes [i]) == 0)
            {
                diskInfo [idx].isLocal = FALSE;
            }
        }

        checkIgnoreList (&diskInfo [idx]);
        checkIncludeList (&diskInfo [idx]);

        if (debug > 0)
        {
            printf ("mnt:%s - %s\n", diskInfo [idx].name,
                    diskInfo [idx].special);
        }
    }

    fclose (f);
    return 0;
}

#endif /* HAS_MNTTAB */

#if defined (HAS_SVR3_MNTTAB)

/*
 * getDiskEntries
 *
 * For SysV.3 we open the file and read it ourselves.
 *
 */

static int
getDiskEntries ()
{
    FILE             *f;
    int              idx;
    struct mnttab    mntEntry;


    if ((f = fopen (MOUNT_FILE, "r")) == (FILE *) NULL)
    {
        fprintf (stderr, "Unable to open: %s errno %d\n", MOUNT_FILE, errno);
        return -1;
    }

    while (fread ((char *) &mntEntry, sizeof (struct mnttab), 1, f) == 1)
    {
            /* xenix allows null mount table entries */
            /* sco nfs background mounts are marked as "nothing" */
        if (mntEntry.mt_filsys [0] &&
                strcmp (mntEntry.mt_filsys, "nothing") != 0)
        {
            idx = diCount;
            ++diCount;
            diskInfo = (DiskInfo *) Realloc ((char *) diskInfo,
                                             sizeof (DiskInfo) * diCount);
            memset ((char *) &diskInfo [idx], '\0', sizeof (DiskInfo));
            diskInfo [idx].printFlag = DI_OK;
            diskInfo [idx].isLocal = TRUE;

# if defined (COHERENT)
                /* Coherent seems to have these fields reversed. oh well. */
            strncpy (diskInfo [idx].name, mntEntry.mt_dev, NAME_LEN);
            strncpy (diskInfo [idx].special, mntEntry.mt_filsys, SPEC_NAME_LEN);
# else
            strncpy (diskInfo [idx].special, mntEntry.mt_dev, SPEC_NAME_LEN);
            strncpy (diskInfo [idx].name, mntEntry.mt_filsys, NAME_LEN);
# endif
            strncpy (diskInfo [idx].options, mntEntry.mnt_mntopts, OPT_LEN);
            strncpy (diskInfo [idx].mountTime, mntEntry.mnt_time,
                    MNT_TIME_LEN);
        }

        if (debug > 0)
        {
            printf ("mnt:%s - %s\n", diskInfo [idx].name,
                    diskInfo [idx].special);
        }
    }

    fclose (f);
    return 0;
}

#endif /* HAS_MNTTAB */

#if defined (HAS_MNTENT)

/*
 * getDiskEntries
 *
 * SunOS supplies an open and close routine for the mount table.
 *
 */

static int
getDiskEntries ()
{
    FILE            *f;
    int             idx;
    struct mntent   *mntEntry;
    char            *devp;   /* local ptr to dev entry */


    if ((f = setmntent (MOUNT_FILE, "r")) == (FILE *) NULL)
    {
        fprintf (stderr, "Unable to open: %s errno %d\n", MOUNT_FILE, errno);
        return -1;
    }

    while ((mntEntry = getmntent (f)) != (struct mntent *) NULL)
    {
        idx = diCount;
        ++diCount;
        diskInfo = (DiskInfo *) Realloc ((char *) diskInfo,
                sizeof (DiskInfo) * diCount);
        memset ((char *) &diskInfo [idx], '\0', sizeof (DiskInfo));
        diskInfo [idx].printFlag = DI_OK;
        diskInfo [idx].isLocal = TRUE;

        strncpy (diskInfo [idx].special, mntEntry->mnt_fsname, SPEC_NAME_LEN);
        strncpy (diskInfo [idx].name, mntEntry->mnt_dir, NAME_LEN);
        strncpy (diskInfo [idx].fsType, mntEntry->mnt_type, TYPE_LEN);

        if (strcmp (mntEntry->mnt_type, MNTTYPE_IGNORE) == 0)
        {
            diskInfo [idx].printFlag = DI_IGNORE;
        }

        if (devp = strstr (mntEntry->mnt_opts, "dev"))
        {
            if (devp != mntEntry->mnt_opts)
            {
                --devp;
            }
            *devp = 0;   /* point to preceeding comma and cut off */
        }
        strncpy (diskInfo [idx].options, mntEntry->mnt_opts, OPT_LEN);

        checkIgnoreList (&diskInfo [idx]);
        checkIncludeList (&diskInfo [idx]);

        if (debug > 0)
        {
            printf ("mnt:%s - %s : %s\n", diskInfo [idx].name,
                    diskInfo [idx].special, diskInfo [idx].fsType);
        }
    }

    endmntent (f);
    return 0;
}

#endif /* HAS_MNTENT */

#if defined (HAS_GETMNTINFO)

/*
 * getDiskEntries
 *
 * OSF/1 does this with a system call and library routine
 *
 *                  [mogul@wrl.dec.com (Jeffrey Mogul)]
 */

#if defined (HAS_INITMOUNTNAMES) && defined (INITMOUNTNAMES)
static char *mnt_names [] = INITMOUNTNAMES;
# define MNT_NUMTYPES (MOUNT_MAXTYPE + 1)
#endif

    /* osf/1 mount flags start w/M_ vs. MNT_     */
    /* this saves us from lots of duplicate code */
#if defined (M_RDONLY)
# define MNT_RDONLY M_RDONLY
#endif
#if defined (M_SYNCHRONOUS)
# define MNT_SYNCHRONOUS M_SYNCHRONOUS
#endif
#if defined (M_NOEXEC)
# define MNT_NOEXEC M_NOEXEC
#endif
#if defined (M_NOSUID)
# define MNT_NOSUID M_NOSUID
#endif
#if defined (M_NODEV)
# define MNT_NODEV M_NODEV
#endif
#if defined (M_GRPID)
# define MNT_GRPID M_GRPID
#endif
#if defined (M_EXPORTED)
# define MNT_EXPORTED M_EXPORTED
#endif
#if defined (M_EXRDONLY)
# define MNT_EXRDONLY M_EXRDONLY
#endif
#if defined (M_EXRDMOSTLY)
# define MNT_EXRDMOSTLY M_EXRDMOSTLY
#endif
#if defined (M_SECURE)
# define MNT_SECURE M_SECURE
#endif
#if defined (M_LOCAL)
# define MNT_LOCAL M_LOCAL
#endif
#if defined (M_QUOTA)
# define MNT_QUOTA M_QUOTA
#endif

static int
getDiskEntries ()
{
    int             count;
    int             idx;
    int             len;
    short           fstype;
    struct statfs   *mntbufp;
    double          mult;

    count = getmntinfo (&mntbufp, MNT_NOWAIT);
    if (count < 1)
    {
        fprintf (stderr, "Unable to do getmntinfo () errno %d\n", errno);
        return -1;
    }

    diCount = count;
    diskInfo = (DiskInfo *) malloc (sizeof (DiskInfo) * count);
    if (diskInfo == (DiskInfo *) NULL)
    {
        fprintf (stderr, "malloc failed for diskInfo. errno %d\n", errno);
        return -1;
    }
    memset ((char *) diskInfo, '\0', sizeof (DiskInfo) * count);

    if (debug > 1)
    {
        printf ("type_len %d name_len %d spec_name_len %d\n", TYPE_LEN,
                NAME_LEN, SPEC_NAME_LEN);
    }

    for (idx = 0; idx < count; idx++)
    {
        diskInfo [idx].printFlag = DI_OK;
        diskInfo [idx].isLocal = FALSE;
#if defined (MNT_LOCAL)
        if ((mntbufp [idx].f_flags & MNT_LOCAL) == MNT_LOCAL)
        {
            diskInfo [idx].isLocal = TRUE;
        }
#endif

        if (diskInfo [idx].isLocal == FALSE && (flags & F_LOCAL_ONLY) == F_LOCAL_ONLY)
        {
            diskInfo [idx].printFlag = DI_IGNORE;
        }

        strncpy (diskInfo [idx].special, mntbufp [idx].f_mntfromname,
                SPEC_NAME_LEN);
        strncpy (diskInfo [idx].name, mntbufp [idx].f_mntonname, NAME_LEN);

        mult = 1.0;

#  if defined (HAS_GETMNTINFO_FSIZE) /* 1.x */
        mult = (double) mntbufp [idx].f_fsize / dispBlockSize;
#  endif
#  if defined (HAS_GETMNTINFO_BSIZE) /* 2.x */
        mult = (double) mntbufp [idx].f_bsize / dispBlockSize;
#  endif
        diskInfo [idx].totalBlocks = ((double) (_s_fs_size_t) mntbufp [idx].f_blocks * mult);
        diskInfo [idx].freeBlocks = ((double) (_s_fs_size_t) mntbufp [idx].f_bfree * mult);
        diskInfo [idx].availBlocks = ((double) (_s_fs_size_t) mntbufp [idx].f_bavail * mult);

        diskInfo [idx].totalInodes = mntbufp [idx].f_files;
        diskInfo [idx].freeInodes = mntbufp [idx].f_ffree;
        diskInfo [idx].availInodes = mntbufp [idx].f_ffree;

        fstype = mntbufp [idx].f_type;
# if defined (HAS_NO_FSTYPES_H) && ! defined (HAS_INITMOUNTNAMES)
        if ((fstype >= 0) && (fstype <= MOUNT_MAXTYPE))
        {
            switch (fstype)
            {
#  if defined (MOUNT_NONE)
                case MOUNT_NONE:         /* No Filesystem */
                {
                    strncpy (diskInfo [idx].fsType, "none", TYPE_LEN);
                    break;
                }
#  endif

#  if defined (MOUNT_UFS)
                case MOUNT_UFS:         /* UNIX "Fast" Filesystem */
                {
                    strncpy (diskInfo [idx].fsType, "ufs", TYPE_LEN);
                    break;
                }
#  endif

#  if defined (MOUNT_NFS)
                case MOUNT_NFS:         /* Network Filesystem */
                {
                    strncpy (diskInfo [idx].fsType, "nfs", TYPE_LEN);
                    break;
                }
#  endif

#  if defined (MOUNT_MFS)
                case MOUNT_MFS:         /* Memory Filesystem */
                {
                    strncpy (diskInfo [idx].fsType, "mfs", TYPE_LEN);
                    break;
                }
#  endif

#  if defined (MOUNT_MSDOS)
                case MOUNT_MSDOS:       /* MSDOS Filesystem */
                {
                    strncpy (diskInfo [idx].fsType, "msdos", TYPE_LEN);
                    break;
                }
#  endif

#  if defined (MOUNT_LFS)
                case MOUNT_LFS:
                {
                    strncpy (diskInfo [idx].fsType, "lfs", TYPE_LEN);
                    break;
                }
#  endif

#  if defined (MOUNT_LOFS)
                case MOUNT_LOFS:
                {
                    strncpy (diskInfo [idx].fsType, "lofs", TYPE_LEN);
                    break;
                }
#  endif

#  if defined (MOUNT_FDESC)
                case MOUNT_FDESC:
                {
                    strncpy (diskInfo [idx].fsType, "fdesc", TYPE_LEN);
                    break;
                }
#  endif

#  if defined (MOUNT_PORTAL)
                case MOUNT_PORTAL:
                {
                    strncpy (diskInfo [idx].fsType, "portal", TYPE_LEN);
                    break;
                }
#  endif

#  if defined (MOUNT_NULL)
                case MOUNT_NULL:
                {
                    strncpy (diskInfo [idx].fsType, "null", TYPE_LEN);
                    break;
                }
#  endif

#  if defined (MOUNT_UMAP)
                case MOUNT_UMAP:
                {
                    strncpy (diskInfo [idx].fsType, "umap", TYPE_LEN);
                    break;
                }
#  endif

#  if defined (MOUNT_KERNFS)
                case MOUNT_KERNFS:
                {
                    strncpy (diskInfo [idx].fsType, "kernfs", TYPE_LEN);
                    break;
                }
#  endif

#  if defined (MOUNT_PROCFS)
                case MOUNT_PROCFS:      /* proc filesystem */
                {
                    strncpy (diskInfo [idx].fsType, "pfs", TYPE_LEN);
                    break;
                }
#  endif

#  if defined (MOUNT_AFS)
                case MOUNT_AFS:
                {
                    strncpy (diskInfo [idx].fsType, "afs", TYPE_LEN);
                    break;
                }
#  endif

#  if defined (MOUNT_ISOFS)
                case MOUNT_ISOFS:       /* iso9660 cdrom */
                {
                    strncpy (diskInfo [idx].fsType, "iso9660fs", TYPE_LEN);
                    break;
                }
#  endif

#  if defined (MOUNT_ISO9660) && ! defined (MOUNT_CD9660)
                case MOUNT_ISO9660:       /* iso9660 cdrom */
                {
                    strncpy (diskInfo [idx].fsType, "iso9660", TYPE_LEN);
                    break;
                }
#  endif

#  if defined (MOUNT_CD9660)
                case MOUNT_CD9660:       /* iso9660 cdrom */
                {
                    strncpy (diskInfo [idx].fsType, "cd9660", TYPE_LEN);
                    break;
                }
#  endif

#  if defined (MOUNT_UNION)
                case MOUNT_UNION:
                {
                    strncpy (diskInfo [idx].fsType, "union", TYPE_LEN);
                    break;
                }
#  endif
            } /* switch on mount type */
        }
# else
            /* could use getvfsbytype here... */
        if ((fstype >= 0) && (fstype < MNT_NUMTYPES))
        {
            strncpy (diskInfo [idx].fsType, mnt_names [fstype], TYPE_LEN);
        }
        else
        {
            sprintf (diskInfo [idx].fsType, UNKNOWN_FSTYPE, fstype);
        }
# endif /* has fs_types.h */

#if defined (MNT_RDONLY)
        if ((mntbufp [idx].f_flags & MNT_RDONLY) == MNT_RDONLY)
        {
            strcat (diskInfo [idx].options, "ro,");
        }
        else
        {
            strcat (diskInfo [idx].options, "rw,");
        }
#endif
#if defined (MNT_SYNCHRONOUS)
        if ((mntbufp [idx].f_flags & MNT_SYNCHRONOUS) == MNT_SYNCHRONOUS)
        {
            strcat (diskInfo [idx].options, "sync,");
        }
#endif
#if defined (MNT_NOEXEC)
        if ((mntbufp [idx].f_flags & MNT_NOEXEC) == MNT_NOEXEC)
        {
            strcat (diskInfo [idx].options, "noexec,");
        }
#endif
#if defined (MNT_NOSUID)
        if ((mntbufp [idx].f_flags & MNT_NOSUID) != MNT_NOSUID)
        {
            strcat (diskInfo [idx].options, "suid,");
        }
#endif
#if defined (MNT_NODEV)
        if ((mntbufp [idx].f_flags & MNT_NODEV) == MNT_NODEV)
        {
            strcat (diskInfo [idx].options, "nodev,");
        }
#endif
#if defined (MNT_GRPID)
        if ((mntbufp [idx].f_flags & MNT_GRPID) == MNT_GRPID)
        {
            strcat (diskInfo [idx].options, "grpid,");
        }
#endif
#if defined (MNT_UNION)
        if ((mntbufp [idx].f_flags & MNT_UNION) == MNT_UNION)
        {
            strcat (diskInfo [idx].options, "union,");
        }
#endif
#if defined (MNT_ASYNC)
        if ((mntbufp [idx].f_flags & MNT_ASYNC) == MNT_ASYNC)
        {
            strcat (diskInfo [idx].options, "async,");
        }
#endif
#if defined (MNT_EXRDONLY)
        if ((mntbufp [idx].f_flags & MNT_EXRDONLY) == MNT_EXRDONLY)
        {
            strcat (diskInfo [idx].options, "exported ro,");
        }
#endif
#if defined (MNT_EXPORTED)
        if ((mntbufp [idx].f_flags & MNT_EXPORTED) == MNT_EXPORTED)
        {
            strcat (diskInfo [idx].options, "exported");
        }
#endif
#if defined (MNT_EXRDMOSTLY)
        if ((mntbufp [idx].f_flags & MNT_EXRDMOSTLY) == MNT_EXRDMOSTLY)
        {
                /* what's read-mostly ? */
            strcat (diskInfo [idx].options, "exported read-mostly");
        }
#endif
#if defined (MNT_DEFEXPORTED)
        if ((mntbufp [idx].f_flags & MNT_DEFEXPORTED) == MNT_DEFEXPORTED)
        {
                /* what's this ? */
            strcat (diskInfo [idx].options, "exported world,");
        }
#endif
#if defined (MNT_EXPORTANON)
        if ((mntbufp [idx].f_flags & MNT_EXPORTANON) == MNT_EXPORTANON)
        {
            strcat (diskInfo [idx].options, "exported anon,");
        }
#endif
#if defined (MNT_EXKERB)
        if ((mntbufp [idx].f_flags & MNT_EXKERB) == MNT_EXKERB)
        {
            strcat (diskInfo [idx].options, "exported kerberos,");
        }
#endif
#if defined (MNT_LOCAL)
        if ((mntbufp [idx].f_flags & MNT_LOCAL) == MNT_LOCAL)
        {
            strcat (diskInfo [idx].options, "local,");
        }
#endif
#if defined (MNT_QUOTA)
        if ((mntbufp [idx].f_flags & MNT_QUOTA) == MNT_QUOTA)
        {
            strcat (diskInfo [idx].options, "quota,");
        }
#endif
#if defined (MNT_ROOTFS)
        if ((mntbufp [idx].f_flags & MNT_ROOTFS) == MNT_ROOTFS)
        {
            strcat (diskInfo [idx].options, "root,");
        }
#endif
#if defined (MNT_USER)
        if ((mntbufp [idx].f_flags & MNT_USER) == MNT_USER)
        {
            strcat (diskInfo [idx].options, "user,");
        }
#endif
#if defined (MNT_SECURE)
        if ((mntbufp [idx].f_flags & MNT_SECURE) == MNT_SECURE)
        {
            strcat (diskInfo [idx].options, "secure,");
        }
#endif

        len = strlen (diskInfo [idx].options);
        if (len > 0)
        {
            --len;
        }
        if (diskInfo [idx].options [len]==',')
        {
            diskInfo [idx].options [len] = '\0';
        }

        if (debug > 1)
        {
            printf ("%s: %s\n", diskInfo [idx].name, diskInfo [idx].fsType);
            printf ("\tblocks: tot:%ld free:%ld avail:%ld\n",
                    mntbufp [idx].f_blocks, mntbufp [idx].f_bfree,
                    mntbufp [idx].f_bavail);
            printf ("\tmult: %f\n", mult);
# if defined (HAS_GETMNTINFO_FSIZE)
            printf ("\tfsize:%ld \n", mntbufp [idx].f_fsize);
            printf ("\tbsize:%ld \n", mntbufp [idx].f_bsize);
# endif
# if defined (HAS_GETMNTINFO_BSIZE)
            printf ("\tbsize:%ld \n", mntbufp [idx].f_bsize);
            printf ("\tiosize:%ld \n", mntbufp [idx].f_iosize);
# endif
            printf ("\tinodes: tot:%ld free:%ld\n",
                    mntbufp [idx].f_files, mntbufp [idx].f_ffree);
        }
    }

    free ((char *) mntbufp);  /* man page says this can't be freed. */
                              /* is it ok to try?                   */
    return 0;
}

/* this is a no-op; we have already done all the work */
static void
getDiskInfo ()
{
}

#endif /* HAS_GETMNTINFO */

#if defined (HAS_GETMNT)

/*
 * getDiskEntries
 *
 * ULTRIX does this with a system call.  The system call allows one
 * to retrieve the information in a series of calls, but coding that
 * looks a little tricky; I just allocate a huge buffer and do it in
 * one shot.
 *
 *                  [mogul@wrl.dec.com (Jeffrey Mogul)]
 */

static int
getDiskEntries ()
{
    int             count;
    int             bufsize;
    int             idx;
    short           fstype;
    struct fs_data  *fsdbuf;
    int             start;
    int             len;


    bufsize = NMOUNT * sizeof (struct fs_data);  /* enough for max # mounts */
    fsdbuf = (struct fs_data *) malloc (bufsize);
    if (fsdbuf == (struct fs_data *) NULL)
    {
        fprintf (stderr, "malloc (%d) for getmnt () failed errno %d\n",
                 bufsize, errno);
        return -1;
    }

    start = 0;
    count = getmnt (&start, fsdbuf, bufsize, STAT_MANY, 0);
    if (count < 1)
    {
        fprintf (stderr, "Unable to do getmnt () [= %d] errno %d\n",
                 count, errno);
        free ((char *) fsdbuf);
        return -1;
    }

    diCount = count;
    diskInfo = (DiskInfo *) malloc (sizeof (DiskInfo) * count);
    if (diskInfo == (DiskInfo *) NULL)
    {
        fprintf (stderr, "malloc failed for diskInfo. errno %d\n", errno);
        free ((char *) fsdbuf);
        return -1;
    }
    memset ((char *) diskInfo, '\0', sizeof (DiskInfo) * count);

    for (idx = 0; idx < count; idx++)
    {
        diskInfo [idx].printFlag = DI_OK;
        diskInfo [idx].isLocal = TRUE;

        if ((fsdbuf [idx].fd_req.flags & M_LOCAL) != M_LOCAL)
        {
            diskInfo [idx].isLocal = FALSE;
        }

        if (diskInfo [idx].isLocal == FALSE && (flags & F_LOCAL_ONLY) == F_LOCAL_ONLY)
        {
            diskInfo [idx].printFlag = DI_IGNORE;
        }

        strncpy (diskInfo [idx].special, fsdbuf [idx].fd_devname, SPEC_NAME_LEN);
        strncpy (diskInfo [idx].name, fsdbuf [idx].fd_path, NAME_LEN);

            /* ULTRIX keeps these fields in units of 1K byte */
        diskInfo [idx].totalBlocks = fsdbuf [idx].fd_btot;
        diskInfo [idx].freeBlocks = fsdbuf [idx].fd_bfree;
        diskInfo [idx].availBlocks = fsdbuf [idx].fd_bfreen;

        diskInfo [idx].totalInodes = fsdbuf [idx].fd_gtot;
        diskInfo [idx].freeInodes = fsdbuf [idx].fd_gfree;
        diskInfo [idx].availInodes = fsdbuf [idx].fd_gfree;

        fstype = fsdbuf [idx].fd_fstype;
        if (fstype == GT_UNKWN)
        {
            diskInfo [idx].printFlag = DI_IGNORE;
        }
        else if ((fstype > 0) && (fstype < GT_NUMTYPES))
        {
            strncpy (diskInfo [idx].fsType, gt_names [fstype], TYPE_LEN);
        }
        else
        {
            sprintf (diskInfo [idx].fsType, "Unknown fstyp %.2d", fstype);
        }

        if ((fsdbuf [idx].fd_req.flags & M_RONLY) == M_RONLY)
        {
            strcat (diskInfo [idx].options, "ro,");
        }
        else
        {
            strcat (diskInfo [idx].options, "rw,");
        }
        if ((fsdbuf [idx].fd_req.flags & M_NOSUID) != M_NOSUID)
        {
            strcat (diskInfo [idx].options, "suid,");
        }
        if ((fsdbuf [idx].fd_req.flags & M_QUOTA) == M_QUOTA)
        {
            strcat (diskInfo [idx].options, "quota,");
        }
        if ((fsdbuf [idx].fd_req.flags & M_LOCAL) != M_LOCAL)
        {
            strcat (diskInfo [idx].options, "remote,");
        }
        if ((fsdbuf [idx].fd_req.flags & M_NODEV) == M_NODEV)
        {
            strcat (diskInfo [idx].options, "nodev,");
        }
        if ((fsdbuf [idx].fd_req.flags & M_FORCE) == M_FORCE)
        {
            strcat (diskInfo [idx].options, "force,");
        }
        if ((fsdbuf [idx].fd_req.flags & M_SYNC) == M_SYNC)
        {
            strcat (diskInfo [idx].options, "sync,");
        }
        if ((fsdbuf [idx].fd_req.flags & M_NOCACHE) == M_NOCACHE)
        {
            strcat (diskInfo [idx].options, "nocache,");
        }
        if ((fsdbuf [idx].fd_req.flags & M_EXPORTED) == M_EXPORTED)
        {
            strcat (diskInfo [idx].options, "exported," );
        }
        if ((fsdbuf [idx].fd_req.flags & M_EXRONLY) == M_EXRONLY)
        {
            strcat (diskInfo [idx].options, "exported ro,");
        }
        if ((fsdbuf [idx].fd_req.flags & M_NOEXEC) == M_NOEXEC)
        {
            strcat (diskInfo [idx].options, "noexec,");
        }

        len = strlen (diskInfo [idx].options);
        if (len > 0)
        {
            --len;
        }
        if (diskInfo [idx].options [len]==',')
        {
            diskInfo [idx].options [len] = '\0';
        }

        if (debug > 1)
        {
            printf ("%s: %s\n", diskInfo [idx].name, diskInfo [idx].fsType);
            printf ("\tblocks: tot:%ld free:%ld avail:%ld\n",
                    fsdbuf [idx].fd_btot, fsdbuf [idx].fd_bfree,
                    fsdbuf [idx].fd_bfreen);
            printf ("\tinodes: tot:%ld free:%ld\n",
                    fsdbuf [idx].fd_gtot, fsdbuf [idx].fd_gfree);
        }
    }

    free ((char *) fsdbuf);
    return 0;
}

/* this is a no-op; we have already done all the work */
static void
getDiskInfo ()
{
}

#endif /* HAS_GETMNT */


#if defined (HAS_MNTCTL)

/*
 * getDiskEntries
 *
 * AIX V3.2 uses mntctl to find out about mounted file systems
 *
 */

# define NUM_AIX_FSTYPES         6
static char *AIX_fsType [NUM_AIX_FSTYPES] =
    { "oaix", "", "nfs", "jfs", "", "cdrom" };

/*
 * from xfsm-1.80:
 *
 * MNT_AIX - "aix"
 * MNT_NFS - "nfs"
 * MNT_JFS - "jfs"
 * MNT_CDROM - "cdrom"
 * other - "user defined"
 *
 */

static int
getDiskEntries ()
{
    int             num;        /* number of vmount structs returned    */
    char            *vmbuf;     /* buffer for vmount structs returned   */
    int             vmbufsz;    /* size in bytes of vmbuf               */
    int             i;          /* index for looping and stuff          */
    char            *bufp;      /* pointer into vmbuf                   */
    struct vmount   *vmtp;      /* pointer into vmbuf                   */
    struct vfs_ent  *ve;        /* pointer for file system type entry   */
    int             len;


    i = 0;
    vmbufsz = sizeof (struct vmount) * FSMAGIC; /* initial vmount buffer */

    do
    {
        if ((vmbuf = (char *) malloc (vmbufsz)) == (char *) NULL)
        {
            fprintf (stderr, "malloc (%d) for mntctl() failed errno %d\n",
                    vmbufsz, errno);
            return -1;
        }

        num = mntctl (MCTL_QUERY, vmbufsz, vmbuf);
            /*
             * vmbuf is too small, could happen for
             * following reasons:
             * - inital buffer is too small
             * - newly mounted file system
             */
        if (num == 0)
        {
            memcpy (&vmbufsz, vmbuf, sizeof (vmbufsz)); /* see mntctl(2) */
            if (debug > 0)
            {
                printf ("vmbufsz too small, new size: %d\n", vmbufsz);
            }
            free ((char *) vmbuf); /* free this last, it's still being used! */
            ++i;
        }
    } while (num == 0 && i < RETRY_COUNT);

    if (i >= RETRY_COUNT)
    {
        free ((char *) vmbuf);
        fprintf (stderr, "unable to allocate adequate buffer for mntctl\n");
        return -1;
    }

    switch (num)
    {
            /* error happened, probably null vmbuf */
        case -1:
        {
            free ((char *) vmbuf);
            fprintf (stderr,"%s errno %d\n", strerror (errno), errno);
            return -1;
        }

            /* 'num' vmount structs returned in vmbuf */
        default:
        {
            diCount = num;
            diskInfo = (DiskInfo *) calloc (sizeof (DiskInfo), diCount);
            if (diskInfo == (DiskInfo *) NULL)
            {
                fprintf (stderr, "malloc failed for diskInfo. errno %d\n", errno);
                return -1;
            }

            bufp = vmbuf;
            for (i = 0; i < num; i++)
            {
                vmtp = (struct vmount *) bufp;
                diskInfo [i].printFlag = DI_OK;
                diskInfo [i].isLocal = TRUE;

                strncpy (diskInfo [i].special,
                        (char *) vmt2dataptr (vmtp, VMT_OBJECT), SPEC_NAME_LEN);
                strncpy (diskInfo [i].name,
                        (char *) vmt2dataptr (vmtp, VMT_STUB), NAME_LEN);

                ve = getvfsbytype (vmtp->vmt_gfstype);
                if (ve == (struct vfs_ent *) NULL || *ve->vfsent_name == '\0')
                {
                    if (vmtp->vmt_gfstype >= 0 &&
                            (vmtp->vmt_gfstype < NUM_AIX_FSTYPES))
                    {
                        strncpy (diskInfo [i].fsType,
                                AIX_fsType [vmtp->vmt_gfstype], TYPE_LEN);
                    }
                }
                else
                {
                    strncpy (diskInfo [i].fsType, ve->vfsent_name, TYPE_LEN);
                }

                if ((vmtp->vmt_flags & MNT_READONLY) == MNT_READONLY)
                {
                    strcat (diskInfo [i].options, "ro,");
                }
                else
                {
                    strcat (diskInfo [i].options, "rw,");
                }
                if ((vmtp->vmt_flags & MNT_NOSUID) != MNT_NOSUID)
                {
                    strcat (diskInfo [i].options, "suid,");
                }
                if ((vmtp->vmt_flags & MNT_REMOVABLE) == MNT_REMOVABLE)
                {
                    strcat (diskInfo [i].options, "removable,");
                }
                if ((vmtp->vmt_flags & MNT_DEVICE) == MNT_DEVICE)
                {
                    strcat (diskInfo [i].options, "device,");
                }
                if ((vmtp->vmt_flags & MNT_REMOTE) == MNT_REMOTE)
                {
                    strcat (diskInfo [i].options, "remote,");
                    diskInfo [i].isLocal = FALSE;
                }
                if ((vmtp->vmt_flags & MNT_UNMOUNTING) == MNT_UNMOUNTING)
                {
                    strcat (diskInfo [i].options, "unmounting,");
                }
                if ((vmtp->vmt_flags & MNT_SYSV_MOUNT) == MNT_SYSV_MOUNT)
                {
                    strcat (diskInfo [i].options, "sysv mount,");
                }
                if ((vmtp->vmt_flags & MNT_NODEV) == MNT_NODEV)
                {
                    strcat (diskInfo [i].options, "nodev,");
                }

                    /* remove trailing comma */
                len = strlen (diskInfo [i].options);
                if (len > 0)
                {
                    --len;
                }
                if (diskInfo [i].options [len] == ',')
                {
                    diskInfo [i].options [len] = '\0';
                }

                strncpy (diskInfo [i].mountTime, ctime (&vmtp->vmt_time),
                        MNT_TIME_LEN);

                if (diskInfo [i].isLocal == FALSE && (flags & F_LOCAL_ONLY) == F_LOCAL_ONLY)
                {
                    diskInfo [i].printFlag = DI_IGNORE;
                }

                bufp += vmtp->vmt_length;
            }

            if (debug > 0)
            {
                printf ("mnt:%s - %s : %s\n", diskInfo [i].name,
                        diskInfo [i].special, diskInfo [i].fsType);
                printf ("\t%s\n", (char *) vmt2dataptr (vmtp, VMT_ARGS));
            }

            break;
        } /* valid value returned */
    } /* switch on num */
}

#endif  /* HAS_MNTCTL */


#if defined (HAS_STATVFS)

/*
 * getDiskInfo
 *
 * SysV.4.  statvfs () returns both the free and available blocks.
 *
 */

static void
getDiskInfo ()
{
    int             i;
    double          mult;
    struct statvfs  statBuf;

    for (i = 0; i < diCount; ++i)
    {
        if (diskInfo [i].isLocal == FALSE && (flags & F_LOCAL_ONLY) == F_LOCAL_ONLY)
        {
            diskInfo [i].printFlag = DI_IGNORE;
        }

        if (diskInfo [i].printFlag == DI_OK || (flags & F_ALL) == F_ALL)
        {
            if (statvfs (diskInfo [i].name, &statBuf) == 0)
            {
                    /* data general DG/UX 5.4R3.00 sometime returns 0   */
                    /* in the fragment size field.                      */
                if (statBuf.f_frsize == 0 && statBuf.f_bsize != 0)
                {
                    mult = (double) (long) statBuf.f_bsize / dispBlockSize;
                }
                else
                {
                    mult = (double) (long) statBuf.f_frsize / dispBlockSize;
                }

                diskInfo [i].totalBlocks = ((double) (_s_fs_size_t) statBuf.f_blocks * mult);
                diskInfo [i].freeBlocks = ((double) (_s_fs_size_t) statBuf.f_bfree * mult);
                diskInfo [i].availBlocks = ((double) (_s_fs_size_t) statBuf.f_bavail * mult);

                diskInfo [i].totalInodes = statBuf.f_files;
                diskInfo [i].freeInodes = statBuf.f_ffree;
                diskInfo [i].availInodes = statBuf.f_favail;

                strncpy (diskInfo [i].fsType, statBuf.f_basetype, TYPE_LEN);

                if (debug > 1)
                {
                    printf ("%s: %s\n", diskInfo [i].name, diskInfo [i].fsType);
                    printf ("\tmult:%f\n", mult);
                    printf ("\tbsize:%ld  frsize:%ld\n", statBuf.f_bsize,
                            statBuf.f_frsize);
#if defined (_LARGEFILE_SOURCE) && _FILE_OFFSET_BITS == 64
                    printf ("\tblocks: tot:%lld free:%lld avail:%lld\n",
                            statBuf.f_blocks, statBuf.f_bfree, statBuf.f_bavail);
                    printf ("\tinodes: tot:%lld free:%lld avail:%lld\n",
                            statBuf.f_files, statBuf.f_ffree, statBuf.f_favail);
#else
                    printf ("\tblocks: tot:%ld free:%ld avail:%ld\n",
                            statBuf.f_blocks, statBuf.f_bfree, statBuf.f_bavail);
                    printf ("\tinodes: tot:%ld free:%ld avail:%ld\n",
                            statBuf.f_files, statBuf.f_ffree, statBuf.f_favail);
#endif
                }
            }
            else
            {
                perror (diskInfo [i].name);
            }
        }
    } /* for each entry */
}

#endif /* HAS_STATVFS */


#if defined (HAS_STATFS)

/*
 * getDiskInfo
 *
 * SunOS/BSD/Pyramid
 *
 */

static void
getDiskInfo ()
{
    int             i;
    double          mult;
    struct statfs   statBuf;

    for (i = 0; i < diCount; ++i)
    {
        if (diskInfo [i].printFlag == DI_OK || (flags & F_ALL) == F_ALL)
        {
            if (statfs (diskInfo [i].name, &statBuf) == 0)
            {
                mult = (double) statBuf.f_bsize / dispBlockSize;
                diskInfo [i].totalBlocks = ((double) (_s_fs_size_t) statBuf.f_blocks * mult);
                diskInfo [i].freeBlocks = ((double) (_s_fs_size_t) statBuf.f_bfree * mult);
                diskInfo [i].availBlocks = ((double) (_s_fs_size_t) statBuf.f_bavail * mult);

                diskInfo [i].totalInodes = statBuf.f_files;
                diskInfo [i].freeInodes = statBuf.f_ffree;
                diskInfo [i].availInodes = statBuf.f_ffree;
# if defined (HAS_SYSFS)
                sysfs (GETFSTYP, statBuf.f_fstyp, diskInfo [i].fsType);
# endif

                if (debug > 1)
                {
                    printf ("%s: %s\n", diskInfo [i].name, diskInfo [i].fsType);
                    printf ("\tmult:%f\n", mult);
                    printf ("\tbsize:%ld\n", statBuf.f_bsize);
                    printf ("\tblocks: tot:%ld free:%ld avail:%ld\n",
                            statBuf.f_blocks, statBuf.f_bfree, statBuf.f_bavail);
                    printf ("\tinodes: tot:%ld free:%ld\n",
                            statBuf.f_files, statBuf.f_ffree);
                }
            } /* if we got the info */
            else
            {
                perror (diskInfo [i].name);
            }
        }
    } /* for each entry */
}

#endif /* HAS_STATFS */

#if defined (HAS_SYSV_STATFS)

/*
 * getDiskInfo
 *
 * SysV.3.  We don't have available blocks; just set it to free blocks.
 * The sysfs () call is used to get the disk type name.
 *
 */

static void
getDiskInfo ()
{
    int             i;
    double          mult;
    struct statfs   statBuf;

    for (i = 0; i < diCount; ++i)
    {
        if (diskInfo [i].printFlag == DI_OK || (flags & F_ALL) == F_ALL)
        {
            if (statfs (diskInfo [i].name, &statBuf, sizeof (statBuf), 0) == 0)
            {
                mult = (double) UBSIZE / dispBlockSize;
                diskInfo [i].totalBlocks = ((double) (_s_fs_size_t) statBuf.f_blocks * mult);
                diskInfo [i].freeBlocks = ((double) (_s_fs_size_t) statBuf.f_bfree * mult);
                diskInfo [i].availBlocks = ((double) (_s_fs_size_t) statBuf.f_bfree * mult);

                diskInfo [i].totalInodes = statBuf.f_files;
                diskInfo [i].freeInodes = statBuf.f_ffree;
                diskInfo [i].availInodes = statBuf.f_ffree;
# if defined (HAS_SYSFS)
                sysfs (GETFSTYP, statBuf.f_fstyp, diskInfo [i].fsType);
# endif

                if (debug > 1)
                {
                    printf ("%s: %s\n", diskInfo [i].name, diskInfo [i].fsType);
                    printf ("\tmult:%f\n", mult);
                    printf ("\tUBSIZE:%ld\n", UBSIZE);
# if defined (HAS_FRSIZE)
                    printf ("\tbsize:%ld\n", statBuf.f_bsize);
                    printf ("\tfrsize:%ld\n", statBuf.f_frsize);
# endif
                    printf ("\tblocks: tot:%ld free:%ld\n",
                            statBuf.f_blocks, statBuf.f_bfree);
                    printf ("\tinodes: tot:%ld free:%ld\n",
                            statBuf.f_files, statBuf.f_ffree);
                }
            } /* if we got the info */
            else
            {
                perror (diskInfo [i].name);
            }
        }
    } /* for each entry */
}

#endif /* HAS_SYSV_STATFS */


#if defined (HAS_GETDISKFREESPACE)


/*
 * getDiskEntries
 *
 * MS-DOS uses a long for the available local drives.
 *
 */

static int
getDiskEntries ()
{
    return 0;
}


/*
 * getDiskInfo
 *
 * MS-DOS.
 *
 */

# define NUM_MSDOS_FSTYPES          7
static char *MSDOS_fsType [NUM_MSDOS_FSTYPES] =
    { "unknown", "", "removable", "fixed", "remote", "cdrom", "ramdisk" };
# define MSDOS_BUFFER_SIZE          128
# define BYTES_PER_LOGICAL_DRIVE    4

static void
getDiskInfo ()
{
    int                 i;
    int                 rc;
    double              mult;
    unsigned long       sectorspercluster;
    unsigned long       bytespersector;
    unsigned long       totalclusters;
    unsigned long       freeclusters;
    char                buff [MSDOS_BUFFER_SIZE];
    char                *p;


    rc = GetLogicalDriveStrings (MSDOS_BUFFER_SIZE, buff);
    diCount = rc / BYTES_PER_LOGICAL_DRIVE;

    diskInfo = (DiskInfo *) calloc (sizeof (DiskInfo), diCount);
    if (diskInfo == (DiskInfo *) NULL)
    {
        fprintf (stderr, "malloc failed for diskInfo. errno %d\n", errno);
        return;
    }

    for (i = 0; i < diCount; ++i)
    {
        p = buff + (BYTES_PER_LOGICAL_DRIVE * i);
        strcpy (diskInfo [i].name, p);
        rc = GetDriveType (p);
        diskInfo [i].printFlag = DI_OK;
        if (rc == DRIVE_NO_ROOT_DIR)
        {
            diskInfo [i].printFlag = DI_BAD;
        }
        if (rc == DRIVE_REMOVABLE)
        {
            diskInfo [i].printFlag = DI_IGNORE;
        }
        strcpy (diskInfo [i].fsType, MSDOS_fsType [rc]);

        if (diskInfo [i].printFlag == DI_OK || (flags & F_ALL) == F_ALL)
        {
            rc = GetDiskFreeSpace (p, &sectorspercluster, &bytespersector,
                    &freeclusters, &totalclusters);
            if (rc > 0)
            {
                mult = (double) (sectorspercluster *
                        bytespersector) / dispBlockSize;
                diskInfo [i].totalBlocks = ((double) (_s_fs_size_t) totalclusters * mult);
                diskInfo [i].freeBlocks = ((double) (_s_fs_size_t) freeclusters * mult);
                diskInfo [i].availBlocks = ((double) (_s_fs_size_t) freeclusters * mult);

                diskInfo [i].totalInodes = 0;
                diskInfo [i].freeInodes = 0;
                diskInfo [i].availInodes = 0;

                if (debug > 1)
                {
                    printf ("%s: %s\n", diskInfo [i].name, diskInfo [i].fsType);
                    printf ("\ts/c:%ld  b/s:%ld\n", sectorspercluster,
                        bytespersector);
                    printf ("\tmult:%f\n", mult);
                    printf ("\tclusters: tot:%ld free:%ld\n",
                        totalclusters, freeclusters);
                }
            }
            else
            {
                diskInfo [i].printFlag = DI_BAD;
                if (debug)
                {
                    printf ("disk %s; could not get disk space\n", p);
                }
            }
        } /* if printable drive */
    } /* for each mounted drive */
}

#endif /* HAS_GETDISKFREESPACE */


