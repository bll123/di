/*
 * Copyright 1994-2018 Brad Lanam, Walnut Creek, CA
 * Copyright 2023-2025 Brad Lanam, Pleasant Hill, CA
 */

/*
 *
 *    In the cases where di_get_disk_entries () does not
 *    get the volume information, di_get_disk_info () is used
 *    to fetch the info.
 *
 *    di_get_disk_info ()
 *        Gets the disk space used/available on the
 *        partitions we want displayed.
 *
 */

#include "config.h"

#if _hdr_stdio
# include <stdio.h>
#endif
#if _hdr_stdlib
# include <stdlib.h>
#endif
#if _sys_types \
    && ! defined (DI_INC_SYS_TYPES_H) /* xenix */
# define DI_INC_SYS_TYPES_H
# include <sys/types.h>
#endif
#if _sys_param
# include <sys/param.h>
#endif
#if _hdr_errno
# include <errno.h>
#endif
#if _hdr_string
# include <string.h>
#endif
#if _hdr_strings
# include <strings.h>
#endif

/* FreeBSD, OpenBSD, NetBSD, HP-UX, MacOS */
#if _sys_mount && ! defined (DI_INC_SYS_MOUNT)
# define DI_INC_SYS_MOUNT 1
# include <sys/mount.h>         /* statfs (); struct statfs; getfsstat () */
#endif
#if _sys_statvfs                /* Linux, Solaris, FreeBSD, NetBSD, HP-UX */
# include <sys/statvfs.h>       /* statvfs (); struct statvfs */
#endif
#if _sys_vfs                    /* Linux, HP-UX, BSD 4.3 */
# include <sys/vfs.h>           /* struct statfs */
#endif
#if _sys_statfs && ! _sys_statvfs     /* Linux, SysV.3 */
# include <sys/statfs.h>                        /* statfs (); struct statfs */
#endif
#if _sys_fstyp                  /* SysV.3 */
# include <sys/fstyp.h>         /* sysfs () */
#endif
#if _hdr_windows            /* windows */
# include <windows.h>       /* GetDiskFreeSpace (); GetVolumeInformation () */
#endif

#include "di.h"
#include "disystem.h"
#include "diinternal.h"
#include "dimntopt.h"
#include "distrutils.h"

/********************************************************/

#if defined (__cplusplus) || defined (c_plusplus)
extern "C" {
#endif

#if ! _lib_statvfs \
	&& _lib_statfs \
	&& _npt_statfs
# if _lib_statfs && _args_statfs == 2
  extern int statfs (char *, struct statfs *);
# endif
# if _lib_statfs && _args_statfs == 3
  extern int statfs (char *, struct statfs *, int);
# endif
# if _lib_statfs && _args_statfs == 4
  extern int statfs (char *, struct statfs *, int, int);
# endif
#endif

#if defined (__cplusplus) || defined (c_plusplus)
}
#endif

/********************************************************/

#if _lib_statvfs \
    && ! _lib_fs_stat_dev \
    && ! _lib_getmntinfo \
    && ! _lib_getfsstat \
    && ! _lib_getvfsstat \
    && ! _lib_GetVolumeInformation

/*
 * di_get_disk_info
 *
 * SysV.4.  statvfs () returns both the free and available blocks.
 *
 */

# define DI_GETDISKINFO_DEF 1

void
di_get_disk_info (di_data_t *di_data, int *diCount)
{
  di_disk_info_t  *diptr;
  int             i;
  Statvfs_t       statBuf;
  di_opt_t        *diopts;

  diopts = (di_opt_t *) di_data->options;

  if (diopts->optval [DI_OPT_DEBUG] > 0) { printf ("# di_get_disk_info: statvfs\n"); }
  for (i = 0; i < *diCount; ++i) {
    diptr = di_data->diskInfo + i;

    if (diptr->printFlag == DI_PRNT_OK ||
        diptr->printFlag == DI_PRNT_SKIP ||
        diptr->printFlag == DI_PRNT_FORCE) {
      di_ui_t    tblocksz;

      if (statvfs (diptr->strdata [DI_DISP_MOUNTPT], &statBuf) == 0) {
        /* data general DG/UX 5.4R3.00 sometime returns 0   */
        /* in the fragment size field.                      */
        if (statBuf.f_frsize == 0 && statBuf.f_bsize != 0) {
          tblocksz = statBuf.f_bsize;
        } else {
          tblocksz = statBuf.f_frsize;
        }
/* Linux! statvfs () returns values in f_bsize rather f_frsize.  Bleah.  */
/* Non-POSIX!  Linux manual pages are incorrect.                        */
#  if defined (linux)
        tblocksz = statBuf.f_bsize;
#  endif /* linux */

        di_save_block_sizes (diptr, tblocksz,
            (di_ui_t) statBuf.f_blocks,
            (di_ui_t) statBuf.f_bfree, (di_ui_t) statBuf.f_bavail);
        di_save_inode_sizes (diptr, (di_ui_t) statBuf.f_files,
            (di_ui_t) statBuf.f_ffree, (di_ui_t) statBuf.f_favail);
# if _mem_struct_statvfs_f_basetype
        if (! *diptr->strdata [DI_DISP_FSTYPE]) {
          stpecpy (diptr->strdata [DI_DISP_FSTYPE],
              diptr->strdata [DI_DISP_FSTYPE] + DI_FSTYPE_LEN,
              statBuf.f_basetype);
        }
# endif

        if (diopts->optval [DI_OPT_DEBUG] > 1)
        {
          printf ("%s: %s\n", diptr->strdata [DI_DISP_MOUNTPT], diptr->strdata [DI_DISP_FSTYPE]);
          printf ("\tbsize:%ld  frsize:%ld\n", (long) statBuf.f_bsize,
                  (long) statBuf.f_frsize);
# if _siz_long_long >= 8
          printf ("\tblocks: tot:%llu free:%lld avail:%llu\n",
              (long long) statBuf.f_blocks,
              (long long) statBuf.f_bfree,
              (long long) statBuf.f_bavail);
          printf ("\tinodes: tot:%llu free:%llu avail:%llu\n",
              (long long) statBuf.f_files,
              (long long) statBuf.f_ffree,
              (long long) statBuf.f_favail);
# else
          printf ("\tblocks: tot:%lu free:%lu avail:%lu\n",
              (long) statBuf.f_blocks, (long) statBuf.f_bfree,
              (long) statBuf.f_bavail);
          printf ("\tinodes: tot:%lu free:%lu avail:%lu\n",
              (long) statBuf.f_files, (long) statBuf.f_ffree,
              (long) statBuf.f_favail);
# endif
        }
      }
      else
      {
        diptr->printFlag = DI_PRNT_BAD;
        if (errno != EACCES && errno != EPERM) {
          fprintf (stderr, "statvfs: %s ", diptr->strdata [DI_DISP_MOUNTPT]);
          perror ("");
        }
      }
    }
  } /* for each entry */
}

#endif /* _lib_statvfs */

#if _lib_statfs && _args_statfs == 4 \
    && ! _lib_statvfs \
    && ! _lib_getmntinfo \
    && ! _lib_getfsstat \
    && ! _lib_getvfsstat \
    && ! _lib_getmnt

  /* xenix reports a block size of 1024 bytes, but the blocks reported */
  /* are based on a 512 byte block size.                               */
# if defined (M_XENIX)
#  define UBSIZE            512
# else
#  if ! defined (UBSIZE)
#   if defined (BSIZE)
#    define UBSIZE            BSIZE
#   else
#    define UBSIZE            512
#   endif
#  endif
# endif

/*
 * di_get_disk_info
 *
 * SysV.3.  We don't have available blocks; just set it to free blocks.
 * The sysfs () call is used to get the disk type name.
 *
 */

# define DI_GETDISKINFO_DEF 1

void
di_get_disk_info (di_data_t *di_data, int *diCount)
{
  di_disk_info_t  *diptr;
  int             i;
  struct statfs   statBuf;
  di_opt_t        *diopts;

  diopts = (di_opt_t *) di_data->options;

  if (diopts->optval [DI_OPT_DEBUG] > 0) { printf ("# di_get_disk_info: sysv-statfs 4arg\n"); }
  for (i = 0; i < *diCount; ++i) {
    diptr = di_data->diskInfo + i;
    if (diptr->printFlag == DI_PRNT_OK ||
        diptr->printFlag == DI_PRNT_SKIP ||
        diptr->printFlag == DI_PRNT_FORCE) {
      dinum_t      tblocksz;

      if (statfs (diptr->strdata [DI_DISP_MOUNTPT], &statBuf, sizeof (statBuf), 0) == 0) {
# if _mem_struct_statfs_f_frsize
        if (statBuf.f_frsize == 0 && statBuf.f_bsize != 0) {
          tblocksz = (dinum_t) statBuf.f_bsize;
        } else {
          tblocksz = (dinum_t) statBuf.f_frsize;
        }
# else
        tblocksz = UBSIZE;
# endif
        di_save_block_sizes (diptr, tblocksz, statBuf.f_blocks,
            statBuf.f_bfree, statBuf.f_bfree);
        di_save_inode_sizes (diptr, statBuf.f_files,
            statBuf.f_ffree, statBuf.f_ffree);
# if _lib_sysfs && _mem_struct_statfs_f_fstyp
        sysfs (GETFSTYP, statBuf.f_fstyp, diptr->strdata [DI_DISP_FSTYPE]);
# endif

        if (diopts->optval [DI_OPT_DEBUG] > 1) {
          printf ("%s: %s\n", diptr->strdata [DI_DISP_MOUNTPT], diptr->strdata [DI_DISP_FSTYPE]);
# if _mem_struct_statfs_f_frsize
          printf ("\tbsize:%ld\n", statBuf.f_bsize);
          printf ("\tfrsize:%ld\n", statBuf.f_frsize);
# else
          printf ("\tUBSIZE:%ld\n", UBSIZE);
# endif
          printf ("\tblocks: tot:%ld free:%ld\n",
                  statBuf.f_blocks, statBuf.f_bfree);
          printf ("\tinodes: tot:%ld free:%ld\n",
                  statBuf.f_files, statBuf.f_ffree);
        }
      } /* if we got the info */
      else
      {
        diptr->printFlag = DI_PRNT_BAD;
        if (errno != EACCES && errno != EPERM) {
          fprintf (stderr, "statfs: %s ", diptr->strdata [DI_DISP_MOUNTPT]);
          perror ("");
        }
      }
    }
  } /* for each entry */
}

#endif /* _args_statfs == 4 */

#if _lib_statfs && (_args_statfs == 2 || _args_statfs == 3) \
        && ! _lib_statvfs \
        && ! _lib_getmntinfo \
        && ! _lib_getfsstat \
        && ! _lib_getmnt \
        && ! _lib_GetDiskFreeSpace \
        && ! _lib_GetDiskFreeSpaceEx

/*
 * di_get_disk_info
 *
 * SunOS/BSD/Pyramid/Some Linux
 *
 */

# define DI_GETDISKINFO_DEF 1

void
di_get_disk_info (di_data_t *di_data, int *diCount)
{
  di_disk_info_t     *diptr;
  int             i;
  struct statfs   statBuf;
  di_opt_t        *diopts;

  diopts = (di_opt_t *) di_data->options;

  if (diopts->optval [DI_OPT_DEBUG] > 0) { printf ("# di_get_disk_info: bsd-statfs 2/3arg\n"); }
  for (i = 0; i < *diCount; ++i) {
    diptr = di_data->diskInfo + i;
    if (diptr->printFlag == DI_PRNT_OK ||
        diptr->printFlag == DI_PRNT_SKIP ||
        diptr->printFlag == DI_PRNT_FORCE) {
      if (statfs (diptr->strdata [DI_DISP_MOUNTPT], &statBuf) == 0) {
        di_save_block_sizes (diptr, statBuf.f_bsize, statBuf.f_blocks,
            statBuf.f_bfree, statBuf.f_bavail);
        di_save_inode_sizes (diptr, statBuf.f_files,
            statBuf.f_ffree, statBuf.f_ffree);

# if _lib_sysfs && _mem_struct_statfs_f_fstyp
        sysfs (GETFSTYP, statBuf.f_fstyp, diptr->strdata [DI_DISP_FSTYPE]);
# endif

        if (diopts->optval [DI_OPT_DEBUG] > 1)
        {
          printf ("%s: %s\n", diptr->strdata [DI_DISP_MOUNTPT], diptr->strdata [DI_DISP_FSTYPE]);
          printf ("\tbsize:%ld\n", (long) statBuf.f_bsize);
          printf ("\tblocks: tot:%ld free:%ld avail:%ld\n",
              (long) statBuf.f_blocks, (long) statBuf.f_bfree,
              (long) statBuf.f_bavail);
          printf ("\tinodes: tot:%ld free:%ld\n",
              (long) statBuf.f_files, (long) statBuf.f_ffree);
        }
      } /* if we got the info */
      else
      {
        diptr->printFlag = DI_PRNT_BAD;
        if (errno != EACCES && errno != EPERM) {
          fprintf (stderr, "statfs: %s ", diptr->strdata [DI_DISP_MOUNTPT]);
          perror ("");
        }
      }
    }
  } /* for each entry */
}

#endif /* _args_statfs == 2 or 3 */


#if _lib_GetVolumeInformation

/*
 * di_get_disk_info
 *
 * Windows
 *
 */

# define DI_GETDISKINFO_DEF 1

# define MSDOS_BUFFER_SIZE          256

void
di_get_disk_info (di_data_t *di_data, int *diCount)
{
  di_disk_info_t         *diptr;
  int                 i;
  int                 rc;
  char                volName [MSDOS_BUFFER_SIZE];
  char                fsName [MSDOS_BUFFER_SIZE];
  DWORD               serialNo;
  DWORD               maxCompLen;
  DWORD               fsFlags;
  di_opt_t        *diopts;

  diopts = (di_opt_t *) di_data->options;

# if _lib_GetDiskFreeSpaceEx
  if (diopts->optval [DI_OPT_DEBUG] > 0) { printf ("# di_get_disk_info: GetDiskFreeSpaceEx\n"); }
# endif
# if _lib_GetDiskFreeSpace
  if (diopts->optval [DI_OPT_DEBUG] > 0) { printf ("# di_get_disk_info: GetDiskFreeSpace\n"); }
# endif
  if (diopts->optval [DI_OPT_DEBUG] > 0) { printf ("# di_get_disk_info: GetVolumeInformation\n"); }
  for (i = 0; i < *diCount; ++i) {
    diptr = di_data->diskInfo + i;
    if (diptr->printFlag == DI_PRNT_OK ||
        diptr->printFlag == DI_PRNT_SKIP ||
        diptr->printFlag == DI_PRNT_FORCE) {
      rc = GetVolumeInformation (diptr->strdata [DI_DISP_MOUNTPT],
          volName, MSDOS_BUFFER_SIZE, &serialNo, &maxCompLen,
          &fsFlags, fsName, MSDOS_BUFFER_SIZE);
      stpecpy (diptr->strdata [DI_DISP_FSTYPE],
          diptr->strdata [DI_DISP_FSTYPE] + DI_FSTYPE_LEN,
          fsName);
      stpecpy (diptr->strdata [DI_DISP_FILESYSTEM],
          diptr->strdata [DI_DISP_FILESYSTEM] + DI_FILESYSTEM_LEN,
          volName);

# if _lib_GetDiskFreeSpaceEx
      {
          ULONGLONG bytesAvail;
          ULONGLONG bytesTotal;
          ULONGLONG bytesFree;

          rc = GetDiskFreeSpaceEx (diptr->strdata [DI_DISP_MOUNTPT],
              (PULARGE_INTEGER) &bytesAvail,
              (PULARGE_INTEGER) &bytesTotal,
              (PULARGE_INTEGER) &bytesFree);
          if (rc > 0) {
            di_save_block_sizes (diptr, 1, bytesTotal,
                bytesFree, bytesAvail);
            di_save_inode_sizes (diptr, 0, 0, 0);
          }
          else
          {
            diptr->printFlag = DI_PRNT_BAD;
            if (diopts->optval [DI_OPT_DEBUG])
            {
              printf ("disk %s; could not get disk space\n",
                  diptr->strdata [DI_DISP_MOUNTPT]);
            }
          }

          if (diopts->optval [DI_OPT_DEBUG] > 1) {
            printf ("%s: %s\n", diptr->strdata [DI_DISP_MOUNTPT], diptr->strdata [DI_DISP_FSTYPE]);
            printf ("\ttot:%llu  free:%llu\n",
                bytesTotal, bytesFree);
            printf ("\tavail:%llu\n", bytesAvail);
          }
      }
# else
#  if _lib_GetDiskFreeSpace
      {
        unsigned long   sectorspercluster;
        unsigned long   bytespersector;
        unsigned long   totalclusters;
        unsigned long   freeclusters;

        rc = GetDiskFreeSpace (diptr->strdata [DI_DISP_MOUNTPT],
            (LPDWORD) &sectorspercluster,
            (LPDWORD) &bytespersector,
            (LPDWORD) &freeclusters,
            (LPDWORD) &totalclusters);
        if (rc > 0) {
          di_save_block_sizes (diptr,
              (sectorspercluster * bytespersector),
              totalclusters, freeclusters, freeclusters);
          di_save_inode_sizes (diptr, 0, 0, 0);
        } else {
          diptr->printFlag = DI_PRNT_BAD;
          if (diopts->optval [DI_OPT_DEBUG]) {
            printf ("disk %s; could not get disk space\n",
                diptr->strdata [DI_DISP_MOUNTPT]);
          }
        }

        if (diopts->optval [DI_OPT_DEBUG] > 1)
        {
          printf ("%s: %s\n", diptr->strdata [DI_DISP_MOUNTPT], diptr->strdata [DI_DISP_FSTYPE]);
          printf ("\ts/c:%ld  b/s:%ld\n", sectorspercluster,
              bytespersector);
          printf ("\tclusters: tot:%ld free:%ld\n",
              totalclusters, freeclusters);
        }
      }
#  endif
# endif
    } /* if printable drive */
  } /* for each mounted drive */
}

#endif  /* _lib_GetVolumeInformation */

#if ! defined (DI_GETDISKINFO_DEF)
void
di_get_disk_info (di_data_t *di_data, int *diCount)
{
  di_opt_t        *diopts;

  diopts = (di_opt_t *) di_data->options;
  if (diopts->optval [DI_OPT_DEBUG] > 0) { printf ("# di_get_disk_info: empty\n"); }
  return;
}
#endif
