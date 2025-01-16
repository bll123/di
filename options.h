/*
 * Copyright 2023-2025 Brad Lanam, Pleasant Hill, CA
 */

#ifndef DI_INC_OPTIONS_H
#define DI_INC_OPTIONS_H

#include "disystem.h"

# if defined (__cplusplus) || defined (c_plusplus)
extern "C" {
# endif

#define DI_VAL_512              512.0
#define DI_VAL_1000             1000.0
#define DI_DISP_1000_IDX        0
#define DI_VAL_1024             1024.0
#define DI_DISP_1024_IDX        1

#define DI_DISP_HR        -20.0
#define DI_DISP_HR_2      -21.0

#define DI_SORT_MAX     10

typedef struct
{
  int    count;
  char   **list;
} di_strarr_t;

typedef struct di_opt {
  char            ** argv;
  const char      *formatString;
  di_strarr_t     ignore_list;
  di_strarr_t     include_list;
  char            zoneDisplay [MAXPATHLEN + 1];
  /* should always be <= 1024 */
  /* usually 1000 or 1024 */
  int             dispBlockSize;
//  unsigned long         dispScaleValue;
  unsigned int    baseDispSize;
  unsigned int    baseDispIdx;
  char            sortType [DI_SORT_MAX + 1];
  unsigned int    posix_compat;
  unsigned int    quota_check;
  unsigned int    csv_output;
  unsigned int    csv_tabs;
  unsigned int    excludeLoopback;
  unsigned int    json_output;
  unsigned int    printTotals;
  unsigned int    printDebugHeader;
  unsigned int    printHeader;
  unsigned int    displayAll;
  unsigned int    localOnly;
  unsigned int    dontResolveSymlink;
  unsigned int    exitFlag;
  int             errorCount;
  int             optidx;
  int             argc;
} di_opt_t;

extern di_opt_t * di_init_options (void);
extern int di_get_options (int , char * argv [], di_opt_t *);

# if defined (__cplusplus) || defined (c_plusplus)
}
# endif

#endif /* DI_INC_OPTIONS_H */
