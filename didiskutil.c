/*
 * Copyright 1994-2018 Brad Lanam, Walnut Creek, CA
 * Copyright 2023-2025 Brad Lanam, Pleasant Hill, CA
 */

#include "config.h"

#if _hdr_stdio
# include <stdio.h>
#endif
#if _hdr_stdlib
# include <stdlib.h>
#endif
#if _hdr_stdbool
# include <stdbool.h>
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

#include "di.h"
#include "strutils.h"
#include "dimath.h"
#include "dimntopt.h"

/********************************************************/
/*
    This module contains utility routines for conversion
    and checking the data.

    di_init_disk_info ()
        initialize disk info structure
    di_save_block_sizes ()
        save the block sizes in the diskinfo structure.
    di_save_inode_sizes ()
        save the inode sizes in the diskinfo structure.
    convertMountOptions ()
        converts mount options to text format.
    convertNFSMountOptions ()
        converts NFS mount options to text format.
    chkMountOptions ()
        Checks to see if the mount option is set.
        Used if hasmntopt() is not present.
    di_is_remote_disk ()
        test a disk to see if it is remote (nfs, nfs3).

*/

void
di_init_disk_info (di_disk_info_t *diptr)
{
  int     i;

  memset ((char *) diptr, '\0', sizeof (di_disk_info_t));
  for (i = 0; i < DI_VALUE_MAX; ++i) {
    dinum_init (&diptr->values [i]);
  }
  diptr->printFlag = DI_PRNT_OK;
  diptr->isLocal = true;
  diptr->isReadOnly = false;
  diptr->isLoopback = false;
}

void
di_save_block_sizes (di_disk_info_t *diptr, di_unum_t block_size,
    di_unum_t total_blocks, di_unum_t free_blocks, di_unum_t avail_blocks)
{
  dinum_mul_uu (&diptr->values [DI_SPACE_TOTAL], total_blocks, block_size);
  dinum_mul_uu (&diptr->values [DI_SPACE_FREE], free_blocks, block_size);
  dinum_mul_uu (&diptr->values [DI_SPACE_AVAIL], avail_blocks, block_size);
}

void
di_save_inode_sizes (di_disk_info_t *diptr, di_unum_t total_nodes,
    di_unum_t free_nodes, di_unum_t avail_nodes)
{
  dinum_set_u (&diptr->values [DI_INODE_TOTAL], total_nodes);
  dinum_set_u (&diptr->values [DI_INODE_FREE], free_nodes);
  dinum_set_u (&diptr->values [DI_INODE_AVAIL], avail_nodes);
}

void
convertMountOptions (unsigned long flags, di_disk_info_t *diptr)
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
convertNFSMountOptions (long flags, long wsize, long rsize, di_disk_info_t *diptr)
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


#if _lib_getmntent \
    && ! _lib_getmntinfo \
    && ! _lib_getfsstat \
    && ! _lib_getvfsstat \
	&& ! _lib_mntctl \
	&& ! _class_os__Volumes

char *
chkMountOptions (const char *mntopts, const char *str)
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
di_is_remote_disk (di_disk_info_t *diskInfo)
{
  if (strncmp (diskInfo->fsType, "nfs", 3) == 0)
  {
    diskInfo->isLocal = false;
  }
}

int
di_isPooledFs (di_disk_info_t *diskInfo)
{
  if (strcmp (diskInfo->fsType, "zfs") == 0 ||
      strcmp (diskInfo->fsType, "advfs") == 0 ||
      strcmp (diskInfo->fsType, "apfs") == 0 ||
      (strcmp (diskInfo->fsType, "null") == 0 &&
       strstr (diskInfo->special, "/@@-") != (char *) NULL)) {
    return true;
  }
  return false;
}

int
di_isLoopbackFs (di_disk_info_t *diskInfo)
{
  if ((strcmp (diskInfo->fsType, "lofs") == 0 && diskInfo->sp_rdev != 0) ||
      (strcmp (diskInfo->fsType, "nullfs") == 0 &&
       strstr (diskInfo->special, "/@@-") == (char *) NULL) ||
      strcmp (diskInfo->fsType, "none") == 0) {
    return true;
  }
  return false;
}

Size_t
di_mungePoolName (char *poolname)
{
  char      *ptr;

  ptr = strchr (poolname, '#');   /* advfs */
  if (ptr != (char *) NULL) {
    *ptr = '\0';
  } else {
    ptr = strchr (poolname, ':');   /* dragonflybsd */
    if (ptr != (char *) NULL) {
      *ptr = '\0';
    } else {
      if (strncmp (poolname, "/dev/disk", 9) == 0) {
        /* apfs uses a standard /dev/diskNsN format */
        ptr = strchr (poolname, 's');
        if (ptr != (char *) NULL) {
          ++ptr;
          if (ptr != (char *) NULL) {
            ptr = strchr (ptr, 's');
            if (ptr != (char *) NULL) {
              *ptr = '\0';
            }
          }
        }
      } else {
        ptr = strchr (poolname, '/');   /* zfs */
        if (ptr != (char *) NULL) {
          *ptr = '\0';
        }
      }
    }
  }
  return strlen (poolname);
}

