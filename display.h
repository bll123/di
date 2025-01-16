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

#ifndef DI_LOCALE_DIR
// ### temporary
# define DI_LOCALE_DIR "/usr/share/di/locale"
#endif

typedef struct {
  Size_t       maxMntTimeString;
  Size_t       maxMountString;
  Size_t       maxOptString;
  Size_t       maxSpecialString;
  Size_t       maxTypeString;
  const char   *dispBlockLabel;
  char         blockFormat [25];
  char         blockFormatNR [25];   /* no radix */
  char         inodeFormat [25];
  char         inodeLabelFormat [25];
  char         mountFormat [25];
  char         mTimeFormat [25];
  char         optFormat [25];
  char         specialFormat [25];
  char         typeFormat [25];
} diOutput_t;

/* display.c */
extern void di_display_data (di_data_t *di_data);
extern char *printDiskInfo      (di_data_t *);
extern void initLocale          (void);

# if defined (__cplusplus) || defined (c_plusplus)
}
# endif

#endif /* DI_INC_DISPLAY_H */
