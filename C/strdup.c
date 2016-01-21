/*
 * $Id$
 * $Source$
 * Copyright 1994-2010 Brad Lanam, Walnut Creek, CA
 */

#include "config.h"

#if ! _lib_strdup

# include "di.h"

#if _hdr_stdio
#  include <stdio.h>
#endif
# if _hdr_stdlib
#  include <stdlib.h>
# endif
# if _hdr_memory
#  include <memory.h>
# endif
# if _hdr_malloc
#  include <malloc.h>
# endif
# if _hdr_string
#  include <string.h>
# endif
# if _hdr_strings
#  include <strings.h>
# endif
#if _use_mcheck
# include <mcheck.h>
#endif

char *
# if _proto_stdc
strdup (const char *ptr)
# else
strdup (ptr)
    const char        *ptr;
# endif
{
  Size_t        len;
  char          *nptr;

  if (ptr == (char *) NULL)
  {
    return (char *) NULL;
  }

  len = strlen (ptr);
  nptr = (char *) malloc (len + 1);
  strncpy (nptr, ptr, len);
  return nptr;
}

#else

extern int debug;

#endif
