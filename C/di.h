/*
 * $Id$
 * $Source$
 * Copyright 1994-2010 Brad Lanam, Walnut Creek, CA
 */

#ifndef __INC_DI_H_
#define __INC_DI_H_

#include "config.h"

/*****************************************************/

#include <stdio.h>
#if _hdr_fcntl
# include <fcntl.h>
#endif
#if _sys_file
# include <sys/file.h>
#endif
#if _sys_types
# include <sys/types.h>
#endif
#if _hdr_limits
# include <limits.h>        /* PATH_MAX */
#endif
#if _sys_param
# include <sys/param.h>     /* MAXPATHLEN */
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
#if _sys_mount && \
    ! defined (_DI_INC_SYS_MOUNT)       /* NetBSD */
# define _DI_INC_SYS_MOUNT 1
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

#if ! _lib_memcpy && ! defined (memcpy)
# if ! _lib_bcopy
   error No_memcpy/bcopy_available.
# else
#  define memcpy(dst, src, cnt)     (bcopy((src), (dst), (cnt)), dst)
# endif
#endif

#if ! _lib_memset && ! defined (memset)
# if ! _lib_bzero
   error No_memset/bzero_available.
# else
#  define memset(s,c,n)    (bzero ((s), (n)), s)
# endif
#endif

#if ! _dcl_errno
  extern int     errno;
#endif
#if ! _dcl_optind
  extern int optind;
#endif
#if ! _dcl_optarg
  extern char *optarg;
#endif

#define DI_NAME_LEN            MAXPATHLEN
#define DI_SPEC_NAME_LEN       MAXPATHLEN
#define DI_OPT_LEN             MAXPATHLEN
#define DI_MNT_TIME_LEN        24

#if _siz_long_long == 0
    typedef unsigned long _fs_size_t;
    typedef long _s_fs_size_t;
#else
 #if _siz_long_long >= 8
    typedef unsigned long long _fs_size_t;
    typedef long long _s_fs_size_t;
 #else
    typedef unsigned long _fs_size_t;
    typedef long _s_fs_size_t;
 #endif
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
#define DI_PRNT_EXCLUDE     4
#define DI_PRNT_FORCE       5

typedef struct
{
    unsigned int    index;
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
    char            doPrint;                     /* do we want to print  */
                                                 /* this entry?          */
    char            printFlag;                   /* print flags          */
    char            isLocal;                     /* is this mount point  */
                                                 /* local?               */
    char            isReadOnly;                  /* is this mount point  */
                                                 /* read-only?           */
    char            name [DI_NAME_LEN + 1];         /* mount point          */
    char            special [DI_SPEC_NAME_LEN + 1]; /* special device name  */
    char            fsType [DI_TYPE_LEN + 1];       /* type of file system  */
    char            options [DI_OPT_LEN + 1];
    char            mountTime [DI_MNT_TIME_LEN + 1];
} diDiskInfo_t;

# if defined(__cplusplus)
   extern "C" {
# endif

 /* digetentries.c */
extern int  di_getDiskEntries       _((diDiskInfo_t **, int *));
 /* digetinfo.c */
extern void di_getDiskInfo          _((diDiskInfo_t **, int *));
 /* getopt.c */
# if ! _lib_getopt || _npt_getopt
extern int getopt                   _((char *, char *[], char *));
# endif
 /* strdup.c */
# if ! _lib_strdup
extern char *strdup                 _((char *, Size_t));
# endif
 /* strstr.c */
# if ! _lib_strstr
extern char *strstr                 _((char *, char *));
# endif
 /* trimchar.c */
extern void trimChar                _((char *, int));
 /* realloc.c */
extern void *Realloc                _((void *, Size_t));
 /* didiskutil.c */
extern void di_initDiskInfo _((diDiskInfo_t *));
extern void di_saveBlockSizes _((diDiskInfo_t *, _fs_size_t, _fs_size_t, _fs_size_t, _fs_size_t));
extern void di_saveInodeSizes _((diDiskInfo_t *, _fs_size_t, _fs_size_t, _fs_size_t));
#if _lib_getmntent && \
    ! _lib_getmntinfo && \
    ! _lib_getfsstat && \
    ! _lib_getvfsstat && \
	! _lib_mntctl && \
	! _class_os__Volumes
extern char *chkMountOptions        _((char *, char *));
#endif
extern void convertMountOptions     _((long, diDiskInfo_t *));
extern void convertNFSMountOptions  _((long, long, long, diDiskInfo_t *));
extern void di_testRemoteDisk       _((diDiskInfo_t *));

/* workaround for cygwin                                              */
/* if we have a getopt header, there's probably a getopt lib function */
# if ! _lib_getopt && ! _hdr_getopt
extern int getopt _((int argc, char *argv [], char *optstring));
# endif

# if defined(__cplusplus)
   }
# endif

     /* macro for gettext() */
#ifndef DI_GT
# if _enable_nls
#  define DI_GT(args) gettext(args)
# else
#  define DI_GT(args) (args)
# endif
#endif

#endif /* __INC_DI_H_ */
