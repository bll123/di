/*
 * Copyright 2023-2025 Brad Lanam, Pleasant Hill, CA
 */

#ifndef INC_OPTIONS_H
#define INC_OPTIONS_H

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

#define DI_SORT_TYPE_MAX     10

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
  /* should be 1, 512, 1000 or 1024 */
  int             dispBlockSize;
  unsigned int    baseDispSize;
  unsigned int    baseDispIdx;
  char            sortType [DI_SORT_TYPE_MAX + 1];
  int             optval [DI_OPT_MAX];
  unsigned int    exitFlag;
  int             formatLen;
  int             errorCount;
  int             optidx;
  int             argc;
  int             optiteridx;
} di_opt_t;

extern di_opt_t * di_init_options (void);
extern void di_opt_cleanup (di_opt_t *);
extern int di_get_options (int, char * argv [], di_opt_t *);
extern void di_opt_format_iter_init (di_opt_t *);
extern int di_opt_format_iterate (di_opt_t *);
int di_opt_check_option (di_opt_t *, int);

# if defined (__cplusplus) || defined (c_plusplus)
}
# endif

#endif /* INC_OPTIONS_H */
