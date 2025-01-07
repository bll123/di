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

/* dilib.c */
extern void di_get_data   (di_data_t *di_data, int, const char * const [], int);
extern void checkDiskInfo       (di_data_t *, int);
extern void checkDiskQuotas     (di_data_t *);
extern int  checkFileInfo       (di_data_t *, int, int, const char *const[]);
extern void di_cleanup             (di_data_t *);
extern int  getDiskSpecialInfo  (di_data_t *, unsigned int);
extern void getDiskStatInfo     (di_data_t *);
extern void preCheckDiskInfo    (di_data_t *);
extern void initLocale          (void);
extern void initZones           (di_data_t *);

# if defined (__cplusplus) || defined (c_plusplus)
}
# endif

#endif /* DI_INC_DILIB_H */
