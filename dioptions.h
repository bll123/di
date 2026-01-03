/* Copyright 2023-2026 Brad Lanam, Pleasant Hill, CA */

#ifndef INC_DIOPTIONS_H
#define INC_DIOPTIONS_H

#include "disystem.h"
#include "di.h"
#include "getoptn.h"

# if defined (__cplusplus) || defined (c_plusplus)
extern "C" {
# endif

#define DI_BLKSZ_1                1
#define DI_BLKSZ_1000             1000
#define DI_BLKSZ_1024             1024

#define DI_SORT_TYPE_MAX        10

typedef struct
{
  Size_t  count;
  char    **list;
} di_strarr_t;

typedef struct di_opt {
  getoptn_opt_t   *opts;
  const char      ** argv;
  const char      *formatString;
  char            *diargsptr;
  di_strarr_t     exclude_list;
  di_strarr_t     include_list;
  char            zoneDisplay [MAXPATHLEN];
  int             optinit;
  /* will be either 1000 or 1024 */
  int             blockSize;
  int             scale;
  char            sortType [DI_SORT_TYPE_MAX];
  int             optval [DI_OPT_MAX];
  int             exitFlag;
  int             formatLen;
  int             errorCount;
  int             optidx;
  int             argc;
  int             optiteridx;
} di_opt_t;

extern di_opt_t * di_init_options (void);
extern void di_opt_cleanup (di_opt_t *diopts);
extern int di_get_options (int argc, const char * argv [], di_opt_t *diopts, int offset);
extern void di_opt_format_iter_init (di_opt_t *diopts);
extern int di_opt_format_iterate (di_opt_t *diopts);
int di_opt_check_option (di_opt_t *diopts, int optidx);

# if defined (__cplusplus) || defined (c_plusplus)
}
# endif

#endif /* INC_DIOPTIONS_H */
