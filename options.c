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

#include "di.h"
#include "strutils.h"
#include "getoptn.h"
#include "options.h"
#include "version.h"

struct pa_tmp {
  di_data_t       *di_data;
  di_opt_t        *diopts;
  diOutput_t      *diout;
  char            *dbsstr;
  Size_t          dbsstr_sz;
};

typedef struct
{
  const char      *disp [4];
} dispTable_t;

#define DI_DISP_PREFIX 0
#define DI_DISP_SI_PREFIX 1
#define DI_DISP_UC_LETTER 2
#define DI_DISP_LC_LETTER 3

static dispTable_t dispTable [] =
{
  { { "Kilo", "Kibi", "K", "k" } },
  { { "Mega", "Mebi", "M", "m" } },
  { { "Giga", "Gibi", "G", "g" } },
  { { "Tera", "Tebi", "T", "t" } },
  { { "Peta", "Pebi", "P", "p" } },
  { { "Exa", "Exbi", "E", "e" } },
  { { "Zetta", "Zebi", "Z", "z" } },
  { { "Yotta", "Yobi", "Y", "y" } },
  { { "Ronna", "Ronni", "R", "r" } },
  { { "Quetta", "Quetti", "Q", "q" } }
};
#define DI_DISPTAB_SIZE ((int)(sizeof (dispTable) / sizeof (dispTable_t)))

#define DI_ARGV_SEP             " 	"  /* space, tab */
#define DI_MAX_ARGV             50
#define DI_LIST_SEP             ","

#define DI_POSIX_FORMAT         "SbuvpM"
#define DI_DEF_MOUNT_FORMAT     "MST\n\tO"
#define DI_ALL_FORMAT           "MTS\n\tO\n\tbuf13\n\tbcvpa\n\tBuv2\n\tiUFP"

dinum_t dispSizes [DI_DISPTAB_SIZE];

extern int debug;

static void processStringArgs   (const char *, char *, di_data_t *, char *);
static int  processArgs         (int, char * argv [], di_data_t *, char *, Size_t);
static int  parseList           (di_strarr_t *, char *);
static void processOptions      (const char *, char *);
static void processOptionsVal   (const char *, char *, char *);
static void setDispBlockSize    (char *, di_opt_t *, diOutput_t *);
static void initDisplayTable    (di_opt_t *);
static void setExitFlag         (di_opt_t *, unsigned int);

static void
processStringArgs (const char *progname, char *ptr, di_data_t *di_data,
    char *dbsstr)
{
  char        *dptr;
  char        *tptr;
  int         nargc;
  char        *nargv [DI_MAX_ARGV];
  di_opt_t *diopts;

  if (ptr == (char *) NULL || strcmp (ptr, "") == 0) {
    return;
  }

  diopts = &di_data->options;

  dptr = (char *) NULL;
  dptr = strdup (ptr);
  if (dptr == (char *) NULL) {
    fprintf (stderr, "strdup failed in main() (1).  errno %d\n", errno);
    setExitFlag (diopts, DI_EXIT_FAIL);
    return;
  }
  if (dptr != (char *) NULL) {
    int optidx;

    tptr = strtok (dptr, DI_ARGV_SEP);
    nargc = 1;
    nargv [0] = (char *) progname;
    while (tptr != (char *) NULL) {
      if (nargc >= DI_MAX_ARGV) {
        break;
      }
      nargv [nargc++] = tptr;
      tptr = strtok ((char *) NULL, DI_ARGV_SEP);
    }
    optidx = processArgs (nargc, nargv, di_data, dbsstr, sizeof (dbsstr) - 1);
    if (optidx < nargc) {
      fprintf (stderr, "%s: unknown data found in DI_ARGS: %s\n",
          progname, nargv [optidx]);
      diopts->errorCount += 1;
      if (diopts->errorCount > 0) {
        setExitFlag (diopts, DI_EXIT_WARN);
      }
    }
    free ((char *) dptr);
  }
}

int
di_get_options (int argc, char * argv [], di_data_t *di_data)
{
  const char *      argvptr;
  char *            ptr;
  char              dbsstr [30];
  int               optidx;
  int               ec;
  di_opt_t          *diopts;
  diOutput_t        *diout;

  diopts = &di_data->options;
  diout = &di_data->output;
  strncpy (dbsstr, DI_DEFAULT_DISP_SIZE, sizeof (dbsstr)); /* default */
  ec = 0;

  argvptr = argv [0] + strlen (argv [0]) - 2;
  if (memcmp (argvptr, "mi", (Size_t) 2) == 0) {
    diopts->formatString = DI_DEF_MOUNT_FORMAT;
  }
  else    /* don't use DIFMT env var if running mi. */
  {
    if ((ptr = getenv ("DIFMT")) != (char *) NULL) {
      diopts->formatString = ptr;
    }
  }

  /* gnu df */
  if ((ptr = getenv ("POSIXLY_CORRECT")) != (char *) NULL) {
    strncpy (dbsstr, "512", sizeof (dbsstr));
    diopts->formatString = DI_POSIX_FORMAT;
    diopts->posix_compat = true;
    diopts->csv_output = false;
    diopts->json_output = false;
  }

  /* bsd df */
  if ((ptr = getenv ("BLOCKSIZE")) != (char *) NULL) {
    strncpy (dbsstr, ptr, sizeof (dbsstr) - 1);
  }

  /* gnu df */
  if ((ptr = getenv ("DF_BLOCK_SIZE")) != (char *) NULL) {
    strncpy (dbsstr, ptr, sizeof (dbsstr) - 1);
  }

  if ((ptr = getenv ("DI_ARGS")) != (char *) NULL) {
    processStringArgs (argv [0], ptr, di_data, dbsstr);
  }

  optidx = processArgs (argc, argv, di_data, dbsstr, sizeof (dbsstr) - 1);

  if (debug > 0) {
    int j;

    printf ("# ARGS:");
    for (j = 0; j < argc; ++j) {
      printf (" %s", argv [j]);
    }
    printf ("\n");
    printf ("# blocksize: %s\n", dbsstr);

    if (memcmp (argvptr, "mi", (Size_t) 2) != 0) {
      if ((ptr = getenv ("DIFMT")) != (char *) NULL) {
        printf ("# DIFMT:%s\n", ptr);
      }
    }

    if ((ptr = getenv ("POSIXLY_CORRECT")) != (char *) NULL) {
      printf ("# POSIXLY_CORRECT:%s\n", ptr);
    }
    if ((ptr = getenv ("BLOCKSIZE")) != (char *) NULL) {
      printf ("# BLOCKSIZE:%s\n", ptr);
    }
    if ((ptr = getenv ("DF_BLOCK_SIZE")) != (char *) NULL) {
      printf ("# DF_BLOCK_SIZE:%s\n", ptr);
    }
    if ((ptr = getenv ("DI_ARGS")) != (char *) NULL) {
      printf ("# DI_ARGS:%s\n", ptr);
    }
  }

  initDisplayTable (diopts);
  setDispBlockSize (dbsstr, diopts, diout);

  return optidx;
}

static int
processArgs (int argc, char * argv [], di_data_t *di_data,
    char *dbsstr, Size_t dbsstr_sz)
{
  int           i;
  int           optidx;
  int           errorCount;
  di_opt_t      *diopts;
  diOutput_t    *diout;
  struct pa_tmp padata;

    /* the really old compilers don't have automatic initialization */
  static getoptn_opt_t opts[] = {
/* 0 */
#define OPT_INT_A 0
    { "-A",     GETOPTN_STRPTR,
        NULL  /*&diopts->formatString*/,
        0,
        (void *) DI_ALL_FORMAT },
/* 1 */
#define OPT_INT_a 1
    { "-a",     GETOPTN_FUNC_BOOL,
        NULL  /*&padata*/,
        0,
        NULL  /*processOptions*/ },
/* 2 */
    { "--all",  GETOPTN_ALIAS,
        (void *) "-a",
        0,
        NULL },
/* 3 */
#define OPT_INT_B 3
    { "-B",     GETOPTN_FUNC_VALUE,
        NULL  /*&padata*/,
        0,
        NULL  /*processOptionsVal*/ },
/* 4 */
    { "-b",     GETOPTN_ALIAS,
        (void *) "-B",
        0,
        NULL },
/* 5 */
    { "--block-size",   GETOPTN_ALIAS,
        (void *) "-B",
        0,
        NULL },
/* 6 */
#define OPT_INT_c 6
    { "-c",     GETOPTN_BOOL,
        NULL  /*&diopts->csv_output*/,
        0     /*sizeof(diopts->csv_output)*/,
        NULL },
/* 7 */
#define OPT_INT_C 7
    { "-C",     GETOPTN_BOOL,
        NULL  /*&diopts->csv_tabs*/,
        0     /*sizeof(diopts->csv_tabs)*/,
        NULL },
/* 8 */
    { "--csv-output", GETOPTN_ALIAS,
        (void *) "-c",
        0,
        NULL },
/* 9 */
    { "--csv-tabs", GETOPTN_ALIAS,
        (void *) "-C",
        0,
        NULL },
/* 10 */
#define OPT_INT_d 10
    { "-d",     GETOPTN_STRING,
        NULL  /*dbsstr*/,
        0  /*dbsstr_sz*/,
        NULL },
/* 11 */
    { "--display-size",     GETOPTN_ALIAS,
        (void *) "-d",
        0,
        NULL },
/* 12 */
    { "--dont-resolve-symlink",     GETOPTN_ALIAS,
        (void *) "-R",
        0,
        NULL },
/* 13 */
#define OPT_INT_f 13
    { "-f",     GETOPTN_STRPTR,
        NULL  /*&diopts->formatString*/,
        0,
        NULL },
/* 14 */
    { "--format-string",    GETOPTN_ALIAS,
        (void *) "-f",
        0,
        NULL },
/* 15 */
    { "-F",     GETOPTN_ALIAS,
        (void *) "-I",
        0,
        NULL },
/* 16 */
#define OPT_INT_g 16
    { "-g",     GETOPTN_STRING,
        NULL  /*dbsstr*/,
        0  /*dbsstr_sz*/,
        (void *) "g" },
/* 17 */
#define OPT_INT_h 17
    { "-h",     GETOPTN_STRING,
        NULL  /*dbsstr*/,
        0  /*dbsstr_sz*/,
        (void *) "h" },
/* 18 */
#define OPT_INT_H 18
    { "-H",     GETOPTN_STRING,
        NULL  /*dbsstr*/,
        0  /*dbsstr_sz*/,
        (void *) "H" },
/* 19 */
#define OPT_INT_help 19
    { "--help", GETOPTN_FUNC_BOOL,
        NULL,
        0,
        NULL  /*processOptions*/ },
/* 20 */
    { "--human-readable",   GETOPTN_ALIAS,
        (void *) "-H",
        0,
        NULL },
/* 21 */
#define OPT_INT_qmark 21
    { "-?",     GETOPTN_FUNC_BOOL,
        NULL,
        0,
        NULL  /*processOptions*/ },
/* 22 */
    { "-i",     GETOPTN_ALIAS,
        (void *) "-x",
        0,
        NULL },
/* 23 */
#define OPT_INT_I 23
    { "-I",     GETOPTN_FUNC_VALUE,
        NULL  /*&padata*/,
        0,
        NULL  /*processOptionsVal*/ },
/* 24 */
    { "--inodes",GETOPTN_IGNORE,
        NULL,
        0,
        NULL },
/* 25 */
#define OPT_INT_j 25
    { "-j",     GETOPTN_BOOL,
        NULL  /*&diopts->json_output*/,
        0     /*sizeof(diopts->json_output)*/,
        NULL },
/* 26 */
    { "--json-output", GETOPTN_ALIAS,
        (void *) "-j",
        0,
        NULL },
/* 27 */
#define OPT_INT_k 27
    { "-k",     GETOPTN_STRING,
        NULL  /*dbsstr*/,
        0  /*dbsstr_sz*/,
        (void *) "k" },
/* 28 */
#define OPT_INT_l 28
    { "-l",     GETOPTN_BOOL,
        NULL  /*&diopts->localOnly*/,
        0  /*sizeof (diopts->localOnly)*/,
        NULL },
/* 29 */
    { "--local",GETOPTN_ALIAS,
        (void *) "-l",
        0,
        NULL },
/* 30 */
#define OPT_INT_L 30
    { "-L",     GETOPTN_BOOL,
        NULL  /*&diopts->excludeLoopback*/,
        0  /*sizeof (diopts->excludeLoopback)*/,
        NULL },
/* 31 */
#define OPT_INT_m 31
    { "-m",     GETOPTN_STRING,
        NULL  /*dbsstr*/,
        0  /*dbsstr_sz*/,
        (void *) "m" },
/* 32 */
#define OPT_INT_n 32
    { "-n",     GETOPTN_BOOL,
        NULL  /*&diopts->printHeader*/,
        0  /*sizeof (diopts->printHeader)*/,
        NULL },
/* 33 */
    { "--no-sync",  GETOPTN_IGNORE,
        NULL,
        0,
        NULL },
/* 34 */
#define OPT_INT_P 34
    { "-P",     GETOPTN_FUNC_BOOL,
        NULL  /*&padata*/,
        0,
        NULL  /*processOptions*/ },
/* 35 */
    { "--portability",  GETOPTN_ALIAS,
        (void *) "-P",
        0,
        NULL },
/* 36 */
    { "--print-type",   GETOPTN_IGNORE,
        NULL,
        0,
        NULL },
/* 37 */
#define OPT_INT_q 37
    { "-q",     GETOPTN_BOOL,
        NULL  /*&diopts->quota_check*/,
        0  /*sizeof (diopts->quota_check)*/,
        NULL },
/* 38 */
#define OPT_INT_R 38
    { "-R",     GETOPTN_BOOL,
        NULL  /*&diopts->dontResolveSymlink*/,
        0  /*sizeof (diopts->dontResolveSymlink)*/,
        NULL },
/* 39 */
#define OPT_INT_s 39
    { "-s",     GETOPTN_FUNC_VALUE,
        NULL  /*&padata*/,
        0,
        NULL  /*processOptionsVal*/ },
/* 40 */
#define OPT_INT_si 40
    { "--si",   GETOPTN_FUNC_BOOL,
        NULL  /*&padata*/,
        0,
        NULL  /*processOptions*/ },
/* 41 */
    { "--sync", GETOPTN_IGNORE,
        NULL,
        0,
        NULL },
/* 42 */
#define OPT_INT_t 42
    { "-t",     GETOPTN_BOOL,
        NULL  /*&diopts->printTotals*/,
        0  /*sizeof (diopts->printTotals)*/,
        NULL },
/* 43 */
    { "--total",GETOPTN_ALIAS,
        (void *) "-t",
        0,
        NULL },
/* 44 */
    { "--type", GETOPTN_ALIAS,
        (void *) "-I",
        0,
        NULL },
/* 45 */
    { "-v",     GETOPTN_IGNORE,
        NULL,
        0,
        NULL },
/* 46 */
#define OPT_INT_version 46
    { "--version", GETOPTN_FUNC_BOOL,
        NULL,
        0,
        NULL  /*processOptions*/ },
/* 47 */
#define OPT_INT_w 47
    { "-w",     GETOPTN_SIZET,
        NULL  /*&diout->width*/,
        0  /*sizeof (diout->width)*/,
        NULL },
/* 48 */
#define OPT_INT_W 48
    { "-W",     GETOPTN_SIZET,
        NULL  /*&diout->inodeWidth*/,
        0  /*sizeof (diout->inodeWidth)*/,
        NULL },
/* 49 */
#define OPT_INT_x 49
    { "-x",     GETOPTN_FUNC_VALUE,
        NULL  /*&padata*/,
        0,
        NULL  /*processOptionsVal*/ },
/* 50 */
    { "--exclude-type",     GETOPTN_ALIAS,
        (void *) "-x",
        0,
        NULL },
/* 51 */
#define OPT_INT_X 51
    { "-X",     GETOPTN_FUNC_VALUE,
        NULL  /*&padata*/,
        0,
        NULL  /*processOptionsVal*/ },
/* 52 */
#define OPT_INT_z 52
    { "-z",     GETOPTN_STRING,
        NULL  /*&di_data->zoneInfo.zoneDisplay*/,
        0  /*sizeof (di_data->zoneInfo.zoneDisplay)*/,
        NULL },
/* 53 */
#define OPT_INT_Z 53
    { "-Z",     GETOPTN_STRING,
        NULL  /*&di_data->zoneInfo.zoneDisplay*/,
        0  /*sizeof (di_data->zoneInfo.zoneDisplay)*/,
        (void *) "all" }
  };
  static int dbsids[] =
    { OPT_INT_d, OPT_INT_g, OPT_INT_h, OPT_INT_H, OPT_INT_k, OPT_INT_m };
  static int paidb[] =
    { OPT_INT_a, OPT_INT_help, OPT_INT_qmark, OPT_INT_P,
      OPT_INT_si, OPT_INT_version };
  static int paidv[] =
    { OPT_INT_B, OPT_INT_I, OPT_INT_s, OPT_INT_x, OPT_INT_X };

  diopts = &di_data->options;
  diout = &di_data->output;

    /* this is seriously gross, but the old compilers don't have    */
    /* automatic aggregate initialization                           */
    /* don't forget to change dbsids, paidb and paidv above also    */
  opts[OPT_INT_A].valptr = (void *) &diopts->formatString;   /* -A */
  opts[OPT_INT_c].valptr = (void *) &diopts->csv_output;     /* -c */
  opts[OPT_INT_c].valsiz = sizeof (diopts->csv_output);
  opts[OPT_INT_C].valptr = (void *) &diopts->csv_tabs;     /* -C */
  opts[OPT_INT_C].valsiz = sizeof (diopts->csv_tabs);
  opts[OPT_INT_f].valptr = (void *) &diopts->formatString;  /* -f */
  opts[OPT_INT_j].valptr = (void *) &diopts->json_output;     /* -j */
  opts[OPT_INT_j].valsiz = sizeof (diopts->json_output);
  opts[OPT_INT_l].valptr = (void *) &diopts->localOnly;     /* -l */
  opts[OPT_INT_l].valsiz = sizeof (diopts->localOnly);
  opts[OPT_INT_L].valptr = (void *) &diopts->excludeLoopback; /* -L */
  opts[OPT_INT_L].valsiz = sizeof (diopts->excludeLoopback);
  opts[OPT_INT_n].valptr = (void *) &diopts->printHeader;   /* -n */
  opts[OPT_INT_n].valsiz = sizeof (diopts->printHeader);
  opts[OPT_INT_q].valptr = (void *) &diopts->quota_check;    /* -q */
  opts[OPT_INT_q].valsiz = sizeof (diopts->quota_check);
  opts[OPT_INT_R].valptr = (void *) &diopts->dontResolveSymlink;    /* -R */
  opts[OPT_INT_R].valsiz = sizeof (diopts->dontResolveSymlink);
  opts[OPT_INT_t].valptr = (void *) &diopts->printTotals;    /* -t */
  opts[OPT_INT_t].valsiz = sizeof (diopts->printTotals);
  opts[OPT_INT_w].valptr = (void *) &diout->width;          /* -w */
  opts[OPT_INT_w].valsiz = sizeof (diout->width);
  opts[OPT_INT_W].valptr = (void *) &diout->inodeWidth;     /* -W */
  opts[OPT_INT_W].valsiz = sizeof (diout->inodeWidth);
  opts[OPT_INT_z].valptr = (void *) di_data->zoneInfo.zoneDisplay;  /* -z */
  opts[OPT_INT_z].valsiz = sizeof (di_data->zoneInfo.zoneDisplay);
  opts[OPT_INT_Z].valptr = (void *) di_data->zoneInfo.zoneDisplay;  /* -Z */
  opts[OPT_INT_Z].valsiz = sizeof (di_data->zoneInfo.zoneDisplay);

  for (i = 0; i < (int) (sizeof (dbsids) / sizeof (int)); ++i) {
    opts[dbsids[i]].valptr = (void *) dbsstr;
    opts[dbsids[i]].valsiz = dbsstr_sz;
  }
  for (i = 0; i < (int) (sizeof (paidb) / sizeof (int)); ++i) {
    opts[paidb[i]].valptr = (void *) &padata;
    opts[paidb[i]].value2 = (void *) processOptions;
    if (diopts->exitFlag != DI_EXIT_NORM) {
      break;
    }
  }
  for (i = 0; i < (int) (sizeof (paidv) / sizeof (int)); ++i) {
    opts[paidv[i]].valptr = (void *) &padata;
    opts[paidv[i]].value2 = (void *) processOptionsVal;
    if (diopts->exitFlag != DI_EXIT_NORM) {
      break;
    }
  }

  optidx = -1;
  if (diopts->exitFlag != DI_EXIT_NORM) {
    return optidx;
  }

  padata.di_data = di_data;
  padata.diopts = diopts;
  padata.diout = diout;
  padata.dbsstr = dbsstr;
  padata.dbsstr_sz = dbsstr_sz;

  optidx = getoptn (GETOPTN_LEGACY, argc, argv,
       sizeof (opts) / sizeof (getoptn_opt_t), opts, &errorCount);
  diopts->errorCount += errorCount;
  if (diopts->errorCount > 0) {
    setExitFlag (diopts, DI_EXIT_WARN);
  }

  if (diopts->csv_tabs) {
    diopts->csv_output = true;
  }
  if (diopts->csv_output || diopts->json_output) {
    diopts->printTotals = false;
  }
  if (diopts->json_output) {
    diopts->printHeader = false;
  }

  return optidx;
}

static void
processOptions (const char *arg, char *valptr)
{
  struct pa_tmp     *padata;

  padata = (struct pa_tmp *) valptr;
  if (strcmp (arg, "-a") == 0) {
    padata->diopts->displayAll = true;
    strncpy (padata->di_data->zoneInfo.zoneDisplay, "all", MAXPATHLEN);
  } else if (strcmp (arg, "--help") == 0 || strcmp (arg, "-?") == 0) {
    setExitFlag (padata->diopts, DI_EXIT_HELP);
  } else if (strcmp (arg, "-P") == 0) {
    /* don't override -k option */
    if (strcmp (padata->dbsstr, "k") != 0) {
      strncpy (padata->dbsstr, "512", padata->dbsstr_sz);
    }
    padata->diopts->formatString = DI_POSIX_FORMAT;
    padata->diopts->posix_compat = true;
    padata->diopts->csv_output = false;
  } else if (strcmp (arg, "--si") == 0) {
    padata->diopts->baseDispSize = DI_VAL_1000;
    padata->diopts->baseDispIdx = DI_DISP_SI_PREFIX;
    strncpy (padata->dbsstr, "H", padata->dbsstr_sz);
  } else if (strcmp (arg, "--version") == 0) {
    printf (DI_GT("di version %s    Default Format: %s\n"), DI_VERSION, DI_DEFAULT_FORMAT);
    setExitFlag (padata->diopts, DI_EXIT_HELP);
  } else {
    fprintf (stderr, "di_panic: bad option setup\n");
  }

  return;
}

static void
processOptionsVal (const char *arg, char *valptr, char *value)
{
  struct pa_tmp     *padata;
  int               rc;

  padata = (struct pa_tmp *) valptr;

  if (strcmp (arg, "-B") == 0) {
    if (isdigit ((int) (*value))) {
      padata->diopts->baseDispSize = (unsigned int) atoi (value);
      padata->diopts->baseDispIdx = DI_DISP_SI_PREFIX; /* unknown, really */
      if (padata->diopts->baseDispSize == DI_VAL_1024)
      {
        padata->diopts->baseDispIdx = DI_DISP_PREFIX;
      }
    } else if (strcmp (value, "k") == 0) {
      padata->diopts->baseDispSize = DI_VAL_1024;
      padata->diopts->baseDispIdx = DI_DISP_PREFIX;
    }
    else if (strcmp (value, "d") == 0 || strcmp (value, "si") == 0) {
      padata->diopts->baseDispSize = DI_VAL_1000;
      padata->diopts->baseDispIdx = DI_DISP_SI_PREFIX;
    }
  } else if (strcmp (arg, "-I") == 0) {
    rc = parseList (&padata->di_data->include_list, value);
    if (rc != 0) {
      setExitFlag (padata->diopts, DI_EXIT_FAIL);
      return;
    }
  } else if (strcmp (arg, "-s") == 0) {
    strncpy (padata->diopts->sortType, value, DI_SORT_MAX);
      /* for backwards compatibility                       */
      /* reverse by itself - change to reverse mount point */
    if (strcmp (padata->diopts->sortType, "r") == 0) {
        strncpy (padata->diopts->sortType, "rm", DI_SORT_MAX);
    }
        /* add some sense to the sort order */
    if (strcmp (padata->diopts->sortType, "t") == 0) {
        strncpy (padata->diopts->sortType, "tm", DI_SORT_MAX);
    }
  } else if (strcmp (arg, "-x") == 0) {
    parseList (&padata->di_data->ignore_list, value);
  } else if (strcmp (arg, "-X") == 0) {
    debug = atoi (value);
    padata->diopts->printDebugHeader = true;
    padata->diopts->printTotals = true;
    padata->diopts->printHeader = true;
    padata->diout->width = 10;
    padata->diout->inodeWidth = 10;
  } else {
    fprintf (stderr, "di_panic: bad option setup\n");
  }

  return;
}

static int
parseList (di_strarr_t *list, char *str)
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
    return 1;
  }

  ptr = strtok (dstr, DI_LIST_SEP);
  count = 0;
  while (ptr != (char *) NULL) {
    ++count;
    ptr = strtok ((char *) NULL, DI_LIST_SEP);
  }

  ocount = list->count;
  list->count += count;
  ncount = list->count;
  list->list = (char **) di_realloc ((char *) list->list,
      (Size_t) list->count * sizeof (char *));
  if (list->list == (char **) NULL) {
    fprintf (stderr, "malloc failed in parseList() (2).  errno %d\n", errno);
    free ((char *) dstr);
    return 1;
  }

  ptr = dstr;
  for (i = ocount; i < ncount; ++i) {
    len = (unsigned int) strlen (ptr);
    lptr = (char *) malloc ((Size_t) len + 1);
    if (lptr == (char *) NULL) {
      fprintf (stderr, "malloc failed in parseList() (3).  errno %d\n", errno);
      free ((char *) dstr);
      return 1;
    }
    strncpy (lptr, ptr, (Size_t) len);
    lptr[len] = '\0';
    list->list [i] = lptr;
    ptr += len + 1;
  }

  free ((char *) dstr);
  return 0;
}


static void
setDispBlockSize (char *ptr, di_opt_t *diopts, diOutput_t *diout)
{
  unsigned int    len;
  int             i;
  int             val;
  char            *tptr;
  static char     tempbl [15];
  char            ttempbl [15];

  val = 1;
  if (isdigit ((int) (*ptr))) {
    /* it is unlikely that anyone is going to type in some large number */
    /* on the command line, atoi() should be good enough */
    val = atoi (ptr);
  }

  tptr = ptr;
  len = (unsigned int) strlen (ptr);
  if (! isdigit ((int) *tptr)) {
    int             idx;

    idx = -1;
    for (i = 0; i < DI_DISPTAB_SIZE; ++i) {
      if (*tptr == *dispTable [i].disp [DI_DISP_LC_LETTER] ||
          *tptr == *dispTable [i].disp [DI_DISP_UC_LETTER]) {
        idx = i;
      }
    }

    if (idx == -1) {
      if (*tptr == 'h') {
        val = DI_DISP_HR;
        diout->dispBlockLabel = "Size";
      }
      if (*tptr == 'H') {
        val = DI_DISP_HR_2;
        diout->dispBlockLabel = "Size";
      }
    }

    if (idx == -1) {
      if (strncmp (ptr, "HUMAN", (Size_t) 5) == 0) {
        val = DI_DISP_HR;
      } else {
        /* some unknown string value */
        idx = DI_MEGA;
      }
    }

    if (idx >= 0) {
      if (len > 1) {
        ++tptr;
        if (*tptr == 'B') {
           diopts->baseDispSize = DI_VAL_1000;
           diopts->baseDispIdx = DI_DISP_SI_PREFIX;
        }
      }

      if (val == 1) {
        diout->dispBlockLabel = dispTable [idx].disp [diopts->baseDispIdx];
      } else {
        Snprintf1 (ttempbl, sizeof (tempbl), "%%.0f");
        Snprintf2 (tempbl, sizeof (tempbl), ttempbl,
            val, DI_GT (dispTable [idx].disp [diopts->baseDispIdx]));
        diout->dispBlockLabel = tempbl;
      }
      diopts->dispBlockSize = val;
      if (idx != -1) {
        dinum_set_u (&diopts->dispScaleValue, val);
        dinum_mul (&diopts->dispScaleValue, &dispSizes [idx]);
      }
    } /* known size multiplier */
  } else {
    int         ok;

    ok = 0;
    for (i = 0; i < (int) DI_DISPTAB_SIZE; ++i) {
      /* only works for the smaller numbers, should be fine */
      if (dinum_cmp_s (&dispSizes [i], val) == 0) {
        diout->dispBlockLabel = dispTable [i].disp [diopts->baseDispIdx];
        ok = 1;
        break;
      }
    }

    if (ok == 0) {
      Snprintf1 (ttempbl, sizeof (ttempbl), "%%.0fb");
      Snprintf1 (tempbl, sizeof (tempbl), ttempbl, val);
      diout->dispBlockLabel = tempbl;
    }
  }  /* some oddball block size */

  if (diopts->posix_compat && val == DI_VAL_512) {
    diout->dispBlockLabel = "512-blocks";
  }
  if (diopts->posix_compat && val == DI_VAL_1024) {
    diout->dispBlockLabel = "1024-blocks";
  }

  diopts->dispBlockSize = val;
//  if (idx != -1) {
//    dinum_set (&diopts->dispScaleValue, &dispSizes [idx]);
//  }
}


static void
initDisplayTable (di_opt_t *diopts)
{
  int       i;

  /* initialize dispTable array */
  dinum_set_u (&dispSizes [0], diopts->baseDispSize);
  for (i = 1; i < (int) DI_DISPTAB_SIZE; ++i) {
    dinum_set (&dispSizes [i], &dispSizes [i - 1]);
    dinum_mul_u (&dispSizes [i], diopts->baseDispSize);
  }
}

static void
setExitFlag (di_opt_t *diopts, unsigned int exitFlag)
{
  if (exitFlag > diopts->exitFlag) {
    diopts->exitFlag = exitFlag;
  }
}
