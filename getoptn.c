/*
 * Copyright 2011-2026 Brad Lanam, Walnut Creek, CA
 * Copyright 2023-2026 Brad Lanam, Pleasant Hill, CA
 */

/*
 * A new version of getopt ()
 *      Boolean short flags: -a -b  (a, b)
 *      With values:         -c 123 -d=abcdef -ef
 *                              (-c = 123, -d = abcdef, -e = f)
 *      Long options:        --a --b --c 123 --d=abcdef
 * LEGACY:
 *      Boolean short flags: -ab    (a, b)
 *      short flags:         -version  (-v = ersion)
 * MODERN:
 *      Boolean long name:   -ab    (ab)
 *      long flags:          -version  (-version)
 *
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
#if _hdr_string
# include <string.h>
#endif
#if _hdr_strings
# include <strings.h>
#endif
#if _sys_types \
    && ! defined (DI_INC_SYS_TYPES_H) /* xenix */
# define DI_INC_SYS_TYPES_H
# include <sys/types.h>
#endif

#if defined (TEST_GETOPTN)
# include <math.h>
#endif

#include "disystem.h"
#include "distrutils.h"
#include "getoptn.h"

typedef struct {
  Size_t      optionlen;
  int         idx;
} getoptn_optinfo_t;

typedef struct {
  getoptn_opt_t       *opts;
  getoptn_optinfo_t   *optinfo;
  const char          **argv;
  const char          *arg;       /* current arg we're processing         */
  Size_t              arglen;     /* and the length of it                 */
  Size_t              argidx;     /* index to the value                   */
  Size_t              optionlen;  /* length of the option for this arg    */
  Size_t              reprocess;  /* -ab legacy form? must be 0 or 1      */
  Size_t              offset;     /* reprocessing offset                  */
  int                 style;
  int                 optidx;
  int                 optcount;
  int                 argc;
  int                 hasvalue;   /* does this arg have a value attached? */
} getoptn_info_t;

typedef void (*getoptn_func_bool_t) (const char *option, void * valptr);
typedef void (*getoptn_func_value_t) (const char *option, void * valptr, const char *value);

static int
find_option (getoptn_info_t *info, const char *arg, const char *oarg, Size_t *argidx)
{
  int       i;
  Size_t    junk;

  for (i = 0; i < info->optcount; ++i) {
    if (strncmp (arg + info->offset, info->opts [i].option + info->reprocess,
             info->optinfo [i].optionlen - info->reprocess) == 0) {
      info->hasvalue = false;
        /* info->argidx == 0 indicates top level of recursion */
      if (info->argidx == 0) {
        info->optionlen = info->optinfo [i].optionlen;
      }

      if (info->style == GETOPTN_LEGACY) {
        if (info->arglen - info->offset > info->optionlen - info->reprocess) {
          if (info->opts [i].option_type == GETOPTN_BOOL) {
            info->reprocess = true;
            if (info->offset == 0) {
              ++info->offset;
            }
            ++info->offset;
            if (info->offset >= info->arglen) {
              info->offset = 0;
              info->reprocess = false;
            }
          } else {
            info->hasvalue = true;
          }
        } else {
          info->offset = 0;
          info->reprocess = false;
        }
      }

      if (info->style == GETOPTN_MODERN) {
        if (info->arglen > info->optionlen) {
          if (info->arg [info->optionlen] == '=') {
            info->hasvalue = true;
          } else {
            continue;  /* partial match */
          }
        }
      }

      *argidx = info->optinfo [i].optionlen;
      if (info->opts [i].option_type == GETOPTN_ALIAS) {
        return find_option (info, (const char *) info->opts [i].valptr,
            oarg, &junk);
      }
      return i;
    }
  }

  info->reprocess = false;
  info->offset = 0;
  return GETOPTN_NOTFOUND;
}

static const char *
getoption_value (getoptn_info_t *info, getoptn_opt_t *opt)
{
  const char    *ptr;

  ptr = (char *) NULL;
  if (opt->option_type != GETOPTN_FUNC_VALUE &&
      opt->value2 != (void * ) NULL) {
    ptr = (const char *) opt->value2;
    return ptr;
  }

  if (info->hasvalue && info->arg [info->argidx] == '=') {
    ptr = &info->arg [info->argidx + 1];
  } else if (info->hasvalue) {
    ptr = &info->arg [info->argidx];
  } else if (info->optidx + 1 < info->argc) {
    ++info->optidx;
    ptr = info->argv [info->optidx];
  }

  return ptr;
}

static int
dooptchecks (getoptn_info_t *info, getoptn_opt_t *opt,
     getoptn_optinfo_t *optinfo, const char *tag, Size_t sz)
{
  if (sz != 0 && opt->valsiz != sz) {
    fprintf (stderr, "%s: %s: invalid size (line %d)\n", info->argv [0], tag, optinfo->idx);
    return 1;
  }
  if (opt->valptr == NULL) {
    fprintf (stderr, "%s: %s: invalid pointer (line %d)\n", info->argv [0], tag, optinfo->idx);
    return 1;
  }

  return 0;
}

static int
process_opt (getoptn_info_t *info, getoptn_opt_t *opt, getoptn_optinfo_t *optinfo)
{
  const char    *ptr;

  ptr = (char *) NULL;
  if (opt->option_type == GETOPTN_INT ||
      opt->option_type == GETOPTN_LONG ||
      opt->option_type == GETOPTN_SIZET ||
      opt->option_type == GETOPTN_DOUBLE ||
      opt->option_type == GETOPTN_STRING ||
      opt->option_type == GETOPTN_STRPTR ||
      opt->option_type == GETOPTN_FUNC_VALUE) {
    ptr = getoption_value (info, opt);
    if (ptr == (char *) NULL) {
      fprintf (stderr, "%s: %s argument missing\n", info->argv [0], info->arg);
      return 1;
    }
  }

  if (opt->option_type == GETOPTN_BOOL) {
    int       *v;
    if (dooptchecks (info, opt, optinfo, "bool", sizeof (int)) != 0) {
      return 1;
    }
    v = (int *) opt->valptr;
    *v = 1 - *v;  /* flip it */
  } else if (opt->option_type == GETOPTN_INT) {
    int       *v;
    if (dooptchecks (info, opt, optinfo, "int", sizeof (int)) != 0) {
      return 1;
    }
    v = (int *) opt->valptr;
    *v = atoi (ptr);
  } else if (opt->option_type == GETOPTN_LONG) {
    long      *v;
    if (dooptchecks (info, opt, optinfo, "long", sizeof (long)) != 0) {
      return 1;
    }
    v = (long *) opt->valptr;
    *v = atol (ptr);
  } else if (opt->option_type == GETOPTN_SIZET) {
    Size_t  *v;
    if (dooptchecks (info, opt, optinfo, "Size_t", sizeof (Size_t)) != 0) {
      return 1;
    }
    v = (Size_t *) opt->valptr;
    *v = (Size_t) atol (ptr);
  } else if (opt->option_type == GETOPTN_DOUBLE) {
    double     *v;
    if (dooptchecks (info, opt, optinfo, "double", sizeof (double)) != 0) {
      return 1;
    }
    v = (double *) opt->valptr;
    *v = atof (ptr);
  } else if (opt->option_type == GETOPTN_STRING) {
    char      *v;
    if (dooptchecks (info, opt, optinfo, "string", 0) != 0) {
      return 1;
    }
    v = (char *) opt->valptr;
    stpecpy (v, v + opt->valsiz, ptr);
  } else if (opt->option_type == GETOPTN_STRPTR) {
    const char **v;
    if (dooptchecks (info, opt, optinfo, "strptr", 0) != 0) {
      return 1;
    }
    v = (const char **) opt->valptr;
    if (v != NULL && ptr != NULL) {
      *v = ptr;
    }
  } else if (opt->option_type == GETOPTN_FUNC_BOOL) {
    getoptn_func_bool_t f;
    if (opt->funcptr == (genfuncptr_t) NULL) {
      fprintf (stderr, "%s: %s: invalid function ptr (line %d)\n", info->argv [0], "func_bool", optinfo->idx);
      return 1;
    }
    f = (getoptn_func_bool_t) opt->funcptr;
    (f) (opt->option, opt->valptr);
  } else if (opt->option_type == GETOPTN_FUNC_VALUE) {
    getoptn_func_value_t f;
    if (opt->funcptr == (genfuncptr_t) NULL) {
      fprintf (stderr, "%s: %s: invalid function ptr (line %d)\n", info->argv [0], "func_val", optinfo->idx);
      return 1;
    }
    f = (getoptn_func_value_t) opt->funcptr;
    (f) (opt->option, opt->valptr, ptr);
  } else {
    info->reprocess = false;
    info->offset = 0;
    fprintf (stderr, "%s: unknown option type %d\n",
         info->argv [0], opt->option_type);
    return 1;
  }

  return 0;
}


int
getoptn (int style, int argc, const char * argv [],
         int optcount, getoptn_opt_t opts [], int offset, int *errorCount)
{
  int               i;
  int               rc;
  const char        *arg;
  getoptn_opt_t     *opt;
  getoptn_info_t    info;
  int               ignorenext;

  info.style = style;
  info.argc = argc;
  info.argv = argv;
  info.optcount = optcount;
  info.opts = opts;
  info.optinfo = (getoptn_optinfo_t *) NULL;

  *errorCount = 0;

  if (optcount > 0) {
    info.optinfo = (getoptn_optinfo_t *)
          malloc (sizeof (getoptn_optinfo_t) * (Size_t) optcount);
    for (i = 0; i < info.optcount; ++i) {
      if (info.opts [i].option == NULL) {
        fprintf (stderr, "fatal error: null option\n");
        return -1;
      }
      info.optinfo [i].optionlen = strlen (info.opts [i].option);
      info.optinfo [i].idx = i;
    }
  } else {
    return 1 < argc ? 1 : -1;
  }

  for (info.optidx = offset; info.optidx < argc; info.optidx++) {
    arg = argv [info.optidx];
    if (*arg != '-') {
      if (info.optinfo != (getoptn_optinfo_t *) NULL) {
        free (info.optinfo);
      }
      return info.optidx;
    }
    if (strcmp (arg, "--") == 0) {
      info.optidx++;
      if (info.optinfo != (getoptn_optinfo_t *) NULL) {
        free (info.optinfo);
      }
      return info.optidx;
    }

    info.argidx = 0;
    info.arg = arg;
    info.arglen = strlen (arg);
    info.reprocess = false;
    info.offset = 0;

    ignorenext = false;
    do {
      if (ignorenext) {
        ignorenext = false;
        continue;
      }
      i = find_option (&info, arg, arg, &info.argidx);
      if (i == GETOPTN_NOTFOUND) {
        if (info.reprocess == false) {
          fprintf (stderr, "%s: unknown option %s\n", argv [0], arg);
          ++*errorCount;
        }
        continue;
      }

      if (opts [i].option_type == GETOPTN_IGNORE) {
        continue;
      }
      if (opts [i].option_type == GETOPTN_IGNORE_ARG) {
        ignorenext = true;
        continue;
      }

      opt = &opts [i];
      rc = process_opt (&info, opt, &info.optinfo [i]);
      if (rc) {
        ++*errorCount;
      }
    } while (info.reprocess);
  }

  if (info.optinfo != (getoptn_optinfo_t *) NULL) {
    free (info.optinfo);
  }
  return argc;
}

#if defined (TEST_GETOPTN)

int
main (int argc, char * argv [])
{
  char        tmp [40];
  char        s [40];
  char        s2 [5];
  char        *sp = NULL;
  long        l;
  double      d;
  int         i;
  int         j;
  int         k;
  int         ec;
  int         optidx;
  int         ac;
  const char  *av [20];

  int  grc = 0;
  int  testno = 0;

  getoptn_opt_t opts [] = {
    { "-D",     &s, (void * ) "abc123",   NULL, sizeof (s),   GETOPTN_STRING },
    { "-b",     &i, NULL,                 NULL, sizeof (i),   GETOPTN_BOOL },
    { "--b",    &i, NULL,                 NULL, sizeof (i),   GETOPTN_BOOL },
    { "-c",     &j, NULL,                 NULL, sizeof (j),   GETOPTN_BOOL },
    { "--c",    (void * ) "-c", NULL,     NULL, 0,            GETOPTN_ALIAS },
    { "-bc",    &k, NULL,                 NULL, sizeof (k),   GETOPTN_BOOL },
    { "-d",     &d, NULL,                 NULL, sizeof (d),   GETOPTN_DOUBLE },
    { "-f1",    &i, NULL,                 NULL, 8,            GETOPTN_INT },
    { "-f2",    &i, NULL,                 NULL, 2,            GETOPTN_LONG },
    { "-f3",    &l, NULL,                 NULL, 12,           GETOPTN_LONG },
    { "--i",    &i, NULL,                 NULL, sizeof (i),   GETOPTN_INT },
    { "-i",     &i, NULL,                 NULL, sizeof (i),   GETOPTN_INT },
    { "-i15",   &j, NULL,                 NULL, sizeof (j),   GETOPTN_INT },
    { "-i17",   &j, NULL,                 NULL, sizeof (j),   GETOPTN_INT },
    { "-l",     &l, NULL,                 NULL, sizeof (l),   GETOPTN_LONG },
    { "-s",     &s, NULL,                 NULL, sizeof (s),   GETOPTN_STRING },
    { "-sabcd", &i, NULL,                 NULL, sizeof (i),   GETOPTN_BOOL },
    { "-sp",    &sp, NULL,                NULL, 0,            GETOPTN_STRPTR },
    { "-p",     &sp, NULL,                NULL, 0,            GETOPTN_STRPTR },
    { "-S",     &sp, (void * ) "abc1234", NULL, 0,            GETOPTN_STRPTR },
    { "-s2",    &s2, NULL,                NULL, sizeof (s2),  GETOPTN_STRING },
    { "-np1",   NULL, NULL,               NULL, sizeof (s2),  GETOPTN_STRING },
    { "-np2",   NULL, NULL,               NULL, sizeof (s2),  GETOPTN_FUNC_BOOL },
    { "-np3",   NULL, NULL,               NULL, sizeof (s2),  GETOPTN_FUNC_VALUE },
    { "-w",     NULL, NULL,               NULL, 0,            GETOPTN_IGNORE },
    { "-W",     NULL, NULL,               NULL, 0,            GETOPTN_IGNORE_ARG },
    { "-z1",    (void * ) "--c", NULL,    NULL, 0,            GETOPTN_ALIAS },
    { "-z2",    (void * ) "-z1", NULL,    NULL, 0,            GETOPTN_ALIAS },
    { "-z3",    (void * ) "-z2", NULL,    NULL, 0,            GETOPTN_ALIAS }
  };

  /* test 1 */
  ++testno;
  memset (s, '\0', sizeof (s));
  ac = 2;
  Snprintf1 (tmp, sizeof (tmp), "test: %d", testno);
  av [0] = tmp;
  av [1] = "-D";
  av [2] = NULL;
  ec = 0;
  optidx = getoptn (GETOPTN_LEGACY, ac, av,
       sizeof (opts) / sizeof (getoptn_opt_t), opts, 1, &ec);
  if (strcmp (s, "abc123") != 0 || optidx != 2 || ec != 0) {
    fprintf (stderr, "fail test %d\n", testno);
    grc = 1;
  }

  /* test 2 */
  ++testno;
  i = 0;
  ac = 2;
  Snprintf1 (tmp, sizeof (tmp), "test %d", testno);
  av [0] = tmp;
  av [1] = "-b";
  av [2] = NULL;
  ec = 0;
  optidx = getoptn (GETOPTN_LEGACY, ac, av,
       sizeof (opts) / sizeof (getoptn_opt_t), opts, 1, &ec);
  if (i != 1 || optidx != 2 || ec != 0) {
    fprintf (stderr, "fail test %d\n", testno);
    grc = 1;
  }

  /* test 3 */
  ++testno;
  i = 0;
  ac = 2;
  Snprintf1 (tmp, sizeof (tmp), "test %d", testno);
  av [0] = tmp;
  av [1] = "--b";
  av [2] = NULL;
  ec = 0;
  optidx = getoptn (GETOPTN_LEGACY, ac, av,
       sizeof (opts) / sizeof (getoptn_opt_t), opts, 1, &ec);
  if (i != 1 || optidx != 2 || ec != 0) {
    fprintf (stderr, "fail test %d\n", testno);
    grc = 1;
  }

  /* test 4 */
  ++testno;
  i = 0;
  ac = 2;
  Snprintf1 (tmp, sizeof (tmp), "test %d", testno);
  av [0] = tmp;
  av [1] = "--i=13";
  av [2] = NULL;
  ec = 0;
  optidx = getoptn (GETOPTN_LEGACY, ac, av,
       sizeof (opts) / sizeof (getoptn_opt_t), opts, 1, &ec);
  if (i != 13 || optidx != 2 || ec != 0) {
    fprintf (stderr, "fail test %d\n", testno);
    grc = 1;
  }

  /* test 5 */
  ++testno;
  i = 0;
  ac = 3;
  Snprintf1 (tmp, sizeof (tmp), "test %d", testno);
  av [0] = tmp;
  av [1] = "--i";
  av [2] = "14";
  av [3] = NULL;
  ec = 0;
  optidx = getoptn (GETOPTN_LEGACY, ac, av,
       sizeof (opts) / sizeof (getoptn_opt_t), opts, 1, &ec);
  if (i != 14 || optidx != 3 || ec != 0) {
    fprintf (stderr, "fail test %d\n", testno);
    grc = 1;
  }

  /* test 6 */
  ++testno;
  i = 0;
  j = 0;
  ac = 2;
  Snprintf1 (tmp, sizeof (tmp), "test %d", testno);
  av [0] = tmp;
  av [1] = "-i15";
  av [2] = NULL;
  ec = 0;
  optidx = getoptn (GETOPTN_LEGACY, ac, av,
       sizeof (opts) / sizeof (getoptn_opt_t), opts, 1, &ec);
  if (i != 15 || j != 0 || optidx != 2 || ec != 0) {
    fprintf (stderr, "fail test %d\n", testno);
    grc = 1;
  }

  /* test 7 */
  ++testno;
  i = 0;
  ac = 3;
  Snprintf1 (tmp, sizeof (tmp), "test %d", testno);
  av [0] = tmp;
  av [1] = "-i";
  av [2] = "16";
  av [3] = NULL;
  ec = 0;
  optidx = getoptn (GETOPTN_LEGACY, ac, av,
       sizeof (opts) / sizeof (getoptn_opt_t), opts, 1, &ec);
  if (i != 16 || optidx != 3 || ec != 0) {
    fprintf (stderr, "fail test %d\n", testno);
    grc = 1;
  }

  /* test 8 */
  ++testno;
  i = 0;
  j = 0;
  ac = 3;
  Snprintf1 (tmp, sizeof (tmp), "test %d", testno);
  av [0] = tmp;
  av [1] = "-i17";
  av [2] = "5";
  av [3] = NULL;
  ec = 0;
  optidx = getoptn (GETOPTN_MODERN, ac, av,
       sizeof (opts) / sizeof (getoptn_opt_t), opts, 1, &ec);
  if (i != 0 || j != 5 || optidx != 3 || ec != 0) {
    fprintf (stderr, "fail test %d\n", testno);
    grc = 1;
  }

  /* test 9 */
  ++testno;
  i = 0;
  ac = 2;
  Snprintf1 (tmp, sizeof (tmp), "test %d", testno);
  av [0] = tmp;
  av [1] = "-i=17";
  av [2] = NULL;
  ec = 0;
  optidx = getoptn (GETOPTN_MODERN, ac, av,
       sizeof (opts) / sizeof (getoptn_opt_t), opts, 1, &ec);
  if (i != 17 || optidx != 2 || ec != 0) {
    fprintf (stderr, "fail test %d\n", testno);
    grc = 1;
  }

  /* test 10 */
  ++testno;
  i = 0;
  ac = 2;
  Snprintf1 (tmp, sizeof (tmp), "test %d", testno);
  av [0] = tmp;
  av [1] = "-i7";
  av [2] = NULL;
  ec = 0;
  optidx = getoptn (GETOPTN_LEGACY, ac, av,
       sizeof (opts) / sizeof (getoptn_opt_t), opts, 1, &ec);
  if (i != 7 || optidx != 2 || ec != 0) {
    fprintf (stderr, "fail test %d\n", testno);
    grc = 1;
  }

  /* test 11 */
  ++testno;
  l = 0L;
  ac = 3;
  Snprintf1 (tmp, sizeof (tmp), "test %d", testno);
  av [0] = tmp;
  av [1] = "-l";
  av [2] = "19";
  av [3] = NULL;
  ec = 0;
  optidx = getoptn (GETOPTN_LEGACY, ac, av,
       sizeof (opts) / sizeof (getoptn_opt_t), opts, 1, &ec);
  if (l != 19 || optidx != 3 || ec != 0) {
    fprintf (stderr, "fail test %d\n", testno);
    grc = 1;
  }

  /* test 12 */
  ++testno;
  i = 0;
  j = 0;
  ac = 3;
  Snprintf1 (tmp, sizeof (tmp), "test %d", testno);
  av [0] = tmp;
  av [1] = "-b";
  av [2] = "-c";
  av [3] = NULL;
  ec = 0;
  optidx = getoptn (GETOPTN_LEGACY, ac, av,
       sizeof (opts) / sizeof (getoptn_opt_t), opts, 1, &ec);
  if (i != 1 || j != 1 || optidx != 3 || ec != 0) {
    fprintf (stderr, "fail test %d\n", testno);
    grc = 1;
  }

  /* test 13 */
  ++testno;
  i = 0;
  j = 0;
  k = 0;
  ac = 2;
  Snprintf1 (tmp, sizeof (tmp), "test %d", testno);
  av [0] = tmp;
  av [1] = "-bc";
  av [2] = NULL;
  ec = 0;
  optidx = getoptn (GETOPTN_LEGACY, ac, av,
       sizeof (opts) / sizeof (getoptn_opt_t), opts, 1, &ec);
  if (i != 1 || j != 1 || k != 0 || optidx != 2 || ec != 0) {
    fprintf (stderr, "fail test %d\n", testno);
    grc = 1;
  }

  /* test 14 */
  ++testno;
  i = 0;
  j = 0;
  k = 0;
  ac = 2;
  Snprintf1 (tmp, sizeof (tmp), "test %d", testno);
  av [0] = tmp;
  av [1] = "-bc";
  av [2] = NULL;
  ec = 0;
  optidx = getoptn (GETOPTN_MODERN, ac, av,
       sizeof (opts) / sizeof (getoptn_opt_t), opts, 1, &ec);
  if (i != 0 || j != 0 || k != 1 || optidx != 2 || ec != 0) {
    fprintf (stderr, "fail test %d\n", testno);
    grc = 1;
  }

  /* test 15 */
  ++testno;
  memset (s, '\0', sizeof (s));
  ac = 2;
  Snprintf1 (tmp, sizeof (tmp), "test %d", testno);
  av [0] = tmp;
  av [1] = "-s=abc";
  av [2] = NULL;
  ec = 0;
  optidx = getoptn (GETOPTN_MODERN, ac, av,
       sizeof (opts) / sizeof (getoptn_opt_t), opts, 1, &ec);
  if (strcmp (s, "abc") != 0 || optidx != 2 || ec != 0) {
    fprintf (stderr, "fail test %d\n", testno);
    grc = 1;
  }

  /* test 16 */
  ++testno;
  d = 0.0;
  ac = 2;
  Snprintf1 (tmp, sizeof (tmp), "test %d", testno);
  av [0] = tmp;
  av [1] = "-d=1.2";
  av [2] = NULL;
  ec = 0;
  optidx = getoptn (GETOPTN_MODERN, ac, av,
       sizeof (opts) / sizeof (getoptn_opt_t), opts, 1, &ec);
  if (fabs (d - 1.2) > 0.00001 || optidx != 2 || ec != 0) {
    fprintf (stderr, "fail test %d: %.2g %d\n", testno, d, optidx);
    grc = 1;
  }

  /* test 17 */
  ++testno;
  memset (s, '\0', sizeof (s));
  i = 0;
  ac = 2;
  Snprintf1 (tmp, sizeof (tmp), "test %d", testno);
  av [0] = tmp;
  av [1] = "-sabcd";
  av [2] = NULL;
  ec = 0;
  optidx = getoptn (GETOPTN_LEGACY, ac, av,
       sizeof (opts) / sizeof (getoptn_opt_t), opts, 1, &ec);
  if (strcmp (s, "abcd") != 0 || i != 0 || optidx != 2 || ec != 0) {
    fprintf (stderr, "fail test %d\n", testno);
    grc = 1;
  }

  /* test 18 */
  ++testno;
  memset (s, '\0', sizeof (s));
  ac = 2;
  Snprintf1 (tmp, sizeof (tmp), "test %d", testno);
  av [0] = tmp;
  av [1] = "-s=abcde";
  av [2] = NULL;
  ec = 0;
  optidx = getoptn (GETOPTN_MODERN, ac, av,
       sizeof (opts) / sizeof (getoptn_opt_t), opts, 1, &ec);
  if (strcmp (s, "abcde") != 0 || optidx != 2 || ec != 0) {
    fprintf (stderr, "fail test %d\n", testno);
    grc = 1;
  }

  /* test 19 */
  ++testno;
  memset (s, '\0', sizeof (s));
  ac = 3;
  Snprintf1 (tmp, sizeof (tmp), "test %d", testno);
  av [0] = tmp;
  av [1] = "-s";
  av [2] = "abcdef";
  av [3] = NULL;
  ec = 0;
  optidx = getoptn (GETOPTN_MODERN, ac, av,
       sizeof (opts) / sizeof (getoptn_opt_t), opts, 1, &ec);
  if (strcmp (s, "abcdef") != 0 || optidx != 3 || ec != 0) {
    fprintf (stderr, "fail test %d\n", testno);
    grc = 1;
  }

  /* test 20 */
  ++testno;
  sp = "";
  ac = 3;
  Snprintf1 (tmp, sizeof (tmp), "test %d", testno);
  av [0] = tmp;
  av [1] = "-sp";
  av [2] = "0123";
  av [3] = NULL;
  ec = 0;
  optidx = getoptn (GETOPTN_MODERN, ac, av,
       sizeof (opts) / sizeof (getoptn_opt_t), opts, 1, &ec);
  if (strcmp (sp, "0123") != 0 || optidx != 3 || ec != 0) {
    fprintf (stderr, "fail test %d\n", testno);
    grc = 1;
  }

  /* test 21 */
  ++testno;
  sp = "";
  ac = 2;
  Snprintf1 (tmp, sizeof (tmp), "test %d", testno);
  av [0] = tmp;
  av [1] = "-sp=01234";
  av [2] = NULL;
  ec = 0;
  optidx = getoptn (GETOPTN_MODERN, ac, av,
       sizeof (opts) / sizeof (getoptn_opt_t), opts, 1, &ec);
  if (strcmp (sp, "01234") != 0 || optidx != 2 || ec != 0) {
    fprintf (stderr, "fail test %d\n", testno);
    grc = 1;
  }

  /* test 22 */
  ++testno;
  sp = "";
  ac = 2;
  Snprintf1 (tmp, sizeof (tmp), "test %d", testno);
  av [0] = tmp;
  av [1] = "-p012345";
  av [2] = NULL;
  ec = 0;
  optidx = getoptn (GETOPTN_LEGACY, ac, av,
       sizeof (opts) / sizeof (getoptn_opt_t), opts, 1, &ec);
  if (strcmp (sp, "012345") != 0 || optidx != 2 || ec != 0) {
    fprintf (stderr, "fail test %d\n", testno);
    grc = 1;
  }

  /* test 23 */
  ++testno;
  sp = "";
  ac = 2;
  Snprintf1 (tmp, sizeof (tmp), "test %d", testno);
  av [0] = tmp;
  av [1] = "-S";
  av [2] = NULL;
  ec = 0;
  optidx = getoptn (GETOPTN_LEGACY, ac, av,
       sizeof (opts) / sizeof (getoptn_opt_t), opts, 1, &ec);
  if (strcmp (sp, "abc1234") != 0 || optidx != 2 || ec != 0) {
    fprintf (stderr, "fail test %d\n", testno);
    grc = 1;
  }

  /* test 24 */
  ++testno;
  sp = "";
  ac = 2;
  Snprintf1 (tmp, sizeof (tmp), "test %d", testno);
  av [0] = tmp;
  av [1] = "-p=0123456";
  av [2] = NULL;
  ec = 0;
  optidx = getoptn (GETOPTN_LEGACY, ac, av,
       sizeof (opts) / sizeof (getoptn_opt_t), opts, 1, &ec);
  if (strcmp (sp, "0123456") != 0 || optidx != 2 || ec != 0) {
    fprintf (stderr, "fail test %d\n", testno);
    grc = 1;
  }

  /* test 25 */
  ++testno;
  memset (s, '\0', sizeof (s));
  i = 0;
  ac = 2;
  Snprintf1 (tmp, sizeof (tmp), "test %d", testno);
  av [0] = tmp;
  av [1] = "-sabcd";
  av [2] = NULL;
  ec = 0;
  optidx = getoptn (GETOPTN_MODERN, ac, av,
       sizeof (opts) / sizeof (getoptn_opt_t), opts, 1, &ec);
  if (strcmp (s, "") != 0 || i != 1 || optidx != 2 || ec != 0) {
    fprintf (stderr, "fail test %d\n", testno);
    grc = 1;
  }

  /* test 26 */
  ++testno;
  i = 1;
  ac = 2;
  Snprintf1 (tmp, sizeof (tmp), "test %d", testno);
  av [0] = tmp;
  av [1] = "-b";
  av [2] = NULL;
  ec = 0;
  optidx = getoptn (GETOPTN_LEGACY, ac, av,
       sizeof (opts) / sizeof (getoptn_opt_t), opts, 1, &ec);
  if (i != 0 || optidx != 2 || ec != 0) {
    fprintf (stderr, "fail test %d\n", testno);
    grc = 1;
  }

  /* test 27 */
  ++testno;
  i = 1;
  ac = 2;
  Snprintf1 (tmp, sizeof (tmp), "test %d", testno);
  av [0] = tmp;
  av [1] = "--b";
  av [2] = NULL;
  ec = 0;
  optidx = getoptn (GETOPTN_LEGACY, ac, av,
       sizeof (opts) / sizeof (getoptn_opt_t), opts, 1, &ec);
  if (i != 0 || optidx != 2 || ec != 0) {
    fprintf (stderr, "fail test %d\n", testno);
    grc = 1;
  }

  /* test 28 */
  ++testno;
  i = 1;
  j = 1;
  ac = 3;
  Snprintf1 (tmp, sizeof (tmp), "test %d", testno);
  av [0] = tmp;
  av [1] = "-b";
  av [2] = "-c";
  av [3] = NULL;
  ec = 0;
  optidx = getoptn (GETOPTN_LEGACY, ac, av,
       sizeof (opts) / sizeof (getoptn_opt_t), opts, 1, &ec);
  if (i != 0 || j != 0 || optidx != 3 || ec != 0) {
    fprintf (stderr, "fail test %d\n", testno);
    grc = 1;
  }

  /* test 29 */
  ++testno;
  i = 1;
  j = 1;
  k = 1;
  ac = 2;
  Snprintf1 (tmp, sizeof (tmp), "test %d", testno);
  av [0] = tmp;
  av [1] = "-bc";
  av [2] = NULL;
  ec = 0;
  optidx = getoptn (GETOPTN_LEGACY, ac, av,
       sizeof (opts) / sizeof (getoptn_opt_t), opts, 1, &ec);
  if (i != 0 || j != 0 || k != 1 || optidx != 2 || ec != 0) {
    fprintf (stderr, "fail test %d / i:%d j:%d k:%d optidx:%d ec:%d\n", testno, i, j, k, optidx, ec);
    grc = 1;
  }

  /* test 30 */
  ++testno;
  i = 1;
  j = 1;
  k = 1;
  ac = 2;
  Snprintf1 (tmp, sizeof (tmp), "test %d", testno);
  av [0] = tmp;
  av [1] = "-bc";
  av [2] = NULL;
  ec = 0;
  optidx = getoptn (GETOPTN_MODERN, ac, av,
       sizeof (opts) / sizeof (getoptn_opt_t), opts, 1, &ec);
  if (i != 1 || j != 1 || k != 0 || optidx != 2 || ec != 0) {
    fprintf (stderr, "fail test %d / i:%d j:%d k:%d optidx:%d ec:%d\n", testno, i, j, k, optidx, ec);
    grc = 1;
  }

  /* test 31 - empty value  */
  ++testno;
  i = 0;
  j = 0;
  k = 0;
  ac = 2;
  Snprintf1 (tmp, sizeof (tmp), "test %d", testno);
  av [0] = tmp;
  av [1] = "-i=";
  av [2] = NULL;
  ec = 0;
  optidx = getoptn (GETOPTN_MODERN, ac, av,
       sizeof (opts) / sizeof (getoptn_opt_t), opts, 1, &ec);
  if (i != 0 || optidx != 2 || ec != 0) {
    fprintf (stderr, "fail test %d / i:%d optidx:%d ec:%d\n", testno, i, optidx, ec);
    grc = 1;
  }

  /* test 32 - no value; should print error */
  ++testno;
  i = 0;
  j = 0;
  k = 0;
  ac = 2;
  Snprintf1 (tmp, sizeof (tmp), "test %d", testno);
  av [0] = tmp;
  av [1] = "-i";
  av [2] = NULL;
  ec = 0;
  fprintf (stderr, "** expect argument missing\n");
  optidx = getoptn (GETOPTN_MODERN, ac, av,
       sizeof (opts) / sizeof (getoptn_opt_t), opts, 1, &ec);
  if (i != 0 || optidx != 2 || ec != 1) {
    fprintf (stderr, "fail test %d\n", testno);
    grc = 1;
  }

  /* test 33 - wrong size; should print error */
  ++testno;
  i = 0;
  j = 0;
  k = 0;
  l = 0L;
  ac = 2;
  Snprintf1 (tmp, sizeof (tmp), "test %d", testno);
  av [0] = tmp;
  av [1] = "-f1=7";
  av [2] = NULL;
  ec = 0;
  fprintf (stderr, "** expect invalid size\n");
  optidx = getoptn (GETOPTN_MODERN, ac, av,
       sizeof (opts) / sizeof (getoptn_opt_t), opts, 1, &ec);
  if (i != 0 || l != 0 || optidx != 2 || ec != 1) {
    fprintf (stderr, "fail test %d; %d %ld %d\n", testno, i, l, optidx);
    grc = 1;
  }

  /* test 34 - wrong size; should print error */
  ++testno;
  i = 0;
  j = 0;
  k = 0;
  l = 0L;
  ac = 2;
  Snprintf1 (tmp, sizeof (tmp), "test %d", testno);
  av [0] = tmp;
  av [1] = "-f2=7";
  av [2] = NULL;
  ec = 0;
  fprintf (stderr, "** expect invalid size\n");
  optidx = getoptn (GETOPTN_MODERN, ac, av,
       sizeof (opts) / sizeof (getoptn_opt_t), opts, 1, &ec);
  if (i != 0 || l != 0 || optidx != 2 || ec != 1) {
    fprintf (stderr, "fail test %d; %d %ld %d\n", testno, i, l, optidx);
    grc = 1;
  }

  /* test 35 - wrong size; should print error */
  ++testno;
  i = 0;
  j = 0;
  k = 0;
  l = 0L;
  ac = 2;
  Snprintf1 (tmp, sizeof (tmp), "test %d", testno);
  av [0] = tmp;
  av [1] = "-f3=7";
  av [2] = NULL;
  ec = 0;
  fprintf (stderr, "** expect invalid size\n");
  optidx = getoptn (GETOPTN_MODERN, ac, av,
       sizeof (opts) / sizeof (getoptn_opt_t), opts, 1, &ec);
  if (i != 0 || l != 0 || optidx != 2 || ec != 1) {
    fprintf (stderr, "fail test %d; %d %ld %d\n", testno, i, l, optidx);
    grc = 1;
  }

  /* test 36 - end of options */
  ++testno;
  i = 0;
  j = 0;
  k = 0;
  l = 0L;
  ac = 4;
  Snprintf1 (tmp, sizeof (tmp), "test %d", testno);
  av [0] = tmp;
  av [1] = "-i=7";
  av [2] = "--";
  av [3] = "abc";
  av [4] = NULL;
  ec = 0;
  optidx = getoptn (GETOPTN_MODERN, ac, av,
       sizeof (opts) / sizeof (getoptn_opt_t), opts, 1, &ec);
  if (i != 7 || optidx != 3 || ec != 0) {
    fprintf (stderr, "fail test %d\n", testno);
    grc = 1;
  }

  /* test 37 - no more options */
  ++testno;
  i = 0;
  j = 0;
  k = 0;
  l = 0L;
  ac = 3;
  Snprintf1 (tmp, sizeof (tmp), "test %d", testno);
  av [0] = tmp;
  av [1] = "-i=7";
  av [2] = "abc";
  av [3] = NULL;
  ec = 0;
  optidx = getoptn (GETOPTN_MODERN, ac, av,
       sizeof (opts) / sizeof (getoptn_opt_t), opts, 1, &ec);
  if (i != 7 || optidx != 2 || ec != 0) {
    fprintf (stderr, "fail test %d\n", testno);
    grc = 1;
  }

  /* test 38 - empty value followed by another option; returns the option */
  ++testno;
  i = 0;
  j = 0;
  k = 0;
  memset (s, '\0', sizeof (s));
  ac = 3;
  Snprintf1 (tmp, sizeof (tmp), "test %d", testno);
  av [0] = tmp;
  av [1] = "-s";
  av [2] = "-s";
  av [3] = NULL;
  ec = 0;
  optidx = getoptn (GETOPTN_MODERN, ac, av,
       sizeof (opts) / sizeof (getoptn_opt_t), opts, 1, &ec);
  if (strcmp (s, "-s") != 0 || optidx != 3 || ec != 0) {
    fprintf (stderr, "fail test %d\n", testno);
    grc = 1;
  }

  /* test 39 - unknown options */
  ++testno;
  i = 0;
  j = 0;
  k = 0;
  memset (s, '\0', sizeof (s));
  ac = 5;
  Snprintf1 (tmp, sizeof (tmp), "test %d", testno);
  av [0] = tmp;
  av [1] = "-b";
  av [2] = "-c";
  av [3] = "-s";
  av [4] = "abc";
  av [5] = NULL;
  ec = 0;
  optidx = getoptn (GETOPTN_LEGACY, ac, av,
       sizeof (opts) / sizeof (getoptn_opt_t), opts, 1, &ec);
  if (strcmp (s, "abc") != 0 || i != 1 || j != 1 || optidx != 5 || ec != 0) {
    fprintf (stderr, "fail test %d\n", testno);
    grc = 1;
  }

  /* test 40 - legacy: mixed boolean + arg */
  ++testno;
  i = 0;
  j = 0;
  k = 0;
  memset (s, '\0', sizeof (s));
  ac = 3;
  Snprintf1 (tmp, sizeof (tmp), "test %d", testno);
  av [0] = tmp;
  av [1] = "-bcs";
  av [2] = "abc";
  av [3] = NULL;
  ec = 0;
  optidx = getoptn (GETOPTN_LEGACY, ac, av,
       sizeof (opts) / sizeof (getoptn_opt_t), opts, 1, &ec);
  if (strcmp (s, "abc") != 0 || i != 1 || j != 1 || optidx != 3 || ec != 0) {
    fprintf (stderr, "fail test %d\n", testno);
    grc = 1;
  }

  /* test 41 */
  ++testno;
  memset (s2, '\0', sizeof (s2));
  ac = 3;
  Snprintf1 (tmp, sizeof (tmp), "test %d", testno);
  av [0] = tmp;
  av [1] = "-s2";
  av [2] = "abc";
  av [3] = NULL;
  ec = 0;
  optidx = getoptn (GETOPTN_MODERN, ac, av,
       sizeof (opts) / sizeof (getoptn_opt_t), opts, 1, &ec);
  if (strcmp (s2, "abc") != 0 || optidx != 3 || ec != 0) {
    fprintf (stderr, "fail test %d\n", testno);
    grc = 1;
  }

  /* test 42 - short string */
  ++testno;
  memset (s2, '\0', sizeof (s2));
  ac = 3;
  Snprintf1 (tmp, sizeof (tmp), "test %d", testno);
  av [0] = tmp;
  av [1] = "-s2";
  av [2] = "abcd";
  av [3] = NULL;
  ec = 0;
  optidx = getoptn (GETOPTN_MODERN, ac, av,
       sizeof (opts) / sizeof (getoptn_opt_t), opts, 1, &ec);
  if (strcmp (s2, "abcd") != 0 || optidx != 3 || ec != 0) {
    fprintf (stderr, "fail test %d\n", testno);
    grc = 1;
  }

  /* test 43 - short string */
  ++testno;
  memset (s2, '\0', sizeof (s2));
  ac = 3;
  Snprintf1 (tmp, sizeof (tmp), "test %d", testno);
  av [0] = tmp;
  av [1] = "-s2";
  av [2] = "abcde";
  av [3] = NULL;
  ec = 0;
  optidx = getoptn (GETOPTN_MODERN, ac, av,
       sizeof (opts) / sizeof (getoptn_opt_t), opts, 1, &ec);
  if (strcmp (s2, "abcd") != 0 || optidx != 3 || ec != 0) {
    fprintf (stderr, "fail test %d: %s\n", testno, s2);
    grc = 1;
  }

  /* test 44 - short string */
  ++testno;
  memset (s2, '\0', sizeof (s2));
  ac = 3;
  Snprintf1 (tmp, sizeof (tmp), "test %d", testno);
  av [0] = tmp;
  av [1] = "-s2";
  av [2] = "abcdef";
  av [3] = NULL;
  ec = 0;
  optidx = getoptn (GETOPTN_MODERN, ac, av,
       sizeof (opts) / sizeof (getoptn_opt_t), opts, 1, &ec);
  if (strcmp (s2, "abcd") != 0 || optidx != 3 || ec != 0) {
    fprintf (stderr, "fail test %d: %s\n", testno, s2);
    grc = 1;
  }

  /* test 45 - null ptr */
  ++testno;
  memset (s2, '\0', sizeof (s2));
  ac = 3;
  Snprintf1 (tmp, sizeof (tmp), "test %d", testno);
  av [0] = tmp;
  av [1] = "-np1";
  av [2] = "abcdef";
  av [3] = NULL;
  ec = 0;
  fprintf (stderr, "** expect invalid pointer\n");
  optidx = getoptn (GETOPTN_MODERN, ac, av,
       sizeof (opts) / sizeof (getoptn_opt_t), opts, 1, &ec);
  if (strcmp (s2, "") != 0 || ec != 1) {
    fprintf (stderr, "fail test %d: %s %d\n", testno, s2, optidx);
    grc = 1;
  }

  /* test 46 - null ptr */
  ++testno;
  memset (s2, '\0', sizeof (s2));
  ac = 3;
  Snprintf1 (tmp, sizeof (tmp), "test %d", testno);
  av [0] = tmp;
  av [1] = "-np2";
  av [2] = "abcdef";
  av [3] = NULL;
  ec = 0;
  fprintf (stderr, "** expect invalid function pointer\n");
  optidx = getoptn (GETOPTN_MODERN, ac, av,
       sizeof (opts) / sizeof (getoptn_opt_t), opts, 1, &ec);
  if (strcmp (s2, "") != 0 || ec != 1) {
    fprintf (stderr, "fail test %d: %s %d\n", testno, s2, optidx);
    grc = 1;
  }

  /* test 47 - null ptr */
  ++testno;
  memset (s2, '\0', sizeof (s2));
  ac = 3;
  Snprintf1 (tmp, sizeof (tmp), "test %d", testno);
  av [0] = tmp;
  av [1] = "-np3";
  av [2] = "abcdef";
  av [3] = NULL;
  ec = 0;
  fprintf (stderr, "** expect invalid function pointer\n");
  optidx = getoptn (GETOPTN_MODERN, ac, av,
       sizeof (opts) / sizeof (getoptn_opt_t), opts, 1, &ec);
  if (strcmp (s2, "") != 0 || ec != 1) {
    fprintf (stderr, "fail test %d: %s %d\n", testno, s2, optidx);
    grc = 1;
  }

  /* test 48 - alias chain */
  ++testno;
  i = 0;
  j = 0;
  k = 0;
  memset (s2, '\0', sizeof (s2));
  ac = 2;
  Snprintf1 (tmp, sizeof (tmp), "test %d", testno);
  av [0] = tmp;
  av [1] = "-z3";
  av [2] = NULL;
  ec = 0;
  optidx = getoptn (GETOPTN_LEGACY, ac, av,
       sizeof (opts) / sizeof (getoptn_opt_t), opts, 1, &ec);
  if (i != 0 || j != 1 || ec != 0) {
    fprintf (stderr, "fail test %d\n", testno);
    grc = 1;
  }

  /* test 49 - test boolean initial value */
  ++testno;
  i = 0;
  ac = 2;
  Snprintf1 (tmp, sizeof (tmp), "test %d", testno);
  av [0] = tmp;
  av [1] = "-c";
  av [2] = NULL;
  ec = 0;
  optidx = getoptn (GETOPTN_LEGACY, ac, av,
       sizeof (opts) / sizeof (getoptn_opt_t), opts, 1, &ec);
  if (i != 0 || ec != 0) {
    fprintf (stderr, "fail test %d\n", testno);
    grc = 1;
  }

  /* test 50 - ignore */
  ++testno;
  i = 0;
  ac = 2;
  Snprintf1 (tmp, sizeof (tmp), "test %d", testno);
  av [0] = tmp;
  av [1] = "-w";
  av [2] = NULL;
  ec = 0;
  optidx = getoptn (GETOPTN_LEGACY, ac, av,
       sizeof (opts) / sizeof (getoptn_opt_t), opts, 1, &ec);
  if (i != 0 || ec != 0) {
    fprintf (stderr, "fail test %d\n", testno);
    grc = 1;
  }

  /* test 51 - ignore args */
  ++testno;
  i = 0;
  ac = 4;
  Snprintf1 (tmp, sizeof (tmp), "test %d", testno);
  av [0] = tmp;
  av [1] = "-w";
  av [2] = "20";
  av [3] = "-c";
  av [4] = NULL;
  ec = 0;
  optidx = getoptn (GETOPTN_LEGACY, ac, av,
       sizeof (opts) / sizeof (getoptn_opt_t), opts, 1, &ec);
  if (i != 0 || ec != 0) {
    fprintf (stderr, "fail test %d\n", testno);
    grc = 1;
  }

  /* test 52 - unknown argument */
  ++testno;
  i = 0;
  ac = 2;
  Snprintf1 (tmp, sizeof (tmp), "test %d", testno);
  av [0] = tmp;
  av [1] = "-G";
  av [2] = NULL;
  ec = 0;
  fprintf (stderr, "** expect unknown option\n");
  optidx = getoptn (GETOPTN_LEGACY, ac, av,
       sizeof (opts) / sizeof (getoptn_opt_t), opts, 1, &ec);
  if (ec != 1) {
    fprintf (stderr, "fail test %d\n", testno);
    grc = 1;
  }

  return grc;
}

#endif /* TEST_GETOPTN */
