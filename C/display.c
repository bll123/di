/*
 * $Id$
 * $Source$
 *
 * Copyright 1994-2013 Brad Lanam, Walnut Creek, CA
 */

#include "config.h"
#include "di.h"
#include "display.h"
#include "options.h"
#include "version.h"

#if _hdr_stdio
# include <stdio.h>
#endif
#if _hdr_stdlib
# include <stdlib.h>
#endif
#if _hdr_ctype
# include <ctype.h>
#endif
#if _sys_types \
    && ! defined (_DI_INC_SYS_TYPES_H) /* xenix */
# define _DI_INC_SYS_TYPES_H
# include <sys/types.h>
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

extern int debug;

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

#define DI_SORT_NONE            'n'
#define DI_SORT_MOUNT           'm'
#define DI_SORT_SPECIAL         's'
#define DI_SORT_AVAIL           'a'
#define DI_SORT_REVERSE         'r'
#define DI_SORT_TYPE            't'
#define DI_SORT_ASCENDING       1

#define DI_PERC_FMT             " %%3.0%s%%%% "
#define DI_POSIX_PERC_FMT       "    %%3.0%s%%%% "
#define DI_JUST_LEFT            0
#define DI_JUST_RIGHT           1

typedef struct
{
    _print_size_t   low;
    _print_size_t   high;
    _print_size_t   dbs;        /* display block size */
    char            *format;
    char            *suffix;
} sizeTable_t;

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

#if defined (__cplusplus) || defined (c_plusplus)
  extern "C" {
#endif

static void addTotals           _((const diDiskInfo_t *, diDiskInfo_t *, int));
static void getMaxFormatLengths _((diData_t *));
static int  diCompare           _((const diOptions_t *, const diDiskInfo_t *, unsigned int, unsigned int));
static int  findDispSize        _((_print_size_t));
static Size_t istrlen           _((const char *));
static void printInfo           _((diDiskInfo_t *, diOptions_t *, diOutput_t *));
static void printSpace          _((const diOptions_t *, const diOutput_t *, _fs_size_t, int));
static void processTitles       _((diOptions_t *, diOutput_t *));
static void printPerc           _((_fs_size_t, _fs_size_t, const char *));
static void initSizeTable       _((diOptions_t *, diOutput_t *));

#if defined (__cplusplus) || defined (c_plusplus)
  }
#endif

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

void
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
    initSizeTable (diopts, diout);

    if (diopts->printTotals)
    {
        di_initDiskInfo (&totals);
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

            ispooled = FALSE;
            startpool = FALSE;
            dinfo = &(diskInfo [diskInfo [i].sortIndex[DI_TOT_SORT_IDX]]);

                /* is it a pooled filesystem type? */
            if (diData->haspooledfs && di_isPooledFs (dinfo)) {
              ispooled = TRUE;
              if (lastpoollen == 0 ||
                  strncmp (lastpool, dinfo->special, lastpoollen) != 0)
              {
                strncpy (lastpool, dinfo->special, sizeof (lastpool));
                lastpoollen = di_mungePoolName (lastpool);
                inpool = FALSE;
                startpool = TRUE;
                if (strcmp (dinfo->fsType, "null") == 0 &&
                    strcmp (dinfo->special + strlen (dinfo->special) - 5,
                            "00000") != 0) {
                    /* dragonflybsd doesn't have the main pool mounted */
                  inpool = TRUE;
                }
              }
            } else {
              inpool = FALSE;
            }

            if (dinfo->doPrint)
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
 * sortArray
 *
 */
void
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

/* for debugging */
const char *
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
              temp = (_print_size_t) diskInfo->totalSpace;
              valid = TRUE;
              break;
          }

          case DI_FMT_BTOT_AVAIL:
          {
              temp = (_print_size_t) (diskInfo->totalSpace -
                      (diskInfo->freeSpace - diskInfo->availSpace));
              valid = TRUE;
              break;
          }

          case DI_FMT_BUSED:
          {
              temp = (_print_size_t) (diskInfo->totalSpace - diskInfo->freeSpace);
              valid = TRUE;
              break;
          }

          case DI_FMT_BCUSED:
          {
              temp = (_print_size_t) (diskInfo->totalSpace - diskInfo->availSpace);
              valid = TRUE;
              break;
          }

          case DI_FMT_BFREE:
          {
              temp = (_print_size_t) diskInfo->freeSpace;
              valid = TRUE;
              break;
          }

          case DI_FMT_BAVAIL:
          {
              temp = (_print_size_t) diskInfo->availSpace;
              valid = TRUE;
              break;
          }
        }

        if (valid) {
          tidx = findDispSize (temp);
            /* want largest index */
          if (tidx > idx) {
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
          printSpace (diopts, diout, diskInfo->totalSpace, idx);
          break;
        }

        case DI_FMT_BTOT_AVAIL:
        {
          printSpace (diopts, diout, diskInfo->totalSpace -
              (diskInfo->freeSpace - diskInfo->availSpace), idx);
          break;
        }

        case DI_FMT_BUSED:
        {
          printSpace (diopts, diout,
              diskInfo->totalSpace - diskInfo->freeSpace, idx);
          break;
        }

        case DI_FMT_BCUSED:
        {
          printSpace (diopts, diout,
              diskInfo->totalSpace - diskInfo->availSpace, idx);
          break;
        }

        case DI_FMT_BFREE:
        {
          printSpace (diopts, diout, diskInfo->freeSpace, idx);
          break;
        }

        case DI_FMT_BAVAIL:
        {
          printSpace (diopts, diout, diskInfo->availSpace, idx);
          break;
        }

        case DI_FMT_BPERC_NAVAIL:
        {
          used = diskInfo->totalSpace - diskInfo->availSpace;
          totAvail = diskInfo->totalSpace;
          printPerc (used, totAvail, percFormat);
          break;
        }

        case DI_FMT_BPERC_USED:
        {
          used = diskInfo->totalSpace - diskInfo->freeSpace;
          totAvail = diskInfo->totalSpace;
          printPerc (used, totAvail, percFormat);
          break;
        }

        case DI_FMT_BPERC_BSD:
        {
          used = diskInfo->totalSpace - diskInfo->freeSpace;
          totAvail = diskInfo->totalSpace -
                  (diskInfo->freeSpace - diskInfo->availSpace);
          printPerc (used, totAvail, percFormat);
          break;
        }

        case DI_FMT_BPERC_AVAIL:
        {
          _fs_size_t          bfree;
          bfree = diskInfo->availSpace;
          totAvail = diskInfo->totalSpace;
          printPerc (bfree, totAvail, percFormat);
          break;
        }

        case DI_FMT_BPERC_FREE:
        {
          _fs_size_t          bfree;
          bfree = diskInfo->freeSpace;
          totAvail = diskInfo->totalSpace;
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
printSpace (const diOptions_t *diopts, const diOutput_t *diout,
             _fs_size_t usage, int idx)
#else
printSpace (diopts, diout, usage, idx)
    const diOptions_t   *diopts;
    const diOutput_t    *diout;
    _fs_size_t          usage;
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
        temp = (_print_size_t) usage;
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

    mult = 1.0 / tdbs;
    printf (format, (_print_size_t) usage * mult, suffix);
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
    const diDiskInfo_t  *diskInfo;
    diDiskInfo_t        *totals;
    int                 inpool;
#endif
{
  if (debug > 2)
  {
    printf ("tot:%s:%s:inp:%d\n",
        diskInfo->special, diskInfo->name, inpool);
  }

  if (inpool)
  {
    if (debug > 2) {printf ("  tot:inpool:add total used\n"); }
    /* if in a pool of disks, add the total used to the totals also */
    totals->totalSpace += diskInfo->totalSpace - diskInfo->freeSpace;
    totals->totalInodes += diskInfo->totalInodes - diskInfo->freeInodes;
  }
  else
  {
    if (debug > 2) {printf ("  tot:not inpool:add all totals\n"); }
    totals->totalSpace += diskInfo->totalSpace;
    totals->freeSpace += diskInfo->freeSpace;
    totals->availSpace += diskInfo->availSpace;
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
    char            *pstr = { "" };
    char            *fstr;
    Size_t          maxsize;
    char            tformat [30];
    char            ttitle [2];
    int             first;


    first = TRUE;
    if (diopts->printDebugHeader)
    {
        printf (DI_GT("di version %s    Default Format: %s\n"),
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
                pstr = "IUsed";
                break;
            }

            case DI_FMT_IFREE:
            {
                justification = DI_JUST_RIGHT;
                wlen = diout->inodeWidth;
                wlenptr = &diout->inodeWidth;
                fstr = diout->inodeLabelFormat;
                maxsize = sizeof (diout->inodeLabelFormat);
                pstr = "IFree";
                break;
            }

            case DI_FMT_IPERC:
            {
                wlen = 6;
                pstr = "%IUsed";
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
          if (fstr != (char *) NULL) {
            if (diopts->csv_output) {
              strncpy (tformat, "\"%s\"", sizeof (tformat));
            }
            if (tlen != len) {
              if (! diopts->csv_output) {
                Snprintf3 (tformat, sizeof (tformat), "%%%s%d.%ds",
                    jstr, (int) len, (int) len);
              }
            }
            strncpy (fstr, tformat, maxsize);
          }
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
          _fs_size_t    temp;

          temp = (d1->availSpace - d2->availSpace);
          if (temp == 0) {
            rc = 0;
          } else {
            rc = temp > 0 ? 1 : -1;
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

static void
#if _proto_stdc
initSizeTable (diOptions_t *diopts, diOutput_t *diout)
#else
initSizeTable (diopts, diout)
  diOptions_t *diopts;
  diOutput_t *diout;
#endif
{
  int       i;

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
}

