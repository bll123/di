/*
 * $Id$
 * $Source$
 * Copyright 1994-2009 Brad Lanam, Walnut Creek, CA
 */

#include "config.h"
#include "di.h"
#include "dimntopt.h"

#include <stdio.h>
#if _hdr_ctype
# include <ctype.h>
#endif
#if _hdr_errno
# include <errno.h>
#endif
#if _hdr_stdlib
# include <stdlib.h>
#endif
#if _sys_types
# include <sys/types.h>
#endif
#if _hdr_string
# include <string.h>
#endif
#if _hdr_strings && ((! defined (_hdr_string)) || (_include_string))
# include <strings.h>
#endif
#if _hdr_memory
# include <memory.h>
#endif
#if _include_malloc && _hdr_malloc
# include <malloc.h>
#endif
#if _hdr_unistd
# include <unistd.h>
#endif
#if _hdr_time
# include <time.h>
#endif
#if _sys_time
# include <sys/time.h>
#endif
#if _sys_stat
# include <sys/stat.h>
#endif
#if _sys_param
# include <sys/param.h>
#endif

/********************************************************/
/*
    This module contains utility routines for conversion
    and checking the data.

    convertMountOptions ()
        converts mount options to text format.
    convertNFSMountOptions ()
        converts NFS mount options to text format.
    chkMountOptions ()
        Checks to see if the mount option is set.
        Used if hasmntopt() is not present.
    di_testRemoteDisk ()
        test a disk to see if it is remote (nfs, nfs3).

*/

void
#if _proto_stdc
convertMountOptions (long flags, diDiskInfo_t *diptr)
#else
convertMountOptions (flags, diptr)
    long          flags;
    diDiskInfo_t   *diptr;
#endif
{
#if defined (MNT_RDONLY)
    if ((flags & MNT_RDONLY) == MNT_RDONLY)
    {
        strncat (diptr->options, "ro,",
                DI_OPT_LEN - strlen (diptr->options) - 1);
    }
    else
    {
        strncat (diptr->options, "rw,",
                DI_OPT_LEN - strlen (diptr->options) - 1);
    }
#endif
#if defined (MNT_FORCE)
    if ((flags & MNT_FORCE) == MNT_FORCE)
    {
        strncat (diptr->options, "force,",
                DI_OPT_LEN - strlen (diptr->options) - 1);
    }
#endif
#if defined (MNT_GRPID)
    if ((flags & MNT_GRPID) == MNT_GRPID)
    {
        strncat (diptr->options, "grpid,",
                DI_OPT_LEN - strlen (diptr->options) - 1);
    }
#endif
#if defined (MNT_MAGICLINKS)
    if ((flags & MNT_MAGICLINKS) == MNT_MAGICLINKS)
    {
        strncat (diptr->options, "magiclinks,",
                DI_OPT_LEN - strlen (diptr->options) - 1);
    }
#endif
#if defined (MNT_MLSD)
    if ((flags & MNT_MLSD) == MNT_MLSD)
    {
        strncat (diptr->options, "mlsd,",
                DI_OPT_LEN - strlen (diptr->options) - 1);
    }
#endif
#if defined (MNT_NOATIMES)
    if ((flags & MNT_NOATIMES) == MNT_NOATIMES)
    {
        strncat (diptr->options, "noatime,",
                DI_OPT_LEN - strlen (diptr->options) - 1);
    }
#endif
#if defined (MNT_NOCACHE)
    if ((flags & MNT_NOCACHE) == MNT_NOCACHE)
    {
        strncat (diptr->options, "nocache,",
                DI_OPT_LEN - strlen (diptr->options) - 1);
    }
#endif
#if defined (MNT_NOCOREDUMP)
    if ((flags & MNT_NOCOREDUMP) == MNT_NOCOREDUMP)
    {
        strncat (diptr->options, "nocoredump,",
                DI_OPT_LEN - strlen (diptr->options) - 1);
    }
#endif
#if defined (MNT_NODEV)
    if ((flags & MNT_NODEV) == MNT_NODEV)
    {
        strncat (diptr->options, "nodev,",
                DI_OPT_LEN - strlen (diptr->options) - 1);
    }
#endif
#if defined (MNT_NODEVMTIME)
    if ((flags & MNT_NODEVMTIME) == MNT_NODEVMTIME)
    {
        strncat (diptr->options, "nodevmtime,",
                DI_OPT_LEN - strlen (diptr->options) - 1);
    }
#endif
#if defined (MNT_NOEXEC)
    if ((flags & MNT_NOEXEC) == MNT_NOEXEC)
    {
        strncat (diptr->options, "noexec,",
                DI_OPT_LEN - strlen (diptr->options) - 1);
    }
#endif
#if defined (MNT_NOSUID)
    if ((flags & MNT_NOSUID) == MNT_NOSUID)
    {
        strncat (diptr->options, "nosuid,",
                DI_OPT_LEN - strlen (diptr->options) - 1);
    }
#endif
#if defined (MNT_QUOTA)
    if ((flags & MNT_QUOTA) == MNT_QUOTA)
    {
        strncat (diptr->options, "quota,",
                DI_OPT_LEN - strlen (diptr->options) - 1);
    }
#endif
#if defined (MNT_SECURE)
    if ((flags & MNT_SECURE) == MNT_SECURE)
    {
        strncat (diptr->options, "secure,",
                DI_OPT_LEN - strlen (diptr->options) - 1);
    }
#endif
#if defined (MNT_SMSYNC2)
    if ((flags & MNT_SMSYNC2) == MNT_SMSYNC2)
    {
        strncat (diptr->options, "smsync2,",
                DI_OPT_LEN - strlen (diptr->options) - 1);
    }
#endif
#if defined (MNT_SOFTDEP)
    if ((flags & MNT_SOFTDEP) == MNT_SOFTDEP)
    {
        strncat (diptr->options, "softdep,",
                DI_OPT_LEN - strlen (diptr->options) - 1);
    }
#endif
#if defined (MNT_SYMPERM)
    if ((flags & MNT_SYMPERM) == MNT_SYMPERM)
    {
        strncat (diptr->options, "symperm,",
                DI_OPT_LEN - strlen (diptr->options) - 1);
    }
#endif
#if defined (MNT_SYNC)
    if ((flags & MNT_SYNC) == MNT_SYNC)
    {
        strncat (diptr->options, "sync,",
                DI_OPT_LEN - strlen (diptr->options) - 1);
    }
#endif
#if defined (MNT_SYNCHRONOUS)
    if ((flags & MNT_SYNCHRONOUS) == MNT_SYNCHRONOUS)
    {
        strncat (diptr->options, "sync,",
                DI_OPT_LEN - strlen (diptr->options) - 1);
    }
#endif
#if defined (MNT_THROTTLE)
    if ((flags & MNT_THROTTLE) == MNT_THROTTLE)
    {
        strncat (diptr->options, "throttle,",
                DI_OPT_LEN - strlen (diptr->options) - 1);
    }
#endif
#if defined (MNT_UNION)
    if ((flags & MNT_UNION) == MNT_UNION)
    {
        strncat (diptr->options, "union,",
                DI_OPT_LEN - strlen (diptr->options) - 1);
    }
#endif
#if defined (MNT_UNION)
    if ((flags & MNT_UNION) == MNT_UNION)
    {
        strncat (diptr->options, "union,",
                DI_OPT_LEN - strlen (diptr->options) - 1);
    }
#endif
#if defined (MNT_REMOVABLE)
    if ((flags & MNT_REMOVABLE) == MNT_REMOVABLE)
    {
        strncat (diptr->options, "removable,",
                DI_OPT_LEN - strlen (diptr->options) - 1);
    }
#endif
#if defined (MNT_PERSISTENT)
    if ((flags & MNT_PERSISTENT) == MNT_PERSISTENT)
    {
        strncat (diptr->options, "persistent,",
                DI_OPT_LEN - strlen (diptr->options) - 1);
    }
#endif
#if defined (MNT_SHARED)
    if ((flags & MNT_SHARED) == MNT_SHARED)
    {
        strncat (diptr->options, "shared,",
                DI_OPT_LEN - strlen (diptr->options) - 1);
    }
#endif
#if defined (MNT_BLOCKBASED)
    if ((flags & MNT_BLOCKBASED) == MNT_BLOCKBASED)
    {
        strncat (diptr->options, "blockbased,",
                DI_OPT_LEN - strlen (diptr->options) - 1);
    }
#endif
#if defined (MNT_HAS_MIME)
    if ((flags & MNT_HAS_MIME) == MNT_HAS_MIME)
    {
        strncat (diptr->options, "mime,",
                DI_OPT_LEN - strlen (diptr->options) - 1);
    }
#endif
#if defined (MNT_HAS_QUERY)
    if ((flags & MNT_HAS_QUERY) == MNT_HAS_QUERY)
    {
        strncat (diptr->options, "query,",
                DI_OPT_LEN - strlen (diptr->options) - 1);
    }
#endif
#if defined (MNT_HAS_ATTR)
    if ((flags & MNT_HAS_ATTR) == MNT_HAS_ATTR)
    {
        strncat (diptr->options, "attr,",
                DI_OPT_LEN - strlen (diptr->options) - 1);
    }
#endif
    return;
}

void
#if _proto_stdc
convertNFSMountOptions (long flags, long wsize, long rsize, diDiskInfo_t *diptr)
#else
convertNFSMountOptions (flags, wsize, rsize, diptr)
    long          flags;
    long          wsize;
    long          rsize;
    diDiskInfo_t   *diptr;
#endif
{
#if defined (NFSMNT_SOFT)
    if ((flags & NFSMNT_SOFT) != NFSMNT_SOFT)
    {
        strncat (diptr->options, "hard,",
                DI_OPT_LEN - strlen (diptr->options) - 1);
    }
#endif
#if defined (NFSMNT_WSIZE)
    if ((flags & NFSMNT_WSIZE) == NFSMNT_WSIZE)
    {
        char          tmp [64];

        Snprintf (tmp, DI_SPF(sizeof (tmp), "wsize=%ld,"), wsize);
        strncat (diptr->options, tmp,
                DI_OPT_LEN - strlen (diptr->options) - 1);
    }
#endif
#if defined (NFSMNT_RSIZE)
    if ((flags & NFSMNT_RSIZE) == NFSMNT_RSIZE)
    {
        char          tmp [64];

        Snprintf (tmp, DI_SPF(sizeof (tmp), "rsize=%ld,"), rsize);
        strncat (diptr->options, tmp,
                DI_OPT_LEN - strlen (diptr->options) - 1);
    }
#endif
#if defined (NFSMNT_INT) && defined (NFSMNT_SOFT)
    if ((flags & NFSMNT_SOFT) != NFSMNT_SOFT &&
        (flags & NFSMNT_INT) == NFSMNT_INT)
    {
        strncat (diptr->options, "intr,",
                DI_OPT_LEN - strlen (diptr->options) - 1);
    }
#endif
#if defined (NFSMNT_TCP)
    if ((flags & NFSMNT_TCP) != NFSMNT_TCP)
    {
        strncat (diptr->options, "udp,",
                DI_OPT_LEN - strlen (diptr->options) - 1);
    }
#endif
    return;
}


#if _lib_getmntent && \
    ! defined (_lib_getmntinfo) && \
    ! defined (_lib_getfsstat) && \
    ! defined (_lib_getvfsstat) && \
	! defined (_lib_mntctl) && \
	! defined (_class_os__Volumes)

extern char *
# if _proto_stdc
chkMountOptions (char *mntopts, char *str)
# else
chkMountOptions (mntopts, str)
    char          *mntopts;
    char          *str;
# endif
{
    char    *ptr;
    char    *tstr;

    tstr = strdup (mntopts);
    ptr = strtok (tstr, ",");
    while (ptr != (char *) NULL)
    {
        if (strcmp (ptr, str) == 0)
        {
            free (tstr);
            return ptr;
        }
        ptr = strtok ((char *) NULL, ",");
    }
    free (tstr);
    return (char *) NULL;
}

#endif /* _lib_getmntent */

void
# if _proto_stdc
di_testRemoteDisk (diDiskInfo_t *diskInfo)
# else
di_testRemoteDisk (diskInfo)
    diDiskInfo_t *diskInfo;
# endif
{
    if (strcmp (diskInfo->fsType, "nfs") == 0 ||
            strcmp (diskInfo->fsType, "nfs3") == 0)
    {
        diskInfo->isLocal = FALSE;
    }
}

