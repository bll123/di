#ifndef DI_INC_DISPLAY_H
#define DI_INC_DISPLAY_H

/*
 * Copyright 2023-2024 Brad Lanam, Pleasant Hill, CA
 */

#include "config.h"
#include "di.h"

# if defined (__cplusplus) || defined (c_plusplus)
extern "C" {
# endif

    /* display.c */
extern char *printDiskInfo      (di_data_t *);
extern void sortArray           (di_opt_t *, di_disk_info_t *, int, int);
extern const char *getPrintFlagText (int);

# if defined (__cplusplus) || defined (c_plusplus)
}
# endif

#endif /* DI_INC_DISPLAY_H */
