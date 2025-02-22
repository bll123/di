/*
 * Copyright 2011-2018 Brad Lanam, Walnut Creek, CA
 * Copyright 2023-2025 Brad Lanam, Pleasant Hill, CA
 */

#ifndef INC_DIMNTOPT_H
#define INC_DIMNTOPT_H

#include "config.h"

#if _hdr_mntent \
  && ! defined (DI_INC_MNTENT)        /* Linux, kFreeBSD, HP-UX */
# define DI_INC_MNTENT 1
# include <mntent.h>                /* MNTOPT_... */
#endif
/* FreeBSD, OpenBSD, NetBSD, HP-UX, MacOS */
#if _sys_mount && ! defined (DI_INC_SYS_MOUNT)
# define DI_INC_SYS_MOUNT 1
# include <sys/mount.h>             /* MNT_...; M_... (hp-ux) */
#endif
#if _sys_mnttab                     /* SCO_SV, UnixWare */
# include <sys/mnttab.h>            /* required for mntent.h */
#endif
#if _sys_mntent                     /* Solaris, SCO_SV, UnixWare */
# include <sys/mntent.h>            /* MNTOPT_... */
#endif
#if _sys_fstypes                    /* NetBSD */
# include <sys/fstypes.h>
#endif
#if _sys_vmount                     /* AIX */
# include <sys/vmount.h>            /* MNT_... */
#endif
#if _hdr_mnttab                     /* SysV.3 */
# include <mnttab.h>
#endif

# if defined (__cplusplus) || defined (c_plusplus)
extern "C" {
# endif

/********************************************************/

    /* remap mount flags */
#if defined (B_FS_IS_READONLY)
# define MNT_RDONLY B_FS_IS_READONLY
#endif
#if defined (FS_IS_READONLY)
# define MNT_RDONLY FS_IS_READONLY
#endif
#if defined (M_RDONLY)
# define MNT_RDONLY M_RDONLY
#endif
#if defined (MNT_READONLY)
# define MNT_RDONLY MNT_READONLY
#endif
#if defined (M_RONLY)
# define MNT_RDONLY M_RONLY
#endif
#if defined (M_SYNCHRONOUS)
# define MNT_SYNCHRONOUS M_SYNCHRONOUS
#endif
#if defined (M_NOEXEC)
# define MNT_NOEXEC M_NOEXEC
#endif
#if defined (M_NOSUID)
# define MNT_NOSUID M_NOSUID
#endif
#if defined (M_NODEV)
# define MNT_NODEV M_NODEV
#endif
#if defined (M_NOATIMES)
# define MNT_NOATIMES M_NOATIMES
#endif
#if defined (M_GRPID)
# define MNT_GRPID M_GRPID
#endif
#if defined (M_SECURE)
# define MNT_SECURE M_SECURE
#endif
#if defined (M_MLSD)
# define MNT_MLSD M_MLSD
#endif
#if defined (M_SMSYNC2)
# define MNT_SMSYNC2 M_SMSYNC2
#endif
#if defined (M_LOCAL)
# define MNT_LOCAL M_LOCAL
#endif
#if defined (M_FORCE)
# define MNT_FORCE M_FORCE
#endif
#if defined (M_SYNC)
# define MNT_SYNC M_SYNC
#endif
#if defined (M_NOCACHE)
# define MNT_NOCACHE M_NOCACHE
#endif
#if defined (B_FS_IS_REMOVABLE)
# define MNT_REMOVABLE B_FS_IS_REMOVABLE
#endif
#if defined (FS_IS_REMOVABLE)
# define MNT_REMOVABLE FS_IS_REMOVABLE
#endif
#if defined (B_FS_IS_PERSISTENT)
# define MNT_PERSISTENT B_FS_IS_PERSISTENT
#endif
#if defined (FS_IS_PERSISTENT)
# define MNT_PERSISTENT FS_IS_PERSISTENT
#endif
#if defined (B_FS_IS_SHARED)
# define MNT_SHARED B_FS_IS_SHARED
#endif
#if defined (FS_IS_SHARED)
# define MNT_SHARED FS_IS_SHARED
#endif
#if defined (FS_IS_BLOCKBASED)
# define MNT_BLOCKBASED FS_IS_BLOCKBASED
#endif
#if defined (B_FS_HAS_MIME)
# define MNT_HAS_MIME B_FS_HAS_MIME
#endif
#if defined (FS_HAS_MIME)
# define MNT_HAS_MIME FS_HAS_MIME
#endif
#if defined (B_FS_HAS_ATTR)
# define MNT_HAS_ATTR B_FS_HAS_ATTR
#endif
#if defined (FS_HAS_ATTR)
# define MNT_HAS_ATTR FS_HAS_ATTR
#endif
#if defined (B_FS_HAS_QUERY)
# define MNT_HAS_QUERY B_FS_HAS_QUERY
#endif
#if defined (FS_HAS_QUERY)
# define MNT_HAS_QUERY FS_HAS_QUERY
#endif

#if defined (MNTOPT_IGNORE)
# define DI_MNTOPT_IGNORE MNTOPT_IGNORE
#else
# define DI_MNTOPT_IGNORE "ignore"
#endif

#if defined (MNTOPT_RO)
# define DI_MNTOPT_RO MNTOPT_RO
#else
# define DI_MNTOPT_RO "ro"
#endif

#if defined (MNTOPT_DEV)
# define DI_MNTOPT_DEV MNTOPT_DEV
#else
# define DI_MNTOPT_DEV "dev="
#endif

# if defined (__cplusplus) || defined (c_plusplus)
}
# endif

#endif /* INC_DIMNTOPT_H */
