/*
 * Copyright 1994-2018 Brad Lanam, Walnut Creek, CA
 * Copyright 2023 Brad Lanam, Pleasant Hill, CA
 */

#include "config.h"
#include "di.h"
#include "getoptn.h"
#include "options.h"
#include "version.h"

#if _hdr_stdio
# include <stdio.h>
#endif
#if _hdr_stdlib
# include <stdlib.h>
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
#if _use_mcheck
# include <mcheck.h>
#endif

struct pa_tmp {
  diData_t        *diData;
  diOptions_t     *diopts;
  diOutput_t      *diout;
  char            *dbsstr;
  Size_t          dbsstr_sz;
};

typedef struct
{
    _print_size_t   size;
    const char      *disp[2];
} dispTable_t;

static dispTable_t dispTable [] =
{
    { (_print_size_t) 0.0, { "KBytes", "KBytes" } },
    { (_print_size_t) 0.0, { "Megs", "Mebis" } },
    { (_print_size_t) 0.0, { "Gigs", "Gibis" } },
    { (_print_size_t) 0.0, { "Teras", "Tebis" } },
    { (_print_size_t) 0.0, { "Petas", "Pebis" } },
    { (_print_size_t) 0.0, { "Exas", "Exbis" } },
    { (_print_size_t) 0.0, { "Zettas", "Zebis" } },
    { (_print_size_t) 0.0, { "Yottas", "Yobis" } }
};
#define DI_DISPTAB_SIZE (sizeof (dispTable) / sizeof (dispTable_t))

#define DI_ARGV_SEP             " 	"  /* space, tab */
#define DI_MAX_ARGV             50
#define DI_LIST_SEP             ","

#define DI_POSIX_FORMAT         "SbuvpM"
#define DI_DEF_MOUNT_FORMAT     "MST\n\tO"
#define DI_ALL_FORMAT           "MTS\n\tO\n\tbuf13\n\tbcvpa\n\tBuv2\n\tiUFP"

# if defined (__cplusplus) || defined (c_plusplus)
   extern "C" {
# endif

extern int debug;

static void processStringArgs   _((const char *, char *, diData_t *, char *));
static int  processArgs         _((int, const char * const [], diData_t *, char *, Size_t));
static int  parseList           _((iList_t *, char *));
static void processOptions      _((const char *, char *));
static void processOptionsVal   _((const char *, char *, char *));
static void usage               _((void));
static void setDispBlockSize    _((char *, diOptions_t *, diOutput_t *));
static void initDisplayTable    _((diOptions_t *));
static void setExitFlag         _((diOptions_t *, unsigned int));

# if defined (__cplusplus) || defined (c_plusplus)
   }
# endif

static void
processStringArgs (const char *progname, char *ptr, diData_t *diData,
    char *dbsstr)
{
  char        *dptr;
  char        *tptr;
  int         nargc;
  const char  *nargv [DI_MAX_ARGV];
  diOptions_t *diopts;

  if (ptr == (char *) NULL || strcmp (ptr, "") == 0) {
    return;
  }

  diopts = &diData->options;

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
    nargv[0] = progname;
    while (tptr != (char *) NULL) {
      if (nargc >= DI_MAX_ARGV) {
        break;
      }
      nargv[nargc++] = tptr;
      tptr = strtok ((char *) NULL, DI_ARGV_SEP);
    }
    optidx = processArgs (nargc, nargv, diData, dbsstr, sizeof (dbsstr) - 1);
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
getDIOptions (int argc, const char * const argv[], diData_t *diData)
{
  const char *      argvptr;
  char *            ptr;
  char              dbsstr [30];
  int               optidx;
  int               ec;
  diOptions_t       *diopts;
  diOutput_t        *diout;

  diopts = &diData->options;
  diout = &diData->output;
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
    diopts->posix_compat = TRUE;
    diopts->csv_output = FALSE;
    diopts->json_output = FALSE;
  }

      /* bsd df */
  if ((ptr = getenv ("BLOCKSIZE")) != (char *) NULL) {
    strncpy (dbsstr, ptr, sizeof (dbsstr)-1);
  }

      /* gnu df */
  if ((ptr = getenv ("DF_BLOCK_SIZE")) != (char *) NULL) {
    strncpy (dbsstr, ptr, sizeof (dbsstr)-1);
  }

  if ((ptr = getenv ("DI_ARGS")) != (char *) NULL) {
    processStringArgs (argv [0], ptr, diData, dbsstr);
  }

  optidx = processArgs (argc, argv, diData, dbsstr, sizeof (dbsstr) - 1);

  if (debug > 0) {
    int j;

    printf ("# ARGS:");
    for (j = 0; j < argc; ++j) {
      printf (" %s", argv[j]);
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
processArgs (int argc,
             const char * const argv [],
             diData_t *diData,
             char *dbsstr,
             Size_t dbsstr_sz)
{
  int           i;
  int           optidx;
  int           errorCount;
  diOptions_t   *diopts;
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
        NULL  /*&diData->zoneInfo.zoneDisplay*/,
        0  /*sizeof (diData->zoneInfo.zoneDisplay)*/,
        NULL },
/* 53 */
#define OPT_INT_Z 53
    { "-Z",     GETOPTN_STRING,
        NULL  /*&diData->zoneInfo.zoneDisplay*/,
        0  /*sizeof (diData->zoneInfo.zoneDisplay)*/,
        (void *) "all" }
  };
  static int dbsids[] =
    { OPT_INT_d, OPT_INT_g, OPT_INT_h, OPT_INT_H, OPT_INT_k, OPT_INT_m };
  static int paidb[] =
    { OPT_INT_a, OPT_INT_help, OPT_INT_qmark, OPT_INT_P,
      OPT_INT_si, OPT_INT_version };
  static int paidv[] =
    { OPT_INT_B, OPT_INT_I, OPT_INT_s, OPT_INT_x, OPT_INT_X };

  diopts = &diData->options;
  diout = &diData->output;

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
  opts[OPT_INT_z].valptr = (void *) diData->zoneInfo.zoneDisplay;  /* -z */
  opts[OPT_INT_z].valsiz = sizeof (diData->zoneInfo.zoneDisplay);
  opts[OPT_INT_Z].valptr = (void *) diData->zoneInfo.zoneDisplay;  /* -Z */
  opts[OPT_INT_Z].valsiz = sizeof (diData->zoneInfo.zoneDisplay);

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

  padata.diData = diData;
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
    diopts->csv_output = TRUE;
  }
  if (diopts->csv_output || diopts->json_output) {
    diopts->printTotals = FALSE;
  }
  if (diopts->json_output) {
    diopts->printHeader = FALSE;
  }

  return optidx;
}

static void
processOptions (const char *arg, char *valptr)
{
  struct pa_tmp     *padata;

  padata = (struct pa_tmp *) valptr;
  if (strcmp (arg, "-a") == 0) {
    padata->diopts->displayAll = TRUE;
    strncpy (padata->diData->zoneInfo.zoneDisplay, "all", MAXPATHLEN);
  } else if (strcmp (arg, "--help") == 0 || strcmp (arg, "-?") == 0) {
    usage();
    setExitFlag (padata->diopts, DI_EXIT_OK);
  } else if (strcmp (arg, "-P") == 0) {
    /* don't override -k option */
    if (strcmp (padata->dbsstr, "k") != 0) {
      strncpy (padata->dbsstr, "512", padata->dbsstr_sz);
    }
    padata->diopts->formatString = DI_POSIX_FORMAT;
    padata->diopts->posix_compat = TRUE;
    padata->diopts->csv_output = FALSE;
  } else if (strcmp (arg, "--si") == 0) {
    padata->diopts->baseDispSize = (_print_size_t) DI_VAL_1000;
    padata->diopts->baseDispIdx = DI_DISP_1000_IDX;
    strncpy (padata->dbsstr, "H", padata->dbsstr_sz);
  } else if (strcmp (arg, "--version") == 0) {
    printf (DI_GT("di version %s    Default Format: %s\n"), DI_VERSION, DI_DEFAULT_FORMAT);
    setExitFlag (padata->diopts, DI_EXIT_OK);
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
      padata->diopts->baseDispSize = (_print_size_t) atof (value);
      padata->diopts->baseDispIdx = DI_DISP_1000_IDX; /* unknown, really */
      if (padata->diopts->baseDispSize == (_print_size_t) DI_VAL_1024)
      {
        padata->diopts->baseDispIdx = DI_DISP_1024_IDX;
      }
    } else if (strcmp (value, "k") == 0) {
      padata->diopts->baseDispSize = (_print_size_t) DI_VAL_1024;
      padata->diopts->baseDispIdx = DI_DISP_1024_IDX;
    }
    else if (strcmp (value, "d") == 0 || strcmp (value, "si") == 0) {
      padata->diopts->baseDispSize = (_print_size_t) DI_VAL_1000;
      padata->diopts->baseDispIdx = DI_DISP_1000_IDX;
    }
  } else if (strcmp (arg, "-I") == 0) {
    rc = parseList (&padata->diData->includeList, value);
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
parseList (iList_t *list, char *str)
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


/*
 * usage
 */

static void
usage (void)
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

static void
setDispBlockSize (char *ptr, diOptions_t *diopts, diOutput_t *diout)
{
  unsigned int    len;
  _print_size_t   val;
  char            *tptr;
  static char     tempbl [15];
  char            ttempbl [15];

  if (isdigit ((int) (*ptr))) {
    val = (_print_size_t) atof (ptr);
  } else {
    val = (_print_size_t) 1.0;
  }

  tptr = ptr;
  len = (unsigned int) strlen (ptr);
  if (! isdigit ((int) *tptr)) {
    int             idx;

    idx = -1;
    switch (*tptr) {
      case 'k':
      case 'K': {
        idx = DI_ONE_K;
        break;
      }

      case 'm':
      case 'M': {
        idx = DI_ONE_MEG;
        break;
      }

      case 'g':
      case 'G': {
        idx = DI_ONE_GIG;
        break;
      }

      case 't':
      case 'T': {
        idx = DI_ONE_TERA;
        break;
      }

      case 'p':
      case 'P': {
        idx = DI_ONE_PETA;
        break;
      }

      case 'e':
      case 'E': {
        idx = DI_ONE_EXA;
        break;
      }

      case 'z':
      case 'Z': {
        idx = DI_ONE_ZETTA;
        break;
      }

      case 'y':
      case 'Y': {
        idx = DI_ONE_YOTTA;
        break;
      }

      case 'h': {
        val = (_print_size_t) DI_DISP_HR;
        diout->dispBlockLabel = "Size";
        break;
      }

      case 'H': {
        val = (_print_size_t) DI_DISP_HR_2;
        diout->dispBlockLabel = "Size";
        break;
      }

      default: {
        if (strncmp (ptr, "HUMAN", (Size_t) 5) == 0) {
          val = (_print_size_t) DI_DISP_HR;
        } else {
          /* some unknown string value */
          idx = DI_ONE_MEG;
        }
        break;
      }
    }

    if (idx >= 0) {
      if (len > 1) {
        ++tptr;
        if (*tptr == 'B') {
           diopts->baseDispSize = (_print_size_t) DI_VAL_1000;
           diopts->baseDispIdx = DI_DISP_1000_IDX;
        }
      }

      if (val == (_print_size_t) 1.0) {
        diout->dispBlockLabel = dispTable [idx].disp [diopts->baseDispIdx];
      }
      else {
        Snprintf1 (ttempbl, sizeof (tempbl), "%%.0%s %%s", DI_Lf);
        Snprintf2 (tempbl, sizeof (tempbl), ttempbl,
            val, DI_GT (dispTable [idx].disp [diopts->baseDispIdx]));
        diout->dispBlockLabel = tempbl;
      }
      val *= dispTable [idx].size;
    } /* known size multiplier */
  } else {
    int         i;
    int         ok;

    ok = 0;
    for (i = 0; i < (int) DI_DISPTAB_SIZE; ++i) {
      if (val == dispTable [i].size) {
        diout->dispBlockLabel = dispTable [i].disp [diopts->baseDispIdx];
        ok = 1;
        break;
      }
    }

    if (ok == 0) {
      Snprintf1 (ttempbl, sizeof (ttempbl), "%%.0%sb", DI_Lf);
      Snprintf1 (tempbl, sizeof (tempbl), ttempbl, val);
      diout->dispBlockLabel = tempbl;
    }
  }  /* some oddball block size */

  if (diopts->posix_compat && val == (_print_size_t) DI_VAL_512) {
    diout->dispBlockLabel = "512-blocks";
  }
  if (diopts->posix_compat && val == (_print_size_t) DI_VAL_1024) {
    diout->dispBlockLabel = "1024-blocks";
  }

  diopts->dispBlockSize = val;
}


static void
initDisplayTable (diOptions_t *diopts)
{
  int       i;

      /* initialize dispTable array */
  dispTable [0].size = diopts->baseDispSize;
  for (i = 1; i < (int) DI_DISPTAB_SIZE; ++i) {
    dispTable [i].size = dispTable [i - 1].size *
        diopts->baseDispSize;
  }
}

static void
setExitFlag (diOptions_t *diopts, unsigned int exitFlag)
{
  if (exitFlag > diopts->exitFlag) {
    diopts->exitFlag = exitFlag;
  }
}
