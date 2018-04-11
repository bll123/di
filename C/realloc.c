/*
 * Copyright 1994-2018 Brad Lanam, Walnut Creek, CA
 */

#include "config.h"
#include "di.h"

#if _hdr_stdio
# include <stdio.h>
#endif
#if _hdr_stdlib
# include <stdlib.h>
#endif
#if _hdr_memory
# include <memory.h>
#endif
#if _hdr_malloc
# include <malloc.h>
#endif
#if _use_mcheck
# include <mcheck.h>
#endif

/*
 *
 * portable realloc
 * some variants don't accept a null pointer for initial allocation.
 *
 */

_pvoid
#if _proto_stdc
di_realloc (_pvoid ptr, Size_t size)
#else
di_realloc (ptr, size)
    _pvoid      ptr;
    Size_t      size;
#endif
{
    if (ptr == (_pvoid) NULL)
    {
        ptr = (_pvoid) malloc (size);
    }
    else
    {
        ptr = (_pvoid) realloc (ptr, size);
    }

    return ptr;
}

