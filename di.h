/*
 * Copyright 2016-2018 Brad Lanam Walnut Creek CA USA
 * Copyright 2023-2025 Brad Lanam, Pleasant Hill, CA
 */

#ifndef INC_DI_H
#define INC_DI_H

#include "dimath.h"
#include "options.h" // ### for sortarray, need to re-work this

# if defined (__cplusplus) || defined (c_plusplus)
extern "C" {
# endif

/* used as indexes into the dispTable array */
#define DI_KILO           0
#define DI_MEGA           1
#define DI_GIGA           2
#define DI_TERA           3
#define DI_PETA           4
#define DI_EXA            5
#define DI_ZETTA          6
#define DI_YOTTA          7
#define DI_RONNA          8
#define DI_QUETTA         9

/* print flags */
#define DI_PRNT_IGNORE      0
#define DI_PRNT_OK          1
#define DI_PRNT_BAD         2
#define DI_PRNT_OUTOFZONE   3
#define DI_PRNT_EXCLUDE     4
#define DI_PRNT_FORCE       5
#define DI_PRNT_SKIP        6

/* value identifiers */
#define DI_SPACE_TOTAL      0
#define DI_SPACE_FREE       1
#define DI_SPACE_AVAIL      2
#define DI_INODE_TOTAL      3
#define DI_INODE_FREE       4
#define DI_INODE_AVAIL      5
#define DI_VALUE_MAX        6

/* options return values */
#define DI_EXIT_NORM      0
#define DI_EXIT_HELP      1
#define DI_EXIT_VERS      2
#define DI_EXIT_WARN      3
#define DI_EXIT_FAIL      4

#define DI_MAX_SORT_IDX     2

/* dilib.c */
extern void * di_initialize (void);
extern int di_process_options (void *di_data, int argc, char * argv []);
extern void di_get_data (void *di_data);
extern void di_cleanup (void *di_data);

# if defined (__cplusplus) || defined (c_plusplus)
}
# endif

#endif /* INC_DI_H */
