#ifndef DI_INC_DIMAIN_H
#define DI_INC_DIMAIN_H

/*
 * Copyright 2016-2018 Brad Lanam Walnut Creek CA USA
 * Copyright 2023-2024 Brad Lanam, Pleasant Hill, CA
 */

#include "config.h"
#include "di.h"

# if defined (__cplusplus) || defined (c_plusplus)
extern "C" {
# endif

#if ! _dcl_errno
  extern int errno;
#endif

 /* dimain.c */
extern char *dimainproc         (int, const char * const [], int, diData_t *);
extern void checkDiskInfo       (diData_t *, int);
extern void checkDiskQuotas     (diData_t *);
extern int  checkFileInfo       (diData_t *, int, int, const char *const[]);
extern void cleanup             (diData_t *);
extern int  getDiskSpecialInfo  (diData_t *, unsigned int);
extern void getDiskStatInfo     (diData_t *);
extern void preCheckDiskInfo    (diData_t *);
extern void initLocale          (void);
extern void initZones           (diData_t *);

# if defined (__cplusplus) || defined (c_plusplus)
}
# endif

#endif /* DI_INC_DIMAIN_H */
