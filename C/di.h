/*
$Id$
$Source$
Copyright 1994-2002 Brad Lanam, Walnut Creek, CA
*/

#ifndef __INC_DI_H_
#define __INC_DI_H_

/* $Id$ */

#include "config.h"

/******************************************************/
/* create 'Configure' forwards compatibility w/'iffe' */
/* until we know how portable iffe is...              */

#if defined (CAN_PROTOTYPE)
# define _proto_stdc 1
#endif
#if defined (HAS_BCOPY)
# define _lib_bcopy 1
#endif
#if defined (HAS_BINDTEXTDOMAIN)
# define _lib_bindtextdomain 1
#endif
#if defined (HAS_BZERO)
# define _lib_bzero 1
#endif
#if defined (HAS_ENDMNTENT)
# define _lib_endmntent 1
#endif
#if defined (HAS_ERRNO)
# define _lib_errno 1
#endif
#if defined (HAS_FS_INFO)
# define _lib_fs_info 1
#endif
#if defined (HAS_FS_STAT_DEV)
# define _lib_fs_stat_dev 1
#endif
#if defined (HAS_GETFSSTAT)
# define _lib_getfsstat 1
#endif
#if defined (HAS_GETMNT)
# define _lib_getmnt 1
#endif
#if defined (HAS_GETTEXT)
# define _lib_gettext 1
#endif
#if defined (HAS_GETMNTENT)
# define _lib_getmntent 1
#endif
#if defined (HAS_GETMNTINFO)
# define _lib_getmntinfo 1
#endif
#if defined (HAS_GETOPT)
# define _lib_getopt 1
#endif
#if defined (HAS_HASMNTOPT)
# define _lib_hasmntopt 1
#endif
#if defined (HAS_MEMCPY)
# define _lib_memcpy 1
#endif
#if defined (HAS_MEMSET)
# define _lib_memset 1
#endif
#if defined (HAS_MNTCTL)
# define _lib_mntctl 1
#endif
#if defined (HAS_MNT_TIME)
# define _lib_mnt_time 1
#endif
#if defined (HAS_SETLOCALE)
# define _lib_setlocale 1
#endif
#if defined (HAS_SETMNTENT)
# define _lib_setmntent 1
#endif
#if defined (HAS_SETMNTENT_1ARG)
# define _setmntent_1arg 1
#endif
#if defined (HAS_SETMNTENT_2ARG)
# define _setmntent_2arg 1
#endif
#if defined (HAS_SNPRINTF)
# define _lib_snprintf 1
#endif
#if defined (HAS_STATVFS)
# define _lib_statvfs 1
#endif
#if defined (HAS_SYSFS)
# define _lib_sysfs 1
#endif
#if defined (HAS_TEXTDOMAIN)
# define _lib_textdomain 1
#endif
#if defined (HAS_OPTIND)
# define _dcl_optind 1
#endif
#if defined (HAS_OPTARG)
# define _dcl_optarg 1
#endif
#if defined (HAS_STATFS_2ARG)
# define _statfs_2arg 1
#endif
#if defined (HAS_STATFS_3ARG)
# define _statfs_3arg 1
#endif
#if defined (HAS_STATFS_4ARG)
# define _statfs_4arg 1
#endif
#if defined (I_CTYPE)
# define _hdr_ctype 1
#endif
#if defined (I_ERRNO)
# define _hdr_errno 1
#endif
#if defined (I_FSHELP)
# define _hdr_fshelp 1
#endif
#if defined (I_KERNFSINFO)
# define _hdr_kernel_fs_info 1
#endif
#if defined (I_LIBINTL)
# define _hdr_libintl 1
#endif
#if defined (I_LIMITS)
# define _hdr_limits 1
#endif
#if defined (I_LOCALE)
# define _hdr_locale 1
#endif
#if defined (I_MALLOC)
# define _hdr_malloc 1
#endif
#if defined (I_MEMORY)
# define _hdr_memory 1
#endif
#if defined (I_MNTENT)
# define _hdr_mntent 1
#endif
#if defined (I_MNTTAB)
# define _hdr_mnttab 1
#endif
#if defined (I_STDLIB)
# define _hdr_stdlib 1
#endif
#if defined (I_GETOPT)
# define _hdr_getopt 1
#endif
#if defined (I_STOR_DIRECTORY)
# define _hdr_storage_Directory 1
#endif
#if defined (I_STOR_ENTRY)
# define _hdr_storage_Entry 1
#endif
#if defined (I_STOR_PATH)
# define _hdr_storage_Path 1
#endif
#if defined (I_STRING)
# define _hdr_string 1
#endif
#if defined (I_STRINGS)
# define _hdr_strings 1
#endif
#if defined (I_SYS_FSTYP)
# define _sys_fstyp 1
#endif
#if defined (I_SYS_FSTYPES)
# define _sys_fstypes 1
#endif
#if defined (I_SYS_FS_TYPES)
# define _sys_fs_types 1
#endif
#if defined (I_SYS_MNTCTL)
# define _sys_mntctl 1
#endif
#if defined (I_SYS_MNTENT)
# define _sys_mntent 1
#endif
#if defined (I_SYS_MNTTAB)
# define _sys_mnttab 1
#endif
#if defined (I_SYS_MOUNT)
# define _sys_mount 1
#endif
#if defined (I_SYS_PARAM)
# define _sys_param 1
#endif
#if defined (I_SYS_STAT)
# define _sys_stat 1
#endif
#if defined (I_SYS_STATFS)
# define _sys_statfs 1
#endif
#if defined (I_SYS_STATVFS)
# define _sys_statvfs 1
#endif
#if defined (I_SYS_TIME)
# define _sys_time 1
#endif
#if defined (I_SYS_TYPES)
# define _sys_types 1
#endif
#if defined (I_SYS_VFS)
# define _sys_vfs 1
#endif
#if defined (I_SYS_VFSTAB)
# define _sys_vfstab 1
#endif
#if defined (I_SYS_VMOUNT)
# define _sys_vmount 1
#endif
#if defined (I_TIME)
# define _hdr_time 1
#endif
#if defined (I_UNISTD)
# define _hdr_unistd 1
#endif
#if defined (I_WINDOWS)
# define _hdr_windows 1
#endif
#if defined (INCLUDE_MALLOC)
# define _include_malloc 1
#endif
#if defined (INCLUDE_STRING)
# define _include_string 1
#endif
#if defined (MEM_F_BSIZE_STATFS)
# define _mem_f_bsize_statfs 1
#endif
#if defined (MEM_F_FSIZE_STATFS)
# define _mem_f_fsize_statfs 1
#endif
#if defined (MEM_F_FRSIZE_STATFS)
# define _mem_f_frsize_statfs 1
#endif
#if defined (MEM_F_FSTYPENAME_STATFS)
# define _mem_f_fstypename_statfs 1
#endif
#if defined (MEM_F_IOSIZE_STATFS)
# define _mem_f_iosize_statfs 1
#endif
#if defined (MEM_MOUNT_INFO_STATFS)
# define _mem_mount_info_statfs 1
#endif
#if defined (MEM_MNT_TIME_MNTTAB)
# define _mem_mnt_time_mnttab 1
#endif
#if defined (MEM_VMT_TIME_VMOUNT)
# define _mem_vmt_time_vmount 1
#endif
#if defined (NPT_GETENV)
# define _npt_getenv 1
#endif
#if defined (NPT_STATFS)
# define _npt_statfs 1
#endif
#if defined (SIZ_LONG_LONG)
# define _siz_long_long SIZ_LONG_LONG
#endif

#if ! defined (_config_by_iffe_)   /* are we using configure? */
# if _lib_bindtextdomain && \
	_lib_gettext && \
	_lib_setlocale && \
	_lib_textdomain && \
	_hdr_libintl && \
	_hdr_locale
#  define _enable_nls 1
# else
#  define _enable_nls 0
# endif
#endif

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

# if ! defined (_lib_getopt)
extern int getopt _((int argc, char *argv [], char *optstring));
# endif

#endif /* __INC_DI_H_ */
