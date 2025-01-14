/*
 * Copyright 1994-2018 Brad Lanam, Walnut Creek, CA
 * Copyright 2023 Brad Lanam, Pleasant Hill, CA
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
    && ! defined (DI_INC_SYS_TYPES_H) /* xenix */
# define DI_INC_SYS_TYPES_H
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
#if _use_mcheck
# include <mcheck.h>
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

typedef struct
{
  const char    fmtChar;
  const char    *displayName;
  const char    *posixName;
  const char    *jsonName;
} formatNames_t;

static formatNames_t formatNames [] =
{
  { DI_FMT_MOUNT,             "Mount", NULL, "mount" },
  { DI_FMT_MOUNT_FULL,        "Mount", "Mounted On", "mount" },
  { DI_FMT_SPECIAL,           "Filesystem", NULL, "filesystem" },
  { DI_FMT_SPECIAL_FULL,      "Filesystem", NULL, "filesystem" },
  { DI_FMT_TYPE,              "fsType", NULL, "fstype" },
  { DI_FMT_TYPE_FULL,         "fs Type", NULL, "fstype" },
  { DI_FMT_BTOT,              NULL, NULL, "size" },
  { DI_FMT_BTOT_AVAIL,        NULL, NULL, "size" },
  { DI_FMT_BUSED,             "Used", NULL, "used" },
  { DI_FMT_BCUSED,            "Used", NULL, "used" },
  { DI_FMT_BFREE,             "Free", NULL, "free" },
  { DI_FMT_BAVAIL,            "Avail", "Available", "available" },
  { DI_FMT_BPERC_NAVAIL,      "%Used", "Capacity", "percused" },
  { DI_FMT_BPERC_USED,        "%Used", "Capacity", "percused" },
  { DI_FMT_BPERC_BSD,         "%Used", "Capacity", "percused" },
  { DI_FMT_BPERC_AVAIL,       "%Free", NULL, "percfree" },
  { DI_FMT_BPERC_FREE,        "%Free", NULL, "percfree" },
  { DI_FMT_ITOT,              "Inodes", NULL, "inodes" },
  { DI_FMT_IUSED,             "IUsed", NULL, "inodesused" },
  { DI_FMT_IFREE,             "IFree", NULL, "inodesfree" },
  { DI_FMT_IPERC,             "%IUsed", NULL, "percinodesused" },
  { DI_FMT_MOUNT_TIME,        "Mount Time", NULL, "mounttime" },
  { DI_FMT_MOUNT_OPTIONS,     "Options", NULL, "options" }
};
#define DI_FORMATNAMES_SIZE (sizeof (formatNames) / sizeof (formatNames_t))


#define DI_SORT_NONE            'n'
#define DI_SORT_MOUNT           'm'
#define DI_SORT_SPECIAL         's'
#define DI_SORT_TOTAL           'T'
#define DI_SORT_FREE            'f'
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
    const char      *format;
    const char      *suffix;
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

static void addTotals           _((const diDiskInfo_t *, diDiskInfo_t *, int));
static void getMaxFormatLengths _((diData_t *));
static int  diCompare           _((const diOptions_t *, const diDiskInfo_t *, unsigned int, unsigned int));
static int  findDispSize        _((_print_size_t));
static Size_t istrlen           _((const char *));
static char *printInfo          _((diDiskInfo_t *, diOptions_t *, diOutput_t *));
static char *printSpace         _((const diOptions_t *, const diOutput_t *, _fs_size_t, int));
static char *processTitles      _((diOptions_t *, diOutput_t *));
static char *printPerc          _((_fs_size_t, _fs_size_t, const char *));
static void initSizeTable       _((diOptions_t *, diOutput_t *));
static void appendFormatStr     _((char *, const char *, char **, Size_t *, Size_t *));
static void appendFormatVal     _((char *, _fs_size_t, char **, Size_t *, Size_t *));
static void append              _((const char *, char **, Size_t *, Size_t *));

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

char *
printDiskInfo (diData_t *diData)
{
    int                 i;
    diOptions_t         *diopts;
    diDiskInfo_t        *diskInfo;
    diDiskInfo_t        totals;
    char                lastpool [DI_SPEC_NAME_LEN + 1];
    Size_t              lastpoollen = { 0 };
    int                 inpool = { FALSE };
    diOutput_t          *diout;
    char                *out;
    Size_t              outlen;
    Size_t              outcurrlen;
    char                *tout;
    int                 first;
    int                 ishr;


    first = TRUE;
    out = (char *) NULL;
    outlen = 0;
    outcurrlen = 0;
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
    tout = processTitles (diopts, diout);
    if (diopts->printHeader) {
      append (tout, &out, &outcurrlen, &outlen);
    }
    free (tout);
    if (diopts->json_output) {
      const char *tjson = "{\n  \"partitions\" : [\n";
      append (tjson, &out, &outcurrlen, &outlen);
    }

    if (diopts->dispBlockSize == (_print_size_t) DI_DISP_HR ||
        diopts->dispBlockSize == (_print_size_t) DI_DISP_HR_2)
    {
        --diout->width;
    }

    ishr = diopts->dispBlockSize == (_print_size_t) DI_DISP_HR ||
      diopts->dispBlockSize == (_print_size_t) DI_DISP_HR_2;

    if (! ishr &&
        (diopts->dispBlockSize > 0 &&
         diopts->dispBlockSize <= (_print_size_t) DI_VAL_1024)) {
      if (diopts->csv_output || diopts->json_output) {
        Snprintf1 (diout->blockFormat, sizeof (diout->blockFormat),
            "%%.0%s%%s", DI_Lf);
      } else {
        Snprintf2 (diout->blockFormat, sizeof (diout->blockFormat),
            "%%%d.0%s%%s", (int) diout->width, DI_Lf);
      }
    } else {
      if (diopts->csv_output || diopts->json_output) {
        Snprintf1 (diout->blockFormatNR, sizeof (diout->blockFormatNR),
            "%%.0%s%%s", DI_Lf);
        if (! ishr) {
          Snprintf1 (diout->blockFormat, sizeof (diout->blockFormat),
              "%%.1%s%%s", DI_Lf);
        } else {
          Snprintf1 (diout->blockFormat, sizeof (diout->blockFormat),
              "\"%%.1%s%%s\"", DI_Lf);
        }
      } else {
        Snprintf2 (diout->blockFormatNR, sizeof (diout->blockFormatNR),
            "%%%d.0%s%%s", (int) diout->width, DI_Lf);
        Snprintf2 (diout->blockFormat, sizeof (diout->blockFormat),
            "%%%d.1%s%%s", (int) diout->width, DI_Lf);
      }
    }

    if (diopts->dispBlockSize == (_print_size_t) DI_DISP_HR ||
        diopts->dispBlockSize == (_print_size_t) DI_DISP_HR_2)
    {
        ++diout->width;
    }

    if (diopts->csv_output || diopts->json_output) {
      Snprintf1 (diout->inodeFormat, sizeof (diout->inodeFormat),
          "%%%s", DI_LLu);
    } else {
      Snprintf2 (diout->inodeFormat, sizeof (diout->inodeFormat),
          "%%%d%s", (int) diout->inodeWidth, DI_LLu);
    }

    diskInfo = diData->diskInfo;
    if (diopts->printTotals)
    {
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
                strncpy (lastpool, dinfo->special, DI_SPEC_NAME_LEN);
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

            if (dinfo->doPrint) {
              addTotals (dinfo, &totals, inpool);
            } else {
              if (debug > 2) {
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

      if (! first && diopts->json_output) {
        append (",\n", &out, &outcurrlen, &outlen);
      }
      first = FALSE;

      tout = printInfo (dinfo, diopts, diout);
      append (tout, &out, &outcurrlen, &outlen);
      free (tout);
    }

    if (diopts->json_output) {
      append ("\n", &out, &outcurrlen, &outlen);
    }

    if (diopts->printTotals)
    {
      tout = printInfo (&totals, diopts, diout);
      append (tout, &out, &outcurrlen, &outlen);
      free (tout);
    }

    if (diopts->json_output) {
      const char *tjson = "  ]\n}\n";
      append (tjson, &out, &outcurrlen, &outlen);
    }

    return out;
}

/*
 * sortArray
 *
 */
void
sortArray (diOptions_t *diopts, diDiskInfo_t *data, int count, int sidx)
{
  unsigned int  tempIndex;
  int           gap;
  int           j;
  int           i;

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


static void
appendFormatStr (char *fmt, const char *val, char **ptr, Size_t *clen, Size_t *len)
{
  char          tdata [1024];

  Snprintf1 (tdata, sizeof(tdata), fmt, val);
  append (tdata, ptr, clen, len);
}

static void
appendFormatVal (char *fmt, _fs_size_t val, char **ptr, Size_t *clen, Size_t *len)
{
  char          tdata [1024];

  Snprintf1 (tdata, sizeof(tdata), fmt, val);
  append (tdata, ptr, clen, len);
}

static void
append (const char *val, char **ptr, Size_t *clen, Size_t *len)
{
  Size_t    vlen;
  Size_t    bumplen;
  Size_t    nlen;

  if (val == (char *) NULL) {
    return;
  }

  bumplen = 100;
  vlen = strlen (val);
  if (*clen + vlen >= *len) {
    nlen = vlen + *clen + 1;
    *ptr = (char *) realloc (*ptr, nlen);
    memset (*ptr+*clen, 0, nlen-*clen);
    *len = nlen;
  }
  strcat (*ptr, val);
  *clen += vlen;
}

/*
 * printInfo
 *
 * Print the information for a single partition.  Loop through the
 * format string and print the particular items wanted.
 *
 */

static char *
printInfo (diDiskInfo_t *diskInfo, diOptions_t *diopts, diOutput_t *diout)
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
    char                ttext[2];
    char                *out;
    char                *tout;
    const char          *t;
    Size_t              outlen;
    Size_t              outcurrlen;
    int                 i;

    out = (char *) NULL;
    outlen = 0;
    outcurrlen = 0;

    if (diopts->json_output) {
      t = "    {\n";
      append (t, &out, &outcurrlen, &outlen);
    }

    first = TRUE;
    if (! percInit) {
      if (diopts->json_output) {
        Snprintf1 (percFormat, sizeof(percFormat), "%%.0%s", DI_Lf);
      } else if (diopts->csv_output) {
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
    temp = (_print_size_t) 0.0;  /* gcc compile warning */
    if (diopts->dispBlockSize == (_print_size_t) DI_DISP_HR_2)
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

      if (valid && (diopts->csv_output || diopts->json_output)) {
        if (! first) {
          t = ",";
          if (diopts->csv_tabs) {
            t = "	"; /* tab here */
          }
          if (diopts->json_output) {
            t = ",\n";
          }
          append (t, &out, &outcurrlen, &outlen);
        }
        first = FALSE;
      }

      if (valid && diopts->json_output) {
        for (i = 0; i < (int) DI_FORMATNAMES_SIZE; ++i) {
          if (*ptr == formatNames[i].fmtChar) {
            t = "      \"";
            append (t, &out, &outcurrlen, &outlen);
            t = formatNames[i].jsonName;
            append (t, &out, &outcurrlen, &outlen);
            t = "\" : ";
            append (t, &out, &outcurrlen, &outlen);
            break;
          }
        }
      }

      switch (*ptr)
      {
        case DI_FMT_MOUNT:
        case DI_FMT_MOUNT_FULL:
        {
          appendFormatStr (diout->mountFormat, diskInfo->name, &out, &outcurrlen, &outlen);
          break;
        }

        case DI_FMT_BTOT:
        {
          tout = printSpace (diopts, diout, diskInfo->totalSpace, idx);
          append (tout, &out, &outcurrlen, &outlen);
          break;
        }

        case DI_FMT_BTOT_AVAIL:
        {
          tout = printSpace (diopts, diout, diskInfo->totalSpace -
              (diskInfo->freeSpace - diskInfo->availSpace), idx);
          append (tout, &out, &outcurrlen, &outlen);
          break;
        }

        case DI_FMT_BUSED:
        {
          tout = printSpace (diopts, diout,
              diskInfo->totalSpace - diskInfo->freeSpace, idx);
          append (tout, &out, &outcurrlen, &outlen);
          break;
        }

        case DI_FMT_BCUSED:
        {
          tout = printSpace (diopts, diout,
              diskInfo->totalSpace - diskInfo->availSpace, idx);
          append (tout, &out, &outcurrlen, &outlen);
          break;
        }

        case DI_FMT_BFREE:
        {
          tout = printSpace (diopts, diout, diskInfo->freeSpace, idx);
          append (tout, &out, &outcurrlen, &outlen);
          break;
        }

        case DI_FMT_BAVAIL:
        {
          tout = printSpace (diopts, diout, diskInfo->availSpace, idx);
          append (tout, &out, &outcurrlen, &outlen);
          break;
        }

        case DI_FMT_BPERC_NAVAIL:
        {
          used = diskInfo->totalSpace - diskInfo->availSpace;
          totAvail = diskInfo->totalSpace;
          tout = printPerc (used, totAvail, percFormat);
          append (tout, &out, &outcurrlen, &outlen);
          break;
        }

        case DI_FMT_BPERC_USED:
        {
          used = diskInfo->totalSpace - diskInfo->freeSpace;
          totAvail = diskInfo->totalSpace;
          tout = printPerc (used, totAvail, percFormat);
          append (tout, &out, &outcurrlen, &outlen);
          break;
        }

        case DI_FMT_BPERC_BSD:
        {
          used = diskInfo->totalSpace - diskInfo->freeSpace;
          totAvail = diskInfo->totalSpace -
                  (diskInfo->freeSpace - diskInfo->availSpace);
          tout = printPerc (used, totAvail, percFormat);
          append (tout, &out, &outcurrlen, &outlen);
          break;
        }

        case DI_FMT_BPERC_AVAIL:
        {
          _fs_size_t          bfree;
          bfree = diskInfo->availSpace;
          totAvail = diskInfo->totalSpace;
          tout = printPerc (bfree, totAvail, percFormat);
          append (tout, &out, &outcurrlen, &outlen);
          break;
        }

        case DI_FMT_BPERC_FREE:
        {
          _fs_size_t          bfree;
          bfree = diskInfo->freeSpace;
          totAvail = diskInfo->totalSpace;
          tout = printPerc (bfree, totAvail, percFormat);
          append (tout, &out, &outcurrlen, &outlen);
          break;
        }

        case DI_FMT_ITOT:
        {
          appendFormatVal (diout->inodeFormat, diskInfo->totalInodes,
              &out, &outcurrlen, &outlen);
          break;
        }

        case DI_FMT_IUSED:
        {
          appendFormatVal (diout->inodeFormat,
              diskInfo->totalInodes - diskInfo->freeInodes,
              &out, &outcurrlen, &outlen);
          break;
        }

        case DI_FMT_IFREE:
        {
          appendFormatVal (diout->inodeFormat, diskInfo->freeInodes,
              &out, &outcurrlen, &outlen);
          break;
        }

        case DI_FMT_IPERC:
        {
          used = diskInfo->totalInodes - diskInfo->availInodes;
          totAvail = diskInfo->totalInodes;
          tout = printPerc (used, totAvail, percFormat);
          append (tout, &out, &outcurrlen, &outlen);
          break;
        }

        case DI_FMT_SPECIAL:
        case DI_FMT_SPECIAL_FULL:
        {
          appendFormatStr (diout->specialFormat, diskInfo->special,
              &out, &outcurrlen, &outlen);
          break;
        }

        case DI_FMT_TYPE:
        case DI_FMT_TYPE_FULL:
        {
          appendFormatStr (diout->typeFormat, diskInfo->fsType, &out, &outcurrlen, &outlen);
          break;
        }

        case DI_FMT_MOUNT_OPTIONS:
        {
          appendFormatStr (diout->optFormat, diskInfo->options, &out, &outcurrlen, &outlen);
          break;
        }

        case DI_FMT_MOUNT_TIME:
        {
          break;
        }

        default:
        {
          if (! diopts->json_output) {
            ttext[0] = *ptr;
            ttext[1] = '\0';
            append (ttext, &out, &outcurrlen, &outlen);
          }
          break;
        }
      }

      ++ptr;
      if (! diopts->csv_output && ! diopts->json_output && *ptr && valid)
      {
        append (" ", &out, &outcurrlen, &outlen);
      }
    }

    if (diopts->json_output) {
      append ("\n    }", &out, &outcurrlen, &outlen);
    }
    if (! diopts->json_output && outcurrlen > 0) {
      append ("\n", &out, &outcurrlen, &outlen);
    }
    return out;
}

static char *
printSpace (const diOptions_t *diopts, const diOutput_t *diout,
             _fs_size_t usage, int idx)
{
    _print_size_t   tdbs;
    _print_size_t   mult;
    _print_size_t   temp;
    const char      *suffix;
    const char      *format;
    static char     tdata [1024];


    suffix = "";
    format = diout->blockFormat;
    tdbs = diopts->dispBlockSize;

    if (diopts->dispBlockSize == (_print_size_t) DI_DISP_HR)
    {
        temp = (_print_size_t) usage;
        idx = findDispSize (temp);
    }

    if (diopts->dispBlockSize == (_print_size_t) DI_DISP_HR ||
        diopts->dispBlockSize == (_print_size_t) DI_DISP_HR_2)
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

    mult = (_print_size_t) 1.0 / tdbs;
    Snprintf2 (tdata, sizeof(tdata), format, (_print_size_t) usage * mult, suffix);
    return tdata;
}


static int
findDispSize (_print_size_t siz)
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
addTotals (const diDiskInfo_t *diskInfo, diDiskInfo_t *totals, int inpool)
{
  if (debug > 2)
  {
    printf ("tot:%s:%s:inp:%d\n",
        diskInfo->special, diskInfo->name, inpool);
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

  if (inpool)
  {
    if (debug > 2) {printf ("  tot:inpool:\n"); }
    if (strcmp (diskInfo->fsType, "apfs") == 0) {
      totals->freeSpace -= diskInfo->totalSpace - diskInfo->freeSpace;
    } else {
      /* zfs, old hammer, advfs */
      totals->totalSpace += diskInfo->totalSpace - diskInfo->freeSpace;
      totals->totalInodes += diskInfo->totalInodes - diskInfo->freeInodes;
    }
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

static char *
processTitles (diOptions_t *diopts, diOutput_t *diout)
{
    const char      *ptr;
    int             valid;
    Size_t          wlen;
    Size_t          *wlenptr;
    int             justification;
    const char      *pstr = { "" };
    char            *fstr;
    Size_t          maxsize;
    char            tformat [30];
    char            ttext [2];
    int             first;
    char            *out;
    Size_t          outcurrlen;
    Size_t          outlen;
    int             i;


    out = (char *) NULL;
    outlen = 0;
    outcurrlen = 0;
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

      for (i = 0; i < (int) DI_FORMATNAMES_SIZE; ++i) {
        if (*ptr == formatNames[i].fmtChar) {
          pstr = formatNames[i].displayName;
          if (diopts->posix_compat && formatNames[i].posixName != NULL) {
            pstr = formatNames[i].posixName;
          }
          if (pstr == NULL) {
            pstr = diout->dispBlockLabel;
          }
          break;
        }
      }

      switch (*ptr)
      {
          case DI_FMT_MOUNT:
          {
              wlen = 15;
              wlenptr = &diout->maxMountString;
              fstr = diout->mountFormat;
              maxsize = sizeof (diout->mountFormat) - 1;
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
              maxsize = sizeof (diout->mountFormat) - 1;
              break;
          }

          case DI_FMT_BTOT:
          case DI_FMT_BTOT_AVAIL:
          {
              wlen = diout->width;
              wlenptr = &diout->width;
              justification = DI_JUST_RIGHT;
              break;
          }

          case DI_FMT_BUSED:
          case DI_FMT_BCUSED:
          {
              wlen = diout->width;
              wlenptr = &diout->width;
              justification = DI_JUST_RIGHT;
              break;
          }

          case DI_FMT_BFREE:
          {
              wlen = diout->width;
              wlenptr = &diout->width;
              justification = DI_JUST_RIGHT;
              break;
          }

          case DI_FMT_BAVAIL:
          {
              wlen = diout->width;
              wlenptr = &diout->width;
              justification = DI_JUST_RIGHT;
              break;
          }

          case DI_FMT_BPERC_NAVAIL:
          case DI_FMT_BPERC_USED:
          case DI_FMT_BPERC_BSD:
          {
              if (diopts->posix_compat)
              {
                  wlen = 9;
              }
              else
              {
                  wlen = 6;
              }
              break;
          }

          case DI_FMT_BPERC_AVAIL:
          case DI_FMT_BPERC_FREE:
          {
              wlen = 5;
              break;
          }

          case DI_FMT_ITOT:
          {
              justification = DI_JUST_RIGHT;
              wlen = diout->inodeWidth;
              wlenptr = &diout->inodeWidth;
              fstr = diout->inodeLabelFormat;
              maxsize = sizeof (diout->inodeLabelFormat) - 1;
              break;
          }

          case DI_FMT_IUSED:
          {
              justification = DI_JUST_RIGHT;
              wlen = diout->inodeWidth;
              wlenptr = &diout->inodeWidth;
              fstr = diout->inodeLabelFormat;
              maxsize = sizeof (diout->inodeLabelFormat) - 1;
              break;
          }

          case DI_FMT_IFREE:
          {
              justification = DI_JUST_RIGHT;
              wlen = diout->inodeWidth;
              wlenptr = &diout->inodeWidth;
              fstr = diout->inodeLabelFormat;
              maxsize = sizeof (diout->inodeLabelFormat) - 1;
              break;
          }

          case DI_FMT_IPERC:
          {
              wlen = 6;
              break;
          }

          case DI_FMT_SPECIAL:
          {
              wlen = 18;
              wlenptr = &diout->maxSpecialString;
              fstr = diout->specialFormat;
              maxsize = sizeof (diout->specialFormat) - 1;
              break;
          }

          case DI_FMT_SPECIAL_FULL:
          {
              wlen = diout->maxSpecialString;
              wlenptr = &diout->maxSpecialString;
              fstr = diout->specialFormat;
              maxsize = sizeof (diout->specialFormat) - 1;
              break;
          }

          case DI_FMT_TYPE:
          {
              wlen = 7;
              wlenptr = &diout->maxTypeString;
              fstr = diout->typeFormat;
              maxsize = sizeof (diout->typeFormat) - 1;
              break;
          }

          case DI_FMT_TYPE_FULL:
          {
              wlen = diout->maxTypeString;
              wlenptr = &diout->maxTypeString;
              fstr = diout->typeFormat;
              maxsize = sizeof (diout->typeFormat) - 1;
              break;
          }

          case DI_FMT_MOUNT_OPTIONS:
          {
              wlen = diout->maxOptString;
              wlenptr = &diout->maxOptString;
              fstr = diout->optFormat;
              maxsize = sizeof (diout->optFormat) - 1;
              break;
          }

          case DI_FMT_MOUNT_TIME:
          {
              break;
          }

          default:
          {
            if (! diopts->json_output) {
              ttext[0] = *ptr;
              ttext[1] = '\0';
              append (ttext, &out, &outcurrlen, &outlen);
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

        if (diopts->csv_output) {
          if (! first) {
            if (diopts->csv_tabs) {
              append ("	", &out, &outcurrlen, &outlen); /* tab here */
            } else {
              append (",", &out, &outcurrlen, &outlen);
            }
          }
          first = FALSE;
        }
          /* title handling */
        if (diopts->csv_output) {
          ttext[0] = *ptr;
          ttext[1] = '\0';
          append (ttext, &out, &outcurrlen, &outlen);
        } else {
          appendFormatStr (tformat, pstr, &out, &outcurrlen, &outlen);
        }

        if (fstr != (char *) NULL) {
          if (diopts->csv_output || diopts->json_output) {
            if (diopts->csv_tabs) {
              strncpy (tformat, "%s", sizeof (tformat));
            } else {
              strncpy (tformat, "\"%s\"", sizeof (tformat));
            }
          }
          if (tlen != len) {
            if (! diopts->csv_output) {
              Snprintf3 (tformat, sizeof (tformat), "%%%s%d.%ds",
                  jstr, (int) len, (int) len);
            }
          }
          /* copy the format string to whereever fstr is pointing */
          strncpy (fstr, tformat, maxsize);
        }
        if (wlenptr != (Size_t *) NULL) {
          *wlenptr = wlen;
        }
      }

      ++ptr;
      if (! diopts->csv_output && *ptr && valid)
      {
        append (" ", &out, &outcurrlen, &outlen);
      }
    }

    if (outcurrlen > 0) {
      append ("\n", &out, &outcurrlen, &outlen);
    }
    return out;
}

/*
 * printPerc
 *
 * Calculate and print a percentage using the values and format passed.
 *
 */

static char *
printPerc (_fs_size_t used, _fs_size_t totAvail, const char *format)
{
    _print_size_t   perc;
    static char     tdata [1024];

    if (totAvail > 0L) {
        perc = (_print_size_t) used / (_print_size_t) totAvail;
        perc *= (_print_size_t) 100.0;
    }
    else {
        perc = (_print_size_t) 0.0;
    }

    Snprintf1 (tdata, sizeof(tdata), format, perc);
    return tdata;
}


static int
diCompare (const diOptions_t *diopts, const diDiskInfo_t *data,
           unsigned int idx1, unsigned int idx2)
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
        case DI_SORT_FREE:
        case DI_SORT_TOTAL:
        {
          int   temp;

          temp = 0;
          switch (*ptr) {
            case DI_SORT_AVAIL:
            {
              temp = d1->availSpace == d2->availSpace ? 0 :
                  d1->availSpace < d2->availSpace ? -1 : 1;
              break;
            }
            case DI_SORT_FREE:
            {
              temp = d1->freeSpace == d2->freeSpace ? 0 :
                  d1->freeSpace < d2->freeSpace ? -1 : 1;
              break;
            }
            case DI_SORT_TOTAL:
            {
              temp = d1->totalSpace == d2->totalSpace ? 0 :
                  d1->totalSpace < d2->totalSpace ? -1 : 1;
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

static void
getMaxFormatLengths (diData_t *diData)
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
istrlen (const char *str)
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
initSizeTable (diOptions_t *diopts, diOutput_t *diout)
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

