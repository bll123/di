/* Copyright 2025 Brad Lanam Pleasant Hill CA */

#ifndef INC_DISYSTEM_H
#define INC_DISYSTEM_H

#include "config.h"

/* first some system stuff to get various sizes */
#if _hdr_stddef
# include <stddef.h>
#endif
#if _hdr_stdint
# include <stdint.h>
#endif
#if _hdr_string
# include <string.h>
#endif
#if _hdr_strings
# include <strings.h>
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
# define MAXPATHLEN         1024
#endif

#if _sys_fstyp                          /* HP-UX, Solaris */
# include <sys/fstyp.h>                 /* FSTYPSZ */
# if defined (FSTYPSZ)
#  define DI_FSTYPE_LEN       FSTYPSZ
# endif
#endif
/* FreeBSD, OpenBSD, NetBSD, HP-UX, MacOS */
#if _sys_mount && ! defined (DI_INC_SYS_MOUNT)
# define DI_INC_SYS_MOUNT 1
# include <sys/mount.h>                 /* MFSNAMELEN */
#endif
#if _sys_mount
# if ! defined (DI_FSTYPE_LEN) && defined (MFSNAMELEN)
#  define DI_FSTYPE_LEN       MFSNAMELEN
# endif
#endif
#if _sys_vfstab                         /* FSTYPSZ sco open server */
# include <stdio.h>
# include <sys/vfstab.h>
# if ! defined (DI_FSTYPE_LEN) && defined (FSTYPSZ)
#  define DI_FSTYPE_LEN       FSTYPSZ
# endif
#endif

#if ! defined (DI_FSTYPE_LEN)
# define DI_FSTYPE_LEN        65
#endif

#if ! _lib_memcpy && ! defined (memcpy)
# if ! _lib_bcopy && ! defined (bcopy)
#  error No_memcpy/bcopy_available.
# else
#  define memcpy (dst, src, cnt) (bcopy ( (src), (dst), (cnt)), dst)
# endif
#endif

#if ! _lib_memset && ! _define_memset
# if ! _lib_bzero && ! _define_bzero
   #error No_memset/bzero_available.
# else
#  define memset (s,c,n)    (bzero ( (s), (n)), s)
# endif
#endif

#if ! _hdr_stdbool
# ifndef false
#  define false 0
# endif
# ifndef true
#  define true 1
# endif
#endif

#endif /* INC_DISYSTEM_H */
