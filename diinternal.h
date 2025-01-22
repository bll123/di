/* Copyright 2025 Brad Lanam Pleasant Hill CA */

#ifndef INC_DIINTERNAL_H
#define INC_DIINTERNAL_H

#include "config.h"
#include "disystem.h"
#include "di.h"
#include "dimath.h"
#include "dioptions.h"    // ### temporary

# if defined (__cplusplus) || defined (c_plusplus)
extern "C" {
# endif

/* di defines */

#define DI_MOUNTPT_LEN         MAXPATHLEN
#define DI_FILESYSTEM_LEN         MAXPATHLEN
#define DI_MOUNT_OPT_LEN             MAXPATHLEN
#define DI_MNT_TIME_LEN        24
#ifndef DI_DEFAULT_DISP_SIZE
# define DI_DEFAULT_DISP_SIZE "H"
#endif
#ifndef DI_DEFAULT_FORMAT
# define DI_DEFAULT_FORMAT "smbuvpT"
#endif

#define DI_SORT_MAIN    0
#define DI_SORT_TOTAL     1
#define DI_SORT_MAX     2

/* structures */

typedef struct
{
  int           sortIndex [DI_SORT_MAX];
  dinum_t       values [DI_VALUE_MAX];
  unsigned long st_dev;                     /* disk device number       */
  unsigned long sp_dev;                     /* special device number    */
  unsigned long sp_rdev;                    /* special rdev #           */
  char          *strdata [DI_DISP_MAX];     /* mount point           */
                                            /* special device name   */
                                            /* type of file system   */
                                            /* mount options         */
  int           doPrint;                    /* should this entry        */
                                            /*   be printed?            */
  int           printFlag;                  /* print flags              */
  int           isLocal;                    /* is this mount point      */
                                            /*   local?                 */
  int           isReadOnly;                 /* is this mount point      */
                                            /*   read-only?             */
  int           isLoopback;                 /* lofs or none fs type?    */
} di_disk_info_t;

typedef struct {
  void            *options;
  di_disk_info_t  *diskInfo;
  di_disk_info_t  totals;
  void            *zoneInfo;
  void            *pub;
  /* fscount is the number of partitions */
  /* the allocation count is one greater to hold the totals bucket */
  int             fscount;
  int             dispcount;
  int             iteridx;
  int             iteropt;
  int             haspooledfs;
  int             disppooledfs;
  int             totsorted;
} di_data_t;

/* dilib.c */
// ### should these be internal?
extern const char *getPrintFlagText (int);

/* digetentries.c */
extern int  di_get_disk_entries (di_disk_info_t **, int *);

/* digetinfo.c */
extern void di_get_disk_info (di_disk_info_t **, int *);

/* didiskutil.c */
extern void di_initialize_disk_info (di_disk_info_t *, int);
extern void di_free_disk_info (di_disk_info_t *);
extern void di_save_block_sizes (di_disk_info_t *, di_unum_t, di_unum_t, di_unum_t, di_unum_t);
extern void di_save_inode_sizes (di_disk_info_t *, di_unum_t, di_unum_t, di_unum_t);
#if _lib_getmntent \
    && ! _lib_getmntinfo \
    && ! _lib_getfsstat \
    && ! _lib_getvfsstat \
    && ! _lib_mntctl
extern char *chkMountOptions        (const char *, const char *);
#endif
extern void convertMountOptions     (unsigned long, di_disk_info_t *);
extern void convertNFSMountOptions  (long, long, long, di_disk_info_t *);
extern void di_is_remote_disk       (di_disk_info_t *);
extern int  di_isPooledFs           (di_disk_info_t *);
extern int  di_isLoopbackFs         (di_disk_info_t *);
extern Size_t di_mungePoolName      (char *);

# if defined (__cplusplus) || defined (c_plusplus)
}
# endif

#endif /* INC_DIINTERNAL_H */
