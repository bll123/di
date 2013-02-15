/*
 * $Id$
 * $Source$
 *
 * Copyright 1994-2013 Brad Lanam, Walnut Creek, CA
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
    && ! defined (_DI_INC_SYS_TYPES_H) /* xenix */
# define _DI_INC_SYS_TYPES_H
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
    char            *disp[2];
} dispTable_t;

static dispTable_t dispTable [] =
{
    { 0.0, { "KBytes", "KBytes" } },
    { 0.0, { "Megs", "Mebis" } },
    { 0.0, { "Gigs", "Gibis" } },
    { 0.0, { "Teras", "Tebis" } },
    { 0.0, { "Petas", "Pebis" } },
    { 0.0, { "Exas", "Exbis" } },
    { 0.0, { "Zettas", "Zebis" } },
    { 0.0, { "Yottas", "Yobis" } }
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

static void processStringArgs _((char *, diData_t *, char *));
static int  processArgs  _((int, const char * const [], diData_t *, char *, Size_t));
static void parseList           _((iList_t *, char *));
static void processOptions      _((const char *, char *));
static void processOptionsVal   _((const char *, char *, char *));
static void usage               _((void));
static void setDispBlockSize    _((char *, diOptions_t *, diOutput_t *));
static void initDisplayTable    _((diOptions_t *));

# if defined (__cplusplus) || defined (c_plusplus)
   }
# endif

static void
#if _proto_stdc
processStringArgs (char *ptr, diData_t *diData, char *dbsstr)
#else
processStringArgs (ptr, diData, dbsstr)
  char      *ptr;
  diData_t  *diData;
  char      *dbsstr;
#endif
{
  char        *dptr;
  char        *tptr;
  int         nargc;
  const char  *nargv [DI_MAX_ARGV];

  if (ptr == (char *) NULL || strcmp (ptr, "") == 0) {
    return;
  }

  dptr = (char *) NULL;
  dptr = strdup (ptr);
  if (dptr == (char *) NULL)
  {
      fprintf (stderr, "strdup failed in main() (1).  errno %d\n", errno);
      exit (1);
  }
  if (dptr != (char *) NULL)
  {
    tptr = strtok (dptr, DI_ARGV_SEP);
    nargc = 1;
    nargv[0] = "";
    while (tptr != (char *) NULL)
    {
        if (nargc >= DI_MAX_ARGV)
        {
            break;
        }
        nargv[nargc++] = tptr;
        tptr = strtok ((char *) NULL, DI_ARGV_SEP);
    }
    processArgs (nargc, nargv, diData, dbsstr, sizeof (dbsstr) - 1);
    free ((char *) dptr); /* ### legal here? */
  }
}

int
#if _proto_stdc
getDIOptions (int argc, const char * const argv[], diData_t *diData)
#else
getDIOptions (argc, argv, diData)
  int           argc;
  const char    * const argv[];
  diData_t      *diData;
#endif
{
  const char *      argvptr;
  char *            ptr;
  char              dbsstr [30];
  int               optidx;
  diOptions_t       *diopts;
  diOutput_t        *diout;

  diopts = &diData->options;
  diout = &diData->output;
  strncpy (dbsstr, DI_DEFAULT_DISP_SIZE, sizeof (dbsstr)); /* default */

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
  }

      /* bsd df */
  if ((ptr = getenv ("BLOCKSIZE")) != (char *) NULL) {
    strncpy (dbsstr, ptr, sizeof (dbsstr));
  }

      /* gnu df */
  if ((ptr = getenv ("DF_BLOCK_SIZE")) != (char *) NULL) {
    strncpy (dbsstr, ptr, sizeof (dbsstr));
  }

  if ((ptr = getenv ("DI_ARGS")) != (char *) NULL) {
    if (debug > 0) {
      printf ("# DI_ARGS:%s\n", ptr);
    }
    processStringArgs (ptr, diData, dbsstr);
  }

  if (debug > 0) {
    int j;
    printf ("# ARGS:");
    for (j = 0; j < argc; ++j)
    {
      printf (" %s", argv[j]);
    }
    printf ("\n");
    printf ("# blocksize: %s\n", dbsstr);
  }
  optidx = processArgs (argc, argv, diData, dbsstr, sizeof (dbsstr) - 1);

  initDisplayTable (diopts);
  setDispBlockSize (dbsstr, diopts, diout);

  return optidx;
}

static int
#if _proto_stdc
processArgs (int argc,
             const char * const argv [],
             diData_t *diData,
             char *dbsstr,
             Size_t dbsstr_sz)
#else
processArgs (argc, argv, diData, dbsstr, dbsstr_sz)
    int             argc;
    const char      * const argv [];
    diData_t        *diData;
    char            *dbsstr;
    Size_t          dbsstr_sz;
#endif
{
  int           i;
  int           optidx;
  diOptions_t   *diopts;
  diOutput_t    *diout;
  struct pa_tmp padata;

    /* the really old compilers don't have automatic initialization */
  static getoptn_opt_t opts[] = {
    { "-A",     GETOPTN_STRPTR,            /* 0 */
        NULL  /*&diopts->formatString*/,
        0,
        (void *) DI_ALL_FORMAT },
    { "-a",     GETOPTN_FUNC_BOOL,
        NULL  /*&padata*/,
        0,
        NULL  /*processOptions*/ },
    { "--all",  GETOPTN_ALIAS,
        (void *) "-a",
        0,
        NULL },
    { "-B",     GETOPTN_FUNC_VALUE,
        NULL  /*&padata*/,
        0,
        NULL  /*processOptionsVal*/ },
    { "-b",     GETOPTN_ALIAS,
        (void *) "-B",
        0,
        NULL },
    { "--block-size",   GETOPTN_ALIAS,      /* 5 */
        (void *) "-B",
        0,
        NULL },
    { "-c",     GETOPTN_BOOL,
        NULL  /*&diopts->csv_output*/,
        0     /*sizeof(diopts->csv_output)*/,
        NULL },
    { "--csv-output", GETOPTN_ALIAS,
        (void *) "-c",
        0,
        NULL },
    { "-d",     GETOPTN_STRING,
        NULL  /*dbsstr*/,
        0  /*dbsstr_sz*/,
        NULL },
    { "--display-size",     GETOPTN_ALIAS,
        (void *) "-d",
        0,
        NULL },
    { "--dont-resolve-symlink",     GETOPTN_ALIAS,  /* 10 */
        (void *) "-R",
        0,
        NULL },
    { "-f",     GETOPTN_STRPTR,             /* 11 */
        NULL  /*&diopts->formatString*/,
        0,
        NULL },
    { "--format-string",    GETOPTN_ALIAS,
        (void *) "-f",
        0,
        NULL },
    { "-F",     GETOPTN_ALIAS,
        (void *) "-I",
        0,
        NULL },
    { "-g",     GETOPTN_STRING,
        NULL  /*dbsstr*/,
        0  /*dbsstr_sz*/,
        (void *) "g" },
    { "-h",     GETOPTN_STRING,            /* 15 */
        NULL  /*dbsstr*/,
        0  /*dbsstr_sz*/,
        (void *) "h" },
    { "-H",     GETOPTN_STRING,             /* 16 */
        NULL  /*dbsstr*/,
        0  /*dbsstr_sz*/,
        (void *) "H" },
    { "--help", GETOPTN_FUNC_BOOL,
        NULL,
        0,
        NULL  /*processOptions*/ },
    { "--human-readable",   GETOPTN_ALIAS,
        (void *) "-H",
        0,
        NULL },
    { "-?",     GETOPTN_FUNC_BOOL,
        NULL,
        0,
        NULL  /*processOptions*/ },
    { "-i",     GETOPTN_ALIAS,              /* 20 */
        (void *) "-x",
        0,
        NULL },
    { "-I",     GETOPTN_FUNC_VALUE,         /* 21 */
        NULL  /*&padata*/,
        0,
        NULL  /*processOptionsVal*/ },
    { "--inodes",GETOPTN_IGNORE,
        NULL,
        0,
        NULL },
    { "-k",     GETOPTN_STRING,
        NULL  /*dbsstr*/,
        0  /*dbsstr_sz*/,
        (void *) "k" },
    { "-l",     GETOPTN_BOOL,
        NULL  /*&diopts->localOnly*/,
        0  /*sizeof (diopts->localOnly)*/,
        NULL },
    { "--local",GETOPTN_ALIAS,              /* 25 */
        (void *) "-l",
        0,
        NULL },
    { "-L",     GETOPTN_BOOL,               /* 26 */
        NULL  /*&diopts->excludeLoopback*/,
        0  /*sizeof (diopts->excludeLoopback)*/,
        NULL },
    { "-m",     GETOPTN_STRING,
        NULL  /*dbsstr*/,
        0  /*dbsstr_sz*/,
        (void *) "m" },
    { "-n",     GETOPTN_BOOL,
        NULL  /*&diopts->printHeader*/,
        0  /*sizeof (diopts->printHeader)*/,
        NULL },
    { "--no-sync",  GETOPTN_IGNORE,
        NULL,
        0,
        NULL },
    { "-P",     GETOPTN_FUNC_BOOL,          /* 30 */
        NULL  /*&padata*/,
        0,
        NULL  /*processOptions*/ },
    { "--portability",  GETOPTN_ALIAS,      /* 31 */
        (void *) "-P",
        0,
        NULL },
    { "--print-type",   GETOPTN_IGNORE,
        NULL,
        0,
        NULL },
    { "-q",     GETOPTN_BOOL,
        NULL  /*&diopts->quota_check*/,
        0  /*sizeof (diopts->quota_check)*/,
        NULL },
    { "-R",     GETOPTN_BOOL,
        NULL  /*&diopts->dontResolveSymlink*/,
        0  /*sizeof (diopts->dontResolveSymlink)*/,
        NULL },
    { "-s",     GETOPTN_FUNC_VALUE,         /* 35 */
        NULL  /*&padata*/,
        0,
        NULL  /*processOptionsVal*/ },
    { "--si",   GETOPTN_FUNC_BOOL,          /* 36 */
        NULL  /*&padata*/,
        0,
        NULL  /*processOptions*/ },
    { "--sync", GETOPTN_IGNORE,             /* 37 */
        NULL,
        0,
        NULL },
    { "-t",     GETOPTN_BOOL,
        NULL  /*&diopts->printTotals*/,
        0  /*sizeof (diopts->printTotals)*/,
        NULL },
    { "--total",GETOPTN_ALIAS,
        (void *) "-t",
        0,
        NULL },
    { "--type", GETOPTN_ALIAS,              /* 40 */
        (void *) "-I",
        0,
        NULL },
    { "-v",     GETOPTN_IGNORE,             /* 41 */
        NULL,
        0,
        NULL },
    { "--version", GETOPTN_FUNC_BOOL,       /* 42 */
        NULL,
        0,
        NULL  /*processOptions*/ },
    { "-w",     GETOPTN_SIZET,
        NULL  /*&diout->width*/,
        0  /*sizeof (diout->width)*/,
        NULL },
    { "-W",     GETOPTN_SIZET,
        NULL  /*&diout->inodeWidth*/,
        0  /*sizeof (diout->inodeWidth)*/,
        NULL },
    { "-x",     GETOPTN_FUNC_VALUE,
        NULL  /*&padata*/,
        0,
        NULL  /*processOptionsVal*/ },
    { "--exclude-type",     GETOPTN_ALIAS,  /* 46 */
        (void *) "-x",
        0,
        NULL },
    { "-X",     GETOPTN_FUNC_VALUE,
        NULL  /*&padata*/,
        0,
        NULL  /*processOptionsVal*/ },
    { "-z",     GETOPTN_STRING,             /* 48 */
        NULL  /*&diData->zoneInfo.zoneDisplay*/,
        0  /*sizeof (diData->zoneInfo.zoneDisplay)*/,
        NULL },
    { "-Z",     GETOPTN_STRING,
        NULL  /*&diData->zoneInfo.zoneDisplay*/,
        0  /*sizeof (diData->zoneInfo.zoneDisplay)*/,
        (void *) "all" }
  };
  static int dbsids[] = { 8, 14, 15, 16, 23, 27 };
  static int paidb[] = { 1, 17, 19, 30, 36, 42, };
  static int paidv[] = { 3, 21, 35, 45, 47 };

  diopts = &diData->options;
  diout = &diData->output;

    /* this is seriously gross, but the old compilers don't have    */
    /* automatic aggregate initialization                           */
    /* don't forget to change dbsids, paidb and paidv above also    */
  opts[0].valptr = (void *) &diopts->formatString;   /* -A */
  opts[6].valptr = (void *) &diopts->csv_output;     /* -c */
  opts[6].valsiz = sizeof (diopts->csv_output);
  opts[11].valptr = (void *) &diopts->formatString;  /* -f */
  opts[24].valptr = (void *) &diopts->localOnly;     /* -l */
  opts[24].valsiz = sizeof (diopts->localOnly);
  opts[26].valptr = (void *) &diopts->excludeLoopback; /* -L */
  opts[26].valsiz = sizeof (diopts->excludeLoopback);
  opts[28].valptr = (void *) &diopts->printHeader;   /* -n */
  opts[28].valsiz = sizeof (diopts->printHeader);
  opts[33].valptr = (void *) &diopts->quota_check;    /* -q */
  opts[33].valsiz = sizeof (diopts->quota_check);
  opts[34].valptr = (void *) &diopts->dontResolveSymlink;    /* -R */
  opts[34].valsiz = sizeof (diopts->dontResolveSymlink);
  opts[38].valptr = (void *) &diopts->printTotals;    /* -t */
  opts[38].valsiz = sizeof (diopts->printTotals);
  opts[43].valptr = (void *) &diout->width;          /* -w */
  opts[43].valsiz = sizeof (diout->width);
  opts[44].valptr = (void *) &diout->inodeWidth;     /* -W */
  opts[44].valsiz = sizeof (diout->inodeWidth);
  opts[48].valptr = (void *) diData->zoneInfo.zoneDisplay;  /* -z */
  opts[48].valsiz = sizeof (diData->zoneInfo.zoneDisplay);
  opts[49].valptr = (void *) diData->zoneInfo.zoneDisplay;  /* -Z */
  opts[49].valsiz = sizeof (diData->zoneInfo.zoneDisplay);

  for (i = 0; i < (int) (sizeof (dbsids) / sizeof (int)); ++i) {
    opts[dbsids[i]].valptr = (void *) dbsstr;
    opts[dbsids[i]].valsiz = dbsstr_sz;
  }
  for (i = 0; i < (int) (sizeof (paidb) / sizeof (int)); ++i) {
    opts[paidb[i]].valptr = (void *) &padata;
    opts[paidb[i]].value2 = (void *) processOptions;
  }
  for (i = 0; i < (int) (sizeof (paidv) / sizeof (int)); ++i) {
    opts[paidv[i]].valptr = (void *) &padata;
    opts[paidv[i]].value2 = (void *) processOptionsVal;
  }

  padata.diData = diData;
  padata.diopts = diopts;
  padata.diout = diout;
  padata.dbsstr = dbsstr;
  padata.dbsstr_sz = dbsstr_sz;

  optidx = getoptn (GETOPTN_LEGACY, argc, argv,
       sizeof (opts) / sizeof (getoptn_opt_t), opts);

  if (diopts->csv_output) {
    diopts->printTotals = FALSE;
  }

  return optidx;
}

static void
#if _proto_stdc
processOptions (const char *arg, char *valptr)
#else
processOptions (arg, valptr)
    const char *arg;
    char *valptr;
#endif
{
  struct pa_tmp     *padata;

  padata = (struct pa_tmp *) valptr;
  if (strcmp (arg, "-a") == 0) {
    padata->diopts->displayAll = TRUE;
    strncpy (padata->diData->zoneInfo.zoneDisplay, "all", MAXPATHLEN);
  } else if (strcmp (arg, "--help") == 0 || strcmp (arg, "-?") == 0) {
    usage();
    exit (0);
  } else if (strcmp (arg, "-P") == 0) {
    if (strcmp (padata->dbsstr, "k") != 0) /* don't override -k option */
    {
      strncpy (padata->dbsstr, "512", padata->dbsstr_sz);
    }
    padata->diopts->formatString = DI_POSIX_FORMAT;
    padata->diopts->posix_compat = TRUE;
    padata->diopts->csv_output = FALSE;
  } else if (strcmp (arg, "--si") == 0) {
    padata->diopts->baseDispSize = DI_VAL_1000;
    padata->diopts->baseDispIdx = DI_DISP_1000_IDX;
    strncpy (padata->dbsstr, "H", padata->dbsstr_sz);
  } else if (strcmp (arg, "--version") == 0) {
    printf (DI_GT("di version %s    Default Format: %s\n"), DI_VERSION, DI_DEFAULT_FORMAT);
    exit (0);
  } else {
    fprintf (stderr, "di_panic: bad option setup\n");
  }

  return;
}

static void
#if _proto_stdc
processOptionsVal (const char *arg, char *valptr, char *value)
#else
processOptionsVal (arg, valptr, value)
    const char  *arg;
    char        *valptr;
    char        *value;
#endif
{
  struct pa_tmp     *padata;

  padata = (struct pa_tmp *) valptr;

  if (strcmp (arg, "-B") == 0) {
    if (isdigit ((int) (*value)))
    {
      padata->diopts->baseDispSize = atof (value);
      padata->diopts->baseDispIdx = DI_DISP_1000_IDX; /* unknown, really */
      if (padata->diopts->baseDispSize == DI_VAL_1024)
      {
        padata->diopts->baseDispIdx = DI_DISP_1024_IDX;
      }
    }
    else if (strcmp (value, "k") == 0)
    {
      padata->diopts->baseDispSize = DI_VAL_1024;
      padata->diopts->baseDispIdx = DI_DISP_1024_IDX;
    }
    else if (strcmp (value, "d") == 0 ||
        strcmp (value, "si") == 0)
    {
      padata->diopts->baseDispSize = DI_VAL_1000;
      padata->diopts->baseDispIdx = DI_DISP_1000_IDX;
    }
  } else if (strcmp (arg, "-I") == 0) {
    parseList (&padata->diData->includeList, value);
  } else if (strcmp (arg, "-s") == 0) {
    strncpy (padata->diopts->sortType, value,
        sizeof (padata->diopts->sortType));
      /* for backwards compatibility                       */
      /* reverse by itself - change to reverse mount point */
    if (strcmp (padata->diopts->sortType, "r") == 0)
    {
        strncpy (padata->diopts->sortType, "rm", DI_SORT_MAX);
    }
        /* add some sense to the sort order */
    if (strcmp (padata->diopts->sortType, "t") == 0)
    {
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

static void
#if _proto_stdc
parseList (iList_t *list, char *str)
#else
parseList (list, str)
    iList_t       *list;
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
    unsigned int len;

    dstr = strdup (str);
    if (dstr == (char *) NULL)
    {
        fprintf (stderr, "strdup failed in parseList() (1).  errno %d\n", errno);
        exit (1);
    }

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
    list->list = (char **) di_realloc ((char *) list->list,
            (Size_t) list->count * sizeof (char *));
    if (list->list == (char **) NULL)
    {
        fprintf (stderr, "malloc failed in parseList() (2).  errno %d\n", errno);
        exit (1);
    }

    ptr = dstr;
    for (i = ocount; i < ncount; ++i)
    {
        len = (unsigned int) strlen (ptr);
        lptr = (char *) malloc ((Size_t) len + 1);
        if (lptr == (char *) NULL)
        {
            fprintf (stderr, "malloc failed in parseList() (3).  errno %d\n",
                    errno);
            exit (1);
        }
        strncpy (lptr, ptr, (Size_t) len);
        lptr[len] = '\0';
        list->list [i] = lptr;
        ptr += len + 1;
    }

    free ((char *) dstr);
}


/*
 * usage
 */

static void
#if _proto_stdc
usage (void)
#else
usage ()
#endif
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
#if _proto_stdc
setDispBlockSize (char *ptr, diOptions_t *diopts, diOutput_t *diout)
#else
setDispBlockSize (ptr, diopts, diout)
    char            *ptr;
    diOptions_t     *diopts;
    diOutput_t      *diout;
#endif
{
  unsigned int    len;
  _print_size_t   val;
  char            *tptr;
  static char     tempbl [15];
  char            ttempbl [15];

  if (isdigit ((int) (*ptr)))
  {
      val = atof (ptr);
  }
  else
  {
      val = 1.0;
  }

  tptr = ptr;
  len = (unsigned int) strlen (ptr);
  if (! isdigit ((int) *tptr))
  {
    int             idx;

    idx = -1;
    switch (*tptr)
    {
      case 'k':
      case 'K':
      {
          idx = DI_ONE_K;
          break;
      }

      case 'm':
      case 'M':
      {
          idx = DI_ONE_MEG;
          break;
      }

      case 'g':
      case 'G':
      {
          idx = DI_ONE_GIG;
          break;
      }

      case 't':
      case 'T':
      {
          idx = DI_ONE_TERA;
          break;
      }

      case 'p':
      case 'P':
      {
          idx = DI_ONE_PETA;
          break;
      }

      case 'e':
      case 'E':
      {
          idx = DI_ONE_EXA;
          break;
      }

      case 'z':
      case 'Z':
      {
          idx = DI_ONE_ZETTA;
          break;
      }

      case 'y':
      case 'Y':
      {
          idx = DI_ONE_YOTTA;
          break;
      }

      case 'h':
      {
          val = DI_DISP_HR;
          diout->dispBlockLabel = "Size";
          break;
      }

      case 'H':
      {
          val = DI_DISP_HR_2;
          diout->dispBlockLabel = "Size";
          break;
      }

      default:
      {
          if (strncmp (ptr, "HUMAN", (Size_t) 5) == 0)
          {
              val = DI_DISP_HR;
          }
          else
          {
              /* some unknown string value */
            idx = DI_ONE_MEG;
          }
          break;
      }
    }

    if (idx >= 0)
    {
      if (len > 1)
      {
        ++tptr;
        if (*tptr == 'B')
        {
           diopts->baseDispSize = DI_VAL_1000;
           diopts->baseDispIdx = DI_DISP_1000_IDX;
        }
      }

      if (val == 1.0)
      {
        diout->dispBlockLabel = dispTable [idx].disp [diopts->baseDispIdx];
      }
      else
      {
        Snprintf1 (ttempbl, sizeof (tempbl), "%%.0%s %%s", DI_Lf);
        Snprintf2 (tempbl, sizeof (tempbl), ttempbl,
            val, DI_GT (dispTable [idx].disp [diopts->baseDispIdx]));
        diout->dispBlockLabel = tempbl;
      }
      val *= dispTable [idx].size;
    } /* known size multiplier */
  }
  else
  {
    int         i;
    int         ok;

    ok = 0;
    for (i = 0; i < (int) DI_DISPTAB_SIZE; ++i)
    {
      if (val == dispTable [i].size)
      {
        diout->dispBlockLabel = dispTable [i].disp [diopts->baseDispIdx];
        ok = 1;
        break;
      }
    }

    if (ok == 0)
    {
      Snprintf1 (ttempbl, sizeof (ttempbl), "%%.0%sb", DI_Lf);
      Snprintf1 (tempbl, sizeof (tempbl), ttempbl, val);
      diout->dispBlockLabel = tempbl;
    }
  }  /* some oddball block size */

  if (diopts->posix_compat && val == DI_VAL_512)
  {
    diout->dispBlockLabel = "512-blocks";
  }
  if (diopts->posix_compat && val == DI_VAL_1024)
  {
    diout->dispBlockLabel = "1024-blocks";
  }

  diopts->dispBlockSize = val;
}


static void
#if _proto_stdc
initDisplayTable (diOptions_t *diopts)
#else
initDisplayTable (diopts)
  diOptions_t *diopts;
#endif
{
  int       i;

      /* initialize dispTable array */
  dispTable [0].size = diopts->baseDispSize;
  for (i = 1; i < (int) DI_DISPTAB_SIZE; ++i)
  {
    dispTable [i].size = dispTable [i - 1].size *
        diopts->baseDispSize;
  }
}

