/*
 * Copyright 2016-2018 Brad Lanam Walnut Creek CA USA
 * Copyright 2023-2025 Brad Lanam, Pleasant Hill, CA
 */

#ifndef DI_INC_DILIB_H
#define DI_INC_DILIB_H

#include "config.h"
#include "disystem.h"
#include "dimath.h"
#include "options.h" // ### for sortarray, may change later

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

typedef struct
{
  unsigned int  sortIndex [DI_MAX_SORT_IDX];
  dinum_t       values [DI_VALUE_MAX];
  unsigned long st_dev;                      /* disk device number       */
  unsigned long sp_dev;                      /* special device number    */
  unsigned long sp_rdev;                     /* special rdev #           */
  char          doPrint;                     /* should this entry        */
                                             /*   be printed?            */
  char          printFlag;                   /* print flags              */
  char          isLocal;                     /* is this mount point      */
                                             /*   local?                 */
  char          isReadOnly;                  /* is this mount point      */
                                             /*   read-only?             */
  char          isLoopback;                  /* lofs or none fs type?    */
  char          *mountpt;                    /* mount point           */
  char          *devname;                    /* special device name   */
  char          *fstype;                     /* type of file system   */
  char          *options;                    /* mount options         */
  char          *mountTime;
} di_disk_info_t;

typedef struct {
  int             count;
  int             haspooledfs;
  int             disppooledfs;
  int             totsorted;
  pvoid           *options;
  di_disk_info_t  *diskInfo;
  pvoid           *zoneInfo;
} di_data_t;

/* dilib.c */
extern void di_initialize (di_data_t *di_data);
extern int di_process_options (di_data_t *di_data, int argc, char * argv []);
extern void di_get_data (di_data_t *di_data);
extern void di_cleanup (di_data_t *di_data);

// ### these should be internal only
extern void sortArray (di_opt_t *, di_disk_info_t *, int, int);
extern const char *getPrintFlagText (int);

# if defined (__cplusplus) || defined (c_plusplus)
}
# endif

#endif /* DI_INC_DILIB_H */
