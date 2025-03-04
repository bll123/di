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
#include "disystem.h"
#include "diinternal.h"
#include "distrutils.h"
#include "dimath.h"
#include "dimntopt.h"

/*
 *    This module contains utility routines for conversion
 *    and checking the data.
 *
 *    di_initialize_disk_info ()
 *        initialize disk info structure
 *    di_save_block_sizes ()
 *        save the block sizes in the diskinfo structure.
 *    di_save_inode_sizes ()
 *        save the inode sizes in the diskinfo structure.
 *    convertMountOptions ()
 *        converts mount options to text format.
 *    convertNFSMountOptions ()
 *        converts NFS mount options to text format.
 *    chkMountOptions ()
 *        Checks to see if the mount option is set.
 *        Used if hasmntopt () is not present.
 *    di_is_remote_disk ()
 *        test a disk to see if it is remote (nfs, nfs3).
 *
 */

void
di_initialize_disk_info (di_disk_info_t *diptr, int idx)
{
  int     i;

  memset ( (char *) diptr, '\0', sizeof (di_disk_info_t));
  diptr->sortIndex [DI_SORT_MAIN] = idx;
  diptr->sortIndex [DI_SORT_TOTAL] = idx;
  for (i = 0; i < DI_VALUE_MAX; ++i) {
    dinum_init (&diptr->values [i]);
  }
  diptr->doPrint = 0;
  diptr->printFlag = DI_PRNT_OK;
  diptr->isLocal = true;
  diptr->isReadOnly = false;
  diptr->isLoopback = false;
  diptr->strdata [DI_DISP_MOUNTPT] = (char *) malloc (DI_MOUNTPT_LEN);
  diptr->strdata [DI_DISP_FILESYSTEM] = (char *) malloc (DI_FILESYSTEM_LEN);
  diptr->strdata [DI_DISP_MOUNTOPT] = (char *) malloc (DI_MOUNTOPT_LEN);
  diptr->strdata [DI_DISP_FSTYPE] = (char *) malloc (DI_FSTYPE_LEN);
  for (i = 0; i < DI_DISP_MAX; ++i) {
    diptr->strdata [i][0] = '\0';
  }
}

void
di_free_disk_info (di_disk_info_t *diptr)
{
  int     i;

  if (diptr == NULL) {
    return;
  }

  for (i = 0; i < DI_VALUE_MAX; ++i) {
    dinum_clear (&diptr->values [i]);
  }
  if (diptr->strdata [DI_DISP_MOUNTPT] != NULL) {
    free (diptr->strdata [DI_DISP_MOUNTPT]);
  }
  if (diptr->strdata [DI_DISP_FILESYSTEM] != NULL) {
    free (diptr->strdata [DI_DISP_FILESYSTEM]);
  }
  if (diptr->strdata [DI_DISP_FSTYPE] != NULL) {
    free (diptr->strdata [DI_DISP_FSTYPE]);
  }
  if (diptr->strdata [DI_DISP_MOUNTOPT] != NULL) {
    free (diptr->strdata [DI_DISP_MOUNTOPT]);
  }
}

void
di_save_block_sizes (di_disk_info_t *diptr, di_ui_t block_size,
    di_ui_t total_blocks, di_ui_t free_blocks, di_ui_t avail_blocks)
{
  dinum_mul_uu (&diptr->values [DI_SPACE_TOTAL], total_blocks, block_size);
  dinum_mul_uu (&diptr->values [DI_SPACE_FREE], free_blocks, block_size);
  dinum_mul_uu (&diptr->values [DI_SPACE_AVAIL], avail_blocks, block_size);
}

void
di_save_inode_sizes (di_disk_info_t *diptr, di_ui_t total_nodes,
    di_ui_t free_nodes, di_ui_t avail_nodes)
{
  dinum_set_u (&diptr->values [DI_INODE_TOTAL], total_nodes);
  dinum_set_u (&diptr->values [DI_INODE_FREE], free_nodes);
  dinum_set_u (&diptr->values [DI_INODE_AVAIL], avail_nodes);
}

void
convertMountOptions (unsigned long flags, di_disk_info_t *diptr)
{
  char    *p;
  char    *end;

  p = diptr->strdata [DI_DISP_MOUNTOPT];
  end = diptr->strdata [DI_DISP_MOUNTOPT] + DI_MOUNTOPT_LEN;
#if defined (MNT_RDONLY)
  if ((flags & MNT_RDONLY) == MNT_RDONLY) {
    p = stpecpy (p, end, "ro,");
  } else {
    p = stpecpy (p, end, "rw,");
  }
#endif
#if defined (MNT_EXRDONLY)
  if ((flags & MNT_EXRDONLY) == MNT_EXRDONLY) {
    p = stpecpy (p, end, "expro,");
  }
#endif
#if defined (MNT_DEFEXPORTED)
  if ((flags & MNT_DEFEXPORTED) == MNT_DEFEXPORTED) {
    p = stpecpy (p, end, "exprwany,");
  }
#endif
#if defined (MNT_EXPORTANON)
  if ((flags & MNT_EXPORTANON) == MNT_EXPORTANON) {
    p = stpecpy (p, end, "expanon,");
  }
#endif
#if defined (MNT_EXKERB)
  if ((flags & MNT_EXKERB) == MNT_EXKERB) {
    p = stpecpy (p, end, "expkerb,");
  }
#endif
#if defined (MNT_FORCE)
  if ((flags & MNT_FORCE) == MNT_FORCE) {
    p = stpecpy (p, end, "force,");
  }
#endif
#if defined (MNT_GRPID)
  if ((flags & MNT_GRPID) == MNT_GRPID) {
    p = stpecpy (p, end, "grpid,");
  }
#endif
#if defined (MNT_MAGICLINKS)
  if ((flags & MNT_MAGICLINKS) == MNT_MAGICLINKS) {
    p = stpecpy (p, end, "magiclinks,");
  }
#endif
#if defined (MNT_MLSD)
  if ((flags & MNT_MLSD) == MNT_MLSD) {
    p = stpecpy (p, end, "mlsd,");
  }
#endif
#if defined (MNT_NOATIMES)
  if ((flags & MNT_NOATIMES) == MNT_NOATIMES) {
    p = stpecpy (p, end, "noatime,");
  }
#endif
#if defined (MNT_NOCACHE)
  if ((flags & MNT_NOCACHE) == MNT_NOCACHE) {
    p = stpecpy (p, end, "nocache,");
  }
#endif
#if defined (MNT_NOCOREDUMP)
  if ((flags & MNT_NOCOREDUMP) == MNT_NOCOREDUMP) {
    p = stpecpy (p, end, "nocoredump,");
  }
#endif
#if defined (MNT_NODEV)
  if ((flags & MNT_NODEV) == MNT_NODEV) {
    p = stpecpy (p, end, "nodev,");
  }
#endif
#if defined (MNT_NODEVMTIME)
  if ((flags & MNT_NODEVMTIME) == MNT_NODEVMTIME) {
    p = stpecpy (p, end, "nodevmtime,");
  }
#endif
#if defined (MNT_NOEXEC)
  if ((flags & MNT_NOEXEC) == MNT_NOEXEC) {
    p = stpecpy (p, end, "noexec,");
  }
#endif
#if defined (MNT_NOSUID)
  if ((flags & MNT_NOSUID) == MNT_NOSUID) {
    p = stpecpy (p, end, "nosuid,");
  }
#endif
#if defined (MNT_QUOTA)
  if ((flags & MNT_QUOTA) == MNT_QUOTA) {
    p = stpecpy (p, end, "quota,");
  }
#endif
#if defined (MNT_SECURE)
  if ((flags & MNT_SECURE) == MNT_SECURE) {
    p = stpecpy (p, end, "secure,");
  }
#endif
#if defined (MNT_SMSYNC2)
  if ((flags & MNT_SMSYNC2) == MNT_SMSYNC2) {
    p = stpecpy (p, end, "smsync2,");
  }
#endif
#if defined (MNT_SOFTDEP)
  if ((flags & MNT_SOFTDEP) == MNT_SOFTDEP) {
    p = stpecpy (p, end, "softdep,");
  }
#endif
#if defined (MNT_SYMPERM)
  if ((flags & MNT_SYMPERM) == MNT_SYMPERM) {
    p = stpecpy (p, end, "symperm,");
  }
#endif
#if defined (MNT_SYNC)
  if ((flags & MNT_SYNC) == MNT_SYNC) {
    p = stpecpy (p, end, "sync,");
  }
#endif
#if defined (MNT_SYNCHRONOUS)
  if ((flags & MNT_SYNCHRONOUS) == MNT_SYNCHRONOUS) {
    p = stpecpy (p, end, "sync,");
  }
#endif
#if defined (MNT_THROTTLE)
  if ((flags & MNT_THROTTLE) == MNT_THROTTLE) {
    p = stpecpy (p, end, "throttle,");
  }
#endif
#if defined (MNT_UNION)
  if ((flags & MNT_UNION) == MNT_UNION) {
    p = stpecpy (p, end, "union,");
  }
#endif
#if defined (MNT_UNION)
  if ((flags & MNT_UNION) == MNT_UNION) {
    p = stpecpy (p, end, "union,");
  }
#endif
#if defined (MNT_REMOVABLE)
  if ((flags & MNT_REMOVABLE) == MNT_REMOVABLE) {
    p = stpecpy (p, end, "removable,");
  }
#endif
#if defined (MNT_PERSISTENT)
  if ((flags & MNT_PERSISTENT) == MNT_PERSISTENT) {
    p = stpecpy (p, end, "persistent,");
  }
#endif
#if defined (MNT_SHARED)
  if ((flags & MNT_SHARED) == MNT_SHARED) {
    p = stpecpy (p, end, "shared,");
  }
#endif
#if defined (MNT_BLOCKBASED)
  if ((flags & MNT_BLOCKBASED) == MNT_BLOCKBASED) {
    p = stpecpy (p, end, "blockbased,");
  }
#endif
#if defined (MNT_HAS_MIME)
  if ((flags & MNT_HAS_MIME) == MNT_HAS_MIME) {
    p = stpecpy (p, end, "mime,");
  }
#endif
#if defined (MNT_HAS_QUERY)
  if ((flags & MNT_HAS_QUERY) == MNT_HAS_QUERY) {
    p = stpecpy (p, end, "query,");
  }
#endif
#if defined (MNT_HAS_ATTR)
  if ((flags & MNT_HAS_ATTR) == MNT_HAS_ATTR) {
    p = stpecpy (p, end, "attr,");
  }
#endif
  return;
}

void
convertNFSMountOptions (long flags, long wsize, long rsize, di_disk_info_t *diptr)
{
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunknown-warning-option"
#pragma clang diagnostic ignored "-Wunused-but-set-variable"
  char    *p;
  char    *end;

  p = diptr->strdata [DI_DISP_MOUNTOPT];
  end = diptr->strdata [DI_DISP_MOUNTOPT] + DI_MOUNTOPT_LEN;
#pragma clang diagnostic pop
#if defined (NFSMNT_SOFT)
  if ((flags & NFSMNT_SOFT) != NFSMNT_SOFT) {
    p = stpecpy (p, end, "hard,");
  }
#endif
#if defined (NFSMNT_WSIZE)
  if ((flags & NFSMNT_WSIZE) == NFSMNT_WSIZE) {
    char          tmp [64];

    Snprintf1 (tmp, sizeof (tmp), "wsize=%ld,", wsize);
    p = stpecpy (p, end, tmp);
  }
#endif
#if defined (NFSMNT_RSIZE)
  if ((flags & NFSMNT_RSIZE) == NFSMNT_RSIZE) {
    char          tmp [64];

    Snprintf1 (tmp, sizeof (tmp), "rsize=%ld,", rsize);
    p = stpecpy (p, end, tmp);
  }
#endif
#if defined (NFSMNT_INT) && defined (NFSMNT_SOFT)
  if ((flags & NFSMNT_SOFT) != NFSMNT_SOFT &&
      (flags & NFSMNT_INT) == NFSMNT_INT) {
    p = stpecpy (p, end, "intr,");
  }
#endif
#if defined (NFSMNT_TCP)
  if ((flags & NFSMNT_TCP) != NFSMNT_TCP) {
    p = stpecpy (p, end, "udp,");
  }
#endif
  return;
}


#if _lib_getmntent \
    && ! _lib_getmntinfo \
    && ! _lib_getfsstat \
    && ! _lib_getvfsstat \
	&& ! _lib_mntctl

char *
chkMountOptions (const char *mntopts, const char *str)
{
  char    *ptr;
  char    *tstr;
  char    *tokstr;

  tstr = strdup (mntopts);
  if (tstr == (char *) NULL) {
    fprintf (stderr, "strdup failed in chkMountOptions (1).  errno %d\n", errno);
    exit (1);
  }
  ptr = di_strtok (tstr, ",", &tokstr);
  while (ptr != (char *) NULL) {
    if (strcmp (ptr, str) == 0) {
      free (tstr);
      return ptr;
    }
    ptr = di_strtok ((char *) NULL, ",", &tokstr);
  }
  free (tstr);
  return (char *) NULL;
}

#endif /* _lib_getmntent */

void
di_is_remote_disk (di_disk_info_t *diskInfo)
{
  if (strncmp (diskInfo->strdata [DI_DISP_FSTYPE], "nfs", 3) == 0) {
    diskInfo->isLocal = false;
  }
}

int
di_isPooledFs (di_disk_info_t *diskInfo)
{
  if (strcmp (diskInfo->strdata [DI_DISP_FSTYPE], "zfs") == 0 ||
      strcmp (diskInfo->strdata [DI_DISP_FSTYPE], "advfs") == 0 ||
      strcmp (diskInfo->strdata [DI_DISP_FSTYPE], "apfs") == 0 ||
      (strcmp (diskInfo->strdata [DI_DISP_FSTYPE], "null") == 0 &&
       strstr (diskInfo->strdata [DI_DISP_FILESYSTEM], "/@@-") != (char *) NULL)) {
    return true;
  }
  return false;
}

int
di_isLoopbackFs (di_disk_info_t *diskInfo)
{
  if ((strcmp (diskInfo->strdata [DI_DISP_FSTYPE], "lofs") == 0 && diskInfo->sp_rdev != 0) ||
      (strcmp (diskInfo->strdata [DI_DISP_FSTYPE], "nullfs") == 0 &&
       strstr (diskInfo->strdata [DI_DISP_FILESYSTEM], "/@@-") == (char *) NULL) ||
      strcmp (diskInfo->strdata [DI_DISP_FSTYPE], "none") == 0) {
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

