#ifndef __INC_DI_H_
#define __INC_DI_H_

/* $Id$ */

#include "config.h"

#if defined (I_LIMITS)
# include <limits.h>        /* has PATH_MAX */
#endif
#if defined (I_SYS_PARAM)
# include <sys/param.h>     /* has MAXPATHLEN */
#endif

#if ! defined (MAXPATHLEN)
# if defined (_POSIX_PATH_MAX)
#  define MAXPATHLEN        _POSIX_PATH_MAX
# else
#  if defined (PATH_MAX)
#   define MAXPATHLEN        PATH_MAX
#  endif
#  if defined (LPNMAX)
#   define MAXPATHLEN         LPNMAX
#  endif
# endif
#endif

#if ! defined (MAXPATHLEN)
# define MAXPATHLEN         255
#endif

#if defined (I_SYS_FSTYP)
# include <sys/fstyp.h>
# define DI_TYPE_LEN          FSTYPSZ
#endif
#if defined (I_SYS_VFSTAB)
# include <sys/vfstab.h>
# if ! defined (DI_TYPE_LEN)
#  define DI_TYPE_LEN         FSTYPSZ
# endif
#endif

#if ! defined (DI_TYPE_LEN)
# define DI_TYPE_LEN          16
#endif

#if (defined (_LARGEFILE_SOURCE) || defined (_LARGEFILE64_SOURCE)) && \
        _FILE_OFFSET_BITS == 64
# define HAS_64BIT_STATFS_FLDS 1
#endif

#if ! defined (HAS_MEMCPY) && ! defined (memcpy)
# if ! defined (HAS_BCOPY)
   error No memcpy/bcopy available.
# else
#  define memcpy(dst, src, cnt)     (bcopy((src), (dst), (cnt)), dst)
# endif
#endif

#if ! defined (HAS_MEMSET) && ! defined (memset)
# if ! defined (HAS_BZERO)
   error No memset/bzero available.
# else
#  define memset(s,c,n)    (bzero ((s), (n)), s)
# endif
#endif

#define DI_NAME_LEN            MAXPATHLEN
#define DI_SPEC_NAME_LEN       MAXPATHLEN
#define DI_OPT_LEN             MAXPATHLEN
#define DI_MNT_TIME_LEN        24

#if defined (HAS_64BIT_STATFS_FLDS)
    typedef unsigned long long _fs_size_t;
    typedef long long _s_fs_size_t;
#else
    typedef unsigned long _fs_size_t;
    typedef long _s_fs_size_t;
#endif

typedef unsigned long _ulong;

#if ! defined (TRUE)
# define TRUE             1
#endif
#if ! defined (FALSE)
# define FALSE            0
#endif

#define DI_PRNT_IGNORE      0
#define DI_PRNT_OK          1
#define DI_PRNT_BAD         2

typedef struct
{
    _fs_size_t      totalBlocks;
    _fs_size_t      freeBlocks;
    _fs_size_t      availBlocks;
    _fs_size_t      blockSize;
    _fs_size_t      totalInodes;
    _fs_size_t      freeInodes;
    _fs_size_t      availInodes;
    _ulong          st_dev;                      /* disk device number   */
    _ulong          sp_dev;                      /* special device number*/
    _ulong          sp_rdev;                     /* special rdev #       */
    char            printFlag;                   /* do we want to print  */
                                                 /* this entry?          */
    char            isLocal;                     /* is this mount point  */
                                                 /* local?               */
    char            isReadOnly;                  /* is this mount point  */
                                                 /* read-only?           */
    char            name [DI_NAME_LEN + 1];         /* mount point          */
    char            special [DI_SPEC_NAME_LEN + 1]; /* special device name  */
    char            fsType [DI_TYPE_LEN + 1];       /* type of file system  */
    char            options [DI_OPT_LEN + 1];
    char            mountTime [DI_MNT_TIME_LEN + 1];
} di_DiskInfo;

extern int  di_getDiskEntries      _((di_DiskInfo **, int *));
extern void di_getDiskInfo         _((di_DiskInfo **, int *));
extern void di_testRemoteDisk      _((di_DiskInfo *));
extern void *Realloc               _((void *, long));

#endif /* __INC_DI_H_ */
