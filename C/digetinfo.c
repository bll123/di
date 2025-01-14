/*
 * Copyright 1994-2018 Brad Lanam, Walnut Creek, CA
 * Copyright 2023 Brad Lanam, Pleasant Hill, CA
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

#if _sys_mount \
  && ! defined (DI_INC_SYS_MOUNT) /* FreeBSD, OpenBSD, NetBSD, HP-UX */
# define DI_INC_SYS_MOUNT 1
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

#if defined (__cplusplus) || defined (c_plusplus)
extern "C" {
#endif

#if ! _lib_statvfs \
	&& _lib_statfs \
	&& _npt_statfs
# if _lib_statfs && _args_statfs == 2
  extern int statfs _((char *, struct statfs *));
# endif
# if _lib_statfs && _args_statfs == 3
  extern int statfs _((char *, struct statfs *, int));
# endif
# if _lib_statfs && _args_statfs == 4
  extern int statfs _((char *, struct statfs *, int, int));
# endif
#endif

#if defined (__cplusplus) || defined (c_plusplus)
}
#endif

extern int debug;

/********************************************************/

#if _lib_statvfs \
    && ! _lib_fs_stat_dev \
    && ! _lib_getmntinfo \
    && ! _lib_getfsstat \
    && ! _lib_getvfsstat \
    && ! _lib_GetVolumeInformation \
	&& ! _class_os__Volumes

/*
 * di_getDiskInfo
 *
 * SysV.4.  statvfs () returns both the free and available blocks.
 *
 */

# define DI_GETDISKINFO_DEF 1

void
di_getDiskInfo (diDiskInfo_t **diskInfo, int *diCount)
{
    diDiskInfo_t     *diptr;
    int             i;
    Statvfs_t       statBuf;

    if (debug > 0) { printf ("# getDiskInfo: statvfs\n"); }
    for (i = 0; i < *diCount; ++i)
    {
        diptr = *diskInfo + i;

        if (diptr->printFlag == DI_PRNT_OK ||
            diptr->printFlag == DI_PRNT_SKIP ||
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
# if _mem_struct_statvfs_f_basetype
                if (! *diptr->fsType) {
                  strncpy (diptr->fsType, statBuf.f_basetype, DI_TYPE_LEN);
                }
# endif

                if (debug > 1)
                {
                    printf ("%s: %s\n", diptr->name, diptr->fsType);
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
                fprintf (stderr, "statvfs: %s ", diptr->name);
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
 * di_getDiskInfo
 *
 * SysV.3.  We don't have available blocks; just set it to free blocks.
 * The sysfs () call is used to get the disk type name.
 *
 */

# define DI_GETDISKINFO_DEF 1

void
di_getDiskInfo (diDiskInfo_t **diskInfo, int *diCount)
{
    diDiskInfo_t     *diptr;
    int             i;
    struct statfs   statBuf;

    if (debug > 0) { printf ("# getDiskInfo: sysv-statfs 4arg\n"); }
    for (i = 0; i < *diCount; ++i)
    {
        diptr = *diskInfo + i;
        if (diptr->printFlag == DI_PRNT_OK ||
            diptr->printFlag == DI_PRNT_SKIP ||
            diptr->printFlag == DI_PRNT_FORCE)
        {
            _fs_size_t      tblocksz;

            if (statfs (diptr->name, &statBuf, sizeof (statBuf), 0) == 0)
            {
# if _mem_struct_statfs_f_frsize
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
# if _lib_sysfs && _mem_struct_statfs_f_fstyp
                sysfs (GETFSTYP, statBuf.f_fstyp, diptr->fsType);
# endif

                if (debug > 1)
                {
                    printf ("%s: %s\n", diptr->name, diptr->fsType);
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
                fprintf (stderr, "statfs: %s ", diptr->name);
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
 * di_getDiskInfo
 *
 * SunOS/BSD/Pyramid/Some Linux
 *
 */

# define DI_GETDISKINFO_DEF 1

void
di_getDiskInfo (diDiskInfo_t **diskInfo, int *diCount)
{
    diDiskInfo_t     *diptr;
    int             i;
    struct statfs   statBuf;

    if (debug > 0) { printf ("# getDiskInfo: bsd-statfs 2/3arg\n"); }
    for (i = 0; i < *diCount; ++i)
    {
        diptr = *diskInfo + i;
        if (diptr->printFlag == DI_PRNT_OK ||
            diptr->printFlag == DI_PRNT_SKIP ||
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

# if _lib_sysfs && _mem_struct_statfs_f_fstyp
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
              diptr->printFlag = DI_PRNT_BAD;
              if (errno != EACCES && errno != EPERM) {
                fprintf (stderr, "statfs: %s ", diptr->name);
                perror ("");
              }
            }
        }
    } /* for each entry */
}

#endif /* _args_statfs == 2 or 3 */


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
di_getDiskInfo (diDiskInfo_t **diskInfo, int *diCount)
{
    diDiskInfo_t         *diptr;
    int                 i;
    int                 rc;
    char                volName [MSDOS_BUFFER_SIZE];
    char                fsName [MSDOS_BUFFER_SIZE];
    DWORD               serialNo;
    DWORD               maxCompLen;
    DWORD               fsFlags;


# if _lib_GetDiskFreeSpaceEx
    if (debug > 0) { printf ("# getDiskInfo: GetDiskFreeSpaceEx\n"); }
# endif
# if _lib_GetDiskFreeSpace
    if (debug > 0) { printf ("# getDiskInfo: GetDiskFreeSpace\n"); }
# endif
    if (debug > 0) { printf ("# getDiskInfo: GetVolumeInformation\n"); }
    for (i = 0; i < *diCount; ++i)
    {
        diptr = *diskInfo + i;
        if (diptr->printFlag == DI_PRNT_OK ||
            diptr->printFlag == DI_PRNT_SKIP ||
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
di_getDiskInfo (diDiskInfo_t **diskInfo, int *diCount)
{
    if (debug > 0) { printf ("# getDiskInfo: empty\n"); }
    return;
}
#endif
