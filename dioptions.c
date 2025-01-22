/*
 * Copyright 1994-2018 Brad Lanam, Walnut Creek, CA
 * Copyright 2023-2025 Brad Lanam, Pleasant Hill, CA
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
#include "diinternal.h" // DI_DEFAULT_FORMAT
#include "distrutils.h"
#include "getoptn.h"
#include "dioptions.h"

struct pa_tmp {
  di_opt_t        *diopts;
  char            *scalestr;
  Size_t          scalestrsz;
};

typedef struct
{
  const char      uc;
  const char      lc;
} di_valid_scale_t;

static di_valid_scale_t validscale [] =
{
  { 'B', 'b' },  /* "Byte", "Byte" */
  { 'K', 'k' },  /* "Kilo", "Kibi" */
  { 'M', 'm' },  /* "Mega", "Mebi" */
  { 'G', 'g' },  /* "Giga", "Gibi" */
  { 'T', 't' },  /* "Tera", "Tebi" */
  { 'P', 'p' },  /* "Peta", "Pebi" */
  { 'E', 'e' },  /* "Exa", "Exbi" */
  { 'Z', 'z' },  /* "Zetta", "Zebi" */
  { 'Y', 'y' },  /* "Yotta", "Yobi" */
  { 'R', 'r' },  /* "Ronna", "Ronni" */
  { 'Q', 'q' }   /* "Quetta", "Quetti" */
};
#define DI_VALID_SCALE_SZ ( (int) (sizeof (validscale) / sizeof (di_valid_scale_t)))

#define DI_ARGV_SEP             " 	"  /* space, tab */
#define DI_MAX_ARGV             50
#define DI_LIST_SEP             ","

#define DI_POSIX_FORMAT         "sbuvpm"
#define DI_ALL_FORMAT           "mts\n\tO\n\tbuf13\n\tbcvpa\n\tBuv2\n\tiUFP"

extern int debug;

static void processStringArgs (const char *, char *, di_opt_t *, char *, Size_t);
static int  processArgs (int, char * argv [], di_opt_t *, char *, Size_t);
static int  parseList (di_strarr_t *, char *);
static void parseScaleValue (di_opt_t *diopts, char *ptr);
static void processOptions (const char *, char *);
static void processOptionsVal (const char *, void *, char *);
static void setExitFlag (di_opt_t *, int);

static void
processStringArgs (const char *progname, char *ptr, di_opt_t *diopts,
    char *scalestr, Size_t scalestrsz)
{
  char        *dptr;
  char        *tptr;
  int         nargc;
  char        *nargv [DI_MAX_ARGV];

  if (ptr == (char *) NULL || strcmp (ptr, "") == 0) {
    return;
  }

  dptr = (char *) NULL;
  dptr = strdup (ptr);
  if (dptr == (char *) NULL) {
    fprintf (stderr, "strdup failed in main () (1).  errno %d\n", errno);
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
      tptr = strtok ( (char *) NULL, DI_ARGV_SEP);
    }
    optidx = processArgs (nargc, nargv, diopts, scalestr, scalestrsz);
    if (optidx < nargc) {
      fprintf (stderr, "%s: unknown data found in DI_ARGS: %s\n",
          progname, nargv [optidx]);
      diopts->errorCount += 1;
      if (diopts->errorCount > 0) {
        setExitFlag (diopts, DI_EXIT_WARN);
      }
    }
    free ( (char *) dptr);
  }
}

di_opt_t *
di_init_options (void)
{
  di_opt_t    *diopts;

  diopts = malloc (sizeof (di_opt_t));
  if (diopts == NULL) {
    return diopts;
  }

  diopts->formatString = DI_DEFAULT_FORMAT;
  diopts->formatLen = (int) strlen (diopts->formatString);
  diopts->zoneDisplay [0] = '\0';
  diopts->ignore_list.count = 0;
  diopts->ignore_list.list = (char **) NULL;
  diopts->include_list.count = 0;
  diopts->include_list.list = (char **) NULL;
  diopts->scale = DI_SCALE_GIGA;
  diopts->blockSize = DI_BLKSZ_1024;
  diopts->optval [DI_OPT_DISP_TOTALS] = false;
  diopts->optval [DI_OPT_DISP_DBG_HEADER] = false;
  diopts->optval [DI_OPT_DISP_HEADER] = true;
  diopts->optval [DI_OPT_LOCAL_ONLY] = false;
  diopts->optval [DI_OPT_DISP_ALL] = false;
  diopts->optval [DI_OPT_NO_SYMLINK] = false;
  diopts->optval [DI_OPT_EXCL_LOOPBACK] = true;

  /* default - by mount point*/
  stpecpy (diopts->sortType, diopts->sortType + sizeof (diopts->sortType), "m");
  diopts->optval [DI_OPT_POSIX_COMPAT] = false;
  diopts->optval [DI_OPT_QUOTA_CHECK] = true;
  diopts->optval [DI_OPT_DISP_CSV] = false;
  diopts->optval [DI_OPT_DISP_CSV_TAB] = false;
  diopts->exitFlag = DI_EXIT_NORM;
  diopts->errorCount = 0;
  diopts->optval [DI_OPT_DISP_JSON] = false;

  return diopts;
}

void
di_opt_cleanup (di_opt_t *diopts)
{
  if (diopts == NULL) {
    return;
  }

  if (diopts->ignore_list.count > 0 &&
      diopts->ignore_list.list != (char **) NULL) {
    free ( (void *) diopts->ignore_list.list);
    diopts->ignore_list.count = 0;
  }

  if (diopts->include_list.count > 0 &&
      diopts->include_list.list != (char **) NULL) {
    free (diopts->include_list.list);
    diopts->include_list.count = 0;
  }
  free (diopts);
}

int
di_get_options (int argc, char * argv [], di_opt_t *diopts)
{
  char *            ptr;
  char              scalestr [30];
  int               optidx;

  if (diopts == NULL) {
    return DI_EXIT_FAIL;
  }

  diopts->argc = argc;
  diopts->argv = argv;
  stpecpy (scalestr, scalestr + sizeof (scalestr), DI_DEFAULT_DISP_SIZE);

  /* gnu df */
  if ( (ptr = getenv ("POSIXLY_CORRECT")) != (char *) NULL) {
    stpecpy (scalestr, scalestr + sizeof (scalestr), "k");
    diopts->formatString = DI_POSIX_FORMAT;
    diopts->optval [DI_OPT_POSIX_COMPAT] = true;
    diopts->optval [DI_OPT_DISP_CSV] = false;
    diopts->optval [DI_OPT_DISP_JSON] = false;
  }

  /* bsd df */
  if ( (ptr = getenv ("BLOCKSIZE")) != (char *) NULL) {
    stpecpy (scalestr, scalestr + sizeof (scalestr), ptr);
  }

  /* gnu df */
  if ( (ptr = getenv ("DF_BLOCK_SIZE")) != (char *) NULL) {
    stpecpy (scalestr, scalestr + sizeof (scalestr), ptr);
  }

  if ( (ptr = getenv ("DI_ARGS")) != (char *) NULL) {
    processStringArgs (argv [0], ptr, diopts, scalestr, sizeof (scalestr));
  }

  optidx = processArgs (argc, argv, diopts, scalestr, sizeof (scalestr));

  if (debug > 0) {
    int j;

    printf ("# ARGS:");
    for (j = 0; j < argc; ++j) {
      printf (" %s", argv [j]);
    }
    printf ("\n");
    printf ("# blocksize: %d\n", diopts->blockSize);
    printf ("# scale: %s\n", scalestr);

    if ( (ptr = getenv ("POSIXLY_CORRECT")) != (char *) NULL) {
      printf ("# POSIXLY_CORRECT: %s\n", ptr);
    }
    if ( (ptr = getenv ("BLOCKSIZE")) != (char *) NULL) {
      printf ("# BLOCKSIZE: %s\n", ptr);
    }
    if ( (ptr = getenv ("DF_BLOCK_SIZE")) != (char *) NULL) {
      printf ("# DF_BLOCK_SIZE: %s\n", ptr);
    }
    if ( (ptr = getenv ("DI_ARGS")) != (char *) NULL) {
      printf ("# DI_ARGS: %s\n", ptr);
    }
  }

  parseScaleValue (diopts, scalestr);

  diopts->formatLen = (int) strlen (diopts->formatString);
  diopts->optidx = optidx;

  return diopts->exitFlag;
}

void
di_opt_format_iter_init (di_opt_t *diopts)
{
  if (diopts == NULL) {
    return;
  }

  diopts->optiteridx = 0;
}

int
di_opt_format_iterate (di_opt_t *diopts)
{
  int     val;

  if (diopts == NULL) {
    return DI_FMT_ITER_STOP;
  }

  if (diopts->optiteridx < 0 || diopts->optiteridx >= diopts->formatLen) {
    return DI_FMT_ITER_STOP;
  }

  val = diopts->formatString [diopts->optiteridx];
  ++diopts->optiteridx;
  return val;
}

int
di_opt_check_option (di_opt_t *diopts, int optidx)
{
  if (diopts == NULL) {
    return 0;
  }

  if (optidx == DI_OPT_FMT_STR_LEN) {
    return diopts->formatLen;
  }
  if (optidx == DI_OPT_SCALE) {
    return diopts->scale;
  }
  if (optidx == DI_OPT_BLOCK_SZ) {
    return diopts->blockSize;
  }

  if (optidx < 0 || optidx >= DI_OPT_MAX) {
    return 0;
  }

  return diopts->optval [optidx];
}

static int
processArgs (int argc, char * argv [], di_opt_t *diopts,
    char *scalestr, Size_t scalestrsz)
{
  int           i;
  int           optidx;
  int           errorCount;
  struct pa_tmp padata;

    /* the really old compilers don't have automatic initialization */
  static getoptn_opt_t opts [] = {
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
        NULL  /*&diopts->optval [DI_OPT_DISP_CSV]*/,
        0     /*sizeof (diopts->optval [DI_OPT_DISP_CSV])*/,
        NULL },
/* 7 */
#define OPT_INT_C 7
    { "-C",     GETOPTN_BOOL,
        NULL  /*&diopts->optval [DI_OPT_DISP_CSV_TAB]*/,
        0     /*sizeof (diopts->optval [DI_OPT_DISP_CSV_TAB])*/,
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
        NULL  /* scalestr */,
        0  /* scalestrsz */,
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
        NULL  /* scalestr */,
        0  /* scalestrsz */,
        (void *) "g" },
/* 17 */
#define OPT_INT_h 17
    { "-h",     GETOPTN_STRING,
        NULL  /* scalestr */,
        0  /* scalestrsz */,
        (void *) "h" },
/* 18 */
#define OPT_INT_H 18
    { "-H",     GETOPTN_STRING,
        NULL  /* scalestr */,
        0  /* scalestrsz */,
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
        NULL  /*&diopts->optval [DI_OPT_DISP_JSON]*/,
        0     /*sizeof (diopts->optval [DI_OPT_DISP_JSON])*/,
        NULL },
/* 26 */
    { "--json-output", GETOPTN_ALIAS,
        (void *) "-j",
        0,
        NULL },
/* 27 */
#define OPT_INT_k 27
    { "-k",     GETOPTN_STRING,
        NULL  /* scalestr */,
        0  /* scalestrsz */,
        (void *) "k" },
/* 28 */
#define OPT_INT_l 28
    { "-l",     GETOPTN_BOOL,
        NULL  /* &diopts->optval [DI_OPT_LOCAL_ONLY] */,
        0  /* sizeof (diopts->optval [DI_OPT_LOCAL_ONLY]) */,
        NULL },
/* 29 */
    { "--local",GETOPTN_ALIAS,
        (void *) "-l",
        0,
        NULL },
/* 30 */
#define OPT_INT_L 30
    { "-L",     GETOPTN_BOOL,
        NULL  /*&diopts->optval [DI_OPT_EXCL_LOOPBACK]*/,
        0  /*sizeof (diopts->optval [DI_OPT_EXCL_LOOPBACK])*/,
        NULL },
/* 31 */
#define OPT_INT_m 31
    { "-m",     GETOPTN_STRING,
        NULL  /* scalestr */,
        0  /* scalestrsz */,
        (void *) "m" },
/* 32 */
#define OPT_INT_n 32
    { "-n",     GETOPTN_BOOL,
        NULL  /*&diopts->optval [DI_OPT_DISP_HEADER]*/,
        0  /*sizeof (diopts->optval [DI_OPT_DISP_HEADER])*/,
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
        NULL  /*&diopts->optval [DI_OPT_QUOTA_CHECK]*/,
        0  /*sizeof (diopts->optval [DI_OPT_QUOTA_CHECK])*/,
        NULL },
/* 38 */
#define OPT_INT_R 38
    { "-R",     GETOPTN_BOOL,
        NULL  /*&diopts->optval [DI_OPT_NO_SYMLINK]*/,
        0  /*sizeof (diopts->optval [DI_OPT_NO_SYMLINK])*/,
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
        NULL  /*&diopts->optval [DI_OPT_DISP_TOTALS] */,
        0  /*sizeof (diopts->optval [DI_OPT_DISP_TOTALS])*/,
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
    { "-w",     GETOPTN_IGNORE_ARG,
        NULL,
        0,
        NULL },
/* 48 */
#define OPT_INT_W 48
    { "-W",     GETOPTN_IGNORE_ARG,
        NULL,
        0,
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
        NULL  /*diopts->zoneDisplay*/,
        0  /*sizeof (diopts->zoneDisplay)*/,
        NULL },
/* 53 */
#define OPT_INT_Z 53
    { "-Z",     GETOPTN_STRING,
        NULL  /*diopts->.zoneDisplay*/,
        0  /*sizeof (diopts->zoneDisplay)*/,
        (void *) "all" }
  };
  static int scaleids [] =
    { OPT_INT_d, OPT_INT_g, OPT_INT_h, OPT_INT_H, OPT_INT_k, OPT_INT_m };
  static int paidb [] =
    { OPT_INT_a, OPT_INT_help, OPT_INT_qmark, OPT_INT_P,
      OPT_INT_si, OPT_INT_version };
  static int paidv [] =
    { OPT_INT_B, OPT_INT_I, OPT_INT_s, OPT_INT_x, OPT_INT_X };

  /* this is seriously gross, but the old compilers don't have    */
  /* automatic aggregate initialization                           */
  /* don't forget to change scaleids, paidb and paidv above also    */
  opts [OPT_INT_A].valptr = (void *) &diopts->formatString;   /* -A */
  opts [OPT_INT_c].valptr = (void *) &diopts->optval [DI_OPT_DISP_CSV];     /* -c */
  opts [OPT_INT_c].valsiz = sizeof (diopts->optval [DI_OPT_DISP_CSV]);
  opts [OPT_INT_C].valptr = (void *) &diopts->optval [DI_OPT_DISP_CSV_TAB];     /* -C */
  opts [OPT_INT_C].valsiz = sizeof (diopts->optval [DI_OPT_DISP_CSV_TAB]);
  opts [OPT_INT_f].valptr = (void *) &diopts->formatString;  /* -f */
  opts [OPT_INT_j].valptr = (void *) &diopts->optval [DI_OPT_DISP_JSON];     /* -j */
  opts [OPT_INT_j].valsiz = sizeof (diopts->optval [DI_OPT_DISP_JSON]);
  opts [OPT_INT_l].valptr = (void *) &diopts->optval [DI_OPT_LOCAL_ONLY];     /* -l */
  opts [OPT_INT_l].valsiz = sizeof (diopts->optval [DI_OPT_LOCAL_ONLY]);
  opts [OPT_INT_L].valptr = (void *) &diopts->optval [DI_OPT_EXCL_LOOPBACK]; /* -L */
  opts [OPT_INT_L].valsiz = sizeof (diopts->optval [DI_OPT_EXCL_LOOPBACK]);
  opts [OPT_INT_n].valptr = (void *) &diopts->optval [DI_OPT_DISP_HEADER];   /* -n */
  opts [OPT_INT_n].valsiz = sizeof (diopts->optval [DI_OPT_DISP_HEADER]);
  opts [OPT_INT_q].valptr = (void *) &diopts->optval [DI_OPT_QUOTA_CHECK];    /* -q */
  opts [OPT_INT_q].valsiz = sizeof (diopts->optval [DI_OPT_QUOTA_CHECK]);
  opts [OPT_INT_R].valptr = (void *) &diopts->optval [DI_OPT_NO_SYMLINK];    /* -R */
  opts [OPT_INT_R].valsiz = sizeof (diopts->optval [DI_OPT_NO_SYMLINK]);
  opts [OPT_INT_t].valptr = (void *) &diopts->optval [DI_OPT_DISP_TOTALS];    /* -t */
  opts [OPT_INT_t].valsiz = sizeof (diopts->optval [DI_OPT_DISP_TOTALS]);
  opts [OPT_INT_z].valptr = (void *) diopts->zoneDisplay;  /* -z */
  opts [OPT_INT_z].valsiz = sizeof (diopts->zoneDisplay);
  opts [OPT_INT_Z].valptr = (void *) diopts->zoneDisplay;  /* -Z */
  opts [OPT_INT_Z].valsiz = sizeof (diopts->zoneDisplay);

  for (i = 0; i < (int) (sizeof (scaleids) / sizeof (int)); ++i) {
    opts [scaleids [i]].valptr = (void *) scalestr;
    opts [scaleids [i]].valsiz = scalestrsz - 1;
  }
  for (i = 0; i < (int) (sizeof (paidb) / sizeof (int)); ++i) {
    opts [paidb [i]].valptr = (void *) &padata;
    opts [paidb [i]].value2 = (void *) processOptions;
    if (diopts->exitFlag != DI_EXIT_NORM) {
      break;
    }
  }
  for (i = 0; i < (int) (sizeof (paidv) / sizeof (int)); ++i) {
    opts [paidv [i]].valptr = (void *) &padata;
    opts [paidv [i]].value2 = (void *) processOptionsVal;
    if (diopts->exitFlag != DI_EXIT_NORM) {
      break;
    }
  }

  optidx = -1;
  if (diopts->exitFlag != DI_EXIT_NORM) {
    return optidx;
  }

  padata.diopts = diopts;
  padata.scalestr = scalestr;
  padata.scalestrsz = scalestrsz;

  optidx = getoptn (GETOPTN_LEGACY, argc, argv,
       sizeof (opts) / sizeof (getoptn_opt_t), opts, &errorCount);
  diopts->errorCount += errorCount;
  if (diopts->errorCount > 0) {
    setExitFlag (diopts, DI_EXIT_WARN);
  }

  if (diopts->optval [DI_OPT_DISP_CSV_TAB]) {
    diopts->optval [DI_OPT_DISP_CSV] = true;
  }
  if (diopts->optval [DI_OPT_DISP_CSV] || diopts->optval [DI_OPT_DISP_JSON]) {
    diopts->optval [DI_OPT_DISP_TOTALS] = false;
  }
  if (diopts->optval [DI_OPT_DISP_JSON]) {
    diopts->optval [DI_OPT_DISP_HEADER] = false;
  }

  return optidx;
}

static void
processOptions (const char *arg, char *valptr)
{
  struct pa_tmp     *padata;

  padata = (struct pa_tmp *) valptr;
  if (strcmp (arg, "-a") == 0) {
    padata->diopts->optval [DI_OPT_DISP_ALL] = true;
    stpecpy (padata->diopts->zoneDisplay,
        padata->diopts->zoneDisplay + sizeof (padata->diopts->zoneDisplay),
        "all");
  } else if (strcmp (arg, "--help") == 0 || strcmp (arg, "-?") == 0) {
    setExitFlag (padata->diopts, DI_EXIT_HELP);
  } else if (strcmp (arg, "-P") == 0) {
    /* always use -k option, 512 is not supported */
    stpecpy (padata->scalestr, padata->scalestr + padata->scalestrsz, "k");
    padata->diopts->formatString = DI_POSIX_FORMAT;
    padata->diopts->optval [DI_OPT_POSIX_COMPAT] = true;
    padata->diopts->optval [DI_OPT_DISP_CSV] = false;
    padata->diopts->optval [DI_OPT_DISP_JSON] = false;
  } else if (strcmp (arg, "--si") == 0) {
    stpecpy (padata->scalestr, padata->scalestr + padata->scalestrsz, "H");
  } else if (strcmp (arg, "--version") == 0) {
    setExitFlag (padata->diopts, DI_EXIT_VERS);
  } else {
    fprintf (stderr, "di_panic: bad option setup\n");
  }

  return;
}

static void
processOptionsVal (const char *arg, void *valptr, char *value)
{
  struct pa_tmp     *padata;
  int               rc;

  padata = (struct pa_tmp *) valptr;

  if (strcmp (arg, "-B") == 0) {
    if (isdigit ( (int) (*value))) {
      int     val;

      val = atoi (value);
      if (val == DI_BLKSZ_1000 || val == DI_BLKSZ_1024 ) {
        padata->diopts->blockSize = val;
      }
    } else if (strcmp (value, "k") == 0) {
      padata->diopts->blockSize = DI_BLKSZ_1024;
    }
    else if (strcmp (value, "d") == 0 || strcmp (value, "si") == 0) {
      padata->diopts->blockSize = DI_BLKSZ_1000;
    }
  } else if (strcmp (arg, "-I") == 0) {
    rc = parseList (&padata->diopts->include_list, value);
    if (rc != 0) {
      setExitFlag (padata->diopts, DI_EXIT_FAIL);
      return;
    }
  } else if (strcmp (arg, "-s") == 0) {
    char      *stend = padata->diopts->sortType + sizeof (padata->diopts->sortType);

    stpecpy (padata->scalestr, padata->scalestr + padata->scalestrsz, "H");
    stpecpy (padata->diopts->sortType, stend, value);
    /* for backwards compatibility                       */
    /* reverse by itself - change to reverse mount point */
    if (strcmp (padata->diopts->sortType, "r") == 0) {
      stpecpy (padata->diopts->sortType, stend, "rm");
    }
    /* add some sense to the sort order */
    if (strcmp (padata->diopts->sortType, "t") == 0) {
      stpecpy (padata->diopts->sortType, stend, "tm");
    }
  } else if (strcmp (arg, "-x") == 0) {
    parseList (&padata->diopts->ignore_list, value);
  } else if (strcmp (arg, "-X") == 0) {
    debug = atoi (value);
    padata->diopts->optval [DI_OPT_DISP_DBG_HEADER] = true;
    padata->diopts->optval [DI_OPT_DISP_TOTALS] = true;
    padata->diopts->optval [DI_OPT_DISP_HEADER] = true;
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
    fprintf (stderr, "strdup failed in parseList () (1).  errno %d\n", errno);
    return 1;
  }

  ptr = strtok (dstr, DI_LIST_SEP);
  count = 0;
  while (ptr != (char *) NULL) {
    ++count;
    ptr = strtok ( (char *) NULL, DI_LIST_SEP);
  }

  ocount = list->count;
  list->count += count;
  ncount = list->count;
  list->list = (char **) di_realloc ( (char *) list->list,
      (Size_t) list->count * sizeof (char *));
  if (list->list == (char **) NULL) {
    fprintf (stderr, "malloc failed in parseList () (2).  errno %d\n", errno);
    free ( (char *) dstr);
    return 1;
  }

  ptr = dstr;
  for (i = ocount; i < ncount; ++i) {
    len = (unsigned int) strlen (ptr);
    lptr = (char *) malloc ( (Size_t) len + 1);
    if (lptr == (char *) NULL) {
      fprintf (stderr, "malloc failed in parseList () (3).  errno %d\n", errno);
      free ( (char *) dstr);
      return 1;
    }
    stpecpy (lptr, lptr + len, ptr);
    lptr [len] = '\0';
    list->list [i] = lptr;
    ptr += len + 1;
  }

  free ( (char *) dstr);
  return 0;
}


static void
parseScaleValue (di_opt_t *diopts, char *ptr)
{
  unsigned int    len;
  int             i;
  int             val;
  char            *tptr;

  if (isdigit (*ptr)) {
    val = atoi (ptr);
    if (val != DI_BLKSZ_1 &&
        val != DI_BLKSZ_1000 && val != DI_BLKSZ_1024) {
      val = DI_BLKSZ_1024;
    }
    if (val == DI_BLKSZ_1) {
      diopts->scale = DI_SCALE_BYTE;
    }
    if (val == DI_BLKSZ_1000) {
      diopts->scale = DI_SCALE_KILO;
      diopts->blockSize = DI_BLKSZ_1000;
    }
    if (val == DI_BLKSZ_1024) {
      diopts->scale = DI_SCALE_KILO;
      diopts->blockSize = DI_BLKSZ_1024;
    }
  }

  tptr = ptr;
  len = (unsigned int) strlen (ptr);
  if (! isdigit (*tptr)) {
    int             idx;

    idx = -1;
    for (i = 0; i < DI_VALID_SCALE_SZ; ++i) {
      if (*tptr == validscale [i].uc || *tptr == validscale [i].lc) {
        idx = i;
        break;
      }
    }

    if (idx == -1) {
      if (*tptr == 'h') {
        idx = DI_SCALE_HR;
      }
      if (*tptr == 'H') {
        idx = DI_SCALE_HR_ALT;
      }
    }

    if (idx == -1) {
      if (strncmp (ptr, "HUMAN", (Size_t) 5) == 0) {
        idx = DI_SCALE_HR;
      } else {
        /* some unknown string value */
        idx = DI_SCALE_GIGA;
      }
    }

    if (idx >= 0) {
      if (len > 1) {
        ++tptr;
        if (*tptr == 'B') {
          diopts->blockSize = DI_BLKSZ_1000;
        }
      }
    } /* known size multiplier */

    diopts->scale = idx;
  }
}


static void
setExitFlag (di_opt_t *diopts, int exitFlag)
{
  if (exitFlag > diopts->exitFlag) {
    diopts->exitFlag = exitFlag;
  }
}
