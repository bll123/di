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
#if _hdr_memory
# include <memory.h>
#endif
#if _hdr_malloc
# include <malloc.h>
#endif

/*
 *
 * portable realloc
 * some variants don't accept a null pointer for initial allocation.
 *
 */

void *
#if _proto_stdc
_realloc (void *ptr, Size_t size)
#else
_realloc (ptr, size)
    void        *ptr;
    Size_t      size;
#endif
{
    if (ptr == (void *) NULL)
    {
        ptr = (void *) malloc (size);
    }
    else
    {
        ptr = (void *) realloc (ptr, size);
    }

    return ptr;
}

