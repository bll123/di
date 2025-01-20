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
#include "distrutils.h"
#include "display.h"
#include "dioptions.h"
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
  { DI_FMT_TYPE,              "fstype", NULL, "fstype" },
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
  { DI_FMT_MOUNT_OPTIONS,     "Options", NULL, "options" }
};
#define DI_FORMATNAMES_SIZE (sizeof (formatNames) / sizeof (formatNames_t))


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

#if 0
static void addTotals           (const di_disk_info_t *, di_disk_info_t *, int);
static void getMaxFormatLengths (di_data_t *);
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
#endif

void
di_display_data (void *di_data)
{
  return;
}

#if 0
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
printDiskInfo (void *di_data)
{
    int                 i;
    di_opt_t         *diopts;
    di_disk_info_t        *diskInfo;
    di_disk_info_t        totals;
    char                lastpool [DI_FILESYSTEM_LEN + 1];
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
    lastpool [0] = '\0';
    diopts = &di_data->strdata [DI_DISP_MOUNTOPT];
    diout = &di_data->output;
    initSizeTable (diopts, diout);

    if (diopts->optval [DI_OPT_DISP_TOTALS) {
      di_initialize_disk_info (&totals, 0);
      strncpy (totals.name, DI_GT ("Total"), (Size_t) DI_MOUNTPT_LEN);
      totals.printFlag = DI_PRNT_OK;
    }

    getMaxFormatLengths (di_data);
    tout = processTitles (diopts, diout);
    if (diopts->optval [DI_OPT_DISP_HEADER]) {
      append (tout, &out, &outcurrlen, &outlen);
    }
    free (tout);
    if (diopts->optval [DI_OPT_DISP_JSON]]) {
      const char *tjson = "{\n  \"partitions\" : [\n";
      append (tjson, &out, &outcurrlen, &outlen);
    }

    if (diopts->dispBlockSize == DI_SCALE_HR ||
        diopts->dispBlockSize == DI_SCALE_HR_ALT)
    {
        --diout->width;
    }

    ishr = diopts->dispBlockSize == DI_SCALE_HR ||
      diopts->dispBlockSize == DI_SCALE_HR_ALT;

    if (! ishr &&
        (diopts->dispBlockSize > 0 &&
         diopts->dispBlockSize <= DI_BLKSZ_1024)) {
      if (diopts->optval [DI_OPT_DISP_CSV]] || diopts->optval [DI_OPT_DISP_JSON]]) {
        Snprintf1 (diout->blockFormat, sizeof (diout->blockFormat),
            "%%" PRIu64);
      } else {
        Snprintf2 (diout->blockFormat, sizeof (diout->blockFormat),
            "%%%d" PRIu64, (int) diout->width);
      }
    } else {
      if (diopts->optval [DI_OPT_DISP_CSV]] || diopts->optval [DI_OPT_DISP_JSON]]) {
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

    if (diopts->dispBlockSize == DI_SCALE_HR ||
        diopts->dispBlockSize == DI_SCALE_HR_ALT)
    {
        ++diout->width;
    }

    if (diopts->optval [DI_OPT_DISP_CSV]] || diopts->optval [DI_OPT_DISP_JSON]]) {
      Snprintf1 (diout->inodeFormat, sizeof (diout->inodeFormat),
          "%%%s", PRIu64);
    } else {
      Snprintf2 (diout->inodeFormat, sizeof (diout->inodeFormat),
          "%%%d%s", (int) diout->inodeWidth, PRIu64);
    }

    diskInfo = di_data->diskInfo;
    if (diopts->optval [DI_OPT_DISP_TOTALS)
    {
        if (di_data->haspooledfs && ! di_data->totsorted)
        {
          char tempSortType [DI_SORT_MAX + 1];
              /* in order to find the main pool entries,              */
              /* we must have the array sorted by special device name */
          strncpy (tempSortType, diopts->sortType, DI_SORT_MAX);
          strncpy (diopts->sortType, "s", DI_SORT_MAX);
          sortArray (diopts, diskInfo, di_data->fscount, DI_SORT_TOTAL);
          strncpy (diopts->sortType, tempSortType, DI_SORT_MAX);
          di_data->totsorted = true;
        }

        for (i = 0; i < di_data->fscount; ++i)
        {
            di_disk_info_t    *dinfo;
            int             ispooled;
            int             startpool;

            ispooled = false;
            startpool = false;
            dinfo = & (diskInfo [diskInfo [i].sortIndex [DI_SORT_TOTAL]]);

                /* is it a pooled filesystem type? */
            if (di_data->haspooledfs && di_isPooledFs (dinfo)) {
              ispooled = true;
              if (lastpoollen == 0 ||
                  strncmp (lastpool, diptr->strdata [DI_DISP_FILESYSTEM], lastpoollen) != 0)
              {
                strncpy (lastpool, diptr->strdata [DI_DISP_FILESYSTEM], DI_FILESYSTEM_LEN);
                lastpoollen = di_mungePoolName (lastpool);
                inpool = false;
                startpool = true;
                if (strcmp (dinfo->strdata [DI_DISP_FSTYPE], "null") == 0 &&
                    strcmp (diptr->strdata [DI_DISP_FILESYSTEM] + strlen (diptr->strdata [DI_DISP_FILESYSTEM]) - 5,
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
                printf ("tot:%s:%s:skip\n", diptr->strdata [DI_DISP_FILESYSTEM], diptr->strdata [DI_DISP_MOUNTPT]);
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
      sortArray (diopts, diskInfo, di_data->fscount, DI_SORT_MAIN);
    }

    for (i = 0; i < di_data->fscount; ++i)
    {
      di_disk_info_t        *dinfo;

      dinfo = & (diskInfo [diskInfo [i].sortIndex [DI_SORT_MAIN]]);
      if (debug > 5)
      {
        printf ("pdi:%s:%s:%d:\n", diptr->strdata [DI_DISP_MOUNTPT],
            getPrintFlagText ( (int) dinfo->printFlag), dinfo->doPrint);
      }

      if (! dinfo->doPrint)
      {
        continue;
      }

      if (! first && diopts->optval [DI_OPT_DISP_JSON]]) {
        append (",\n", &out, &outcurrlen, &outlen);
      }
      first = false;

      tout = printInfo (dinfo, diopts, diout);
      append (tout, &out, &outcurrlen, &outlen);
      free (tout);
    }

    if (diopts->optval [DI_OPT_DISP_JSON]]) {
      append ("\n", &out, &outcurrlen, &outlen);
    }

    if (diopts->optval [DI_OPT_DISP_TOTALS)
    {
      tout = printInfo (&totals, diopts, diout);
      append (tout, &out, &outcurrlen, &outlen);
      free (tout);
    }

    if (diopts->optval [DI_OPT_DISP_JSON]]) {
      const char *tjson = "  ]\n}\n";
      append (tjson, &out, &outcurrlen, &outlen);
    }

    return out;
  return strdup ("dbg\n");
}

static void
appendFormatStr (char *fmt, const char *val, char **ptr, Size_t *clen, Size_t *len)
{
  char          tdata [1024];

  Snprintf1 (tdata, sizeof (tdata), fmt, val);
  append (tdata, ptr, clen, len);
}

static void
appendFormatVal (char *fmt, dinum_t *val, char **ptr, Size_t *clen, Size_t *len)
{
  char          tdata [1024];

  Snprintf1 (tdata, sizeof (tdata), fmt, val);
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
printInfo (di_disk_info_t *diskInfo, di_opt_t *diopts, diOutput_t *diout)
{
    dinum_t          used;
    dinum_t          totAvail;
    const char          *ptr;
    char                tfmt [2];
    int                 valid;
    dinum_t             temp;
    int                 idx;
    int                 tidx;
    static char         percFormat [15];
    static int          percInit = false;
    int                 first;
    char                ttext [2];
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

    if (diopts->optval [DI_OPT_DISP_JSON]]) {
      t = "    {\n";
      append (t, &out, &outcurrlen, &outlen);
    }

    first = true;
    if (! percInit) {
      if (diopts->optval [DI_OPT_DISP_JSON]]) {
        Snprintf1 (percFormat, sizeof (percFormat), "%%" PRIu64);
      } else if (diopts->optval [DI_OPT_DISP_CSV]]) {
        Snprintf1 (percFormat, sizeof (percFormat), "%%" PRIu64 "%%%%");
      } else {
        if (diopts->optval [DI_OPT_POSIX_COMPAT]) {
          Snprintf1 (percFormat, sizeof (percFormat), DI_POSIX_PERC_FMT);
        } else {
          Snprintf1 (percFormat, sizeof (percFormat), DI_PERC_FMT);
        }
      }
      percInit = true;
    }
    idx = 0;
    dinum_set_u (&temp, 0);
    if (diopts->dispBlockSize == DI_SCALE_HR_ALT)
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
              dinum_set (&temp, &diskInfo->values [DI_SPACE_TOTAL]);
              valid = true;
              break;
          }

          case DI_FMT_BTOT_AVAIL:
          {
              dinum_set (&temp, &diskInfo->values [DI_SPACE_TOTAL]);
              dinum_sub (&temp, &diskInfo->values [DI_SPACE_FREE]);
              dinum_sub (&temp, &diskInfo->values [DI_SPACE_AVAIL]);
              valid = true;
              break;
          }

          case DI_FMT_BUSED:
          {
              dinum_set (&temp, &diskInfo->values [DI_SPACE_TOTAL]);
              dinum_sub (&temp, &diskInfo->values [DI_SPACE_FREE]);
              valid = true;
              break;
          }

          case DI_FMT_BCUSED:
          {
              dinum_set (&temp, &diskInfo->values [DI_SPACE_TOTAL]);
              dinum_set (&temp, &diskInfo->values [DI_SPACE_AVAIL]);
              valid = true;
              break;
          }

          case DI_FMT_BFREE:
          {
              dinum_set (&temp, &diskInfo->values [DI_SPACE_FREE]);
              valid = true;
              break;
          }

          case DI_FMT_BAVAIL:
          {
              dinum_set (&temp, &diskInfo->values [DI_SPACE_AVAIL]);
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
      tfmt [0] = *ptr;
      tfmt [1] = '\0';
      valid = strstr (DI_FMT_VALID_CHARS, tfmt) == NULL ? false : true;
      if (*ptr == DI_FMT_MOUNT_TIME && diout->maxMntTimeString == 0) {
        valid = false;
      }

      if (valid && (diopts->optval [DI_OPT_DISP_CSV]] || diopts->optval [DI_OPT_DISP_JSON]])) {
        if (! first) {
          t = ",";
          if (diopts->optval [DI_OPT_DISP_CSV_TAB]) {
            t = "	"; /* tab here */
          }
          if (diopts->optval [DI_OPT_DISP_JSON]]) {
            t = ",\n";
          }
          append (t, &out, &outcurrlen, &outlen);
        }
        first = false;
      }

      if (valid && diopts->optval [DI_OPT_DISP_JSON]]) {
        for (i = 0; i < (int) DI_FORMATNAMES_SIZE; ++i) {
          if (*ptr == formatNames [i].fmtChar) {
            t = "      \"";
            append (t, &out, &outcurrlen, &outlen);
            t = formatNames [i].jsonName;
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
          appendFormatStr (diout->mountFormat, diskInfo->strdata [DI_DISP_MOUNTPT], &out, &outcurrlen, &outlen);
          break;
        }

        case DI_FMT_BTOT:
        {
          tout = printSpace (diopts, diout, &diskInfo->values [DI_SPACE_TOTAL], idx);
          append (tout, &out, &outcurrlen, &outlen);
          break;
        }

        case DI_FMT_BTOT_AVAIL:
        {
          dinum_t   tot;
          dinum_t   tval;

          dinum_init (&tot);
          dinum_init (&tval);
          dinum_set (&tot, &diskInfo->values [DI_SPACE_TOTAL]);
          dinum_set (&tval, &diskInfo->values [DI_SPACE_FREE]);
          dinum_sub (&tval, &diskInfo->values [DI_SPACE_AVAIL]);
          dinum_sub (&tot, &tval);
          tout = printSpace (diopts, diout, &tot, idx);
          append (tout, &out, &outcurrlen, &outlen);
          break;
        }

        case DI_FMT_BUSED:
        {
          dinum_t   tval;

          dinum_init (&tval);
          dinum_set (&tval, &diskInfo->values [DI_SPACE_TOTAL]);
          dinum_sub (&tval, &diskInfo->values [DI_SPACE_FREE]);
          tout = printSpace (diopts, diout, &tval, idx);
          append (tout, &out, &outcurrlen, &outlen);
          dinum_clear (&tval);
          break;
        }

        case DI_FMT_BCUSED:
        {
          dinum_t   tval;

          dinum_init (&tval);
          dinum_set (&tval, &diskInfo->values [DI_SPACE_TOTAL]);
          dinum_sub (&tval, &diskInfo->values [DI_SPACE_AVAIL]);
          tout = printSpace (diopts, diout, &tval, idx);
          append (tout, &out, &outcurrlen, &outlen);
          dinum_clear (&tval);
          break;
        }

        case DI_FMT_BFREE:
        {
          tout = printSpace (diopts, diout, &diskInfo->values [DI_SPACE_FREE], idx);
          append (tout, &out, &outcurrlen, &outlen);
          break;
        }

        case DI_FMT_BAVAIL:
        {
          tout = printSpace (diopts, diout, &diskInfo->values [DI_SPACE_AVAIL], idx);
          append (tout, &out, &outcurrlen, &outlen);
          break;
        }

        case DI_FMT_BPERC_NAVAIL:
        {
          dinum_set (&used, &diskInfo->values [DI_SPACE_TOTAL]);
          dinum_sub (&used, &diskInfo->values [DI_SPACE_AVAIL]);
          dinum_set (&totAvail, &diskInfo->values [DI_SPACE_TOTAL]);
          tout = printPerc (&used, &totAvail, percFormat);
          append (tout, &out, &outcurrlen, &outlen);
          break;
        }

        case DI_FMT_BPERC_USED:
        {
          dinum_set (&used, &diskInfo->values [DI_SPACE_TOTAL]);
          dinum_sub (&used, &diskInfo->values [DI_SPACE_FREE]);
          dinum_set (&totAvail, &diskInfo->values [DI_SPACE_TOTAL]);
          tout = printPerc (&used, &totAvail, percFormat);
          append (tout, &out, &outcurrlen, &outlen);
          break;
        }

        case DI_FMT_BPERC_BSD:
        {
          dinum_t     tval;

          dinum_init (&tval);;
          dinum_set (&used, &diskInfo->values [DI_SPACE_TOTAL]);
          dinum_sub (&used, &diskInfo->values [DI_SPACE_FREE]);
          dinum_set (&totAvail, &diskInfo->values [DI_SPACE_TOTAL]);
          dinum_set (&tval, &diskInfo->values [DI_SPACE_FREE]);
          dinum_sub (&tval, &diskInfo->values [DI_SPACE_AVAIL]);
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
          dinum_set (&bfree, &diskInfo->values [DI_SPACE_AVAIL]);
          dinum_set (&totAvail, &diskInfo->values [DI_SPACE_TOTAL]);
          tout = printPerc (&bfree, &totAvail, percFormat);
          append (tout, &out, &outcurrlen, &outlen);
          dinum_clear (&bfree);
          break;
        }

        case DI_FMT_BPERC_FREE:
        {
          dinum_t     bfree;

          dinum_init (&bfree);
          dinum_set (&bfree, &diskInfo->values [DI_SPACE_FREE]);
          dinum_set (&totAvail, &diskInfo->values [DI_SPACE_TOTAL]);
          tout = printPerc (&bfree, &totAvail, percFormat);
          append (tout, &out, &outcurrlen, &outlen);

          break;
        }

        case DI_FMT_ITOT:
        {
          appendFormatVal (diout->inodeFormat, &diskInfo->values [DI_INODE_TOTAL],
              &out, &outcurrlen, &outlen);
          break;
        }

        case DI_FMT_IUSED:
        {
          dinum_t     tval;

          dinum_init (&tval);
          dinum_set (&tval, &diskInfo->values [DI_INODE_TOTAL]);
          dinum_sub (&tval, &diskInfo->values [DI_INODE_FREE]);
          appendFormatVal (diout->inodeFormat, &tval,
              &out, &outcurrlen, &outlen);
          dinum_clear (&tval);
          break;
        }

        case DI_FMT_IFREE:
        {
          appendFormatVal (diout->inodeFormat, &diskInfo->values [DI_INODE_FREE],
              &out, &outcurrlen, &outlen);
          break;
        }

        case DI_FMT_IPERC:
        {
          dinum_init (&used);
          dinum_set (&used, &diskInfo->values [DI_INODE_TOTAL]);
          dinum_sub (&used, &diskInfo->values [DI_INODE_AVAIL]);
          dinum_set (&totAvail, &diskInfo->values [DI_INODE_TOTAL]);
          tout = printPerc (&used, &totAvail, percFormat);
          append (tout, &out, &outcurrlen, &outlen);
          break;
        }

        case DI_FMT_SPECIAL:
        case DI_FMT_SPECIAL_FULL:
        {
          appendFormatStr (diout->specialFormat, diskInfo->strdata [DI_DISP_FILESYSTEM],
              &out, &outcurrlen, &outlen);
          break;
        }

        case DI_FMT_TYPE:
        case DI_FMT_TYPE_FULL:
        {
          appendFormatStr (diout->typeFormat, diskInfo->strdata [DI_DISP_FSTYPE], &out, &outcurrlen, &outlen);
          break;
        }

        case DI_FMT_MOUNT_OPTIONS:
        {
          appendFormatStr (diout->optFormat, diskInfo->strdata [DI_DISP_MOUNTOPT], &out, &outcurrlen, &outlen);
          break;
        }

        case DI_FMT_MOUNT_TIME:
        {
          break;
        }

        default:
        {
          if (! diopts->optval [DI_OPT_DISP_JSON]]) {
            ttext [0] = *ptr;
            ttext [1] = '\0';
            append (ttext, &out, &outcurrlen, &outlen);
          }
          break;
        }
      }

      ++ptr;
      if (! diopts->optval [DI_OPT_DISP_CSV]] && ! diopts->optval [DI_OPT_DISP_JSON]] && *ptr && valid)
      {
        append (" ", &out, &outcurrlen, &outlen);
      }
    }

    if (diopts->optval [DI_OPT_DISP_JSON]]) {
      append ("\n    }", &out, &outcurrlen, &outlen);
    }
    if (! diopts->optval [DI_OPT_DISP_JSON]] && outcurrlen > 0) {
      append ("\n", &out, &outcurrlen, &outlen);
    }

  dinum_clear (&temp);
  dinum_clear (&used);
  dinum_clear (&totAvail);
  return out;
  return (strdup ("dbg\n"));
}

static char *
printSpace (const di_opt_t *diopts, const diOutput_t *diout,
             dinum_t *usage, int idx)
{
    static char     tdata [1024];
    dinum_t         tdbs;
    double          mult;
    dinum_t         temp;
    const char      *suffix;
    const char      *format;


    suffix = "";
    format = diout->blockFormat;
    dinum_set_u (&tdbs, diopts->dispBlockSize);

    if (diopts->dispBlockSize == DI_SCALE_HR) {
      dinum_set (&temp, usage);
      idx = findDispSize (&temp);
    }

    if (diopts->dispBlockSize == DI_SCALE_HR ||
        diopts->dispBlockSize == DI_SCALE_HR_ALT) {
      if (idx == -1) {
        dinum_set (&tdbs, &sizeTable [DI_MEGA].dbs);
      } else {
        dinum_set (&tdbs, &sizeTable [idx].dbs);
        format = sizeTable [idx].format;
        suffix = sizeTable [idx].suffix;
      }
    }

//    mult = 1.0 / tdbs;
//    Snprintf2 (tdata, sizeof (tdata), format, usage * mult, suffix);
    return tdata;
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
        diskInfo->strdata [DI_DISP_FILESYSTEM], diskInfo->strdata [DI_DISP_MOUNTPT], inpool);
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
    if (strcmp (diskInfo->strdata [DI_DISP_FSTYPE], "apfs") == 0) {
      dinum_t   tval;

      dinum_init (&tval);
      dinum_set (&tval, &diskInfo->values [DI_SPACE_TOTAL]);
      dinum_sub (&tval, &diskInfo->values [DI_SPACE_FREE]);
      dinum_sub (&totals->values [DI_SPACE_FREE], &tval);
      dinum_clear (&tval);
    } else {
      /* zfs, old hammer, advfs */
      dinum_t   tval;

      dinum_init (&tval);
      dinum_set (&tval, &diskInfo->values [DI_SPACE_TOTAL]);
      dinum_sub (&tval, &diskInfo->values [DI_SPACE_FREE]);
      dinum_add (&totals->values [DI_SPACE_TOTAL], &tval);
      dinum_set (&tval, &diskInfo->values [DI_INODE_TOTAL]);
      dinum_sub (&tval, &diskInfo->values [DI_INODE_FREE]);
      dinum_add (&totals->values [DI_INODE_TOTAL], &tval);
      dinum_clear (&tval);
    }
  }
  else
  {
    if (debug > 2) {printf ("  tot:not inpool:add all totals\n"); }
    dinum_add (&totals->values [DI_SPACE_TOTAL], &diskInfo->values [DI_SPACE_TOTAL]);
    dinum_add (&totals->values [DI_SPACE_FREE], &diskInfo->values [DI_SPACE_FREE]);
    dinum_add (&totals->values [DI_SPACE_AVAIL], &diskInfo->values [DI_SPACE_AVAIL]);
    dinum_add (&totals->values [DI_INODE_TOTAL], &diskInfo->values [DI_INODE_TOTAL]);
    dinum_add (&totals->values [DI_INODE_FREE], &diskInfo->values [DI_INODE_FREE]);
    dinum_add (&totals->values [DI_INODE_AVAIL], &diskInfo->values [DI_INODE_AVAIL]);
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
    if (diopts->optval [DI_OPT_DISP_DBG_HEADER])
    {
        printf (DI_GT ("di version %s    Default Format: %s\n"),
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
        if (*ptr == formatNames [i].fmtChar) {
          pstr = formatNames [i].displayName;
          if (diopts->optval [DI_OPT_POSIX_COMPAT] && formatNames [i].posixName != NULL) {
            pstr = formatNames [i].posixName;
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
              if (diopts->optval [DI_OPT_POSIX_COMPAT])
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
            if (! diopts->optval [DI_OPT_DISP_JSON]]) {
              ttext [0] = *ptr;
              ttext [1] = '\0';
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

        if (diopts->optval [DI_OPT_DISP_CSV]]) {
          if (! first) {
            if (diopts->optval [DI_OPT_DISP_CSV_TAB]) {
              append ("	", &out, &outcurrlen, &outlen); /* tab here */
            } else {
              append (",", &out, &outcurrlen, &outlen);
            }
          }
          first = false;
        }
          /* title handling */
        if (diopts->optval [DI_OPT_DISP_CSV]]) {
          ttext [0] = *ptr;
          ttext [1] = '\0';
          append (ttext, &out, &outcurrlen, &outlen);
        } else {
          appendFormatStr (tformat, pstr, &out, &outcurrlen, &outlen);
        }

        if (fstr != (char *) NULL) {
          if (diopts->optval [DI_OPT_DISP_CSV]] || diopts->optval [DI_OPT_DISP_JSON]]) {
            if (diopts->optval [DI_OPT_DISP_CSV_TAB]) {
              strncpy (tformat, "%s", sizeof (tformat));
            } else {
              strncpy (tformat, "\"%s\"", sizeof (tformat));
            }
          }
          if (tlen != len) {
            if (! diopts->optval [DI_OPT_DISP_CSV]]) {
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
      if (! diopts->optval [DI_OPT_DISP_CSV]] && *ptr && valid)
      {
        append (" ", &out, &outcurrlen, &outlen);
      }
    }

    if (outcurrlen > 0) {
      append ("\n", &out, &outcurrlen, &outlen);
    }
    return out;
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
  double        perc;

  if (dinum_cmp_s (&totAvail, 0) > 0) {
    perc = used / (_print_size_t) totAvail;
    perc *= 100.0;
  } else {
    perc = 0.0;
  }

  Snprintf1 (tdata, sizeof (tdata), format, perc);
*tdata = '\0';
  return tdata;
}


static void
getMaxFormatLengths (di_data_t *di_data)
{
    int             i;
    unsigned int    len;
    diOutput_t      *diout;

    diout = &di_data->output;

        /* this loop gets the max string lengths */
    for (i = 0; i < di_data->fscount; ++i)
    {
        di_disk_info_t        *dinfo;

        dinfo = &di_data->diskInfo [i];
        if (dinfo->doPrint)
        {
            if (di_data->haspooledfs &&
                (strcmp (dinfo->strdata [DI_DISP_FSTYPE], "zfs") == 0 ||
                 strcmp (dinfo->strdata [DI_DISP_FSTYPE], "advfs") == 0))
            {
              di_data->disppooledfs = true;
            }

            len = (unsigned int) strlen (diptr->strdata [DI_DISP_MOUNTPT]);
            if (len > diout->maxMountString)
            {
                diout->maxMountString = len;
            }

            len = (unsigned int) strlen (diptr->strdata [DI_DISP_FILESYSTEM]);
            if (len > diout->maxSpecialString)
            {
                diout->maxSpecialString = len;
            }

            len = (unsigned int) strlen (dinfo->strdata [DI_DISP_FSTYPE]);
            if (len > diout->maxTypeString)
            {
                diout->maxTypeString = len;
            }

            len = (unsigned int) strlen (dinfo->strdata [DI_DISP_MOUNTOPT]);
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
#if _lib_mbrlen
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
    if ( (int) mlen <= 0) {
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
  dinum_set_u (&sizeTable [0].high, diopts->blockSize);
  dinum_set_u (&sizeTable [0].dbs, 1);
  sizeTable [0].format = diout->blockFormatNR;
  sizeTable [0].suffix = sztabsuffix [0];

  dinum_set_u (&sizeTable [1].low, diopts->blockSize);
  dinum_set_u (&sizeTable [1].high, diopts->blockSize * diopts->blockSize);
  dinum_set_u (&sizeTable [1].dbs, diopts->blockSize);
  sizeTable [1].format = diout->blockFormat;
  sizeTable [1].suffix = sztabsuffix [1];

  for (size_t i = 2; i < (int) DI_SIZETAB_SIZE; ++i) {
    sizeTable [i].format = diout->blockFormat;
    sizeTable [i].suffix = sztabsuffix [i];
    dinum_set (&sizeTable [i].low, &sizeTable [i - 1].low);
    dinum_mul_u (&sizeTable [i].low, diopts->blockSize);
    dinum_set (&sizeTable [i].high, &sizeTable [i - 1].high);
    dinum_mul_u (&sizeTable [i].high, diopts->blockSize);
    dinum_set (&sizeTable [i].dbs, &sizeTable [i - 1].dbs);
    dinum_mul_u (&sizeTable [i].dbs, diopts->blockSize);
  }
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
  if ( (localeptr = getenv ("DI_LOCALE_DIR")) == (char *) NULL) {
    localeptr = DI_LOCALE_DIR;
  }
  bindtextdomain ("di", localeptr);
  textdomain ("di");
#endif
}

