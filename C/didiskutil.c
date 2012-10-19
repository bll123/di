/*
 * $Id$
 * $Source$
 * Copyright 1994-2011 Brad Lanam, Walnut Creek, CA
 */

#include "config.h"
#include "di.h"
#include "dimntopt.h"

#if _hdr_stdio
# include <stdio.h>
#endif
#if _hdr_stdlib
# include <stdlib.h>
#endif
#if _hdr_string
# include <string.h>
#endif
#if _hdr_strings
# include <strings.h>
#endif
#if _hdr_memory
# include <memory.h>
#endif
#if _hdr_malloc
# include <malloc.h>
#endif
#if _hdr_errno
# include <errno.h>
#endif


/********************************************************/
/*
    This module contains utility routines for conversion
    and checking the data.

    di_initDiskInfo ()
        initialize disk info structure
    di_saveBlockSizes ()
        save the block sizes in the diskinfo structure.
    di_saveInodeSizes ()
        save the inode sizes in the diskinfo structure.
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
di_initDiskInfo (diDiskInfo_t *diptr)
#else
di_initDiskInfo (diptr)
    diDiskInfo_t        *diptr;
#endif
{
    memset ((char *) diptr, '\0', sizeof (diDiskInfo_t));
    diptr->printFlag = DI_PRNT_OK;
    diptr->isLocal = TRUE;
    diptr->isReadOnly = FALSE;
    diptr->isLoopback = FALSE;
}

void
#if _proto_stdc
di_saveBlockSizes (diDiskInfo_t *diptr, _fs_size_t block_size,
        _fs_size_t total_blocks, _fs_size_t free_blocks,
        _fs_size_t avail_blocks)
#else
di_saveBlockSizes (diptr, block_size, total_blocks, free_blocks, avail_blocks)
    diDiskInfo_t *diptr;
    _fs_size_t block_size;
    _fs_size_t total_blocks;
    _fs_size_t free_blocks;
    _fs_size_t avail_blocks;
#endif
{
    diptr->blockSize = (_fs_size_t) block_size;
    diptr->totalBlocks = (_fs_size_t) total_blocks;
    diptr->freeBlocks = (_fs_size_t) free_blocks;
    diptr->availBlocks = (_fs_size_t) avail_blocks;
}

void
#if _proto_stdc
di_saveInodeSizes (diDiskInfo_t *diptr,
        _fs_size_t total_nodes, _fs_size_t free_nodes,
        _fs_size_t avail_nodes)
#else
di_saveInodeSizes (diptr, total_nodes, free_nodes, avail_nodes)
    diDiskInfo_t *diptr;
    _fs_size_t total_nodes;
    _fs_size_t free_nodes;
    _fs_size_t avail_nodes;
#endif
{
    diptr->totalInodes = total_nodes;
    diptr->freeInodes = free_nodes;
    diptr->availInodes = avail_nodes;
}

void
#if _proto_stdc
convertMountOptions (unsigned long flags, diDiskInfo_t *diptr)
#else
convertMountOptions (flags, diptr)
    unsigned long  flags;
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
#if defined (MNT_EXRDONLY)
    if ((flags & MNT_EXRDONLY) == MNT_EXRDONLY)
    {
        strncat (diptr->options, "expro,",
                DI_OPT_LEN - strlen (diptr->options) - 1);
    }
#endif
#if defined (MNT_DEFEXPORTED)
    if ((flags & MNT_DEFEXPORTED) == MNT_DEFEXPORTED)
    {
        strncat (diptr->options, "exprwany,",
                DI_OPT_LEN - strlen (diptr->options) - 1);
    }
#endif
#if defined (MNT_EXPORTANON)
    if ((flags & MNT_EXPORTANON) == MNT_EXPORTANON)
    {
        strncat (diptr->options, "expanon,",
                DI_OPT_LEN - strlen (diptr->options) - 1);
    }
#endif
#if defined (MNT_EXKERB)
    if ((flags & MNT_EXKERB) == MNT_EXKERB)
    {
        strncat (diptr->options, "expkerb,",
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

        Snprintf1 (tmp, sizeof (tmp), "wsize=%ld,", wsize);
        strncat (diptr->options, tmp,
                DI_OPT_LEN - strlen (diptr->options) - 1);
    }
#endif
#if defined (NFSMNT_RSIZE)
    if ((flags & NFSMNT_RSIZE) == NFSMNT_RSIZE)
    {
        char          tmp [64];

        Snprintf1 (tmp, sizeof (tmp), "rsize=%ld,", rsize);
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
    ! _lib_getmntinfo && \
    ! _lib_getfsstat && \
    ! _lib_getvfsstat && \
	! _lib_mntctl && \
	! _class_os__Volumes

char *
# if _proto_stdc
chkMountOptions (const char *mntopts, const char *str)
# else
chkMountOptions (mntopts, str)
    const char          *mntopts;
    const char          *str;
# endif
{
    char    *ptr;
    char    *tstr;

    tstr = strdup (mntopts);
    if (tstr == (char *) NULL)
    {
        fprintf (stderr, "strdup failed in chkMountOptions (1).  errno %d\n", errno);
        exit (1);
    }
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

