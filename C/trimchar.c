/*
 * $Id$
 * $Source$
 * Copyright 1994-2010 Brad Lanam, Walnut Creek, CA
 */

#include "config.h"
#include "di.h"

#include <stdio.h>
#if _hdr_stdlib
# include <stdlib.h>
#endif
#if _hdr_string
# include <string.h>
#endif
#if _hdr_strings && ((! _hdr_string) || (_include_string))
# include <strings.h>
#endif

void
#if _proto_stdc
trimChar (char *str, int ch)
#else
trimChar (str, ch)
    char         *str;
    int          ch;
#endif
{
    unsigned int  len;

    len = strlen (str);
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
