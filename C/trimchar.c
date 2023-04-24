/*
 * Copyright 1994-2018 Brad Lanam, Walnut Creek, CA
 * Copyright 2023 Brad Lanam, Pleasant Hill, CA
 */

#include "config.h"
#include "di.h"

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

void
trimChar (char *str, int ch)
{
  int     len;

  len = (int) strlen (str);
  if (len > 0)
  {
    --len;
  }
  if (len >= 0)
  {
    if (str [len] == ch)
    {
      str [len] = '\0';
    }
  }
}
