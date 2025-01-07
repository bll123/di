/*
 * Copyright 1994-2018 Brad Lanam, Walnut Creek, CA
 * Copyright 2023 Brad Lanam, Pleasant Hill, CA
 */

#include "config.h"

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

#include "di.h"

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

