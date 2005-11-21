/*
$Id$
$Source$
Copyright 1994-2004 Brad Lanam, Walnut Creek, CA
*/

#ifndef __INC_DI_H_
#define __INC_DI_H_

/* $Id$ */

#include "config.h"

/*****************************************************/

#include <stdio.h>
#if _sys_types
# include <sys/types.h>
#endif
#if _hdr_limits
# include <limits.h>        /* has PATH_MAX */
#endif
#if _sys_param
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

#if _sys_fstyp
# include <sys/fstyp.h>
# if defined (FSTYPSZ)
#  define DI_TYPE_LEN          FSTYPSZ
# endif
#endif
#if _sys_mount
# include <sys/mount.h>
# if ! defined (DI_TYPE_LEN) && defined (MFSNAMELEN)
#  define DI_TYPE_LEN          MFSNAMELEN
# endif
#endif
#if _sys_vfstab
# include <sys/vfstab.h>
# if ! defined (DI_TYPE_LEN) && defined (FSTYPSZ)
#  define DI_TYPE_LEN         FSTYPSZ
# endif
#endif

#if ! defined (DI_TYPE_LEN)
# define DI_TYPE_LEN          16
#endif

#if ! defined (_lib_memcpy) && ! defined (memcpy)
# if ! defined (_lib_bcopy)
   error No memcpy/bcopy available.
# else
#  define memcpy(dst, src, cnt)     (bcopy((src), (dst), (cnt)), dst)
# endif
#endif

#if ! defined (_lib_memset) && ! defined (memset)
# if ! defined (_lib_bzero)
   error No memset/bzero available.
# else
#  define memset(s,c,n)    (bzero ((s), (n)), s)
# endif
#endif

#define DI_NAME_LEN            MAXPATHLEN
#define DI_SPEC_NAME_LEN       MAXPATHLEN
#define DI_OPT_LEN             MAXPATHLEN
#define DI_MNT_TIME_LEN        24

#if _siz_long_long >= 8
    typedef unsigned long long _fs_size_t;
    typedef long long _s_fs_size_t;
#else
    typedef unsigned long _fs_size_t;
    typedef long _s_fs_size_t;
#endif

typedef unsigned long __ulong;

#if ! defined (TRUE)
# define TRUE             1
#endif
#if ! defined (FALSE)
# define FALSE            0
#endif

#define DI_PRNT_IGNORE      0
#define DI_PRNT_OK          1
#define DI_PRNT_BAD         2
#define DI_PRNT_OUTOFZONE   3

typedef struct
{
    _fs_size_t      totalBlocks;
    _fs_size_t      freeBlocks;
    _fs_size_t      availBlocks;
    _fs_size_t      blockSize;
    _fs_size_t      totalInodes;
    _fs_size_t      freeInodes;
    _fs_size_t      availInodes;
    __ulong         st_dev;                      /* disk device number   */
    __ulong         sp_dev;                      /* special device number*/
    __ulong         sp_rdev;                     /* special rdev #       */
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
extern void *Realloc               _((void *, Size_t));

/* workaround for cygwin                                              */
/* if we have a getopt header, there's probably a getopt lib function */
# if ! defined (_lib_getopt) && ! defined (_hdr_getopt)
extern int getopt _((int argc, char *argv [], char *optstring));
# endif

#endif /* __INC_DI_H_ */
