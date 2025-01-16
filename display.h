/*
 * Copyright 2023-2025 Brad Lanam, Pleasant Hill, CA
 */

#ifndef DI_INC_DISPLAY_H
#define DI_INC_DISPLAY_H

#include "config.h"
#include "di.h"

# if defined (__cplusplus) || defined (c_plusplus)
extern "C" {
# endif

/* display.c */
extern void di_display_data (di_data_t *di_data);
extern char *printDiskInfo      (di_data_t *);
extern void initLocale          (void);

# if defined (__cplusplus) || defined (c_plusplus)
}
# endif

#endif /* DI_INC_DISPLAY_H */
