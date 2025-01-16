/*
 * Copyright 2016-2018 Brad Lanam Walnut Creek CA USA
 * Copyright 2023-2025 Brad Lanam, Pleasant Hill, CA
 */

#ifndef DI_INC_DILIB_H
#define DI_INC_DILIB_H

#include "config.h"
#include "di.h"
#include "options.h" // ### for sortarray, may change later

# if defined (__cplusplus) || defined (c_plusplus)
extern "C" {
# endif

#if ! _dcl_errno
  extern int errno;
#endif

extern void sortArray (di_opt_t *, di_disk_info_t *, int, int);
extern const char *getPrintFlagText (int);

# if defined (__cplusplus) || defined (c_plusplus)
}
# endif

#endif /* DI_INC_DILIB_H */
