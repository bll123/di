/*
 * $Id$
 * $Source$
 * Copyright 1994-2010 Brad Lanam, Walnut Creek, CA
 */

/********************************************************/
/*

    In the cases where di_getDiskEntries() does not
    get the volume information, di_getDiskInfo() is used
    to fetch the info.

    di_getDiskInfo ()
        Gets the disk space used/available on the
        partitions we want displayed.

*/
/********************************************************/

#include "config.h"
#include "di.h"
#include "dimntopt.h"

#include <stdio.h>
#if _hdr_stdlib
# include <stdlib.h>
#endif
#if _sys_types
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
#if _hdr_strings && ((! _hdr_string) || (_include_string))
# include <strings.h>
#endif

#if _sys_mount && \
  ! defined (_DI_INC_SYS_MOUNT) /* FreeBSD, OpenBSD, NetBSD, HP-UX */
# define _DI_INC_SYS_MOUNT 1
# include <sys/mount.h>         /* statfs(); struct statfs; getfsstat() */
#endif
#if _sys_statvfs                /* Linux, Solaris, FreeBSD, NetBSD, HP-UX */
# include <sys/statvfs.h>       /* statvfs(); struct statvfs */
#endif
#if _sys_vfs                    /* Linux, HP-UX, BSD 4.3 */
# include <sys/vfs.h>           /* struct statfs */
#endif
#if _sys_statfs && ! _sys_statvfs     /* Linux, SysV.3 */
# include <sys/statfs.h>                        /* statfs(); struct statfs */
#endif
#if _sys_fstyp                  /* SysV.3 */
# include <sys/fstyp.h>         /* sysfs() */
#endif
#if _hdr_windows            /* windows */
# include <windows.h>       /* GetDiskFreeSpace(); GetVolumeInformation() */
#endif

/********************************************************/

#if defined(__cplusplus)
  extern "C" {
#endif

#if ! _lib_statvfs && \
	_lib_statfs && \
	_npt_statfs
# if _lib_statfs && _statfs_args == 2
  extern int statfs _((char *, struct statfs *));
# endif
# if _lib_statfs && _statfs_args == 3
  extern int statfs _((char *, struct statfs *, int));
# endif
# if _lib_statfs && _statfs_args == 4
  extern int statfs _((char *, struct statfs *, int, int));
# endif
#endif

extern int debug;

#if defined(__cplusplus)
  }
#endif

/********************************************************/

#if _lib_statvfs && \
    ! _lib_fs_stat_dev && \
    ! _lib_getmntinfo && \
    ! _lib_getfsstat && \
    ! _lib_getvfsstat && \
    ! _lib_GetVolumeInformation && \
	! _class_os__Volumes

/*
 * di_getDiskInfo
 *
 * SysV.4.  statvfs () returns both the free and available blocks.
 *
 */

# define DI_GETDISKINFO_DEF 1

void
# if _proto_stdc
di_getDiskInfo (diDiskInfo_t **diskInfo, int *diCount)
# else
di_getDiskInfo (diskInfo, diCount)
    diDiskInfo_t **diskInfo;
    int *diCount;
# endif
{
    diDiskInfo_t     *diptr;
    int             i;
    Statvfs_t       statBuf;

    if (debug > 0) { printf ("# lib:getDiskInfo: statvfs\n"); }
    for (i = 0; i < *diCount; ++i)
    {
        diptr = *diskInfo + i;

        if (diptr->printFlag == DI_PRNT_OK ||
            diptr->printFlag == DI_PRNT_FORCE)
        {
            _fs_size_t      tblocksz;
            if (statvfs (diptr->name, &statBuf) == 0)
            {
                    /* data general DG/UX 5.4R3.00 sometime returns 0   */
                    /* in the fragment size field.                      */
                if (statBuf.f_frsize == 0 && statBuf.f_bsize != 0)
                {
                    tblocksz = statBuf.f_bsize;
                }
                else
                {
                    tblocksz = statBuf.f_frsize;
                }
/* Linux! statvfs() returns values in f_bsize rather f_frsize.  Bleah.  */
/* Non-POSIX!  Linux manual pages are incorrect.                        */
#  if linux
                tblocksz = statBuf.f_bsize;
#  endif /* linux */

                di_saveBlockSizes (diptr, tblocksz,
                    (_fs_size_t) statBuf.f_blocks,
                    (_fs_size_t) statBuf.f_bfree,
                    (_fs_size_t) statBuf.f_bavail);
                di_saveInodeSizes (diptr,
                    (_fs_size_t) statBuf.f_files,
                    (_fs_size_t) statBuf.f_ffree,
                    (_fs_size_t) statBuf.f_favail);

                if (debug > 1)
                {
                    printf ("%s: %s\n", diptr->name, diptr->fsType);
                    printf ("\tbsize:%ld  frsize:%ld\n", (long) statBuf.f_bsize,
                            (long) statBuf.f_frsize);
# if _siz_long_long >= 8
                    printf ("\tblocks: tot:%llu free:%lld avail:%llu\n",
                           statBuf.f_blocks, statBuf.f_bfree, statBuf.f_bavail);
                    printf ("\tinodes: tot:%llu free:%llu avail:%llu\n",
                            statBuf.f_files, statBuf.f_ffree, statBuf.f_favail);
# else
                    printf ("\tblocks: tot:%lu free:%lu avail:%lu\n",
                           statBuf.f_blocks, statBuf.f_bfree, statBuf.f_bavail);
                    printf ("\tinodes: tot:%lu free:%lu avail:%lu\n",
                            statBuf.f_files, statBuf.f_ffree, statBuf.f_favail);
# endif
                }
            }
            else
            {
                fprintf (stderr, "statvfs: %s ", diptr->name);
                perror ("");
            }
        }
    } /* for each entry */
}

#endif /* _lib_statvfs */

#if _lib_statfs && _statfs_args == 4 && \
    ! _lib_statvfs && \
    ! _lib_getmntinfo && \
    ! _lib_getfsstat && \
    ! _lib_getvfsstat && \
    ! _lib_getmnt

# if ! defined (UBSIZE)
#  if defined (BSIZE)
#   define UBSIZE            BSIZE
#  else
#   define UBSIZE            512
#  endif
# endif

/*
 * di_getDiskInfo
 *
 * SysV.3.  We don't have available blocks; just set it to free blocks.
 * The sysfs () call is used to get the disk type name.
 *
 */

# define DI_GETDISKINFO_DEF 1

void
# if _proto_stdc
di_getDiskInfo (diDiskInfo_t **diskInfo, int *diCount)
# else
di_getDiskInfo (diskInfo, diCount)
    diDiskInfo_t **diskInfo;
    int *diCount;
# endif
{
    diDiskInfo_t     *diptr;
    int             i;
    struct statfs   statBuf;

    if (debug > 0) { printf ("# lib:getDiskInfo: sysv-statfs 4arg\n"); }
    for (i = 0; i < *diCount; ++i)
    {
        diptr = *diskInfo + i;
        if (diptr->printFlag == DI_PRNT_OK ||
            diptr->printFlag == DI_PRNT_FORCE)
        {
            _fs_size_t      tblocksz;

            if (statfs (diptr->name, &statBuf, sizeof (statBuf), 0) == 0)
            {
# if _mem_f_frsize_statfs
                if (statBuf.f_frsize == 0 && statBuf.f_bsize != 0)
                {
                    tblocksz = (_fs_size_t) statBuf.f_bsize;
                }
                else
                {
                    tblocksz = (_fs_size_t) statBuf.f_frsize;
                }
# else
                tblocksz = UBSIZE;
# endif
                di_saveBlockSizes (diptr, tblocksz,
                    (_fs_size_t) statBuf.f_blocks,
                    (_fs_size_t) statBuf.f_bfree,
                    (_fs_size_t) statBuf.f_bfree);
                di_saveInodeSizes (diptr,
                    (_fs_size_t) statBuf.f_files,
                    (_fs_size_t) statBuf.f_ffree,
                    (_fs_size_t) statBuf.f_ffree);
# if _lib_sysfs && _mem_f_fstyp_statfs
                sysfs (GETFSTYP, statBuf.f_fstyp, diptr->fsType);
# endif

                if (debug > 1)
                {
                    printf ("%s: %s\n", diptr->name, diptr->fsType);
# if _mem_f_frsize_statfs
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
                fprintf (stderr, "statfs: %s ", diptr->name);
                perror ("");
            }
        }
    } /* for each entry */
}

#endif /* _statfs_args == 4 */

#if _lib_statfs && (_statfs_args == 2 || _statfs_args == 3) && \
        ! _lib_statvfs && \
        ! _lib_getmntinfo && \
        ! _lib_getfsstat && \
        ! _lib_getmnt && \
        ! _lib_GetDiskFreeSpace && \
        ! _lib_GetDiskFreeSpaceEx

/*
 * di_getDiskInfo
 *
 * SunOS/BSD/Pyramid/Some Linux
 *
 */

# define DI_GETDISKINFO_DEF 1

void
# if _proto_stdc
di_getDiskInfo (diDiskInfo_t **diskInfo, int *diCount)
# else
di_getDiskInfo (diskInfo, diCount)
    diDiskInfo_t **diskInfo;
    int *diCount;
# endif
{
    diDiskInfo_t     *diptr;
    int             i;
    struct statfs   statBuf;

    if (debug > 0) { printf ("# lib:getDiskInfo: bsd-statfs 2/3arg\n"); }
    for (i = 0; i < *diCount; ++i)
    {
        diptr = *diskInfo + i;
        if (diptr->printFlag == DI_PRNT_OK ||
            diptr->printFlag == DI_PRNT_FORCE)
        {
            if (statfs (diptr->name, &statBuf) == 0)
            {
                di_saveBlockSizes (diptr, (_fs_size_t) statBuf.f_bsize,
                    (_fs_size_t) statBuf.f_blocks,
                    (_fs_size_t) statBuf.f_bfree,
                    (_fs_size_t) statBuf.f_bavail);
                di_saveInodeSizes (diptr,
                    (_fs_size_t) statBuf.f_files,
                    (_fs_size_t) statBuf.f_ffree,
                    (_fs_size_t) statBuf.f_ffree);

# if _lib_sysfs && _mem_f_fstyp_statfs
                sysfs (GETFSTYP, statBuf.f_fstyp, diptr->fsType);
# endif

                if (debug > 1)
                {
                    printf ("%s: %s\n", diptr->name, diptr->fsType);
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
                fprintf (stderr, "statfs: %s ", diptr->name);
                perror ("");
            }
        }
    } /* for each entry */
}

#endif /* _statfs_args == 2 or 3 */


#if _lib_GetVolumeInformation

/*
 * di_getDiskInfo
 *
 * Windows
 *
 */

# define DI_GETDISKINFO_DEF 1

# define MSDOS_BUFFER_SIZE          256

void
# if _proto_stdc
di_getDiskInfo (diDiskInfo_t **diskInfo, int *diCount)
# else
di_getDiskInfo (diskInfo, diCount)
    diDiskInfo_t **diskInfo;
    int *diCount;
# endif
{
    diDiskInfo_t         *diptr;
    int                 i;
    int                 rc;
    char                volName [MSDOS_BUFFER_SIZE];
    char                fsName [MSDOS_BUFFER_SIZE];
    DWORD               serialNo;
    DWORD               maxCompLen;
    DWORD               fsFlags;


    if (debug > 0) { printf ("# lib:getDiskInfo: GetVolumeInformation\n"); }
    for (i = 0; i < *diCount; ++i)
    {
        diptr = *diskInfo + i;
        if (diptr->printFlag == DI_PRNT_OK ||
            diptr->printFlag == DI_PRNT_FORCE)
        {
            rc = GetVolumeInformation (diptr->name,
                    volName, MSDOS_BUFFER_SIZE, &serialNo, &maxCompLen,
                    &fsFlags, fsName, MSDOS_BUFFER_SIZE);
            strncpy (diptr->fsType, fsName, DI_TYPE_LEN);
            strncpy (diptr->special, volName, DI_SPEC_NAME_LEN);

# if _lib_GetDiskFreeSpaceEx
            {
                ULONGLONG bytesAvail;
                ULONGLONG bytesTotal;
                ULONGLONG bytesFree;

                rc = GetDiskFreeSpaceEx (diptr->name,
                        (PULARGE_INTEGER) &bytesAvail,
                        (PULARGE_INTEGER) &bytesTotal,
                        (PULARGE_INTEGER) &bytesFree);
                if (rc > 0)
                {
                    di_saveBlockSizes (diptr, (_fs_size_t) 1,
                        (_fs_size_t) bytesTotal,
                        (_fs_size_t) bytesFree,
                        (_fs_size_t) bytesAvail);
                    di_saveInodeSizes (diptr,
                        (_fs_size_t) 0,
                        (_fs_size_t) 0,
                        (_fs_size_t) 0);
                }
                else
                {
                    diptr->printFlag = DI_PRNT_BAD;
                    if (debug)
                    {
                        printf ("disk %s; could not get disk space\n",
                                diptr->name);
                    }
                }

                if (debug > 1)
                {
                    printf ("%s: %s\n", diptr->name, diptr->fsType);
                    printf ("\ttot:%llu  free:%llu\n",
                            bytesTotal, bytesFree);
                    printf ("\tavail:%llu\n", bytesAvail);
                }
            }
# else
#  if _lib_GetDiskFreeSpace
            {
                unsigned long           sectorspercluster;
                unsigned long           bytespersector;
                unsigned long           totalclusters;
                unsigned long           freeclusters;

                rc = GetDiskFreeSpace (diptr->name,
                        (LPDWORD) &sectorspercluster,
                        (LPDWORD) &bytespersector,
                        (LPDWORD) &freeclusters,
                        (LPDWORD) &totalclusters);
                if (rc > 0)
                {
                    di_saveBlockSizes (diptr,
                        (_fs_size_t) (sectorspercluster * bytespersector),
                        (_fs_size_t) totalclusters,
                        (_fs_size_t) freeclusters,
                        (_fs_size_t) freeclusters);
                    di_saveInodeSizes (diptr,
                        (_fs_size_t) 0,
                        (_fs_size_t) 0,
                        (_fs_size_t) 0);
                }
                else
                {
                    diptr->printFlag = DI_PRNT_BAD;
                    if (debug)
                    {
                        printf ("disk %s; could not get disk space\n",
                                diptr->name);
                    }
                }

                if (debug > 1)
                {
                    printf ("%s: %s\n", diptr->name, diptr->fsType);
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
#  if _proto_stdc
di_getDiskInfo (diDiskInfo_t **diskInfo, int *diCount)
#  else
di_getDiskInfo (diskInfo, diCount)
    diDiskInfo_t **diskInfo;
    int *diCount;
#  endif
{
    if (debug > 0) { printf ("# lib:getDiskInfo: empty\n"); }
    return;
}
#endif
