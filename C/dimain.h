#ifndef _INC_DIMAIN_H
#define _INC_DIMAIN_H

/*
 * Copyright 2016 Brad Lanam Walnut Creek CA USA
 */

#include "config.h"
#include "di.h"
#if _hdr_zone
# include <zone.h>
#endif

# if defined (__cplusplus) || defined (c_plusplus)
   extern "C" {
# endif

#if ! _dcl_errno
  extern int errno;
#endif

 /* dimain.c */
extern char *dimainproc         _((int, const char * const [], int));
extern void checkDiskInfo       _((diData_t *, int));
extern void checkDiskQuotas     _((diData_t *));
extern int  checkFileInfo       _((diData_t *, int, int, const char *const[]));
#if _lib_zone_list && _lib_getzoneid && _lib_zone_getattr
extern void checkZone           _((diDiskInfo_t *, zoneInfo_t *, unsigned int));
#endif
extern void cleanup             _((diData_t *));
extern int  getDiskSpecialInfo  _((diData_t *, unsigned int));
extern void getDiskStatInfo     _((diData_t *));
extern void preCheckDiskInfo     _((diData_t *));
extern void initLocale          _((void));
extern void initZones           _((diData_t *));

# if defined (__cplusplus) || defined (c_plusplus)
   }
# endif

#endif /* _INC_DIMAIN_H */
