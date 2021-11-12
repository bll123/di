#ifndef DI_INC_DISPLAY_H
#define DI_INC_DISPLAY_H

#include "config.h"
#include "di.h"

# if defined (__cplusplus) || defined (c_plusplus)
   extern "C" {
# endif

    /* display.c */
extern char *printDiskInfo      (diData_t *);
extern void sortArray           (diOptions_t *, diDiskInfo_t *, int, int);
extern const char *getPrintFlagText (int);

# if defined (__cplusplus) || defined (c_plusplus)
   }
# endif

#endif /* DI_INC_DISPLAY_H */
