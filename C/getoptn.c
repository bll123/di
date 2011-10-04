/*
 * $Id$
 * $Source$
 * Copyright 2011 Brad Lanam, Walnut Creek, CA
 */
/*
 * A new version of getopt()
 *      Boolean short flags: -a -b  (a, b)
 *      Boolean short flags: -ab    (a, b)
 * or   Boolean long name:   -ab    (ab)
 *      With values:         -c 123 -d=abcdef (-c = 123, -d = abcdef)
 *      short flags:         -version  (-v = ersion)
 * or   long flags:          -version  (-version)
 *      Long options:        --a --b --c 123 --d=abcdef
 *
 */

#include "config.h"
#include "getoptn.h"

#if _hdr_stdio
# include <stdio.h>
#endif
#if _hdr_stdlib
# include <stdlib.h>
#endif
#if _hdr_string
# include <string.h>
#endif
#if _hdr_strings
# include <strings.h>
#endif
#if _sys_types
# include <sys/types.h>
#endif

typedef struct {
    Size_t      optionlen;
} getoptn_optinfo_t;

typedef struct {
    int                 style;
    int                 optidx;
    Size_t              optcount;
    getoptn_opt_t       *opts;
    getoptn_optinfo_t   *optinfo;
    int                 argc;
    const char          * const *argv;
    const char          *arg;       /* current arg we're processing         */
    Size_t              arglen;     /* and the length of it                 */
    int                 hasvalue;   /* does this arg have a value attached? */
    Size_t              argidx;     /* index to the value                   */
    Size_t              optionlen;  /* length of the option for this arg    */
    Size_t              reprocess;  /* -ab legacy form? must be 0 or 1      */
    Size_t              offset;     /* reprocessing offset                  */
} getoptn_info_t;

typedef void (*getoptn_func_bool_t) _((const char *option, void *valptr));
typedef void (*getoptn_func_value_t) _((const char *option, void *valptr, const char *value));

static int
#if _proto_stdc
find_option (getoptn_info_t *info, const char *arg, const char *oarg, Size_t *argidx)
#else
find_option (info, arg, oarg, argidx)
    getoptn_info_t  *info;
    const char      *arg;
    const char      *oarg;
    Size_t          *argidx;
#endif
{
  int       i;
  Size_t    junk;

  for (i = 0; i < (int) info->optcount; ++i) {
    if (info->optinfo[i].optionlen == 0) {
      info->optinfo[i].optionlen = strlen (info->opts[i].option);
    }
    if (info->reprocess &&
        ((info->opts[i].option_type != GETOPTN_BOOL) ||
         ((info->arglen - info->offset) >
          (info->optinfo[i].optionlen - info->reprocess)))) {
      continue;
    }
    if (strncmp (arg + info->offset, info->opts[i].option + info->reprocess,
             info->optinfo[i].optionlen - info->reprocess) == 0) {
      info->hasvalue = FALSE;
        /* info->argidx == 0 indicates top level of recursion */
      if (info->argidx == 0) {
        info->optionlen = info->optinfo[i].optionlen;
      }
      if (info->style == GETOPTN_LEGACY) {
        if (info->arglen - info->offset > info->optionlen - info->reprocess) {
          if (info->opts[i].option_type == GETOPTN_BOOL) {
            info->reprocess = TRUE;
            if (info->offset == 0) {
              ++info->offset;
            }
            ++info->offset;
            if (info->offset >= info->arglen) {
              info->offset = 0;
              info->reprocess = FALSE;
            }
          } else {
            info->hasvalue = TRUE;
          }
        } else {
          info->offset = 0;
          info->reprocess = FALSE;
        }
      }
      if (info->style == GETOPTN_MODERN) {
        if (info->arglen > info->optionlen) {
          if (info->arg[info->optionlen] == '=') {
            info->hasvalue = TRUE;
          } else {
            continue;  /* partial match */
          }
        }
      }
      *argidx = info->optinfo[i].optionlen;
      if (info->opts[i].option_type == GETOPTN_ALIAS) {
        return find_option (info, info->opts[i].valptr, oarg, &junk);
      }
      return i;
    }
  }

  info->reprocess = FALSE;
  info->offset = 0;
  return GETOPTN_NOTFOUND;
}

static const char *
#if _proto_stdc
getoption_value (getoptn_info_t *info, getoptn_opt_t *opt)
#else
getoption_value (info, opt)
    getoptn_info_t      info;
    getoptn_opt_t       *opt;
#endif
{
  const char    *ptr;

  ptr = (char *) NULL;
  if (opt->option_type != GETOPTN_FUNC_VALUE &&
      opt->value2 != (void *) NULL) {
    ptr = opt->value2;
  }

  if (info->hasvalue && info->arg[info->argidx] == '=') {
    ptr = &info->arg[info->argidx + 1];
  } else if (info->hasvalue) {
    ptr = &info->arg[info->argidx];
  } else if (info->optidx + 1 < info->argc) {
    ++info->optidx;
    ptr = info->argv[info->optidx];
  }

  return ptr;
}

static void
#if _proto_stdc
process_opt (getoptn_info_t *info, getoptn_opt_t *opt)
#else
process_opt (info, opt)
    getoptn_info_t  *info;
    getoptn_opt_t   *opt;
#endif
{
  const char    *ptr;

  if (opt->option_type == GETOPTN_INT ||
      opt->option_type == GETOPTN_LONG ||
      opt->option_type == GETOPTN_DOUBLE ||
      opt->option_type == GETOPTN_STRING ||
      opt->option_type == GETOPTN_STRPTR ||
      opt->option_type == GETOPTN_FUNC_VALUE) {
    ptr = getoption_value (info, opt);
    if (ptr == (char *) NULL) {
      fprintf (stderr, "%s: %s argument missing\n", info->argv[0], info->arg);
      return;
    }
  }

  if (opt->option_type == GETOPTN_BOOL) {
    int       *v;
    if (opt->valsiz != sizeof (int)) {
      fprintf (stderr, "%s: invalid size\n", info->argv[0]);
    }
    v = (int *) opt->valptr;
    *v = 1 - *v;  /* flip it */
  } else if (opt->option_type == GETOPTN_INT) {
    int       *v;
    if (opt->valsiz != sizeof (int)) {
      fprintf (stderr, "%s: invalid size\n", info->argv[0]);
    }
    v = (int *) opt->valptr;
    *v = atoi (ptr);
  } else if (opt->option_type == GETOPTN_LONG) {
    long      *v;
    if (opt->valsiz != sizeof (long)) {
      fprintf (stderr, "%s: invalid size\n", info->argv[0]);
    }
    v = (long *) opt->valptr;
    *v = atol (ptr);
  } else if (opt->option_type == GETOPTN_DOUBLE) {
    double     *v;
    if (opt->valsiz != sizeof (double)) {
      fprintf (stderr, "%s: invalid size\n", info->argv[0]);
    }
    v = (double *) opt->valptr;
    *v = atof (ptr);
  } else if (opt->option_type == GETOPTN_STRING) {
    char      *v;
    v = (char *) opt->valptr;
    strncpy (v, ptr, opt->valsiz);
  } else if (opt->option_type == GETOPTN_STRPTR) {
    const char **v;
    v = (const char **) opt->valptr;
    *v = ptr;
  } else if (opt->option_type == GETOPTN_FUNC_BOOL) {
    getoptn_func_bool_t f;
    f = (getoptn_func_bool_t) opt->value2;
    (f)(opt->option, opt->valptr);
  } else if (opt->option_type == GETOPTN_FUNC_VALUE) {
    getoptn_func_value_t f;
    f = (getoptn_func_value_t) opt->value2;
    (f)(opt->option, opt->valptr, ptr);
  } else {
    info->reprocess = FALSE;
    info->offset = 0;
    fprintf (stderr, "%s: unknown option type %d\n",
         info->argv[0], opt->option_type);
  }
}


int
#if _proto_stdc
getoptn (int style, int argc, const char * const argv [], Size_t optcount,
         getoptn_opt_t opts [])
#else
getoptn (style, argc, argv, optcount, opts)
    int         style;
    int         argc;
    const char * const argv [];
    Size_t      optcount;
    getoptn_opt_t opts [];
#endif
{
  int               i;
  const char        *arg;
  getoptn_opt_t     *opt;
  getoptn_info_t    info;

  info.style = style;
  info.argc = argc;
  info.argv = argv;
  info.optcount = optcount;
  info.opts = opts;
  info.optinfo = (getoptn_optinfo_t *) NULL;

  if (optcount > 0) {
    info.optinfo = (getoptn_optinfo_t *)
          malloc (sizeof (getoptn_optinfo_t) * optcount);
    for (i = 0; i < (int) info.optcount; ++i) {
      info.optinfo[i].optionlen = 0;
    }
  } else {
    return 1 < argc ? 1 : -1;
  }

  for (info.optidx = 1; info.optidx < argc; info.optidx++) {
    arg = argv[info.optidx];
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
    info.reprocess = FALSE;
    info.offset = 0;

    do {
      i = find_option (&info, arg, arg, &info.argidx);
      if (opts[i].option_type == GETOPTN_IGNORE) {
        continue;
      }
      if (i == GETOPTN_NOTFOUND) {
        if (info.reprocess == FALSE) {
          fprintf (stderr, "%s: unknown option %s\n", argv[0], arg);
        }
        continue;
      }

      opt = &opts[i];
      process_opt (&info, opt);
    } while (info.reprocess);
  }

  if (info.optinfo != (getoptn_optinfo_t *) NULL) {
    free (info.optinfo);
  }
  return argc;
}

#if TEST_GETOPTN

#include <assert.h>

static void
process_opts (const char *arg, char *valptr) {
  if (strcmp (arg, "-h") == 0) {
    double *v;
    v = (double *) valptr;
    *v = 9.9;
  }
  return;
}

static void
process_opts_val (const char *arg, char *valptr, char *value) {
  if (strcmp (arg, "-g") == 0) {
    double *v;
    v = (double *) valptr;
    *v = atof (value);
  }
  return;
}

int
main (int argc, const char * const argv[])
{
  char s[40];
  int  i;
  int  j;
  int  k;
  long l;
  double d;
  int   ac;
  const char *av[4];
  int  testno = 0;

  getoptn_opt_t opts[] = {
    { "-D",  GETOPTN_STRING,     &s, sizeof(s), "abc123" },
    { "-b",  GETOPTN_BOOL,       &i, sizeof(i), NULL },
    { "--b", GETOPTN_BOOL,       &i, sizeof(i), NULL },
    { "-c",  GETOPTN_BOOL,       &j, sizeof(j), NULL },
    { "--c", GETOPTN_ALIAS,      "-c", 0, NULL },
    { "-bc", GETOPTN_BOOL,       &k, sizeof(k), NULL },
    { "-d",  GETOPTN_DOUBLE,     &d, sizeof(d), NULL },
    { "--i", GETOPTN_INT,        &i, sizeof(i), NULL },
    { "-i",  GETOPTN_INT,        &i, sizeof(i), NULL },
    { "-i15",GETOPTN_INT,        &j, sizeof(j), NULL },
    { "-i17",GETOPTN_INT,        &j, sizeof(j), NULL },
    { "-l",  GETOPTN_LONG,       &l, sizeof(l), NULL },
    { "-s",  GETOPTN_STRING,     &s, sizeof(s), NULL },
  };

  /* test 1 */
  ++testno;
  *s = '\0';
  ac = 2;
  av[0] = "scriptname";
  av[1] = "-D";
  av[2] = NULL;
  getoptn (GETOPTN_LEGACY, ac, av,
       sizeof (opts) / sizeof (getoptn_opt_t), opts);
  if (strcmp (s, "abc123") != 0) {
    printf ("fail test %d\n", testno);
  }

  /* test 2 */
  ++testno;
  i = 0;
  ac = 2;
  av[0] = "scriptname";
  av[1] = "-b";
  av[2] = NULL;
  getoptn (GETOPTN_LEGACY, ac, av,
       sizeof (opts) / sizeof (getoptn_opt_t), opts);
  if (i != 1) {
    printf ("fail test %d\n", testno);
  }

  /* test 3 */
  ++testno;
  i = 0;
  ac = 2;
  av[0] = "scriptname";
  av[1] = "--b";
  av[2] = NULL;
  getoptn (GETOPTN_LEGACY, ac, av,
       sizeof (opts) / sizeof (getoptn_opt_t), opts);
  if (i != 1) {
    printf ("fail test %d\n", testno);
  }

  /* test 4 */
  ++testno;
  i = 0;
  ac = 2;
  av[0] = "scriptname";
  av[1] = "--i=13";
  av[2] = NULL;
  getoptn (GETOPTN_LEGACY, ac, av,
       sizeof (opts) / sizeof (getoptn_opt_t), opts);
  if (i != 13) {
    printf ("fail test %d\n", testno);
  }

  /* test 5 */
  ++testno;
  i = 0;
  ac = 3;
  av[0] = "scriptname";
  av[1] = "--i";
  av[2] = "14";
  av[3] = NULL;
  getoptn (GETOPTN_LEGACY, ac, av,
       sizeof (opts) / sizeof (getoptn_opt_t), opts);
  if (i != 14) {
    printf ("fail test %d\n", testno);
  }

  /* test 6 */
  ++testno;
  i = 0;
  j = 0;
  ac = 2;
  av[0] = "scriptname";
  av[1] = "-i15";
  av[2] = NULL;
  getoptn (GETOPTN_LEGACY, ac, av,
       sizeof (opts) / sizeof (getoptn_opt_t), opts);
  if (i != 15 || j != 0) {
    printf ("fail test %d\n", testno);
  }

  /* test 7 */
  ++testno;
  i = 0;
  ac = 3;
  av[0] = "scriptname";
  av[1] = "-i";
  av[2] = "16";
  av[3] = NULL;
  getoptn (GETOPTN_LEGACY, ac, av,
       sizeof (opts) / sizeof (getoptn_opt_t), opts);
  if (i != 16) {
    printf ("fail test %d\n", testno);
  }

  /* test 8 */
  ++testno;
  i = 0;
  j = 0;
  ac = 3;
  av[0] = "scriptname";
  av[1] = "-i17";
  av[2] = "5";
  av[3] = NULL;
  getoptn (GETOPTN_MODERN, ac, av,
       sizeof (opts) / sizeof (getoptn_opt_t), opts);
  if (i != 0 || j != 5) {
    printf ("fail test %d\n", testno);
  }

  /* test 9 */
  ++testno;
  i = 0;
  ac = 2;
  av[0] = "scriptname";
  av[1] = "-i=17";
  av[2] = NULL;
  getoptn (GETOPTN_MODERN, ac, av,
       sizeof (opts) / sizeof (getoptn_opt_t), opts);
  if (i != 17) {
    printf ("fail test %d\n", testno);
  }

  /* test 10 */
  ++testno;
  i = 0;
  ac = 2;
  av[0] = "scriptname";
  av[1] = "-i7";
  av[2] = NULL;
  getoptn (GETOPTN_LEGACY, ac, av,
       sizeof (opts) / sizeof (getoptn_opt_t), opts);
  if (i != 7) {
    printf ("fail test %d\n", testno);
  }

  /* test 11 */
  ++testno;
  l = 0;
  ac = 3;
  av[0] = "scriptname";
  av[1] = "-l";
  av[2] = "19";
  av[3] = NULL;
  getoptn (GETOPTN_LEGACY, ac, av,
       sizeof (opts) / sizeof (getoptn_opt_t), opts);
  if (l != 19) {
    printf ("fail test %d\n", testno);
  }

  /* test 12 */
  ++testno;
  i = 0;
  j = 0;
  ac = 3;
  av[0] = "scriptname";
  av[1] = "-b";
  av[2] = "-c";
  av[3] = NULL;
  getoptn (GETOPTN_LEGACY, ac, av,
       sizeof (opts) / sizeof (getoptn_opt_t), opts);
  if (i != 1 || j != 1) {
    printf ("fail test %d\n", testno);
  }

  /* test 13 */
  ++testno;
  i = 0;
  j = 0;
  k = 0;
  ac = 2;
  av[0] = "scriptname";
  av[1] = "-bc";
  av[2] = NULL;
  getoptn (GETOPTN_LEGACY, ac, av,
       sizeof (opts) / sizeof (getoptn_opt_t), opts);
  if (i != 1 || j != 1 || k != 0) {
    printf ("fail test %d\n", testno);
  }

  /* test 14 */
  ++testno;
  i = 0;
  j = 0;
  k = 0;
  ac = 2;
  av[0] = "scriptname";
  av[1] = "-bc";
  av[2] = NULL;
  getoptn (GETOPTN_MODERN, ac, av,
       sizeof (opts) / sizeof (getoptn_opt_t), opts);
  if (i != 0 || j != 0 || k != 1) {
    printf ("fail test %d\n", testno);
  }

  /* test 15 */
  ++testno;
  *s = '\0';
  ac = 2;
  av[0] = "scriptname";
  av[1] = "-s=abc";
  av[2] = NULL;
  getoptn (GETOPTN_MODERN, ac, av,
       sizeof (opts) / sizeof (getoptn_opt_t), opts);
  if (strcmp (s, "abc") != 0) {
    printf ("fail test %d\n", testno);
  }

  /* test 16 */
  ++testno;
  d = 0.0;
  ac = 2;
  av[0] = "scriptname";
  av[1] = "-d=1.2";
  av[2] = NULL;
  getoptn (GETOPTN_MODERN, ac, av,
       sizeof (opts) / sizeof (getoptn_opt_t), opts);
  if (d != 1.2) {
    printf ("fail test %d\n", testno);
  }

  return 0;
}

#endif /* TEST_GETOPTN */
