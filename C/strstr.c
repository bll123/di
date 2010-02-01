/*
 * $Id$
 * $Source$
 * Copyright 1994-2010 Brad Lanam, Walnut Creek, CA
 */

#include "config.h"

#if ! _lib_strstr

# include "di.h"

# include <stdio.h>
# if _hdr_stdlib
#  include <stdlib.h>
# endif
# if _hdr_string
#  include <string.h>
# endif
# if _hdr_strings && ((! _hdr_string) || (_include_string))
#  include <strings.h>
# endif

char *
# if _proto_stdc
strstr (const char *buff, const char *srch)
# else
strstr (buff, srch)
  const char *buff;
  const char *srch;
# endif
{
  Size_t    len;
  char *    p;

  p = (char *) buff;
  if (srch == (char *) NULL) { return p; }

  len = strlen (srch);
  for (; (p = strchr (p, *srch)) != (char *) NULL; p++)
  {
    if (strncmp (p, srch, len) == 0)
    {
      return (p);
    }
  }

  return (char *) NULL;
}

#else

extern int debug;

#endif
