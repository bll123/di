/*
 * Copyright 1994-2018 Brad Lanam, Walnut Creek, CA
 * Copyright 2023 Brad Lanam, Pleasant Hill, CA
 */

#include "config.h"

#if ! _lib_strstr

#if _hdr_stdio
#  include <stdio.h>
#endif
# if _hdr_stdlib
#  include <stdlib.h>
# endif
# if _hdr_string
#  include <string.h>
# endif
# if _hdr_strings
#  include <strings.h>
# endif

# include "di.h"

char *
strstr (const char *buff, const char *srch)
{
  Size_t    len;
  char *    p;

  p = buff;
  if (srch == NULL) {
    return p;
  }

  len = strlen (srch);
  for (; (p = strchr (p, *srch)) != NULL; p++) {
    if (strncmp (p, srch, len) == 0) {
      return (p);
    }
  }

  return (char *) NULL;
}

#else

extern int debug;

#endif
