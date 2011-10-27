#ifndef _INC_DISPLAY_H
#define _INC_DISPLAY_H

#include "config.h"
#include "di.h"

# if defined (__cplusplus) || defined (c_plusplus)
   extern "C" {
# endif

    /* display.c */
extern void printDiskInfo       _((diData_t *));
extern void sortArray           _((diOptions_t *, diDiskInfo_t *, int, int));
extern const char *getPrintFlagText _((int));

# if defined (__cplusplus) || defined (c_plusplus)
   }
# endif

#endif /* _INC_DISPLAY_H */
