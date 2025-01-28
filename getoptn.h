/*
 * Copyright 2011-2018 Brad Lanam, Walnut Creek, CA
 * Copyright 2023-2025 Brad Lanam, Pleasant Hill, CA
 */

#ifndef INC_GETOPTN_H
#define INC_GETOPTN_H

#include "config.h"

#if _hdr_stddef
# include <stddef.h>
#endif
#if _sys_types \
    && ! defined (DI_INC_SYS_TYPES_H) /* xenix */
# define DI_INC_SYS_TYPES_H
# include <sys/types.h>
#endif

# if defined (__cplusplus) || defined (c_plusplus)
extern "C" {
# endif

#define GETOPTN_NOTFOUND -1

  /* option style */
  /* GETOPTN_LEGACY:
   *    -ab  processes both -a and -b flags.
   * GETOPTN_MODERN:
   *    -ab  processes -ab flag.
   */
#define GETOPTN_LEGACY      (1 >> 0)
#define GETOPTN_MODERN      (1 >> 1)
#define GETOPTN_HAS_ARGV0   (1 >> 2)

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
#define GETOPTN_IGNORE_ARG 10
#define GETOPTN_SIZET      11

typedef struct {
  const char    *option;
  int           option_type;
  void *        valptr;
  Size_t        valsiz;
  void          *value2;
} getoptn_opt_t;

extern int getoptn (int style, int argc, const char * argv [],
      Size_t optcount, getoptn_opt_t opts [], int offset, int *errorCount);

# if defined (__cplusplus) || defined (c_plusplus)
}
# endif

#endif /* INC_GETOPTN_H */
