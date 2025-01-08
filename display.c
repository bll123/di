/*
 * Copyright 1994-2018 Brad Lanam, Walnut Creek, CA
 * Copyright 2023 Brad Lanam, Pleasant Hill, CA
 */

#include "config.h"

#if _hdr_stdio
# include <stdio.h>
#endif
#if _hdr_stdlib
# include <stdlib.h>
#endif
#if _hdr_stdbool
# include <stdbool.h>
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

#include "di.h"
#include "display.h"
#include "options.h"
#include "version.h"

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

#define DI_PERC_FMT             " %%3.0" PRIu64 "%%%% "
#define DI_POSIX_PERC_FMT       "    %%3.0" PRIu64 "%%%% "
#define DI_JUST_LEFT            0
#define DI_JUST_RIGHT           1

typedef struct
{
  dinum_t         low;
  dinum_t         high;
  dinum_t         dbs;        /* display block size */
  const char      *format;
  const char      *suffix;
} sizeTable_t;

#define DI_MEGA_SZTAB   2
const char *sztabsuffix [] = {
  " ", "K", "M", "G", "T", "P", "E", "Z", "Y",
};

#define DI_SIZETAB_SIZE (sizeof (sztabsuffix) / sizeof (const char *))

static sizeTable_t sizeTable [DI_SIZETAB_SIZE];

static void addTotals           (const di_disk_info_t *, di_disk_info_t *, int);
static void getMaxFormatLengths (di_data_t *);
static int  diCompare           (const di_opt_t *, const di_disk_info_t *, unsigned int, unsigned int);
static int  findDispSize        (dinum_t *);
static Size_t istrlen           (const char *);
static char *printInfo          (di_disk_info_t *, di_opt_t *, diOutput_t *);
static char *printSpace         (const di_opt_t *, const diOutput_t *, dinum_t *, int);
static char *processTitles      (di_opt_t *, diOutput_t *);
static char *printPerc          (dinum_t *, dinum_t *, const char *);
static void initSizeTable       (di_opt_t *, diOutput_t *);
static void appendFormatStr     (char *, const char *, char **, Size_t *, Size_t *);
static void appendFormatVal     (char *, dinum_t *, char **, Size_t *, Size_t *);
static void append              (const char *, char **, Size_t *, Size_t *);

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
printDiskInfo (di_data_t *di_data)
{
#if 0
    int                 i;
    di_opt_t         *diopts;
    di_disk_info_t        *diskInfo;
    di_disk_info_t        totals;
    char                lastpool [DI_SPEC_NAME_LEN + 1];
    Size_t              lastpoollen = { 0 };
    int                 inpool = { false };
    diOutput_t          *diout;
    char                *out;
    Size_t              outlen;
    Size_t              outcurrlen;
    char                *tout;
    int                 first;
    int                 ishr;


    first = true;
    out = (char *) NULL;
    outlen = 0;
    outcurrlen = 0;
    lastpool[0] = '\0';
    diopts = &di_data->options;
    diout = &di_data->output;
    initSizeTable (diopts, diout);

    if (diopts->printTotals)
    {
        di_init_disk_info (&totals);
        strncpy (totals.name, DI_GT("Total"), (Size_t) DI_NAME_LEN);
        totals.printFlag = DI_PRNT_OK;
    }

    getMaxFormatLengths (di_data);
    tout = processTitles (diopts, diout);
    if (diopts->printHeader) {
      append (tout, &out, &outcurrlen, &outlen);
    }
    free (tout);
    if (diopts->json_output) {
      const char *tjson = "{\n  \"partitions\" : [\n";
      append (tjson, &out, &outcurrlen, &outlen);
    }

    if (diopts->dispBlockSize == DI_DISP_HR ||
        diopts->dispBlockSize == DI_DISP_HR_2)
    {
        --diout->width;
    }

    ishr = diopts->dispBlockSize == DI_DISP_HR ||
      diopts->dispBlockSize == DI_DISP_HR_2;

    if (! ishr &&
        (diopts->dispBlockSize > 0 &&
         diopts->dispBlockSize <= DI_VAL_1024)) {
      if (diopts->csv_output || diopts->json_output) {
        Snprintf1 (diout->blockFormat, sizeof (diout->blockFormat),
            "%%" PRIu64);
      } else {
        Snprintf2 (diout->blockFormat, sizeof (diout->blockFormat),
            "%%%d" PRIu64, (int) diout->width);
      }
    } else {
      if (diopts->csv_output || diopts->json_output) {
        Snprintf1 (diout->blockFormatNR, sizeof (diout->blockFormatNR),
            "%%" PRIu64);
        if (! ishr) {
          Snprintf1 (diout->blockFormat, sizeof (diout->blockFormat),
              "%%.1" PRIu64);
        } else {
          Snprintf1 (diout->blockFormat, sizeof (diout->blockFormat),
              "\"%%.1" PRIu64 "\"");
        }
      } else {
        Snprintf2 (diout->blockFormatNR, sizeof (diout->blockFormatNR),
            "%%%d" PRIu64, (int) diout->width);
        Snprintf2 (diout->blockFormat, sizeof (diout->blockFormat),
            "%%%d.1" PRIu64, (int) diout->width);
      }
    }

    if (diopts->dispBlockSize == DI_DISP_HR ||
        diopts->dispBlockSize == DI_DISP_HR_2)
    {
        ++diout->width;
    }

    if (diopts->csv_output || diopts->json_output) {
      Snprintf1 (diout->inodeFormat, sizeof (diout->inodeFormat),
          "%%%s", PRIu64);
    } else {
      Snprintf2 (diout->inodeFormat, sizeof (diout->inodeFormat),
          "%%%d%s", (int) diout->inodeWidth, PRIu64);
    }

    diskInfo = di_data->diskInfo;
    if (diopts->printTotals)
    {
        if (di_data->haspooledfs && ! di_data->totsorted)
        {
          char tempSortType [DI_SORT_MAX + 1];
              /* in order to find the main pool entries,              */
              /* we must have the array sorted by special device name */
          strncpy (tempSortType, diopts->sortType, DI_SORT_MAX);
          strncpy (diopts->sortType, "s", DI_SORT_MAX);
          sortArray (diopts, diskInfo, di_data->count, DI_TOT_SORT_IDX);
          strncpy (diopts->sortType, tempSortType, DI_SORT_MAX);
          di_data->totsorted = true;
        }

        for (i = 0; i < di_data->count; ++i)
        {
            di_disk_info_t    *dinfo;
            int             ispooled;
            int             startpool;

            ispooled = false;
            startpool = false;
            dinfo = &(diskInfo [diskInfo [i].sortIndex[DI_TOT_SORT_IDX]]);

                /* is it a pooled filesystem type? */
            if (di_data->haspooledfs && di_isPooledFs (dinfo)) {
              ispooled = true;
              if (lastpoollen == 0 ||
                  strncmp (lastpool, dinfo->special, lastpoollen) != 0)
              {
                strncpy (lastpool, dinfo->special, DI_SPEC_NAME_LEN);
                lastpoollen = di_mungePoolName (lastpool);
                inpool = false;
                startpool = true;
                if (strcmp (dinfo->fsType, "null") == 0 &&
                    strcmp (dinfo->special + strlen (dinfo->special) - 5,
                            "00000") != 0) {
                    /* dragonflybsd doesn't have the main pool mounted */
                  inpool = true;
                }
              }
            } else {
              inpool = false;
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
              inpool = true;
            }
        } /* for each entry */
    } /* if the totals are to be printed */

    diskInfo = di_data->diskInfo;
    if (strcmp (diopts->sortType, "n") != 0)
    {
      sortArray (diopts, diskInfo, di_data->count, DI_MAIN_SORT_IDX);
    }

    for (i = 0; i < di_data->count; ++i)
    {
      di_disk_info_t        *dinfo;

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
      first = false;

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
#endif
  return strdup ("dbg\n");
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
#if 0
  char          tdata [1024];

  Snprintf1 (tdata, sizeof(tdata), fmt, val);
  append (tdata, ptr, clen, len);
#endif
}

static void
appendFormatVal (char *fmt, dinum_t *val, char **ptr, Size_t *clen, Size_t *len)
{
#if 0
  char          tdata [1024];

  Snprintf1 (tdata, sizeof(tdata), fmt, val);
  append (tdata, ptr, clen, len);
#endif
}

static void
append (const char *val, char **ptr, Size_t *clen, Size_t *len)
{
#if 0
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
#endif
}

/*
 * printInfo
 *
 * Print the information for a single partition.  Loop through the
 * format string and print the particular items wanted.
 *
 */

static char *
printInfo (di_disk_info_t *diskInfo, di_opt_t *diopts, diOutput_t *diout)
{
#if 0
    dinum_t          used;
    dinum_t          totAvail;
    const char          *ptr;
    char                tfmt[2];
    int                 valid;
    dinum_t             temp;
    int                 idx;
    int                 tidx;
    static char         percFormat [15];
    static int          percInit = false;
    int                 first;
    char                ttext[2];
    char                *out;
    char                *tout;
    const char          *t;
    Size_t              outlen;
    Size_t              outcurrlen;
    int                 i;

    dinum_init (&used);
    dinum_init (&totAvail);
    dinum_init (&temp);

    out = (char *) NULL;
    outlen = 0;
    outcurrlen = 0;

    if (diopts->json_output) {
      t = "    {\n";
      append (t, &out, &outcurrlen, &outlen);
    }

    first = true;
    if (! percInit) {
      if (diopts->json_output) {
        Snprintf1 (percFormat, sizeof(percFormat), "%%" PRIu64);
      } else if (diopts->csv_output) {
        Snprintf1 (percFormat, sizeof(percFormat), "%%" PRIu64 "%%%%");
      } else {
        if (diopts->posix_compat) {
          Snprintf1 (percFormat, sizeof(percFormat), DI_POSIX_PERC_FMT);
        } else {
          Snprintf1 (percFormat, sizeof(percFormat), DI_PERC_FMT);
        }
      }
      percInit = true;
    }
    idx = 0;
    dinum_set_u (&temp, 0);
    if (diopts->dispBlockSize == DI_DISP_HR_2)
    {
      idx = DI_MEGA_SZTAB; /* default */

      ptr = diopts->formatString;
      while (*ptr)
      {
        valid = false;

        switch (*ptr)
        {
          case DI_FMT_BTOT:
          {
              dinum_set (&temp, &diskInfo->total_space);
              valid = true;
              break;
          }

          case DI_FMT_BTOT_AVAIL:
          {
              dinum_set (&temp, &diskInfo->total_space);
              dinum_sub (&temp, &diskInfo->free_space);
              dinum_sub (&temp, &diskInfo->avail_space);
              valid = true;
              break;
          }

          case DI_FMT_BUSED:
          {
              dinum_set (&temp, &diskInfo->total_space);
              dinum_sub (&temp, &diskInfo->free_space);
              valid = true;
              break;
          }

          case DI_FMT_BCUSED:
          {
              dinum_set (&temp, &diskInfo->total_space);
              dinum_set (&temp, &diskInfo->avail_space);
              valid = true;
              break;
          }

          case DI_FMT_BFREE:
          {
              dinum_set (&temp, &diskInfo->free_space);
              valid = true;
              break;
          }

          case DI_FMT_BAVAIL:
          {
              dinum_set (&temp, &diskInfo->avail_space);
              valid = true;
              break;
          }
        }

        if (valid) {
          tidx = findDispSize (&temp);
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
      valid = strstr (DI_FMT_VALID_CHARS, tfmt) == NULL ? false : true;
      if (*ptr == DI_FMT_MOUNT_TIME && diout->maxMntTimeString == 0) {
        valid = false;
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
        first = false;
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
          tout = printSpace (diopts, diout, &diskInfo->total_space, idx);
          append (tout, &out, &outcurrlen, &outlen);
          break;
        }

        case DI_FMT_BTOT_AVAIL:
        {
          dinum_t   tot;
          dinum_t   tval;

          dinum_init (&tot);
          dinum_init (&tval);
          dinum_set (&tot, &diskInfo->total_space);
          dinum_set (&tval, &diskInfo->free_space);
          dinum_sub (&tval, &diskInfo->avail_space);
          dinum_sub (&tot, &tval);
          tout = printSpace (diopts, diout, &tot, idx);
          append (tout, &out, &outcurrlen, &outlen);
          break;
        }

        case DI_FMT_BUSED:
        {
          dinum_t   tval;

          dinum_init (&tval);
          dinum_set (&tval, &diskInfo->total_space);
          dinum_sub (&tval, &diskInfo->free_space);
          tout = printSpace (diopts, diout, &tval, idx);
          append (tout, &out, &outcurrlen, &outlen);
          dinum_clear (&tval);
          break;
        }

        case DI_FMT_BCUSED:
        {
          dinum_t   tval;

          dinum_init (&tval);
          dinum_set (&tval, &diskInfo->total_space);
          dinum_sub (&tval, &diskInfo->avail_space);
          tout = printSpace (diopts, diout, &tval, idx);
          append (tout, &out, &outcurrlen, &outlen);
          dinum_clear (&tval);
          break;
        }

        case DI_FMT_BFREE:
        {
          tout = printSpace (diopts, diout, &diskInfo->free_space, idx);
          append (tout, &out, &outcurrlen, &outlen);
          break;
        }

        case DI_FMT_BAVAIL:
        {
          tout = printSpace (diopts, diout, &diskInfo->avail_space, idx);
          append (tout, &out, &outcurrlen, &outlen);
          break;
        }

        case DI_FMT_BPERC_NAVAIL:
        {
          dinum_set (&used, &diskInfo->total_space);
          dinum_sub (&used, &diskInfo->avail_space);
          dinum_set (&totAvail, &diskInfo->total_space);
          tout = printPerc (&used, &totAvail, percFormat);
          append (tout, &out, &outcurrlen, &outlen);
          break;
        }

        case DI_FMT_BPERC_USED:
        {
          dinum_set (&used, &diskInfo->total_space);
          dinum_sub (&used, &diskInfo->free_space);
          dinum_set (&totAvail, &diskInfo->total_space);
          tout = printPerc (&used, &totAvail, percFormat);
          append (tout, &out, &outcurrlen, &outlen);
          break;
        }

        case DI_FMT_BPERC_BSD:
        {
          dinum_t     tval;

          dinum_init (&tval);;
          dinum_set (&used, &diskInfo->total_space);
          dinum_sub (&used, &diskInfo->free_space);
          dinum_set (&totAvail, &diskInfo->total_space);
          dinum_set (&tval, &diskInfo->free_space);
          dinum_sub (&tval, &diskInfo->avail_space);
          dinum_sub (&totAvail, &tval);
          tout = printPerc (&used, &totAvail, percFormat);
          append (tout, &out, &outcurrlen, &outlen);
          dinum_clear (&tval);
          break;
        }

        case DI_FMT_BPERC_AVAIL:
        {
          dinum_t     bfree;

          dinum_init (&bfree);
          dinum_set (&bfree, &diskInfo->avail_space);
          dinum_set (&totAvail, &diskInfo->total_space);
          tout = printPerc (&bfree, &totAvail, percFormat);
          append (tout, &out, &outcurrlen, &outlen);
          dinum_clear (&bfree);
          break;
        }

        case DI_FMT_BPERC_FREE:
        {
          dinum_t     bfree;

          dinum_init (&bfree);
          dinum_set (&bfree, &diskInfo->free_space);
          dinum_set (&totAvail, &diskInfo->total_space);
          tout = printPerc (&bfree, &totAvail, percFormat);
          append (tout, &out, &outcurrlen, &outlen);

          break;
        }

        case DI_FMT_ITOT:
        {
          appendFormatVal (diout->inodeFormat, &diskInfo->total_inodes,
              &out, &outcurrlen, &outlen);
          break;
        }

        case DI_FMT_IUSED:
        {
          dinum_t     tval;

          dinum_init (&tval);
          dinum_set (&tval, &diskInfo->total_inodes);
          dinum_sub (&tval, &diskInfo->free_inodes);
          appendFormatVal (diout->inodeFormat, &tval,
              &out, &outcurrlen, &outlen);
          dinum_clear (&tval);
          break;
        }

        case DI_FMT_IFREE:
        {
          appendFormatVal (diout->inodeFormat, &diskInfo->free_inodes,
              &out, &outcurrlen, &outlen);
          break;
        }

        case DI_FMT_IPERC:
        {
          dinum_init (&used);
          dinum_set (&used, &diskInfo->total_inodes);
          dinum_sub (&used, &diskInfo->avail_inodes);
          dinum_set (&totAvail, &diskInfo->total_inodes);
          tout = printPerc (&used, &totAvail, percFormat);
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

  dinum_clear (&temp);
  dinum_clear (&used);
  dinum_clear (&totAvail);
  return out;
#endif
  return (strdup ("dbg\n"));
}

static char *
printSpace (const di_opt_t *diopts, const diOutput_t *diout,
             dinum_t *usage, int idx)
{
    static char     tdata [1024];
#if 0
    dinum_t         tdbs;
    double          mult;
    dinum_t         temp;
    const char      *suffix;
    const char      *format;


    suffix = "";
    format = diout->blockFormat;
    dinum_set_u (&tdbs, diopts->dispBlockSize);

    if (diopts->dispBlockSize == DI_DISP_HR) {
      dinum_set (&temp, usage);
      idx = findDispSize (&temp);
    }

    if (diopts->dispBlockSize == DI_DISP_HR ||
        diopts->dispBlockSize == DI_DISP_HR_2) {
      if (idx == -1) {
        dinum_set (&tdbs, &sizeTable [DI_MEGA].dbs);
      } else {
        dinum_set (&tdbs, &sizeTable [idx].dbs);
        format = sizeTable [idx].format;
        suffix = sizeTable [idx].suffix;
      }
    }

//    mult = 1.0 / tdbs;
//    Snprintf2 (tdata, sizeof(tdata), format, usage * mult, suffix);
    return tdata;
#endif
*tdata = '\0';
    return tdata;
}


static int
findDispSize (dinum_t *siz)
{
  for (int i = 0; i < (int) DI_SIZETAB_SIZE; ++i) {
    if (dinum_cmp (siz, &sizeTable [i].low) >= 0 &&
        dinum_cmp (siz, &sizeTable [i].high) < 0) {
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
addTotals (const di_disk_info_t *diskInfo, di_disk_info_t *totals, int inpool)
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
      dinum_t   tval;

      dinum_init (&tval);
      dinum_set (&tval, &diskInfo->total_space);
      dinum_sub (&tval, &diskInfo->free_space);
      dinum_sub (&totals->free_space, &tval);
      dinum_clear (&tval);
    } else {
      /* zfs, old hammer, advfs */
      dinum_t   tval;

      dinum_init (&tval);
      dinum_set (&tval, &diskInfo->total_space);
      dinum_sub (&tval, &diskInfo->free_space);
      dinum_add (&totals->total_space, &tval);
      dinum_set (&tval, &diskInfo->total_inodes);
      dinum_sub (&tval, &diskInfo->free_inodes);
      dinum_add (&totals->total_inodes, &tval);
      dinum_clear (&tval);
    }
  }
  else
  {
    if (debug > 2) {printf ("  tot:not inpool:add all totals\n"); }
    dinum_add (&totals->total_space, &diskInfo->total_space);
    dinum_add (&totals->free_space, &diskInfo->free_space);
    dinum_add (&totals->avail_space, &diskInfo->avail_space);
    dinum_add (&totals->total_inodes, &diskInfo->total_inodes);
    dinum_add (&totals->free_inodes, &diskInfo->free_inodes);
    dinum_add (&totals->avail_inodes, &diskInfo->avail_inodes);
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
processTitles (di_opt_t *diopts, diOutput_t *diout)
{
#if 0
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
    first = true;
    if (diopts->printDebugHeader)
    {
        printf (DI_GT("di version %s    Default Format: %s\n"),
                DI_VERSION, DI_DEFAULT_FORMAT);
    }

    ptr = diopts->formatString;

    while (*ptr)
    {
      valid = true;
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
            valid = false;
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
          first = false;
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
#endif
    return strdup ("dbg-out\n");
}

/*
 * printPerc
 *
 * Calculate and print a percentage using the values and format passed.
 *
 */

static char *
printPerc (dinum_t *used, dinum_t *totAvail, const char *format)
{
  static char   tdata [1024];
#if 0
  double        perc;

  if (dinum_cmp_s (&totAvail, 0) > 0) {
    perc = used / (_print_size_t) totAvail;
    perc *= 100.0;
  } else {
    perc = 0.0;
  }

  Snprintf1 (tdata, sizeof(tdata), format, perc);
#endif
*tdata = '\0';
  return tdata;
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
              temp = dinum_cmp (&d1->avail_space, &d2->avail_space);
              break;
            }
            case DI_SORT_FREE:
            {
              temp = dinum_cmp (&d1->free_space, &d2->free_space);
              break;
            }
            case DI_SORT_TOTAL:
            {
              temp = dinum_cmp (&d1->total_space, &d2->total_space);
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
getMaxFormatLengths (di_data_t *di_data)
{
#if 0
    int             i;
    unsigned int    len;
    diOutput_t      *diout;

    diout = &di_data->output;

        /* this loop gets the max string lengths */
    for (i = 0; i < di_data->count; ++i)
    {
        di_disk_info_t        *dinfo;

        dinfo = &di_data->diskInfo[i];
        if (dinfo->doPrint)
        {
            if (di_data->haspooledfs &&
                (strcmp (dinfo->fsType, "zfs") == 0 ||
                 strcmp (dinfo->fsType, "advfs") == 0))
            {
              di_data->disppooledfs = true;
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
#endif
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
initSizeTable (di_opt_t *diopts, diOutput_t *diout)
{
  for (size_t i = 0; i < DI_SIZETAB_SIZE; ++i) {
    dinum_init (&sizeTable [i].high);
    dinum_init (&sizeTable [i].low);
    dinum_init (&sizeTable [i].dbs);
  }

  /* initialize display size tables */

  dinum_set_u (&sizeTable [0].low, 0);
  dinum_set_u (&sizeTable [0].high, diopts->baseDispSize);
  dinum_set_u (&sizeTable [0].dbs, 1);
  sizeTable [0].format = diout->blockFormatNR;
  sizeTable [0].suffix = sztabsuffix [0];

  dinum_set_u (&sizeTable [1].low, diopts->baseDispSize);
  dinum_set_u (&sizeTable [1].high, diopts->baseDispSize * diopts->baseDispSize);
  dinum_set_u (&sizeTable [1].dbs, diopts->baseDispSize);
  sizeTable [1].format = diout->blockFormat;
  sizeTable [1].suffix = sztabsuffix [1];

  for (size_t i = 2; i < (int) DI_SIZETAB_SIZE; ++i) {
    sizeTable [i].format = diout->blockFormat;
    sizeTable [i].suffix = sztabsuffix [i];
    dinum_set (&sizeTable [i].low, &sizeTable [i - 1].low);
    dinum_mul_u (&sizeTable [i].low, diopts->baseDispSize);
    dinum_set (&sizeTable [i].high, &sizeTable [i - 1].high);
    dinum_mul_u (&sizeTable [i].high, diopts->baseDispSize);
    dinum_set (&sizeTable [i].dbs, &sizeTable [i - 1].dbs);
    dinum_mul_u (&sizeTable [i].dbs, diopts->baseDispSize);
  }
}

