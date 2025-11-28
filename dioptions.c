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
#include "disystem.h"
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

/* these must match the DI_SCALE* defines in di.h */
static di_valid_scale_t validscale [DI_SCALE_MAX] =
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

#define OPT_IDX_A         0
#define OPT_IDX_a         1
#define OPT_IDX_B         2
#define OPT_IDX_c         3
#define OPT_IDX_C         4
#define OPT_IDX_d         5
#define OPT_IDX_f         6
#define OPT_IDX_g         7
#define OPT_IDX_h         8
#define OPT_IDX_H         9
#define OPT_IDX_help      10
#define OPT_IDX_I         11
#define OPT_IDX_j         12
#define OPT_IDX_k         13
#define OPT_IDX_l         14
#define OPT_IDX_L         15
#define OPT_IDX_m         16
#define OPT_IDX_n         17
#define OPT_IDX_P         18
#define OPT_IDX_q         19
#define OPT_IDX_R         20
#define OPT_IDX_s         21
#define OPT_IDX_si        22
#define OPT_IDX_t         23
#define OPT_IDX_version   24
#define OPT_IDX_x         25
#define OPT_IDX_X         26
#define OPT_IDX_z         27
#define OPT_IDX_Z         28
#define OPT_IDX_MAX_NAMED 29
#define OPT_IDX_MAX       53


static int scaleids [] =
  { OPT_IDX_d, OPT_IDX_g, OPT_IDX_h, OPT_IDX_H, OPT_IDX_k, OPT_IDX_m };
static int paidb [] =
  { OPT_IDX_a, OPT_IDX_help, OPT_IDX_P, OPT_IDX_si, OPT_IDX_version };
static int paidv [] =
  { OPT_IDX_B, OPT_IDX_I, OPT_IDX_s, OPT_IDX_x, OPT_IDX_X };

#define DI_ARGV_SEP             " 	"  /* space, tab */
#define DI_MAX_ARGV             50
#define DI_LIST_SEP             ","

#define DI_POSIX_FORMAT         "sbuvpm"
#define DI_ALL_FORMAT           "mts\n\tO\n\tbuf13\n\tbcvpa\n\tBuv2\n\tiUFP"

static void processStringArgs (char *, di_opt_t *, int offset, char *, Size_t);
static int  processArgs (int, const char * argv [], di_opt_t *, int offset, char *, Size_t);
static int  parseList (di_strarr_t *, char *);
static void parseScaleValue (di_opt_t *diopts, char *ptr);
static void processOptions (const char *, char *);
static void processOptionsVal (const char *, void *, char *);
static void setExitFlag (di_opt_t *, int);
static void diopt_init (di_opt_t *diopts, struct pa_tmp *);

static void
processStringArgs (char *ptr, di_opt_t *diopts,
    int offset, char *scalestr, Size_t scalestrsz)
{
  char        *dptr;
  char        *tptr;
  int         nargc;
  char        *nargv [DI_MAX_ARGV];
  int         optidx;
  char        *tokstr = NULL;


  if (ptr == (char *) NULL || strcmp (ptr, "") == 0) {
    return;
  }

  dptr = strdup (ptr);
  if (dptr == (char *) NULL) {
    fprintf (stderr, "strdup failed in main () (1).  errno %d\n", errno);
    setExitFlag (diopts, DI_EXIT_FAIL);
    return;
  }

  tptr = di_strtok (dptr, DI_ARGV_SEP, &tokstr);
  nargc = 0;
  while (tptr != (char *) NULL) {
    if (nargc >= DI_MAX_ARGV) {
      break;
    }
    nargv [nargc++] = tptr;
    tptr = di_strtok ((char *) NULL, DI_ARGV_SEP, &tokstr);
  }
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wcast-qual"
  optidx = processArgs (nargc, (const char **) nargv,
      diopts, 0, scalestr, scalestrsz);
#pragma clang diagnostic pop
  if (optidx < nargc) {
    fprintf (stderr, "unknown data found in DI_ARGS %s\n",
        nargv [optidx]);
    diopts->errorCount += 1;
    if (diopts->errorCount > 0) {
      setExitFlag (diopts, DI_EXIT_WARN);
    }
  }

  /* the tokenized DI_ARGS data must be preserved so it can be used */
  diopts->diargsptr = dptr;
}

di_opt_t *
di_init_options (void)
{
  di_opt_t    *diopts;
  int         i;

  diopts = (di_opt_t *) malloc (sizeof (di_opt_t));
  if (diopts == NULL) {
    return diopts;
  }

  diopts->opts = NULL;
  diopts->optinit = false;

  diopts->formatString = DI_DEFAULT_FORMAT;
  diopts->formatLen = (int) strlen (diopts->formatString);
  diopts->zoneDisplay [0] = '\0';
  diopts->exclude_list.count = 0;
  diopts->exclude_list.list = (char **) NULL;
  diopts->include_list.count = 0;
  diopts->include_list.list = (char **) NULL;
  diopts->scale = DI_SCALE_GIGA;
  diopts->blockSize = DI_BLKSZ_1024;
  for (i = 0; i < DI_OPT_MAX; ++i) {
    diopts->optval [i] = false;
  }
  diopts->optval [DI_OPT_DISP_HEADER] = true;
  diopts->optval [DI_OPT_EXCL_LOOPBACK] = true;
  diopts->optval [DI_OPT_QUOTA_CHECK] = true;
  diopts->optval [DI_OPT_DEBUG] = 0;

  /* default - by mount point*/
  stpecpy (diopts->sortType, diopts->sortType + sizeof (diopts->sortType), "m");
  diopts->exitFlag = DI_EXIT_NORM;
  diopts->errorCount = 0;
  diopts->diargsptr = NULL;

  return diopts;
}

void
di_opt_cleanup (di_opt_t *diopts)
{
  if (diopts == NULL) {
    return;
  }

  if (diopts->exclude_list.count > 0 &&
      diopts->exclude_list.list != (char **) NULL) {
    free ( (void *) diopts->exclude_list.list);
    diopts->exclude_list.count = 0;
  }

  if (diopts->include_list.count > 0 &&
      diopts->include_list.list != (char **) NULL) {
    free (diopts->include_list.list);
    diopts->include_list.count = 0;
  }

  if (diopts->diargsptr != NULL) {
    free (diopts->diargsptr);
  }

  if (diopts->opts != NULL) {
    free (diopts->opts);
  }

  free (diopts);
}

int
di_get_options (int argc, const char * argv [], di_opt_t *diopts, int offset)
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

  /* gnu-utils processes in BLOCKSIZE, BLOCK_SIZE, DF_BLOCK_SIZE order */

  /* bsd df */
  if ( (ptr = getenv ("BLOCKSIZE")) != (char *) NULL) {
    stpecpy (scalestr, scalestr + sizeof (scalestr), ptr);
  }

  /* gnu utils */
  if ( (ptr = getenv ("BLOCK_SIZE")) != (char *) NULL) {
    stpecpy (scalestr, scalestr + sizeof (scalestr), ptr);
  }

  /* gnu df, overrides BLOCK_SIZE */
  if ( (ptr = getenv ("DF_BLOCK_SIZE")) != (char *) NULL) {
    stpecpy (scalestr, scalestr + sizeof (scalestr), ptr);
  }

  if ( (ptr = getenv ("DI_ARGS")) != (char *) NULL) {
    processStringArgs (ptr, diopts, offset, scalestr, sizeof (scalestr));
  }

  optidx = processArgs (argc, argv, diopts, offset, scalestr, sizeof (scalestr));

  parseScaleValue (diopts, scalestr);

  if (diopts->optval [DI_OPT_DEBUG] > 0) {
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
    if ( (ptr = getenv ("BLOCK_SIZE")) != (char *) NULL) {
      printf ("# BLOCK_SIZE: %s\n", ptr);
    }
    if ( (ptr = getenv ("DF_BLOCK_SIZE")) != (char *) NULL) {
      printf ("# DF_BLOCK_SIZE: %s\n", ptr);
    }
    if ( (ptr = getenv ("DI_ARGS")) != (char *) NULL) {
      printf ("# DI_ARGS: %s\n", ptr);
    }
  }

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
processArgs (int argc, const char * argv [], di_opt_t *diopts,
    int offset, char *scalestr, Size_t scalestrsz)
{
  int           i;
  int           optidx;
  int           errorCount;
  struct pa_tmp padata;

  diopt_init (diopts, &padata);

  for (i = 0; i < (int) (sizeof (scaleids) / sizeof (int)); ++i) {
    diopts->opts [scaleids [i]].valptr = (void *) scalestr;
    diopts->opts [scaleids [i]].valsiz = scalestrsz;
  }
  for (i = 0; i < (int) (sizeof (paidb) / sizeof (int)); ++i) {
    if (diopts->exitFlag != DI_EXIT_NORM) {
      break;
    }
    diopts->opts [paidb [i]].valptr = (void *) &padata;
    diopts->opts [paidb [i]].value2 = (void *) processOptions;
  }
  for (i = 0; i < (int) (sizeof (paidv) / sizeof (int)); ++i) {
    if (diopts->exitFlag != DI_EXIT_NORM) {
      break;
    }
    diopts->opts [paidv [i]].valptr = (void *) &padata;
    diopts->opts [paidv [i]].value2 = (void *) processOptionsVal;
  }

  optidx = -1;
  if (diopts->exitFlag != DI_EXIT_NORM) {
    return optidx;
  }

  padata.diopts = diopts;
  padata.scalestr = scalestr;
  padata.scalestrsz = scalestrsz;

  optidx = getoptn (GETOPTN_LEGACY, argc, argv,
       OPT_IDX_MAX, diopts->opts, offset, &errorCount);
  diopts->errorCount += errorCount;
  if (diopts->errorCount > 0) {
    setExitFlag (diopts, DI_EXIT_WARN);
  }

  if (diopts->optval [DI_OPT_DISP_CSV_TAB]) {
    diopts->optval [DI_OPT_DISP_CSV] = true;
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
    padata->diopts->formatLen = (int) strlen (padata->diopts->formatString);
    padata->diopts->optval [DI_OPT_POSIX_COMPAT] = true;
    padata->diopts->optval [DI_OPT_DISP_CSV] = false;
    padata->diopts->optval [DI_OPT_DISP_JSON] = false;
  } else if (strcmp (arg, "--si") == 0) {
    stpecpy (padata->scalestr, padata->scalestr + padata->scalestrsz, "h");
    padata->diopts->blockSize = DI_BLKSZ_1000;
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
    parseScaleValue (padata->diopts, value);
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
    parseList (&padata->diopts->exclude_list, value);
  } else if (strcmp (arg, "-X") == 0) {
    padata->diopts->optval [DI_OPT_DEBUG] = atoi (value);
    padata->diopts->optval [DI_OPT_DISP_HEADER] = true;
  } else {
    fprintf (stderr, "di_panic: bad option setup\n");
  }

  return;
}

static int
parseList (di_strarr_t *list, char *str)
{
  char          *dstr;
  char          *ptr;
  char          *lptr;
  Size_t        count;
  Size_t        ocount;
  Size_t        ncount;
  Size_t        i;
  char          *tokstr = NULL;

  dstr = strdup (str);
  if (dstr == (char *) NULL) {
    fprintf (stderr, "strdup failed in parseList () (1).  errno %d\n", errno);
    return 1;
  }

  ptr = di_strtok (dstr, DI_LIST_SEP, &tokstr);
  count = 0;
  while (ptr != (char *) NULL) {
    ++count;
// fprintf (stderr, "%ld : %s\n", count, ptr);
    ptr = di_strtok ( (char *) NULL, DI_LIST_SEP, &tokstr);
  }

  ocount = list->count;
// fprintf (stderr, "ocount: %ld\n", ocount);
  list->count += count;
  ncount = list->count;
// fprintf (stderr, "ncount: %ld\n", ncount);
  list->list = (char **) di_realloc ( (char *) list->list,
      ncount * sizeof (char *));
  if (list->list == (char **) NULL) {
    fprintf (stderr, "realloc failed in parseList () (2).  errno %d\n", errno);
    free ( (char *) dstr);
    return 1;
  }

  ptr = dstr;
  for (i = ocount; i < ncount; ++i) {
    Size_t      len;

    lptr = strdup (ptr);
    if (lptr == (char *) NULL) {
      fprintf (stderr, "strdup failed in parseList () (3).  errno %d\n", errno);
      free ( (char *) dstr);
      return 1;
    }
    list->list [i] = lptr;
// fprintf (stderr, "b: %ld : %s\n", i, lptr);
    len = (unsigned int) strlen (ptr);
    ptr += len + 1;
  }

  free ( (char *) dstr);
  return 0;
}


static void
parseScaleValue (di_opt_t *diopts, char *ptr)
{
  int             i;
  int             val;

  if (*ptr == '\'') {
    /* gnu-utils leading ' indicates separator printing */
    /* ignore this */
    ++ptr;
  }

  /* the environment variables that bsd-df and gnu-utils use specify */
  /* both the block size and the scaling value in a single string */
  /* https://man.freebsd.org/cgi/man.cgi?query=environ&sektion=7&manpath=freebsd-release-ports */
  /* https://www.gnu.org/software/coreutils/manual/html_node/Block-size.html */

  /* if a numeric value is specified, only 1, 1000 and 1024 are allowed */
  /* bad values will default to 1024 (..ibi-bytes) */
  /* di does not support the 512 byte blocksize */
  if (isdigit ((int) *ptr)) {
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

  while (isdigit ((int) *ptr)) {
    ++ptr;
  }

  /* at this point, ptr will be pointing at the string modifier, */
  /* or at a null byte */
  if (*ptr != '\0' && ! isdigit ((int) *ptr)) {
    int             idx;

    idx = -1;
    for (i = 0; i < DI_SCALE_MAX; ++i) {
      if (*ptr == validscale [i].uc || *ptr == validscale [i].lc) {
        idx = i;
        break;
      }
    }

    if (idx == -1) {
      if (*ptr == 'h') {
        /* gnu-utils: "human-readable" */
        idx = DI_SCALE_HR;
      }
      if (*ptr == 'H') {
        idx = DI_SCALE_HR_ALT;
      }
      /* 'd' is for old di compatibility -B d */
      /* it did not set the scaling */
      if (*ptr == 'd') {
        diopts->blockSize = DI_BLKSZ_1000;
      }
      if (*ptr == 's') {
        /* gnu-utils: si */
        idx = DI_SCALE_HR;
        diopts->blockSize = DI_BLKSZ_1000;
      }
    }

    if (idx == -1) {
      /* BSD uses 'HUMAN' */
      if (strncmp (ptr, "HUMAN", (Size_t) 5) == 0) {
        idx = DI_SCALE_HR;
      } else {
        /* some unknown string value, this is the default */
        idx = DI_SCALE_GIGA;
      }
    }

    if (idx >= 0) {
      ++ptr;
      if (*ptr == '\0') {
        /* a string without a following i or B has a block size of 1024 */
        /* this is a historical default */
        diopts->blockSize = DI_BLKSZ_1024;
      }
      if (*ptr == 'i') {
        /* gnu df allows MiB, etc. */
        diopts->blockSize = DI_BLKSZ_1024;
      }
      if (*ptr == 'B') {
        diopts->blockSize = DI_BLKSZ_1000;
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

static void
diopt_init (di_opt_t *diopts, struct pa_tmp *padata)
{
  int       i;
  int       c;

  if (diopts->optinit) {
    return;
  }

  diopts->opts = (getoptn_opt_t *) malloc (sizeof (getoptn_opt_t) * OPT_IDX_MAX);
  if (diopts->opts == NULL) {
    fprintf (stderr, "malloc failed in diopt_init.  errno %d\n", errno);
    return;
  }
  for (i = 0; i < OPT_IDX_MAX; ++i) {
    diopts->opts [i].option = NULL;
    diopts->opts [i].option_type = GETOPTN_BOOL;
    diopts->opts [i].valptr = NULL;
    diopts->opts [i].valsiz = 0;
    diopts->opts [i].value2 = NULL;
  }

  diopts->opts [OPT_IDX_A].option = "-A";
  diopts->opts [OPT_IDX_A].option_type = GETOPTN_STRPTR;
  diopts->opts [OPT_IDX_A].valptr = &diopts->formatString;
  diopts->opts [OPT_IDX_A].value2 = (void *) DI_ALL_FORMAT;

  diopts->opts [OPT_IDX_a].option = "-a";
  diopts->opts [OPT_IDX_a].option_type = GETOPTN_FUNC_BOOL;
  /* valptr : padata */
  /* value2 : processOptions */

  diopts->opts [OPT_IDX_B].option = "-B";
  diopts->opts [OPT_IDX_B].option_type = GETOPTN_FUNC_VALUE;
  /* valptr : padata */
  /* value2 : processOptionsVal */

  diopts->opts [OPT_IDX_c].option = "-c";
  diopts->opts [OPT_IDX_c].option_type = GETOPTN_BOOL;
  diopts->opts [OPT_IDX_c].valptr = &diopts->optval [DI_OPT_DISP_CSV];
  diopts->opts [OPT_IDX_c].valsiz = sizeof (diopts->optval [DI_OPT_DISP_CSV]);

  diopts->opts [OPT_IDX_C].option = "-C";
  diopts->opts [OPT_IDX_C].option_type = GETOPTN_BOOL;
  diopts->opts [OPT_IDX_C].valptr = &diopts->optval [DI_OPT_DISP_CSV_TAB];
  diopts->opts [OPT_IDX_C].valsiz = sizeof (diopts->optval [DI_OPT_DISP_CSV_TAB]);

  diopts->opts [OPT_IDX_d].option = "-d";
  diopts->opts [OPT_IDX_d].option_type = GETOPTN_STRING;
  /* valptr :  scalestr  */
  /* valsiz :  scalestrsz  */

  diopts->opts [OPT_IDX_f].option = "-f";
  diopts->opts [OPT_IDX_f].option_type = GETOPTN_STRPTR;
  diopts->opts [OPT_IDX_f].valptr = &diopts->formatString;

  diopts->opts [OPT_IDX_g].option = "-g";
  diopts->opts [OPT_IDX_g].option_type = GETOPTN_STRING;
  /* valptr :  scalestr  */
  /* valsiz :  scalestrsz  */
  diopts->opts [OPT_IDX_g].value2 = (void *) "g";

  diopts->opts [OPT_IDX_h].option = "-h";
  diopts->opts [OPT_IDX_h].option_type = GETOPTN_STRING;
  /* valptr :  scalestr  */
  /* valsiz :  scalestrsz  */
  diopts->opts [OPT_IDX_h].value2 = (void *) "h";

  diopts->opts [OPT_IDX_H].option = "-H";
  diopts->opts [OPT_IDX_H].option_type = GETOPTN_STRING;
  /* valptr :  scalestr  */
  /* valsiz :  scalestrsz  */
  diopts->opts [OPT_IDX_H].value2 = (void *) "H";

  diopts->opts [OPT_IDX_help].option = "--help";
  diopts->opts [OPT_IDX_help].option_type = GETOPTN_FUNC_BOOL;
  /* value2 : processOptions */

  diopts->opts [OPT_IDX_I].option = "-I";
  diopts->opts [OPT_IDX_I].option_type = GETOPTN_FUNC_VALUE;
  /* valptr : padata */
  /* value2 : processOptionsVal */

  diopts->opts [OPT_IDX_j].option = "-j";
  diopts->opts [OPT_IDX_j].option_type = GETOPTN_BOOL;
  diopts->opts [OPT_IDX_j].valptr = &diopts->optval [DI_OPT_DISP_JSON];
  diopts->opts [OPT_IDX_j].valsiz = sizeof (diopts->optval [DI_OPT_DISP_JSON]);

  diopts->opts [OPT_IDX_k].option = "-k";
  diopts->opts [OPT_IDX_k].option_type = GETOPTN_STRING;
  /* valptr :  scalestr  */
  /* valsiz :  scalestrsz  */
  diopts->opts [OPT_IDX_k].value2 = (void *) "k";

  diopts->opts [OPT_IDX_l].option = "-l";
  diopts->opts [OPT_IDX_l].option_type = GETOPTN_BOOL;
  diopts->opts [OPT_IDX_l].valptr =  &diopts->optval [DI_OPT_LOCAL_ONLY];
  diopts->opts [OPT_IDX_l].valsiz =  sizeof (diopts->optval [DI_OPT_LOCAL_ONLY]);

  diopts->opts [OPT_IDX_L].option  = "-L";
  diopts->opts [OPT_IDX_L].option_type = GETOPTN_BOOL;
  diopts->opts [OPT_IDX_L].valptr = &diopts->optval [DI_OPT_EXCL_LOOPBACK];
  diopts->opts [OPT_IDX_L].valsiz = sizeof (diopts->optval [DI_OPT_EXCL_LOOPBACK]);

  diopts->opts [OPT_IDX_m].option = "-m";
  diopts->opts [OPT_IDX_m].option_type = GETOPTN_STRING;
  /* valptr :  scalestr  */
  /* valsiz :  scalestrsz  */
  diopts->opts [OPT_IDX_m].value2 = (void *) "m";

  diopts->opts [OPT_IDX_n].option = "-n";
  diopts->opts [OPT_IDX_n].option_type = GETOPTN_BOOL;
  diopts->opts [OPT_IDX_n].valptr = &diopts->optval [DI_OPT_DISP_HEADER];
  diopts->opts [OPT_IDX_n].valsiz = sizeof (diopts->optval [DI_OPT_DISP_HEADER]);

  diopts->opts [OPT_IDX_P].option = "-P";
  diopts->opts [OPT_IDX_P].option_type = GETOPTN_FUNC_BOOL;
  /* valptr : padata */
  /* value2 : processOptions */

  diopts->opts [OPT_IDX_q].option = "-q";
  diopts->opts [OPT_IDX_q].option_type = GETOPTN_BOOL;
  diopts->opts [OPT_IDX_q].valptr = &diopts->optval [DI_OPT_QUOTA_CHECK];
  diopts->opts [OPT_IDX_q].valsiz = sizeof (diopts->optval [DI_OPT_QUOTA_CHECK]);

  diopts->opts [OPT_IDX_R].option = "-R";
  diopts->opts [OPT_IDX_R].option_type = GETOPTN_BOOL;
  diopts->opts [OPT_IDX_R].valptr = &diopts->optval [DI_OPT_NO_SYMLINK];
  diopts->opts [OPT_IDX_R].valsiz = sizeof (diopts->optval [DI_OPT_NO_SYMLINK]);

  diopts->opts [OPT_IDX_s].option = "-s";
  diopts->opts [OPT_IDX_s].option_type = GETOPTN_FUNC_VALUE;
  /* valptr : padata */
  /* value2 : processOptionsVal */

  diopts->opts [OPT_IDX_si].option = "--si";
  diopts->opts [OPT_IDX_si].option_type = GETOPTN_FUNC_BOOL;
  /* valptr : padata */
  /* value2 : processOptions */

  diopts->opts [OPT_IDX_t].option = "-t";
  diopts->opts [OPT_IDX_t].option_type = GETOPTN_BOOL;
  diopts->opts [OPT_IDX_t].valptr = &diopts->optval [DI_OPT_DISP_TOTALS] ;
  diopts->opts [OPT_IDX_t].valsiz = sizeof (diopts->optval [DI_OPT_DISP_TOTALS]);

  diopts->opts [OPT_IDX_version].option = "--version";
  diopts->opts [OPT_IDX_version].option_type = GETOPTN_FUNC_BOOL;
  /* value2 : processOptions */

  diopts->opts [OPT_IDX_x].option = "-x";
  diopts->opts [OPT_IDX_x].option_type = GETOPTN_FUNC_VALUE;
  /* valptr : padata */
  /* value2 : processOptionsVal */

  diopts->opts [50].option = "--exclude-type";
  diopts->opts [50].option_type = GETOPTN_ALIAS;
  diopts->opts [50].valptr = (void *) "-x";

  diopts->opts [OPT_IDX_X].option = "-X";
  diopts->opts [OPT_IDX_X].option_type = GETOPTN_FUNC_VALUE;
  /* valptr : padata */
  /* value2 : processOptionsVal */

  diopts->opts [OPT_IDX_z].option = "-z";
  diopts->opts [OPT_IDX_z].option_type = GETOPTN_STRING;
  diopts->opts [OPT_IDX_z].valptr = (void *) diopts->zoneDisplay;
  diopts->opts [OPT_IDX_z].valsiz = sizeof (diopts->zoneDisplay);

  diopts->opts [OPT_IDX_Z].option = "-Z";
  diopts->opts [OPT_IDX_Z].option_type = GETOPTN_STRING;
  diopts->opts [OPT_IDX_Z].valptr = (void *) diopts->zoneDisplay;
  diopts->opts [OPT_IDX_Z].valsiz = sizeof (diopts->zoneDisplay);
  diopts->opts [OPT_IDX_Z].value2 = (void *) "all";

  c = OPT_IDX_MAX_NAMED;

  diopts->opts [c].option = "--all";
  diopts->opts [c].option_type = GETOPTN_ALIAS;
  diopts->opts [c].valptr = (void *) "-a";
  ++c;

  diopts->opts [c].option = "-b";
  diopts->opts [c].option_type = GETOPTN_ALIAS;
  diopts->opts [c].valptr = (void *) "-B";
  ++c;

  diopts->opts [c].option = "--block-size";
  diopts->opts [c].option_type = GETOPTN_ALIAS;
  diopts->opts [c].valptr = (void *) "-B";
  ++c;

  diopts->opts [c].option = "--display-size";
  diopts->opts [c].option_type = GETOPTN_ALIAS;
  diopts->opts [c].valptr = (void *) "-d";
  ++c;

  diopts->opts [c].option = "--dont-resolve-symlink";
  diopts->opts [c].option_type = GETOPTN_ALIAS;
  diopts->opts [c].valptr = (void *) "-R";
  ++c;

  diopts->opts [c].option = "--csv-output";
  diopts->opts [c].option_type = GETOPTN_ALIAS;
  diopts->opts [c].valptr = (void *) "-c";
  ++c;

  diopts->opts [c].option = "--csv-tabs";
  diopts->opts [c].option_type = GETOPTN_ALIAS;
  diopts->opts [c].valptr = (void *) "-C";
  ++c;

  diopts->opts [c].option = "--format-string";
  diopts->opts [c].option_type = GETOPTN_ALIAS;
  diopts->opts [c].valptr = (void *) "-f";
  ++c;

  diopts->opts [c].option = "-F";
  diopts->opts [c].option_type = GETOPTN_ALIAS;
  diopts->opts [c].valptr = (void *) "-I";
  ++c;

  diopts->opts [c].option = "--human-readable";
  diopts->opts [c].option_type = GETOPTN_ALIAS;
  diopts->opts [c].valptr = (void *) "-H";
  ++c;

  diopts->opts [c].option = "-?";
  diopts->opts [c].option_type = GETOPTN_ALIAS;
  diopts->opts [c].valptr = (void *) "--help";
  ++c;

  diopts->opts [c].option = "-i";
  diopts->opts [c].option_type = GETOPTN_ALIAS;
  diopts->opts [c].valptr = (void *) "-x";
  ++c;

  diopts->opts [c].option = "--inodes";
  diopts->opts [c].option_type = GETOPTN_IGNORE;
  ++c;

  diopts->opts [c].option = "--json-output";
  diopts->opts [c].option_type = GETOPTN_ALIAS;
  diopts->opts [c].valptr = (void *) "-j";
  ++c;

  diopts->opts [c].option = "--local";
  diopts->opts [c].option_type = GETOPTN_ALIAS;
  diopts->opts [c].valptr = (void *) "-l";
  ++c;

  diopts->opts [c].option = "--no-sync";
  diopts->opts [c].option_type = GETOPTN_IGNORE;
  ++c;

  diopts->opts [c].option = "--portability";
  diopts->opts [c].option_type = GETOPTN_ALIAS;
  diopts->opts [c].valptr = (void *) "-P";
  ++c;

  diopts->opts [c].option = "--print-type";
  diopts->opts [c].option_type = GETOPTN_IGNORE;
  ++c;

  diopts->opts [c].option = "--sync";
  diopts->opts [c].option_type = GETOPTN_IGNORE;
  ++c;

  diopts->opts [c].option = "--total";
  diopts->opts [c].option_type = GETOPTN_ALIAS;
  diopts->opts [c].valptr = (void *) "-t";
  ++c;

  diopts->opts [c].option = "--type";
  diopts->opts [c].option_type = GETOPTN_ALIAS;
  diopts->opts [c].valptr = (void *) "-I";
  ++c;

  diopts->opts [c].option = "-v";
  diopts->opts [c].option_type = GETOPTN_IGNORE;
  ++c;

  diopts->opts [c].option = "-w";
  diopts->opts [c].option_type = GETOPTN_IGNORE_ARG;
  ++c;

  diopts->opts [c].option = "-W";
  diopts->opts [c].option_type = GETOPTN_IGNORE_ARG;
  ++c;

  if (c != OPT_IDX_MAX) {
    fprintf (stderr, "incorrect option initialization %d/%d\n",
        c, OPT_IDX_MAX);
    return;
  }
  diopts->optinit = true;
}
