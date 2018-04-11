/*
 * $Id$
 * $Source$
 * Copyright 2011-2013 Brad Lanam, Walnut Creek, CA
 */

#ifndef DI_INC_GETOPTN_H_
#define DI_INC_GETOPTN_H_

#if defined(TEST_GETOPTN)
# include "gconfig.h"
#else
# include "config.h"
#endif

#if _sys_types \
    && ! defined (DI_INC_SYS_TYPES_H) /* xenix */
# define DI_INC_SYS_TYPES_H
# include <sys/types.h>
#endif

#if ! defined (TRUE)
# define TRUE             1
#endif
#if ! defined (FALSE)
# define FALSE            0
#endif

#define GETOPTN_NOTFOUND -1

  /* option style */
  /* GETOPTN_LEGACY:
   *    -ab  processes both -a and -b flags.
   * GETOPTN_MODERN:
   *    -ab  processes -ab flag.
   */
#define GETOPTN_LEGACY      'l'
#define GETOPTN_MODERN      'm'

  /* option types */
#define GETOPTN_BOOL        0    /* flips the value */
#define GETOPTN_INT         1
#define GETOPTN_LONG        2
#define GETOPTN_DOUBLE      3
#define GETOPTN_STRING      4
#define GETOPTN_STRPTR      5
#define GETOPTN_FUNC_BOOL   6
#define GETOPTN_FUNC_VALUE  7
#define GETOPTN_ALIAS       8
#define GETOPTN_IGNORE      9
#define GETOPTN_SIZET      10

typedef struct {
  const char    *option;
  int           option_type;
  void          *valptr;
  Size_t        valsiz;
  void          *value2;
} getoptn_opt_t;

extern int getoptn _((int style, int argc, const char * const argv [],
      Size_t optcount, getoptn_opt_t opts [], int *errorCount));

#endif /*DI_INC_GETOPTN_H_ */
