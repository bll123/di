/*
 * Copyright 2016-2018 Brad Lanam Walnut Creek CA USA
 * Copyright 2023-2025 Brad Lanam, Pleasant Hill, CA
 */

#ifndef DI_INC_DILIB_H
#define DI_INC_DILIB_H

#include "config.h"
#include "di.h"

# if defined (__cplusplus) || defined (c_plusplus)
extern "C" {
# endif

#if ! _dcl_errno
  extern int errno;
#endif

# if defined (__cplusplus) || defined (c_plusplus)
}
# endif

#endif /* DI_INC_DILIB_H */
