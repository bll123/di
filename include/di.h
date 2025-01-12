/*
 * Copyright 1994-2018 Brad Lanam, Walnut Creek, CA
 * Copyright 2023-2025 Brad Lanam, Pleasant Hill, CA
 */

#ifndef DI_INC_DI_H
#define DI_INC_DI_H

#include "config.h"

/*****************************************************/

#include <stdio.h>
#if _hdr_stddef
# include <stddef.h>
#endif
#if _hdr_stdint
# include <stdint.h>
#endif
#if _hdr_fcntl \
    && ! defined (DI_INC_FCNTL_H)  /* xenix */
# define DI_INC_FCNTL_H
# include <fcntl.h>
#endif
#if _sys_file
# include <sys/file.h>
#endif
#if _sys_types \
    && ! defined (DI_INC_SYS_TYPES_H) /* xenix */
# define DI_INC_SYS_TYPES_H
# include <sys/types.h>
#endif
#if _hdr_limits
# include <limits.h>        /* PATH_MAX */
#endif
#if _sys_param
# include <sys/param.h>     /* MAXPATHLEN */
#endif
#if _hdr_zone
# include <zone.h>
#endif

#if ! defined (O_NOCTTY)
# define O_NOCTTY 0
#endif

#if ! defined (MAXPATHLEN)
# if defined (_POSIX_PATH_MAX)
#  define MAXPATHLEN        _POSIX_PATH_MAX
# else
#  if defined (PATH_MAX)
#   define MAXPATHLEN       PATH_MAX
#  endif
#  if defined (LPNMAX)
#   define MAXPATHLEN       LPNMAX
#  endif
# endif
#endif

#if ! defined (MAXPATHLEN)
# define MAXPATHLEN         255
#endif

#if _sys_fstyp                          /* HP-UX, Solaris */
# include <sys/fstyp.h>                 /* FSTYPSZ */
# if defined (FSTYPSZ)
#  define DI_TYPE_LEN       FSTYPSZ
# endif
#endif
#if _sys_mount \
    && ! defined (DI_INC_SYS_MOUNT)       /* NetBSD */
# define DI_INC_SYS_MOUNT 1
# include <sys/mount.h>                 /* MFSNAMELEN */
# if ! defined (DI_TYPE_LEN) && defined (MFSNAMELEN)
#  define DI_TYPE_LEN       MFSNAMELEN
# endif
#endif
#if _sys_vfstab                         /* ??? */
# include <sys/vfstab.h>
# if ! defined (DI_TYPE_LEN) && defined (FSTYPSZ)
#  define DI_TYPE_LEN       FSTYPSZ
# endif
#endif

#if ! defined (DI_TYPE_LEN)
# define DI_TYPE_LEN        65
#endif

#if ! _lib_memcpy && ! _define_memcpy
# if ! _lib_bcopy && ! _define_bcopy
   #error No_memcpy/bcopy_available.
# else
#  define memcpy(dst, src, cnt)     (bcopy((src), (dst), (cnt)), dst)
# endif
#endif

#if ! _lib_memset && ! _define_memset
# if ! _lib_bzero && ! _define_bzero
   #error No_memset/bzero_available.
# else
#  define memset(s,c,n)    (bzero ((s), (n)), s)
# endif
#endif

#include "dimath.h"

# if defined (__cplusplus) || defined (c_plusplus)
extern "C" {
# endif

#define DI_NAME_LEN            MAXPATHLEN
#define DI_SPEC_NAME_LEN       MAXPATHLEN
#define DI_OPT_LEN             MAXPATHLEN
#define DI_MNT_TIME_LEN        24
#ifndef DI_DEFAULT_DISP_SIZE
# define DI_DEFAULT_DISP_SIZE "H"
#endif
#ifndef DI_DEFAULT_FORMAT
# define DI_DEFAULT_FORMAT "smbuvpT"
#endif
#ifndef DI_LOCALE_DIR
/* temporary */
# define DI_LOCALE_DIR "/usr/share/di/locale"
#endif

typedef unsigned long __ulong;

#define DI_PRNT_IGNORE      0
#define DI_PRNT_OK          1
#define DI_PRNT_BAD         2
#define DI_PRNT_OUTOFZONE   3
#define DI_PRNT_EXCLUDE     4
#define DI_PRNT_FORCE       5
#define DI_PRNT_SKIP        6

#define DI_MAIN_SORT_IDX    0
#define DI_TOT_SORT_IDX     1

#define DI_SPACE_TOTAL      0
#define DI_SPACE_FREE       1
#define DI_SPACE_AVAIL      2
#define DI_INODE_TOTAL      3
#define DI_INODE_FREE       4
#define DI_INODE_AVAIL      5
#define DI_VALUE_MAX        6

#define DI_QUOTA_BLOCK_SZ   0
#define DI_QUOTA_LIMIT      1
#define DI_QUOTA_USED       2
#define DI_QUOTA_ILIMIT     3
#define DI_QUOTA_IUSED      4
#define DI_QVAL_MAX         5

#if 0
  dinum_t       total_space;
  dinum_t       free_space;
  dinum_t       avail_space;
  dinum_t       total_inodes;
  dinum_t       free_inodes;
  dinum_t       avail_inodes;
#endif

typedef struct
{
  unsigned int  sortIndex [2];
  dinum_t       values [DI_VALUE_MAX];
  __ulong       st_dev;                      /* disk device number       */
  __ulong       sp_dev;                      /* special device number    */
  __ulong       sp_rdev;                     /* special rdev #           */
  char          doPrint;                     /* do we want to print      */
                                               /* this entry?              */
  char          printFlag;                   /* print flags              */
  char          isLocal;                     /* is this mount point      */
                                               /* local?                   */
  char          isReadOnly;                  /* is this mount point      */
                                               /* read-only?               */
  char          isLoopback;                  /* lofs or none fs type?    */
  char          name [DI_NAME_LEN + 1];         /* mount point           */
  char          special [DI_SPEC_NAME_LEN + 1]; /* special device name   */
  char          fsType [DI_TYPE_LEN + 1];       /* type of file system   */
  char          options [DI_OPT_LEN + 1];
  char          mountTime [DI_MNT_TIME_LEN + 1];
} di_disk_info_t;

typedef struct
{
  char         *special;
  char         *name;
  char         *type;
  Uid_t        uid;
  Gid_t        gid;
  dinum_t       values [DI_QVAL_MAX];
#if 0
  dinum_t      block_size;
  dinum_t      limit;
  dinum_t      used;
  dinum_t      ilimit;
  dinum_t      iused;
#endif
} di_quota_t;

typedef struct
{
  int    count;
  char   **list;
} di_strarr_t;

#if ! _lib_zone_list
# define zoneid_t       int
# define ZONENAME_MAX   65
#endif

typedef struct {
  zoneid_t    zoneid;
  char        name [ZONENAME_MAX + 1];
  char        rootpath [MAXPATHLEN + 1];
  Size_t      rootpathlen;
} di_zone_summ_t;

typedef struct {
  Uid_t           uid;
  zoneid_t        myzoneid;
  di_zone_summ_t   *zones;
  Uint_t          zoneCount;
  char            zoneDisplay [MAXPATHLEN + 1];
  int             globalIdx;
} di_zone_info_t;

#define DI_SORT_MAX             10

/* order from best to worst */
#define DI_EXIT_NORM      0
#define DI_EXIT_HELP      1
#define DI_EXIT_WARN      2
#define DI_EXIT_FAIL      3

typedef struct {
  char            ** argv;
  const char      *formatString;
  /* should always be <= 1024 */
  /* usually 1000 or 1024 */
  int             dispBlockSize;
  dinum_t         dispScaleValue;
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

typedef struct {
  Size_t       inodeWidth;
  Size_t       maxMntTimeString;
  Size_t       maxMountString;
  Size_t       maxOptString;
  Size_t       maxSpecialString;
  Size_t       maxTypeString;
  Size_t       width;
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

typedef struct {
  int             count;
  int             haspooledfs;
  int             disppooledfs;
  int             totsorted;
  di_opt_t        options;
  diOutput_t      output;
  di_disk_info_t  *diskInfo;
  di_strarr_t     ignore_list;
  di_strarr_t     include_list;
  di_zone_info_t  zoneInfo;
} di_data_t;

#if ! _dcl_errno
  extern int errno;
#endif

/* digetentries.c */
extern int  di_get_disk_entries       (di_disk_info_t **, int *);

/* digetinfo.c */
extern void di_get_disk_info          (di_disk_info_t **, int *);

/* diquota.c */
extern void diquota                 (di_quota_t *);

/* didiskutil.c */
extern void di_init_disk_info (di_disk_info_t *);
extern void di_save_block_sizes (di_disk_info_t *, diuint_t, diuint_t, diuint_t, diuint_t);
extern void di_save_inode_sizes (di_disk_info_t *, diuint_t, diuint_t, diuint_t);
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

/* macro for gettext() */
#ifndef DI_GT
# if _enable_nls
#  define DI_GT(args) gettext(args)
# else
#  define DI_GT(args) (args)
# endif
#endif

/* these are also indexes into the dispTable array */
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

/* dilib.c */
extern void di_init (di_data_t *di_data, int intfcflag);
extern void di_process_options (di_data_t *di_data, int argc, char * argv []);
extern void di_get_data (di_data_t *di_data);
extern void di_cleanup (di_data_t *di_data);

# if defined (__cplusplus) || defined (c_plusplus)
}
# endif

#endif /* DI_INC_DI_H */
