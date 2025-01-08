/*
 * Copyright 1994-2018 Brad Lanam, Walnut Creek, CA
 * Copyright 2023-2025 Brad Lanam, Pleasant Hill, CA
 */

#include "config.h"

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

/*
 *
 * portable realloc
 * some very old variants don't accept a null pointer for initial allocation.
 *
 */

void *
di_realloc (void * ptr, Size_t size)
{
  if (ptr == NULL) {
    ptr = malloc (size);
  } else {
    ptr = realloc (ptr, size);
  }

  return ptr;
}

void
di_trimchar (char *str, int ch)
{
  int     len;

  len = (int) strlen (str);
  if (len > 0) {
    --len;
  }
  if (len >= 0) {
    if (str [len] == ch) {
      str [len] = '\0';
    }
  }
}

#if ! _lib_strdup

char *
strdup (const char *ptr)
{
  Size_t        len;
  char          *nptr;

  if (ptr == NULL) {
    return NULL;
  }

  len = strlen (ptr);
  nptr = malloc (len + 1);
  strncpy (nptr, ptr, len);
  return nptr;
}

#endif /* ! _lib_strdup */

#if ! _lib_strstr

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

#endif /* ! _lib_strstr */
